"""Independent GNU-ld controls for retail target section placement and bytes."""

from __future__ import annotations

import io
import shutil
import struct
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path
from unittest import mock

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import Datum, Function
from scripts.kf.manifest import Unit
from scripts.kf.mips_elf import STT_OBJECT, DefinedSymbol, MipsRelocation, write_mips_elf
from scripts.kf.roundtrip import LINKER, SectionPlacement, UnitResult, audit, find_overlaps, plan, verify_unit
from scripts.kf.sema.image import RetailImage


BASE = 0x80010000
TEXT = struct.pack('<8I', 0x3C040000, 0x2484FFFC, 0x0C000000, 0,
                   0x08000006, 0, 0x03E00008, 0)
RETAIL_TEXT = struct.pack('<8I', 0x3C048003, 0x24848000, 0x0C0040A0, 0,
                          0x08004006, 0, 0x03E00008, 0)
RELOCS = (MipsRelocation(0, 'R_MIPS_HI16', 'external_data'),
          MipsRelocation(4, 'R_MIPS_LO16', 'external_data'),
          MipsRelocation(8, 'R_MIPS_26', 'external_function'),
          MipsRelocation(16, 'R_MIPS_26', '.text'))
DATA = struct.pack('<3I', 0xFFFFFFFC, 24, 4)
DATA_RELOCS = (MipsRelocation(0, 'R_MIPS_32', 'external_data'),
               MipsRelocation(4, 'R_MIPS_32', '.text'),
               MipsRelocation(8, 'R_MIPS_32', 'values'))
BOOK = {'external_data': {0x80028004}, 'external_function': {0x80010280}}


def unit() -> Unit:
    function = Function('GAME.EXE', BASE, len(TEXT), len(TEXT), 1, 'control', 'test', 'test')
    return Unit('game.control', 'GAME.EXE', 'src/game/control.c', 'test', (function,),
                (Datum(BASE + 0x100, 12, 'values', 'load'),
                 Datum(BASE + 0x500, 4, 'state', 'bss')), (BASE + 0x200, 8))


def obj(**overrides) -> bytes:
    kwargs = dict(text=TEXT, function_name='control', function_size=len(TEXT),
                  relocations=RELOCS, data=DATA,
                  data_symbols=(DefinedSymbol('values', 0, 12, STT_OBJECT),),
                  data_relocations=DATA_RELOCS, bss_size=4,
                  bss_symbols=(DefinedSymbol('state', 0, 4, STT_OBJECT),),
                  rodata=struct.pack('<2I', 0, 24), rodata_relocations=(
                      MipsRelocation(0, 'R_MIPS_32', '.text'),
                      MipsRelocation(4, 'R_MIPS_32', '.text')))
    return write_mips_elf(**(kwargs | overrides))


def retail() -> RetailImage:
    payload = bytearray(0x400)
    payload[:len(TEXT)] = RETAIL_TEXT
    payload[0x100:0x10C] = struct.pack('<3I', 0x80028000, BASE + 24, BASE + 0x104)
    payload[0x10C:0x110] = b'NEXT'  # Not padding owned by `values`.
    payload[0x200:0x208] = struct.pack('<2I', BASE, BASE + 24)
    payload[0x208:0x20C] = b'NEXT'
    return RetailImage.synthetic('GAME.EXE', BASE, bytes(payload))


def kinds(result: UnitResult) -> set[str]:
    return {issue['kind'] for issue in result.issues}


class PlacementTests(unittest.TestCase):
    def test_one_base_is_required_for_every_owned_symbol(self):
        claims = (Datum(BASE + 0x100, 4, 'first', 'load'),
                  Datum(BASE + 0x200, 4, 'second', 'load'))
        sample = replace(unit(), data=claims)
        blob = obj(data=bytes(8), data_relocations=(), bss_size=0, bss_symbols=(),
                   data_symbols=(DefinedSymbol('first', 0, 4, STT_OBJECT),
                                 DefinedSymbol('second', 4, 4, STT_OBJECT)))
        result = UnitResult(sample.image, sample.unit, 'synthetic')
        bases = plan(ELFFile(io.BytesIO(blob)), sample, result)
        self.assertNotIn('.data', bases)
        issue = next(i for i in result.issues if i['kind'] == 'conflicting-section-bases')
        self.assertEqual([r['implied_base'] for r in issue['claims']],
                         [BASE + 0x100, BASE + 0x1FC])

    def test_alignment_conflict_is_not_scattered_or_overridden(self):
        sample = replace(unit(), data=(unit().data[0], Datum(BASE + 0x501, 4, 'state', 'bss')))
        result = UnitResult(sample.image, sample.unit, 'synthetic')
        plan(ELFFile(io.BytesIO(obj())), sample, result)
        self.assertIn('invalid-section-placement', kinds(result))

    def test_unclaimed_data_section_is_not_discarded(self):
        sample = replace(unit(), data=(unit().data[1],))
        result = UnitResult(sample.image, sample.unit, 'synthetic')
        plan(ELFFile(io.BytesIO(obj())), sample, result)
        self.assertIn('unclaimed-section', kinds(result))

    def test_missing_owned_symbol_is_not_inferred_from_section_size(self):
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(ELFFile(io.BytesIO(obj(data_symbols=(), data_relocations=DATA_RELOCS[:2]))),
             unit(), result)
        self.assertIn('missing-or-ambiguous-owned-symbol', kinds(result))

    def test_undefined_symbol_does_not_supply_owned_storage(self):
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(ELFFile(io.BytesIO(obj(data_symbols=()))), unit(), result)
        self.assertIn('owned-symbol-layout', kinds(result))

    def test_empty_rodata_does_not_satisfy_a_claim(self):
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(ELFFile(io.BytesIO(obj(rodata=b'', rodata_relocations=()))), unit(), result)
        self.assertTrue(kinds(result) & {'missing-owned-section', 'owned-section-extent'})

    def test_owned_symbol_size_is_checked(self):
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(ELFFile(io.BytesIO(obj(data_symbols=(DefinedSymbol('values', 0, 8, STT_OBJECT),)))),
             unit(), result)
        self.assertIn('owned-symbol-layout', kinds(result))

    def test_full_placed_extents_detect_cross_unit_padding_overlap(self):
        left = UnitResult('GAME.EXE', 'game.left', 'left.o',
                          [SectionPlacement('.data', BASE, 16, 8, 'load')])
        right = UnitResult('GAME.EXE', 'game.right', 'right.o',
                           [SectionPlacement('.rodata', BASE + 8, 8, 8, 'load')])
        find_overlaps([left, right])
        self.assertIn('overlapping-section-placements', kinds(left))
        self.assertIn('overlapping-section-placements', kinds(right))
        self.assertEqual(left.issues[0]['size'], 8)

    def test_overlay_addresses_do_not_overlap_across_images(self):
        results = [UnitResult(image, image + '.control', 'synthetic',
                              [SectionPlacement('.data', BASE, 16, 16, 'load')])
                   for image in ('GAME.EXE', 'OPEN.EXE')]
        find_overlaps(results)
        self.assertEqual([r.issues for r in results], [[], []])

    def test_empty_selection_cannot_verify_the_goal(self):
        with tempfile.TemporaryDirectory() as directory:
            report = audit(retail(), (), {}, Path(directory), Path(directory))
        self.assertEqual(report['issues'], {'empty-selection': 1})
        self.assertFalse(report['complete_reachable_bytes_proven'])
        self.assertFalse(report['linked_image_equality_proven'])


@unittest.skipUnless(shutil.which(LINKER), 'requires pinned GNU MIPS linker')
class LinkerTests(unittest.TestCase):
    def verify(self, blob=None, *, sample=None, image=None, book=None, **kwargs):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target = root / 'target.o'
            target.write_bytes(obj() if blob is None else blob)
            return verify_unit(sample or unit(), image or retail(), BOOK if book is None else book,
                               target, root, **kwargs)

    def test_signed_carry_negative_addend_calls_local_jumps_and_data(self):
        result = self.verify()
        self.assertEqual(result.issues, [])
        self.assertTrue(result.linker_ran)
        self.assertEqual(result.relocation_rows, 9)
        self.assertEqual(result.initialized_bytes_compared, 32 + 12 + 8)
        self.assertEqual(next(s for s in result.sections if s.name == '.bss').address, BASE + 0x500)

    def test_byte_and_halfword_bss_relink_without_alignment_overrides(self):
        for alignment in (1, 2):
            with self.subTest(alignment=alignment):
                sample = replace(unit(), data=(unit().data[0],
                                 Datum(BASE + 0x500 + alignment, 4, 'state', 'bss')))
                result = self.verify(obj(bss_alignment=alignment), sample=sample)
                self.assertEqual(result.issues, [])
                self.assertTrue(result.linker_ran)
                section = next(s for s in result.sections if s.name == '.bss')
                self.assertEqual(section.address, BASE + 0x500 + alignment)
                self.assertEqual(section.size, 4)

    def test_byte_data_relinks_complete_section_at_an_odd_address(self):
        sample = replace(unit(), data=(Datum(BASE + 0x101, 3, 'values', 'load'), unit().data[1]))
        image = retail()
        blob = obj(data=image.require(BASE + 0x101, 3), data_relocations=(),
                   data_symbols=(DefinedSymbol('values', 0, 3, STT_OBJECT),), data_alignment=1)
        result = self.verify(blob, sample=sample)
        self.assertEqual(result.issues, [])
        self.assertEqual(result.initialized_bytes_compared, len(TEXT) + 3 + 8)

    def test_wrong_external_binding_changes_retail_bytes(self):
        result = self.verify(book=BOOK | {'external_data': {0x80028008}})
        self.assertIn('retail-byte-mismatch', kinds(result))

    def test_data_addend_changes_retail_bytes(self):
        result = self.verify(obj(data=struct.pack('<3I', 0, 24, 4)))
        issue = next(i for i in result.issues if i['kind'] == 'retail-byte-mismatch')
        self.assertEqual((issue['section'], issue['offset']), ('.data', 0))

    def test_missing_data_relocation_changes_retail_bytes(self):
        result = self.verify(obj(data_relocations=DATA_RELOCS[1:]))
        self.assertIn('retail-byte-mismatch', kinds(result))

    def test_jump_table_addend_changes_retail_bytes(self):
        result = self.verify(obj(rodata=struct.pack('<2I', 0, 20)))
        issue = next(i for i in result.issues if i['kind'] == 'retail-byte-mismatch')
        self.assertEqual((issue['section'], issue['offset']), ('.rodata', 4))

    def test_synthetic_data_tail_is_compared_not_masked(self):
        result = self.verify(obj(data=DATA + bytes(4)))
        issue = next(i for i in result.issues if i['kind'] == 'retail-byte-mismatch')
        self.assertEqual((issue['section'], issue['offset']), ('.data', 12))

    def test_synthetic_rodata_tail_is_compared_not_masked(self):
        result = self.verify(obj(rodata=struct.pack('<2I', 0, 24) + bytes(4)))
        issue = next(i for i in result.issues if i['kind'] == 'retail-byte-mismatch')
        self.assertEqual((issue['section'], issue['offset']), ('.rodata', 8))

    def test_return_delay_slot_is_part_of_byte_comparison(self):
        result = self.verify(obj(text=TEXT[:-4] + struct.pack('<I', 0x24020001)))
        issue = next(i for i in result.issues if i['kind'] == 'retail-byte-mismatch')
        self.assertEqual((issue['section'], issue['offset']), ('.text', 28))

    def test_unknown_external_is_not_assumed_zero(self):
        result = self.verify(book={})
        self.assertIn('unresolved-external', kinds(result))
        self.assertFalse(result.linker_ran)

    def test_ambiguous_identity_does_not_choose_an_address(self):
        result = self.verify(book=BOOK | {'external_data': {0x80028004, 0x80038004}})
        self.assertIn('unresolved-external', kinds(result))
        self.assertFalse(result.linker_ran)

    def test_other_image_never_supplies_bytes(self):
        result = self.verify(image=replace(retail(), image='OPEN.EXE'))
        self.assertIn('artifact-or-link-error', kinds(result))
        self.assertFalse(result.linker_ran)

    def test_missing_linker_is_an_error(self):
        result = self.verify(linker='/missing-kf-test-linker')
        self.assertIn('artifact-or-link-error', kinds(result))

    def test_truncated_object_is_an_error(self):
        self.assertIn('artifact-or-link-error', kinds(self.verify(b'\x7fELF')))

    def test_no_owned_objects_is_not_complete_coverage(self):
        with tempfile.TemporaryDirectory() as directory, mock.patch(
            'scripts.kf.roundtrip.verify_unit', return_value=UnitResult('GAME.EXE', 'game.control', '')
        ):
            report = audit(retail(), (unit(),), BOOK, Path(directory), Path(directory))
        self.assertFalse(report['complete_reachable_bytes_proven'])
        self.assertFalse(report['linked_image_equality_proven'])


if __name__ == '__main__':
    unittest.main()
