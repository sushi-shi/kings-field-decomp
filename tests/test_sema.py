from __future__ import annotations

import io
import unittest
from contextlib import redirect_stdout

from scripts.kf.cli import main as cli_main
from scripts.kf.sema import Context
from scripts.kf.sema.cfg import build_graph
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding, index
from scripts.kf.sema.mips import (
    branch_target,
    decode_control,
    load_delay_register,
)
from scripts.kf.sema.strings import decode_string


class FakeIndex:
    def function(self, _va: int):
        return None


def function(va: int, size: int) -> Binding:
    return Binding(
        image="GAME.EXE",
        va=va,
        size=size,
        body_size=size,
        space="text",
        kind="function",
        name=f"func_{va:08x}",
        curated_name="",
        confidence="test",
        provenance="test",
    )


def words(*values: int) -> bytes:
    return b"".join(value.to_bytes(4, "little") for value in values)


class SemanticToolTests(unittest.TestCase):
    def test_mips_control_and_load_delay_decode(self) -> None:
        branch = 0x10000002  # beq $zero,$zero,+2
        self.assertEqual(branch_target(0x1004, branch), 0x1010)
        control = decode_control(0x1004, branch)
        self.assertIsNotNone(control)
        self.assertEqual(
            (control.kind, control.target, control.delay_slot),
            ("branch", 0x1010, True),
        )
        self.assertEqual(decode_control(0x2000, 0x03E00008).kind, "return")
        self.assertEqual(
            decode_control(0x2000, 0x00A0F809).kind,
            "indirect-call",
        )
        self.assertEqual(load_delay_register(0x8C820000), 2)
        self.assertIsNone(load_delay_register(0xC8820000))

    def test_cfg_keeps_delay_slots_and_edges(self) -> None:
        payload = words(
            0x27BDFFF8,  # addiu sp,sp,-8
            0x10000002,  # beq zero,zero,0x1010
            0x24420001,  # delay slot
            0x24630001,  # fallthrough-only instruction
            0x00000000,
            0x03E00008,  # jr ra
            0x27BD0008,  # delay slot
        )
        graph = build_graph(function(0x1000, len(payload)), payload, FakeIndex())
        self.assertEqual(
            [block.start for block in graph.blocks],
            [0x1000, 0x100C, 0x1010],
        )
        self.assertEqual(graph.delay_slots, {0x1008: 0x1004, 0x1018: 0x1014})
        self.assertEqual(
            [(edge.kind, edge.target) for edge in graph.edges],
            [
                ("taken", 0x1010),
                ("fallthrough", 0x100C),
                ("fallthrough", 0x1010),
                ("return", None),
            ],
        )

    def test_call_keeps_delay_slot_and_proven_target(self) -> None:
        target = 0x3000
        jal = (3 << 26) | ((target >> 2) & 0x03FFFFFF)
        payload = words(jal, 0, 0x03E00008, 0)
        graph = build_graph(function(0x2000, len(payload)), payload, FakeIndex())
        call = next(edge for edge in graph.edges if edge.kind == "call")
        self.assertEqual(
            (call.site, call.target, call.confidence),
            (0x2000, target, "proven"),
        )
        self.assertEqual(graph.delay_slots, {0x2004: 0x2000, 0x200C: 0x2008})

    def test_overlay_indexes_do_not_share_extents(self) -> None:
        game = index("GAME.EXE").function(0x80014268)
        opening = index("OPEN.EXE").function(0x80014268)
        self.assertIsNotNone(game)
        self.assertIsNotNone(opening)
        self.assertEqual((game.image, game.size), ("GAME.EXE", 0x24))
        self.assertEqual((opening.image, opening.size), ("OPEN.EXE", 0x174))

    def test_shift_jis_string_decodes_from_selected_image(self) -> None:
        payload = "王".encode("shift_jis") + b"\0\0"
        image = RetailImage.synthetic("GAME.EXE", 0x80010000, payload)
        ctx = Context("GAME.EXE")
        ctx.__dict__["img"] = image
        binding = Binding(
            image="GAME.EXE",
            va=0x80010000,
            size=len(payload),
            space="data",
            kind="string",
            name="DAT_80010000",
            curated_name="",
            confidence="test",
            provenance="test",
            datatype="shift_jis",
        )
        decoded = decode_string(ctx, binding)
        self.assertEqual(
            (decoded.text, decoded.encoding, decoded.terminated),
            ("王", "shift_jis", True),
        )

    def test_cli_forwards_sema_help_without_consuming_options(self) -> None:
        output = io.StringIO()
        with redirect_stdout(output):
            self.assertEqual(cli_main(["sema", "--help"]), 0)
        self.assertIn("kf sema --image game", output.getvalue())


if __name__ == "__main__":
    unittest.main()
