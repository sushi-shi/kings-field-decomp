"""Native GCC -> ASPSX controls; inspect exactly the objects PSYLINK receives."""

from __future__ import annotations

import hashlib
import io
import json
from pathlib import Path
import struct
import sys
import tempfile

from elftools.elf.elffile import ELFFile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.compile import compile_source
from scripts.kf.lnk import read
from scripts.kf.retail import write_tsv


ORDER_SOURCE = '''static __inline__ int inline_candidate(int value) { return value - 7; }
int third(int value) { return inline_candidate(value * 3 + 8); }
int first(int value) { return third(value) + 5; }
int second(int value) { return first(value) - 2; }
'''
DATA_SOURCE = '''#define DATA(va, size)
DATA(0x80050000, 9)
unsigned char initialized[9] = {1, 0, 2, 0, 3, 0, 0, 0, 0};
DATA(0x80050010, 5)
const unsigned char message[5] = {120, 121, 0, 0, 0};
DATA(0x80060000, 4)
static int private_word;
DATA(0x80060010, 4)
int public_word;
int witness(int x) { private_word += x; public_word = private_word; return public_word; }
'''


def main() -> int:
    with tempfile.TemporaryDirectory(prefix='kf-native-compiler-') as directory:
        root = Path(directory)
        names = ('order257O0', 'order257O2', 'order260O0', 'order260O2',
                 'data257', 'data260', 'division')
        write_tsv(root / 'delink/game/objects.tsv', ('object', 'scope'),
                  tuple({'object': 'objects/' + name + '.o', 'scope': 'decomp'}
                        for name in names), ())

        def compile(name, source, compiler, optimization='O2'):
            path = root / (name + '.c')
            path.write_text(source)
            output = root / (name + '.o')
            compile_source(path, 'GAME.EXE', output, root / 'delink',
                           optimization=optimization, compiler='gcc' + compiler + '-native')
            native = output.with_suffix('.OBJ').read_bytes()
            metadata = json.loads(output.with_suffix('.o.json').read_text())
            assert metadata['native_object_sha256'] == hashlib.sha256(native).hexdigest()
            assert 'native ASPSX' in metadata['assembler_model']
            return ELFFile(io.BytesIO(output.read_bytes())), read(native), metadata

        for compiler in ('257', '260'):
            for optimization in ('O0', 'O2'):
                elf, native, _ = compile('order' + compiler + optimization,
                                         ORDER_SOURCE, compiler, optimization)
                functions = sorted((s['st_value'], s.name) for s in
                                   elf.get_section_by_name('.symtab').iter_symbols()
                                   if s['st_info']['type'] == 'STT_FUNC')
                names = [name for _, name in functions]
                assert [name for name in names if name != 'inline_candidate'] == ['third', 'first', 'second']
                assert ('inline_candidate' in names) == (optimization == 'O0')
                native_text = next(section for section in native.sections.values() if section.name == '.text')
                assert elf.get_section_by_name('.text')['sh_size'] == len(native_text.data)
                for symbol in elf.get_section_by_name('.symtab').iter_symbols():
                    if symbol['st_info']['type'] == 'STT_FUNC':
                        assert symbol['st_size'] > 0 and symbol['st_size'] % 4 == 0

            elf, native, metadata = compile('data' + compiler, DATA_SOURCE, compiler)
            symbols = {s.name: s for s in elf.get_section_by_name('.symtab').iter_symbols()}
            expected = {'initialized': 9, 'message': 5, 'private_word': 4, 'public_word': 4}
            assert metadata['data_symbol_sizes']['sizes'] == expected
            assert symbols['private_word']['st_info']['bind'] == 'STB_LOCAL'
            assert elf.get_section(symbols['private_word']['st_shndx']).name == '.bss'
            assert symbols['public_word']['st_shndx'] == 'SHN_COMMON'
            assert symbols['public_word']['st_size'] == (8 if compiler == '257' else 4)
            for native_name, elf_name in (('.data', '.data'), ('.rdata', '.rodata')):
                section = next(s for s in native.sections.values() if s.name == native_name)
                assert elf.get_section_by_name(elf_name).data() == section.data
                assert elf.get_section_by_name(elf_name)['sh_addralign'] == section.alignment
            assert elf.get_section_by_name('.data').data()[:9] == bytes((1, 0, 2, 0, 3, 0, 0, 0, 0))
            assert elf.get_section_by_name('.rodata').data()[:5] == b'xy\0\0\0'

        elf, _, _ = compile('division',
            'extern void callee(int); int ratio(int x, int y) {return (x << 6)/(y+1)+1;}\n'
            'void framed(int x) {callee(x); callee(x+1);}\n', '257')
        text = elf.get_section_by_name('.text').data()
        words = struct.unpack(f'<{len(text)//4}I', text)
        assert 0x0007000d in words  # native checked division expansion
        assert any(word == 0x03e00008 and words[index + 1] >> 16 == 0x27bd
                   for index, word in enumerate(words[:-1]))
    print('Native compiler controls passed: function order, inlining, widths, COMMON, bytes, and delay slots')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
