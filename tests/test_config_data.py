"""Independent config-data import, complete-object comparison and closure gates."""

from dataclasses import replace
import io
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

from scripts.kf import config_data, graph
from scripts.kf.data_match import run as data_gate
from scripts.kf.data_reachability import DataExtent, audit as reachability
from scripts.kf.delink import Module, OBJECT_FIELDS
from scripts.kf.manifest import load as load_manifest
from scripts.kf.mips_elf import DefinedSymbol, MipsRelocation, STB_LOCAL, STT_OBJECT, write_mips_elf
from scripts.kf.objdiff import generate_projects, generate_report
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.progress import _report_scores
from scripts.kf.retail import read_tsv, validate_config, write_tsv
from scripts.kf.sema.evidence import Reference
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


CONTRIBUTIONS = config_data.load()
GAME = CONTRIBUTIONS[0]
PAYLOAD = bytes(range(256)) * 8
PROVIDER = config_data.SdkSection(PAYLOAD, 8, ((GAME.identity, 0),))
SDK_HEADER = "16 : Section symbol number 3 '.data' in group 0 alignment 8\n6 : Switch to section 3\n"
SDK_EXPORT = "12 : XDEF symbol number 4 'values' at offset 0 in section 3\n"


def target_blob(payload=PAYLOAD, **kwargs):
    args = {"data": payload, "data_alignment": 8,
            "data_symbols": (DefinedSymbol(GAME.identity, 0, len(payload), STT_OBJECT),)}
    return write_mips_elf(b"", None, 0, **{**args, **kwargs})


def exact_report():
    return {"units": [{"name": GAME.unit,
                       "measures": {"total_data": "2048", "matched_data": "2048"},
                       "sections": [{"name": ".data", "size": "2048", "fuzzy_match_percent": 100.0}]}]}


class ConfigDataTests(unittest.TestCase):
    def test_manifest_uses_existing_image_qualified_owners_not_duplicate_addresses(self):
        self.assertEqual([(c.image, c.identity, c.va, c.size) for c in CONTRIBUTIONS], [
            ("GAME.EXE", "rsin_tbl", 0x80057070, 2048),
            ("OPEN.EXE", "rsin_tbl", 0x800367E8, 2048),
        ])
        self.assertEqual(config_data.load(modules=load_manifest().modules()), CONTRIBUTIONS)
        self.assertNotIn("va", config_data.FIELDS)
        self.assertNotIn("size", config_data.FIELDS)
        self.assertEqual(validate_config(RETAIL_CONFIG)['data_contributions'], 2)

    def test_config_contribution_cannot_duplicate_source_ownership(self):
        module = Module(GAME.image, "game.control", "control", (), GAME.data)
        with self.assertRaisesRegex(ValueError, "overlaps"):
            config_data.load(modules=(module,))

    def test_manifest_rejects_unsupported_or_ambiguous_claims(self):
        original = read_tsv(RETAIL_CONFIG / 'data_contributions.tsv')[1][0]
        for changes in ({"unit": "../bad"}, {"unit": "open.sdk.bad"}, {"identity": "unknown"},
                        {"provider": "retail"}, {"section": ".bss"}, {"alignment": "3"},
                        {"alignment": "16"}, {"library": "../LIBGTE.LIB"}, {"evidence": ""},
                        {"object_sha256": "bad"}):
            # GAME's table admits 16-byte alignment; use OPEN for that control.
            row = {**original, **changes}
            if changes == {"alignment": "16"}:
                row.update(image="OPEN.EXE", unit="open.sdk.bad")
            def reader(path):
                return (config_data.FIELDS, [row]) if path.name == "data_contributions.tsv" else read_tsv(path)
            with self.subTest(changes=changes), patch("scripts.kf.config_data.read_tsv", side_effect=reader):
                with self.assertRaises(ValueError):
                    config_data.load()

    def test_sdk_parser_accumulates_whole_section_and_preserves_export(self):
        listing = SDK_HEADER + "2 : Code 2 bytes\n\n0000: 01 02\n2 : Code 2 bytes\n0000: 03 04\n" + SDK_EXPORT
        self.assertEqual(config_data.parse_sdk_section(listing, '.data'),
                         config_data.SdkSection(bytes((1, 2, 3, 4)), 8, (("values", 0),)))

    def test_sdk_parser_never_discards_patches_reservations_or_unknown_records(self):
        for record in ("10 : Patch type 2 at offset 0 with $0", "8 : Uninitialized 4 bytes",
                       "62 : Unknown allocation operation", "18 : Local symbol 'private' in section 3 at offset 0"):
            listing = SDK_HEADER + "2 : Code 4 bytes\n0000: 00 00 00 00\n" + record + "\n" + SDK_EXPORT
            with self.subTest(record=record), self.assertRaises(ValueError):
                config_data.parse_sdk_section(listing, '.data')

    def test_sdk_parser_rejects_missing_disordered_truncated_and_extra_bytes(self):
        for body in ("", "2 : Code 4 bytes\n0000: 00 00\n", "2 : Code 4 bytes\n0002: 00 00 00 00\n",
                     "2 : Code 4 bytes\n0000: 00 00 00 00 00\n",
                     "2 : Code 4 bytes\n0000: 00 00 00 00\n0004: 00 00 00 00\n"):
            with self.subTest(body=body), self.assertRaises(ValueError):
                config_data.parse_sdk_section(SDK_HEADER + body + SDK_EXPORT, '.data')

    def test_sdk_text_patches_referencing_data_do_not_become_data_patches(self):
        listing = (SDK_HEADER + "2 : Code 4 bytes\n0000: 00 00 00 00\n6 : Switch to section 2\n"
                   + "10 : Patch type 82 at offset 0 with (sectbase(3)+$0)\n" + SDK_EXPORT)
        self.assertEqual(len(config_data.parse_sdk_section(listing, '.data').data), 4)

    def test_data_only_elf_has_no_function_or_executable_bytes(self):
        elf = ELFFile(io.BytesIO(target_blob()))
        self.assertEqual(elf.get_section_by_name('.text')['sh_size'], 0)
        self.assertFalse(any(s['st_info']['type'] == 'STT_FUNC'
                             for s in elf.get_section_by_name('.symtab').iter_symbols()))
        config_data.validate_object(target_blob(), GAME)
        with self.assertRaisesRegex(ValueError, "cannot own code"):
            write_mips_elf(bytes(4), None, 0, data=PAYLOAD)
        with self.assertRaisesRegex(ValueError, "require data"):
            write_mips_elf(b"", None, 0)

    def test_object_checks_complete_extent_linkage_alignment_and_extra_storage(self):
        variants = (target_blob(PAYLOAD[:-1]), target_blob(PAYLOAD + b'\0'), target_blob(data_alignment=4),
                    target_blob(bss_size=4), target_blob(rodata=bytes(4)),
                    target_blob(data_symbols=(DefinedSymbol(GAME.identity, 0, 2048, STT_OBJECT, STB_LOCAL),)),
                    target_blob(data_symbols=(DefinedSymbol("wrong", 0, 2048, STT_OBJECT),)),
                    target_blob(data_symbols=(DefinedSymbol(GAME.identity, 0, 1024, STT_OBJECT),)),
                    target_blob(data_symbols=(DefinedSymbol(GAME.identity, 0, 2048),)),
                    target_blob(data_symbols=(DefinedSymbol(GAME.identity, 0, 2048, STT_OBJECT),
                                              DefinedSymbol('alias', 4, 2, STT_OBJECT))),
                    target_blob(data_relocations=(MipsRelocation(0, "R_MIPS_32", "external"),)))
        for number, blob in enumerate(variants):
            with self.subTest(number=number), self.assertRaises(ValueError):
                config_data.validate_object(blob, GAME)

    def test_data_section_flags_are_storage_identity_not_just_equal_payload(self):
        blob = bytearray(target_blob())
        elf = ELFFile(io.BytesIO(blob))
        offset = elf['e_shoff'] + elf.get_section_index('.data') * elf['e_shentsize'] + 8
        struct.pack_into('<I', blob, offset, 2)  # read-only allocation is not writable .data
        with self.assertRaisesRegex(ValueError, 'storage/extent/alignment'):
            config_data.validate_object(bytes(blob), GAME)

    def test_target_carving_does_not_read_the_provider_or_silently_drop_relocations(self):
        image = RetailImage.synthetic(GAME.image, GAME.va, PAYLOAD)
        with patch('scripts.kf.config_data.source_section', side_effect=AssertionError("provider read")):
            blob = config_data.delink_object(GAME, image, [])
        self.assertEqual(blob, target_blob())
        for site, paired in ((GAME.va + 4, ""), (GAME.va - 8, hex(GAME.va)), (GAME.va - 2, "")):
            row = {"image": GAME.image, "site_va": hex(site), "paired_site_va": paired, "status": "candidate"}
            with self.subTest(site=site), self.assertRaisesRegex(ValueError, "relocation"):
                config_data.delink_object(GAME, image, [row])
            self.assertEqual(config_data.delink_object(GAME, image, [dict(row, status="rejected")]), blob)
        with self.assertRaisesRegex(ValueError, "namespace"):
            config_data.delink_object(GAME, replace(image, image="OPEN.EXE"), [])

    def test_only_current_full_two_sided_relink_proof_satisfies_config_reachability(self):
        proof = config_data.Comparison(GAME.image, GAME.unit, GAME.va, GAME.size, GAME.identity)
        success = {"linker_ran": True, "initialized_bytes_compared": GAME.size, "issues": []}
        good = replace(proof, target_relink=success, base_relink=success)
        self.assertFalse(proof.matched)
        self.assertTrue(good.matched)
        root = Binding(GAME.image, GAME.va - 16, 16, "text", "function", "root", "", "test", "test", body_size=16)
        datum = DataExtent(GAME.image, GAME.va, GAME.size, GAME.identity, "load", "data_identities.tsv")
        reference = Reference(GAME.image, root.va, GAME.va, "address", root.va, tier="validated")
        image = RetailImage.synthetic(GAME.image, GAME.va, PAYLOAD)
        for candidate, expected in ((proof, 0), (good, 1), (replace(good, image="OPEN.EXE"), 0),
                                    (replace(good, size=1024), 0), (replace(good, identity="other"), 0),
                                    (replace(good, issues=["stale"]), 0),
                                    (replace(good, base_relink=dict(success, issues=["failed"])), 0)):
            report = reachability(GAME.image, (root,), (datum,), (reference,), image,
                                  config_comparisons=(candidate,))
            self.assertEqual(report['summary']['config_matched_data_ranges'], expected)
            self.assertEqual('config-only-data-not-compared' in report['summary']['issues'], not expected)
            self.assertFalse(report['complete_reachable_bytes_proven'])

    def test_native_report_requires_exact_data_totals_no_fake_code_and_real_section(self):
        document = exact_report()
        self.assertEqual(config_data.report_failures(document, (GAME,)), [])
        scores, errors = _report_scores(document)
        self.assertEqual((scores[GAME.unit], errors), (0.0, []))  # positive data extent, no code score fallback
        for changes in ({"matched_data": "2047"}, {"total_data": "2047"},
                        {"total_code": "4"}, {"total_functions": 1}):
            mutated = exact_report()
            mutated['units'][0]['measures'].update(changes)
            self.assertTrue(config_data.report_failures(mutated, (GAME,)))
        self.assertTrue(config_data.report_failures(None, (GAME,)))
        self.assertTrue(config_data.report_failures({'units': document['units'] * 2}, (GAME,)))

    def test_default_data_gate_includes_config_provider_failures(self):
        with (patch('scripts.kf.data_match.diff_image', return_value=([], [])),
              patch('scripts.kf.config_data.run', return_value=1) as gate,
              patch('builtins.print')):
            self.assertEqual(data_gate((GAME.image,), show_detail=False, show_coverage=False,
                                       delink_dir=Path('delink'), objdiff_dir=Path('objdiff')), 1)
            gate.assert_called_once()

    def test_sdk_data_build_and_reports_are_incremental_inputs(self):
        with (patch.object(graph, 'configured_retail_dir', return_value=Path('/retail')),
              patch.object(graph, '_prune_orphans', return_value=0), patch.object(graph, '_write_if_changed'),
              patch.object(graph, '_write_generator') as generated):
            graph.emit()
        lines = generated.call_args.args[1].splitlines()
        sdk = [line for line in lines if ': sdkdata ' in line]
        self.assertEqual(len(sdk), 2)
        for line in sdk:
            self.assertIn('LIBGTE.LIB', line)
            self.assertIn('data_contributions.tsv', line)
            self.assertIn('data_identities.tsv', line)
            self.assertIn('toolchain.id', line)
        for contribution in CONTRIBUTIONS:
            report = next(line for line in lines if f'build/objdiff/{contribution.image_key}/report.json: report' in line)
            self.assertIn(contribution.object_name, report)


@unittest.skipUnless(shutil.which('mipsel-linux-gnu-as') and shutil.which('mipsel-linux-gnu-ld'),
                     'pinned MIPS tools are required')
class ConfigDataIntegrationTests(unittest.TestCase):
    def test_sdk_converter_preserves_small_extents_without_rounding_or_trimming(self):
        for size, alignment in ((1, 1), (3, 1), (6, 2), (12, 4), (3, 8), (24, 8), (48, 16)):
            contribution = replace(GAME, size=size, alignment=alignment)
            section = config_data.SdkSection(bytes(range(size)), alignment, ((GAME.identity, 0),))
            with (self.subTest(size=size, alignment=alignment), tempfile.TemporaryDirectory() as directory,
                  patch('scripts.kf.config_data.source_section', return_value=section)):
                output = Path(directory) / 'base.o'
                config_data.build_base(contribution, output)
                elf = ELFFile(io.BytesIO(output.read_bytes()))
                self.assertEqual(elf.get_section_by_name('.data').data(), section.data)
                self.assertEqual(elf.get_section_by_name('.data')['sh_addralign'], alignment)
                self.assertEqual(sum(s.name == '.data' for s in elf.iter_sections()), 1)
                image = RetailImage.synthetic(GAME.image, contribution.va, section.data)
                target = Path(directory) / 'target.o'
                target.write_bytes(config_data.delink_object(contribution, image, []))
                result = config_data.compare(contribution, image, target, output, Path(directory))
                self.assertTrue(result.matched, result.issues)

    def test_gas_ordinary_data_tail_cannot_be_used_as_sdk_extent_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'control.s').write_text('.data\n.balign 8\n.space 24\n')
            subprocess.run(['mipsel-linux-gnu-as', '-EL', '-mips1', '-mabi=32',
                            '-o', str(root / 'control.o'), str(root / 'control.s')], check=True)
            section = ELFFile(io.BytesIO((root / 'control.o').read_bytes())).get_section_by_name('.data')
            self.assertEqual((section['sh_size'], section['sh_addralign']), (32, 16))

    def test_production_audit_requires_a_present_exact_current_native_report(self):
        image = RetailImage.synthetic(GAME.image, GAME.va, PAYLOAD)
        with (tempfile.TemporaryDirectory() as directory,
              patch('scripts.kf.config_data.source_section', return_value=PROVIDER),
              patch.object(RetailImage, 'load', return_value=image)):
            root = Path(directory)
            target_dir, objdiff_dir = root / 'delink', root / 'objdiff'
            target, base = GAME.target_path(target_dir), GAME.base_path(objdiff_dir)
            target.parent.mkdir(parents=True)
            target.write_bytes(target_blob())
            config_data.build_base(GAME, base)
            def audit():
                return config_data.audit((GAME.image,), delink_dir=target_dir,
                                         objdiff_dir=objdiff_dir, scratch=root)[0]
            self.assertFalse(audit().matched)
            report_path = objdiff_dir / GAME.image_key / 'report.json'
            report_path.write_text(json.dumps(exact_report()))
            self.assertTrue(audit().matched)
            wrong = exact_report()
            wrong['units'][0]['measures']['matched_data'] = '2047'
            report_path.write_text(json.dumps(wrong))
            self.assertFalse(audit().matched)
            report_path.write_text(json.dumps(exact_report()))
            later = report_path.stat().st_mtime_ns + 1
            os.utime(base, ns=(later, later))
            result = audit()
            self.assertFalse(result.matched)
            self.assertIn('config-data native report is stale', result.issues)

    def compare(self, *, target=None, base=None, image=None):
        with tempfile.TemporaryDirectory() as directory, patch('scripts.kf.config_data.source_section', return_value=PROVIDER):
            root = Path(directory)
            target_path, base_path = root / 'target.o', root / 'base.o'
            target_path.write_bytes(target_blob() if target is None else target)
            if base is None:
                config_data.build_base(GAME, base_path)
            else:
                base_path.write_bytes(base)
            return config_data.compare(GAME, image or RetailImage.synthetic(GAME.image, GAME.va, PAYLOAD),
                                       target_path, base_path, root)

    def test_independent_gas_base_and_delinked_target_both_relink_every_byte(self):
        result = self.compare()
        self.assertTrue(result.matched, result.issues)
        for record in (result.base_relink, result.target_relink):
            self.assertEqual(record['initialized_bytes_compared'], 2048)
            self.assertEqual([(s['name'], s['address'], s['size']) for s in record['sections']],
                             [('.data', GAME.va, 2048)])

    def test_equal_but_wrong_objects_cannot_match_each_other_instead_of_sdk(self):
        bad = target_blob(b'x' + PAYLOAD[1:])
        result = self.compare(target=bad, base=bad)
        self.assertFalse(result.matched)
        self.assertIn('independent SDK', result.issues[0])

    def test_two_matching_objects_still_need_correct_retail_bytes_and_placement(self):
        wrong_image = RetailImage.synthetic(GAME.image, GAME.va, b'x' + PAYLOAD[1:])
        result = self.compare(image=wrong_image)
        self.assertFalse(result.matched)
        self.assertTrue(result.base_relink['issues'])
        self.assertTrue(result.target_relink['issues'])

    def test_bad_elf_missing_bytes_and_wrong_linkage_do_not_get_proof(self):
        for blob in (b'bad elf', target_blob(PAYLOAD[:-8]),
                     target_blob(data_symbols=(DefinedSymbol(GAME.identity, 0, 2048, STT_OBJECT, STB_LOCAL),))):
            result = self.compare(target=blob)
            self.assertFalse(result.matched)

    def test_real_data_only_pair_enters_native_objdiff_without_fake_functions(self):
        if shutil.which('objdiff-cli') is None:
            self.skipTest('pinned objdiff required')
        with tempfile.TemporaryDirectory() as directory, patch('scripts.kf.config_data.source_section', return_value=PROVIDER):
            root = Path(directory)
            target_dir, objdiff_dir = root / 'delink', root / 'objdiff'
            target = GAME.target_path(target_dir)
            target.parent.mkdir(parents=True)
            target.write_bytes(target_blob())
            config_data.build_base(GAME, GAME.base_path(objdiff_dir))
            row = {key: '' for key in OBJECT_FIELDS}
            row.update(image=GAME.image, va=hex(GAME.va), body_size='0', data_size='2048',
                       name=GAME.identity, unit=GAME.unit, scope='config-data',
                       object=f'data/{GAME.object_name}')
            write_tsv(target_dir / GAME.image_key / 'objects.tsv', OBJECT_FIELDS, [row], ())
            generate_projects(target_dir, objdiff_dir, (GAME.image,))
            report = json.loads(generate_report(objdiff_dir / GAME.image_key).read_text())
            self.assertEqual(config_data.report_failures(report, (GAME,)), [])
            unit = report['units'][0]
            self.assertEqual((unit['measures']['total_data'], unit['measures']['matched_data']), ('2048', '2048'))
            self.assertFalse(unit.get('functions'))

    def test_missing_artifacts_remain_a_failed_comparison(self):
        with tempfile.TemporaryDirectory() as directory, patch('scripts.kf.config_data.source_section', return_value=PROVIDER):
            root = Path(directory)
            result = config_data.compare(GAME, RetailImage.synthetic(GAME.image, GAME.va, PAYLOAD),
                                         root / 'missing-target.o', root / 'missing-base.o', root)
            self.assertFalse(result.matched)


class SdkProviderTests(unittest.TestCase):
    def test_both_full_sdk_contributions_equal_retail_through_the_production_path(self):
        try:
            images = {c.image: RetailImage.load(c.image) for c in CONTRIBUTIONS}
        except (OSError, ValueError):
            self.skipTest('configured retail images required')
        rows = read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for contribution in CONTRIBUTIONS:
                image = images[contribution.image]
                target, base = root / 'target.o', root / 'base.o'
                target.write_bytes(config_data.delink_object(contribution, image, rows))
                config_data.build_base(contribution, base)
                result = config_data.compare(contribution, image, target, base, root)
                self.assertTrue(result.matched, result.issues)
                self.assertEqual(result.size, 2048)
                self.assertEqual(result.evidence['sdk_payload_sha256'],
                                 '74743e361fc4d78cbd41bd99b2acf5c75c9b5b0268ccd753ed282ec4394fb25a')

    def test_sdk_extent_exports_and_alignment_cannot_be_selected_to_fit_target(self):
        for section in (replace(PROVIDER, data=PAYLOAD[:1024]), replace(PROVIDER, alignment=4),
                        replace(PROVIDER, exports=((GAME.identity, 4),)),
                        replace(PROVIDER, exports=((GAME.identity, 0), ('extra', 8)))):
            with patch('scripts.kf.config_data.sdk_section', return_value=section):
                with self.assertRaisesRegex(ValueError, 'extent/export/alignment'):
                    config_data.source_section(GAME)

    def test_pinned_sdk_object_hash_must_match(self):
        if shutil.which('psyk') is None:
            self.skipTest('pinned SDK tools required')
        with self.assertRaisesRegex(ValueError, 'hash differs'):
            config_data.source_section(replace(GAME, object_sha256='0' * 64))

    def test_sdk_section_alignment_does_not_imply_rounded_payload_extent(self):
        tool, directory = shutil.which('psyk'), os.environ.get('PSYQ_LIB')
        if tool is None or directory is None:
            self.skipTest('pinned SDK tools required')
        section = config_data.sdk_section(
            'LIBSPU.LIB', 'S_N2P.OBJ', '.data',
            'e1b18023a561e14ae4871a7a27eb867455d5424d0a498ba42abc0cabbcf222df',
            directory, tool)
        self.assertEqual((len(section.data), section.alignment, section.exports), (386, 8, ()))
        self.assertEqual(section.data[-2:], b'\x00\x20')
        # Full object records, not a trimmed symbol or a desired retail extent.
        self.assertNotEqual(len(section.data) % section.alignment, 0)

    def test_false_game_jal_candidate_is_two_sdk_sine_samples_without_a_patch(self):
        provider = config_data.source_section(GAME)
        self.assertEqual(struct.unpack_from('<2h', provider.data, 0x450), (3068, 3073))
        row = next(r for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
                   if r['image'] == GAME.image and r['site_va'] == '0x800574c0')
        self.assertEqual((row['target_va'], row['opcode'], row['status']), ('0x80042ff0', 'jal', 'rejected'))


if __name__ == '__main__':
    unittest.main()
