"""Curated semantic identities and evidence proposals for retail functions/data.

The structural censuses answer *where* code and data live.  This module layers
WIP source identities over those rows without changing delinking symbols:

* ``function_identities.tsv`` covers every carveable non-vendored function;
* ``data_identities.tsv`` contains reviewed global/static identities; and
* ``structures.tsv`` plus ``structure_fields.tsv`` describe checked C layouts;
* ``propose`` writes instruction/xref/string dossiers under ``build/`` only.

Address-derived names are explicit unresolved identities, not semantic claims.
"""

from __future__ import annotations

import bisect

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    IMAGE_ORDER,
    RELOC_FIELDS,
    parse_int,
    read_tsv,
    write_tsv,
)
from scripts.kf.sema.image import retail
from scripts.kf.relocations import decode_hi_lo_target


FUNCTION_IDENTITY_FIELDS = (
    "image",
    "va",
    "name",
    "owner",
    "action",
    "return_type",
    "parameters",
    "name_confidence",
    "signature_confidence",
    "evidence",
    "note",
)
DATA_IDENTITY_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "scope",
    "storage",
    "datatype",
    "owner",
    "confidence",
    "evidence",
    "note",
)
STRUCTURE_FIELDS = (
    "name",
    "size",
    "layout_confidence",
    "evidence",
    "note",
)
STRUCTURE_FIELD_FIELDS = (
    "structure",
    "offset",
    "size",
    "name",
    "datatype",
    "meaning_confidence",
    "evidence",
    "note",
)
EVIDENCE_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "current_return_type",
    "current_parameters",
    "mips_parameters_hint",
    "mips_return_hint",
    "callers",
    "callees",
    "indirect_calls",
    "strings",
    "data_references",
    "shape",
)
DATA_EVIDENCE_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "storage",
    "datatype",
    "current_scope",
    "current_owner",
    "confirmed_users",
    "candidate_users",
    "reads",
    "writes",
    "addresses",
    "scope_hint",
    "scope_hint_confidence",
    "owner_hint",
)
GHIDRA_DATA_XREF_FIELDS = (
    "image",
    "target_va",
    "ghidra_name",
    "binding",
    "binding_offset",
    "region",
    "reference_types",
    "users",
    "sites",
    "admitted_sites",
)
CONFIDENCE = {"address-only", "candidate", "supported", "proven"}
LAYOUT_CONFIDENCE = {"candidate", "supported", "proven"}
FIELD_CONFIDENCE = {"opaque", "candidate", "supported", "proven"}
SCOPES = {"unknown", "global", "static", "function-static"}
IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
REGISTER_NAMES = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)
LOW_MNEMONICS = {
    0x08: "addi",
    0x09: "addiu",
    0x0D: "ori",
    0x20: "lb",
    0x21: "lh",
    0x23: "lw",
    0x24: "lbu",
    0x25: "lhu",
    0x28: "sb",
    0x29: "sh",
    0x2B: "sw",
}


@dataclass(frozen=True)
class FunctionIdentity:
    image: str
    va: int
    name: str
    owner: str
    action: str
    return_type: str
    parameters: str
    name_confidence: str
    signature_confidence: str
    evidence: str
    note: str

    @property
    def unresolved(self) -> bool:
        return self.name_confidence == "address-only"

    def as_dict(self) -> dict[str, object]:
        return {
            "image": self.image,
            "va": self.va,
            "name": self.name,
            "owner": self.owner or None,
            "action": self.action or None,
            "return_type": self.return_type or None,
            "parameters": self.parameters or None,
            "name_confidence": self.name_confidence,
            "signature_confidence": self.signature_confidence,
            "evidence": self.evidence,
            "note": self.note or None,
            "unresolved": self.unresolved,
        }


@dataclass(frozen=True)
class DataIdentity:
    image: str
    va: int
    size: int
    name: str
    scope: str
    storage: str
    datatype: str
    owner: str
    confidence: str
    evidence: str
    note: str

    def as_dict(self) -> dict[str, object]:
        return {
            "image": self.image,
            "va": self.va,
            "size": self.size,
            "name": self.name,
            "scope": self.scope,
            "storage": self.storage,
            "datatype": self.datatype or None,
            "owner": self.owner or None,
            "confidence": self.confidence,
            "evidence": self.evidence,
            "note": self.note or None,
        }


@dataclass(frozen=True)
class StructureIdentity:
    name: str
    size: int
    layout_confidence: str
    evidence: str
    note: str


@dataclass(frozen=True)
class StructureFieldIdentity:
    structure: str
    offset: int
    size: int
    name: str
    datatype: str
    meaning_confidence: str
    evidence: str
    note: str


@dataclass(frozen=True)
class HeaderFieldLayout:
    offset: int
    size: int
    name: str
    datatype: str


@dataclass(frozen=True)
class HeaderStructureLayout:
    size: int
    alignment: int
    fields: tuple[HeaderFieldLayout, ...]


def _function_universe(config_dir: Path) -> dict[tuple[str, int], dict[str, str]]:
    _, functions = read_tsv(config_dir / "functions.tsv")
    _, vendors = read_tsv(config_dir / "functions_vendored.tsv")
    vendored = {(row["image"], parse_int(row["va"])) for row in vendors}
    return {
        (row["image"], parse_int(row["va"])): row
        for row in functions
        if (row["image"], parse_int(row["va"])) not in vendored
        and int(row["fragments"]) == 1
    }


def load_function_identities(
    config_dir: Path = RETAIL_CONFIG,
    *,
    required: bool = False,
) -> dict[tuple[str, int], FunctionIdentity]:
    path = config_dir / "function_identities.tsv"
    if not path.is_file():
        if required:
            raise ValueError(f"{path}: missing function identity inventory")
        return {}
    fields, rows = read_tsv(path)
    if fields != FUNCTION_IDENTITY_FIELDS:
        raise ValueError(f"{path}: fields {fields!r}, expected {FUNCTION_IDENTITY_FIELDS!r}")
    result = {}
    for row in rows:
        identity = FunctionIdentity(
            image=row["image"],
            va=parse_int(row["va"]),
            name=row["name"],
            owner=row["owner"],
            action=row["action"],
            return_type=row["return_type"],
            parameters=row["parameters"],
            name_confidence=row["name_confidence"],
            signature_confidence=row["signature_confidence"],
            evidence=row["evidence"],
            note=row["note"],
        )
        key = identity.image, identity.va
        if key in result:
            raise ValueError(f"{path}: duplicate identity {key!r}")
        result[key] = identity
    return result


def load_data_identities(
    config_dir: Path = RETAIL_CONFIG,
) -> dict[tuple[str, int], DataIdentity]:
    path = config_dir / "data_identities.tsv"
    if not path.is_file():
        return {}
    fields, rows = read_tsv(path)
    if fields != DATA_IDENTITY_FIELDS:
        raise ValueError(f"{path}: fields {fields!r}, expected {DATA_IDENTITY_FIELDS!r}")
    result = {}
    for row in rows:
        identity = DataIdentity(
            image=row["image"],
            va=parse_int(row["va"]),
            size=parse_int(row["size"]),
            name=row["name"],
            scope=row["scope"],
            storage=row["storage"],
            datatype=row["datatype"],
            owner=row["owner"],
            confidence=row["confidence"],
            evidence=row["evidence"],
            note=row["note"],
        )
        key = identity.image, identity.va
        if key in result:
            raise ValueError(f"{path}: duplicate identity {key!r}")
        result[key] = identity
    return result


def load_structure_identities(
    config_dir: Path = RETAIL_CONFIG,
) -> dict[str, StructureIdentity]:
    path = config_dir / "structures.tsv"
    fields, rows = read_tsv(path)
    if fields != STRUCTURE_FIELDS:
        raise ValueError(f"{path}: fields {fields!r}, expected {STRUCTURE_FIELDS!r}")
    result = {}
    for row in rows:
        identity = StructureIdentity(
            name=row["name"],
            size=parse_int(row["size"]),
            layout_confidence=row["layout_confidence"],
            evidence=row["evidence"],
            note=row["note"],
        )
        if identity.name in result:
            raise ValueError(f"{path}: duplicate structure {identity.name!r}")
        result[identity.name] = identity
    return result


def load_structure_field_identities(
    config_dir: Path = RETAIL_CONFIG,
) -> tuple[StructureFieldIdentity, ...]:
    path = config_dir / "structure_fields.tsv"
    fields, rows = read_tsv(path)
    if fields != STRUCTURE_FIELD_FIELDS:
        raise ValueError(
            f"{path}: fields {fields!r}, expected {STRUCTURE_FIELD_FIELDS!r}"
        )
    return tuple(
        StructureFieldIdentity(
            structure=row["structure"],
            offset=parse_int(row["offset"]),
            size=parse_int(row["size"]),
            name=row["name"],
            datatype=row["datatype"],
            meaning_confidence=row["meaning_confidence"],
            evidence=row["evidence"],
            note=row["note"],
        )
        for row in rows
    )


def _align(value: int, alignment: int) -> int:
    return (value + alignment - 1) // alignment * alignment


def _header_structure_layouts() -> dict[str, HeaderStructureLayout]:
    """Calculate target 32-bit layouts of flat named structs and unions.

    Inline anonymous aggregates remain outside this inventory; a containing
    checked type must use a separately named member type.
    """
    primitive_layouts = {
        "s8": (1, 1),
        "u8": (1, 1),
        "s16": (2, 2),
        "u16": (2, 2),
        "s32": (4, 4),
        "u32": (4, 4),
        "KfBool": (4, 4),
        "KfBool32": (4, 4),
        "KfBoolU32": (4, 4),
        "KfBool8": (1, 1),
        "KfBool16": (2, 2),
        # Target O32/Psy-Q long, independent of the host Python ABI.
        "long": (4, 4),
        # Psy-Q SDK fixed-layout types (declared in real LIBGTE/LIBGPU headers,
        # not parsed here); registered so project structs can use them.
        "MATRIX": (0x20, 4),
        "VECTOR": (0x10, 4),
        "SVECTOR": (8, 2),
        "CVECTOR": (4, 1),
        "DVECTOR": (4, 2),
        "DRAWENV": (0x5C, 4),
        "DISPENV": (0x14, 2),
        "POLY_F4": (0x18, 4),
        "POLY_FT4": (0x28, 4),
    }
    layouts: dict[str, HeaderStructureLayout] = {}
    definition_pattern = re.compile(
        r"(?:typedef\s+)?(struct|union)\s+([A-Za-z_]\w*)\s*\{([^{}]*)\}\s*"
        r"(?:[A-Za-z_]\w*)?\s*;",
        re.DOTALL,
    )
    array_bound = r"(?:0x[0-9a-fA-F]+|\d+|[A-Za-z_]\w*)"
    declaration_pattern = re.compile(
        rf"(.+?)\s+(\**)([A-Za-z_]\w*)((?:\s*\[\s*{array_bound}\s*\])*)"
    )
    array_pattern = re.compile(rf"\[\s*({array_bound})\s*\]")
    enum_pattern = re.compile(r"\benum(?:\s+[A-Za-z_]\w*)?\s*\{([^{}]*)\}", re.DOTALL)
    stored_enum_pattern = re.compile(
        r"\bKF_ENUM_BEGIN\(\s*([A-Za-z_]\w*)\s*,\s*([A-Za-z_]\w*)\s*\)"
        r"(.*?)\bKF_ENUM_END\(\s*\1\s*\)", re.DOTALL,
    )
    enum_storage_pattern = re.compile(
        r"KF_ENUM_STORAGE\(\s*([A-Za-z_]\w*)\s*,\s*([A-Za-z_]\w*)\s*\)"
    )
    enum_storage_typedef_pattern = re.compile(
        r"\btypedef\s+" + enum_storage_pattern.pattern + r"\s+([A-Za-z_]\w*)\s*;"
    )
    enum_storage_types = {"s8", "u8", "s16", "u16", "s32", "u32", "long"}
    integer_enumerator = re.compile(r"([A-Za-z_]\w*)\s*=\s*(0x[0-9a-fA-F]+|\d+)")
    alias_enumerator = re.compile(r"([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)")
    implicit_enumerator = re.compile(r"[A-Za-z_]\w*")
    checked_headers = (
        REPO / "include/kf/game_types.h",
        REPO / "include/kf/memory_layout.h",
        REPO / "include/kf/combat.h",
        REPO / "include/kf/overlay.h",
        REPO / "include/kf/animation.h",
        REPO / "include/kf/cd_file.h",
        REPO / "include/kf/floor.h",
        REPO / "include/kf/audio.h",
        REPO / "include/kf/resources.h",
        REPO / "include/kf/map_data.h",
        REPO / "include/kf/memory.h",
        REPO / "include/kf/game_math.h",
        REPO / "include/kf/player_status.h",
        REPO / "include/kf/game_actor.h",
        REPO / "include/kf/game_map.h",
        REPO / "include/kf/game_collision.h",
        REPO / "include/kf/item.h",
        REPO / "include/kf/pool.h",
        REPO / "include/kf/magic.h",
        REPO / "include/kf/game_effect.h",
        REPO / "include/kf/game_equipment.h",
        REPO / "include/kf/game_player.h",
        REPO / "include/kf/render_types.h",
        REPO / "include/kf/tmd.h",
        REPO / "include/kf/game_asset.h",
        REPO / "include/kf/game_render.h",
        REPO / "include/kf/notify_types.h",
        REPO / "include/kf/notify.h",
        REPO / "include/kf/open_render.h",
        REPO / "include/kf/game_graphics.h",
        REPO / "include/kf/open_resources.h",
        REPO / "include/kf/game_save.h",
        REPO / "include/kf/game_menu.h",
        REPO / "include/kf/game_cd.h",
    )
    # Resolve named aggregates recursively, including standalone union views.
    definitions = {}
    constants: dict[str, int] = {}
    enum_domains: set[str] = set()
    for path in checked_headers:
        text = re.sub(r"/\*.*?\*/", "", path.read_text(), flags=re.DOTALL)
        text = re.sub(r"//[^\n]*", "", text)
        enum_bodies = [enum[1] for enum in enum_pattern.finditer(text)]
        for enum in stored_enum_pattern.finditer(text):
            name, storage, body = enum.groups()
            if storage not in enum_storage_types:
                raise ValueError(f"{path}: unsupported enum storage {storage!r} for {name}")
            if name in primitive_layouts or name in definitions:
                raise ValueError(f"{path}: duplicate checked type {name}")
            primitive_layouts[name] = primitive_layouts[storage]
            enum_domains.add(name)
            enum_bodies.append(body)
        for alias in enum_storage_typedef_pattern.finditer(text):
            domain, storage, name = alias.groups()
            if domain not in enum_domains:
                raise ValueError(f"{path}: undeclared enum domain {domain!r}")
            if storage not in enum_storage_types:
                raise ValueError(f"{path}: unsupported enum storage {storage!r}")
            if name in primitive_layouts or name in definitions:
                raise ValueError(f"{path}: duplicate checked type {name}")
            primitive_layouts[name] = primitive_layouts[storage]
        # Aliases may use a previously resolved member, including one from a
        # shared header. Unknown aliases/expressions break the implicit chain;
        # never guess an inventory's byte extent.
        for body in enum_bodies:
            next_value = 0
            for enumerator in body.split(","):
                enumerator = enumerator.strip()
                if not enumerator:
                    continue
                constant = integer_enumerator.fullmatch(enumerator)
                alias = alias_enumerator.fullmatch(enumerator)
                if constant:
                    constant_name, value = constant.groups()
                    number = int(value, 0)
                elif alias and alias[2] in constants:
                    constant_name, number = alias[1], constants[alias[2]]
                elif implicit_enumerator.fullmatch(enumerator) and next_value is not None:
                    constant_name, number = enumerator, next_value
                else:
                    next_value = None
                    continue
                if constant_name in constants:
                    raise ValueError(f"{path}: duplicate checked constant {constant_name}")
                constants[constant_name] = number
                next_value = number + 1 if number < 0x7fffffff else None
        for match in definition_pattern.finditer(text):
            kind, name, body = match.groups()
            if name in definitions or name in primitive_layouts:
                raise ValueError(f"{path}: duplicate checked structure {name}")
            definitions[name] = (path, kind, body)

    def layout_definition(name: str) -> HeaderStructureLayout:
        if name not in layouts:
            path, kind, body = definitions[name]
            offset = 0
            alignment = 1
            fields = []
            for declaration in body.split(";"):
                declaration = " ".join(declaration.split())
                if not declaration:
                    continue
                field_match = declaration_pattern.fullmatch(declaration)
                if field_match is None:
                    raise ValueError(
                        f"{path}: cannot parse {name} declaration {declaration!r}"
                    )
                datatype, pointer, field_name, arrays = field_match.groups()
                datatype = (
                    datatype.removeprefix("const ")
                    .removeprefix("struct ")
                    .removeprefix("union ")
                )
                storage_match = enum_storage_pattern.fullmatch(datatype)
                if storage_match and not pointer:
                    domain, storage = storage_match.groups()
                    if domain not in enum_domains:
                        raise ValueError(f"{path}: undeclared enum domain {domain!r}")
                    if storage not in enum_storage_types:
                        raise ValueError(f"{path}: unsupported enum storage {storage!r}")
                    base_size, base_alignment = primitive_layouts[storage]
                    display_type = f"KF_ENUM_STORAGE({domain}, {storage})"
                elif pointer:
                    base_size, base_alignment = 4, 4
                    display_type = f"{datatype} {'*' * len(pointer)}"
                elif datatype in primitive_layouts:
                    base_size, base_alignment = primitive_layouts[datatype]
                    display_type = datatype
                elif datatype in definitions:
                    nested = layout_definition(datatype)
                    base_size, base_alignment = nested.size, nested.alignment
                    display_type = datatype
                else:
                    raise ValueError(
                        f"{path}: unknown field type {datatype!r} in {name}.{field_name}"
                    )
                count = 1
                dimensions = []
                for bound in array_pattern.findall(arrays):
                    if bound[0].isdigit():
                        dimension = int(bound, 0)
                    elif bound in constants:
                        dimension = constants[bound]
                    else:
                        raise ValueError(
                            f"{path}: unresolved array bound {bound!r} in {name}.{field_name}"
                        )
                    if dimension <= 0:
                        raise ValueError(
                            f"{path}: nonpositive array bound {bound!r} in {name}.{field_name}"
                        )
                    dimensions.append(dimension)
                    count *= dimension
                size = base_size * count
                field_offset = 0 if kind == "union" else _align(offset, base_alignment)
                if dimensions:
                    display_type += "".join(f"[{value}]" for value in dimensions)
                fields.append(
                    HeaderFieldLayout(field_offset, size, field_name, display_type)
                )
                offset = max(offset, field_offset + size)
                alignment = max(alignment, base_alignment)
            layouts[name] = HeaderStructureLayout(
                size=_align(offset, alignment),
                alignment=alignment,
                fields=tuple(fields),
            )
        return layouts[name]

    for name in definitions:
        layout_definition(name)
    return layouts


def _check_identifier(path: Path, field: str, value: str, key: tuple[str, int]) -> None:
    if value and IDENTIFIER.fullmatch(value) is None:
        raise ValueError(f"{path}: invalid {field} {value!r} at {key!r}")


def _check_parameters(path: Path, value: str, key: tuple[str, int]) -> None:
    parameters = value.split(";")
    for index, parameter in enumerate(parameters):
        if not parameter:
            continue
        if parameter == "...":
            if index == 0 or index != len(parameters) - 1:
                raise ValueError(
                    f"{path}: variadic marker must follow named parameters "
                    f"and appear last at {key!r}"
                )
            continue
        type_name, separator, name = parameter.rpartition(" ")
        declarator_name = name.lstrip("*")
        if (
            not separator
            or not type_name
            or not declarator_name
            or IDENTIFIER.fullmatch(declarator_name) is None
        ):
            raise ValueError(f"{path}: invalid parameter {parameter!r} at {key!r}")


def _validate_structure_identities(config_dir: Path) -> dict[str, int]:
    structures_path = config_dir / "structures.tsv"
    fields_path = config_dir / "structure_fields.tsv"
    structures = load_structure_identities(config_dir)
    fields = load_structure_field_identities(config_dir)
    header_layouts = _header_structure_layouts()
    if set(structures) != set(header_layouts):
        missing = sorted(set(header_layouts) - set(structures))
        extra = sorted(set(structures) - set(header_layouts))
        raise ValueError(
            f"{structures_path}: coverage differs from checked C layouts "
            f"(missing={missing!r}, extra={extra!r})"
        )
    if list(structures) != sorted(structures):
        raise ValueError(f"{structures_path}: rows are not canonically sorted")
    for name, identity in structures.items():
        _check_identifier(structures_path, "name", name, ("structure", 0))
        if identity.size <= 0 or identity.layout_confidence not in LAYOUT_CONFIDENCE:
            raise ValueError(f"{structures_path}: invalid structure {name!r}")
        if identity.size != header_layouts[name].size:
            raise ValueError(
                f"{structures_path}: {name} size is {identity.size:#x}, "
                f"header layout is {header_layouts[name].size:#x}"
            )
        if not identity.evidence:
            raise ValueError(f"{structures_path}: {name} lacks evidence")

    ordered = [(row.structure, row.offset) for row in fields]
    if ordered != sorted(ordered):
        raise ValueError(f"{fields_path}: rows are not canonically sorted")
    fields_by_structure: dict[str, list[StructureFieldIdentity]] = {}
    for row in fields:
        if row.structure not in structures:
            raise ValueError(
                f"{fields_path}: field belongs to unknown structure {row.structure!r}"
            )
        _check_identifier(fields_path, "name", row.name, (row.structure, row.offset))
        if (
            row.offset < 0
            or row.size <= 0
            or row.offset + row.size > structures[row.structure].size
            or row.meaning_confidence not in FIELD_CONFIDENCE
            or not row.datatype
            or not row.evidence
        ):
            raise ValueError(
                f"{fields_path}: invalid field {row.structure}.{row.name}"
            )
        fields_by_structure.setdefault(row.structure, []).append(row)

    for name, header in header_layouts.items():
        actual = fields_by_structure.get(name, [])
        expected = header.fields
        actual_shape = tuple(
            (row.offset, row.size, row.name, row.datatype) for row in actual
        )
        expected_shape = tuple(
            (row.offset, row.size, row.name, row.datatype) for row in expected
        )
        if actual_shape != expected_shape:
            raise ValueError(
                f"{fields_path}: {name} fields differ from checked C layout; "
                f"actual={actual_shape!r}, expected={expected_shape!r}"
            )
    return {
        "structures": len(structures),
        "structure_fields": len(fields),
        "structure_fields_named": sum(
            row.meaning_confidence != "opaque" for row in fields
        ),
    }


def validate(config_dir: Path = RETAIL_CONFIG) -> dict[str, int]:
    structure_counts = _validate_structure_identities(config_dir)
    universe = _function_universe(config_dir)
    functions = load_function_identities(config_dir, required=True)
    path = config_dir / "function_identities.tsv"
    missing = sorted(set(universe) - set(functions), key=lambda k: (IMAGE_ORDER[k[0]], k[1]))
    extra = sorted(set(functions) - set(universe), key=lambda k: (IMAGE_ORDER.get(k[0], 99), k[1]))
    if missing or extra:
        raise ValueError(
            f"{path}: coverage differs from carveable non-vendored functions "
            f"(missing={len(missing)}, extra={len(extra)})"
        )
    ordered = list(functions)
    if ordered != sorted(ordered, key=lambda k: (IMAGE_ORDER[k[0]], k[1])):
        raise ValueError(f"{path}: rows are not canonically sorted")
    names: set[tuple[str, str]] = set()
    for key, row in functions.items():
        if row.name_confidence not in CONFIDENCE or row.signature_confidence not in CONFIDENCE:
            raise ValueError(f"{path}: invalid confidence at {key!r}")
        for field, value in (("name", row.name), ("owner", row.owner), ("action", row.action)):
            _check_identifier(path, field, value, key)
        if not row.name:
            raise ValueError(f"{path}: empty stable name at {key!r}")
        if bool(row.owner) != bool(row.action):
            raise ValueError(f"{path}: owner and action must be set together at {key!r}")
        if row.owner and row.name != f"{row.owner}_{row.action}":
            raise ValueError(f"{path}: method name must be owner_action at {key!r}")
        if row.name_confidence == "address-only" and row.name != f"func_{row.va:08x}":
            raise ValueError(f"{path}: address-only row has semantic-looking name at {key!r}")
        _check_parameters(path, row.parameters, key)
        name_key = row.image, row.name
        if name_key in names:
            raise ValueError(f"{path}: duplicate image-qualified name {name_key!r}")
        names.add(name_key)

    _, structural_data = read_tsv(config_dir / "data.tsv")
    structural_data_by_start = {
        (row["image"], parse_int(row["va"])): row for row in structural_data
    }
    data_starts = set(structural_data_by_start)
    _, relocations = read_tsv(config_dir / "relocs.tsv")
    bss_starts = {
        (row["image"], parse_int(row["target_va"]))
        for row in relocations
        if row["target_region"] == "bss"
        and row["status"] != "rejected"
    }
    bss_targets_by_image: dict[str, list[int]] = {}
    for image, target in bss_starts:
        bss_targets_by_image.setdefault(image, []).append(target)
    for targets in bss_targets_by_image.values():
        targets.sort()

    def bss_interior_evidence(row: DataIdentity) -> bool:
        # An aggregate whose members are what code references (matrices reached
        # by folded offsets from a sibling field, for example) is evidenced by
        # any BSS relocation inside its extent, not only at its first byte.
        targets = bss_targets_by_image.get(row.image, [])
        index = bisect.bisect_left(targets, row.va)
        return index < len(targets) and targets[index] < row.va + row.size
    from scripts.kf.data_sections import load as load_sections

    from scripts.kf.data_reservations import load as load_reservations

    load_sections(config_dir)
    load_reservations(config_dir)
    data = load_data_identities(config_dir)
    data_path = config_dir / "data_identities.tsv"
    ordered_data = list(data)
    if ordered_data != sorted(ordered_data, key=lambda k: (IMAGE_ORDER[k[0]], k[1])):
        raise ValueError(f"{data_path}: rows are not canonically sorted")
    data_names: set[tuple[str, str]] = set()
    bss_by_image: dict[str, list[DataIdentity]] = {}
    for key, row in data.items():
        ghidra_bss = (
            row.storage == "bss"
            and "ghidra-12.0.4-memory-xref" in row.evidence.split(";")
            and IMAGE_LAYOUTS[row.image].load_end
            <= row.va
            < row.va + row.size
            <= 0x80200000
        )
        if (
            key not in data_starts
            and key not in bss_starts
            and not ghidra_bss
            and not (row.storage == "bss" and bss_interior_evidence(row))
        ):
            raise ValueError(
                f"{data_path}: identity lacks structural/BSS evidence {key!r}"
            )
        if (
            row.confidence not in CONFIDENCE
            or row.scope not in SCOPES
            or row.storage not in {"load", "bss"}
            or row.size <= 0
        ):
            raise ValueError(f"{data_path}: invalid confidence/scope at {key!r}")
        if key in data_starts:
            structural = structural_data_by_start[key]
            if (
                row.size != parse_int(structural["size"])
                or (row.storage == "bss" and structural["kind"] != "bss")
            ):
                raise ValueError(
                    f"{data_path}: structural storage/extent differs at {key!r}"
                )
        elif row.storage != "bss":
            raise ValueError(f"{data_path}: BSS identity uses load storage at {key!r}")
        if not row.name:
            raise ValueError(f"{data_path}: empty stable name at {key!r}")
        if row.confidence == "address-only" and row.name != f"DAT_{row.va:08x}":
            raise ValueError(
                f"{data_path}: address-only row has semantic-looking name at {key!r}"
            )
        for field, value in (("name", row.name), ("owner", row.owner)):
            _check_identifier(data_path, field, value, key)
        name_key = row.image, row.name
        if name_key in data_names:
            raise ValueError(f"{data_path}: duplicate image-qualified name {name_key!r}")
        data_names.add(name_key)
        if row.storage == "bss":
            bss_by_image.setdefault(row.image, []).append(row)
    for image, rows in bss_by_image.items():
        rows.sort(key=lambda row: row.va)
        for previous, current in zip(rows, rows[1:]):
            if previous.va + previous.size > current.va:
                raise ValueError(
                    f"{data_path}: overlapping BSS identities in {image}: "
                    f"{previous.name!r} and {current.name!r}"
                )
    return {
        "functions": len(functions),
        "functions_named": sum(not row.unresolved for row in functions.values()),
        "signatures_started": sum(row.signature_confidence != "address-only" for row in functions.values()),
        "typed_returns": sum(row.return_type not in {"", "unknown"} for row in functions.values()),
        "parameterized": sum(bool(row.parameters) for row in functions.values()),
        "data": len(data),
        "data_named": sum(row.confidence != "address-only" for row in data.values()),
        **structure_counts,
    }


def _register_use(word: int) -> tuple[set[int], set[int]]:
    """Return registers read/written by the common integer MIPS-I forms."""
    op = word >> 26
    rs = (word >> 21) & 31
    rt = (word >> 16) & 31
    rd = (word >> 11) & 31
    funct = word & 63
    if word == 0:
        return set(), set()
    if op == 0:
        if funct in {0, 2, 3}:  # immediate shifts
            return {rt}, {rd}
        if funct in {8}:  # jr
            return {rs}, set()
        if funct in {9}:  # jalr
            return {rs}, {rd or 31}
        if funct in {16, 18}:  # mfhi/mflo
            return set(), {rd}
        if funct in {17, 19}:  # mthi/mtlo
            return {rs}, set()
        if funct in {24, 25, 26, 27}:  # mult/div
            return {rs, rt}, set()
        return {rs, rt}, {rd}
    if op in {2, 3, 15}:  # j, jal, lui
        return set(), ({31} if op == 3 else ({rt} if op == 15 else set()))
    if op in {4, 5}:
        return {rs, rt}, set()
    if op in {6, 7} or op == 1:
        return {rs}, set()
    if 32 <= op <= 38 or op in {48, 49, 50, 51, 54, 55}:
        return {rs}, {rt}
    if 40 <= op <= 46 or op in {56, 57, 58, 59, 62, 63}:
        return {rs, rt}, set()
    return {rs}, {rt}


def _signature_hints(payload: bytes) -> tuple[str, str, str]:
    live_args: set[int] = set()
    written: set[int] = set()
    pointer_args: set[int] = set()
    return_written = False
    calls = 0
    branches = 0
    loads = 0
    stores = 0
    for offset in range(0, len(payload) - 3, 4):
        word = int.from_bytes(payload[offset:offset + 4], "little")
        op = word >> 26
        reads, writes = _register_use(word)
        live_args.update(reg for reg in reads if 4 <= reg <= 7 and reg not in written)
        written.update(writes)
        return_written |= 2 in writes
        rs = (word >> 21) & 31
        if 32 <= op <= 46 and 4 <= rs <= 7:
            pointer_args.add(rs)
        calls += op == 3 or (op == 0 and (word & 63) == 9)
        branches += op in {1, 2, 4, 5, 6, 7}
        loads += 32 <= op <= 38
        stores += 40 <= op <= 46
    highest = max(live_args, default=3)
    parameters = []
    for reg in range(4, highest + 1):
        position = reg - 4
        if reg in pointer_args:
            name = "object" if position == 0 else f"pointer{position}"
            parameters.append(f"unknown *{name}")
        else:
            parameters.append(f"unknown arg{position}")
    shape = f"calls={calls};branches={branches};loads={loads};stores={stores}"
    return ";".join(parameters), "unknown" if return_written else "void", shape


def _joined_names(values: list[str], *, limit: int = 8) -> str:
    unique = list(dict.fromkeys(value for value in values if value))
    shown = unique[:limit]
    if len(unique) > limit:
        shown.append(f"+{len(unique) - limit}")
    return ";".join(shown)


def seed(config_dir: Path = RETAIL_CONFIG) -> tuple[Path, Path]:
    """Initialize the hand-owned inventories once, refusing any overwrite."""
    function_path = config_dir / "function_identities.tsv"
    data_path = config_dir / "data_identities.tsv"
    for path in (function_path, data_path):
        if path.exists():
            raise ValueError(f"{path}: refusing to overwrite curated inventory")

    universe = _function_universe(config_dir)
    function_rows = []
    for (image, va), structural in sorted(
        universe.items(), key=lambda item: (IMAGE_ORDER[item[0][0]], item[0][1])
    ):
        body_size = parse_int(structural["body_size"])
        payload = retail(image).require(va, body_size)
        parameters, return_hint, _shape = _signature_hints(payload)
        curated = structural["name"]
        function_rows.append({
            "image": image,
            "va": f"0x{va:08x}",
            "name": curated or f"func_{va:08x}",
            "owner": "",
            "action": "",
            "return_type": return_hint,
            "parameters": parameters,
            "name_confidence": "supported" if curated else "address-only",
            "signature_confidence": "candidate",
            "evidence": (
                f"functions.tsv:{structural['confidence']};mips-live-in-heuristic"
                if curated else "mips-live-in-heuristic"
            ),
            "note": "initial static signature seed; requires call-site review",
        })
    write_tsv(
        function_path,
        FUNCTION_IDENTITY_FIELDS,
        function_rows,
        (
            "MANUALLY MANAGED - semantic WIP identities for all carveable non-vendored functions.",
            "Address-only func_ names are unresolved; candidate signatures require caller/callee review.",
            "Methods use name=owner_action with the owner and action columns populated.",
            "Future analysis writes build/function-inventory proposals and never overwrites this file.",
        ),
    )

    _, structural_data = read_tsv(config_dir / "data.tsv")
    data_rows = []
    for row in structural_data:
        if row["kind"] in {"string", "unclassified"}:
            continue
        image = row["image"]
        va = parse_int(row["va"])
        curated = row["name"]
        data_rows.append({
            "image": image,
            "va": f"0x{va:08x}",
            "size": row["size"],
            "name": curated or f"DAT_{va:08x}",
            "scope": "global" if curated else "unknown",
            "storage": "load",
            "datatype": row["datatype"],
            "owner": "",
            "confidence": "supported" if curated else "address-only",
            "evidence": f"data.tsv:{row['confidence']}",
            "note": "initial data-identity seed" if curated else "unresolved global/static candidate",
        })
    existing = {(row["image"], row["va"]) for row in data_rows}
    _, relocations = read_tsv(config_dir / "relocs.tsv")
    for row in relocations:
        if (
            row["target_region"] != "bss"
            or row["status"] == "rejected"
            or re.fullmatch(r"DAT_[0-9A-Fa-f]{8}", row["target_name"]) is None
        ):
            continue
        va = int(row["target_name"][4:], 16)
        key = row["image"], f"0x{va:08x}"
        if key in existing:
            continue
        existing.add(key)
        data_rows.append({
            "image": row["image"],
            "va": key[1],
            "size": "0x4",
            "name": row["target_name"],
            "scope": "unknown",
            "storage": "bss",
            "datatype": "",
            "owner": "",
            "confidence": "address-only",
            "evidence": "relocs.tsv:bss-reference",
            "note": "unresolved referenced BSS global/static candidate",
        })
    data_rows.sort(key=lambda row: (IMAGE_ORDER[str(row["image"])], parse_int(str(row["va"]))))
    write_tsv(
        data_path,
        DATA_IDENTITY_FIELDS,
        data_rows,
        (
            "MANUALLY MANAGED - semantic WIP identities for defined/pointer/global data rows.",
            "Strings retain their decoded content in data.tsv; unclassified coverage gaps are excluded.",
            "Address-only DAT_ names and unknown scope are unresolved, not semantic claims.",
            "Future analysis writes build/function-inventory proposals and never overwrites this file.",
        ),
    )
    return function_path, data_path


def propose(config_dir: Path = RETAIL_CONFIG, output: Path | None = None) -> Path:
    from scripts.kf.sema.evidence import Evidence
    from scripts.kf.sema.index import Index
    from scripts.kf.sema.strings import decode_string

    output = output or BUILD / "function-inventory" / "evidence.tsv"
    identities = load_function_identities(config_dir, required=True)
    _, relocation_rows = read_tsv(config_dir / "relocs.tsv")
    rows = []
    for image in IMAGE_LAYOUTS:
        idx = Index(image, config_dir)
        img = retail(image)
        graph = Evidence(
            image,
            idx=idx,
            img=img,
            rows=[row for row in relocation_rows if row["image"] == image],
        )
        for binding in idx.functions:
            identity = identities.get((image, binding.va))
            if identity is None:
                continue
            payload = img.require(binding.va, binding.body_size)
            parameters, return_hint, shape = _signature_hints(payload)
            callers = []
            for reference in graph.incoming(binding, confirmed_only=True):
                owner = idx.function_owner(reference.site)
                if (
                    owner is not None
                    and owner.va != binding.va
                    and reference.kind in {"call", "tail"}
                ):
                    callers.append(owner.name)
            callees = []
            indirect = 0
            data_refs = []
            string_refs = []
            for reference in graph.outgoing(binding, confirmed_only=True):
                if reference.kind == "indirect-call":
                    indirect += 1
                    continue
                if reference.target is None:
                    continue
                target_function = idx.function(reference.target)
                if target_function is not None and reference.kind in {"call", "tail"}:
                    callees.append(target_function.name)
                    continue
                datum = idx.data_owner(reference.destination)
                if datum is None:
                    continue
                if datum.kind == "string":
                    try:
                        text = decode_string(type("Ctx", (), {"img": img})(), datum).text
                    except (UnicodeError, ValueError):
                        text = datum.name
                    string_refs.append(text.replace("\t", " ").replace("\n", "\\n"))
                else:
                    data_refs.append(datum.name)
            rows.append({
                "image": image,
                "va": f"0x{binding.va:08x}",
                "size": f"0x{binding.body_size:x}",
                "name": identity.name,
                "current_return_type": identity.return_type,
                "current_parameters": identity.parameters,
                "mips_parameters_hint": parameters,
                "mips_return_hint": return_hint,
                "callers": _joined_names(callers),
                "callees": _joined_names(callees),
                "indirect_calls": indirect,
                "strings": _joined_names(string_refs, limit=5),
                "data_references": _joined_names(data_refs),
                "shape": shape,
            })
    write_tsv(
        output,
        EVIDENCE_FIELDS,
        rows,
        (
            "GENERATED candidate dossiers; never an authority or direct config input.",
            "Parameter/return hints are conservative MIPS live-in heuristics.",
            "Names, xrefs, strings, and data labels come from curated retail inventories.",
        ),
    )
    return output


def _data_access(img, reference) -> str:
    if reference.kind == "pointer":
        return "initializer"
    if reference.paired_site is None:
        return "address"
    word = img.u32(reference.paired_site)
    if word is None:
        return "address"
    opcode = word >> 26
    if 32 <= opcode <= 38:
        return "read"
    if 40 <= opcode <= 46:
        return "write"
    return "address"


def propose_data(
    config_dir: Path = RETAIL_CONFIG,
    output: Path | None = None,
) -> Path:
    """Write xref-backed scope/owner proposals for every data identity."""
    from scripts.kf.sema.evidence import Evidence
    from scripts.kf.sema.index import Index

    output = output or BUILD / "function-inventory" / "data-evidence.tsv"
    identities = load_data_identities(config_dir)
    _, relocation_rows = read_tsv(config_dir / "relocs.tsv")
    rows = []
    for image in IMAGE_LAYOUTS:
        idx = Index(image, config_dir)
        img = retail(image)
        graph = Evidence(
            image,
            idx=idx,
            img=img,
            rows=[row for row in relocation_rows if row["image"] == image],
        )
        for (identity_image, va), identity in identities.items():
            if identity_image != image:
                continue
            binding = idx.datum(va)
            if binding is None:
                continue
            users: dict[str, set[str]] = {"confirmed": set(), "candidate": set()}
            access = {"read": 0, "write": 0, "address": 0, "initializer": 0}
            for reference in graph.incoming(binding):
                owner = idx.function_owner(reference.site)
                if owner is not None:
                    channel = (
                        "confirmed"
                        if reference.tier in {"proven", "validated"}
                        else "candidate"
                    )
                    users[channel].add(owner.name)
                access[_data_access(img, reference)] += 1
            confirmed_users = sorted(users["confirmed"])
            candidate_users = sorted(users["candidate"] - users["confirmed"])
            scope_hint = ""
            scope_hint_confidence = ""
            owner_hint = ""
            if len(confirmed_users) == 1:
                scope_hint = "function-static"
                scope_hint_confidence = "confirmed-xref"
                owner_hint = confirmed_users[0]
            elif len(confirmed_users) > 1:
                scope_hint = "shared"
                scope_hint_confidence = "confirmed-xref"
            elif len(candidate_users) == 1:
                scope_hint = "function-static"
                scope_hint_confidence = "candidate-xref"
                owner_hint = candidate_users[0]
            elif len(candidate_users) > 1:
                scope_hint = "shared"
                scope_hint_confidence = "candidate-xref"
            rows.append({
                "image": image,
                "va": f"0x{va:08x}",
                "size": f"0x{identity.size:x}",
                "name": identity.name,
                "storage": identity.storage,
                "datatype": identity.datatype,
                "current_scope": identity.scope,
                "current_owner": identity.owner,
                "confirmed_users": _joined_names(confirmed_users, limit=16),
                "candidate_users": _joined_names(candidate_users, limit=16),
                "reads": access["read"],
                "writes": access["write"],
                "addresses": access["address"] + access["initializer"],
                "scope_hint": scope_hint,
                "scope_hint_confidence": scope_hint_confidence,
                "owner_hint": owner_hint,
            })
    write_tsv(
        output,
        DATA_EVIDENCE_FIELDS,
        rows,
        (
            "GENERATED xref-backed global/static dossiers; never a direct config input.",
            "A single confirmed user is only a function-static hint, not proof of linkage scope.",
            "Read/write classification comes from the paired MIPS low instruction opcode.",
        ),
    )
    return output


def propose_data_relocations(
    config_dir: Path = RETAIL_CONFIG,
    output: Path | None = None,
) -> Path:
    """Propose LUI/low pairs that refer to admitted BSS identities."""
    from scripts.kf.sema.index import Index

    output = output or BUILD / "function-inventory" / "relocs-bss.tsv"
    _, existing = read_tsv(config_dir / "relocs.tsv")
    keys = {
        (
            row["image"],
            parse_int(row["site_va"]),
            parse_int(row["paired_site_va"]) if row["paired_site_va"] else None,
            parse_int(row["target_va"]),
        )
        for row in existing
    }
    additions = []
    universe = _function_universe(config_dir)
    caller_saved = {*range(1, 16), 24, 25}
    for image, layout in IMAGE_LAYOUTS.items():
        idx = Index(image, config_dir)
        img = retail(image)
        for (function_image, va), structural in universe.items():
            if function_image != image:
                continue
            body_size = parse_int(structural["body_size"])
            payload = img.require(va, body_size)
            words = [
                int.from_bytes(payload[offset:offset + 4], "little")
                for offset in range(0, len(payload) - 3, 4)
            ]
            for position, hi_word in enumerate(words):
                if hi_word >> 26 != 0x0F:
                    continue
                register = (hi_word >> 16) & 31
                for low_position in range(position + 1, min(position + 33, len(words))):
                    low_word = words[low_position]
                    low_opcode = low_word >> 26
                    if (
                        low_opcode in LOW_MNEMONICS
                        and (low_word >> 21) & 31 == register
                    ):
                        try:
                            target = decode_hi_lo_target(hi_word, low_word)
                        except ValueError:
                            target = -1
                        datum = idx.data_owner(target) if target >= 0 else None
                        if datum is not None and datum.kind == "bss":
                            hi_va = va + position * 4
                            low_va = va + low_position * 4
                            key = image, hi_va, low_va, target
                            if key not in keys:
                                keys.add(key)
                                additions.append({
                                    "image": image,
                                    "site_va": f"0x{hi_va:08x}",
                                    "site_file_offset": f"0x{layout.file_offset(hi_va):x}",
                                    "paired_site_va": f"0x{low_va:08x}",
                                    "kind": "mips_hi16_lo16",
                                    "channel": "reachable-code",
                                    "target_va": f"0x{target:08x}",
                                    "target_region": "bss",
                                    "target_name": datum.link_name,
                                    "opcode": f"lui+{LOW_MNEMONICS[low_opcode]}",
                                    "register": REGISTER_NAMES[(low_word >> 16) & 31],
                                    "confidence": "mips-hi-lo-data-pair",
                                    "status": "candidate",
                                    "provenance": "scripts/kf/inventory.py:bss-pair-scan",
                                })
                    _reads, writes = _register_use(low_word)
                    if register in writes:
                        break
                    if (
                        register in caller_saved
                        and (
                            low_opcode == 3
                            or low_opcode == 0 and (low_word & 63) == 9
                        )
                    ):
                        break
    rows = [*existing, *additions]
    rows.sort(key=lambda row: (
        IMAGE_ORDER[row["image"]],
        parse_int(row["site_va"]),
        parse_int(row["paired_site_va"]) if row["paired_site_va"] else -1,
        row["kind"],
        parse_int(row["target_va"]),
    ))
    write_tsv(
        output,
        RELOC_FIELDS,
        rows,
        (
            "GENERATED BSS-relocation proposal; review before copying changes into config/retail.",
            "New rows are decoded LUI/low pairs targeting an admitted BSS extent.",
            "Rows remain candidates and safe delinking withholds out-of-load targets.",
        ),
    )
    return output


GHIDRA_TYPE_MAP = {
    "undefined": "unknown",
    "undefined1": "u8",
    "undefined2": "u16",
    "undefined4": "u32",
    "byte": "u8",
    "char": "char",
    "short": "s16",
    "ushort": "u16",
    "int": "s32",
    "uint": "u32",
    "bool": "bool",
    "void": "void",
}


def _ghidra_type(value: str) -> str:
    base = value.strip()
    pointers = 0
    while base.endswith("*"):
        pointers += 1
        base = base[:-1].strip()
    if pointers and base in {"undefined", "undefined4"}:
        mapped = "void" if pointers == 1 else "unknown"
    else:
        mapped = GHIDRA_TYPE_MAP.get(base, base)
    return mapped + " " + "*" * pointers if pointers else mapped


def _parameter_name(value: str) -> str:
    _type_name, separator, name = value.rpartition(" ")
    return name.lstrip("*") if separator else ""


def propose_ghidra_signatures(
    config_dir: Path = RETAIL_CONFIG,
    input_dir: Path | None = None,
    output: Path | None = None,
) -> Path:
    """Merge Ghidra's inferred prototypes into a review-only identity TSV."""
    input_dir = input_dir or BUILD / "ghidra-inventory"
    output = output or BUILD / "function-inventory" / "ghidra-signatures.tsv"
    identities = load_function_identities(config_dir, required=True)
    candidates: dict[tuple[str, int], dict[str, object]] = {}
    for image in IMAGE_LAYOUTS:
        path = input_dir / image.lower().removesuffix(".exe") / "candidates.json"
        if not path.is_file():
            raise ValueError(f"{path}: run 'kf inventory ghidra' first")
        document = json.loads(path.read_text(encoding="utf-8"))
        if document.get("schema") != "kf-ghidra-inventory-v1":
            raise ValueError(f"{path}: unsupported Ghidra inventory schema")
        for row in document["rows"]:
            candidates[(image, int(row["va"]))] = row

    rows = []
    for key, identity in identities.items():
        candidate = candidates.get(key)
        return_type = identity.return_type
        parameters = identity.parameters
        evidence = identity.evidence
        note = identity.note
        if (
            candidate is not None
            and candidate["status"] == "decompiled"
            and identity.signature_confidence in {"address-only", "candidate"}
        ):
            variadic = identity.parameters.endswith(";...")
            current_parameters = [
                (_parameter_name(value), "*" in value.rpartition(" ")[0])
                for value in identity.parameters.split(";")
                if value and value != "..."
            ]
            proposed_parameters = []
            for parameter in candidate["parameters"]:
                ordinal = int(parameter["ordinal"])
                datatype = str(parameter["datatype"])
                current_name, current_pointer = (
                    current_parameters[ordinal]
                    if ordinal < len(current_parameters)
                    else ("", False)
                )
                name = (
                    current_name
                    if current_name
                    and (
                        current_name.startswith("arg")
                        or current_pointer == ("*" in datatype)
                    )
                    else f"arg{ordinal}"
                )
                proposed_parameters.append(
                    f"{_ghidra_type(datatype)} {name}"
                )
            if variadic:
                proposed_parameters.append("...")
            parameters = ";".join(proposed_parameters)
            return_type = _ghidra_type(str(candidate["return_type"]))
            channel = "ghidra-12.0.4-decompiler-candidate"
            evidence = ";".join(dict.fromkeys(filter(
                None,
                (*identity.evidence.split(";"), channel),
            )))
            note = "Ghidra/static signature seed; requires caller and body review"
        rows.append({
            "image": identity.image,
            "va": f"0x{identity.va:08x}",
            "name": identity.name,
            "owner": identity.owner,
            "action": identity.action,
            "return_type": return_type,
            "parameters": parameters,
            "name_confidence": identity.name_confidence,
            "signature_confidence": identity.signature_confidence,
            "evidence": evidence,
            "note": note,
        })
    write_tsv(
        output,
        FUNCTION_IDENTITY_FIELDS,
        rows,
        (
            "GENERATED signature proposal; review before copying changes into config/retail.",
            "Supported/proven signatures are preserved; only address-only/candidate rows change.",
            "Ghidra undefined-width integers map to candidate fixed-width project types.",
        ),
    )
    return output


def _target_region(idx, target: int) -> tuple[str, str, str]:
    binding = idx.data_owner(target) or idx.function_owner(target)
    if binding is not None:
        return binding.name, f"0x{target - binding.va:x}", binding.kind
    if 0x1F800000 <= target < 0x1F800400:
        return "", "", "scratchpad"
    if 0x1F801000 <= target < 0x1F803000:
        return "", "", "hardware"
    if 0x80000000 <= target < 0x80200000:
        return "", "", "unowned-ram"
    return "", "", "other"


def propose_ghidra_data_references(
    config_dir: Path = RETAIL_CONFIG,
    input_dir: Path | None = None,
    output: Path | None = None,
) -> Path:
    """Aggregate Ghidra memory xrefs to expose unowned globals and MMIO."""
    from scripts.kf.sema.index import Index

    input_dir = input_dir or BUILD / "ghidra-inventory"
    output = output or BUILD / "function-inventory" / "ghidra-data-xrefs.tsv"
    _, relocation_rows = read_tsv(config_dir / "relocs.tsv")
    admitted = {
        (row["image"], site, parse_int(row["target_va"]))
        for row in relocation_rows
        if row["status"] != "rejected"
        for site in (
            parse_int(row["site_va"]),
            parse_int(row["paired_site_va"]) if row["paired_site_va"] else -1,
        )
    }
    rows = []
    for image in IMAGE_LAYOUTS:
        path = input_dir / image.lower().removesuffix(".exe") / "candidates.json"
        if not path.is_file():
            raise ValueError(f"{path}: run 'kf inventory ghidra' first")
        document = json.loads(path.read_text(encoding="utf-8"))
        idx = Index(image, config_dir)
        groups: dict[int, dict[str, object]] = {}
        for function in document["rows"]:
            for reference in function.get("data_references", []):
                target = int(reference["target"])
                group = groups.setdefault(target, {
                    "names": set(),
                    "types": set(),
                    "users": set(),
                    "sites": set(),
                })
                if reference.get("target_name"):
                    group["names"].add(str(reference["target_name"]))
                group["types"].add(str(reference["type"]))
                group["users"].add(str(function["inventory_name"]))
                group["sites"].add(int(reference["site"]))
        for target, group in sorted(groups.items()):
            binding, offset, region = _target_region(idx, target)
            sites = sorted(group["sites"])
            rows.append({
                "image": image,
                "target_va": f"0x{target:08x}",
                "ghidra_name": _joined_names(sorted(group["names"]), limit=4),
                "binding": binding,
                "binding_offset": offset,
                "region": region,
                "reference_types": _joined_names(sorted(group["types"])),
                "users": _joined_names(sorted(group["users"]), limit=16),
                "sites": len(sites),
                "admitted_sites": sum(
                    (image, site, target) in admitted for site in sites
                ),
            })
    write_tsv(
        output,
        GHIDRA_DATA_XREF_FIELDS,
        rows,
        (
            "GENERATED Ghidra non-flow memory-reference aggregate; never a config input.",
            "Unowned RAM rows prioritize missing global/static extents or relocations.",
            "Scratchpad/hardware rows are addresses, not C storage identities.",
        ),
    )
    return output


def _ghidra_access_size(reference: dict[str, object]) -> int:
    mnemonic = str(reference.get("mnemonic") or "").upper()
    access_size = 1
    if mnemonic in {"LB", "LBU", "SB"}:
        access_size = 1
    elif mnemonic in {"LH", "LHU", "SH"}:
        access_size = 2
    elif mnemonic in {
        "LW", "LWL", "LWR", "SW", "SWL", "SWR", "LWC2", "SWC2",
    }:
        access_size = 4
    return max(access_size, int(reference.get("data_size") or 1))


def _candidate_datatype(size: int) -> str:
    return {1: "u8", 2: "u16", 4: "u32"}.get(size, f"u8[0x{size:x}]")


def propose_ghidra_data_identities(
    config_dir: Path = RETAIL_CONFIG,
    input_dir: Path | None = None,
    output: Path | None = None,
) -> Path:
    """Merge non-overlapping Ghidra-referenced RAM extents into a review TSV."""
    input_dir = input_dir or BUILD / "ghidra-inventory"
    output = output or BUILD / "function-inventory" / "data-identities.ghidra.tsv"
    identities = load_data_identities(config_dir)
    proposed = list(identities.values())
    for image, layout in IMAGE_LAYOUTS.items():
        path = input_dir / image.lower().removesuffix(".exe") / "candidates.json"
        if not path.is_file():
            raise ValueError(f"{path}: run 'kf inventory ghidra' first")
        document = json.loads(path.read_text(encoding="utf-8"))
        existing = [
            (row.va, row.va + row.size)
            for row in proposed
            if row.image == image
        ]
        intervals = set()
        for function in document["rows"]:
            for reference in function.get("data_references", []):
                if reference["type"] not in {"READ", "WRITE", "READ_WRITE"}:
                    continue
                target = int(reference["target"])
                start = int(reference.get("data_start") or target)
                size = _ghidra_access_size(reference)
                end = start + size
                if (
                    start < layout.load_end
                    or not (0x80000000 <= start < end <= 0x80200000)
                    or any(start < old_end and old_start < end for old_start, old_end in existing)
                ):
                    continue
                intervals.add((start, end))
        merged = []
        for start, end in sorted(intervals):
            if merged and start < merged[-1][1]:
                merged[-1] = (merged[-1][0], max(merged[-1][1], end))
            else:
                merged.append((start, end))
        for start, end in merged:
            size = end - start
            proposed.append(DataIdentity(
                image=image,
                va=start,
                size=size,
                name=f"DAT_{start:08x}",
                scope="unknown",
                storage="bss",
                datatype=_candidate_datatype(size),
                owner="",
                confidence="address-only",
                evidence="ghidra-12.0.4-memory-xref",
                note="unresolved referenced RAM global/static candidate",
            ))
    rows = [
        {
            "image": row.image,
            "va": f"0x{row.va:08x}",
            "size": f"0x{row.size:x}",
            "name": row.name,
            "scope": row.scope,
            "storage": row.storage,
            "datatype": row.datatype,
            "owner": row.owner,
            "confidence": row.confidence,
            "evidence": row.evidence,
            "note": row.note,
        }
        for row in sorted(
            proposed,
            key=lambda row: (IMAGE_ORDER[row.image], row.va),
        )
    ]
    write_tsv(
        output,
        DATA_IDENTITY_FIELDS,
        rows,
        (
            "GENERATED data-identity proposal; review before copying changes into config/retail.",
            "Ghidra-referenced RAM extents are address-only BSS candidates, not semantic names.",
            "Overlapping access extents are merged; admitted load/BSS objects are preserved.",
        ),
    )
    return output


def _print_counts(counts: dict[str, int]) -> None:
    print("function/global inventory valid: " + ", ".join(
        f"{name}={count}" for name, count in counts.items()
    ))


def main(argv: list[str] | None = None) -> int:
    raw = list(sys.argv[1:] if argv is None else argv)
    if raw and raw[0] == "ghidra":
        from scripts.kf.ghidra_inventory import main as ghidra_main

        return ghidra_main(raw[1:])
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    seed_parser = subparsers.add_parser("seed")
    seed_parser.add_argument("--config-dir", type=Path, default=RETAIL_CONFIG)
    check_parser = subparsers.add_parser("check")
    check_parser.add_argument("--config-dir", type=Path, default=RETAIL_CONFIG)
    propose_parser = subparsers.add_parser("propose")
    propose_parser.add_argument("--config-dir", type=Path, default=RETAIL_CONFIG)
    propose_parser.add_argument("--output", type=Path)
    propose_parser.add_argument("--data-output", type=Path)
    propose_parser.add_argument("--reloc-output", type=Path)
    ghidra_propose = subparsers.add_parser("propose-ghidra")
    ghidra_propose.add_argument("--config-dir", type=Path, default=RETAIL_CONFIG)
    ghidra_propose.add_argument("--input-dir", type=Path)
    ghidra_propose.add_argument("--output", type=Path)
    ghidra_propose.add_argument("--data-output", type=Path)
    ghidra_propose.add_argument("--identity-output", type=Path)
    subparsers.add_parser("ghidra", add_help=False)
    args = parser.parse_args(raw)
    if args.command == "seed":
        print("\n".join(str(path) for path in seed(args.config_dir)))
    elif args.command == "check":
        _print_counts(validate(args.config_dir))
    elif args.command == "propose-ghidra":
        print(propose_ghidra_signatures(
            args.config_dir,
            args.input_dir,
            args.output,
        ))
        print(propose_ghidra_data_references(
            args.config_dir,
            args.input_dir,
            args.data_output,
        ))
        print(propose_ghidra_data_identities(
            args.config_dir,
            args.input_dir,
            args.identity_output,
        ))
    else:
        print(propose(args.config_dir, args.output))
        print(propose_data(args.config_dir, args.data_output))
        print(propose_data_relocations(args.config_dir, args.reloc_output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
