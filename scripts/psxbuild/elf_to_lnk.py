"""Bridge a modern MIPS relocatable object to the native Psy-Q linker."""

import io
import struct

from elftools.elf.elffile import ELFFile


def integer(value, size):
    return (value & ((1 << (size * 8)) - 1)).to_bytes(size, 'little')


def name(value):
    encoded = value.encode('ascii')
    if not 0 < len(encoded) < 256:
        raise ValueError(f'unsupported native symbol name: {value}')
    return bytes([len(encoded)]) + encoded


def constant(value):
    return b'\0' + integer(value, 4)


def add(expression, value):
    return b'\x2c' + constant(value) + expression if value else expression


def convert(data):
    elf = ELFFile(io.BytesIO(data))
    if elf.elfclass != 32 or not elf.little_endian or elf['e_machine'] != 'EM_MIPS':
        raise ValueError('expected ELF32 little-endian MIPS')
    if elf['e_type'] != 'ET_REL':
        raise ValueError('expected a relocatable object')
    sections = {i: section for i, section in enumerate(elf.iter_sections())
                if section['sh_flags'] & 2 and section['sh_type'] in ('SHT_PROGBITS', 'SHT_NOBITS')}
    allowed = {'.text', '.rodata', '.data', '.sdata', '.bss', '.sbss'}
    if any(section.name not in allowed for section in sections.values()):
        raise ValueError(f'unmerged allocated ELF sections: {[s.name for s in sections.values()]}')
    numbers = {index: number for number, index in enumerate(sections, 1)}
    symbols = elf.get_section_by_name('.symtab')
    externals = {}
    output = bytearray(b'LNK\x02\x2e\x07')
    for index, section in sections.items():
        if section['sh_addralign'] > 16:
            raise ValueError('native bridge supports section alignment up to 16 bytes')
        output += b'\x10' + integer(numbers[index], 2) + b'\0\0\x10' + name(
            '.rdata' if section.name == '.rodata' else section.name)
    for index, symbol in enumerate(symbols.iter_symbols()):
        owner = symbol['st_shndx']
        if symbol['st_info']['bind'] == 'STB_LOCAL' or not symbol.name:
            continue
        if owner == 'SHN_ABS':
            continue
        if owner != 'SHN_UNDEF' and owner not in numbers:
            raise ValueError(f'{symbol.name}: unsupported ELF symbol section {owner}')
        externals[index] = len(externals) + 0x1000
        output += (b'\x0e' if owner == 'SHN_UNDEF' else b'\x0c') + integer(externals[index], 2)
        if owner != 'SHN_UNDEF':
            output += integer(numbers[owner], 2) + integer(symbol['st_value'], 4)
        output += name(symbol.name)

    def reference(index):
        symbol = symbols.get_symbol(index)
        if index in externals:
            return b'\x02' + integer(externals[index], 2)
        if symbol['st_shndx'] == 'SHN_ABS':
            return constant(symbol['st_value'])
        if symbol['st_shndx'] not in numbers:
            raise ValueError(f'{symbol.name}: unresolved local symbol')
        return add(b'\x04' + integer(numbers[symbol['st_shndx']], 2), symbol['st_value'])

    for index, section in sections.items():
        output += b'\x06' + integer(numbers[index], 2)
        if section['sh_type'] == 'SHT_NOBITS':
            output += b'\x08' + integer(section['sh_size'], 4)
            continue
        payload = section.data()
        if section.name == '.text':
            for offset in range(0, len(payload), 4):
                word = struct.unpack_from('<I', payload, offset)[0]
                if word >> 26 == 0 and (word & 63) in (48, 49, 50, 51, 52, 54):
                    raise ValueError(f'MIPS-II trap instruction at .text+{offset:#x}; '
                                     'the PlayStation requires MIPS-I code')
        relocations = []
        for reloc_section in elf.iter_sections():
            if reloc_section['sh_type'] == 'SHT_REL' and reloc_section['sh_info'] == index:
                relocations.extend(reloc_section.iter_relocations())
        patches = []
        for position, reloc in enumerate(relocations):
            offset, kind, symbol = reloc['r_offset'], reloc['r_info_type'], reloc['r_info_sym']
            if kind == 0:
                continue
            if offset % 4 or not 0 <= offset <= len(payload) - 4:
                raise ValueError('unaligned or out-of-bounds ELF relocation')
            word = struct.unpack_from('<I', payload, offset)[0]
            if kind == 2:
                patch, value = 16, word
            elif kind == 4:
                patch, value = 74, (word & 0x3ffffff) << 2
            elif kind == 5:
                low = next((r for r in relocations[position + 1:]
                            if r['r_info_type'] == 6 and r['r_info_sym'] == symbol), None)
                if low is None:
                    raise ValueError('unpaired ELF HI16 relocation')
                immediate = struct.unpack_from('<h', payload, low['r_offset'])[0]
                patch, value = 82, ((word & 0xffff) << 16) + immediate
            elif kind == 6:
                patch, value = 84, struct.unpack_from('<h', payload, offset)[0]
            else:
                raise ValueError(f'unsupported MIPS relocation {kind} in {section.name}')
            patches.append((offset, patch, add(reference(symbol), value)))
        for start in range(0, len(payload), 65532):
            chunk = payload[start:start + 65532]
            output += b'\x02' + integer(len(chunk), 2) + chunk
            for offset, kind, expression in patches:
                if start <= offset < start + len(chunk):
                    output += b'\x0a' + bytes([kind]) + integer(offset - start, 2) + expression
    return bytes(output + b'\0')
