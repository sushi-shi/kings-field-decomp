from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import _target_names
from scripts.kf.delink import (
    Catalog,
    Function,
    _apply_relocation,
    decode_hi_lo_target,
    decode_mips26_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
)
from scripts.kf.mips_elf import MipsRelocation, write_mips_elf
from scripts.kf.objdiff import generate_projects
from scripts.kf.retail import write_tsv


def elf_sections(data: bytes) -> dict[str, tuple[int, ...]]:
    header = struct.unpack_from("<16sHHIIIIIHHHHHH", data)
    section_offset = header[6]
    section_size = header[11]
    section_count = header[12]
    string_index = header[13]
    headers = [
        struct.unpack_from("<IIIIIIIIII", data, section_offset + index * section_size)
        for index in range(section_count)
    ]
    string_header = headers[string_index]
    strings = data[string_header[4]:string_header[4] + string_header[5]]

    def name(offset: int) -> str:
        end = strings.index(b"\0", offset)
        return strings[offset:end].decode()

    return {name(item[0]): item for item in headers[1:]}


class MipsElfTests(unittest.TestCase):
    def test_elf_relocations_and_function_extent(self) -> None:
        data = write_mips_elf(
            b"\0" * 12,
            "function",
            8,
            (
                MipsRelocation(0, "R_MIPS_26", ".text"),
                MipsRelocation(4, "R_MIPS_HI16", "external"),
                MipsRelocation(8, "R_MIPS_LO16", "external"),
            ),
        )
        header = struct.unpack_from("<16sHHIIIIIHHHHHH", data)
        self.assertEqual(header[0][:7], b"\x7fELF\x01\x01\x01")
        self.assertEqual(header[1], 1)  # ET_REL
        self.assertEqual(header[2], 8)  # EM_MIPS
        self.assertEqual(header[7], 0x1001)

        sections = elf_sections(data)
        self.assertEqual(sections[".text"][5], 12)
        rel = sections[".rel.text"]
        entries = [
            struct.unpack_from("<II", data, rel[4] + offset)
            for offset in range(0, rel[5], 8)
        ]
        self.assertEqual([info & 0xFF for _, info in entries], [4, 5, 6])
        self.assertEqual(entries[0][1] >> 8, 1)  # local .text section symbol

        symtab = sections[".symtab"]
        symbols = [
            struct.unpack_from("<IIIBBH", data, symtab[4] + offset)
            for offset in range(0, symtab[5], 16)
        ]
        self.assertEqual(symbols[2][2], 8)
        self.assertEqual(symbols[2][3] & 0xF, 2)  # STT_FUNC

    def test_mips_instruction_addends(self) -> None:
        site = 0x80010000
        target = 0x80010020
        linked_j = 0x08000000 | ((target >> 2) & 0x03FFFFFF)
        self.assertEqual(decode_mips26_target(site, linked_j), target)
        self.assertEqual(encode_mips26_addend(linked_j, 0x20) & 0x03FFFFFF, 8)

        lui = 0x3C080000 | 0x8002
        addiu = 0x25080000 | 0x9234
        self.assertEqual(decode_hi_lo_target(lui, addiu), 0x80019234)
        rewritten_hi, rewritten_lo = encode_hi_lo_addend(lui, addiu, 0x9234)
        self.assertEqual(rewritten_hi & 0xFFFF, 1)
        self.assertEqual(rewritten_lo & 0xFFFF, 0x9234)

    def test_local_jump_uses_text_section_relocation(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 16, 16, 1, "test", "test", "test"
        )
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": ()},
        )
        target = function.va + 8
        blob = bytearray(struct.pack(
            "<4I",
            0x08000000 | ((target >> 2) & 0x03FFFFFF),
            0,
            0x03E00008,
            0,
        ))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": "",
            "kind": "mips26",
            "channel": "reachable-code",
            "target_va": f"{target:#x}",
            "target_region": "load",
            "target_name": "",
            "opcode": "j",
            "confidence": "control-flow",
            "status": "candidate",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(relocations, [MipsRelocation(0, "R_MIPS_26", ".text")])
        self.assertEqual(struct.unpack_from("<I", blob)[0] & 0x03FFFFFF, 2)
        self.assertEqual(used["action"], "local-section")


class ObjdiffProjectTests(unittest.TestCase):
    def test_missing_and_present_base_pairings(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target_dir = root / "delink/game"
            object_dir = target_dir / "objects"
            object_dir.mkdir(parents=True)
            object_name = "80010000_test.o"
            vendored_name = "80010004_library_test.o"
            object_data = write_mips_elf(b"\0\0\0\0", "test", 4)
            (object_dir / object_name).write_bytes(object_data)
            (object_dir / vendored_name).write_bytes(
                write_mips_elf(b"\0\0\0\0", "library_test", 4)
            )
            write_tsv(
                target_dir / "objects.tsv",
                (
                    "image", "va", "size", "body_size", "name", "scope",
                    "provider", "library", "object", "relocations", "confidence",
                    "provenance",
                ),
                (
                    {
                        "image": "GAME.EXE",
                        "va": "0x80010000",
                        "size": "0x4",
                        "body_size": "0x4",
                        "name": "test",
                        "scope": "decomp",
                        "provider": "",
                        "library": "",
                        "object": f"objects/{object_name}",
                        "relocations": 0,
                        "confidence": "test",
                        "provenance": "test",
                    },
                    {
                        "image": "GAME.EXE",
                        "va": "0x80010004",
                        "size": "0x4",
                        "body_size": "0x4",
                        "name": "library_test",
                        "scope": "vendored",
                        "provider": "Sony",
                        "library": "LIBTEST",
                        "object": f"objects/{vendored_name}",
                        "relocations": 0,
                        "confidence": "test",
                        "provenance": "test",
                    },
                ),
                (),
            )
            output = root / "objdiff"
            self.assertEqual(
                _target_names(root / "delink", "GAME.EXE"), {object_name}
            )
            results = generate_projects(
                root / "delink", output, ("GAME.EXE",)
            )
            self.assertEqual(results["GAME.EXE"][1:], (0, 1, 1))
            project = json.loads((output / "game/objdiff.json").read_text())
            self.assertEqual(project["units"][0]["base_path"], "./missing-base.o")
            self.assertNotIn("library_test", json.dumps(project))
            excluded = (output / "game/vendored_excluded.tsv").read_text()
            self.assertIn("library_test", excluded)

            base = output / "game/base" / object_name
            base.write_bytes(object_data)
            results = generate_projects(
                root / "delink", output, ("GAME.EXE",)
            )
            self.assertEqual(results["GAME.EXE"][1:], (1, 1, 1))
            project = json.loads((output / "game/objdiff.json").read_text())
            self.assertEqual(project["units"][0]["base_path"], f"./base/{object_name}")


if __name__ == "__main__":
    unittest.main()
