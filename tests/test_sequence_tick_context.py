"""SDK sequence context ownership; static retail controls, not BIOS emulation."""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest

from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv, validate_config
from scripts.kf.seed_vendored_functions import run_psyk
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


@dataclass(frozen=True)
class TickImage:
    image: str
    start: int
    pointer: int
    saved: int
    sp: int
    context: int
    stack: int
    copy: int
    setjmp: int
    longjmp: int

    def owners(self):
        return ((self.pointer, 4, 'ss_tick_low_ram_pointer', 'load', 'u8 *'),
                (self.saved, 8, 'ss_tick_saved_low_ram', 'bss', 'u8[8]'),
                (self.sp, 4, 'ss_tick_saved_sp', 'bss', 'u32'),
                (self.context, 48, 'ss_tick_context', 'bss', 'jmp_buf'))


IMAGES = (
    TickImage('GAME.EXE', 0x8004A55C, 0x80057D08, 0x80058010, 0x80058018,
              0x8005B270, 0x80063278, 0x8004A52C, 0x8005033C, 0x800503DC),
    TickImage('OPEN.EXE', 0x8002A330, 0x80037478, 0x80037750, 0x80037758,
              0x80039340, 0x80041348, 0x8002A300, 0x800300A0, 0x80030120),
)
# Offset, owning address field, addend, raw high/low opcode+register halves.
PAIRS = (
    (0x02C, 'pointer', 0, 0x3C050000, 0x8CA50000),
    (0x034, 'saved', 0, 0x3C040000, 0x24840000),
    (0x044, 'context', 0, 0x3C040000, 0x24840000),
    (0x090, 'context', 4, 0x3C040000, 0x24840000),
    (0x09C, 'stack', 0, 0x3C020000, 0x24420000),
    (0x0AC, 'sp', 0, 0x3C010000, 0xAC230000),
    (0x2A0, 'pointer', 0, 0x3C040000, 0x8C840000),
    (0x2A8, 'saved', 0, 0x3C050000, 0x24A50000),
    (0x2B8, 'context', 4, 0x3C020000, 0x24420000),
    (0x2C0, 'sp', 0, 0x3C030000, 0x8C630000),
)


class SequenceTickContextTests(unittest.TestCase):
    def retail(self, image):
        try:
            return RetailImage.load(image)
        except (OSError, ValueError):
            self.skipTest('hash-checked local retail images required')

    def test_authentic_sdk_jump_context_is_twelve_words_with_sp_at_word_one(self):
        directory = os.environ.get('PSYQ_INCLUDE')
        if directory is None:
            self.skipTest('pinned Psy-Q headers required')
        header = (Path(directory) / 'SETJMP.H').read_text()
        self.assertRegex(header, r'#define\s+JB_SIZE\s+12\b')
        self.assertRegex(header, r'#define\s+JB_SP\s+1\b')
        self.assertRegex(header, r'typedef\s+int\s+jmp_buf\[JB_SIZE\];')

    def test_complete_owners_replace_interior_aliases_without_claiming_stack_extent(self):
        identities = load_data_identities(RETAIL_CONFIG)
        for image in IMAGES:
            for va, size, name, storage, datatype in image.owners():
                with self.subTest(image=image.image, owner=name):
                    overlaps = [d for (key, address), d in identities.items()
                                if key == image.image and address < va + size
                                and va < address + d.size]
                    self.assertEqual(len(overlaps), 1)
                    datum = overlaps[0]
                    self.assertEqual((datum.va, datum.size, datum.name,
                                      datum.storage, datum.datatype),
                                     (va, size, name, storage, datatype))
                    self.assertEqual((datum.scope, datum.owner, datum.confidence),
                                     ('unknown', 'psyq_sscall', 'supported'))
            self.assertFalse(any(key == image.image and va <= image.stack < va + d.size
                                 for (key, va), d in identities.items()))

    def test_callback_remains_vendor_lineage_not_a_matched_game_function(self):
        vendors = {(r['image'], int(r['va'], 0)): r
                   for r in read_tsv(RETAIL_CONFIG / 'functions_vendored.tsv')[1]}
        units = load_manifest().units
        for image in IMAGES:
            row = vendors[image.image, image.start]
            self.assertEqual((row['name'], int(row['size'], 0), row['library'], row['module']),
                             ('SsSeqCalledTbyT', 0x2F8, 'LIBSND.LIB', 'SSCALL'))
            self.assertEqual(row['confidence'], 'sdk-lineage-supported')
            self.assertFalse(any(u.image == image.image and f.va == image.start
                                 for u in units for f in u.functions))

    def test_release25_archive_is_a_revision_negative_control_not_a_data_provider(self):
        directory, tool = os.environ.get('PSYQ_LIB'), shutil.which('psyk')
        if directory is None or tool is None:
            self.skipTest('pinned Psy-Q archive tools required')
        with tempfile.TemporaryDirectory(prefix='kf-sequence-context-') as temporary:
            root = Path(temporary)
            run_psyk(Path(tool), 'extract', str(Path(directory) / 'LIBSND.LIB'), cwd=root)
            member = root / 'SSCALL.OBJ'
            self.assertEqual(hashlib.sha256(member.read_bytes()).hexdigest(),
                             '7dd7838ada671a3901ebbf8766f484d43d1a5204d274a5e6b11d9dd727e63d9c')
            listing = run_psyk(Path(tool), 'list', '--code', str(member))
        self.assertIn("'SsSeqCalledTbyT'", listing)
        self.assertIn('Code 580 bytes', listing)
        for name in ('SpuVmFlush', 'Snd_play', 'Snd_crescendo', 'Snd_decrescendo',
                     'Snd_tempo', 'Snd_pause', 'Snd_replay', 'Snd_stop'):
            self.assertIn(f"'{name}'", listing)
        for absent in ('setjmp', 'longjmp', 'ss_tick_context'):
            self.assertNotIn(f"'{absent}'", listing)

    def test_open_zero_tail_classification_preserves_the_complete_census(self):
        rows = {(r['image'], int(r['va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]}
        for va, size, kind in ((0x8003774C, 4, 'unclassified'),
                               (0x80037750, 8, 'bss'), (0x80037758, 4, 'bss')):
            row = rows['OPEN.EXE', va]
            self.assertEqual((int(row['size'], 0), row['kind']), (size, kind))
        validate_config(RETAIL_CONFIG)
        self.assertEqual(self.retail('OPEN.EXE').require(0x8003774C, 16), bytes(16))

    def test_twenty_reviewed_pairs_preserve_raw_targets_and_canonical_context_addends(self):
        catalog = load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        for image in IMAGES:
            retail, ctx = self.retail(image.image), Context(image.image)
            function = catalog.function_starts[image.image][image.start]
            for offset, field, addend, hi_opcode, lo_opcode in PAIRS:
                site, base = image.start + offset, getattr(image, field)
                target = base + addend
                with self.subTest(image=image.image, offset=hex(offset)):
                    row = rows[image.image, site]
                    self.assertEqual((row['kind'], row['channel'], row['status']),
                                     ('mips_hi16_lo16', 'reachable-code', 'reviewed'))
                    self.assertEqual((int(row['paired_site_va'], 0), int(row['target_va'], 0)),
                                     (site + 4, target))
                    raw_hi, raw_lo = retail.u32(site), retail.u32(site + 4)
                    self.assertEqual((raw_hi & 0xFFFF0000, raw_lo & 0xFFFF0000),
                                     (hi_opcode, lo_opcode))
                    self.assertEqual(decode_hi_lo_target(raw_hi, raw_lo), target)
                    body = bytearray(retail.require(image.start, 0x2F8))
                    relocations, used = _apply_relocation(body, function, row, catalog, 'safe')
                    self.assertEqual([r.symbol for r in relocations], [row['target_name']] * 2)
                    self.assertEqual(int(used['addend'], 0), addend)
                    high, low = struct.unpack_from('<II', body, offset)
                    self.assertEqual(decode_hi_lo_target(high, low), addend)
                    self.assertEqual(encode_hi_lo_addend(high, low, target), (raw_hi, raw_lo))
                    if field == 'stack':
                        self.assertIsNone(ctx.idx.datum(target))
                    else:
                        refs = {r.site: r for r in ctx.refs.incoming(ctx.idx.datum(base),
                                                                   confirmed_only=True)}
                        self.assertEqual((refs[site].destination, refs[site].referent.name),
                                         (base, row['target_name']))

    def test_save_restore_widths_context_sp_updates_and_call_delay_slots(self):
        # The library body is inspected, not reconstructed or executed with fake BIOS hooks.
        words = {0x040: 0x34060008, 0x050: 0, 0x098: 0x8C830000,
                 0x0A4: 0xAC820000, 0x0A8: 0x2484FFFC, 0x0B8: 0x34050001,
                 0x2B4: 0x34060008, 0x2C8: 0x2444FFFC, 0x2CC: 0x34050002,
                 0x2D4: 0xAC430000, 0x2F0: 0x03E00008, 0x2F4: 0x27BD0038}
        for image in IMAGES:
            retail = self.retail(image.image)
            self.assertEqual(retail.u32(image.pointer), 0x100)
            for offset, word in words.items():
                self.assertEqual(retail.u32(image.start + offset), word)
            for offset, target in ((0x03C, image.copy), (0x04C, image.setjmp),
                                    (0x0B4, image.longjmp), (0x2B0, image.copy),
                                    (0x2D0, image.longjmp)):
                word = retail.u32(image.start + offset)
                self.assertEqual(word >> 26, 3)  # jal, arguments include the following slot
                self.assertEqual((word & 0x03FFFFFF) << 2 | 0x80000000, target)

    def test_setjmp_and_longjmp_are_bios_thunks_not_local_context_implementations(self):
        for image in IMAGES:
            retail = self.retail(image.image)
            for address, selector in ((image.setjmp, 0x13), (image.longjmp, 0x14)):
                self.assertEqual(retail.require(address, 12),
                                 struct.pack('<III', 0x240A00A0, 0x01400008,
                                             0x24090000 | selector))


if __name__ == '__main__':
    unittest.main()
