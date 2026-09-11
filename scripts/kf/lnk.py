"""Read native Psy-Q LNK v2 objects for inspection; never produce linker inputs.

Record encodings follow the pinned psy-k parser (src/lib.rs). Unsupported
records and relocation expressions fail closed. Only the ELF view translates
relocation addends; the native object and its instruction bytes stay intact.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import struct


@dataclass
class Section:
    name: str
    alignment: int
    data: bytearray = field(default_factory=bytearray)
    patches: list[tuple[int, int, tuple]] = field(default_factory=list)


@dataclass(frozen=True)
class Symbol:
    name: str
    section: int | None
    value: int = 0
    local: bool = False
    common: bool = False


@dataclass
class Object:
    sections: dict[int, Section]
    symbols: dict[int, Symbol]
    locals: list[Symbol]
    functions: dict[str, tuple[int, int, int]] = field(default_factory=dict)
    declared_functions: set[str] = field(default_factory=set)


class Reader:
    def __init__(self, data: bytes) -> None:
        self.data, self.position = data, 0

    def take(self, size: int) -> bytes:
        end = self.position + size
        if size < 0 or end > len(self.data):
            raise ValueError(f'truncated LNK record at {self.position:#x}')
        result = self.data[self.position:end]
        self.position = end
        return result

    def integer(self, size: int) -> int:
        return int.from_bytes(self.take(size), 'little')

    def name(self) -> str:
        raw = self.take(self.integer(1))
        if not raw or b'\0' in raw:
            raise ValueError('unsupported empty or anonymous LNK name')
        return raw.decode('ascii')

    def expression(self, depth: int = 0) -> tuple:
        if depth > 32:
            raise ValueError('LNK relocation expression is too deep')
        tag = self.integer(1)
        if tag == 0:
            return ('constant', self.integer(4))
        if tag in (2, 4):
            return ('symbol' if tag == 2 else 'section', self.integer(2))
        if tag in (44, 46):
            right, left = self.expression(depth + 1), self.expression(depth + 1)
            # Native PSYLINK evaluates the operand serialized second as the
            # left operand. Addition hides this; negative symbol addends do not.
            return ('add' if tag == 44 else 'subtract', left, right)
        raise ValueError(f'unsupported LNK relocation expression {tag}')


def read(data: bytes) -> Object:
    reader = Reader(data)
    if reader.take(4) != b'LNK\x02':
        raise ValueError('expected native Psy-Q LNK v2 object')
    result = Object({}, {}, [])
    active = None
    chunk_start = 0
    function = None
    while True:
        tag = reader.integer(1)
        if tag == 0:
            if function is not None or reader.position != len(data):
                raise ValueError('bytes after LNK end record')
            return result
        if tag == 16:
            number, group, alignment = reader.integer(2), reader.integer(2), reader.integer(1)
            name = reader.name()
            if group or number in result.sections or alignment not in (2, 4, 8, 16):
                raise ValueError('unsupported or duplicate LNK section declaration')
            result.sections[number] = Section(name, {2: 1, 4: 2, 8: 4, 16: 16}[alignment])
        elif tag == 6:
            active = result.sections[reader.integer(2)]
            chunk_start = len(active.data)
        elif tag in (2, 8, 10):
            if active is None:
                raise ValueError('LNK data record without a section')
            if tag == 2:
                chunk_start = len(active.data)
                active.data.extend(reader.take(reader.integer(2)))
            elif tag == 8:
                size = reader.integer(4)
                if size > 0x200000:
                    raise ValueError('LNK reservation exceeds PlayStation RAM')
                active.data.extend(bytes(size))
            else:
                kind, offset = reader.integer(1), reader.integer(2)
                active.patches.append((chunk_start + offset, kind, reader.expression()))
        elif tag in (12, 14, 48):
            number = reader.integer(2)
            section = None if tag == 14 else reader.integer(2)
            value = 0 if tag == 14 else reader.integer(4)
            symbol = Symbol(reader.name(), section, value, common=tag == 48)
            if number in result.symbols:
                raise ValueError(f'duplicate LNK symbol number {number:#x}')
            result.symbols[number] = symbol
        elif tag in (18, 40):
            section, offset = reader.integer(2), reader.integer(4)
            result.locals.append(Symbol(reader.name(), section, offset, local=True))
        elif tag == 28:
            reader.integer(2)
            reader.name()
        elif tag == 46:
            if reader.integer(1) != 7:
                raise ValueError('LNK object is not MIPS')
        elif tag in (50, 52, 54, 56, 58, 60):
            # Tag 54 stores a u16 offset and u16 line increment. The pinned
            # psy-k parser incorrectly treats that increment as u32.
            reader.take({50: 2, 52: 3, 54: 4, 56: 6, 58: 8, 60: 2}[tag])
        elif tag == 74:
            if function is not None:
                raise ValueError('nested LNK function debug records')
            section, offset = reader.integer(2), reader.integer(4)
            reader.take(22)
            function = reader.name(), section, offset
        elif tag in (76, 78, 80):
            section, offset = reader.integer(2), reader.integer(4)
            reader.take(4)
            if tag == 76:
                if function is None or function[1] != section or function[2] > offset:
                    raise ValueError('invalid LNK function extent')
                name, section, start = function
                result.functions[name] = section, start, offset - start
                function = None
        elif tag in (82, 84):
            reader.take(6)
            storage, datatype = reader.integer(2), reader.integer(2)
            reader.take(4)
            if tag == 84:
                dimensions = reader.integer(2)
                reader.take(4 * dimensions)
                reader.take(reader.integer(1))  # optional type tag
            name = reader.take(reader.integer(1)).decode('ascii')
            if storage in (2, 3) and (datatype >> 4) & 3 == 2:
                result.declared_functions.add(name)
        else:
            raise ValueError(f'unsupported LNK record {tag} at {reader.position - 1:#x}')


def affine(expression: tuple) -> tuple[tuple[str, int] | None, int]:
    """Represent the supported ELF relocation form: one referent plus addend."""
    kind, *operands = expression
    if kind == 'constant':
        value = operands[0]
        return None, value - 0x100000000 if value & 0x80000000 else value
    if kind in ('symbol', 'section'):
        return (kind, operands[0]), 0
    left, a = affine(operands[0])
    right, b = affine(operands[1])
    if kind == 'add' and not (left and right):
        return left or right, a + b
    if kind == 'subtract' and right is None:
        return left, a - b
    raise ValueError('LNK expression cannot be represented by one ELF relocation')


def elf_view(data: bytes, *, functions: tuple[str, ...] = (),
             sizes: dict[str, int] | None = None) -> bytes:
    """Build an objdiff view of an actual native object, without retail inputs."""
    from scripts.kf.mips_elf import (
        DefinedSymbol, MipsRelocation, STB_GLOBAL, STB_LOCAL, STT_FUNC,
        STT_NOTYPE, STT_OBJECT, write_mips_elf,
    )

    obj = read(data)
    sizes = sizes or {}
    functions = set(functions) | set(obj.functions) | obj.declared_functions
    names = {number: '.rodata' if section.name == '.rdata' else section.name
             for number, section in obj.sections.items()}
    allowed = {'.text', '.rodata', '.data', '.sdata', '.bss', '.sbss'}
    if set(names.values()) - allowed or len(set(names.values())) != len(names):
        raise ValueError('unsupported native section family')
    sections = {names[number]: section for number, section in obj.sections.items()}
    symbols = {name: [] for name in allowed}
    common = []
    for symbol in [*obj.symbols.values(), *obj.locals]:
        if symbol.section is None:
            continue
        section = names[symbol.section]
        kind = STT_FUNC if symbol.name in functions else (
            STT_NOTYPE if section == '.text' else STT_OBJECT)
        if symbol.common:
            if section not in ('.bss', '.sbss'):
                raise ValueError('native COMMON request outside BSS')
            common.append(DefinedSymbol(symbol.name, sections[section].alignment,
                                        symbol.value, STT_OBJECT))
        else:
            symbols[section].append(DefinedSymbol(
                symbol.name, symbol.value, sizes.get(symbol.name, 0), kind,
                STB_LOCAL if symbol.local else STB_GLOBAL))
    # ASPSX 1.07 can omit debug extents after the first function in a TU.
    # Bound those definitions by the next native function start or text end.
    # No ADDRESS extent or disassembly heuristic enters this view.
    from dataclasses import replace

    boundaries = sorted({s.value for s in symbols['.text'] if s.kind == STT_FUNC}
                        | {len(sections['.text'].data)})
    symbols['.text'] = [replace(s, size=(obj.functions[s.name][2] if s.name in obj.functions else
        next((end for end in boundaries if end > s.value), s.value) - s.value))
        if s.kind == STT_FUNC else s for s in symbols['.text']]
    payloads, relocations = {}, {}
    for name, section in sections.items():
        payload = bytearray(section.data)
        relocations[name] = []
        for offset, kind, expression in section.patches:
            if offset % 4 or not 0 <= offset <= len(payload) - 4:
                raise ValueError('native relocation outside its code record')
            referent, addend = affine(expression)
            if referent is None:
                raise ValueError('absolute native patch is not an ELF relocation')
            symbol = (names[referent[1]] if referent[0] == 'section'
                      else obj.symbols[referent[1]].name)
            word = struct.unpack_from('<I', payload, offset)[0]
            if kind == 16:
                relocation, value = 'R_MIPS_32', addend & 0xffffffff
            elif kind == 74:
                relocation, value = 'R_MIPS_26', (word & 0xfc000000) | ((addend >> 2) & 0x3ffffff)
            elif kind == 82:
                relocation, value = 'R_MIPS_HI16', (word & 0xffff0000) | (((addend + 0x8000) >> 16) & 0xffff)
            elif kind == 84:
                relocation, value = 'R_MIPS_LO16', (word & 0xffff0000) | (addend & 0xffff)
            else:
                raise ValueError(f'unsupported native MIPS patch {kind}')
            struct.pack_into('<I', payload, offset, value)
            relocations[name].append(MipsRelocation(offset, relocation, symbol))
        payloads[name] = bytes(payload)
    arguments = {}
    for name in ('.data', '.sdata', '.bss', '.sbss'):
        stem = name[1:]
        arguments[stem + ('_size' if name in ('.bss', '.sbss') else '')] = (
            len(payloads.get(name, b'')) if name in ('.bss', '.sbss') else payloads.get(name, b''))
        arguments[stem + '_symbols'] = symbols[name]
        arguments[stem + '_alignment'] = sections[name].alignment if name in sections else 4
        if name in ('.data', '.sdata'):
            arguments[stem + '_relocations'] = relocations.get(name, [])
    return write_mips_elf(
        payloads.get('.text', b''), None, 0, relocations.get('.text', []), symbols['.text'],
        rodata=payloads.get('.rodata', b''), rodata_symbols=symbols['.rodata'],
        rodata_relocations=relocations.get('.rodata', []), common_symbols=common, **arguments)
