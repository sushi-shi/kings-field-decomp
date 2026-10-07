"""Census written integer literals by semantic sink and join them into value domains.

Run in nix develop: kf literals [--image game] [--output build/literals.json]

Every manifest C variant is parsed with the retail MIPS flags (pylibclang, as
pointer_zeros and enums do). Each written integer or character literal, and
each reference to an enum constant, is attributed to its spelling location and
to the sink that consumes it: the field, variable, parameter or return value
it is assigned to or compared with, the switch subject of its case label, the
array it indexes, the operand of a mask/shift/arithmetic operator, and so on.

Value flow between sinks (copies, call arguments, returns, comparisons of two
sinks, case labels echoed into another sink, and slots indexing the same
array) is unioned into candidate domains. A domain is a search lead for one
enum: it is not proof that the joined sinks share a meaning, and a sink with
very many distinct flow partners is reported as a hub instead of joining them.
The report never edits sources.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
from dataclasses import asdict, dataclass, field
import json
import multiprocessing
import os
from pathlib import Path
import re
import sys
from typing import Any, Iterable

from scripts.kf.check_types import select_units
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import REPO


IMAGES = {"psx": "PSX.EXE", "game": "GAME.EXE", "open": "OPEN.EXE"}
NUMBER = re.compile(r"(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*")
CHARACTER = re.compile(r"'(?:[^'\\]|\\.)+'")
IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
WRITTEN = {"literal", "character"}

# Sinks that carry a value into a slot. Everything else is an operator or a
# declaration context that does not by itself name a value domain.
FLOW_SINKS = {"assign", "init", "compare", "case", "argument", "return"}
# SDK geometry/colour records: their members are quantities, not codes.
GEOMETRY_RECORDS = {
    "SVECTOR", "VECTOR", "DVECTOR", "CVECTOR", "MATRIX", "RECT", "RECT32", "DRAWENV",
    "DISPENV", "POLY_F3", "POLY_F4", "POLY_FT3", "POLY_FT4", "POLY_G3", "POLY_G4",
    "POLY_GT3", "POLY_GT4", "SPRT", "SPRT_8", "SPRT_16", "TILE", "LINE_F2", "LINE_G2",
}
GEOMETRY_FIELDS = {"vx", "vy", "vz", "pad", "r", "g", "b", "r0", "g0", "b0", "r1", "g1",
                   "b1", "r2", "g2", "b2", "r3", "g3", "b3", "x", "y", "w", "h", "x0", "y0",
                   "x1", "y1", "x2", "y2", "x3", "y3", "u0", "v0", "u1", "v1", "u2", "v2",
                   "u3", "v3", "m", "t"}
# Hubs: a slot with more distinct flow partners than this is reported, not unioned.
HUB_DEGREE = 12
LOCAL_HUB_DEGREE = 4


@dataclass(frozen=True, order=True)
class Sink:
    kind: str
    target: str = ""
    target_type: str = ""
    width: int = 0
    operator: str = ""


@dataclass(frozen=True, order=True)
class Site:
    file: str
    line: int
    column: int
    offset: int
    spelling: str
    value: int
    origin: str          # literal | character | enum-constant
    constant: str        # enum constant name, or the macro that wrote the literal
    constant_file: str
    sink: Sink
    function: str
    line_text: str
    spelled: str = ""    # file:offset of a macro body literal


@dataclass(frozen=True, order=True)
class Edge:
    left: str
    right: str
    kind: str
    file: str
    line: int


@dataclass
class UnitFacts:
    sites: list[tuple[Site, str, str]] = field(default_factory=list)  # site, image, unit
    edges: set[Edge] = field(default_factory=set)
    slots: dict[str, dict[str, Any]] = field(default_factory=dict)
    counters: set[str] = field(default_factory=set)
    declared: dict[str, tuple[int, str]] = field(default_factory=dict)  # enum constant
    errors: list[str] = field(default_factory=list)


def _same(left: Any, right: Any) -> bool:
    return left is not None and right is not None and left == right


def _cindex():
    try:
        from pylibclang import cindex, _C  # type: ignore[import-not-found]
    except ImportError as error:  # pragma: no cover - environment guard
        raise RuntimeError("the literal census requires pylibclang; enter nix develop") from error
    return cindex, _C


class Scanner:
    """Walk one translation unit, recording literal sinks and slot flows."""

    def __init__(self, unit: Unit, repo: Path, arguments: list[str]):
        self.cindex, self.C = _cindex()
        self.ck = self.cindex.CursorKind
        self.tk = self.cindex.TypeKind
        self.unit, self.repo = unit, repo.resolve()
        self.facts = UnitFacts()
        self.texts: dict[str, list[str]] = {}
        self.raw: dict[str, str] = {}
        # (file, line) -> [(macro end column, enum)] for typed declarations.
        self.enum_storage: dict[tuple[str, int], list[tuple[int, str]]] = {}
        self.enum_names: set[str] = set()
        self.tu = self.cindex.Index.create().parse(
            str((repo / unit.source).resolve()), args=arguments,
            options=self.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )
        self.facts.errors.extend(
            str(d) for d in self.tu.diagnostics if d.severity >= self.cindex.Diagnostic.Error)
        self.wrappers = {self.ck.CXCursor_UnexposedExpr, self.ck.CXCursor_ParenExpr,
                         self.ck.CXCursor_CStyleCastExpr}

    # ---- locations -------------------------------------------------------
    def relative(self, filename: str | None) -> str | None:
        if not filename:
            return None
        try:
            return str(Path(filename).resolve().relative_to(self.repo))
        except ValueError:
            return None

    def text(self, relative: str) -> str:
        if relative not in self.raw:
            self.raw[relative] = (self.repo / relative).read_text(encoding="utf-8")
            self.texts[relative] = self.raw[relative].splitlines()
        return self.raw[relative]

    def project(self, cursor: Any) -> bool:
        location = cursor.location
        relative = self.relative(location.file.name if location.file else None)
        return relative is not None and relative.startswith(("src/", "include/"))

    def spelling(self, cursor: Any) -> tuple[str, int, int, int, str] | None:
        """Return the written token location of a literal, even through macros."""
        token = next(cursor.get_tokens(), None)
        location = spelling = None
        if token is not None:
            location, spelling = token.location, token.spelling
        else:
            pointer = self.C.clang_getToken(self.tu, cursor.extent.start)
            if pointer is not None:
                location = self.C.clang_getTokenLocation(self.tu, pointer)
                spelling = self.C.clang_getTokenSpelling(self.tu, pointer)
                location = self.cindex.SourceLocation(location) \
                    if not hasattr(location, "file") else location
        if location is None or location.file is None:
            return None
        relative = self.relative(location.file.name)
        if relative is None:
            return None
        return relative, location.line, location.column, location.offset, spelling

    def macro_at(self, cursor: Any) -> str:
        """Name the macro whose expansion produced a cursor, if any."""
        location = cursor.location
        relative = self.relative(location.file.name if location.file else None)
        if relative is None:
            return ""
        match = IDENTIFIER.match(self.text(relative), location.offset)
        return match.group(0) if match else ""

    # ---- types and slots -------------------------------------------------
    def evaluate(self, cursor: Any) -> int | None:
        result = self.C.clang_Cursor_Evaluate(cursor)
        if result is None:
            return None
        try:
            if self.C.clang_EvalResult_getKind(result) == self.C.CXEvalResultKind.CXEval_Int:
                return int(self.C.clang_EvalResult_getAsLongLong(result))
        finally:
            self.C.clang_EvalResult_dispose(result)
        return None

    def strip(self, cursor: Any) -> Any:
        while cursor.kind in self.wrappers:
            children = [child for child in cursor.get_children() if child.kind.is_expression()]
            if len(children) != 1:
                break
            cursor = children[0]
        return cursor

    def expressions(self, cursor: Any) -> list[Any]:
        return [child for child in cursor.get_children() if child.kind.is_expression()]

    def adopt(self, cursor: Any) -> Any:
        """Cursors derived from types can lose their owning translation unit."""
        if cursor is not None and getattr(cursor, "_tu", None) is None:
            cursor._tu = self.tu
        return cursor

    def record_name(self, record: Any) -> str:
        if record is None:
            return "?"
        self.adopt(record)
        spelling = record.type.spelling if record.type else record.spelling
        spelling = spelling.removeprefix("struct ").removeprefix("union ").removeprefix("const ")
        if not spelling or "unnamed" in spelling or "anonymous" in spelling:
            parent = self.adopt(record.semantic_parent)
            if parent is not None and parent.kind in {self.ck.CXCursor_StructDecl,
                                                      self.ck.CXCursor_UnionDecl}:
                for member in parent.get_children():
                    if member.kind == self.ck.CXCursor_FieldDecl \
                            and _same(self.adopt(member.type.get_declaration()), record):
                        return f"{self.record_name(parent)}.{member.spelling}"
                return self.record_name(parent)
            return "?"
        return spelling

    def width(self, typ: Any) -> tuple[str, int]:
        canonical = typ.get_canonical()
        while canonical.kind in {self.tk.CXType_ConstantArray, self.tk.CXType_IncompleteArray}:
            canonical = canonical.get_array_element_type().get_canonical()
        try:
            size = canonical.get_size()
        except Exception:  # incomplete types report through the binding
            size = -1
        return typ.spelling, size

    def declare(self, key: str, cursor: Any, typ: Any, kind: str, name: str) -> str:
        if key not in self.facts.slots:
            spelling, size = self.width(typ)
            canonical = typ.get_canonical()
            while canonical.kind in {self.tk.CXType_ConstantArray,
                                     self.tk.CXType_IncompleteArray}:
                canonical = canonical.get_array_element_type().get_canonical()
            location = cursor.location
            relative = self.relative(location.file.name if location.file else None) or "<sdk>"
            self.facts.slots[key] = {
                "key": key, "kind": kind, "name": name, "type": spelling, "size": size,
                "canonical": canonical.spelling,
                "pointer": canonical.kind == self.tk.CXType_Pointer,
                "signed": canonical.kind in {self.tk.CXType_Char_S, self.tk.CXType_SChar,
                                             self.tk.CXType_Short, self.tk.CXType_Int,
                                             self.tk.CXType_Long, self.tk.CXType_LongLong},
                "file": relative, "line": location.line,
                "enum_domain": self.declared_enum(relative, location.line, location.column)
                or (spelling.removeprefix("const ") if spelling.removeprefix("const ")
                    in self.enum_names else ""),
            }
        elif not self.facts.slots[key]["name"] and name:
            self.facts.slots[key]["name"] = name
        return key

    def declared_enum(self, relative: str, line: int, column: int) -> str:
        """Enum of a typed-storage macro that directly precedes the declared name.

        A declaration line can also name a function or another parameter, so
        only a macro followed by whitespace up to the name types the slot.
        """
        macros = self.enum_storage.get((relative, line))
        if not macros:
            return ""
        self.text(relative)
        source = self.texts[relative][line - 1] if line - 1 < len(self.texts[relative]) else ""
        preceding = [(end, name) for end, name in macros if end <= column]
        if not preceding:
            return ""
        end, name = max(preceding)
        return name if not source[end - 1:column - 1].strip() else ""

    def parameter_key(self, function: Any, index: int) -> str:
        return f"{function.spelling}:arg{index}"

    def slot(self, cursor: Any, function: str) -> str | None:
        """Name the storage an lvalue/rvalue expression reads, through casts.

        An explicit KF_ENUM_ENCODE boundary reads the enum's integer storage:
        it names a raw view of the slot that does not carry the enum domain.
        """
        key = self.storage_slot(cursor, function)
        if key is None or not self.encoded(cursor):
            return key
        raw = f"{key}#raw"
        base = raw.lstrip("*")
        if base not in self.facts.slots:
            info = self.facts.slots.get(key.lstrip("*"), {
                "type": "", "size": 0, "pointer": False, "name": key, "signed": False,
                "file": "", "line": 0, "canonical": ""})
            self.facts.slots[base] = {**info, "key": base, "kind": "derived",
                                      "enum_domain": ""}
        return raw

    def encoded(self, cursor: Any) -> bool:
        """True when a wrapper on the way to the read value is KF_ENUM_ENCODE."""
        while cursor.kind in self.wrappers:
            if self.macro_at(cursor) == "KF_ENUM_ENCODE":
                return True
            children = [child for child in cursor.get_children() if child.kind.is_expression()]
            if len(children) != 1:
                break
            cursor = children[0]
        return False

    def storage_slot(self, cursor: Any, function: str) -> str | None:
        cursor = self.strip(cursor)
        kind = cursor.kind
        if kind == self.ck.CXCursor_MemberRefExpr:
            declaration = self.adopt(cursor.referenced)
            if declaration is None:
                return None
            owner = self.record_name(self.adopt(declaration.semantic_parent))
            key = f"{owner}.{declaration.spelling}"
            return self.declare(key, declaration, declaration.type, "field", declaration.spelling)
        if kind == self.ck.CXCursor_DeclRefExpr:
            declaration = self.adopt(cursor.referenced)
            if declaration is None:
                return None
            if declaration.kind == self.ck.CXCursor_ParmDecl:
                owner = self.adopt(declaration.semantic_parent)
                if owner is None or owner.kind != self.ck.CXCursor_FunctionDecl:
                    return None
                for index, parameter in enumerate(owner.get_arguments()):
                    if _same(parameter, declaration):
                        return self.declare(self.parameter_key(owner, index), declaration,
                                            declaration.type, "parameter", declaration.spelling)
                return None
            if declaration.kind == self.ck.CXCursor_VarDecl:
                parent = self.adopt(declaration.semantic_parent)
                local = parent is not None and parent.kind == self.ck.CXCursor_FunctionDecl
                key = f"{parent.spelling}::{declaration.spelling}" if local \
                    else declaration.spelling
                return self.declare(key, declaration, declaration.type,
                                    "local" if local else "global", declaration.spelling)
            return None
        if kind == self.ck.CXCursor_ArraySubscriptExpr:
            children = self.expressions(cursor)
            base = self.slot(children[0], function) if children else None
            if base is None:
                return None
            info = self.facts.slots.get(base)
            return f"*{base}" if info and info["pointer"] else base
        if kind == self.ck.CXCursor_UnaryOperator \
                and self.unary(cursor) == "*":
            children = self.expressions(cursor)
            base = self.slot(children[0], function) if children else None
            return f"*{base}" if base else None
        if kind == self.ck.CXCursor_CallExpr:
            callee = self.adopt(cursor.referenced)
            if callee is not None and callee.kind == self.ck.CXCursor_FunctionDecl:
                return self.declare(f"{callee.spelling}:return", callee, callee.result_type,
                                    "return", callee.spelling)
        if kind == self.ck.CXCursor_BinaryOperator and self.binary(cursor) in {"And", "Shr"}:
            # A masked or shifted field is a packed sub-domain of its storage.
            parts = self.expressions(cursor)
            if len(parts) == 2:
                for operand, other in ((parts[0], parts[1]), (parts[1], parts[0])):
                    value = self.evaluate(other)
                    base = self.slot(operand, function) if value is not None else None
                    if base is None:
                        continue
                    suffix = f"&{value:#x}" if self.binary(cursor) == "And" else f">>{value}"
                    key = base + suffix
                    if key not in self.facts.slots:
                        # A masked or shifted read is an integer view of the
                        # storage, not a member of the stored enum.
                        self.facts.slots[key] = {**self.facts.slots.get(
                            base.lstrip("*"), {"type": "", "size": 0, "pointer": False,
                                               "name": base, "signed": False, "file": "",
                                               "line": 0, "canonical": ""}),
                            "key": key, "kind": "derived", "enum_domain": ""}
                    return key
        return None

    def unary(self, cursor: Any) -> str:
        name = str(self.C.clang_getCursorUnaryOperatorKind(cursor)).rsplit("_", 1)[-1]
        return {"Deref": "*", "AddrOf": "&", "Minus": "-", "Plus": "+", "Not": "~",
                "LNot": "!", "PostInc": "++", "PreInc": "++", "PostDec": "--",
                "PreDec": "--"}.get(name, name)

    def binary(self, cursor: Any) -> str:
        return str(self.C.clang_getCursorBinaryOperatorKind(cursor)).rsplit("_", 1)[-1]

    def slot_sink(self, kind: str, slot: str | None, operator: str = "") -> Sink:
        if slot is None:
            return Sink(kind, "", "", 0, operator)
        info = self.facts.slots.get(slot.lstrip("*"), {})
        return Sink(kind, slot, info.get("type", ""), info.get("size", 0), operator)

    def edge(self, left: str | None, right: str | None, kind: str, cursor: Any) -> None:
        if not left or not right or left == right:
            return
        if left.startswith("__builtin") or right.startswith("__builtin"):
            return
        location = cursor.location
        relative = self.relative(location.file.name if location.file else None) or ""
        a, b = sorted((left, right))
        self.facts.edges.add(Edge(a, b, kind, relative, location.line))

    # ---- recording -------------------------------------------------------
    def record(self, cursor: Any, sink: Sink, function: str, negate: bool) -> None:
        written = self.spelling(cursor)
        if written is None and cursor.kind != self.ck.CXCursor_DeclRefExpr:
            # Cross-file macro extents cannot be tokenized; name the expansion.
            name = self.macro_at(cursor)
            if not name or not self.macro_literal(name):
                return
            relative, line, column, offset = self.expansion(cursor)
            written = relative, line, column, offset, name
        if written is None:
            return
        relative, line, column, offset, spelling = written
        if not relative.startswith(("src/", "include/")):
            return
        origin = "character" if cursor.kind == self.ck.CXCursor_CharacterLiteral else "literal"
        constant = constant_file = spelled = ""
        if cursor.kind == self.ck.CXCursor_DeclRefExpr:
            declaration = self.adopt(cursor.referenced)
            origin, constant = "enum-constant", declaration.spelling
            location = declaration.location
            constant_file = self.relative(location.file.name if location.file else None) or ""
            value = declaration.enum_value
            relative, line, column, offset = self.expansion(cursor)
            spelling = constant
        else:
            value = self.evaluate(cursor)
            if value is None:
                return
            pattern = CHARACTER if origin == "character" else NUMBER
            if spelling is not None and IDENTIFIER.fullmatch(spelling) \
                    and self.macro_literal(spelling):
                # An object-like macro whose body is the literal.
                definition = self.macros()[spelling]
                relative, line, column, offset = self.expansion(cursor)
                if not relative.startswith(("src/", "include/")):
                    return
                self.facts.sites.append((Site(
                    relative, line, column, offset, spelling, -value if negate else value,
                    "macro-constant", spelling, definition[0], sink, function,
                    self.line_text(relative, line), self.macro_spelled(spelling)),
                    self.unit.image, self.unit.unit))
                return
            if spelling is None or not pattern.fullmatch(spelling):
                return
            expansion = self.expansion(cursor)
            if (expansion[0], expansion[3]) != (relative, offset) \
                    and self.in_define(relative, line):
                # A macro body supplies the value: the expansion names it.
                origin, constant, constant_file = "macro-constant", self.macro_at(cursor), relative
                spelled = f"{relative}:{offset}"
                relative, line, column, offset = expansion
        if not relative.startswith(("src/", "include/")):
            return
        if negate:
            value = -value
        self.text(relative)
        lines = self.texts[relative]
        text = lines[line - 1].strip() if 0 < line <= len(lines) else ""
        self.facts.sites.append((Site(relative, line, column, offset, spelling, value, origin,
                                      constant, constant_file, sink, function, text, spelled),
                                 self.unit.image, self.unit.unit))

    def macros(self) -> dict[str, tuple[str, list[str], list[int]]]:
        if not hasattr(self, "_macros"):
            self._macros: dict[str, tuple[str, list[str], list[int]]] = {}
            for node in self.tu.cursor.get_children():
                if node.kind == self.ck.CXCursor_MacroDefinition:
                    relative = self.relative(node.location.file.name
                                             if node.location.file else None) or ""
                    tokens = list(node.get_tokens())[1:]
                    self._macros[node.spelling] = (
                        relative, [token.spelling for token in tokens],
                        [token.location.offset for token in tokens])
        return self._macros

    def macro_spelled(self, name: str, depth: int = 0) -> str:
        relative, tokens, offsets = self.macros().get(name, ("", [], []))
        for token, offset in zip(tokens, offsets):
            if NUMBER.fullmatch(token) or CHARACTER.fullmatch(token):
                return f"{relative}:{offset}"
            if IDENTIFIER.fullmatch(token) and depth < 8:
                return self.macro_spelled(token, depth + 1)
        return ""

    def macro_literal(self, name: str, depth: int = 0) -> bool:
        """True when an object-like macro reduces to one written literal."""
        definition = self.macros().get(name)
        if definition is None or depth > 8:
            return False
        tokens = [token for token in definition[1] if token not in {"(", ")"}]
        if tokens and tokens[0] == "-":
            tokens = tokens[1:]
        if len(tokens) != 1:
            return False
        return bool(NUMBER.fullmatch(tokens[0]) or CHARACTER.fullmatch(tokens[0])
                    or self.macro_literal(tokens[0], depth + 1))

    def line_text(self, relative: str, line: int) -> str:
        self.text(relative)
        lines = self.texts[relative]
        return lines[line - 1].strip() if 0 < line <= len(lines) else ""

    def in_define(self, relative: str, line: int) -> bool:
        self.text(relative)
        lines = self.texts[relative]
        index = line - 1
        while index > 0 and lines[index - 1].rstrip().endswith("\\"):
            index -= 1
        return 0 <= index < len(lines) and lines[index].lstrip().startswith("#")

    def expansion(self, cursor: Any) -> tuple[str, int, int, int]:
        location = cursor.location
        relative = self.relative(location.file.name if location.file else None) or ""
        return relative, location.line, location.column, location.offset

    # ---- walking ---------------------------------------------------------
    def run(self) -> UnitFacts:
        for node in self.tu.cursor.get_children():
            if node.kind == self.ck.CXCursor_MacroExpansion \
                    and node.spelling in {"KF_ENUM_STORAGE", "KF_ENUM_PARAM", "KF_ENUM_PROMOTED"}:
                tokens = [token.spelling for token in node.get_tokens()]
                relative = self.relative(node.location.file.name if node.location.file else None)
                if relative and len(tokens) > 2:
                    self.enum_storage.setdefault((relative, node.location.line), []).append(
                        (node.extent.end.column, tokens[2]))
            if node.kind == self.ck.CXCursor_MacroExpansion and node.spelling == "KF_ENUM_BEGIN":
                tokens = [token.spelling for token in node.get_tokens()]
                if len(tokens) > 2:
                    self.enum_names.add(tokens[2])
        for child in self.tu.cursor.get_children():
            if child.location.is_in_system_header or not self.project(child):
                continue
            self.walk(child, Sink("statement"), "", None)
        return self.facts

    def walk(self, cursor: Any, sink: Sink, function: str, switch: str | None,
             negate: bool = False, case_values: frozenset[int] = frozenset()) -> None:
        ck = self.ck
        kind = cursor.kind
        if kind in {ck.CXCursor_IntegerLiteral, ck.CXCursor_CharacterLiteral}:
            self.record(cursor, sink, function, negate)
            return
        if kind == ck.CXCursor_DeclRefExpr:
            declaration = self.adopt(cursor.referenced)
            if declaration is not None and declaration.kind == ck.CXCursor_EnumConstantDecl:
                self.record(cursor, sink, function, negate)
            return
        if kind in {ck.CXCursor_MacroDefinition, ck.CXCursor_MacroExpansion,
                    ck.CXCursor_InclusionDirective, ck.CXCursor_TypeRef}:
            return
        if kind == ck.CXCursor_UnaryExpr:  # sizeof/alignof: a layout quantity
            return
        children = list(cursor.get_children())
        if kind == ck.CXCursor_FunctionDecl:
            if cursor.is_definition() \
                    and cursor.result_type.get_canonical().kind != self.tk.CXType_Void:
                self.declare(f"{cursor.spelling}:return", cursor, cursor.result_type,
                             "return", cursor.spelling)
            for child in children:
                if child.kind == ck.CXCursor_CompoundStmt:
                    self.body(child, cursor.spelling, None)
            return
        if kind == ck.CXCursor_EnumConstantDecl:
            location = cursor.location
            relative = self.relative(location.file.name if location.file else None) or ""
            self.facts.declared[cursor.spelling] = (cursor.enum_value, relative)
            for child in children:
                self.walk(child, Sink("enum-definition", cursor.spelling), function, switch)
            return
        if kind in {ck.CXCursor_FieldDecl, ck.CXCursor_TypedefDecl, ck.CXCursor_ParmDecl}:
            for child in children:
                self.layout(child, cursor.spelling, function)
            return
        if kind == ck.CXCursor_VarDecl:
            self.variable(cursor, children, function, switch)
            return
        if kind in self.wrappers:
            stripped = self.strip(cursor)
            if cursor.type.get_canonical().kind == self.tk.CXType_Pointer \
                    and stripped.kind == ck.CXCursor_IntegerLiteral:
                sink = Sink("pointer-zero", sink.target, cursor.type.spelling, 4, sink.kind)
            for child in children:
                self.walk(child, sink, function, switch, negate, case_values)
            return
        if kind == ck.CXCursor_UnaryOperator:
            operator = self.unary(cursor)
            operand = self.expressions(cursor)
            if not operand:
                return
            if operator == "-":
                self.walk(operand[0], sink, function, switch, not negate, case_values)
            elif operator == "+":
                self.walk(operand[0], sink, function, switch, negate, case_values)
            elif operator == "~":
                self.walk(operand[0], Sink("mask", sink.target, sink.target_type, sink.width, "~"),
                          function, switch)
            elif operator == "!":
                self.walk(operand[0], Sink("truth"), function, switch)
            elif operator in {"++", "--"}:
                target = self.slot(operand[0], function)
                if target:
                    self.facts.counters.add(target)
                self.walk(operand[0], Sink("lvalue"), function, switch)
            else:
                self.walk(operand[0], Sink("address"), function, switch)
            return
        if kind in {ck.CXCursor_BinaryOperator, ck.CXCursor_CompoundAssignOperator}:
            self.operator(cursor, sink, function, switch, case_values)
            return
        if kind == ck.CXCursor_ConditionalOperator:
            parts = self.expressions(cursor)
            if len(parts) == 3:
                self.walk(parts[0], Sink("truth"), function, switch)
                self.walk(parts[1], sink, function, switch, negate, case_values)
                self.walk(parts[2], sink, function, switch, negate, case_values)
            return
        if kind == ck.CXCursor_CallExpr:
            self.call(cursor, function, switch)
            return
        if kind == ck.CXCursor_ArraySubscriptExpr:
            parts = self.expressions(cursor)
            if len(parts) == 2:
                base = self.slot(parts[0], function)
                self.walk(parts[0], Sink("address"), function, switch)
                index_node = f"[{base}]" if base else None
                self.edge(self.slot(parts[1], function), index_node, "index", cursor)
                self.walk(parts[1], Sink("index", index_node or "", "", 0), function, switch)
            return
        if kind == ck.CXCursor_MemberRefExpr:
            for child in children:
                self.walk(child, Sink("address"), function, switch)
            return
        if kind == ck.CXCursor_ReturnStmt:
            target = f"{function}:return" if function else None
            for child in self.expressions(cursor):
                if target and target in self.facts.slots:
                    if self.arithmetic(child):
                        self.facts.counters.add(target)
                    self.edge(target, self.slot(child, function), "return", cursor)
                self.walk(child, self.slot_sink("return", target if target in self.facts.slots
                                                else None), function, switch)
            return
        if kind == ck.CXCursor_SwitchStmt:
            subject = self.expressions(cursor)[0] if self.expressions(cursor) else None
            target = self.slot(subject, function) if subject is not None else None
            if target is None and subject is not None:
                # An unnamed computed subject still groups its own case labels.
                target = f"switch@{function}:{cursor.location.line}"
                self.facts.slots.setdefault(target, {
                    "key": target, "kind": "switch", "name": "", "type": subject.type.spelling,
                    "size": 4, "pointer": False, "signed": True, "canonical": "",
                    "file": "", "line": cursor.location.line, "enum_domain": ""})
            for child in children:
                if _same(child, subject):
                    self.walk(child, Sink("switch-subject"), function, switch)
                elif child.kind == ck.CXCursor_CompoundStmt:
                    self.body(child, function, target or "")
                else:
                    self.walk(child, Sink("statement"), function, target or "")
            return
        if kind in {ck.CXCursor_CaseStmt, ck.CXCursor_DefaultStmt}:
            self.case(cursor, function, switch)
            return
        if kind in {ck.CXCursor_IfStmt, ck.CXCursor_WhileStmt, ck.CXCursor_DoStmt}:
            condition = len(children) - 1 if kind == ck.CXCursor_DoStmt else 0
            for index, child in enumerate(children):
                self.walk(child, Sink("truth") if index == condition else Sink("statement"),
                          function, switch, case_values=case_values)
            return
        if kind == ck.CXCursor_CompoundStmt:
            self.body(cursor, function, switch, case_values)
            return
        if kind == ck.CXCursor_InitListExpr:
            self.initializer(cursor, cursor.type, None, function, switch)
            return
        for child in children:
            self.walk(child, sink if child.kind.is_expression() else Sink("statement"),
                      function, switch, case_values=case_values)

    def layout(self, cursor: Any, name: str, function: str) -> None:
        """Array extents, bit widths and static layout checks are declaration sizes."""
        if cursor.kind in {self.ck.CXCursor_IntegerLiteral, self.ck.CXCursor_CharacterLiteral}:
            self.record(cursor, Sink("declaration", name), function, False)
            return
        if cursor.kind == self.ck.CXCursor_DeclRefExpr:
            declaration = self.adopt(cursor.referenced)
            if declaration is not None and declaration.kind == self.ck.CXCursor_EnumConstantDecl:
                self.record(cursor, Sink("declaration", name), function, False)
            return
        for child in cursor.get_children():
            self.layout(child, name, function)

    def body(self, cursor: Any, function: str, switch: str | None,
             case_values: frozenset[int] = frozenset()) -> None:
        """Walk a compound statement; case labels scope the following siblings."""
        for child in cursor.get_children():
            if child.kind in {self.ck.CXCursor_CaseStmt, self.ck.CXCursor_DefaultStmt} \
                    and switch is not None:
                case_values = self.case(child, function, switch)
                continue
            self.walk(child, Sink("statement"), function, switch, case_values=case_values)

    def case(self, cursor: Any, function: str, switch: str | None) -> frozenset[int]:
        """Record one label (nested labels chain); return the values in scope."""
        values: set[int] = set()
        while cursor.kind in {self.ck.CXCursor_CaseStmt, self.ck.CXCursor_DefaultStmt}:
            children = list(cursor.get_children())
            if cursor.kind == self.ck.CXCursor_CaseStmt and children:
                label = children[0]
                value = self.evaluate(label)
                if value is not None:
                    values.add(value)
                self.walk(label, self.slot_sink("case", switch or None), function, switch)
                children = children[1:]
            if not children:
                return frozenset(values)
            cursor = children[-1]
        self.walk(cursor, Sink("statement"), function, switch, case_values=frozenset(values))
        return frozenset(values)

    def operator(self, cursor: Any, sink: Sink, function: str, switch: str | None,
                 case_values: frozenset[int]) -> None:
        parts = self.expressions(cursor)
        if len(parts) != 2:
            return
        left, right = parts
        operator = self.binary(cursor)
        left_slot, right_slot = self.slot(left, function), self.slot(right, function)
        if operator == "Assign":
            self.walk(left, Sink("lvalue"), function, switch)
            if left_slot and self.facts.slots.get(left_slot.lstrip("*"), {}).get("pointer") \
                    and not left_slot.startswith("*"):
                self.walk(right, Sink("pointer-zero", left_slot, "", 4), function, switch)
                return
            if left_slot and self.arithmetic(right):
                self.facts.counters.add(left_slot)
            self.edge(left_slot, right_slot, "copy", cursor)
            for branch in self.branches(right):
                self.edge(left_slot, self.slot(branch, function), "copy", cursor)
                value = self.evaluate(branch)
                if value is not None and value in case_values and switch:
                    self.edge(left_slot, switch, f"case-echo={value}", cursor)
            self.walk(right, self.slot_sink("assign", left_slot), function, switch)
            return
        if operator.endswith("Assign"):
            base = operator.removesuffix("Assign")
            if left_slot and base in {"Add", "Sub", "Mul", "Div"}:
                self.facts.counters.add(left_slot)
            self.walk(left, Sink("lvalue"), function, switch)
            self.walk(right, self.slot_sink(self.operator_kind(base, right=True), left_slot,
                                            base), function, switch)
            return
        if operator in {"EQ", "NE", "LT", "GT", "LE", "GE"}:
            if operator not in {"EQ", "NE"} and left_slot and right_slot:
                # Ordering two variables compares magnitudes, not codes.
                self.facts.counters.update((left_slot, right_slot))
            self.edge(left_slot, right_slot, "compare", cursor)
            self.walk(left, self.slot_sink("compare", right_slot, operator), function, switch)
            self.walk(right, self.slot_sink("compare", left_slot, operator), function, switch)
            return
        if operator in {"LAnd", "LOr"}:
            self.walk(left, Sink("truth"), function, switch)
            self.walk(right, Sink("truth"), function, switch)
            return
        if operator == "Comma":
            self.walk(left, Sink("statement"), function, switch)
            self.walk(right, sink, function, switch, case_values=case_values)
            return
        constants = self.evaluate(left) is not None, self.evaluate(right) is not None
        if operator in {"Mul", "Div", "Rem", "Shl"} \
                or operator in {"Add", "Sub"} and not any(constants):
            self.facts.counters.update(slot for slot in (left_slot, right_slot) if slot)
        self.walk(left, self.slot_sink(self.operator_kind(operator, right=False), right_slot,
                                       operator), function, switch)
        self.walk(right, self.slot_sink(self.operator_kind(operator, right=True), left_slot,
                                        operator), function, switch)

    def arithmetic(self, cursor: Any) -> bool:
        """A value computed by arithmetic marks its destination as a quantity."""
        cursor = self.strip(cursor)
        return cursor.kind == self.ck.CXCursor_BinaryOperator \
            and self.binary(cursor) in {"Add", "Sub", "Mul", "Div", "Rem", "Shl"} \
            and self.evaluate(cursor) is None

    def operator_kind(self, operator: str, *, right: bool) -> str:
        if operator in {"And", "Or", "Xor"}:
            return "mask"
        if operator in {"Shl", "Shr"}:
            return "shift" if right else "arithmetic"
        return "arithmetic"

    def branches(self, cursor: Any) -> list[Any]:
        cursor = self.strip(cursor)
        if cursor.kind == self.ck.CXCursor_ConditionalOperator:
            parts = self.expressions(cursor)
            return self.branches(parts[1]) + self.branches(parts[2]) if len(parts) == 3 else []
        return [cursor]

    def call(self, cursor: Any, function: str, switch: str | None) -> None:
        callee = self.adopt(cursor.referenced)
        direct = callee is not None and callee.kind == self.ck.CXCursor_FunctionDecl
        arguments = list(cursor.get_arguments())
        children = self.expressions(cursor)
        if children:
            self.walk(children[0], Sink("callee"), function, switch)
        parameters = list(callee.get_arguments()) if direct else []
        unprototyped = direct and callee.type.kind == self.tk.CXType_FunctionNoProto
        for index, argument in enumerate(arguments):
            key = None
            positional = True
            if direct:
                parameter = parameters[index] if index < len(parameters) else None
                key = self.parameter_key(callee, index)
                if parameter is not None:
                    self.declare(key, parameter, parameter.type, "parameter", parameter.spelling)
                elif unprototyped:
                    # A K&R declaration (Psy-Q MEMORY.H memset) still has
                    # positional parameters; the argument supplies the type.
                    positional = False
                    self.declare(key, callee, argument.type, "parameter", f"arg{index}")
                else:  # variadic
                    positional = False
                    key = f"{callee.spelling}:vararg"
                    self.declare(key, callee, argument.type, "parameter", "...")
                if self.arithmetic(argument):
                    self.facts.counters.add(key)
                self.edge(key, self.slot(argument, function), "argument", argument)
            pointer = self.facts.slots[key]["pointer"] if key and positional else \
                argument.type.get_canonical().kind == self.tk.CXType_Pointer
            if key and pointer:
                self.walk(argument, Sink("pointer-zero", key, "", 4), function, switch)
                continue
            self.walk(argument, self.slot_sink("argument", key), function, switch)

    def variable(self, cursor: Any, children: list[Any], function: str,
                 switch: str | None) -> None:
        parent = self.adopt(cursor.semantic_parent)
        local = parent is not None and parent.kind == self.ck.CXCursor_FunctionDecl
        key = f"{parent.spelling}::{cursor.spelling}" if local else cursor.spelling
        self.declare(key, cursor, cursor.type, "local" if local else "global", cursor.spelling)
        tokens = [token.spelling for token in cursor.get_tokens()]
        expressions = [child for child in children if child.kind.is_expression()]
        initializer = expressions[-1] if expressions and "=" in tokens else None
        for child in children:
            if _same(child, initializer):
                continue
            self.layout(child, cursor.spelling, function)
        if initializer is None:
            return
        stripped = self.strip(initializer)
        if stripped.kind == self.ck.CXCursor_InitListExpr:
            self.initializer(stripped, cursor.type, key, function, switch)
            return
        if self.facts.slots[key]["pointer"]:
            self.walk(initializer, Sink("pointer-zero", key, "", 4), function, switch)
            return
        if self.arithmetic(initializer):
            self.facts.counters.add(key)
        for branch in self.branches(initializer):
            self.edge(key, self.slot(branch, function), "copy", cursor)
        self.walk(initializer, self.slot_sink("init", key), function, switch)

    def initializer(self, cursor: Any, typ: Any, key: str | None, function: str,
                    switch: str | None) -> None:
        """Map aggregate initializer leaves onto fields or the array element slot."""
        canonical = typ.get_canonical()
        elements = self.expressions(cursor)
        if canonical.kind in {self.tk.CXType_ConstantArray, self.tk.CXType_IncompleteArray}:
            element = canonical.get_array_element_type()
            for child in elements:
                self.element(child, element, key, function, switch)
            return
        if canonical.kind == self.tk.CXType_Record:
            fields = [self.adopt(field) for field in canonical.get_fields()]
            record = self.adopt(canonical.get_declaration())
            index = 0
            for child in elements:
                designators = [part for part in child.get_children()
                               if part.kind == self.ck.CXCursor_MemberRef]
                if designators:
                    target = self.adopt(designators[-1].referenced)
                    value = self.expressions(child)[-1] if self.expressions(child) else child
                    for position, candidate in enumerate(fields):
                        if _same(candidate, self.adopt(designators[0].referenced)):
                            index = position
                            break
                elif index < len(fields):
                    target, value = fields[index], child
                else:
                    self.walk(child, Sink("init"), function, switch)
                    continue
                field_key = self.declare(
                    f"{self.record_name(self.adopt(target.semantic_parent) or record)}"
                    f".{target.spelling}",
                    target, target.type, "field", target.spelling)
                self.element(value, target.type, field_key, function, switch)
                index += 1
            return
        for child in elements:
            self.element(child, typ, key, function, switch)

    def element(self, cursor: Any, typ: Any, key: str | None, function: str,
                switch: str | None) -> None:
        stripped = self.strip(cursor)
        canonical = typ.get_canonical()
        if stripped.kind == self.ck.CXCursor_InitListExpr:
            self.initializer(stripped, typ, key, function, switch)
        elif canonical.kind == self.tk.CXType_Pointer:
            self.walk(cursor, Sink("pointer-zero", key or "", typ.spelling, 4), function, switch)
        elif canonical.kind == self.tk.CXType_Record:
            self.walk(cursor, Sink("init"), function, switch)
        else:
            self.walk(cursor, self.slot_sink("init", key), function, switch)


def _scan(arguments: tuple[Unit, Path, list[str]]) -> UnitFacts:
    unit, repo, flags = arguments
    return Scanner(unit, repo, flags).run()


# ---- aggregation -----------------------------------------------------------
class Union:
    def __init__(self) -> None:
        self.parent: dict[str, str] = {}

    def find(self, key: str) -> str:
        self.parent.setdefault(key, key)
        while self.parent[key] != key:
            self.parent[key] = self.parent[self.parent[key]]
            key = self.parent[key]
        return key

    def join(self, left: str, right: str) -> None:
        a, b = self.find(left), self.find(right)
        if a != b:
            self.parent[max(a, b)] = min(a, b)


def _site_class(site: dict[str, Any], slots: dict[str, dict[str, Any]]) -> str:
    """Coarse census class for one site occurrence."""
    sink = site["sink"]
    kind = sink["kind"]
    if kind == "pointer-zero":
        return "pointer-zero"
    if kind in {"enum-definition", "declaration"}:
        return kind
    if kind == "truth":
        return "truth"
    if kind in FLOW_SINKS:
        info = slots.get(sink["target"].lstrip("*"), {})
        if not sink["target"]:
            return f"{kind}-untracked"
        if _geometry(sink["target"]):
            return "geometry"
        if info.get("enum_domain") or "Bool" in info.get("type", "") \
                or info.get("type", "") in {"b8", "b16", "b32"}:
            return "typed-" + ("boolean" if "Bool" in info.get("type", "")
                               or info.get("type", "") in {"b8", "b16", "b32"} else "enum")
        return kind
    return kind


def _geometry(target: str) -> bool:
    target = target.lstrip("*")
    if "." not in target:
        return False
    record, name = target.rsplit(".", 1)
    return record.split(".")[0] in GEOMETRY_RECORDS or (
        name in GEOMETRY_FIELDS and record.split(".")[0] in GEOMETRY_RECORDS)


def _domain_node(sink: dict[str, Any]) -> str | None:
    kind, target = sink["kind"], sink["target"]
    if not target:
        return None
    if kind in FLOW_SINKS or kind == "mask":
        return target
    if kind == "index":
        return target
    return None


def _flow_nodes(edges: Iterable[dict[str, Any]]) -> dict[str, set[str]]:
    neighbours: dict[str, set[str]] = defaultdict(set)
    for edge in edges:
        neighbours[edge["left"]].add(edge["right"])
        neighbours[edge["right"]].add(edge["left"])
    return neighbours


def build_domains(sites: list[dict[str, Any]], edges: list[dict[str, Any]],
                  slots: dict[str, dict[str, Any]], counters: set[str],
                  declared: dict[str, tuple[int, str]], *, hub_degree: int = HUB_DEGREE,
                  kf2: dict[str, Any] | None = None) -> tuple[list[dict[str, Any]], list[str]]:
    # A case label echoed into another slot once can be a coincidence; a
    # selector copied case by case repeats it for several values.
    echoes: dict[tuple[str, str], set[str]] = defaultdict(set)
    for edge in edges:
        if edge["kind"].startswith("case-echo="):
            echoes[(edge["left"], edge["right"])].add(edge["kind"])
    edges = [edge for edge in edges if not edge["kind"].startswith("case-echo=")
             or len(echoes[(edge["left"], edge["right"])]) >= 2]
    neighbours = _flow_nodes(edges)
    # Geometry and high-degree hubs would weld unrelated codes together. A
    # function local reused for several sources is a hub much sooner.
    hubs = sorted(node for node, partners in neighbours.items()
                  if len(partners) > hub_degree or _geometry(node)
                  or "::" in node and len(partners) > LOCAL_HUB_DEGREE)
    hub_set = set(hubs)
    union = Union()
    for edge in edges:
        # Quantities (arithmetic-written slots) keep their literals but do not
        # weld the codes they are compared with or copied from into one domain.
        if edge["left"] in hub_set or edge["right"] in hub_set \
                or edge["left"] in counters or edge["right"] in counters:
            continue
        union.join(edge["left"], edge["right"])
    members: dict[str, set[str]] = defaultdict(set)
    domain_sites: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for site in sites:
        node = _domain_node(site["sink"])
        if node is None or _geometry(node) or site["class"] in {"pointer-zero"}:
            continue
        root = union.find(node)
        members[root].add(node)
        domain_sites[root].append(site)
    for node in list(union.parent):
        root = union.find(node)
        if root in domain_sites:
            members[root].add(node)
    domains = []
    domain_edges: dict[str, list[str]] = defaultdict(list)
    for edge in edges:
        if edge["left"] in hub_set or edge["right"] in hub_set \
                or edge["left"] in counters or edge["right"] in counters:
            continue
        domain_edges[union.find(edge["left"])].append(
            f"{edge['kind']}:{edge['left']}~{edge['right']}@{edge['file']}:{edge['line']}")
    for root, rows in domain_sites.items():
        values = Counter(row["value"] for row in rows if row["origin"] in WRITTEN)
        constants = sorted({(row["constant"], row["value"], row["constant_file"])
                            for row in rows if row["origin"] not in WRITTEN})
        sinks = Counter(row["sink"]["kind"] for row in rows)
        member_rows = []
        for member in sorted(members[root]):
            info = slots.get(member.lstrip("*").strip("[]").lstrip("*"), {})
            member_rows.append({
                "slot": member, "kind": info.get("kind", "index" if member.startswith("[")
                                                 else "pointee"),
                "type": info.get("type", ""), "size": info.get("size", 0),
                "signed": info.get("signed", False), "enum_domain": info.get("enum_domain", ""),
                "counter": member in counters,
                "name": info.get("name", ""),
            })
        literal_values = sorted(values)
        all_values = sorted(set(literal_values) | {value for _, value, _ in constants})
        covered = {value for _, value, _ in constants}
        prefix = _common_prefix([name for name, _, _ in constants])
        siblings = sorted((name, value, file) for name, (value, file) in declared.items()
                          if prefix and name.startswith(prefix) and value in values
                          and (name, value, file) not in constants)
        verdict = _verdict(literal_values, sinks, member_rows, constants)
        domain = {
            "id": "",
            "root": root,
            "verdict": verdict,
            "members": member_rows,
            "values": [{"value": value, "hex": hex(value), "count": count,
                        "covered": value in covered}
                       for value, count in sorted(values.items())],
            "all_values": all_values,
            "sinks": dict(sorted(sinks.items())),
            "literal_sites": sum(values.values()),
            "constant_sites": sum(1 for row in rows if row["origin"] not in WRITTEN),
            "constants_used": [{"name": name, "value": value, "file": file}
                               for name, value, file in constants],
            "constant_prefix": prefix,
            "uncovered_values_with_prefix_constants": [
                {"name": name, "value": value, "file": file} for name, value, file in siblings],
            "functions": sorted({row["function"] for row in rows if row["function"]}),
            "files": sorted({row["file"] for row in rows}),
            "sites": [f"{row['file']}:{row['line']}" for row in rows
                      if row["origin"] in WRITTEN],
            "edges": domain_edges.get(root, []),
        }
        if kf2 is not None:
            domain["kf2"] = kf2_counterparts(domain, kf2)
        domains.append(domain)
    domains.sort(key=lambda row: (-row["literal_sites"], row["root"]))
    for index, domain in enumerate(domains, 1):
        domain["id"] = f"D{index:04d}"
    return domains, hubs


def _common_prefix(names: list[str]) -> str:
    if len(names) < 1:
        return ""
    prefix = os.path.commonprefix(names)
    if "_" not in prefix:
        return ""
    prefix = prefix[:prefix.rfind("_") + 1]
    return prefix if prefix.count("_") >= 3 else ""


def _verdict(values: list[int], sinks: Counter, members: list[dict[str, Any]],
             constants: list[tuple[str, int, str]]) -> str:
    distinct = set(values) | {value for _, value, _ in constants}
    arithmetic = sinks.get("arithmetic", 0) + sinks.get("shift", 0)
    coded = sinks.get("case", 0) + sinks.get("compare", 0)
    total = sum(sinks.values())
    if any(member["enum_domain"] for member in members):
        return "typed-enum"
    if total and sinks.get("index", 0) == total:
        return "array-index"
    if total and sinks.get("init", 0) >= 0.8 * total and not sinks.get("case", 0):
        return "data-table"
    if distinct <= {0, 1}:
        return "boolean-candidate"
    if sinks.get("mask", 0) > sinks.get("assign", 0) + coded:
        return "flags-candidate"
    if any(member["counter"] for member in members) or arithmetic > coded:
        return "quantity"
    if sinks.get("case", 0) or (coded and len(distinct) >= 2) or len(distinct) >= 3:
        return "enum-candidate"
    if len(distinct) == 1:
        return "singleton"
    return "review"


# ---- KF2 counterparts --------------------------------------------------------
ENUM_BLOCK = re.compile(r"KF_ENUM_BEGIN\(\s*(\w+)\s*,\s*(\w+)\s*\)(.*?)KF_ENUM_END\(\s*\1\s*\)",
                        re.S)
ENUM_MEMBER = re.compile(r"\b([A-Z][A-Z0-9_]+)\s*=\s*([^,\n]+)")
STORAGE_FIELD = re.compile(
    r"(?:KF_ENUM_STORAGE|KF_ENUM_PARAM|KF_ENUM_PROMOTED)"
    r"\(\s*(\w+)\s*(?:,\s*\w+\s*)?\)\s*\**\s*(\w+)")


def _strip_comments(text: str) -> str:
    return re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)


def _typed_name(declaration: str, names: set[str]) -> tuple[str, str] | None:
    """Return (enum, declarator) when a field/parameter declaration uses an enum."""
    storage = STORAGE_FIELD.search(declaration)
    if storage and storage.group(1) in names:
        return storage.group(1), storage.group(2)
    words = IDENTIFIER.findall(declaration)
    for index, word in enumerate(words[:-1]):
        if word in names:
            return word, words[-1]
    return None


def load_kf2(root: Path) -> dict[str, Any]:
    """Read KF2 scoped enums and the fields/parameters typed by them (regex, no Clang).

    Qualified record fields (Record.field) and function parameters (fn:argN)
    are strong counterparts; a bare field/parameter name is only a weak one.
    """
    enums: dict[str, dict[str, Any]] = {}
    files = sorted(list((root / "include").rglob("*.h")) + list((root / "src").rglob("*.c")))
    texts = {path: _strip_comments(path.read_text(encoding="utf-8", errors="replace"))
             for path in files}
    for path, text in texts.items():
        for match in ENUM_BLOCK.finditer(text):
            members = {}
            for name, value in ENUM_MEMBER.findall(match.group(3)):
                value = value.strip().rstrip(",")
                try:
                    members[name] = int(value.rstrip("uUlL"), 0)
                except ValueError:
                    if value in members:
                        members[name] = members[value]
            enums[match.group(1)] = {"storage": match.group(2), "members": members,
                                     "file": str(path.relative_to(root))}
    names = set(enums)
    qualified: dict[str, set[str]] = defaultdict(set)
    bare: dict[str, set[str]] = defaultdict(set)
    for text in texts.values():
        for match in re.finditer(r"\b(?:struct|union)\s+(\w+)\s*\{", text):
            depth, index = 1, match.end()
            while depth and index < len(text):
                depth += {"{": 1, "}": -1}.get(text[index], 0)
                index += 1
            body = text[match.end():index - 1]
            flat = re.sub(r"\{[^{}]*\}", " ", body)
            for declaration in flat.split(";"):
                typed = _typed_name(declaration, names)
                if typed:
                    qualified[f"{match.group(1)}.{typed[1]}"].add(typed[0])
                    bare[typed[1]].add(typed[0])
        for match in re.finditer(r"\b(\w+)\s*\(([^;{}()]*(?:\([^()]*\)[^;{}()]*)*)\)\s*[;{]",
                                 text):
            for position, declaration in enumerate(match.group(2).split(",")):
                typed = _typed_name(declaration, names)
                if typed:
                    qualified[f"{match.group(1)}:arg{position}"].add(typed[0])
                    bare[typed[1]].add(typed[0])
    return {"enums": enums,
            "qualified": {key: sorted(value) for key, value in qualified.items()},
            "typed": {key: sorted(value) for key, value in bare.items()}}


def kf2_counterparts(domain: dict[str, Any], kf2: dict[str, Any]) -> list[dict[str, Any]]:
    candidates: dict[str, set[str]] = defaultdict(set)
    weight: Counter = Counter()
    for member in domain["members"]:
        slot = member["slot"].lstrip("*").strip("[]")
        for enum in kf2["qualified"].get(slot, ()):
            candidates[enum].add(f"slot:{slot}")
            weight[enum] += 4
        names = {slot.rsplit(".", 1)[-1].rsplit("::", 1)[-1], member.get("name", "")}
        for name in names - {""}:
            for enum in kf2["typed"].get(name, ()):
                candidates[enum].add(f"name:{name}")
                weight[enum] += 1
    for constant in domain["constants_used"]:
        for enum, info in kf2["enums"].items():
            if constant["name"] in info["members"]:
                candidates[enum].add(f"constant:{constant['name']}")
                weight[enum] += 2
    values = set(domain["all_values"])
    rows = []
    for enum, reasons in candidates.items():
        info = kf2["enums"][enum]
        kf2_values = set(info["members"].values())
        overlap = len(values & kf2_values)
        rows.append({"enum": enum, "file": info["file"], "storage": info["storage"],
                     "reasons": sorted(reasons), "score": weight[enum],
                     "value_overlap": overlap, "values": len(values),
                     "kf2_values": len(kf2_values)})
    rows.sort(key=lambda row: (-row["score"], -row["value_overlap"], row["enum"]))
    return rows[:5]


# ---- written-text reconciliation ------------------------------------------------
TEXT_NUMBER = re.compile(r"(?<![\w.])(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*(?![\w.])")
CLAIM = re.compile(r"\b(?:ADDRESS|ADDRESS_AT|DATA|RODATA)\s*\(")


def text_literals(repo: Path, prefixes: tuple[str, ...] = ("src/", "include/")):
    """Yield (file, offset, line, spelling, text) for numeric tokens outside comments/strings."""
    from scripts.kf.clean_lexer import tokens

    for root in prefixes:
        for path in sorted((repo / root).rglob("*")):
            if path.suffix not in {".c", ".h", ".inc"} or not path.is_file():
                continue
            text = path.read_text(encoding="utf-8")
            lines = text.splitlines()
            masked = list(text)
            offset = 0
            for kind, spelling in tokens(text):
                if kind in {"comment", "literal"}:
                    for index in range(offset, offset + len(spelling)):
                        if masked[index] != "\n":
                            masked[index] = " "
                offset += len(spelling)
            relative = str(path.relative_to(repo))
            for match in TEXT_NUMBER.finditer("".join(masked)):
                line = text.count("\n", 0, match.start()) + 1
                yield relative, match.start(), line, match.group(0), lines[line - 1].strip()


def reconcile(repo: Path, sites: list[dict[str, Any]]) -> dict[str, Any]:
    """Account for written numeric tokens that no parsed variant consumed."""
    covered = {(row["file"], row["offset"]) for row in sites if row["origin"] in WRITTEN}
    covered |= {(file, int(offset)) for file, offset in
                (row["spelled"].rsplit(":", 1) for row in sites if row["spelled"])}
    reasons: Counter = Counter()
    examples: dict[str, list[str]] = defaultdict(list)
    total = 0
    for relative, offset, line_number, _, line in text_literals(repo):
        total += 1
        if (relative, offset) in covered:
            reasons["parsed"] += 1
            continue
        if CLAIM.search(line):
            reason = "retail-claim"
        elif "sizeof" in line:
            reason = "sizeof-operand"
        elif line.startswith("#define") or line.endswith("\\"):
            reason = "macro-body-unattributed"
        elif line.startswith("#"):
            reason = "preprocessor"
        else:
            reason = "unparsed-or-inactive"
        reasons[reason] += 1
        if len(examples[reason]) < 5:
            examples[reason].append(f"{relative}:{line_number}: {line[:80]}")
    return {"written_tokens": total, "reasons": dict(sorted(reasons.items())),
            "examples": dict(examples)}


# ---- driver -----------------------------------------------------------------
def collect(*, images: tuple[str, ...] = (), names: tuple[str, ...] = (), jobs: int = 4,
            repo: Path = REPO, manifest: Manifest | None = None, sdk: Path | None = None,
            kf2_root: Path | None = None, hub_degree: int = HUB_DEGREE) -> dict[str, Any]:
    from scripts.kf.clangd import FLAGS, MODES

    repo = repo.resolve()
    if sdk is None:
        value = os.environ.get("PSYQ_INCLUDE")
        if not value:
            raise ValueError("the literal census requires PSYQ_INCLUDE; enter nix develop")
        sdk = Path(value)
    units = select_units(manifest or load_manifest(), images=images, names=names)
    work = []
    for unit in units:
        flags = [*MODES["retail"], *FLAGS, "-Wno-unknown-warning-option",
                 "-I", str(repo / "include"),
                 "-I", str(repo / "vendor/include"), "-isystem", str(sdk),
                 *(f"-D{define}" for define in unit.defines)]
        work.append((unit, repo, flags))
    if jobs <= 1 or len(work) == 1:
        results = [_scan(item) for item in work]
    else:
        with ProcessPoolExecutor(max_workers=min(jobs, len(work)),
                                 mp_context=multiprocessing.get_context("fork")) as pool:
            results = list(pool.map(_scan, work))
    errors = [error for result in results for error in result.errors]
    if errors:
        raise RuntimeError("target-C parsing failed:\n" + "\n".join(errors[:20]))
    slots: dict[str, dict[str, Any]] = {}
    counters: set[str] = set()
    declared: dict[str, tuple[int, str]] = {}
    edges: dict[tuple[str, str, str], dict[str, Any]] = {}
    occurrences: dict[tuple, dict[str, Any]] = {}
    for result in results:
        for key, info in result.slots.items():
            previous = slots.get(key)
            if previous is None or (not previous["name"] and info["name"]):
                slots[key] = info
            if previous is not None and info["enum_domain"] and not previous["enum_domain"]:
                slots[key] = info
        counters |= result.counters
        declared.update(result.declared)
        for edge in result.edges:
            edges.setdefault((edge.left, edge.right, edge.kind), asdict(edge))
        for site, image, unit_name in result.sites:
            key = (site.file, site.offset, site.origin, site.constant, site.sink)
            row = occurrences.get(key)
            if row is None:
                row = asdict(site)
                row["images"], row["units"] = [], []
                occurrences[key] = row
            if image not in row["images"]:
                row["images"].append(image)
            if unit_name not in row["units"]:
                row["units"].append(unit_name)
    sites = sorted(occurrences.values(), key=lambda row: (row["file"], row["offset"],
                                                          row["sink"]["kind"]))
    for row in sites:
        row["class"] = _site_class(row, slots)
    edge_rows = sorted(edges.values(), key=lambda row: (row["left"], row["right"], row["kind"]))
    kf2 = load_kf2(kf2_root) if kf2_root and (kf2_root / "include").is_dir() else None
    domains, hubs = build_domains(sites, edge_rows, slots, counters, declared,
                                  hub_degree=hub_degree, kf2=kf2)
    literal_sites = [row for row in sites if row["origin"] in WRITTEN]
    locations = {(row["file"], row["offset"]) for row in literal_sites}
    in_domain = {(site_file, site_line) for domain in domains for site_file, site_line in
                 (entry.rsplit(":", 1) for entry in domain["sites"])}
    text = reconcile(repo, sites) if not images and not names else {}
    return {
        "schema": 1,
        "text": text,
        "coverage": {"variants": len(units), "literal_locations": len(locations),
                     "literal_sink_rows": len(literal_sites),
                     "constant_sink_rows": len(sites) - len(literal_sites),
                     "slots": len(slots), "edges": len(edge_rows), "domains": len(domains),
                     "hubs": len(hubs), "kf2": str(kf2_root) if kf2 else "",
                     "domain_site_lines": len(in_domain)},
        "by_class": dict(sorted(Counter(row["class"] for row in literal_sites).items())),
        "by_sink": dict(sorted(Counter(row["sink"]["kind"] for row in literal_sites).items())),
        "by_verdict": dict(sorted(Counter(domain["verdict"] for domain in domains).items())),
        "hubs": hubs,
        "domains": domains,
        "edges": edge_rows,
        "slots": slots,
        "sites": sites,
    }


def default_kf2(repo: Path = REPO) -> Path | None:
    if os.environ.get("KF2_REPO"):
        return Path(os.environ["KF2_REPO"])
    for parent in repo.resolve().parents:
        candidate = parent / "kings-field-2-decomp"
        if (candidate / "include/kf").is_dir():
            return candidate
    return None


def typed_enum_literals(**selection: Any) -> list[dict[str, Any]]:
    """Written literals whose sink slot is declared with a scoped enum type."""
    report = collect(**selection)
    return [row for row in report["sites"]
            if row["origin"] in WRITTEN and row["class"] == "typed-enum"]


def add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--image", action="append", choices=tuple(IMAGES), default=[])
    parser.add_argument("--unit", action="append", default=[])
    parser.add_argument("-j", "--jobs", type=int, default=4)
    parser.add_argument("--kf2", type=Path, default=default_kf2(),
                        help="KF2 checkout used for counterpart domains (read only; "
                        "default $KF2_REPO or a sibling kings-field-2-decomp checkout)")
    parser.add_argument("--hub-degree", type=int, default=HUB_DEGREE)
    parser.add_argument("--output", type=Path, help="write the complete JSON census")
    parser.add_argument("--domains", action="store_true", help="list domains as TSV")
    parser.add_argument("--member", action="append", default=[],
                        help="only domains with a member slot containing this text")
    parser.add_argument("--value", action="append", type=lambda text: int(text, 0), default=[],
                        help="only domains that use this value")


def run(args: argparse.Namespace) -> int:
    try:
        report = collect(images=tuple(IMAGES[image] for image in args.image),
                         names=tuple(args.unit), jobs=args.jobs, kf2_root=args.kf2,
                         hub_degree=args.hub_degree)
    except (ValueError, RuntimeError, OSError) as error:
        print(f"literals: {error}", file=sys.stderr)
        return 2
    coverage = report["coverage"]
    print(f"[literals] {coverage['literal_locations']} literal locations, "
          f"{coverage['literal_sink_rows']} literal sink rows, {coverage['constant_sink_rows']} "
          f"enum-constant sink rows in {coverage['variants']} variants", file=sys.stderr)
    for name in ("by_class", "by_verdict"):
        print(f"[literals] {name}: " + ", ".join(f"{key}={value}" for key, value
                                                 in report[name].items()), file=sys.stderr)
    print(f"[literals] {coverage['domains']} candidate domains; {coverage['hubs']} hub slots",
          file=sys.stderr)
    if report["text"]:
        print(f"[literals] written numeric tokens {report['text']['written_tokens']}: "
              + ", ".join(f"{key}={value}" for key, value in report["text"]["reasons"].items()),
              file=sys.stderr)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=1) + "\n", encoding="utf-8")
        print(f"[literals] wrote {args.output}", file=sys.stderr)
    if args.domains or args.member or args.value:
        print("id\tverdict\tliteral_sites\tvalues\tconstants\tmembers\tkf2")
        for domain in report["domains"]:
            if args.member and not any(text in member["slot"] for text in args.member
                                       for member in domain["members"]):
                continue
            if args.value and not set(args.value) & set(domain["all_values"]):
                continue
            print("\t".join((
                domain["id"], domain["verdict"], str(domain["literal_sites"]),
                ",".join(row["hex"] if abs(row["value"]) > 9 else str(row["value"])
                         for row in domain["values"]),
                ",".join(row["name"] for row in domain["constants_used"]),
                ",".join(member["slot"] for member in domain["members"]),
                ",".join(row["enum"] for row in domain.get("kf2", [])[:2]),
            )))
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    add_arguments(parser)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
