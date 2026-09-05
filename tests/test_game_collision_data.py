"""Typed collision data and bounded retail controls, not whole-TU closure."""

from __future__ import annotations

import hashlib
import re
import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import (
    load_data_identities, load_structure_identities, load_structure_field_identities,
)
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import (
    CandidateFunction, CandidateProgram, ExternalHook, GameSymbols, HookReturn,
    MemoryInput, ParserMachine, RetailProgram,
)
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


RECORDS = ((500, -2000, 1500, -1250), (1000, -2500, 2000, -1000),
           (0, -2500, 2000, -1000), (0, -2500, 1000, -1000),
           (1000, -10000, 2000, -15000), (0, -10000, 2000, -15000),
           (0, -10000, 1000, -15000))
OWNERS = (
    ('game.collision', 0x800558B8, 'map_cell_attribute_height_table', 510,
     '61b46d401d6f82e808978b121e8344f6e4055f90751591c4b969ea1cc21206d4'),
    ('game.effect_map_collision', 0x80055AB8, 'map_cell_height_records', 56,
     'c8ec78b2662ebd64140f199d1ed306b8f1afc3b6737a26668da4d0fd74aefa0a'),
)
REFERENCES = (
    (0x800379B8, 0x80055AB8, 'map_cell_height_records', 0),
    (0x80037B94, 0x80098018, 'map_collision_grid', -100),
    (0x8003A440, 0x800558B8, 'map_cell_attribute_height_table', -2),
    (0x80030004, 0x800558B8, 'map_cell_attribute_height_table', -2),
)
FUNCTION = 'effect_map_collision'


def collision_inputs(symbols, attr, orient, x, z, y, kind=0):
    inputs = [MemoryInput(0x800F0000, struct.pack('<4i', 20000 + x, y, 20000 + z, 0)),
              MemoryInput(0x800F0100, bytes([kind])),
              MemoryInput(symbols.datum('effect_state')[0] + 0xD24, struct.pack('<I', 0x800F0100))]
    for name, value in (('map_collision_grid', 0), ('map_floor_height_grid', 0),
                        ('map_cell_attribute_grid', attr), ('map_cell_orientation_grid', orient)):
        data = bytearray(10000)
        data[1010] = value
        inputs.append(MemoryInput(symbols.datum(name)[0], bytes(data)))
    return inputs


def collision_programs(symbols, object_path):
    hooks = [ExternalHook('collision_query_world', lambda _ctx: HookReturn(0x1234), stack_words=2)]
    return (RetailProgram.link(symbols, [FUNCTION], hooks=hooks),
            CandidateProgram.link(symbols, [CandidateFunction(FUNCTION, object_path)], hooks=hooks))


class GameCollisionDataTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def test_scalar_and_record_owners_leave_the_unused_halfword_unclassified(self):
        manifest = load_manifest()
        identities = load_data_identities(RETAIL_CONFIG)
        census = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1] if r['image'] == 'GAME.EXE']
        for unit, va, name, size, _digest in OWNERS:
            claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                      for d in u.data if d.va == va]
            self.assertEqual(len(claims), 1)
            owner, datum = claims[0]
            self.assertEqual((owner, datum.symbol, datum.size, datum.storage, datum.scope),
                             (unit, name, size, 'load', 'global'))
            identity = identities['GAME.EXE', va]
            self.assertEqual((identity.name, identity.size), (name, size))
            overlap = [r for r in census if int(r['va'], 0) < va + size
                       and va < int(r['va'], 0) + int(r['size'], 0)]
            self.assertEqual(len(overlap), 1)
            self.assertEqual((int(overlap[0]['va'], 0), int(overlap[0]['size'], 0)), (va, size))
        gap = next(r for r in census if int(r['va'], 0) == 0x80055AB6)
        self.assertEqual((gap['kind'], int(gap['size'], 0)), ('unclassified', 2))
        self.assertNotIn(('GAME.EXE', 0x80055AB6), identities)

    def test_shared_record_type_and_literal_initializers(self):
        header = (REPO / 'include/kf/game_collision.h').read_text()
        self.assertRegex(header, r'typedef struct KfCellHeightRecord\s*\{\s*s16 x_min;\s*'
                         r's16 y_min;\s*s16 x_max;\s*s16 y_max;\s*\}')
        self.assertEqual(load_structure_identities(RETAIL_CONFIG)['KfCellHeightRecord'].size, 8)
        fields = [f for f in load_structure_field_identities(RETAIL_CONFIG)
                  if f.structure == 'KfCellHeightRecord']
        self.assertEqual([(f.offset, f.size, f.name, f.datatype) for f in fields],
                         [(0, 2, 'x_min', 's16'), (2, 2, 'y_min', 's16'),
                          (4, 2, 'x_max', 's16'), (6, 2, 'y_max', 's16')])
        for unit, _va, name, size, digest in OWNERS:
            source = load_manifest().by_name()[unit].source_path.read_text()
            definition = re.search(name + r'\[\d+\] = \{(.*?)\};', source, re.S)
            values = [int(v) for v in re.findall(r'-?\d+', definition[1])]
            payload = struct.pack('<' + 'h' * len(values), *values)
            self.assertEqual((len(payload), hashlib.sha256(payload).hexdigest()), (size, digest))
            if name == 'map_cell_height_records':
                self.assertEqual(tuple(struct.iter_unpack('<4h', payload)), RECORDS)
                self.assertNotIn('typedef struct KfCellHeightRecord', source)
            else:
                self.assertEqual(set(v for v in values if v >= 0), set(range(7)))
        magic = (REPO / 'src/game/magic.c').read_text()
        self.assertNotIn('DAT_80055878', magic)
        self.assertIn('#include <kf/game_collision.h>', magic)

    def test_corrected_named_referents_delink_and_restore_raw_pairs(self):
        image = self.retail()
        ctx, catalog = Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        for site, va, name, addend in REFERENCES:
            row = rows['GAME.EXE', site]
            self.assertEqual((row['target_name'], int(row['target_va'], 0), row['status']),
                             (name, va + addend, 'reviewed'))
            reference = next(r for r in ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
                             if r.site == site)
            self.assertEqual((reference.target, reference.destination, reference.referent.name),
                             (va + addend, va, name))
            owner = ctx.idx.function_owner(site)
            body = bytearray(image.require(owner.va, owner.body_size))
            relocs, used = _apply_relocation(body, catalog.function_starts['GAME.EXE'][owner.va],
                                            row, catalog, 'safe')
            self.assertEqual([r.symbol for r in relocs], [name, name])
            self.assertEqual(int(used['addend'], 0), addend)
            high, low = struct.unpack_from('<2I', body, site - owner.va)
            self.assertEqual(decode_hi_lo_target(high, low), addend & 0xFFFFFFFF)
            self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(high, low, va + addend)),
                             image.require(site, 8))
        for wrong, site in ((0x80055878, 0x8003A440), (0x80095900, 0x80037B94)):
            self.assertNotIn(site, [r.site for r in ctx.refs.incoming(ctx.idx.datum(wrong),
                                                                   confirmed_only=True)])

    def test_complete_compiled_and_delinked_payloads_equal_retail(self):
        image = self.retail()
        units = load_manifest().by_name()
        for unit_name, va, name, size, digest in OWNERS:
            self.assertEqual(hashlib.sha256(image.require(va, size)).hexdigest(), digest)
            for directory in (BUILD / 'objdiff/game/base', BUILD / 'delink/game/modules'):
                path = directory / units[unit_name].object_name
                if not path.is_file():
                    self.skipTest('freshly built collision source and target objects required')
                with path.open('rb') as stream:
                    elf = ELFFile(stream)
                    symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(name)
                    self.assertEqual(len(symbols or ()), 1)
                    symbol = symbols[0]
                    self.assertEqual((symbol['st_size'], symbol['st_info']['bind']), (size, 'STB_GLOBAL'))
                    section = elf.get_section(symbol['st_shndx'])
                    self.assertEqual(section.name, '.data')
                    start = symbol['st_value']
                    self.assertEqual(section.data()[start:start + size], image.require(va, size))
        self.assertEqual(image.require(0x80055AB6, 2), b'\0\0')

    def test_bounded_retail_and_candidate_orientation_and_inclusive_bounds(self):
        image = self.retail()
        unit = load_manifest().by_name()['game.effect_map_collision']
        path = BUILD / 'objdiff/game/base' / unit.object_name
        if not path.is_file():
            self.skipTest('freshly built collision source object required')
        symbols = GameSymbols.load()
        programs = collision_programs(symbols, path)
        for attr, bounds in zip((53, 76, 77, 78, 71, 72, 73), RECORDS, strict=True):
            xmin, ymin, xmax, ymax = bounds
            for orient in (0, 1, 2, 3, 4, 5, 255):
                for x in (0, 499, 500, 750, 999, 1000, 1001, 1250, 1499, 1500, 1501, 1999):
                    for y in sorted({ymin - 1, ymin, ymax, ymax + 1}):
                        coordinate = {1: 1250, 2: x, 3: 750, 4: 2000 - x}.get(orient)
                        hit = coordinate is not None and xmin <= coordinate <= xmax and ymin <= y <= ymax
                        inputs = collision_inputs(symbols, attr, orient, x, 1250, y)
                        for program in programs:
                            with self.subTest(attr=attr, orient=orient, x=x, y=y, program=program.label):
                                result = ParserMachine(image, program).call(
                                    FUNCTION, [0x800F0000, 120], memory=inputs,
                                    allowed_writes=[], instruction_limit=1000)
                                self.assertEqual(result.v0, 1 if hit else 0x10000)
                                self.assertEqual(len(result.trace), 1)
        # The downstream world query is an explicit hook, not proof of its body.
        for kind, flags in ((1, 0xE1), (2, 0x71), (3, 0x61)):
            for program in programs:
                result = ParserMachine(image, program).call(
                    FUNCTION, [0x800F0000, 120],
                    memory=collision_inputs(symbols, 76, 2, 1250, 1250, -2000, kind),
                    allowed_writes=[], instruction_limit=1000)
                self.assertEqual(result.v0, 0x1234)
                call = result.trace[-1]
                self.assertEqual((call.name, call.args, call.stack_args),
                                 ('collision_query_world', (21250, (-2000) & 0xFFFFFFFF, 21250, 120),
                                  (0, flags)))


if __name__ == '__main__':
    unittest.main()
