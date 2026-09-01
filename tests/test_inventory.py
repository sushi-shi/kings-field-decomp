from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    _data_access,
    _ghidra_type,
    _signature_hints,
    load_function_identities,
    validate,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.index import index


def words(*values: int) -> bytes:
    return b"".join(value.to_bytes(4, "little") for value in values)


class FakeImage:
    def __init__(self, word: int):
        self.word = word

    def u32(self, _va: int) -> int:
        return self.word


class FakeReference:
    def __init__(self, kind: str = "address", paired_site: int | None = 0x1004):
        self.kind = kind
        self.paired_site = paired_site


class InventoryTests(unittest.TestCase):
    def test_curated_inventories_cover_the_wip_universe(self) -> None:
        counts = validate(RETAIL_CONFIG)
        self.assertEqual(counts["functions"], 744)
        self.assertEqual(counts["signatures_started"], 744)
        self.assertEqual(counts["typed_returns"], 744)
        self.assertEqual(counts["parameterized"], 498)
        self.assertEqual(counts["data"], 3846)
        self.assertGreaterEqual(counts["functions_named"], 52)
        self.assertGreaterEqual(counts["data_named"], 31)

    def test_static_signature_hint_tracks_live_arguments_and_result(self) -> None:
        parameters, result, shape = _signature_hints(words(
            0x8C820000,  # lw v0,0(a0)
            0x00451021,  # addu v0,v0,a1
            0x03E00008,  # jr ra
            0x00000000,
        ))
        self.assertEqual(parameters, "unknown *object;unknown arg1")
        self.assertEqual(result, "unknown")
        self.assertIn("loads=1", shape)

    def test_semantic_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_math_lifecycle.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 17)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = (
                f"{identity.return_type} {identity.name}({parameters})"
            )
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_data_access_uses_low_instruction_opcode(self) -> None:
        reference = FakeReference()
        self.assertEqual(_data_access(FakeImage(0x8C820000), reference), "read")
        self.assertEqual(_data_access(FakeImage(0xAC820000), reference), "write")
        self.assertEqual(
            _data_access(FakeImage(0), FakeReference(kind="pointer")),
            "initializer",
        )

    def test_ghidra_types_are_explicitly_candidate_project_widths(self) -> None:
        self.assertEqual(_ghidra_type("undefined4"), "u32")
        self.assertEqual(_ghidra_type("undefined2 *"), "u16 *")
        self.assertEqual(_ghidra_type("short *"), "s16 *")

    def test_semantic_function_and_bss_identity_are_queryable(self) -> None:
        game = index("GAME.EXE")
        function = game.function(0x80014E08)
        self.assertEqual(function.name, "vector2s_scale_shift11")
        self.assertEqual(function.link_name, "func_80014e08")
        self.assertEqual(
            (function.owner_type, function.action),
            ("vector2s", "scale_shift11"),
        )
        datum = game.datum(0x800910C0)
        self.assertEqual(
            (datum.name, datum.kind, datum.size),
            ("pool_records", "bss", 0xF0),
        )
        candidate = game.datum(0x80058000)
        self.assertEqual(
            (candidate.name, candidate.kind, candidate.size),
            ("DAT_80058000", "bss", 4),
        )
        matrix = game.function(0x80014CCC)
        self.assertEqual(
            (matrix.name, matrix.owner_type, matrix.action),
            ("matrix_set_rotation_yxz", "matrix", "set_rotation_yxz"),
        )
        frame_counter = game.datum(0x80057B0C)
        self.assertEqual(
            (frame_counter.name, frame_counter.datatype, frame_counter.owner_type),
            ("frame_pacer_vsync_count", "u32", "frame_pacer"),
        )
        exit_code = game.datum(0x800958F8)
        self.assertEqual(
            (exit_code.name, exit_code.datatype, exit_code.owner_type),
            ("game_exit_code", "u32", "game"),
        )


if __name__ == "__main__":
    unittest.main()
