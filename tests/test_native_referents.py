"""Explicit native referents preserve addresses and apply only at reviewed sites."""

import copy
from dataclasses import replace
import io
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

from scripts.kf import graph
from scripts.kf.lnk import elf_view, read
from scripts.kf.manifest import load as load_manifest
from scripts.kf.native_referents import FIELDS, NativeReferent, load
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import read_tsv
from scripts.kf.sdk import assemble_many


class ManifestTests(unittest.TestCase):
    def test_manifest_rejects_ambiguous_and_incomplete_rows(self):
        header = '\t'.join(FIELDS) + '\n'
        row = 'GAME.EXE\twitness\t0x0\t0x4\ttable\t-2\treviewed control\n'
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'referents.tsv'
            path.write_text(header + row)
            self.assertEqual(load(path)[0].addend, -2)
            for text in (header + row + row, header + row.replace('GAME.EXE', 'other'),
                         header + row.replace('0x4', '0x2'),
                         header + row.replace('0x4', '0x0'),
                         header + row.replace('-2', '0xfffffffe'),
                         header + row.replace('reviewed control', ''),
                         header.replace('owner', 'guess') + row):
                with self.subTest(text=text):
                    path.write_text(text)
                    with self.assertRaises(ValueError):
                        load(path)

    def test_every_live_row_agrees_with_independent_retail_owner_and_site(self):
        manifest = load_manifest()
        relocations = read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
        for row in load():
            matches = [(unit, function) for unit in manifest.units
                       if unit.image == row.image for function in unit.functions
                       if function.symbol == row.function]
            self.assertEqual(len(matches), 1)
            unit, function = matches[0]
            owners = [datum for datum in unit.data if datum.symbol == row.owner]
            self.assertEqual(len(owners), 1)
            sites = [r for r in relocations if r['image'] == row.image
                     and int(r['site_va'], 0) == function.va + row.site_offset]
            self.assertEqual(len(sites), 1)
            site = sites[0]
            self.assertEqual((site['kind'], site['status'], site['target_name']),
                             ('mips_hi16_lo16', 'reviewed', row.owner))
            self.assertEqual(int(site['paired_site_va'], 0),
                             function.va + row.paired_site_offset)
            self.assertEqual(int(site['target_va'], 0), owners[0].va + row.addend)

    def test_table_is_a_direct_compile_dependency(self):
        with (patch.object(graph, 'configured_retail_dir', return_value=Path('/retail')),
              patch.object(graph, '_prune_orphans', return_value=0),
              patch.object(graph, '_write_if_changed'),
              patch.object(graph, 'generate_clangd'),
              patch.object(graph, '_write_generator') as write):
            graph.emit()
        edges = [line for line in write.call_args.args[1].splitlines() if ': compile ' in line]
        self.assertTrue(edges)
        for edge in edges:
            self.assertIn('config/native_reloc_referents.tsv', edge.split(' | ')[1].split())


@unittest.skipUnless(shutil.which('dosbox-x') and os.environ.get('PSYQ_ASPSX')
                     and os.environ.get('PSYQ_BIN'), 'native SDK tools required')
class NativeTests(unittest.TestCase):
    def test_explicit_pair_preserves_address_without_renaming_equal_neighbours(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'UNIT.S').write_text(
                '\t.file 1 "control.s"\n\t.text\n\t.globl witness\n\t.ent witness\nwitness:\n'
                '\t.set noreorder\n\tla $2,table-2\n\tla $3,pad+14\n'
                '\tjr $31\n\tnop\n\t.end witness\n'
                '\t.globl neighbour\n\t.ent neighbour\nneighbour:\n'
                '\tla $2,pad+14\n\tjr $31\n\tnop\n\t.end neighbour\n'
                '\t.data\n\t.globl pad\npad:\n\t.word 0,0,0,0\n'
                '\t.globl table\ntable:\n\t.half 1,2,3,4,5\n'
                '\t.rdata\n\t.globl other\nother:\n\t.word 1\n')
            native = assemble_many(root, ('UNIT',), 0)['UNIT']
            row = NativeReferent('GAME.EXE', 'witness', 0, 4, 'table', -2, 'control')
            plain = ELFFile(io.BytesIO(elf_view(native)))
            explicit = ELFFile(io.BytesIO(elf_view(native, referents=(row,))))
            names = []
            for view in (plain, explicit):
                symbols = view.get_section_by_name('.symtab')
                names.append([symbols.get_symbol(r['r_info_sym']).name or '.data'
                              for r in view.get_section_by_name('.rel.text').iter_relocations()])
            self.assertEqual(names[0], ['.data'] * 6)
            self.assertEqual(names[1], ['table', 'table', '.data', '.data', '.data', '.data'])
            self.assertEqual(plain.get_section_by_name('.text').data()[8:],
                             explicit.get_section_by_name('.text').data()[8:])
            for view, base in ((plain, 0), (explicit, 16)):
                high, low = struct.unpack_from('<2I', view.get_section_by_name('.text').data())
                addend = (high & 0xffff) << 16
                addend += (low & 0xffff) - (0x10000 if low & 0x8000 else 0)
                self.assertEqual(base + addend, 14)
            self.assertEqual((root / 'UNIT.OBJ').read_bytes(), native)
            self.assertEqual(plain.get_section_by_name('.data').data(),
                             explicit.get_section_by_name('.data').data())
            for bad in (replace(row, owner='missing'), replace(row, owner='other'),
                        replace(row, addend=-1), replace(row, site_offset=4, paired_site_offset=8),
                        replace(row, paired_site_offset=100), replace(row, function='missing')):
                with self.subTest(row=bad), self.assertRaises(ValueError):
                    elf_view(native, referents=(bad,))
            with self.assertRaisesRegex(ValueError, 'duplicate'):
                elf_view(native, referents=(row, row))
            obj = read(native)
            text = next(section for section in obj.sections.values() if section.name == '.text')
            original = copy.deepcopy(text)
            text.patches[1] = (4, 84, ('add', text.patches[1][2], ('constant', 1)))
            with patch('scripts.kf.lnk.read', return_value=obj), self.assertRaisesRegex(
                    ValueError, 'one section target'):
                elf_view(native, referents=(row,))
            text.patches = original.patches
            low = struct.unpack_from('<I', text.data, 4)[0]
            struct.pack_into('<I', text.data, 4, (low & 0x03ffffff) | (0x0d << 26))
            with patch('scripts.kf.lnk.read', return_value=obj), self.assertRaisesRegex(
                    ValueError, 'sign-extending'):
                elf_view(native, referents=(row,))
