"""Paired CFG clues must not confuse return counts with guard scope."""

from __future__ import annotations

import subprocess
import unittest
from dataclasses import replace
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch

from scripts.kf.mips_elf import DefinedSymbol, MipsRelocation, STT_FUNC, write_mips_elf
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD
from scripts.kf.sema.index import index
from scripts.kf.trial import listing
from scripts.kf.trial_flow import compare_flow, object_flow
from test_sema import FakeIndex, function, words


class TrialIndex(FakeIndex):
    def resolve(self, name: str):
        return (0x2000,) if name == "callee" else ()


class TrialFlowTests(unittest.TestCase):
    def setUp(self):
        self.temporary = TemporaryDirectory(prefix="kf-trial-flow-test-")
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / "test.o"
        self.binding = replace(function(0x1000, 8), name="subject")
        self.idx = TrialIndex()

    def obj(self, payload, relocations=(), *, prefix=0, body_size=None):
        size = len(payload) if body_size is None else body_size
        self.path.write_bytes(write_mips_elf(
            words(*([0] * (prefix // 4))) + payload, "preceding" if prefix else "subject",
            prefix or size,
            [replace(r, offset=r.offset + prefix) for r in relocations],
            [DefinedSymbol("subject", prefix, size, STT_FUNC)] if prefix else (),
        ))
        return _load_object(self.path)

    def flow(self, payload, relocations=(), **kwargs):
        return object_flow(self.obj(payload, relocations, **kwargs), self.binding, self.idx)

    def test_equal_counts_do_not_hide_changed_early_return_target(self):
        # Both bodies have two guards and one return; the first guard must
        # skip the second guard, not enter it. The block counts also agree.
        retail = words(0x10800003, 0, 0x10A00003, 0, 0x24020001, 0x24030002, 0x03E00008, 0)
        wrong = words(0x10800001, 0, 0x10A00003, 0, 0x24020001, 0x24030002, 0x03E00008, 0)
        text = "\n".join(compare_flow(self.flow(retail), self.flow(wrong)))
        self.assertIn("2/2 branches, 1/1 returns", text)
        self.assertIn("4/4 blocks", text)
        self.assertIn("first differing successors at B0", text)
        self.assertIn("first differing control #0", text)

    def test_shared_epilogue_preserves_both_incoming_jumps(self):
        payload = words(0x10800003, 0, 0x08000006, 0x24020001,
                        0x08000006, 0x24020002, 0x03E00008, 0)
        relocs = [MipsRelocation(offset, "R_MIPS_26", ".text") for offset in (8, 16)]
        graph = self.flow(payload, relocs).graph
        self.assertTrue(graph.reachability_complete)
        self.assertEqual(sum(e.kind == "return" for e in graph.edges), 1)
        self.assertEqual(graph.blocks[-1].predecessors, [0x1008, 0x1010])

    def test_internal_jump_uses_each_objects_symbol_offset(self):
        a = self.flow(words(0x08000002, 0, 0x03E00008, 0),
                      [MipsRelocation(0, "R_MIPS_26", ".text")])
        b = self.flow(words(0x08000006, 0, 0x03E00008, 0),
                      [MipsRelocation(0, "R_MIPS_26", ".text")], prefix=16)
        self.assertEqual(a.graph.as_dict(), b.graph.as_dict())

    def test_external_call_and_tail_targets_resolve_by_identity(self):
        for opcode, kind in ((0x0C000000, "call"), (0x08000000, "jump-external")):
            with self.subTest(kind=kind):
                flow = self.flow(words(opcode, 0, 0x03E00008, 0),
                                 [MipsRelocation(0, "R_MIPS_26", "callee")])
                edge = next(e for e in flow.graph.edges if e.kind == kind)
                self.assertEqual(edge.target, 0x2000)

    def test_missing_relocation_or_unknown_symbol_is_not_guessed(self):
        for relocs in ([], [MipsRelocation(0, "R_MIPS_26", "unknown")]):
            flow = self.flow(words(0x08000002, 0, 0x03E00008, 0), relocs)
            self.assertIsNone(flow.graph)
            self.assertIn("unavailable", "\n".join(compare_flow(flow, flow)))

    def test_section_jump_to_another_function_uses_its_identity(self):
        self.path.write_bytes(write_mips_elf(
            words(0x0C000004, 0, 0x03E00008, 0, 0x03E00008, 0), "subject", 16,
            [MipsRelocation(0, "R_MIPS_26", ".text")],
            [DefinedSymbol("callee", 16, 8, STT_FUNC)],
        ))
        graph = object_flow(_load_object(self.path), self.binding, self.idx).graph
        self.assertEqual(next(e.target for e in graph.edges if e.kind == "call"), 0x2000)

    def test_changed_branch_operand_is_visible_with_unchanged_edges(self):
        a = self.flow(words(0x10800001, 0, 0x03E00008, 0))
        b = self.flow(words(0x10A00001, 0, 0x03E00008, 0))
        text = "\n".join(compare_flow(a, b))
        self.assertIn("known successor lists agree", text)
        self.assertIn("first differing control #0", text)

    def test_real_actor_guard_scope_regression_is_visible_with_one_return(self):
        unit = load_manifest().by_name()["game.actor"]
        target = BUILD / "delink/game/modules" / unit.object_name
        if not target.is_file():
            self.skipTest("delinked GAME actor target is required")
        obj = _load_object(target)
        idx = index("GAME.EXE")
        binding = idx.function(0x8002D120)
        fn = obj.named_symbol(binding.name)
        payload = bytearray(obj.sections[fn.section])
        # Deliberately make the boss floor/id mismatch paths enter the health
        # guard (+0xbc) instead of bypassing it (+0xcc). No retail file changes.
        for site in (0x94, 0xA0):
            offset = fn.value + site
            word = int.from_bytes(payload[offset:offset + 4], "little")
            self.assertEqual(word >> 26, 5)
            self.assertEqual(site + 4 + (word & 0xFFFF) * 4, 0xCC)
            word = (word & 0xFFFF0000) | ((0xBC - site - 4) // 4)
            payload[offset:offset + 4] = word.to_bytes(4, "little")
        wrong = replace(obj, sections={**obj.sections, fn.section: bytes(payload)})
        a, b = object_flow(obj, binding, idx), object_flow(wrong, binding, idx)
        self.assertEqual(a.issues, ())
        self.assertEqual(b.issues, ())
        text = "\n".join(compare_flow(a, b))
        self.assertIn("27/27 branches, 1/1 returns", text)
        self.assertIn("first differing successors at B0", text)
        self.assertIn("first differing control #0", text)

    def test_unresolved_indirect_dispatch_retains_incomplete_warning(self):
        flow = self.flow(words(0x00400008, 0, 0x03E00008, 0))
        self.assertFalse(flow.graph.reachability_complete)
        self.assertIn("unresolved indirect jump", "\n".join(compare_flow(flow, flow)))

    def test_missing_return_delay_slot_is_visible(self):
        complete = self.flow(words(0x03E00008, 0))
        truncated = self.flow(words(0x03E00008))
        text = "\n".join(compare_flow(complete, truncated))
        self.assertIn("no decoded delay slot", text)
        self.assertIn("delay MISSING", text)

    def test_same_branch_word_different_condition_producer_is_visible(self):
        a = self.flow(words(0x28820005, 0x10400001, 0, 0x03E00008, 0))
        b = self.flow(words(0x28820006, 0x10400001, 0, 0x03E00008, 0))
        text = "\n".join(compare_flow(a, b))
        self.assertIn("known successor lists agree", text)
        self.assertIn("preceding-word/delay-slot window", text)
        self.assertIn("28820005", text)
        self.assertIn("28820006", text)

    def test_instruction_insertion_is_not_a_missing_exit(self):
        a = self.flow(words(0x10800001, 0, 0x03E00008, 0))
        b = self.flow(words(0x24020001, 0x10800001, 0, 0x03E00008, 0))
        text = "\n".join(compare_flow(a, b))
        self.assertIn("known successor lists agree", text)
        self.assertNotIn("first differing control", text)
        self.assertEqual(b.graph.function.body_size, 20)

    def test_listing_keeps_return_delay_slot_but_not_outside_padding(self):
        obj = self.obj(words(0x03E00008, 0, 0, 0), body_size=8)
        output = """Disassembly of section .text:
00000000 <subject>:
   0: 03e00008 jr ra
   4: 00000000 nop
   8: 00000000 nop
   c: 00000000 nop
"""
        with patch("scripts.kf.trial.shutil.which", return_value="objdump"), patch(
            "scripts.kf.trial.subprocess.run",
            return_value=subprocess.CompletedProcess([], 0, stdout=output),
        ) as run:
            self.assertEqual(listing(self.path, obj), {"subject": ["jr ra", "nop"]})
            self.assertIn("-drz", run.call_args.args[0])


if __name__ == "__main__":
    unittest.main()
