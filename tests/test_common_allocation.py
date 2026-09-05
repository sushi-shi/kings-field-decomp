"""Unallocated ELF COMMON must not disappear from data matching or relinking."""

from __future__ import annotations

import io
import shutil
import struct
import subprocess
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import diff_image, run
from scripts.kf.delink import Datum, Function
from scripts.kf.manifest import Manifest, Unit
from scripts.kf.mips_elf import STT_OBJECT, DefinedSymbol, write_mips_elf
from scripts.kf.roundtrip import LINKER, UnitResult, plan, verify_unit
from scripts.kf.sema.image import RetailImage


BASE = 0x80010000
TEXT = struct.pack('<2I', 0x03E00008, 0)
WORD = struct.pack('<I', 0x12345678)
ASSEMBLER = 'mipsel-linux-gnu-as'


def unit(with_data=True):
    function = Function('GAME.EXE', BASE, 8, 8, 1, 'control', 'test', 'test')
    return Unit('game.control', 'GAME.EXE', 'src/game/control.c', 'test', (function,),
                (Datum(BASE + 0x100, 4, 'word', 'load'),) if with_data else ())


def object_bytes(*, common=False, with_data=True, size=8, alignment=8, anonymous=False):
    # The production writer intentionally cannot invent a COMMON allocation.
    # Replace one test-only symbol record, leaving all section bytes unchanged.
    blob = bytearray(write_mips_elf(
        TEXT, 'control', 8,
        defined_symbols=(DefinedSymbol('unclaimed', 0, 0, STT_OBJECT),) if common else (),
        data=WORD if with_data else b'', data_alignment=16,
        data_symbols=(DefinedSymbol('word', 0, 4, STT_OBJECT),) if with_data else (),
    ))
    if common:
        symtab = ELFFile(io.BytesIO(blob)).get_section_by_name('.symtab')
        index = next(i for i, symbol in enumerate(symtab.iter_symbols())
                     if symbol.name == 'unclaimed')
        offset = symtab['sh_offset'] + index * symtab['sh_entsize']
        if anonymous:
            struct.pack_into('<I', blob, offset, 0)
        struct.pack_into('<II', blob, offset + 4, alignment, size)
        struct.pack_into('<H', blob, offset + 14, 0xFFF2)  # SHN_COMMON
    return bytes(blob)


def retail():
    payload = bytearray(0x104)
    payload[:8], payload[0x100:] = TEXT, WORD
    return RetailImage.synthetic('GAME.EXE', BASE, bytes(payload))


class CommonAllocationTests(unittest.TestCase):
    def test_every_common_symbol_is_rejected_before_section_placement_can_hide_it(self):
        for size, alignment, anonymous in ((8, 8, False), (0, 4, False), (8, 8, True)):
            with self.subTest(size=size, anonymous=anonymous):
                elf = ELFFile(io.BytesIO(object_bytes(
                    common=True, size=size, alignment=alignment, anonymous=anonymous)))
                result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
                plan(elf, unit(), result)
                self.assertEqual(result.issues, [{
                    'kind': 'unsupported-common-allocation',
                    'symbol': '' if anonymous else 'unclaimed',
                    'size': size, 'alignment': alignment,
                }])

    def test_claim_does_not_convert_common_to_placed_bss(self):
        sample = replace(unit(), data=unit().data + (Datum(BASE + 0x200, 8, 'unclaimed', 'bss'),))
        result = UnitResult('GAME.EXE', 'game.control', 'synthetic')
        plan(ELFFile(io.BytesIO(object_bytes(common=True))), sample, result)
        self.assertIn('unsupported-common-allocation', {i['kind'] for i in result.issues})
        self.assertIn('owned-symbol-layout', {i['kind'] for i in result.issues})
        self.assertNotIn('.bss', {s.name for s in result.sections})

    def test_data_gate_checks_source_target_and_identical_common_without_data_claims(self):
        for with_data in (False, True):
            for target_common, source_common in ((False, False), (True, False),
                                                 (False, True), (True, True)):
                with (self.subTest(data=with_data, target=target_common, source=source_common),
                      tempfile.TemporaryDirectory() as directory):
                    root, sample = Path(directory), unit(with_data)
                    manifest = Manifest({}, (sample,))
                    for prefix, common in (('delink/game/modules', target_common),
                                           ('objdiff/game/base', source_common)):
                        path = root / prefix / sample.object_name
                        path.parent.mkdir(parents=True)
                        path.write_bytes(object_bytes(common=common, with_data=with_data))
                    results, failures = diff_image('GAME.EXE', manifest, root / 'delink',
                                                  root / 'objdiff')
                    self.assertEqual(failures, [])
                    if target_common or source_common:
                        self.assertEqual(len(results), 1)
                        self.assertFalse(results[0].matches)
                        self.assertEqual(len(results[0].divergent), 1)
                        issue = results[0].divergent[0]
                        self.assertEqual((issue.name, issue.status), ('object', 'placement'))
                        self.assertIn('unsupported-common-allocation', issue.detail)
                        self.assertEqual('target:' in issue.detail, target_common)
                        self.assertEqual('reconstruction:' in issue.detail, source_common)
                    else:
                        self.assertTrue(all(r.matches for r in results))
                    with (patch('scripts.kf.data_match.load_manifest', return_value=manifest),
                          patch('scripts.kf.config_data.run', return_value=0),
                          patch('sys.stdout', new_callable=io.StringIO)):
                        self.assertEqual(run(
                            ('GAME.EXE',), show_detail=True, show_coverage=False,
                            delink_dir=root / 'delink', objdiff_dir=root / 'objdiff',
                        ), int(target_common or source_common))

    def test_relink_rejects_unreferenced_common_before_invoking_linker(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / 'source.o'
            path.write_bytes(object_bytes(common=True))
            with patch('scripts.kf.roundtrip.subprocess.run') as linker:
                result = verify_unit(unit(), retail(), {}, path, root)
            linker.assert_not_called()
            self.assertFalse(result.linker_ran)
            self.assertEqual(result.initialized_bytes_compared, 0)
            self.assertEqual([i['kind'] for i in result.issues], ['unsupported-common-allocation'])


@unittest.skipUnless(shutil.which(ASSEMBLER) and shutil.which(LINKER), 'pinned GNU MIPS tools required')
class NativeCommonAllocationTests(unittest.TestCase):
    def test_real_gas_common_survives_input_but_old_discard_script_loses_it(self):
        assembly = '''.set noreorder
.text
.globl control
.type control,@function
control:
jr $ra
nop
.size control,.-control
.data
.globl word
.type word,@object
word:
.word 0x12345678
.size word,4
'''
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for common in (False, True):
                with self.subTest(common=common):
                    path = root / 'source.o'
                    subprocess.run([ASSEMBLER, '-march=r3000', '-mabi=32', '-G0',
                                    '-no-pad-sections', '-o', str(path)],
                                   input=assembly + ('.comm unclaimed,8,8\n' if common else ''),
                                   text=True, check=True, capture_output=True)
                    elf = ELFFile(io.BytesIO(path.read_bytes()))
                    self.assertEqual(elf.get_section_by_name('.text').data(), TEXT)
                    self.assertEqual(elf.get_section_by_name('.data').data(), WORD)
                    if common:
                        symbol, = elf.get_section_by_name('.symtab').get_symbol_by_name('unclaimed')
                        self.assertEqual((symbol['st_shndx'], symbol['st_value'], symbol['st_size']),
                                         ('SHN_COMMON', 8, 8))
                        # Independent real-ld control: the old script succeeds,
                        # compares all 12 initialized bytes, but loses 8 B COMMON.
                        script, linked = root / 'old.ld', root / 'old.elf'
                        script.write_text('SECTIONS { .text 0x80010000 : { *(.text) }\n'
                                          '.data 0x80010100 : { *(.data) }\n'
                                          '/DISCARD/ : { *(*) } }\n')
                        subprocess.run([LINKER, '-EL', '--entry', hex(BASE), '-T', str(script),
                                        '-o', str(linked), str(path)], check=True, capture_output=True)
                        output = ELFFile(io.BytesIO(linked.read_bytes()))
                        self.assertEqual(output.get_section_by_name('.text').data(), TEXT)
                        self.assertEqual(output.get_section_by_name('.data').data(), WORD)
                        self.assertIsNone(output.get_section_by_name('.symtab').get_symbol_by_name('unclaimed'))
                        self.assertEqual(sum(s['sh_size'] for s in output.iter_sections()
                                             if s['sh_flags'] & 2), 12)
                    result = verify_unit(unit(), retail(), {}, path, root)
                    if common:
                        self.assertFalse(result.linker_ran)
                        self.assertEqual([i['kind'] for i in result.issues],
                                         ['unsupported-common-allocation'])
                    else:
                        self.assertEqual(result.issues, [])
                        self.assertTrue(result.linker_ran)
                        self.assertEqual(result.initialized_bytes_compared, 12)


if __name__ == '__main__':
    unittest.main()
