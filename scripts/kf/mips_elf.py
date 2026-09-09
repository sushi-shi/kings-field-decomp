"""Minimal deterministic ELF32 little-endian MIPS relocatable-object writer.

The King's Field delinker needs only a small subset of ELF: one executable
``.text`` section, optional ``.data``/``.bss`` sections for the data a module
claims, function/object/undefined symbols, and MIPS REL relocations.  Keeping
this writer in-tree makes target objects independent of host assembler quirks;
reconstructed source objects still come from CC1PSX -> maspsx -> GNU ``as``.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Iterable


ELF_HEADER_SIZE = 52
SECTION_HEADER_SIZE = 40
SYMBOL_SIZE = 16
REL_SIZE = 8

EM_MIPS = 8
ET_REL = 1
EV_CURRENT = 1

SHT_NULL = 0
SHT_PROGBITS = 1
SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_NOBITS = 8
SHT_REL = 9

SHF_WRITE = 0x1
SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4
SHF_MIPS_GPREL = 0x10000000

STB_LOCAL = 0
STB_GLOBAL = 1
STT_NOTYPE = 0
STT_OBJECT = 1
STT_FUNC = 2
STT_SECTION = 3

SHN_UNDEF = 0

R_MIPS_32 = 2
R_MIPS_26 = 4
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6
R_MIPS_GPREL16 = 7

RELOCATION_TYPES = {
    "R_MIPS_32": R_MIPS_32,
    "R_MIPS_26": R_MIPS_26,
    "R_MIPS_HI16": R_MIPS_HI16,
    "R_MIPS_LO16": R_MIPS_LO16,
    "R_MIPS_GPREL16": R_MIPS_GPREL16,
}
SECTION_SYMBOL = ".text"
DATA_SECTION_SYMBOL = ".data"
BSS_SECTION_SYMBOL = ".bss"
RODATA_SECTION_SYMBOL = ".rodata"


@dataclass(frozen=True)
class MipsRelocation:
    offset: int
    kind: str
    symbol: str


@dataclass(frozen=True)
class DefinedSymbol:
    name: str
    value: int
    size: int = 0
    kind: int = STT_NOTYPE
    binding: int = STB_GLOBAL


@dataclass(frozen=True)
class _Section:
    name: str
    section_type: int
    flags: int
    data: bytes
    link: int = 0
    info: int = 0
    alignment: int = 1
    entry_size: int = 0


def _align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & -alignment


def _string_table(strings: Iterable[str]) -> tuple[bytes, dict[str, int]]:
    data = bytearray(b"\0")
    offsets = {"": 0}
    for value in strings:
        if value in offsets:
            continue
        offsets[value] = len(data)
        data.extend(value.encode("utf-8"))
        data.append(0)
    return bytes(data), offsets


def _symbol(
    name_offset: int,
    value: int,
    size: int,
    binding: int,
    kind: int,
    section: int,
) -> bytes:
    return struct.pack(
        "<IIIBBH",
        name_offset,
        value,
        size,
        (binding << 4) | kind,
        0,
        section,
    )


def _check_relocations(
    relocations: Iterable[MipsRelocation], section_size: int, section: str
) -> tuple[MipsRelocation, ...]:
    ordered = tuple(sorted(relocations, key=lambda item: (item.offset, item.kind)))
    for relocation in ordered:
        if relocation.kind not in RELOCATION_TYPES:
            raise ValueError(f"unsupported relocation {relocation.kind!r}")
        if not 0 <= relocation.offset <= section_size - 4 or relocation.offset & 3:
            raise ValueError(f"invalid {section} relocation offset {relocation.offset:#x}")
    return ordered


def _check_symbols(
    symbols: Iterable[DefinedSymbol], section_size: int, section: str
) -> tuple[DefinedSymbol, ...]:
    ordered = tuple(sorted(symbols, key=lambda item: (item.value, item.name)))
    for symbol in ordered:
        if not 0 <= symbol.value <= section_size:
            raise ValueError(f"defined symbol {symbol.name!r} lies outside {section}")
        if symbol.size < 0 or symbol.value + symbol.size > section_size:
            raise ValueError(f"defined symbol {symbol.name!r} has invalid size")
        if symbol.binding not in (STB_LOCAL, STB_GLOBAL):
            raise ValueError(f"defined symbol {symbol.name!r} has invalid binding")
    return ordered


def write_mips_elf(
    text: bytes,
    function_name: str | None,
    function_size: int,
    relocations: Iterable[MipsRelocation] = (),
    defined_symbols: Iterable[DefinedSymbol] = (),
    *,
    data: bytes = b"",
    data_symbols: Iterable[DefinedSymbol] = (),
    data_relocations: Iterable[MipsRelocation] = (),
    data_alignment: int = 4,
    sdata: bytes = b"",
    sdata_symbols: Iterable[DefinedSymbol] = (),
    sdata_relocations: Iterable[MipsRelocation] = (),
    sdata_alignment: int = 4,
    bss_size: int = 0,
    bss_symbols: Iterable[DefinedSymbol] = (),
    bss_alignment: int = 4,
    rodata: bytes = b"",
    rodata_relocations: Iterable[MipsRelocation] = (),
) -> bytes:
    """Build a deterministic MIPS-I/O32 relocatable object.

    Relocation addends are implicit in ``text``/``data`` because MIPS ELF uses
    REL, not RELA. Callers must rewrite relocated fields before invoking the
    writer. ``function_size`` may be smaller than the section when the retail
    extent contains linker padding after the body. ``data``/``bss_size`` and
    their symbols describe the data a module claims; the sections are omitted
    when empty so single-function objects keep their historical shape.
    Data section alignments are placement constraints, not permission to round
    their extents or add owned tail bytes. A data-only object passes None as
    function_name and an empty text/zero function size; no fake function symbol
    is emitted and no code enters the progress denominator.
    """
    if function_name is None:
        if text or function_size:
            raise ValueError("data-only objects cannot own code")
    elif not function_name:
        raise ValueError("function name must not be empty")
    elif not 0 < function_size <= len(text):
        raise ValueError("function size must lie within .text")
    if bss_size < 0:
        raise ValueError("bss size must be non-negative")
    for name, alignment in ((".data", data_alignment), (".sdata", sdata_alignment),
                            (".bss", bss_alignment)):
        if not 0 < alignment <= 0x80000000 or alignment & (alignment - 1):
            raise ValueError(f"{name} alignment must be a positive ELF32 power of two")

    relocations = _check_relocations(relocations, len(text), ".text")
    defined_symbols = _check_symbols(defined_symbols, len(text), ".text")
    data_relocations = _check_relocations(data_relocations, len(data), ".data")
    data_symbols = _check_symbols(data_symbols, len(data), ".data")
    bss_symbols = _check_symbols(bss_symbols, bss_size, ".bss")
    rodata_relocations = _check_relocations(rodata_relocations, len(rodata), ".rodata")
    sdata_relocations = _check_relocations(sdata_relocations, len(sdata), ".sdata")
    sdata_symbols = _check_symbols(sdata_symbols, len(sdata), ".sdata")
    has_sdata = bool(sdata) or bool(sdata_symbols) or bool(sdata_relocations)
    has_data = bool(data) or bool(data_symbols) or bool(data_relocations)
    has_bss = bss_size > 0 or bool(bss_symbols)
    has_rodata = bool(rodata)
    if function_name is None and (defined_symbols or relocations or not (has_data or has_sdata or has_bss or has_rodata)):
        raise ValueError("data-only objects require data and cannot define text symbols/relocations")

    # Section order: .text, .rel.text, [.data, [.rel.data]], [.bss], .symtab,
    # .strtab, .shstrtab. Indices are assigned as the list is built.
    sections: list[_Section] = [
        _Section(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, text, alignment=4),
        _Section(".rel.text", SHT_REL, 0, b"", alignment=4, entry_size=REL_SIZE),
    ]
    text_index = 1
    data_index = 0
    bss_index = 0
    if has_data:
        sections.append(_Section(".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, data,
                                 alignment=data_alignment))
        data_index = len(sections)
        if data_relocations:
            sections.append(
                _Section(".rel.data", SHT_REL, 0, b"", alignment=4, entry_size=REL_SIZE)
            )
    if has_bss:
        sections.append(_Section(".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE, b"",
                                 alignment=bss_alignment))
        bss_index = len(sections)
    rodata_index = 0
    if has_rodata:
        sections.append(_Section(".rodata", SHT_PROGBITS, SHF_ALLOC, rodata, alignment=4))
        rodata_index = len(sections)
        if rodata_relocations:
            sections.append(
                _Section(".rel.rodata", SHT_REL, 0, b"", alignment=4, entry_size=REL_SIZE)
            )
    sdata_index = 0
    if has_sdata:
        sections.append(_Section(".sdata", SHT_PROGBITS,
                                 SHF_ALLOC | SHF_WRITE | SHF_MIPS_GPREL, sdata,
                                 alignment=sdata_alignment))
        sdata_index = len(sections)
        if sdata_relocations:
            sections.append(
                _Section(".rel.sdata", SHT_REL, 0, b"", alignment=4, entry_size=REL_SIZE)
            )
    symtab_index = len(sections) + 1
    strtab_index = symtab_index + 1
    shstrtab_index = strtab_index + 1

    section_symbols = [(SECTION_SYMBOL, text_index)]
    if has_data:
        section_symbols.append((DATA_SECTION_SYMBOL, data_index))
    if has_bss:
        section_symbols.append((BSS_SECTION_SYMBOL, bss_index))
    if has_rodata:
        section_symbols.append((RODATA_SECTION_SYMBOL, rodata_index))
    if has_sdata:
        section_symbols.append((".sdata", sdata_index))
    section_symbol_names = {name for name, _ in section_symbols}

    placed = [
        *([(DefinedSymbol(function_name, 0, function_size, STT_FUNC), text_index)]
          if function_name is not None else []),
        *((symbol, text_index) for symbol in defined_symbols if symbol.name != function_name),
        *((symbol, data_index) for symbol in data_symbols),
        *((symbol, bss_index) for symbol in bss_symbols),
        *((symbol, sdata_index) for symbol in sdata_symbols),
    ]
    defined_names = [symbol.name for symbol, _ in placed]
    if any(name in section_symbol_names for name in defined_names):
        raise ValueError("section symbol names are reserved")
    if len(set(defined_names)) != len(defined_names):
        raise ValueError("defined symbol names must be unique")
    undefined_names = sorted(
        {item.symbol for item in (*relocations, *data_relocations, *sdata_relocations, *rodata_relocations)}
        - set(defined_names)
        - section_symbol_names
    )
    locals_placed = [item for item in placed if item[0].binding == STB_LOCAL]
    globals_placed = [item for item in placed if item[0].binding == STB_GLOBAL]
    strtab, string_offsets = _string_table([
        *(symbol.name for symbol, _ in locals_placed),
        *(symbol.name for symbol, _ in globals_placed),
        *undefined_names,
    ])

    # Symbol 0 is null; section symbols and local data follow; every remaining
    # symbol is global so objdiff can pair them with compiler-produced names.
    symbol_records = [b"\0" * SYMBOL_SIZE]
    symbol_indices: dict[str, int] = {}
    for name, index in section_symbols:
        symbol_indices[name] = len(symbol_records)
        symbol_records.append(_symbol(0, 0, 0, STB_LOCAL, STT_SECTION, index))
    for symbol, index in locals_placed:
        symbol_indices[symbol.name] = len(symbol_records)
        symbol_records.append(_symbol(
            string_offsets[symbol.name], symbol.value, symbol.size, STB_LOCAL, symbol.kind, index,
        ))
    first_global = len(symbol_records)
    for symbol, index in globals_placed:
        symbol_indices[symbol.name] = len(symbol_records)
        symbol_records.append(_symbol(
            string_offsets[symbol.name], symbol.value, symbol.size, STB_GLOBAL, symbol.kind, index,
        ))
    for name in undefined_names:
        symbol_indices[name] = len(symbol_records)
        symbol_records.append(
            _symbol(string_offsets[name], 0, 0, STB_GLOBAL, STT_NOTYPE, SHN_UNDEF)
        )
    symtab = b"".join(symbol_records)

    def rel_section(entries: tuple[MipsRelocation, ...]) -> bytes:
        return b"".join(
            struct.pack(
                "<II",
                relocation.offset,
                (symbol_indices[relocation.symbol] << 8) | RELOCATION_TYPES[relocation.kind],
            )
            for relocation in entries
        )

    resolved: list[_Section] = []
    for section in sections:
        if section.section_type == SHT_REL:
            entries, target_index = {
                ".rel.text": (relocations, text_index),
                ".rel.data": (data_relocations, data_index),
                ".rel.sdata": (sdata_relocations, sdata_index),
                ".rel.rodata": (rodata_relocations, rodata_index),
            }[section.name]
            section = _Section(
                section.name,
                SHT_REL,
                0,
                rel_section(entries),
                link=symtab_index,
                info=target_index,
                alignment=4,
                entry_size=REL_SIZE,
            )
        resolved.append(section)
    sections = resolved
    sections.append(_Section(
        ".symtab", SHT_SYMTAB, 0, symtab, link=strtab_index, info=first_global,
        alignment=4, entry_size=SYMBOL_SIZE,
    ))
    sections.append(_Section(".strtab", SHT_STRTAB, 0, strtab))
    shstrtab, section_name_offsets = _string_table(
        [section.name for section in sections] + [".shstrtab"]
    )
    sections.append(_Section(".shstrtab", SHT_STRTAB, 0, shstrtab))

    blob = bytearray(b"\0" * ELF_HEADER_SIZE)
    section_offsets: list[int] = []
    for section in sections:
        offset = _align(len(blob), section.alignment)
        blob.extend(b"\0" * (offset - len(blob)))
        section_offsets.append(offset)
        if section.section_type != SHT_NOBITS:
            blob.extend(section.data)
    section_header_offset = _align(len(blob), 4)
    blob.extend(b"\0" * (section_header_offset - len(blob)))

    # Null section header.
    blob.extend(b"\0" * SECTION_HEADER_SIZE)
    for section, offset in zip(sections, section_offsets, strict=True):
        size = bss_size if section.section_type == SHT_NOBITS else len(section.data)
        blob.extend(struct.pack(
            "<IIIIIIIIII",
            section_name_offsets[section.name],
            section.section_type,
            section.flags,
            0,
            offset,
            size,
            section.link,
            section.info,
            section.alignment,
            section.entry_size,
        ))

    ident = bytearray(16)
    ident[:7] = b"\x7fELF\x01\x01\x01"  # ELF32, little-endian, current version
    header = struct.pack(
        "<16sHHIIIIIHHHHHH",
        bytes(ident),
        ET_REL,
        EM_MIPS,
        EV_CURRENT,
        0,
        0,
        section_header_offset,
        0x1001,  # O32 ABI, MIPS-I, noreorder
        ELF_HEADER_SIZE,
        0,
        0,
        SECTION_HEADER_SIZE,
        len(sections) + 1,
        shstrtab_index,
    )
    blob[:ELF_HEADER_SIZE] = header
    return bytes(blob)
