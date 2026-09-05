from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import _target_names
from scripts.kf.delink import (
    Catalog,
    DataObject,
    Datum,
    Function,
    Module,
    _apply_relocation,
    _competes_for_site,
    _module_object,
    decode_hi_lo_target,
    decode_mips26_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
    load_catalog,
)
from scripts.kf.mips_elf import MipsRelocation, write_mips_elf
from scripts.kf.objdiff import generate_projects
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv, write_tsv


def elf_symbols(data: bytes) -> list[tuple[str, tuple[int, ...]]]:
    """(name, (name_off, value, size, info, other, shndx)) for every symbol."""
    sections = elf_sections(data)
    symtab = sections[".symtab"]
    strtab = sections[".strtab"]
    strings = data[strtab[4]:strtab[4] + strtab[5]]
    symbols = []
    for offset in range(0, symtab[5], 16):
        record = struct.unpack_from("<IIIBBH", data, symtab[4] + offset)
        end = strings.index(b"\0", record[0])
        symbols.append((strings[record[0]:end].decode(), record))
    return symbols


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
    def test_data_section_alignment_is_explicit_not_tail_padding(self):
        for alignment in (1, 2, 4, 8, 16):
            with self.subTest(alignment=alignment):
                blob = write_mips_elf(bytes(8), 'control', 8, data=b'abc', bss_size=3,
                                      data_alignment=alignment, bss_alignment=alignment)
                sections = elf_sections(blob)
                for name in ('.data', '.bss'):
                    self.assertEqual(sections[name][8], alignment)
                    self.assertEqual(sections[name][5], 3)
                    self.assertEqual(sections[name][4] % alignment, 0)

    def test_data_section_alignment_rejects_invalid_elf_values(self):
        for name in ('data_alignment', 'bss_alignment'):
            for alignment in (-1, 0, 3, 6, 0x100000000):
                with self.subTest(name=name, alignment=alignment), self.assertRaises(ValueError):
                    write_mips_elf(bytes(8), 'control', 8, **{name: alignment})

    def test_unreviewed_bss_candidate_does_not_compete_in_safe_policy(self) -> None:
        candidate = {"status": "candidate", "target_region": "bss"}
        reviewed = {"status": "reviewed", "target_region": "bss"}
        load = {"status": "candidate", "target_region": "load"}
        self.assertFalse(_competes_for_site(candidate, "safe"))
        self.assertTrue(_competes_for_site(reviewed, "safe"))
        self.assertTrue(_competes_for_site(load, "safe"))
        self.assertTrue(_competes_for_site(candidate, "all"))

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

    def test_reviewed_outside_load_hi_lo_pair_is_safe(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": ()},
        )
        target = 0x800A0770
        blob = bytearray(struct.pack("<2I", 0x3C02800A, 0x84420770))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": f"{target:#x}",
            "target_region": "outside-load",
            "target_name": "DAT_800a0770",
            "opcode": "lui+lh",
            "confidence": "manual-paired-pattern",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(
            relocations,
            [
                MipsRelocation(0, "R_MIPS_HI16", "DAT_800a0770"),
                MipsRelocation(4, "R_MIPS_LO16", "DAT_800a0770"),
            ],
        )
        self.assertEqual(used["action"], "paired-symbol")

    def test_reviewed_instruction_word_pair_is_safe(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        target = 0x800A0770
        datum = DataObject("GAME.EXE", target, 4, "player_value")
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": (datum,)},
        )
        blob = bytearray(struct.pack("<2I", 0x3C02800A, 0x84420770))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "instruction-word",
            "target_va": f"{target:#x}",
            "target_region": "bss",
            "target_name": "player_value",
            "opcode": "lui+lh",
            "confidence": "paired-reviewed",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(
            relocations,
            [
                MipsRelocation(0, "R_MIPS_HI16", "player_value"),
                MipsRelocation(4, "R_MIPS_LO16", "player_value"),
            ],
        )
        self.assertEqual(used["source_channel"], "instruction-word")

    def test_curated_collision_grid_pairs_delink_at_the_lui_not_the_low(self) -> None:
        # Retail GAME.EXE instruction windows, independently recorded in
        # config/evidence/game_collision_grid_relocations.md. No retail file
        # dependency: this also protects the curated rows in flake checks.
        catalog = load_catalog(RETAIL_CONFIG)
        function = catalog.function_starts["GAME.EXE"][0x8001A5AC]
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        for site, low, index, load, target, symbol in (
            (0x8001A67C, 0x24218018, 0x00220821, 0x90290000,
             0x80098018, "map_collision_grid"),
            (0x8001A6DC, 0x2421A748, 0x00310821, 0x90240000,
             0x8009A748, "map_cell_attribute_grid"),
        ):
            with self.subTest(symbol=symbol):
                matches = [row for row in rows if row["image"] == "GAME.EXE"
                           and row["kind"] == "mips_hi16_lo16"
                           and function.va <= int(row["site_va"], 0)
                           < function.va + function.size
                           and int(row["target_va"], 0) == target
                           and row["status"] != "rejected"]
                self.assertEqual(len(matches), 1)
                row = matches[0]
                self.assertEqual(int(row["site_va"], 0), site)
                self.assertEqual(int(row["paired_site_va"], 0), site + 4)
                self.assertEqual(int(row["site_file_offset"], 0),
                                 IMAGE_LAYOUTS["GAME.EXE"].file_offset(site))
                self.assertEqual(row["target_name"], symbol)
                self.assertEqual(row["status"], "reviewed")
                blob = bytearray(function.size)
                offset = site - function.va
                struct.pack_into("<4I", blob, offset, 0x3C01800A, low, index, load)

                # The old sites were four bytes late: ADDIU/ADDU is not a
                # HI16/LO16 pair, even if the row is marked reviewed.
                late = dict(row, site_va=hex(site + 4), paired_site_va=hex(site + 8))
                with self.assertRaisesRegex(ValueError, "instruction-pair-mismatch"):
                    _apply_relocation(blob, function, late, catalog, "safe")

                relocs, used = _apply_relocation(blob, function, row, catalog, "safe")
                self.assertEqual(relocs, [
                    MipsRelocation(offset, "R_MIPS_HI16", symbol),
                    MipsRelocation(offset + 4, "R_MIPS_LO16", symbol),
                ])
                self.assertEqual(used["action"], "paired-symbol")
                hi, lo, rewritten_index, rewritten_load = struct.unpack_from(
                    "<4I", blob, offset,
                )
                self.assertEqual((hi, lo), (0x3C010000, 0x24210000))
                self.assertEqual((rewritten_index, rewritten_load), (index, load))
                linked_hi, linked_lo = encode_hi_lo_addend(hi, lo, target)
                self.assertEqual((linked_hi, linked_lo), (0x3C01800A, low))
                self.assertEqual(decode_hi_lo_target(linked_hi, linked_lo), target)

    def test_reviewed_raw_pointer_word_is_safe(self) -> None:
        function = Function(
            "GAME.EXE", 0x80050000, 4, 4, 1, "table", "test", "test"
        )
        target = DataObject("GAME.EXE", 0x80012000, 0x14, "path")
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": (target,)},
        )
        blob = bytearray(struct.pack("<I", target.va))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": "",
            "kind": "mips32_candidate",
            "channel": "raw-word",
            "target_va": f"{target.va:#x}",
            "target_region": "load",
            "target_name": target.symbol,
            "opcode": ".word",
            "confidence": "pointer-reviewed",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(
            blob, function, row, catalog, "safe"
        )
        self.assertEqual(
            relocations,
            [MipsRelocation(0, "R_MIPS_32", target.symbol)],
        )
        self.assertEqual(struct.unpack("<I", blob)[0], 0)
        self.assertEqual(used["action"], "raw-word")

    def test_unreviewed_raw_pointer_word_is_not_safe(self) -> None:
        function = Function(
            "GAME.EXE", 0x80050000, 4, 4, 1, "table", "test", "test"
        )
        target = DataObject("GAME.EXE", 0x80012000, 0x14, "path")
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": (target,)},
        )
        blob = bytearray(struct.pack("<I", target.va))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": "",
            "kind": "mips32_candidate",
            "channel": "raw-word",
            "target_va": f"{target.va:#x}",
            "target_region": "load",
            "target_name": target.symbol,
            "opcode": ".word",
            "confidence": "range-only",
            "status": "candidate",
        }
        with self.assertRaisesRegex(ValueError, "non-reachable-code-channel"):
            _apply_relocation(blob, function, row, catalog, "safe")

    def test_address_named_symbol_preserves_interior_addend(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": ()},
        )
        target = 0x800A0796
        blob = bytearray(struct.pack("<2I", 0x3C02800A, 0x24420796))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": f"{target:#x}",
            "target_region": "outside-load",
            "target_name": "DAT_800a0770",
            "opcode": "lui+addiu",
            "confidence": "manual-paired-pattern",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(
            relocations,
            [
                MipsRelocation(0, "R_MIPS_HI16", "DAT_800a0770"),
                MipsRelocation(4, "R_MIPS_LO16", "DAT_800a0770"),
            ],
        )
        high, low = struct.unpack("<2I", blob)
        self.assertEqual(high & 0xFFFF, 0)
        self.assertEqual(low & 0xFFFF, 0x26)
        self.assertEqual(used["addend"], "0x00000026")

    def test_semantic_bss_owner_preserves_interior_addend(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        progress = DataObject(
            "GAME.EXE", 0x800A0788, 4, "player_progress_state"
        )
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": (progress,)},
        )
        target = progress.va + 3
        blob = bytearray(struct.pack("<2I", 0x3C02800A, 0x2442078B))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": f"{target:#x}",
            "target_region": "bss",
            "target_name": "player_progress_state",
            "opcode": "lui+addiu",
            "confidence": "paired-reviewed",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(
            relocations,
            [
                MipsRelocation(0, "R_MIPS_HI16", "player_progress_state"),
                MipsRelocation(4, "R_MIPS_LO16", "player_progress_state"),
            ],
        )
        high, low = struct.unpack("<2I", blob)
        self.assertEqual(high & 0xFFFF, 0)
        self.assertEqual(low & 0xFFFF, 3)
        self.assertEqual(used["addend"], "0x00000003")

    def test_named_owner_resolves_an_address_before_its_extent(self) -> None:
        # `&table[index - 1]` folds to table - stride: a reviewed row that
        # names the owner keeps the symbol and carries the negative addend.
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        table = DataObject("GAME.EXE", 0x8005582C, 0xA, "floor_entry_cells")
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": (table,)},
        )
        blob = bytearray(struct.pack("<2I", 0x3C038005, 0x2463582A))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": "0x8005582a",
            "target_region": "load",
            "target_name": "floor_entry_cells",
            "opcode": "lui+addiu",
            "confidence": "paired-reviewed",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(
            [item.symbol for item in relocations],
            ["floor_entry_cells", "floor_entry_cells"],
        )
        self.assertEqual(int(used["addend"], 0), -2)

    def test_candidate_outside_load_hi_lo_pair_remains_withheld(self) -> None:
        function = Function(
            "GAME.EXE", 0x80010000, 8, 8, 1, "test", "test", "test"
        )
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": ()},
        )
        blob = bytearray(struct.pack("<2I", 0x3C02800A, 0x84420770))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": "0x800a0770",
            "target_region": "outside-load",
            "target_name": "DAT_800a0770",
            "opcode": "lui+lh",
            "confidence": "paired-pattern",
            "status": "candidate",
        }
        with self.assertRaisesRegex(ValueError, "target-outside-load-image"):
            _apply_relocation(blob, function, row, catalog, "safe")


class ObjdiffProjectTests(unittest.TestCase):
    def test_missing_and_present_base_pairings(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target_dir = root / "delink/game"
            object_dir = target_dir / "objects"
            object_dir.mkdir(parents=True)
            object_name = "80010000_test.o"
            vendored_name = "80010004_library_test.o"
            module_name = "80010000_unit.o"
            object_data = write_mips_elf(b"\0\0\0\0", "test", 4)
            (object_dir / object_name).write_bytes(object_data)
            (object_dir / vendored_name).write_bytes(
                write_mips_elf(b"\0\0\0\0", "library_test", 4)
            )
            (target_dir / "modules").mkdir()
            (target_dir / "modules" / module_name).write_bytes(object_data)
            write_tsv(
                target_dir / "objects.tsv",
                (
                    "image", "va", "size", "body_size", "name", "unit", "scope",
                    "provider", "library", "object", "relocations", "confidence",
                    "provenance",
                ),
                (
                    {
                        "image": "GAME.EXE",
                        "va": "0x80010000",
                        "size": "0x4",
                        "body_size": "0x4",
                        "name": "unit",
                        "unit": "game.unit",
                        "scope": "module",
                        "provider": "",
                        "library": "",
                        "object": f"modules/{module_name}",
                        "relocations": 0,
                        "confidence": "test",
                        "provenance": "config/units.toml",
                    },
                    {
                        "image": "GAME.EXE",
                        "va": "0x80010000",
                        "size": "0x4",
                        "body_size": "0x4",
                        "name": "test",
                        "unit": "",
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
                        "unit": "",
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
                _target_names(root / "delink", "GAME.EXE"), {object_name, module_name}
            )
            results = generate_projects(
                root / "delink", output, ("GAME.EXE",)
            )
            self.assertEqual(results["GAME.EXE"][1:], (0, 1, 1))
            project = json.loads((output / "game/objdiff.json").read_text())
            self.assertEqual(project["units"], [])
            self.assertNotIn("library_test", json.dumps(project))
            pairings = (output / "game/pairings.tsv").read_text()
            self.assertIn("unstarted", pairings)
            excluded = (output / "game/vendored_excluded.tsv").read_text()
            self.assertIn("library_test", excluded)

            base = output / "game/base" / module_name
            base.write_bytes(object_data)
            results = generate_projects(
                root / "delink", output, ("GAME.EXE",)
            )
            self.assertEqual(results["GAME.EXE"][1:], (1, 1, 1))
            project = json.loads((output / "game/objdiff.json").read_text())
            self.assertEqual(project["units"][0]["name"], "game.unit")
            self.assertEqual(project["units"][0]["base_path"], f"./base/{module_name}")
            self.assertEqual(
                project["units"][0]["target_path"], f"../../delink/game/modules/{module_name}"
            )


FUNCTION_COLUMNS = (
    "image", "va", "file_offset", "size", "body_size", "fragments", "kind",
    "confidence", "name", "labels", "provenance", "note",
)
VENDORED_COLUMNS = (
    "image", "va", "size", "name", "aliases", "provider", "library", "module",
    "member_offset", "source_version", "evidence", "confidence", "provenance",
    "note",
)
RELOC_COLUMNS = (
    "image", "site_va", "site_file_offset", "paired_site_va", "kind", "channel",
    "target_va", "target_region", "target_name", "opcode", "register",
    "confidence", "status", "provenance",
)
DATA_COLUMNS = (
    "image", "va", "file_offset", "size", "kind", "confidence", "name",
    "datatype", "provenance", "note",
)


def _function_row(va: int, name: str) -> dict[str, str]:
    return {
        "image": "GAME.EXE", "va": f"{va:#x}", "file_offset": f"{va - 0x8000f800:#x}",
        "size": "0x10", "body_size": "0x10", "fragments": "1", "kind": "function",
        "confidence": "test", "name": name, "labels": "", "provenance": "test",
        "note": "",
    }


def _vendored_row(va: int, name: str, library: str) -> dict[str, str]:
    return {
        "image": "GAME.EXE", "va": f"{va:#x}", "size": "0x10", "name": name,
        "aliases": "", "provider": "Sony", "library": library, "module": "X",
        "member_offset": "0x0", "source_version": "test", "evidence": "test",
        "confidence": "test", "provenance": "test", "note": "",
    }


def _jal_row(site: int, target: int, status: str = "candidate") -> dict[str, str]:
    return {
        "image": "GAME.EXE", "site_va": f"{site:#x}",
        "site_file_offset": f"{site - 0x8000f800:#x}", "paired_site_va": "",
        "kind": "mips26", "channel": "reachable-code", "target_va": f"{target:#x}",
        "target_region": "load", "target_name": "", "opcode": "jal", "register": "",
        "confidence": "control-flow", "status": status, "provenance": "test",
    }


class CatalogNamingTests(unittest.TestCase):
    def _catalog(self, relocation_rows: tuple[dict[str, str], ...]) -> Catalog:
        directory = tempfile.mkdtemp(prefix="kf-catalog-")
        config = Path(directory)
        write_tsv(config / "functions.tsv", FUNCTION_COLUMNS, (
            _function_row(0x80010000, "game_caller"),
            _function_row(0x80050000, "memset"),
            _function_row(0x80050100, "memset"),
            _function_row(0x80050200, "helper"),
        ), ())
        write_tsv(config / "functions_vendored.tsv", VENDORED_COLUMNS, (
            _vendored_row(0x80050000, "memset", "LIBAPI.LIB"),
            _vendored_row(0x80050100, "memset", "LIBGPU.LIB"),
            _vendored_row(0x80050200, "helper", "LIBGPU.LIB"),
        ), ())
        write_tsv(config / "relocs.tsv", RELOC_COLUMNS, relocation_rows, ())
        write_tsv(config / "data.tsv", DATA_COLUMNS, (), ())
        return load_catalog(config)

    def test_duplicate_name_goes_to_the_instance_game_code_calls(self) -> None:
        catalog = self._catalog((
            _jal_row(0x80010004, 0x80050000),
            _jal_row(0x80050204, 0x80050100),  # a vendored-internal call does not count
        ))
        starts = catalog.function_starts["GAME.EXE"]
        self.assertEqual(starts[0x80050000].symbol, "memset")
        self.assertEqual(starts[0x80050100].symbol, "memset_80050100")
        self.assertEqual(starts[0x80050200].symbol, "helper")

    def test_duplicate_name_without_game_call_evidence_stays_qualified(self) -> None:
        catalog = self._catalog((
            _jal_row(0x80010004, 0x80050000, status="rejected"),
        ))
        starts = catalog.function_starts["GAME.EXE"]
        self.assertEqual(starts[0x80050000].symbol, "memset_80050000")
        self.assertEqual(starts[0x80050100].symbol, "memset_80050100")

    def test_duplicate_name_called_at_both_bodies_stays_qualified(self) -> None:
        catalog = self._catalog((
            _jal_row(0x80010004, 0x80050000),
            _jal_row(0x80010008, 0x80050100),
        ))
        starts = catalog.function_starts["GAME.EXE"]
        self.assertEqual(starts[0x80050000].symbol, "memset_80050000")
        self.assertEqual(starts[0x80050100].symbol, "memset_80050100")


class ModuleObjectTests(unittest.TestCase):
    def test_module_concatenates_functions_and_rebases_section_jumps(self) -> None:
        first = Function("GAME.EXE", 0x80010000, 8, 8, 1, "first", "test", "test")
        second = Function("GAME.EXE", 0x80010008, 12, 8, 1, "second", "test", "test")
        # second: `j 0x80010008` (own start, section-relative addend 0) + nop + pad
        second_text = struct.pack("<3I", 0x08000000 | (0x80010008 >> 2), 0, 0)
        carved = {
            0x80010000: (struct.pack("<2I", 0x03E00008, 0), []),
            0x80010008: (
                second_text,
                [MipsRelocation(0, "R_MIPS_26", ".text")],
            ),
        }
        # the delinker stores the section-relative addend inside the word
        rebased = bytearray(second_text)
        rebased[0:4] = struct.pack("<I", encode_mips26_addend(0x08000000, 0))
        carved[0x80010008] = (bytes(rebased), carved[0x80010008][1])
        module = Module("GAME.EXE", "game.pair", "pair", (0x80010000, 0x80010008))
        built = _module_object(module, {first.va: first, second.va: second}, carved)
        self.assertEqual(built.size, 20)
        self.assertEqual(built.body_size, 16)
        self.assertEqual(built.relocations, [MipsRelocation(8, "R_MIPS_26", ".text")])
        self.assertEqual((built.data_size, built.bss_size), (0, 0))
        sections = elf_sections(built.data)
        self.assertNotIn(".data", sections)
        self.assertNotIn(".bss", sections)
        text = built.data[sections[".text"][4]:sections[".text"][4] + sections[".text"][5]]
        word = struct.unpack_from("<I", text, 8)[0]
        self.assertEqual((word & 0x03FFFFFF) << 2, 8)  # rebased to the module offset

    def test_module_carries_claimed_data_and_bss(self) -> None:
        function = Function("GAME.EXE", 0x80010000, 8, 8, 1, "only", "test", "test")
        carved = {0x80010000: (struct.pack("<2I", 0x03E00008, 0), [])}
        module = Module(
            "GAME.EXE", "game.unit", "unit", (0x80010000,),
            (
                Datum(0x80050000, 4, "counter", "load", "static"),
                Datum(0x80050005, 1, "flag", "load", ""),
                Datum(0x80050008, 4, "word", "load", ""),
                Datum(0x800A0000, 0x10, "buffer", "bss", ""),
                Datum(0x800A0011, 1, "byte", "bss", "static"),
                Datum(0x800A0014, 4, "last", "bss", ""),
            ),
        )
        blobs = {
            0x80050000: (b"\x01\x02\x03\x04", [MipsRelocation(0, "R_MIPS_32", "callee")]),
            0x80050005: (b"\x05", []),
            0x80050008: (b"\x09\x0a\x0b\x0c", []),
        }
        built = _module_object(module, {function.va: function}, carved, blobs)
        # Claims pack in order with their retail alignment: the byte follows the
        # word directly, the next word waits for a 4-byte boundary.
        self.assertEqual(built.data_size, 12)
        self.assertEqual(built.bss_size, 0x18)
        self.assertEqual(built.relocations, [MipsRelocation(0, "R_MIPS_32", "callee")])
        sections = elf_sections(built.data)
        data_header = sections[".data"]
        self.assertEqual(built.data[data_header[4]:data_header[4] + data_header[5]],
                         b"\x01\x02\x03\x04\x05\0\0\0\x09\x0a\x0b\x0c")
        self.assertEqual(sections[".bss"][1], 8)  # SHT_NOBITS
        self.assertEqual(sections[".bss"][5], 0x18)
        rel_data = sections[".rel.data"]
        self.assertEqual(rel_data[5], 8)
        symbols = elf_symbols(built.data)
        names = {name: symbol for name, symbol in symbols}
        self.assertEqual(names["counter"][3] >> 4, 0)   # STB_LOCAL from scope=static
        self.assertEqual(names["counter"][3] & 0xF, 1)  # STT_OBJECT
        self.assertEqual((names["counter"][1], names["counter"][2]), (0, 4))
        self.assertEqual(names["flag"][3] >> 4, 1)      # STB_GLOBAL
        self.assertEqual((names["flag"][1], names["flag"][2]), (4, 1))
        self.assertEqual((names["word"][1], names["word"][2]), (8, 4))
        self.assertEqual((names["buffer"][1], names["buffer"][2]), (0, 0x10))
        self.assertEqual((names["byte"][1], names["byte"][2]), (0x10, 1))
        self.assertEqual((names["last"][1], names["last"][2]), (0x14, 4))
        section_names = list(sections)
        self.assertEqual(names["flag"][5], section_names.index(".data") + 1)
        self.assertEqual(names["buffer"][5], section_names.index(".bss") + 1)
        self.assertEqual(names["only"][5], section_names.index(".text") + 1)
        self.assertEqual(names["callee"][5], 0)  # undefined

    def test_module_preserves_static_bss_eight_byte_alignment(self) -> None:
        function = Function("GAME.EXE", 0x80010000, 8, 8, 1, "only", "test", "test")
        carved = {0x80010000: (struct.pack("<2I", 0x03E00008, 0), [])}
        module = Module(
            "GAME.EXE", "game.unit", "unit", (0x80010000,),
            (
                Datum(0x80058020, 4, "first", "bss", "static"),
                Datum(0x80058028, 4, "second", "bss", "static"),
                Datum(0x8006BD88, 4, "third", "bss", "static"),
            ),
        )
        built = _module_object(module, {function.va: function}, carved)
        self.assertEqual(built.bss_size, 0x14)
        sections = elf_sections(built.data)
        self.assertEqual(sections[".bss"][5], 0x14)
        self.assertEqual(sections[".bss"][8], 8)
        names = {name: symbol for name, symbol in elf_symbols(built.data)}
        self.assertEqual(names["first"][1], 0)
        self.assertEqual(names["second"][1], 8)
        self.assertEqual(names["third"][1], 0x10)

    def test_exported_bss_preserves_retail_bank_allocation_gap_without_an_extra_global(self):
        function = Function('GAME.EXE', 0x80010000, 8, 8, 1, 'control', 'test', 'test')
        module = Module('GAME.EXE', 'game.control', 'control', (function.va,), (
            Datum(0x80059400, 180, 'names', 'bss', 'global'),
            Datum(0x800594B8, 320, 'buy', 'bss', 'global'),
            Datum(0x800595F8, 320, 'sell', 'bss', 'global'),
        ))
        built = _module_object(module, {function.va: function}, {function.va: (bytes(8), [])})
        names = {name: record for name, record in elf_symbols(built.data)}
        self.assertEqual([(names[name][1], names[name][2]) for name in ('names', 'buy', 'sell')],
                         [(0, 180), (184, 320), (504, 320)])
        self.assertEqual((built.bss_size, elf_sections(built.data)['.bss'][8]), (824, 8))
        self.assertEqual(Datum(0x800594B4, 4, 'word', 'bss', 'global').alignment, 4)
        self.assertEqual(Datum(0x800594B8, 4, 'word', 'load', 'global').alignment, 4)

    def test_module_section_alignment_follows_existing_claim_packing(self):
        function = Function('GAME.EXE', 0x80010000, 8, 8, 1, 'control', 'test', 'test')
        for alignment in (1, 2, 4, 8):
            with self.subTest(alignment=alignment):
                data_va, bss_va = 0x80050000 + alignment, 0x800A0000 + alignment
                module = Module('GAME.EXE', 'game.control', 'control', (function.va,), (
                    Datum(data_va, 3, 'initialized', 'load'),
                    Datum(bss_va, 3, 'tentative', 'bss', 'static'),
                ))
                built = _module_object(module, {function.va: function},
                                       {function.va: (bytes(8), [])}, {data_va: (b'abc', [])})
                sections = elf_sections(built.data)
                self.assertEqual(sections['.data'][8], min(alignment, 4))
                self.assertEqual(sections['.bss'][8], alignment)
                self.assertEqual((built.data_size, built.bss_size), (3, 3))

    def test_section_alignment_keeps_the_strongest_member_constraint(self):
        function = Function('GAME.EXE', 0x80010000, 8, 8, 1, 'control', 'test', 'test')
        module = Module('GAME.EXE', 'game.control', 'control', (function.va,), (
            Datum(0x800A0001, 1, 'byte', 'bss'),
            Datum(0x800A0004, 4, 'word', 'bss'),
        ))
        built = _module_object(module, {function.va: function}, {function.va: (bytes(8), [])})
        self.assertEqual(elf_sections(built.data)['.bss'][8], 4)
        # This contradictory owner remains unplaceable; an odd first claim
        # does not authorize weakening a later member's packing constraint.
        self.assertEqual({name: record[1] for name, record in elf_symbols(built.data)
                          if name in {'byte', 'word'}}, {'byte': 0, 'word': 4})


class ModuleRodataTests(unittest.TestCase):
    def test_module_rodata_rebases_in_module_code_pointers(self) -> None:
        first = Function("GAME.EXE", 0x80010000, 8, 8, 1, "first", "test", "test")
        second = Function("GAME.EXE", 0x80010008, 8, 8, 1, "second", "test", "test")
        carved = {
            0x80010000: (struct.pack("<2I", 0x03E00008, 0), []),
            0x80010008: (struct.pack("<2I", 0x03E00008, 0), []),
        }
        module = Module(
            "GAME.EXE", "game.pair", "pair", (0x80010000, 0x80010008), (), (0x80012000, 0x10)
        )
        # a two-entry jump table into the module followed by a string literal
        blob = struct.pack("<2I", 0x80010008, 0x80010000) + b"TIM\0\0\0\0\0"
        built = _module_object(
            module, {first.va: first, second.va: second}, carved, {}, blob
        )
        self.assertEqual(built.rodata_size, 0x10)
        sections = elf_sections(built.data)
        rodata = built.data[sections[".rodata"][4]:sections[".rodata"][4] + sections[".rodata"][5]]
        self.assertEqual(struct.unpack_from("<2I", rodata), (8, 0))
        self.assertEqual(rodata[8:11], b"TIM")
        self.assertEqual(
            [item for item in built.relocations if item.symbol == ".text"],
            [MipsRelocation(0, "R_MIPS_32", ".text"), MipsRelocation(4, "R_MIPS_32", ".text")],
        )
        self.assertIn(".rel.rodata", sections)

    def test_module_switch_table_rodata_ends_at_the_retail_claim(self) -> None:
        function = Function("GAME.EXE", 0x80010000, 8, 8, 1, "first", "test", "test")
        carved = {0x80010000: (struct.pack("<2I", 0x03E00008, 0), [])}
        module = Module(
            "GAME.EXE", "game.switch", "switch", (0x80010000,), (),
            (0x80012000, 0x0C),
        )
        blob = struct.pack("<I", 0x80010000) + b"literal\0"
        built = _module_object(module, {function.va: function}, carved, {}, blob)
        self.assertEqual(built.rodata_size, 0x0C)
        sections = elf_sections(built.data)
        header = sections[".rodata"]
        rodata = built.data[header[4]:header[4] + header[5]]
        self.assertEqual(header[5], 0x0C)
        self.assertEqual(rodata, struct.pack("<I", 0) + b"literal\0")

    def test_code_reference_inside_the_claimed_range_resolves_to_rodata(self) -> None:
        function = Function("GAME.EXE", 0x80010000, 8, 8, 1, "first", "test", "test")
        catalog = Catalog(
            functions={"GAME.EXE": (function,)},
            function_starts={"GAME.EXE": {function.va: function}},
            data={"GAME.EXE": ()},
        )
        target = 0x80012008
        blob = bytearray(struct.pack("<2I", 0x3C018001, 0x24212008))
        row = {
            "image": "GAME.EXE",
            "site_va": f"{function.va:#x}",
            "paired_site_va": f"{function.va + 4:#x}",
            "kind": "mips_hi16_lo16",
            "channel": "reachable-code",
            "target_va": f"{target:#x}",
            "target_region": "load",
            "target_name": "DAT_80012008",
            "opcode": "lui+addiu",
            "confidence": "paired-pattern",
            "status": "candidate",
        }
        relocations, used = _apply_relocation(
            blob, function, row, catalog, "safe", (0x80012000, 0x10)
        )
        self.assertEqual(
            relocations,
            [MipsRelocation(0, "R_MIPS_HI16", ".rodata"), MipsRelocation(4, "R_MIPS_LO16", ".rodata")],
        )
        self.assertEqual(struct.unpack_from("<I", blob, 4)[0] & 0xFFFF, 8)


if __name__ == "__main__":
    unittest.main()
