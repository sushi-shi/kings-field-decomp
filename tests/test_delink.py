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
from scripts.kf.retail import write_tsv


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

if __name__ == "__main__":
    unittest.main()
