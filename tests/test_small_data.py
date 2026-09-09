"""Small-data ownership must preserve bytes, referents and physical placement."""

import io
import shutil
import struct
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import Elf, _diff_init_section
from scripts.kf.data_sections import load
from scripts.kf.delink import Datum
from scripts.kf.manifest import Unit
from scripts.kf.mips_elf import DefinedSymbol, MipsRelocation, STT_OBJECT, write_mips_elf
from scripts.kf.roundtrip import LINKER, UnitResult, plan, verify_unit
from scripts.kf.sema.image import RetailImage


BASE = 0x80010000


def sample():
    return Unit('game.control', 'GAME.EXE', 'src/game/control.c', 'test', (), (
        Datum(BASE, 4, 'large', 'load'),
        Datum(BASE + 0x100, 4, 'small', 'load', 'global', '.sdata'),
    ))


def blob(value=0, referent='large'):
    return write_mips_elf(b'', None, 0, data=b'ABCD',
                          data_symbols=(DefinedSymbol('large', 0, 4, STT_OBJECT),),
                          sdata=struct.pack('<I', value),
                          sdata_symbols=(DefinedSymbol('small', 0, 4, STT_OBJECT),),
                          sdata_relocations=(MipsRelocation(0, 'R_MIPS_32', referent),))


class SmallDataTests(unittest.TestCase):
    def test_assignment_requires_exact_identity_load_class_and_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'data_identities.tsv').write_text(
                'image\tva\tname\tstorage\nGAME.EXE\t0x80010000\tsmall\tload\n')
            path = root / 'data_sections.tsv'
            header = 'image\tva\tname\tsection\tevidence\n'
            row = 'GAME.EXE\t0x80010000\tsmall\t.sdata\tnative-control\n'
            path.write_text(header + row)
            self.assertEqual(load(root), {('GAME.EXE', BASE): '.sdata'})
            for wrong in (row.replace('GAME.EXE', 'OPEN.EXE'),
                          row.replace('small', 'wrong'), row.replace('.sdata', '.bss'),
                          row.replace('native-control', ''), row + row):
                with self.subTest(row=wrong):
                    path.write_text(header + wrong)
                    with self.assertRaises(ValueError):
                        load(root)

    def test_each_small_data_claim_requires_one_base_and_correct_section(self):
        elf = ELFFile(io.BytesIO(blob()))
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        self.assertEqual(plan(elf, sample(), result), {'.data': BASE, '.sdata': BASE + 0x100})
        self.assertEqual(result.issues, [])
        wrong = replace(sample(), data=(sample().data[0], replace(sample().data[1], section='')))
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(elf, wrong, result)
        self.assertIn('owned-symbol-layout', {i['kind'] for i in result.issues})

    def test_strict_small_data_addend_referent_and_flags(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            left, right = root / 'left.o', root / 'right.o'
            left.write_bytes(blob())
            rt = Elf(left)
            for data, status in ((blob(), 'match'), (blob(4), 'addend'),
                                 (blob(referent='other'), 'referent')):
                right.write_bytes(data)
                diff = _diff_init_section('.sdata', rt, Elf(right))
                self.assertEqual(diff.status, status)
            rc = Elf(left)
            rc.sections['.sdata'] = replace(rc.sections['.sdata'], flags=3)
            self.assertEqual(_diff_init_section('.sdata', rt, rc).status, 'class')

    @unittest.skipUnless(shutil.which(LINKER), 'requires pinned GNU MIPS linker')
    def test_independent_relink_compares_complete_small_data_bytes(self):
        payload = bytearray(0x104)
        payload[:4] = b'ABCD'
        payload[0x100:] = struct.pack('<I', BASE)
        retail = RetailImage.synthetic('GAME.EXE', BASE, bytes(payload))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / 'object.o'
            for value, expected in ((0, set()), (4, {'retail-byte-mismatch'})):
                path.write_bytes(blob(value))
                result = verify_unit(sample(), retail, {}, path, root)
                self.assertTrue(result.linker_ran)
                self.assertEqual({i['kind'] for i in result.issues}, expected)
                if not expected:
                    self.assertEqual(result.initialized_bytes_compared, 8)
