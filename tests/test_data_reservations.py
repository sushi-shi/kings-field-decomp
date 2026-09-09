"""Fixed reservations must retain C width, storage class and physical extent."""

import io
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.data_reservations import load
from scripts.kf.delink import Datum, Module, _module_data
from scripts.kf.manifest import Unit
from scripts.kf.mips_elf import DefinedSymbol, STB_LOCAL, STT_OBJECT, write_mips_elf
from scripts.kf.roundtrip import UnitResult, plan


class DataReservationTests(unittest.TestCase):
    def test_only_reviewed_nonoverlapping_private_bss_can_reserve(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            identities = ('image\tva\tname\tsize\tstorage\tscope\n'
                          'GAME.EXE\t0x80060000\tfirst\t0x4\tbss\tstatic\n'
                          'GAME.EXE\t0x80060008\tnext\t0x4\tbss\tstatic\n')
            identity_path = root / 'data_identities.tsv'
            identity_path.write_text(identities)
            path = root / 'data_reservations.tsv'
            header = 'image\tva\tname\tsize\tclass\tevidence\n'
            row = 'GAME.EXE\t0x80060000\tfirst\t0x8\tfixed\tnative-control\n'
            path.write_text(header + row)
            self.assertEqual(load(root), {('GAME.EXE', 0x80060000): 8})
            for wrong in (row.replace('first', 'wrong'), row.replace('0x8', '0x3'),
                          row.replace('0x8', '0x9'), row.replace('fixed', 'common'),
                          row.replace('native-control', ''), row + row):
                with self.subTest(row=wrong):
                    path.write_text(header + wrong)
                    with self.assertRaises(ValueError):
                        load(root)
            path.write_text(header + row)
            for wrong in (identities.replace('bss', 'load'),
                          identities.replace('static', 'global')):
                identity_path.write_text(wrong)
                with self.assertRaises(ValueError):
                    load(root)

    def test_sbss_keeps_object_sizes_and_reservations_separate(self):
        data = (Datum(0x80060000, 4, 'first', 'bss', 'static', '.sbss', 8),
                Datum(0x80060008, 4, 'next', 'bss', 'static', '.sbss', 8))
        module = Module('GAME.EXE', 'game.control', 'control', (), data)
        _, _, _, size, symbols = _module_data(module, {}, section='.sbss')
        self.assertEqual(size, 16)
        self.assertEqual([(s.name, s.value, s.size) for s in symbols],
                         [('first', 0, 4), ('next', 8, 4)])
        unit = Unit('game.control', 'GAME.EXE', 'control.c', 'test', (), data)
        for end, second_offset, expected in ((16, 8, set()),
                                             (12, 8, {'owned-symbol-out-of-bounds'}),
                                             (16, 4, {'conflicting-section-bases'})):
            with self.subTest(end=end, offset=second_offset):
                rows = (symbols[0], replace(symbols[1], value=second_offset))
                elf = ELFFile(io.BytesIO(write_mips_elf(
                    b'', None, 0, sbss_size=end, sbss_symbols=rows)))
                result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
                placed = plan(elf, unit, result)
                self.assertEqual({i['kind'] for i in result.issues}, expected)
                if not expected:
                    self.assertEqual(placed, {'.sbss': 0x80060000})
                    self.assertEqual(result.sections[0].storage, 'bss')
                    self.assertEqual(result.sections[0].claimed_bytes, 8)

    def test_bss_and_sbss_have_independent_sizes_flags_and_bases(self):
        first = DefinedSymbol('first', 0, 4, STT_OBJECT, STB_LOCAL)
        small = DefinedSymbol('small', 0, 4, STT_OBJECT, STB_LOCAL)
        elf = ELFFile(io.BytesIO(write_mips_elf(
            b'', None, 0, bss_size=24, bss_symbols=(first,),
            sbss_size=8, sbss_symbols=(small,))))
        unit = Unit('game.control', 'GAME.EXE', 'control.c', 'test', (), (
            Datum(0x80060000, 4, 'first', 'bss', 'static', '.bss', 24),
            Datum(0x80070000, 4, 'small', 'bss', 'static', '.sbss', 8)))
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        self.assertEqual(plan(elf, unit, result), {'.bss': 0x80060000, '.sbss': 0x80070000})
        self.assertEqual(result.issues, [])
        self.assertEqual([(elf.get_section_by_name(n)['sh_size'],
                           elf.get_section_by_name(n)['sh_flags']) for n in ('.bss', '.sbss')],
                         [(24, 3), (8, 0x10000003)])
