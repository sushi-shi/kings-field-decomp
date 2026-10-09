from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import LOCAL_CONFIG, REPO
from scripts.kf.sema.image import RetailImage
from scripts.kf.tmd_oracle import length_prefixed_chunks


class ActorAttachmentContractTests(unittest.TestCase):
    def test_all_slots_and_special_attack_interpretations(self) -> None:
        compiler = shutil.which("clang++")
        linker = shutil.which("cc")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if compiler is None or linker is None or sdk is None:
            self.skipTest("pinned Clang and SDK required; run in nix develop")
        with TemporaryDirectory(prefix="kf-actor-attachment-") as directory:
            output = Path(directory) / "contract"
            object_path = Path(directory) / "contract.o"
            result = subprocess.run(
                [compiler, "-std=c++20", "-O2", "-fno-exceptions", "-fno-rtti",
                 "-I", str(REPO / "include"),
                 "-I", str(REPO / "vendor/include"), "-I", sdk,
                 "-c",
                 str(REPO / "tests/fixtures/actor_attachment_contract.cpp"),
                 "-o", str(object_path)],
                capture_output=True, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([linker, str(object_path), "-o", str(output)], check=True)
            subprocess.run([str(output)], check=True)

    @unittest.skipUnless(LOCAL_CONFIG.is_file(), "local retail resources required")
    def test_shipped_third_slot_and_retail_dispatch(self) -> None:
        root = configured_retail_dir()
        consumers = []
        for floor in range(1, 6):
            chunks = length_prefixed_chunks(
                (root / f"KF/B{floor}/MIXA.DAT").read_bytes(), f"B{floor}/MIXA.DAT",
            )
            for index in range(12):
                record = chunks[6][index * 0x98:(index + 1) * 0x98]
                code = record[7]
                if code not in (0, 0xFF):
                    consumers.append((floor, index, code, record[24],
                                      struct.unpack_from("<hhh", record, 0x34)))
        self.assertEqual(consumers, [(5, 7, 56, 3, (0, -1000, -2200))])
        kind = consumers[0][2] & 0x1F
        # This row reaches the indexed coordinate loads at GAME 8002ee94.
        target = struct.unpack("<I", RetailImage.load("GAME.EXE").require(
            0x800124D4 + (kind - 5) * 4, 4,
        ))[0]
        self.assertEqual(target, 0x8002EE94)
