"""Private VMANAGER data: independent complete SDK bytes and retail referents."""

from functools import lru_cache
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf import config_data
from scripts.kf.delink import OBJECT_FIELDS, _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.mips_elf import DefinedSymbol, STT_OBJECT, write_mips_elf
from scripts.kf.objdiff import generate_projects, generate_report
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv, validate_config, write_tsv
from scripts.kf.seed_vendored_functions import run_psyk
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


CASES = (
    ('GAME.EXE', 0x80056858, 0x80042DA0, 0x80044EA8, 0x8004F3E4, 0x800569DC),
    ('OPEN.EXE', 0x80035FD0, 0x80022BC0, 0x80024CC8, 0x8002F1B8, 0x800361BC),
)
FUNCTIONS = (('note2pitch', 0xBC, 0x7C), ('note2pitch2', 0x104, 0xC0))
OBJECT_HASH = '96242c8cb04ee3038d1f91133e4155a9a3356870301523b21fe3778f0ef4c570'
PAYLOAD_HASH = '293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2'
NAME = 'svm_pitch_table'


@lru_cache(maxsize=1)
def sdk_vmanager():
    tool, directory = shutil.which('psyk'), os.environ.get('PSYQ_LIB')
    if tool is None or directory is None:
        raise unittest.SkipTest('pinned SDK tools required')
    with tempfile.TemporaryDirectory(prefix='kf-vmanager-pitch-') as temporary:
        root = Path(temporary)
        run_psyk(Path(tool), 'extract', str(Path(directory) / 'LIBSND.LIB'), cwd=root)
        obj = root / 'VMANAGER.OBJ'
        digest = hashlib.sha256(obj.read_bytes()).hexdigest()
        listing = run_psyk(Path(tool), 'list', '--code', str(obj))
    return digest, listing, config_data.parse_sdk_section(listing, '.data')


class PsyqPitchTableTests(unittest.TestCase):
    def retail(self, name):
        try:
            return RetailImage.load(name)
        except (OSError, ValueError):
            self.skipTest('configured hash-identical retail images required')

    def test_sdk_whole_anonymous_section_and_actual_consumer_patches(self):
        digest, listing, section = sdk_vmanager()
        self.assertEqual(digest, OBJECT_HASH)
        self.assertEqual((len(section.data), section.alignment, section.exports), (386, 8, ()))
        self.assertEqual(hashlib.sha256(section.data).hexdigest(), PAYLOAD_HASH)
        values = struct.unpack('<193H', section.data)
        self.assertEqual((values[0], values[-1]), (0x1000, 0x2000))
        self.assertTrue(all(a < b for a, b in zip(values, values[1:])))
        for offset, name in ((0xA14, 'note2pitch'), (0x28CC, 'note2pitch2')):
            self.assertIn(f"Local symbol '{name}' at offset {offset:x} in section 51af", listing)
        for site in (0xA90, 0x3F0):
            self.assertIn(f'Patch type 82 at offset {site:x} with (sectbase(51b0)+$0)', listing)
            self.assertIn(f'Patch type 84 at offset {site + 4:x} with (sectbase(51b0)+$0)', listing)
        # There is no named allocation or additional record hidden in .data.
        self.assertFalse(re.search(r"(?:XDEF|Local|XBSS) symbol .*in section 51b0", listing))

    def test_complete_private_owners_preserve_surrounding_census_and_vendor_status(self):
        identities = load_data_identities(RETAIL_CONFIG)
        rows = read_tsv(RETAIL_CONFIG / 'data.tsv')[1]
        catalog = load_catalog(RETAIL_CONFIG)
        for image, va, first, second, _cosine, old_end in CASES:
            identity = identities[image, va]
            self.assertEqual((identity.name, identity.size, identity.scope, identity.storage,
                              identity.datatype, identity.owner),
                             (NAME, 386, 'static', 'load', 'u16[193]', 'psyq_vmanager'))
            start = va - 0x2F4
            owned = [r for r in rows if r['image'] == image and start <= int(r['va'], 0) < old_end]
            self.assertEqual([(int(r['va'], 0), int(r['size'], 0), r['kind']) for r in owned],
                             [(start, 0x2F4, 'unclassified'), (va, 386, 'array'),
                              (va + 386, old_end - va - 386, 'unclassified')])
            for function_va, (name, size, _offset) in zip((first, second), FUNCTIONS):
                function = catalog.function_starts[image][function_va]
                self.assertEqual((function.symbol, function.body_size, function.scope), (name, size, 'vendored'))
        validate_config(RETAIL_CONFIG)  # Entire three-image partition, not only table bytes.

    def test_whole_payload_occurs_once_in_each_retail_image(self):
        _digest, _listing, section = sdk_vmanager()
        for name, va, *_ in CASES:
            image = self.retail(name)
            self.assertEqual(image.require(va, 386), section.data)
            self.assertEqual(image.data.count(section.data), 1)

    def test_four_reviewed_pairs_restore_raw_words_and_exact_private_referents(self):
        catalog = load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        for image, va, first, second, _cosine, _end in CASES:
            retail = self.retail(image)
            ctx = Context(image)
            refs = {r.site: r for r in ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)}
            self.assertEqual(set(refs), {first + 0x7C, second + 0xC0})
            for start, (_name, size, offset) in zip((first, second), FUNCTIONS):
                with self.subTest(image=image, start=hex(start)):
                    site = start + offset
                    row = rows[image, site]
                    self.assertEqual((row['target_name'], row['status'], row['channel']),
                                     (NAME, 'reviewed', 'reachable-code'))
                    self.assertEqual((int(row['paired_site_va'], 0), int(row['target_va'], 0)), (site + 4, va))
                    hi, lo = retail.u32(site), retail.u32(site + 4)
                    self.assertEqual((hi & 0xFFFF0000, lo & 0xFFFF0000), (0x3C010000, 0x24210000))
                    self.assertEqual(decode_hi_lo_target(hi, lo), va)
                    self.assertEqual(retail.u32(site + 12), 0x94230000)  # lhu v1,0(at)
                    self.assertEqual((retail.u32(start + size - 8), retail.u32(start + size - 4)),
                                     (0x03E00008, 0x3062FFFF))  # jr ra; 16-bit return in delay slot
                    body = bytearray(retail.require(start, size))
                    relocs, used = _apply_relocation(body, catalog.function_starts[image][start], row, catalog, 'safe')
                    self.assertEqual([r.symbol for r in relocs], [NAME, NAME])
                    self.assertEqual(int(used['addend'], 0), 0)
                    raw_hi, raw_lo = struct.unpack_from('<II', body, offset)
                    self.assertEqual(decode_hi_lo_target(raw_hi, raw_lo), 0)
                    self.assertEqual(encode_hi_lo_addend(raw_hi, raw_lo, va), (hi, lo))
                    self.assertEqual((refs[site].destination, refs[site].referent.name), (va, NAME))

    def test_coincident_cosine_base_still_owns_the_sine_table_with_negative_addend(self):
        for image, va, _first, _second, site, _end in CASES:
            self.retail(image)
            ctx = Context(image)
            row = next(r for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
                       if r['image'] == image and int(r['site_va'], 0) == site)
            self.assertEqual((row['target_name'], int(row['target_va'], 0)), ('rsin_tbl', va + 0x18))
            sine_va = int(row['target_va'], 0) + 0x800
            refs = {r.site: r for r in ctx.refs.incoming(ctx.idx.datum(sine_va), confirmed_only=True)}
            self.assertEqual((refs[site].destination, refs[site].referent.name,
                              refs[site].target - refs[site].referent.va),
                             (sine_va, 'rsin_tbl', -0x800))

    def test_private_sdk_object_pairs_match_native_report_and_both_relinks(self):
        if not shutil.which('objdiff-cli'):
            self.skipTest('pinned native objdiff required')
        contributions = [c for c in config_data.load() if c.identity == NAME]
        rows = read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
        for contribution in contributions:
            retail = self.retail(contribution.image)
            with self.subTest(image=contribution.image), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                target_dir, objdiff_dir = root / 'delink', root / 'objdiff'
                target, base = contribution.target_path(target_dir), contribution.base_path(objdiff_dir)
                target.parent.mkdir(parents=True)
                target.write_bytes(config_data.delink_object(contribution, retail, rows))
                config_data.build_base(contribution, base)
                for path in (target, base):
                    elf = ELFFile(io.BytesIO(path.read_bytes()))
                    config_data.validate_object(path.read_bytes(), contribution)
                    symbol = elf.get_section_by_name('.symtab').get_symbol_by_name(NAME)[0]
                    self.assertEqual((symbol['st_info']['bind'], symbol['st_size']), ('STB_LOCAL', 386))
                    self.assertFalse(any(s['st_info']['type'] == 'STT_FUNC'
                                         for s in elf.get_section_by_name('.symtab').iter_symbols()))
                record = dict.fromkeys(OBJECT_FIELDS, '')
                record.update(image=contribution.image, va=hex(contribution.va), body_size='0', data_size='386',
                              name=NAME, unit=contribution.unit, scope='config-data',
                              object=f'data/{contribution.object_name}')
                write_tsv(target_dir / contribution.image_key / 'objects.tsv', OBJECT_FIELDS, [record], ())
                generate_projects(target_dir, objdiff_dir, (contribution.image,))
                report = json.loads(generate_report(objdiff_dir / contribution.image_key).read_text())
                self.assertEqual(config_data.report_failures(report, (contribution,)), [])
                result = config_data.compare(contribution, retail, target, base, root)
                self.assertTrue(result.matched, result.issues)
                for side in (result.target_relink, result.base_relink):
                    self.assertEqual(side['initialized_bytes_compared'], 386)
                # Changing only linkage must fail, even with the same full payload.
                wrong = write_mips_elf(b'', None, 0, data=retail.require(contribution.va, 386), data_alignment=8,
                                       data_symbols=(DefinedSymbol(NAME, 0, 386, STT_OBJECT),))
                with self.assertRaisesRegex(ValueError, 'linkage'):
                    config_data.validate_object(wrong, contribution)


if __name__ == '__main__':
    unittest.main()
