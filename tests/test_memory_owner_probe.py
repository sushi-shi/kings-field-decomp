"""Retail instruction and wrong-owner controls for shared arena state."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import unittest
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS
from tests.test_open_runtime_owner_probe import linked_words


class MemoryOwnerProbeTests(unittest.TestCase):
    def test_allocator_and_opening_reset_retain_retail_instructions(self):
        if not shutil.which("cpppsx-257") or not shutil.which("cc1psx-257"):
            self.skipTest("pinned GCC is required")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if sdk is None:
            self.skipTest("pinned SDK headers are required")
        try:
            retail = {image: (configured_retail_dir() / image).read_bytes()
                      for image in ("GAME.EXE", "OPEN.EXE")}
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        manifest = load()
        catalog = load_catalog(RETAIL_CONFIG)
        identities = load_data_identities(RETAIL_CONFIG)
        for name in ("game.memory", "open.memory", "open.opening_controller"):
            unit = manifest.by_name()[name]
            profile = manifest.profiles[unit.profile]
            data = {item.name: item.va for (image, _), item in identities.items()
                    if image == unit.image}
            arena_base = data["memory_arena"]
            if name.endswith(".memory"):
                self.assertEqual([(d.symbol, d.va, d.size) for d in unit.data],
                                 [("memory_arena", arena_base, 0x58)])
            functions = {f.symbol: f.va for f in catalog.functions[unit.image]}
            with TemporaryDirectory(prefix="kf-memory-owner-") as directory:
                root = Path(directory)
                source = root / unit.source_path.name
                source.write_text(unit.source_path.read_text())
                output = root / unit.object_name
                compile_source(
                    source, unit.image, output, BUILD / "delink",
                    profile.optimization, profile.small_data, profile.aspsx_version,
                    (REPO / "include", REPO / "vendor/include", Path(sdk)), profile.cc1_flags,
                    profile.compiler, defines=unit.defines,
                )
                obj = _load_object(output)
                for claim in unit.functions:
                    with self.subTest(unit=name, function=claim.symbol):
                        words, calls, targets = linked_words(
                            obj, unit, claim, data, functions)
                        expected = list(struct.unpack_from(
                            f"<{claim.body_size // 4}I", retail[unit.image],
                            IMAGE_LAYOUTS[unit.image].file_offset(claim.va)))
                        self.assertEqual(words, expected)
                        if any(arena_base <= va < arena_base + 0x58 for va in targets):
                            shifted = dict(data, memory_arena=arena_base + 4)
                            if ".bss" in shifted:
                                shifted[".bss"] += 4
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, shifted, functions)
                            self.assertNotEqual(wrong, expected)
                            self.assertEqual(same_calls, calls)


if __name__ == "__main__":
    unittest.main()
