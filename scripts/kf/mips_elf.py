"""Minimal deterministic ELF32 little-endian MIPS relocatable-object writer.

The King's Field delinker needs only a small subset of ELF: one executable
``.text`` section, function/undefined symbols, and MIPS REL relocations.  Keeping
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
SHT_REL = 9

SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4

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


def write_mips_elf(
    text: bytes,
    function_name: str,
    function_size: int,
    relocations: Iterable[MipsRelocation] = (),
    defined_symbols: Iterable[DefinedSymbol] = (),
) -> bytes:
    """Build a deterministic MIPS-I/O32 relocatable object.

    Relocation addends are implicit in ``text`` because MIPS ELF uses REL, not
    RELA. Callers must rewrite relocated instruction fields before invoking the
    writer. ``function_size`` may be smaller than the section when the retail
    extent contains linker padding after the body.
    """
    if not function_name:
        raise ValueError("function name must not be empty")
    if not 0 < function_size <= len(text):
        raise ValueError("function size must lie within .text")

    relocations = tuple(sorted(relocations, key=lambda item: (item.offset, item.kind)))
    defined_symbols = tuple(sorted(defined_symbols, key=lambda item: (item.value, item.name)))
    for relocation in relocations:
        if relocation.kind not in RELOCATION_TYPES:
            raise ValueError(f"unsupported relocation {relocation.kind!r}")
        if not 0 <= relocation.offset <= len(text) - 4 or relocation.offset & 3:
            raise ValueError(f"invalid relocation offset {relocation.offset:#x}")
    for symbol in defined_symbols:
        if not 0 <= symbol.value <= len(text):
            raise ValueError(f"defined symbol {symbol.name!r} lies outside .text")
        if symbol.size < 0 or symbol.value + symbol.size > len(text):
            raise ValueError(f"defined symbol {symbol.name!r} has invalid size")

    defined_names = {function_name, *(symbol.name for symbol in defined_symbols)}
    if SECTION_SYMBOL in defined_names:
        raise ValueError(f"{SECTION_SYMBOL!r} is reserved for the section symbol")
    if len(defined_names) != len(defined_symbols) + 1:
        raise ValueError("defined symbol names must be unique")
    undefined_names = sorted(
        {relocation.symbol for relocation in relocations}
        - defined_names
        - {SECTION_SYMBOL}
    )
    global_names = [
        function_name,
        *(symbol.name for symbol in defined_symbols if symbol.name != function_name),
        *undefined_names,
    ]
    strtab, string_offsets = _string_table(global_names)

    # Symbol 0 is null; symbol 1 is the sole local section symbol. All remaining
    # symbols are global so objdiff can pair them with compiler-produced names.
    symbol_records = [
        b"\0" * SYMBOL_SIZE,
        _symbol(0, 0, 0, STB_LOCAL, STT_SECTION, 1),
        _symbol(string_offsets[function_name], 0, function_size, STB_GLOBAL, STT_FUNC, 1),
    ]
    symbol_indices = {SECTION_SYMBOL: 1, function_name: 2}
    for symbol in defined_symbols:
        if symbol.name == function_name:
            continue
        symbol_indices[symbol.name] = len(symbol_records)
        symbol_records.append(_symbol(
            string_offsets[symbol.name],
            symbol.value,
            symbol.size,
            STB_GLOBAL,
            symbol.kind,
            1,
        ))
    for name in undefined_names:
        symbol_indices[name] = len(symbol_records)
        symbol_records.append(
            _symbol(string_offsets[name], 0, 0, STB_GLOBAL, STT_NOTYPE, SHN_UNDEF)
        )
    symtab = b"".join(symbol_records)

    rel_text = b"".join(
        struct.pack(
            "<II",
            relocation.offset,
            (symbol_indices[relocation.symbol] << 8)
            | RELOCATION_TYPES[relocation.kind],
        )
        for relocation in relocations
    )

    section_names = (".text", ".rel.text", ".symtab", ".strtab", ".shstrtab")
    shstrtab, section_name_offsets = _string_table(section_names)
    sections = (
        _Section(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, text, alignment=4),
        _Section(
            ".rel.text",
            SHT_REL,
            0,
            rel_text,
            link=3,
            info=1,
            alignment=4,
            entry_size=REL_SIZE,
        ),
        _Section(
            ".symtab",
            SHT_SYMTAB,
            0,
            symtab,
            link=4,
            info=2,
            alignment=4,
            entry_size=SYMBOL_SIZE,
        ),
        _Section(".strtab", SHT_STRTAB, 0, strtab),
        _Section(".shstrtab", SHT_STRTAB, 0, shstrtab),
    )

    data = bytearray(b"\0" * ELF_HEADER_SIZE)
    section_offsets: list[int] = []
    for section in sections:
        offset = _align(len(data), section.alignment)
        data.extend(b"\0" * (offset - len(data)))
        section_offsets.append(offset)
        data.extend(section.data)
    section_header_offset = _align(len(data), 4)
    data.extend(b"\0" * (section_header_offset - len(data)))

    # Null section header.
    data.extend(b"\0" * SECTION_HEADER_SIZE)
    for section, offset in zip(sections, section_offsets, strict=True):
        data.extend(struct.pack(
            "<IIIIIIIIII",
            section_name_offsets[section.name],
            section.section_type,
            section.flags,
            0,
            offset,
            len(section.data),
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
        5,
    )
    data[:ELF_HEADER_SIZE] = header
    return bytes(data)
