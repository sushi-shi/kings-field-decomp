"""Audit integer value domains with the target-C libclang AST.

This is a source evidence census, not a proof about unreconstructed retail code.
Every integral declaration/result is retained, including rejected and unknown
cases. Image-qualified declaration identities join assignments, direct call
arguments, returns, and pointer aliases across translation units. Unknown
writers, escaped addresses, and overlapping union views remain explicit.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
from ctypes import c_int, c_longlong, c_void_p
from dataclasses import asdict, dataclass
import json
import multiprocessing
from pathlib import Path
from typing import Any

from scripts.kf.casts import (
    IMAGE_NAMES, Location, _cindex, _describe, _hashes, _location, _sdk_path,
)
from scripts.kf.check_types import select_units
from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import REPO


# Bottom means no observed producer yet. Unknown and non-Boolean are distinct:
# a file read must not look like a demonstrated assignment of 2.
ZERO, ONE, OTHER, UNKNOWN = 1, 2, 4, 8
BOOLEAN = ZERO | ONE
Expr = tuple


@dataclass(frozen=True)
class Slot:
    key: str
    image: str
    kind: str
    name: str
    owner: str
    location: Location
    declared_type: str
    scalar_type: str
    canonical_type: str
    size: int
    arrays: int
    bit_width: int
    const: bool
    definition: bool
    domain: str


@dataclass(frozen=True)
class Write:
    target: str
    expression: Expr
    location: Location
    function: str
    reason: str


@dataclass(frozen=True)
class Use:
    target: str
    kind: str
    location: Location
    function: str


@dataclass(frozen=True)
class Call:
    callee: str
    pointers: tuple[str, ...]
    location: Location
    function: str
    memory_fill: Expr | None = None


@dataclass
class Facts:
    slots: dict[str, Slot]
    writes: list[Write]
    uses: list[Use]
    pointer_targets: dict[str, set[str]]
    pointer_edges: set[tuple[str, str]]
    indirect_writes: list[Write]
    calls: list[Call]
    definitions: set[str]
    parameters: dict[str, set[str]]
    address_taken: set[str]
    overlaps: set[tuple[str, str]]
    hazards: set[tuple[str, str]]
    declarations: set[tuple[str, str]]
    headers: set[str]
    errors: list[str]
    indirect_uses: list[Use]

    @classmethod
    def empty(cls) -> Facts:
        return cls({}, [], [], {}, set(), [], [], set(), {}, set(), set(), set(), set(),
                   set(), [], [])


def _integer(typ: Any, cindex: Any) -> bool:
    return typ.get_canonical().kind in {
        cindex.TypeKind.BOOL, cindex.TypeKind.CHAR_U, cindex.TypeKind.UCHAR,
        cindex.TypeKind.CHAR_S, cindex.TypeKind.SCHAR, cindex.TypeKind.USHORT,
        cindex.TypeKind.SHORT, cindex.TypeKind.UINT, cindex.TypeKind.INT,
        cindex.TypeKind.ULONG, cindex.TypeKind.LONG, cindex.TypeKind.ULONGLONG,
        cindex.TypeKind.LONGLONG, cindex.TypeKind.ENUM,
    }


def _array_element(typ: Any, cindex: Any) -> tuple[Any, int]:
    depth = 0
    while typ.get_canonical().kind in {
        cindex.TypeKind.CONSTANTARRAY, cindex.TypeKind.INCOMPLETEARRAY,
        cindex.TypeKind.VARIABLEARRAY, cindex.TypeKind.DEPENDENTSIZEDARRAY,
    }:
        typ = typ.get_array_element_type()
        depth += 1
    return typ, depth


def _constant_api(cindex: Any):
    library = cindex.conf.lib
    library.clang_Cursor_Evaluate.argtypes = [cindex.Cursor]
    library.clang_Cursor_Evaluate.restype = c_void_p
    library.clang_EvalResult_getKind.argtypes = [c_void_p]
    library.clang_EvalResult_getKind.restype = c_int
    library.clang_EvalResult_getAsLongLong.argtypes = [c_void_p]
    library.clang_EvalResult_getAsLongLong.restype = c_longlong
    library.clang_EvalResult_dispose.argtypes = [c_void_p]
    library.clang_EvalResult_dispose.restype = None
    library.clang_getCursorUnaryOperatorKind.argtypes = [cindex.Cursor]
    library.clang_getCursorUnaryOperatorKind.restype = c_int
    library.clang_getUnaryOperatorKindSpelling.argtypes = [c_int]
    library.clang_getUnaryOperatorKindSpelling.restype = cindex._CXString
    return library


class Extractor:
    def __init__(self, unit: Unit, repo: Path, sdk: Path):
        self.unit, self.repo = unit, repo
        self.cindex = _cindex()
        self.ck = self.cindex.CursorKind
        self.library = _constant_api(self.cindex)
        self.facts = Facts.empty()
        self.function = ""
        self.function_key = ""
        self.visited_records: set[str] = set()
        self.pointer_counter = 0
        self.enum_domains: set[str] = set()
        self.domain_ranges: dict[str, list[tuple[int, int, str]]] = defaultdict(list)
        self.return_warnings: list[Location] = []
        self.tu = self.cindex.Index.create().parse(
            str(repo / unit.source),
            args=[*MODES['retail'], *FLAGS, '-I', str(repo / 'include'),
                  '-isystem', str(sdk), *(f'-D{define}' for define in unit.defines)],
            options=self.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )

    def location(self, cursor: Any) -> Location | None:
        return _location(cursor.location, self.repo)

    def project(self, cursor: Any) -> bool:
        here = self.location(cursor)
        return here is not None and here.file.startswith(('src/', 'include/'))

    def ident(self, cursor: Any) -> str:
        if cursor.kind == self.ck.PARM_DECL:
            function = cursor.semantic_parent
            for index, parameter in enumerate(function.get_arguments()):
                if parameter == cursor:
                    return f'{self.ident(function)}:parameter:{index}'
        usr = cursor.get_usr()
        if not usr:
            here = self.location(cursor)
            usr = f'{cursor.kind.name}:{here}'
        return f'{self.unit.image}:{usr}'

    def children(self, cursor: Any) -> tuple[Any, ...]:
        return tuple(cursor.get_children())

    def expressions(self, cursor: Any) -> tuple[Any, ...]:
        return tuple(child for child in cursor.get_children() if child.kind.is_expression())

    def strip(self, cursor: Any, *, casts: bool = False) -> Any:
        kinds = {self.ck.UNEXPOSED_EXPR, self.ck.PAREN_EXPR}
        if casts:
            kinds.add(self.ck.CSTYLE_CAST_EXPR)
        while cursor.kind in kinds:
            children = self.expressions(cursor)
            if len(children) != 1:
                break
            cursor = children[0]
        return cursor

    def constant(self, cursor: Any) -> int | None:
        if not _integer(cursor.type, self.cindex):
            return None
        result = self.library.clang_Cursor_Evaluate(cursor)
        if not result:
            return None
        try:
            if self.library.clang_EvalResult_getKind(result) == 1:  # CXEval_Int
                return self.library.clang_EvalResult_getAsLongLong(result)
        finally:
            self.library.clang_EvalResult_dispose(result)
        return None

    def unary(self, cursor: Any) -> str:
        kind = self.library.clang_getCursorUnaryOperatorKind(cursor)
        return self.cindex._CXString.from_result(
            self.library.clang_getUnaryOperatorKindSpelling(kind))

    def register(self, cursor: Any, *, result: bool = False,
                 pointee: bool = False) -> str | None:
        if cursor.kind not in {self.ck.FIELD_DECL, self.ck.PARM_DECL,
                               self.ck.FUNCTION_DECL, self.ck.VAR_DECL} \
                or not self.project(cursor):
            return None
        here = self.location(cursor)
        assert here is not None
        key = self.ident(cursor)
        typ = cursor.result_type if result else cursor.type
        suffix = ':return' if result else ':pointee' if pointee else ''
        if pointee:
            typ = typ.get_pointee()
        scalar, arrays = _array_element(typ, self.cindex)
        self.facts.declarations.add((key + suffix, cursor.kind.name))
        if not _integer(scalar, self.cindex):
            return None
        kind = {
            self.ck.FIELD_DECL: 'field', self.ck.PARM_DECL: 'parameter',
            self.ck.FUNCTION_DECL: 'return', self.ck.VAR_DECL: 'global',
        }.get(cursor.kind, 'other')
        parent = cursor.semantic_parent
        owner = parent.spelling if parent and parent.kind != self.ck.TRANSLATION_UNIT else ''
        if kind == 'global' and parent and parent.kind == self.ck.FUNCTION_DECL:
            kind = 'local'
        if result:
            owner = cursor.spelling
        if pointee:
            kind += '_pointee'
        desc = _describe(scalar)
        domain = 'enum' if scalar.spelling in self.enum_domains \
            or scalar.get_canonical().kind == self.cindex.TypeKind.ENUM else ''
        # Written wrappers disappear in the retail preprocessing view.
        for start, end, macro_domain in self.domain_ranges[here.file]:
            if cursor.extent.start.offset <= start and end <= here.offset:
                domain = macro_domain
        if scalar.get_canonical().kind == self.cindex.TypeKind.BOOL \
                or scalar.spelling.removeprefix('const ').startswith('KfBool'):
            domain = 'boolean'
        definition = (parent is not None and parent.is_definition()) \
            if cursor.kind == self.ck.PARM_DECL else cursor.is_definition()
        slot = Slot(key + suffix, self.unit.image, kind, cursor.spelling, owner, here,
                    typ.spelling, scalar.spelling, desc.canonical, desc.size, arrays,
                    cursor.get_bitfield_width() if cursor.kind == self.ck.FIELD_DECL
                    and cursor.is_bitfield() else -1,
                    scalar.is_const_qualified(), definition, domain)
        previous = self.facts.slots.get(slot.key)
        if previous is None or slot.definition and not previous.definition:
            self.facts.slots[slot.key] = slot
        return slot.key

    def record_fields(self, typ: Any) -> tuple[Any, ...]:
        typ, _ = _array_element(typ, self.cindex)
        if typ.get_canonical().kind != self.cindex.TypeKind.RECORD:
            return ()
        return tuple(typ.get_canonical().get_fields())

    def targets_for_type(self, typ: Any, *, depth: int = 0) -> set[str]:
        if depth > 20:
            return set()
        result: set[str] = set()
        for field in self.record_fields(typ):
            key = self.register(field)
            if key is not None:
                result.add(key)
            else:
                result.update(self.targets_for_type(field.type, depth=depth + 1))
        return result

    def targets(self, cursor: Any) -> set[str]:
        cursor = self.strip(cursor, casts=True)
        if cursor.kind in {self.ck.DECL_REF_EXPR, self.ck.MEMBER_REF_EXPR,
                           self.ck.MEMBER_REF}:
            referenced = cursor.referenced
            if referenced:
                key = self.register(referenced)
                return {key} if key else self.targets_for_type(referenced.type)
        if cursor.kind == self.ck.ARRAY_SUBSCRIPT_EXPR:
            base = self.strip(self.expressions(cursor)[0], casts=True)
            if base.type.get_canonical().kind in {
                self.cindex.TypeKind.CONSTANTARRAY, self.cindex.TypeKind.INCOMPLETEARRAY,
            }:
                return self.targets(base)
        return self.targets_for_type(cursor.type)

    def pointer(self, cursor: Any) -> str:
        cursor = self.strip(cursor, casts=True)
        if cursor.kind in {self.ck.DECL_REF_EXPR, self.ck.MEMBER_REF_EXPR}:
            referenced = cursor.referenced
            if referenced and referenced.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                return self.ident(referenced)
        if cursor.kind == self.ck.CALL_EXPR and cursor.referenced:
            return self.ident(cursor.referenced) + ':pointer_return'
        self.pointer_counter += 1
        key = f'{self.unit.unit}:pointer-expression:{self.pointer_counter}'
        target_set = self.facts.pointer_targets.setdefault(key, set())
        if cursor.kind == self.ck.UNARY_OPERATOR and self.unary(cursor) == '&':
            operand = self.expressions(cursor)[0]
            target_set.update(self.targets(operand))
            bare = self.strip(operand, casts=True)
            if not target_set and bare.kind == self.ck.ARRAY_SUBSCRIPT_EXPR:
                self.facts.pointer_edges.add((key, self.pointer(self.expressions(bare)[0])))
        elif cursor.kind == self.ck.UNARY_OPERATOR and self.unary(cursor) in {'++', '--'}:
            self.facts.pointer_edges.add((key, self.pointer(self.expressions(cursor)[0])))
        elif cursor.kind == self.ck.CONDITIONAL_OPERATOR:
            for child in self.expressions(cursor)[1:]:
                self.facts.pointer_edges.add((key, self.pointer(child)))
        elif cursor.kind == self.ck.BINARY_OPERATOR:
            # Pointer arithmetic can reach any element in the tracked array.
            for child in self.expressions(cursor):
                if child.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                    self.facts.pointer_edges.add((key, self.pointer(child)))
        else:
            target_set.update(self.targets(cursor))
        return key

    def expr(self, cursor: Any) -> Expr:
        value = self.constant(cursor)
        if value is not None:
            return ('constant', value)
        if cursor.kind == self.ck.UNEXPOSED_EXPR:
            children = self.expressions(cursor)
            if len(children) == 1:
                return self.expr(children[0])
        cursor = self.strip(cursor)
        children = self.expressions(cursor)
        if cursor.kind in {self.ck.DECL_REF_EXPR, self.ck.MEMBER_REF_EXPR}:
            referenced = cursor.referenced
            key = self.register(referenced) if referenced else None
            return ('slot', key) if key else ('unknown', 'non-project-value')
        if cursor.kind == self.ck.CALL_EXPR:
            referenced = cursor.referenced
            key = self.register(referenced, result=True) if referenced \
                and referenced.kind == self.ck.FUNCTION_DECL else None
            return ('slot', key) if key else ('unknown', 'external-or-indirect-return')
        if cursor.kind == self.ck.ARRAY_SUBSCRIPT_EXPR:
            keys = self.targets(cursor)
            return ('union', *(('slot', key) for key in sorted(keys))) if keys \
                else ('deref', self.pointer(children[0]))
        if cursor.kind == self.ck.CSTYLE_CAST_EXPR and children:
            desc = _describe(cursor.type)
            return ('cast', desc.size, desc.kind, self.expr(children[-1]))
        if cursor.kind == self.ck.UNARY_OPERATOR and children:
            operator = self.unary(cursor)
            if operator == '*':
                return ('deref', self.pointer(children[0]))
            return ('unary', operator, self.expr(children[0]))
        if cursor.kind in {self.ck.BINARY_OPERATOR, self.ck.COMPOUND_ASSIGNMENT_OPERATOR}:
            if len(children) == 2:
                return ('binary', cursor.binary_operator.name,
                        self.expr(children[0]), self.expr(children[1]))
        if cursor.kind == self.ck.CONDITIONAL_OPERATOR and len(children) == 3:
            return ('conditional', *(self.expr(child) for child in children))
        return ('unknown', cursor.kind.name.lower())

    def write(self, keys: set[str], expression: Expr, cursor: Any, reason: str) -> None:
        here = self.location(cursor)
        if here is None:
            return
        for key in keys:
            self.facts.writes.append(Write(key, expression, here, self.function, reason))

    def assign(self, lhs: Any, rhs: Any, cursor: Any, *, operator: str = 'Assign') -> None:
        bare = self.strip(lhs, casts=True)
        if lhs.type.get_canonical().kind == self.cindex.TypeKind.POINTER \
                and bare.referenced:
            self.facts.pointer_edges.add((self.ident(bare.referenced), self.pointer(rhs)))
            return
        keys = self.targets(lhs)
        value = self.expr(rhs)
        if operator != 'Assign':
            value = ('binary', operator.removesuffix('Assign'), self.expr(lhs), value)
        if self.record_fields(lhs.type):
            # Field identities are type-wide, so a typed same-record copy adds
            # no new values. Unknown aggregate providers still contaminate it.
            if not self.targets(rhs):
                self.write(keys, ('unknown', 'aggregate-source'), cursor, 'aggregate-copy')
            return
        if keys:
            self.write(keys, value, cursor, 'assignment')
        elif bare.kind == self.ck.UNARY_OPERATOR and self.unary(bare) == '*':
            here = self.location(cursor)
            if here:
                self.facts.indirect_writes.append(Write(
                    self.pointer(self.expressions(bare)[0]), value, here, self.function,
                    'indirect-assignment'))
        elif bare.kind == self.ck.ARRAY_SUBSCRIPT_EXPR:
            here = self.location(cursor)
            if here:
                self.facts.indirect_writes.append(Write(
                    self.pointer(self.expressions(bare)[0]), value, here, self.function,
                    'indexed-pointer-assignment'))

    def initialize(self, declaration: Any, initializer: Any | None, *, implicit: bool = False,
                   typ: Any | None = None, keys: set[str] | None = None) -> None:
        typ = typ or declaration.type
        key = self.register(declaration)
        keys = keys if keys is not None else ({key} if key else self.targets_for_type(typ))
        if initializer is None:
            if implicit:
                self.write(keys, ('constant', 0), declaration, 'implicit-zero-initialization')
            return
        node = self.strip(initializer)
        if typ.get_canonical().kind == self.cindex.TypeKind.POINTER:
            self.facts.pointer_edges.add((self.ident(declaration), self.pointer(node)))
            return
        if node.kind != self.ck.INIT_LIST_EXPR:
            if self.record_fields(typ):
                if not self.targets(node):
                    self.write(keys, ('unknown', 'aggregate-initializer'), node, 'initialization')
            else:
                self.write(keys, self.expr(node), node, 'initialization')
            return
        fields = self.record_fields(typ)
        children = self.expressions(node)
        scalar, arrays = _array_element(typ, self.cindex)
        if arrays:
            if _integer(scalar, self.cindex):
                count = 0
                def leaves(value: Any) -> None:
                    nonlocal count
                    if value.kind == self.ck.INIT_LIST_EXPR:
                        for child in self.expressions(value):
                            leaves(child)
                    else:
                        count += 1
                        self.write(keys, self.expr(value), value, 'array-initialization')
                for child in children:
                    leaves(child)
                # Flattened scalar initializers also cover brace elision.
                extent = typ.get_size() // scalar.get_size()
                if count < extent:
                    self.write(keys, ('constant', 0), node, 'array-zero-fill')
            else:
                for child in children:
                    self.initialize(declaration, child, typ=scalar, keys=keys)
            return
        if not fields:
            for child in children:
                self.write(keys, self.expr(child), child, 'initialization')
            return
        assigned: set[str] = set()
        index = 0
        for child in children:
            components = self.children(child)
            designators = [part for part in components if part.kind == self.ck.MEMBER_REF]
            if designators:
                field = designators[-1].referenced
                expression = self.expressions(child)[-1]
                for candidate_index, candidate in enumerate(fields):
                    if candidate == designators[0].referenced:
                        index = candidate_index
                        break
            elif index < len(fields):
                field, expression = fields[index], child
            else:
                self.write(keys, ('unknown', 'unmapped-aggregate-initializer'), child,
                           'initialization')
                continue
            if field:
                assigned.add(self.ident(field))
                self.initialize(field, expression)
            index += 1
        for field in fields:
            if self.ident(field) not in assigned:
                self.initialize(field, None, implicit=True)

    def use(self, keys: set[str], kind: str, cursor: Any) -> None:
        here = self.location(cursor)
        if here:
            self.facts.uses.extend(Use(key, kind, here, self.function) for key in keys)

    def scan_address(self, cursor: Any) -> None:
        """Visit index/call side effects without reading every field of a record."""
        cursor = self.strip(cursor, casts=True)
        children = self.expressions(cursor)
        if cursor.kind == self.ck.ARRAY_SUBSCRIPT_EXPR and len(children) == 2:
            self.scan_address(children[0])
            self.scan(children[1], 'index')
        elif cursor.kind in {self.ck.MEMBER_REF_EXPR, self.ck.UNARY_OPERATOR}:
            for child in children:
                self.scan_address(child)
        elif cursor.kind == self.ck.BINARY_OPERATOR:
            for child in children:
                if _integer(child.type, self.cindex):
                    self.scan(child, 'index')
                else:
                    self.scan_address(child)
        elif cursor.kind == self.ck.CALL_EXPR:
            self.call(cursor)

    def read(self, cursor: Any, usage: str) -> None:
        scalar, _ = _array_element(cursor.type, self.cindex)
        if not _integer(scalar, self.cindex):
            self.scan_address(cursor)
            return
        keys = self.targets(cursor)
        self.use(keys, usage, cursor)
        if not keys:
            children = self.expressions(cursor)
            if cursor.kind in {self.ck.ARRAY_SUBSCRIPT_EXPR, self.ck.UNARY_OPERATOR} and children:
                here = self.location(cursor)
                if here:
                    self.facts.indirect_uses.append(Use(
                        self.pointer(children[0]), usage, here, self.function))
        self.scan_address(cursor)

    def call(self, cursor: Any) -> None:
        callee = cursor.referenced
        direct = callee is not None and callee.kind == self.ck.FUNCTION_DECL
        callee_key = self.ident(callee) if direct else ''
        arguments = tuple(cursor.get_arguments())
        parameters = tuple(callee.get_arguments()) if direct else ()
        mutable_pointers: list[str] = []
        for index, argument in enumerate(arguments):
            if index < len(parameters):
                parameter = parameters[index]
                key = self.register(parameter)
                if key:
                    self.write({key}, self.expr(argument), argument, 'call-argument')
            else:
                parameter = None
            if argument.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                pointer = self.pointer(argument)
                if parameter:
                    self.facts.pointer_edges.add((self.ident(parameter), pointer))
                formal_type = parameter.type if parameter else argument.type
                if not formal_type.get_pointee().is_const_qualified():
                    mutable_pointers.append(pointer)
            self.scan(argument, 'argument')
        here = self.location(cursor)
        if here:
            fill = None
            if direct and callee.spelling == 'memset' and len(arguments) == 3:
                destination = self.strip(arguments[0], casts=True)
                if destination.kind == self.ck.UNARY_OPERATOR and self.unary(destination) == '&':
                    extent = self.expressions(destination)[0].type.get_size()
                elif destination.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                    extent = destination.type.get_pointee().get_size()
                else:
                    extent = destination.type.get_size()
                count = self.constant(arguments[2])
                if extent > 0 and count == extent:
                    fill = self.expr(arguments[1])
            self.facts.calls.append(Call(callee_key, tuple(mutable_pointers), here,
                                         self.function, fill))

    def scan(self, cursor: Any, usage: str = 'value') -> None:
        kind = cursor.kind
        if kind == self.ck.TRANSLATION_UNIT:
            for child in cursor.get_children():
                self.scan(child)
            return
        if not self.project(cursor):
            return
        if kind in {self.ck.MACRO_DEFINITION, self.ck.MACRO_INSTANTIATION,
                    self.ck.INCLUSION_DIRECTIVE, self.ck.TYPE_REF}:
            return
        if kind == self.ck.FUNCTION_DECL:
            self.register(cursor, result=True)
            key = self.ident(cursor)
            for parameter in cursor.get_arguments():
                parameter_key = self.register(parameter)
                if parameter_key:
                    self.facts.parameters.setdefault(key, set()).add(parameter_key)
                elif parameter.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                    pointee = self.register(parameter, pointee=True)
                    self.facts.pointer_targets.setdefault(self.ident(parameter), set()).update(
                        {pointee} if pointee else self.targets_for_type(parameter.type.get_pointee()))
            if cursor.is_definition():
                self.facts.definitions.add(key)
                previous = self.function, self.function_key
                self.function, self.function_key = cursor.spelling, key
                for warning in self.return_warnings:
                    if warning.file == self.location(cursor).file \
                            and cursor.extent.start.offset <= warning.offset <= cursor.extent.end.offset:
                        self.write({key + ':return'}, ('unknown', 'missing-return-value'),
                                   cursor, 'missing-return-value')
                for child in cursor.get_children():
                    if child.kind == self.ck.COMPOUND_STMT:
                        self.scan(child)
                self.function, self.function_key = previous
            return
        if kind in {self.ck.STRUCT_DECL, self.ck.UNION_DECL}:
            record_key = self.ident(cursor)
            if record_key in self.visited_records:
                return
            self.visited_records.add(record_key)
            for child in cursor.get_children():
                self.scan(child)
            if kind == self.ck.UNION_DECL:
                alternatives = []
                for field in self.record_fields(cursor.type):
                    key = self.register(field)
                    alternatives.append({key} if key else self.targets_for_type(field.type))
                for index, left in enumerate(alternatives):
                    for right in alternatives[index + 1:]:
                        self.facts.overlaps.update((a, b) for a in left for b in right if a != b)
                        self.facts.overlaps.update((b, a) for a in left for b in right if a != b)
            return
        if kind == self.ck.FIELD_DECL:
            self.register(cursor)
            for child in cursor.get_children():
                if child.kind in {self.ck.STRUCT_DECL, self.ck.UNION_DECL}:
                    self.scan(child)
            return
        if kind == self.ck.VAR_DECL:
            self.register(cursor)
            if cursor.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                self.facts.pointer_targets.setdefault(self.ident(cursor), set()).update(
                    self.targets_for_type(cursor.type.get_pointee()))
            expressions = self.expressions(cursor)
            initializer = expressions[-1] if expressions else None
            # A VLA bound is an expression child, but is not an initializer.
            tokens = tuple(token.spelling for token in cursor.get_tokens())
            if '=' not in tokens:
                initializer = None
            storage = cursor.storage_class.name
            global_storage = cursor.semantic_parent.kind != self.ck.FUNCTION_DECL \
                or storage == 'STATIC'
            self.initialize(cursor, initializer,
                            implicit=global_storage and cursor.is_definition())
            if initializer:
                self.scan(initializer, 'initialization')
            return
        if kind == self.ck.RETURN_STMT:
            children = self.expressions(cursor)
            if children:
                child = children[0]
                result = self.function_key + ':return'
                if result in self.facts.slots:
                    self.write({result}, self.expr(child), cursor, 'return')
                elif child.type.get_canonical().kind == self.cindex.TypeKind.POINTER:
                    self.facts.pointer_edges.add((self.function_key + ':pointer_return',
                                                  self.pointer(child)))
                self.scan(child, 'return')
            elif self.function_key + ':return' in self.facts.slots:
                self.write({self.function_key + ':return'}, ('unknown', 'bare-return'),
                           cursor, 'bare-return')
            return
        if kind == self.ck.CALL_EXPR:
            self.call(cursor)
            key = self.register(cursor.referenced, result=True) if cursor.referenced \
                and cursor.referenced.kind == self.ck.FUNCTION_DECL else None
            if key:
                self.use({key}, usage, cursor)
            return
        if kind in {self.ck.IF_STMT, self.ck.WHILE_STMT, self.ck.DO_STMT,
                    self.ck.SWITCH_STMT}:
            children = self.children(cursor)
            condition_index = len(children) - 1 if kind == self.ck.DO_STMT else 0
            for index, child in enumerate(children):
                self.scan(child, ('switch' if kind == self.ck.SWITCH_STMT else 'truth')
                          if index == condition_index else 'value')
            return
        if kind in {self.ck.BINARY_OPERATOR, self.ck.COMPOUND_ASSIGNMENT_OPERATOR}:
            children = self.expressions(cursor)
            if len(children) != 2:
                return
            left, right = children
            operator = cursor.binary_operator.name
            if operator == 'Assign' or operator.endswith('Assign'):
                self.assign(left, right, cursor, operator=operator)
                if operator != 'Assign':
                    self.scan(left, 'arithmetic')
                self.scan(right, 'copy' if operator == 'Assign' else 'arithmetic')
                self.scan_address(left)
            else:
                purpose = 'arithmetic'
                if operator in {'LAnd', 'LOr'}:
                    purpose = 'truth'
                elif operator in {'EQ', 'NE'} and (
                    self.constant(left) in {0, 1} or self.constant(right) in {0, 1}
                ):
                    purpose = 'truth-comparison'
                elif operator in {'EQ', 'NE', 'LT', 'LE', 'GT', 'GE'}:
                    purpose = 'numeric-comparison'
                elif operator == 'Comma':
                    purpose = usage
                self.scan(left, purpose)
                self.scan(right, purpose)
            return
        if kind == self.ck.UNARY_OPERATOR:
            children = self.expressions(cursor)
            operator = self.unary(cursor)
            if not children:
                return
            child = children[0]
            if operator in {'++', '--'}:
                self.write(self.targets(child), ('unknown', 'increment-or-decrement'),
                           cursor, 'arithmetic-write')
            if operator == '&':
                bare = self.strip(child)
                if bare.referenced and bare.referenced.kind == self.ck.FUNCTION_DECL:
                    self.facts.address_taken.add(self.ident(bare.referenced))
                self.use(self.targets(child), 'address', cursor)
                self.scan_address(child)
            elif operator == '*':
                self.read(cursor, usage)
            else:
                self.scan(child, 'truth' if operator == '!' else
                          'arithmetic' if operator in {'++', '--', '~', '-', '+'} else usage)
            return
        if kind == self.ck.CONDITIONAL_OPERATOR:
            for index, child in enumerate(self.expressions(cursor)):
                self.scan(child, 'truth' if index == 0 else usage)
            return
        if kind == self.ck.ARRAY_SUBSCRIPT_EXPR:
            self.read(cursor, usage)
            return
        if kind in {self.ck.DECL_REF_EXPR, self.ck.MEMBER_REF_EXPR}:
            referenced = cursor.referenced
            if referenced and referenced.kind == self.ck.FUNCTION_DECL:
                self.facts.address_taken.add(self.ident(referenced))
            else:
                self.read(cursor, usage)
            return
        for child in cursor.get_children():
            self.scan(child, usage)

    def run(self) -> Facts:
        self.facts.errors.extend(str(diagnostic) for diagnostic in self.tu.diagnostics
                                 if diagnostic.severity >= self.cindex.Diagnostic.Error)
        self.return_warnings = [here for diagnostic in self.tu.diagnostics
                                if diagnostic.option == '-Wreturn-type'
                                and (here := _location(diagnostic.location, self.repo)) is not None]
        for inclusion in self.tu.get_includes():
            try:
                path = Path(str(inclusion.include)).resolve().relative_to(self.repo)
            except ValueError:
                continue
            if path.parts and path.parts[0] == 'include':
                self.facts.headers.add(str(path))
        for node in self.tu.cursor.get_children():
            if node.kind != self.ck.MACRO_INSTANTIATION:
                continue
            if node.spelling == 'KF_ENUM_BEGIN':
                tokens = tuple(token.spelling for token in node.get_tokens())
                if len(tokens) > 2:
                    self.enum_domains.add(tokens[2])
            if node.spelling in {'KF_ENUM_STORAGE', 'KF_ENUM_PARAM', 'KF_ENUM_PROMOTED',
                                 'KF_BOOL', 'KF_BOOL_STORAGE', 'KF_BOOL_PARAM'}:
                here = self.location(node)
                if here:
                    self.domain_ranges[here.file].append((
                        node.extent.start.offset, node.extent.end.offset,
                        'enum' if node.spelling.startswith('KF_ENUM') else 'boolean'))
        self.scan(self.tu.cursor)
        return self.facts


def _extract(arguments: tuple[Unit, Path, Path]) -> Facts:
    return Extractor(*arguments).run()


def _mask(value: int) -> int:
    return ZERO if value == 0 else ONE if value == 1 else OTHER


def _members(mask: int) -> tuple[int, ...] | None:
    if mask & (OTHER | UNKNOWN):
        return None
    return tuple(value for bit, value in ((ZERO, 0), (ONE, 1)) if mask & bit)


def _union(values) -> int:
    result = 0
    for value in values:
        result |= value
    return result


def evaluate(expression: Expr, values: dict[str, int], pointers: dict[str, set[str]]) -> int:
    kind = expression[0]
    if kind == 'constant':
        return _mask(expression[1])
    if kind == 'unknown':
        return UNKNOWN
    if kind == 'slot':
        return values.get(expression[1], UNKNOWN)
    if kind == 'deref':
        targets = pointers.get(expression[1], set())
        return _union(values.get(target, UNKNOWN) for target in targets) if targets else UNKNOWN
    if kind == 'union':
        return _union(evaluate(child, values, pointers) for child in expression[1:])
    if kind == 'cast':
        mask = evaluate(expression[-1], values, pointers)
        if expression[2] == 'bool':
            return BOOLEAN if mask else 0
        return mask
    if kind == 'byte-fill':
        byte = evaluate(expression[1], values, pointers)
        if byte == ZERO:
            return ZERO
        if byte == ONE:
            return ONE if expression[2] == 1 else OTHER
        return UNKNOWN
    if kind == 'conditional':
        condition = evaluate(expression[1], values, pointers)
        if not condition:
            return 0
        return _union(evaluate(expression[index], values, pointers)
                      for index in ((2,) if condition == ONE else (3,) if condition == ZERO
                                    else (2, 3)))
    if kind == 'unary':
        mask = evaluate(expression[2], values, pointers)
        if expression[1] == '!':
            return ONE if mask == ZERO else ZERO if mask and not mask & (ZERO | UNKNOWN) \
                else BOOLEAN
        members = _members(mask)
        if members is None:
            return UNKNOWN | (OTHER if mask & OTHER else 0)
        operations = {'+': lambda value: value, '-': lambda value: -value,
                      '~': lambda value: ~value}
        operation = operations.get(expression[1])
        return _union(_mask(operation(value)) for value in members) if operation else UNKNOWN
    if kind == 'binary':
        operator = expression[1]
        if operator in {'EQ', 'NE', 'LT', 'LE', 'GT', 'GE', 'LAnd', 'LOr'}:
            return BOOLEAN
        if operator in {'Assign', 'Comma'}:
            return evaluate(expression[3], values, pointers)
        left = evaluate(expression[2], values, pointers)
        right = evaluate(expression[3], values, pointers)
        if operator == 'And' and (expression[2] == ('constant', 1)
                                  or expression[3] == ('constant', 1)):
            return BOOLEAN
        if not left or not right:
            return 0
        a, b = _members(left), _members(right)
        if a is None or b is None:
            return UNKNOWN | (OTHER if (left | right) & OTHER else 0)
        operations = {
            'Add': lambda x, y: x + y, 'Sub': lambda x, y: x - y,
            'Mul': lambda x, y: x * y, 'Div': lambda x, y: x // y,
            'Rem': lambda x, y: x % y, 'And': lambda x, y: x & y,
            'Or': lambda x, y: x | y, 'Xor': lambda x, y: x ^ y,
            'Shl': lambda x, y: x << y, 'Shr': lambda x, y: x >> y,
        }
        operation = operations.get(operator)
        if operation:
            try:
                return _union(_mask(operation(x, y)) for x in a for y in b)
            except (ZeroDivisionError, ValueError):
                pass
    return UNKNOWN


def _merge(results: tuple[Facts, ...]) -> Facts:
    merged = Facts.empty()
    for result in results:
        for key, slot in result.slots.items():
            previous = merged.slots.get(key)
            if previous is None or slot.definition and not previous.definition:
                merged.slots[key] = slot
        for key, targets in result.pointer_targets.items():
            merged.pointer_targets.setdefault(key, set()).update(targets)
        for key, parameters in result.parameters.items():
            merged.parameters.setdefault(key, set()).update(parameters)
        for name in ('writes', 'uses', 'indirect_writes', 'indirect_uses', 'calls', 'errors'):
            getattr(merged, name).extend(getattr(result, name))
        for name in ('pointer_edges', 'definitions', 'address_taken', 'overlaps', 'hazards',
                     'declarations', 'headers'):
            getattr(merged, name).update(getattr(result, name))
    merged.writes = list(dict.fromkeys(merged.writes))
    merged.uses = list(dict.fromkeys(merged.uses))
    return merged


def solve(facts: Facts) -> tuple[dict[str, int], list[Write], dict[str, set[str]]]:
    pointers = {key: set(targets) for key, targets in facts.pointer_targets.items()}
    changed = True
    while changed:
        changed = False
        for destination, source in facts.pointer_edges:
            targets = pointers.setdefault(destination, set())
            before = len(targets)
            targets.update(pointers.get(source, ()))
            changed |= len(targets) != before
    writes = list(facts.writes)
    hazards: dict[str, set[str]] = defaultdict(set)
    for key, reason in facts.hazards:
        hazards[key].add(reason)
    for slot in facts.slots.values():
        if slot.kind != 'parameter_pointee':
            continue
        inputs = pointers.get(slot.key.removesuffix(':pointee'), set()) - {slot.key}
        if inputs:
            writes.append(Write(slot.key, ('union', *(('slot', key) for key in sorted(inputs))),
                                slot.location, slot.owner, 'pointee-input'))
    for write in facts.indirect_writes:
        for target in pointers.get(write.target, ()):
            writes.append(Write(target, write.expression, write.location, write.function,
                                write.reason))
            hazards[target].add('pointer-write')
    for use in facts.indirect_uses:
        facts.uses.extend(Use(target, use.kind, use.location, use.function)
                          for target in pointers.get(use.target, ()))
    called = {call.callee for call in facts.calls}
    for function, parameters in facts.parameters.items():
        reason = 'callback-input' if function in facts.address_taken else \
            'no-observed-callers' if function not in called else ''
        if reason:
            for parameter in parameters:
                slot = facts.slots[parameter]
                writes.append(Write(parameter, ('unknown', reason), slot.location, slot.owner,
                                    reason))
                hazards[parameter].add(reason)
    for call in facts.calls:
        if call.callee in facts.definitions:
            continue
        for pointer in call.pointers:
            for target in pointers.get(pointer, ()):
                if call.memory_fill is not None:
                    slot = facts.slots.get(target)
                    writes.append(Write(target, ('byte-fill', call.memory_fill,
                                                  slot.size if slot else -1), call.location,
                                        call.function, 'whole-object-memset'))
                    continue
                writes.append(Write(target, ('unknown', 'external-pointer-write'), call.location,
                                    call.function, 'external-pointer-write'))
                hazards[target].add('external-pointer-write')
    by_target: dict[str, list[Write]] = defaultdict(list)
    for write in writes:
        by_target[write.target].append(write)
    for source, target in facts.overlaps:
        if source not in by_target:
            continue
        hazards[target].add('overlapping-union-write')
        witness = by_target[source][0]
        writes.append(Write(target, ('unknown', 'overlapping-union-write'), witness.location,
                            witness.function, 'overlapping-union-write'))
    values = {key: 0 for key in facts.slots}
    for slot in facts.slots.values():
        if slot.kind == 'return' and slot.key.removesuffix(':return') not in facts.definitions:
            values[slot.key] = UNKNOWN
            hazards[slot.key].add('missing-function-body')
        if slot.kind.startswith('parameter') \
                and slot.key.split(':parameter:')[0] not in facts.definitions:
            hazards[slot.key].add('missing-function-body')
    changed = True
    while changed:
        changed = False
        for write in writes:
            previous = values.get(write.target, 0)
            value = previous | evaluate(write.expression, values, pointers)
            if value != previous:
                values[write.target] = value
                changed = True
    # Unproduced references are unknown, not an empty proof of a Boolean range.
    empty = {key for key, value in values.items() if not value}
    for key in empty:
        values[key] = UNKNOWN
        hazards[key].add('no-observed-writes')
    if empty:
        changed = True
        while changed:
            changed = False
            for write in writes:
                previous = values.get(write.target, 0)
                value = previous | evaluate(write.expression, values, pointers)
                if value != previous:
                    values[write.target] = value
                    changed = True
    def copied_slots(expression: Expr) -> set[str]:
        if expression[0] == 'slot':
            return {expression[1]}
        if expression[0] == 'cast':
            return copied_slots(expression[-1])
        if expression[0] == 'union':
            return set().union(*(copied_slots(child) for child in expression[1:]))
        return set()

    numeric = {'arithmetic', 'index', 'switch', 'numeric-comparison'}
    consumers = {use.target for use in facts.uses if use.kind in numeric}
    changed = True
    while changed:
        changed = False
        for write in writes:
            if write.target not in consumers:
                continue
            for source in copied_slots(write.expression) - consumers:
                consumers.add(source)
                facts.uses.append(Use(source, 'numeric-flow', write.location, write.function))
                changed = True
    return values, sorted(set(writes), key=lambda write: (
        write.target, write.location, write.function, write.reason, repr(write.expression))), hazards


def _classification(slot: Slot, value: int, writes: list[Write], uses: list[Use],
                    hazards: set[str]) -> str:
    if slot.domain:
        return 'already-' + slot.domain
    if slot.location.file.startswith('src/vendor/'):
        return 'vendored'
    if slot.location.file.startswith('include/kf/psyq_') or 'missing-function-body' in hazards:
        return 'external-boundary'
    if value & OTHER:
        return 'non-boolean'
    observed = any(write.expression[0] == 'constant' and write.expression[1] in (0, 1)
                   or write.expression[0] in {'unary', 'binary'} and (
                       write.expression[1] == '!' or write.expression[1] in
                       {'EQ', 'NE', 'LT', 'LE', 'GT', 'GE', 'LAnd', 'LOr'})
                   for write in writes)
    if value & UNKNOWN:
        return 'review-unknown-writers' if observed else 'unknown'
    if any(use.kind in {'arithmetic', 'index', 'switch', 'numeric-comparison', 'numeric-flow'} for use in uses):
        return 'review-numeric-use'
    if slot.bit_width >= 0:
        return 'review-storage-conversion'
    if value in {ZERO, ONE}:
        return 'review-single-value'
    return 'candidate'


def collect(*, images: tuple[str, ...] = (), names: tuple[str, ...] = (), jobs: int = 4,
            repo: Path = REPO, manifest: Manifest | None = None,
            sdk: Path | None = None) -> dict[str, Any]:
    if jobs < 1:
        raise ValueError('Boolean-audit jobs must be positive')
    repo = repo.resolve()
    selected = select_units(manifest or load_manifest(), images=images, names=names)
    before = _hashes(repo)
    sdk = _sdk_path(sdk)
    work = tuple((unit, repo, sdk) for unit in selected)
    if jobs == 1:
        results = tuple(_extract(arguments) for arguments in work)
    else:
        with ProcessPoolExecutor(max_workers=min(jobs, len(work)),
                                 mp_context=multiprocessing.get_context('fork')) as pool:
            results = tuple(pool.map(_extract, work))
    if _hashes(repo) != before:
        raise RuntimeError('source changed during Boolean audit; rerun on a stable tree')
    facts = _merge(results)
    if facts.errors:
        raise RuntimeError('target-C parsing failed:\n' + '\n'.join(facts.errors[:20]))
    sources = {unit.source for unit in selected}
    missing_sources = sorted(path for path in before if path.startswith('src/')
                             and path.endswith('.c') and path not in sources)
    missing_headers = sorted(path for path in before if path.startswith('include/')
                             and path.endswith('.h') and path not in facts.headers)
    if not images and not names and (missing_sources or missing_headers):
        raise RuntimeError(f'incomplete Boolean-audit coverage: sources={missing_sources}; '
                           f'headers={missing_headers}')
    values, writes, hazards = solve(facts)
    by_writes: dict[str, list[Write]] = defaultdict(list)
    by_uses: dict[str, list[Use]] = defaultdict(list)
    for write in writes:
        by_writes[write.target].append(write)
    for use in facts.uses:
        by_uses[use.target].append(use)
    rows = []
    for key, slot in sorted(facts.slots.items(), key=lambda item: (
        item[1].image, item[1].location, item[1].kind, item[1].name,
    )):
        value = values[key]
        row = asdict(slot)
        row.update({
            'classification': _classification(slot, value, by_writes[key], by_uses[key],
                                              hazards[key]),
            'values': [label for bit, label in ((ZERO, '0'), (ONE, '1'), (OTHER, 'other'),
                                               (UNKNOWN, 'unknown')) if value & bit],
            'hazards': sorted(hazards[key]),
            'writes': [asdict(write) for write in by_writes[key]],
            'uses': [asdict(use) for use in sorted(set(by_uses[key]), key=lambda use:
                (use.location, use.function, use.kind))],
        })
        rows.append(row)
    return {
        'schema': 1, 'mode': 'target-c-gnu89', 'proof_scope': 'observed-source-value-flow',
        'coverage': {
            'variants': len(selected), 'sources': sorted(sources),
            'headers': sorted(facts.headers), 'declarations': len(facts.declarations),
            'integral_slots': len(rows), 'function_bodies': len(facts.definitions),
            'indirect_write_sites': len(set(facts.indirect_writes)),
            'indirect_call_sites': len({call for call in facts.calls if not call.callee}),
            'missing_sources': missing_sources, 'missing_headers': missing_headers,
            'partial_selection': bool(images or names), 'parse_errors': 0,
        },
        'limitations': [
            'Flow-insensitive: observed writes do not prove initialization on every path.',
            'Fields are type-wide; arrays join all elements; unions are conservative.',
            'Unresolved pointers, indirect calls, and unreconstructed retail may hide writers.',
            'The other value class may be an overapproximation after arithmetic or casts.',
            'Bitfield truncation requires manual storage review.',
            'Candidate is a source-review proposal, never an automatic type conversion.',
        ],
        'summary': dict(sorted(Counter(row['classification'] for row in rows).items())),
        'by_kind': dict(sorted(Counter(row['kind'] for row in rows).items())),
        'slots': rows,
    }


def add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument('--image', action='append', choices=tuple(IMAGE_NAMES))
    parser.add_argument('--unit', action='append')
    parser.add_argument('-j', '--jobs', type=int, default=4)
    parser.add_argument('--output', type=Path, help='write the complete JSON evidence census')
    parser.add_argument('--list', action='store_true', help='list candidates and review cases')
    parser.add_argument('--all', action='store_true', help='include rejected and modeled slots')


def run(args: argparse.Namespace) -> int:
    audit = collect(images=tuple(IMAGE_NAMES[image] for image in args.image or ()),
                    names=tuple(args.unit or ()), jobs=args.jobs)
    coverage = audit['coverage']
    print(f"[bools] {coverage['integral_slots']} integral slots; "
          f"{coverage['variants']} source/image variants; "
          f"{len(coverage['headers'])} project headers")
    for category, count in audit['summary'].items():
        print(f'[bools] {category}: {count}')
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(audit, indent=2) + '\n')
        print(f'[bools] complete evidence: {args.output}')
    if args.list or args.all:
        for row in audit['slots']:
            if not args.all and not (row['classification'] == 'candidate'
                                    or row['classification'].startswith('review-')):
                continue
            here = row['location']
            print(f"{row['image']} {here['file']}:{here['line']} "
                  f"{row['kind']} {row['owner']}::{row['name']} "
                  f"[{row['declared_type']}] {row['classification']} "
                  f"values={','.join(row['values'])}")
    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    add_arguments(parser)
    raise SystemExit(run(parser.parse_args()))
