"""Named allocation identity is distinct from the encoded S+A address."""

from dataclasses import replace
import struct
from types import SimpleNamespace
import unittest

from scripts.kf.data_reachability import DataExtent, access_span, audit
from scripts.kf.delink import Catalog, DataObject, Function, _apply_relocation
from scripts.kf.relocations import DataReferent, encode_hi_lo_addend, named_data_referent
from scripts.kf.sema.evidence import Evidence, Reference
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


IMAGE = "GAME.EXE"
BASE = 0x80010000
FUNCTION = Function(IMAGE, BASE, 16, 16, 1, "test", "test", "test")
TABLE = DataObject(IMAGE, BASE + 0x200, 16, "table")
BINDING = Binding(IMAGE, BASE, 16, "text", "function", "test", "", "test", "test",
                  body_size=16)


def catalog(*data):
    return Catalog(functions={IMAGE: (FUNCTION,)},
                   function_starts={IMAGE: {BASE: FUNCTION}}, data={IMAGE: data})


def pair(target=BASE + 4, *, status="reviewed", name="table", low=0x24420000):
    payload = struct.pack("<4I", *encode_hi_lo_addend(0x3C020000, low, target),
                          0x03E00008, 0)
    row = {"image": IMAGE, "site_va": hex(BASE), "paired_site_va": hex(BASE + 4),
           "target_va": hex(target), "target_name": name, "target_region": "load",
           "kind": "mips_hi16_lo16", "opcode": "lui+addiu", "status": status,
           "channel": "reachable-code", "confidence": "test", "provenance": "test"}
    return payload, row


def evidence(payload, rows, *data):
    result = object.__new__(Evidence)
    result.image, result.rows, result.catalog = IMAGE, rows, catalog(*data)
    result.img = RetailImage.synthetic(IMAGE, BASE, payload)
    result.idx = SimpleNamespace(function_owner=lambda va: BINDING if BINDING.contains(va) else None,
                                 data_owner=lambda _: None)
    result._control_references = lambda: []
    result._references = result._by_site = None
    return result


class DataReferentTests(unittest.TestCase):
    def test_name_lookup_is_image_local_and_address_alias_uses_exact_start(self):
        objects = catalog(TABLE)
        objects.data["OPEN.EXE"] = (replace(TABLE, image="OPEN.EXE", va=BASE + 0x400),)
        self.assertEqual(named_data_referent(objects, IMAGE, "table"),
                         DataReferent("table", TABLE.va))
        self.assertEqual(named_data_referent(objects, "OPEN.EXE", "table").va, BASE + 0x400)
        self.assertEqual(named_data_referent(objects, IMAGE, f"DAT_{TABLE.va:08x}"),
                         DataReferent("table", TABLE.va))
        self.assertIsNone(named_data_referent(objects, IMAGE, f"DAT_{TABLE.va + 1:08x}"))
        self.assertIsNone(named_data_referent(objects, IMAGE, "unknown"))

    def test_duplicate_names_do_not_select_first_owner(self):
        with self.assertRaisesRegex(ValueError, "ambiguous-data-referent"):
            named_data_referent(catalog(TABLE, replace(TABLE, va=TABLE.va + 16)), IMAGE, "table")

    def test_reviewed_data_base_can_coincide_with_code_start_or_interior(self):
        for target in (BASE, BASE + 4):
            with self.subTest(target=target):
                payload, row = pair(target)
                relocs, used = _apply_relocation(bytearray(payload), FUNCTION, row,
                                               catalog(TABLE), "safe")
                self.assertEqual([r.symbol for r in relocs], ["table", "table"])
                self.assertEqual(int(used["addend"], 0), target - TABLE.va)

    def test_pointer_initializer_can_have_biased_data_referent(self):
        payload, row = pair()
        row.update(kind="mips32_candidate", paired_site_va="", target_va=hex(BASE))
        blob = bytearray(struct.pack("<I", BASE))
        relocs, used = _apply_relocation(blob, FUNCTION, row, catalog(TABLE), "safe")
        self.assertEqual([r.symbol for r in relocs], ["table"])
        self.assertEqual(int(used["addend"], 0), -0x200)
        self.assertEqual(struct.unpack("<i", blob)[0], -0x200)

    def test_data_base_coinciding_with_unrelated_rodata_retains_actual_owner(self):
        payload, row = pair(BASE + 0x100)
        relocs, used = _apply_relocation(bytearray(payload), FUNCTION, row,
                                       catalog(TABLE), "safe", (BASE + 0x100, 16))
        self.assertEqual([r.symbol for r in relocs], ["table", "table"])
        self.assertEqual(int(used["addend"], 0), -0x100)

    def test_unit_owned_rodata_keeps_section_identity_even_for_biased_base(self):
        for target in (TABLE.va + 2, BASE + 4):
            with self.subTest(target=target):
                payload, row = pair(target)
                relocs, used = _apply_relocation(bytearray(payload), FUNCTION, row,
                                               catalog(TABLE), "safe", (TABLE.va - 8, 24))
                self.assertEqual([r.symbol for r in relocs], [".rodata", ".rodata"])
                self.assertEqual(int(used["addend"], 0), target - (TABLE.va - 8))

    def test_reviewed_owner_preserves_raw_target_and_signed_addend(self):
        payload, row = pair()
        graph = evidence(payload, [row], TABLE)
        reference = graph.references[0]
        self.assertEqual((reference.target, reference.destination, reference.tier),
                         (BASE + 4, TABLE.va, "validated"))
        self.assertEqual(reference.as_dict()["addend"], -0x1FC)
        self.assertEqual(reference.target_label(None), "table-0x1fc")
        self.assertEqual(graph.incoming(TABLE.va, confirmed_only=True), (reference,))
        self.assertEqual(graph.incoming(BINDING, confirmed_only=True), ())

    def test_candidate_name_does_not_override_code_owner_or_gain_review(self):
        payload, row = pair(status="candidate")
        graph = evidence(payload, [row], TABLE)
        reference = graph.references[0]
        # Byte validation is not a promotion of the candidate's owner spelling.
        self.assertIsNone(reference.referent)
        self.assertEqual(reference.destination, BASE + 4)
        self.assertEqual(reference.origins[0].status, "candidate")
        relocs, _used = _apply_relocation(bytearray(payload), FUNCTION, row, catalog(TABLE), "safe")
        self.assertEqual([r.symbol for r in relocs], [".text", ".text"])

    def test_rejected_or_byte_invalid_row_cannot_redirect_traversal(self):
        payload, row = pair()
        for changed in (dict(row, status="rejected"), dict(row, target_va=hex(BASE + 8))):
            graph = evidence(payload, [changed], TABLE)
            reference = graph.references[0]
            self.assertEqual(reference.tier, "rejected")
            self.assertIsNone(reference.referent)
            self.assertEqual(graph.incoming(TABLE.va), ())

    def test_ambiguous_named_owner_remains_candidate_and_fails_delinking(self):
        payload, row = pair()
        objects = (TABLE, replace(TABLE, va=TABLE.va + 16))
        reference = evidence(payload, [row], *objects).references[0]
        self.assertIsNone(reference.referent)
        self.assertEqual(reference.tier, "candidate")
        self.assertIn("ambiguous-data-referent", reference.origins[0].reason)
        with self.assertRaisesRegex(ValueError, "ambiguous-data-referent"):
            _apply_relocation(bytearray(payload), FUNCTION, row, catalog(*objects), "safe")

    def test_conflicting_explicit_owners_are_not_merged_or_promoted(self):
        payload, row = pair()
        other = replace(TABLE, symbol="other", va=TABLE.va + 16)
        graph = evidence(payload, [row, dict(row, target_name="other")], TABLE, other)
        self.assertEqual(len(graph.references), 2)
        self.assertEqual({r.referent.name for r in graph.references}, {"table", "other"})
        self.assertEqual({r.tier for r in graph.references}, {"candidate"})
        self.assertEqual(graph.incoming(TABLE.va, confirmed_only=True), ())

    def test_biased_base_and_one_past_pointer_reach_allocation_without_fake_access(self):
        data = DataExtent(IMAGE, TABLE.va, 16, "table", "load", "DATA")
        for target in (BASE + 4, TABLE.va + TABLE.size):
            for low in (None, 0x24420000):
                with self.subTest(target=target, low=low):
                    payload, _row = pair(target)
                    img = RetailImage.synthetic(IMAGE, BASE, payload)
                    reference = Reference(IMAGE, BASE, target, "address", BASE,
                                          paired_site=BASE + 4 if low is not None else None,
                                          tier="validated", referent=DataReferent("table", TABLE.va))
                    report = audit(IMAGE, (BINDING,), (data,), (reference,), img)
                    self.assertEqual(report["issues"], [])
                    self.assertEqual(report["summary"]["source_owned_data_ranges"], 1)
                    self.assertEqual(access_span(reference, img), (TABLE.va, 1))
                    self.assertEqual(report["references"][0]["target"], target)

    def test_direct_memory_low_still_reports_access_outside_named_owner(self):
        target = TABLE.va - 2
        payload, row = pair(target, low=0x84420000)  # lh v0,-2(v0)
        graph = evidence(payload, [row], TABLE)
        reference = graph.references[0]
        data = DataExtent(IMAGE, TABLE.va, 16, "table", "load", "DATA")
        report = audit(IMAGE, (BINDING,), (data,), (reference,), graph.img)
        self.assertEqual(access_span(reference, graph.img), (target, 2))
        self.assertIn("access-crosses-owner", report["summary"]["issues"])


if __name__ == "__main__":
    unittest.main()
