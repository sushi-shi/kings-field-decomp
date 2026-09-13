"""Controls for image-qualified, confidence-preserving data ownership closure."""

from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout
from dataclasses import replace
from types import SimpleNamespace

from scripts.kf.cli import main as cli_main
from scripts.kf.data_reachability import DataExtent, access_span, audit
from scripts.kf.relocations import validate_relocation
from scripts.kf.sema.evidence import Evidence, Origin, Reference
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


IMAGE = "GAME.EXE"


def function(va=0x1000, *, vendor=False, image=IMAGE, fragments=1):
    return Binding(image, va, 0x20, "text", "function", f"fn_{va:x}", "",
                   "test", "test", body_size=0x20, fragments=fragments,
                   provider="psyq" if vendor else "")


def datum(va, size=8, *, source="DATA", image=IMAGE, storage="load"):
    return DataExtent(image, va, size, f"data_{va:x}", storage, source)


def ref(site, target, *, tier="validated", kind="address", image=IMAGE, origins=()):
    return Reference(image, site, target, kind, None, tier=tier, origins=origins)


class ReachabilityTest(unittest.TestCase):
    def audit(self, functions, data=(), refs=(), img=None):
        img = img or RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000)
        return audit(IMAGE, tuple(functions), tuple(data), tuple(refs), img)

    def test_all_game_functions_are_roots_without_source_or_exact_scores(self):
        report = self.audit([function(), function(0x1100), function(0x1200, vendor=True)])
        self.assertEqual(report["roots"], ["function:00001000", "function:00001100"])
        self.assertEqual(report["summary"]["reached_vendor_functions"], 0)
        self.assertFalse(report["complete_reachable_bytes_proven"])

    def test_empty_root_set_is_not_a_vacuous_success(self):
        report = self.audit([function(vendor=True)])
        self.assertIn("no-game-roots", report["summary"]["issues"])

    def test_optional_vendor_entry_traverses_startup_without_changing_game_roots(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1100)
        report = audit(IMAGE, (function(), function(0x1100, vendor=True),
                               function(0x1200, vendor=True)), (),
                       (ref(0x1100, 0x1200, tier="proven", kind="call"),), img,
                       include_entry=True)
        self.assertEqual(report["summary"]["game_roots"], 1)
        self.assertEqual(report["summary"]["reached_vendor_functions"], 2)
        self.assertEqual(report["entry_roots"], ["function:00001100"])

    def test_unmodelled_entry_is_an_explicit_gap(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1800)
        report = audit(IMAGE, (function(),), (), (), img, include_entry=True)
        self.assertEqual(report["summary"]["issues"]["entry-owner-not-unique"], 1)

    def test_overlapping_entry_owners_stay_candidate(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, b"\0" * 0x1000, entry=0x1010)
        report = audit(IMAGE, (function(0x1800), function(0x1000, vendor=True),
                               function(0x1010, vendor=True)), (), (), img, include_entry=True)
        self.assertEqual(report["summary"]["issues"]["entry-owner-not-unique"], 1)
        vendors = [r for r in report["ranges"] if r.get("provider")]
        self.assertEqual([r["reachability"] for r in vendors], ["candidate", "candidate"])

    def test_bss_is_accounted_without_inventing_initialized_bytes(self):
        report = self.audit([function()], [datum(0x9000, storage="bss")], [
            ref(0x1000, 0x9000),
        ])
        self.assertEqual(report["issues"], [])
        bss = next(r for r in report["ranges"] if r["kind"] == "data")
        self.assertEqual(bss["storage"], "bss")

    def test_cross_owner_hi_lo_pair_remains_a_diagnostic(self):
        reference = replace(ref(0x1000, 0x1200), paired_site=0x1100)
        report = self.audit([function()], [datum(0x1200)], [reference])
        self.assertIn("relocation-pair-crosses-owner", report["summary"]["issues"])

    def test_transitive_vendor_and_pointer_chain(self):
        report = self.audit(
            [function(), function(0x1100, vendor=True)],
            [datum(0x1200), datum(0x1300), datum(0x1400)],
            [ref(0x1000, 0x1100, tier="proven", kind="call"),
             ref(0x1100, 0x1200), ref(0x1204, 0x1300, kind="pointer")],
        )
        self.assertEqual(report["summary"]["reached_vendor_functions"], 1)
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 2)
        self.assertEqual(report["issues"], [])
        end = next(r for r in report["ranges"] if r["va"] == 0x1300)
        self.assertIsNotNone(end["via_reference"])
        self.assertIn("00001200", end["via_owner"])

    def test_cycles_terminate_and_do_not_duplicate_allocations(self):
        report = self.audit([function()], [datum(0x1200), datum(0x1300)], [
            ref(0x1000, 0x1200), ref(0x1200, 0x1300, kind="pointer"),
            ref(0x1300, 0x1200, kind="pointer"),
        ])
        self.assertEqual(len(report["ranges"]), 3)
        self.assertEqual(len(report["references"]), 3)

    def test_candidate_path_never_promotes_downstream_validated_reference(self):
        report = self.audit([function()], [datum(0x1200), datum(0x1300)], [
            ref(0x1000, 0x1200, tier="candidate"), ref(0x1200, 0x1300, kind="pointer"),
        ])
        data = [r for r in report["ranges"] if r["kind"] == "data"]
        self.assertEqual([r["reachability"] for r in data], ["candidate", "candidate"])
        self.assertEqual(report["summary"]["issues"]["candidate-reference-path"], 2)

    def test_stronger_path_revisits_a_previously_candidate_subgraph(self):
        report = self.audit(
            [function(), function(0x1100, vendor=True)],
            [datum(0x1200), datum(0x1300)],
            [ref(0x1000, 0x1200, tier="candidate"),
             ref(0x1004, 0x1100, kind="call", tier="proven"),
             ref(0x1100, 0x1200), ref(0x1200, 0x1300, kind="pointer")],
        )
        end = next(r for r in report["ranges"] if r["va"] == 0x1300)
        self.assertEqual(end["reachability"], "validated")
        self.assertEqual(report["summary"]["issues"]["candidate-reference-path"], 1)

    def test_reviewed_rejection_is_retained_but_not_traversed(self):
        origin = Origin("rejected", "raw-word", "test", "rejected", "review")
        report = self.audit([function()], [datum(0x1200)], [
            ref(0x1000, 0x1200, tier="rejected", origins=(origin,)),
        ])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 0)
        self.assertEqual(report["issues"], [])
        self.assertEqual(report["references"][0]["origins"][0]["status"], "rejected")

    def test_invalid_live_row_is_not_silently_removed(self):
        origin = Origin("rejected", "reachable-code", "test", "candidate", "test",
                        "decoded-target-mismatch")
        report = self.audit([function()], refs=[
            ref(0x1000, 0x1200, tier="rejected", origins=(origin,)),
        ])
        self.assertIn("invalid-live-reference", report["summary"]["issues"])

    def test_same_address_in_other_image_cannot_supply_owner_or_edges(self):
        report = self.audit(
            [function(), function(0x1100, image="OPEN.EXE")],
            [datum(0x1200, image="OPEN.EXE")],
            [ref(0x1000, 0x1200), ref(0x1004, 0x1300, image="OPEN.EXE")],
        )
        self.assertEqual(len(report["references"]), 1)
        self.assertEqual(report["summary"]["issues"], {"unmodeled-target": 1})

    def test_partial_config_prefix_is_not_promoted_to_source_ownership(self):
        prefix = replace(datum(0x1200, 4, source="data_identities.tsv"),
                         note="referenced prefix; full array capacity unknown")
        report = self.audit([function()], [prefix], [ref(0x1000, 0x1200)])
        self.assertEqual(report["summary"]["config_only_data_ranges"], 1)
        self.assertIn("config-only-data-not-compared", report["summary"]["issues"])
        self.assertEqual(report["ranges"][0]["size"], 4)

    def test_source_owner_supersedes_interior_tsv_rows_without_duplication(self):
        report = self.audit([function()], [
            datum(0x1200, 16, source="RODATA"),
            datum(0x1200, 4, source="data.tsv"),
            datum(0x1204, 4, source="data_identities.tsv"),
        ], [ref(0x1000, 0x1204)])
        data = [r for r in report["ranges"] if r["kind"] == "data"]
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0]["size"], 16)
        self.assertEqual(len(data[0]["inventory"]), 2)

    def test_conflicting_source_owners_remain_ambiguous(self):
        report = self.audit([function()], [datum(0x1200, 16), datum(0x1204, 8)], [
            ref(0x1000, 0x1204),
        ])
        self.assertIn("ambiguous-target-owner", report["summary"]["issues"])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 2)
        self.assertEqual(report["references"][0]["path_tier"], "candidate")

    def test_code_data_overlap_is_reported_instead_of_choosing_by_name(self):
        report = self.audit([function()], [datum(0x1008, 4, source="data.tsv")], [
            ref(0x1000, 0x1008, tier="proven", kind="branch"),
        ])
        self.assertIn("ambiguous-target-owner", report["summary"]["issues"])

    def test_unreferenced_census_overlap_is_not_hidden_by_reference_traversal(self):
        scan = replace(datum(0x1002, 4, source="data.tsv"),
                       confidence="byte-pattern", classification="string", note="R& b")
        report = self.audit([function()], [scan])
        self.assertEqual(report["summary"]["issues"], {"code-data-owner-overlap": 1})
        overlap = report["issues"][0]["overlaps"][0]
        self.assertEqual((overlap["overlap_va"], overlap["overlap_size"]), (0x1002, 4))
        self.assertEqual((overlap["confidence"], overlap["note"]), ("byte-pattern", "R& b"))
        self.assertEqual(report["summary"]["config_only_data_ranges"], 0)
        self.assertEqual(report["references"], [])

    def test_overlaps_keep_all_claims_and_clip_at_function_boundaries(self):
        report = self.audit([function(), function(0x1020)], [
            datum(0x101E, 4, source="data.tsv"), datum(0x1008, source="DATA"),
            datum(0x1008, source="data_identities.tsv"),
        ])
        issues = report["issues"]
        self.assertEqual(len(issues), 2)
        self.assertEqual([len(i["overlaps"]) for i in issues], [3, 1])
        boundary = [d for i in issues for d in i["overlaps"] if d["va"] == 0x101E]
        self.assertEqual([(d["overlap_va"], d["overlap_size"]) for d in boundary],
                         [(0x101E, 2), (0x1020, 2)])

    def test_overlap_scope_excludes_other_images_unreached_vendors_and_body_tail(self):
        root = replace(function(), size=0x40)
        report = self.audit([root, function(0x1100, vendor=True)], [
            datum(0x1020), datum(0x0FFC, 4), datum(0x1108),
            datum(0x1004, image="OPEN.EXE"),
        ])
        self.assertEqual(report["issues"], [])
        reached = self.audit([root, function(0x1100, vendor=True)], [datum(0x1108)], [
            ref(0x1000, 0x1100, kind="call", tier="candidate"),
        ])
        self.assertEqual(reached["summary"]["issues"]["code-data-owner-overlap"], 1)
        vendor = next(r for r in reached["ranges"] if r["va"] == 0x1100)
        self.assertEqual(vendor["reachability"], "candidate")

    def test_fragmented_extent_does_not_invent_an_overlap_inside_unknown_holes(self):
        report = self.audit([function(fragments=2)], [datum(0x1008)])
        self.assertEqual(report["summary"]["issues"], {"fragmented-function": 1})

    def test_unresolved_indirect_call_blocks_closure(self):
        report = self.audit([function()], refs=[
            ref(0x1000, None, kind="indirect-call", tier="proven"),
        ])
        self.assertIn("unresolved-indirect-control", report["summary"]["issues"])

    def test_fragmented_function_does_not_claim_false_contiguous_coverage(self):
        report = self.audit([function(fragments=2)], [datum(0x1200)], [
            ref(0x1004, 0x1200),
        ])
        self.assertIn("fragmented-function", report["summary"]["issues"])
        self.assertEqual(report["summary"]["source_owned_data_ranges"], 0)

    def test_direct_word_access_cannot_fit_into_one_byte_identity(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x8C421200).to_bytes(4, "little"))
        reference = replace(ref(0x1000, 0x1200), paired_site=0x1000)
        report = self.audit([function()], [datum(0x1200, 1)], [reference], img)
        self.assertIn("access-crosses-owner", report["summary"]["issues"])

    def test_unaligned_load_checks_containing_word(self):
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x88421203).to_bytes(4, "little"))
        reference = replace(ref(0x1000, 0x1203), paired_site=0x1000)
        self.assertEqual(access_span(reference, img), (0x1200, 4))

    def test_cli_exposes_reachability_without_consuming_its_flags(self):
        output = io.StringIO()
        with redirect_stdout(output), self.assertRaises(SystemExit) as caught:
            cli_main(["verify", "reachability", "--help"])
        self.assertEqual(caught.exception.code, 0)
        self.assertIn("--image", output.getvalue())
        self.assertIn("--output", output.getvalue())


class DataPointerEvidenceTest(unittest.TestCase):
    def test_reviewed_pointer_uses_delinker_validation_and_candidates_stay_weak(self):
        data = replace(function(0x1000), space="data", kind="pointer", size=4)
        img = RetailImage.synthetic(IMAGE, 0x1000, (0x1200).to_bytes(4, "little"))
        for status, tier in (("reviewed", "validated"), ("candidate", "candidate")):
            with self.subTest(status=status):
                row = {"image": IMAGE, "site_va": "0x1000", "paired_site_va": "",
                       "target_va": "0x1200", "kind": "mips32_candidate",
                       "status": status, "channel": "raw-word", "confidence": "test",
                       "provenance": "test"}
                evidence = object.__new__(Evidence)
                evidence.image, evidence.img, evidence.rows = IMAGE, img, [row]
                evidence.catalog = None
                evidence.idx = SimpleNamespace(function_owner=lambda _: None,
                                               data_owner=lambda _: data)
                result = evidence._relocation_references()[0]
                self.assertEqual(result.tier, tier)
                if status == "reviewed":
                    decoded = validate_relocation(img.require(0x1000, 4), data,
                                                  row, None, "safe")
                    self.assertEqual((result.site, result.target), (decoded.site, decoded.target))


if __name__ == "__main__":
    unittest.main()
