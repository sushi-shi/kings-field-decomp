import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.mips_elf import (
    RODATA_SECTION_SYMBOL,
    SECTION_SYMBOL,
    STT_FUNC,
    STT_OBJECT,
    DefinedSymbol,
    MipsRelocation,
    write_mips_elf,
)
from scripts.kf.parser_machine import (
    CODE_BASE,
    CandidateFunction,
    CandidateProgram,
    ExternalHook,
    GameSymbols,
    HookReturn,
    LinkedFunction,
    LinkedProgram,
    MemoryInput,
    MemoryPatch,
    MemoryRange,
    ParserMachine,
    ParserMachineError,
    ParserMachineFault,
    RetailProgram,
)
from scripts.kf.sema.image import RetailImage


LOAD_VA = 0x80012000
DATA_VA = 0x80090000


def words(*values: int) -> bytes:
    return struct.pack(f"<{len(values)}I", *values)


def synthetic_retail(code: bytes) -> RetailImage:
    return RetailImage.synthetic("GAME.EXE", LOAD_VA, code)


class ParserMachineTest(unittest.TestCase):
    def test_memory_input_cannot_overwrite_candidate_code_or_linked_patch(self) -> None:
        code = words(0x03E00008, 0)
        program = LinkedProgram(
            (LinkedFunction("root", CODE_BASE, len(code), "candidate"),),
            (
                MemoryPatch("candidate:root", CODE_BASE, code),
                MemoryPatch("candidate:switch-table", DATA_VA + 0x100, words(0x12345678)),
            ),
            (),
            "candidate",
        )
        machine = ParserMachine(synthetic_retail(b""), program)
        # Use the low physical alias of a range spanning 0x80100000. This is
        # the shape of the oversized VB input that previously erased code.
        with self.assertRaisesRegex(ParserMachineError, "executable function 'root'"):
            machine.call(
                "root",
                memory=[MemoryInput((CODE_BASE - 8) & 0x1FFFFFFF, bytes(16))],
            )
        with self.assertRaisesRegex(ParserMachineError, "linked patch.*switch-table"):
            machine.call(
                "root", memory=[MemoryInput(DATA_VA + 0x100, bytes(4))]
            )

    def test_memory_input_cannot_overwrite_hook_stub_but_adjacent_data_is_valid(self) -> None:
        code = words(0x03E00008, 0)
        service = LOAD_VA + 0x40
        symbols = GameSymbols(
            {"root": (LOAD_VA, len(code)), "service": (service, 12)}, {}
        )
        program = RetailProgram.link(
            symbols,
            ["root"],
            hooks=[ExternalHook("service", lambda _context: 0)],
        )
        machine = ParserMachine(synthetic_retail(code), program)
        with self.assertRaisesRegex(ParserMachineError, "hook stub 'service'"):
            machine.call("root", memory=[MemoryInput(service + 4, bytes(4))])
        result = machine.call(
            "root", memory=[MemoryInput(LOAD_VA + len(code), words(0xA5A5A5A5))]
        )
        self.assertEqual(result.v0, 0)

    def test_retail_executes_non_nop_return_delay_slot(self) -> None:
        # addiu v0, zero, 0x1234; jr ra; sw v0, 0(a0)
        code = words(0x24021234, 0x03E00008, 0xAC820000)
        symbols = GameSymbols({"return_store": (LOAD_VA, len(code))}, {})
        program = RetailProgram.link(symbols, ["return_store"])
        result = ParserMachine(synthetic_retail(code), program).call(
            "return_store",
            [DATA_VA],
            memory=[MemoryInput(DATA_VA, b"\0" * 4)],
            capture=[MemoryRange("out", DATA_VA, 4)],
            allowed_writes=[MemoryRange("out", DATA_VA, 4)],
        )
        self.assertEqual(result.v0, 0x1234)
        self.assertEqual(result.memory_by_name()["out"], words(0x1234))
        self.assertEqual(result.instructions, 3)
        self.assertEqual([item.name for item in result.trace], ["return_store"])

    def test_executed_path_load_delay_audit_rejects_hazard(self) -> None:
        # lw v0, 0(a0); addiu v1, v0, 1; jr ra; nop
        code = words(0x8C820000, 0x24430001, 0x03E00008, 0)
        symbols = GameSymbols({"hazard": (LOAD_VA, len(code))}, {})
        program = RetailProgram.link(symbols, ["hazard"])
        with self.assertRaisesRegex(ParserMachineFault, "load-delay hazard reading r2"):
            ParserMachine(synthetic_retail(code), program).call(
                "hazard", [DATA_VA], memory=[MemoryInput(DATA_VA, words(5))]
            )

    def test_scheduled_load_is_accepted(self) -> None:
        # lw v0, 0(a0); nop; jr ra; nop
        code = words(0x8C820000, 0, 0x03E00008, 0)
        symbols = GameSymbols({"safe": (LOAD_VA, len(code))}, {})
        result = ParserMachine(
            synthetic_retail(code), RetailProgram.link(symbols, ["safe"])
        ).call("safe", [DATA_VA], memory=[MemoryInput(DATA_VA, words(5))])
        self.assertEqual(result.v0, 5)

    def test_initial_non_abi_register_is_bounded_and_visible(self) -> None:
        # addu v0, s5, zero; jr ra; nop
        code = words(0x02A01021, 0x03E00008, 0)
        symbols = GameSymbols({"inherit_s5": (LOAD_VA, len(code))}, {})
        machine = ParserMachine(
            synthetic_retail(code), RetailProgram.link(symbols, ["inherit_s5"])
        )
        self.assertEqual(
            machine.call("inherit_s5", initial_gprs={21: 0x76543210}).v0,
            0x76543210,
        )
        with self.assertRaisesRegex(ValueError, "conflicts"):
            machine.call("inherit_s5", initial_gprs={4: 1})

    def test_actor_shaped_cross_block_jump_executes_target_once(self) -> None:
        # Unicorn 2.1.4's memory-write hook duplicates the first instruction at
        # 0x80030a28 in this shape from actor_pool_load_placements. Real MIPS-I
        # increments s0 once, so the two delay-slot stores are 72 bytes apart.
        base = 0x8003090C
        target = 0x80030A28
        code_words = [
            0x16B70004,  # bne s5,s7, +4 (not taken)
            0,
            0xA2560000,
            0x08000000 | ((target >> 2) & 0x03FFFFFF),
            0xA200FFEE,  # sb zero,-18(s0), jump delay
        ]
        while base + len(code_words) * 4 < target:
            code_words.append(0)
        code_words.extend(
            [
                0x26100048,  # addiu s0,s0,72 -- duplicated by raw Unicorn
                0x26520048,
                0x02801021,
                0x3042FFFF,
                0x1440FFB4,
                0x2694FFFF,
                0x03E00008,
                0,
            ]
        )
        code = words(*code_words)
        symbols = GameSymbols({"actor_loop": (base, len(code))}, {})
        result = ParserMachine(
            RetailImage.synthetic("GAME.EXE", base, code),
            RetailProgram.link(symbols, ["actor_loop"]),
        ).call(
            "actor_loop",
            memory=[MemoryInput(DATA_VA, bytes(256))],
            allowed_writes=[MemoryRange("actors", DATA_VA, 256)],
            initial_gprs={
                16: DATA_VA + 24,
                18: DATA_VA,
                20: 1,
                21: 1,
                22: 0xFF,
                23: 1,
            },
        )
        stores = [item.address for item in result.writes if item.pc == base + 0x10]
        self.assertEqual(stores, [DATA_VA + 6, DATA_VA + 78])

    def test_animation_store_delay_branch_to_jump(self) -> None:
        # Reduced allocation-success path: BNE + SW, then J + SW, then JR.
        # UC_HOOK_MEM_WRITE makes Unicorn fault before the J delay slot.
        base = 0x80020698
        code = words(
            0x14400005, 0xAE82000C, 0, 0, 0, 0,
            0x080081B7, 0xAE340000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0x03E00008, 0,
        )
        symbols = GameSymbols({"allocate": (base, len(code))}, {})
        for value in (0, 0x800A0000):
            with self.subTest(value=value):
                result = ParserMachine(
                    RetailImage.synthetic("GAME.EXE", base, code),
                    RetailProgram.link(symbols, ["allocate"]),
                ).call(
                    "allocate",
                    initial_gprs={2: value, 17: DATA_VA + 0x100, 20: DATA_VA},
                    capture=[MemoryRange("out", DATA_VA, 0x104)],
                    allowed_writes=[MemoryRange("out", DATA_VA, 0x104)],
                )
                output = result.memory_by_name()["out"]
                self.assertEqual(output[12:16], words(value))
                self.assertEqual(output[0x100:0x104], words(DATA_VA))
                self.assertEqual(
                    [(item.pc, item.address, item.value) for item in result.writes],
                    [(base + 4, DATA_VA + 12, value), (base + 28, DATA_VA + 0x100, DATA_VA)],
                )

    def test_integer_store_audit_widths_and_unaligned_merges(self) -> None:
        cases = [
            (0x28, 1, 1, b"\x11"),
            (0x29, 2, 2, b"\x11\x22"),
            (0x2B, 0, 0, b"\x11\x22\x33\x44"),
            (0x2A, 0, 0, b"\x44"),
            (0x2A, 1, 0, b"\x33\x44"),
            (0x2A, 2, 0, b"\x22\x33\x44"),
            (0x2A, 3, 0, b"\x11\x22\x33\x44"),
            (0x2E, 0, 0, b"\x11\x22\x33\x44"),
            (0x2E, 1, 1, b"\x11\x22\x33"),
            (0x2E, 2, 2, b"\x11\x22"),
            (0x2E, 3, 3, b"\x11"),
        ]
        for opcode, displacement, start, data in cases:
            with self.subTest(opcode=opcode, displacement=displacement):
                # Use a negative displacement and KSEG1 data alias too.
                instruction = (opcode << 26) | (4 << 21) | (2 << 16) | (displacement - 8 & 0xFFFF)
                code = words(instruction, 0x03E00008, 0)
                symbols = GameSymbols({"store": (LOAD_VA, len(code))}, {})
                result = ParserMachine(
                    synthetic_retail(code), RetailProgram.link(symbols, ["store"])
                ).call(
                    "store", [DATA_VA + 0x20000008], initial_gprs={2: 0x44332211},
                    memory=[MemoryInput(DATA_VA, b"\xA5" * 8)],
                    capture=[MemoryRange("out", DATA_VA, 8)],
                    allowed_writes=[MemoryRange("store", DATA_VA + start, len(data))],
                )
                expected = bytearray(b"\xA5" * 8)
                expected[start:start + len(data)] = data
                self.assertEqual(result.memory_by_name()["out"], expected)
                self.assertEqual(len(result.writes), 1)
                record = result.writes[0]
                self.assertEqual((record.address, record.size), (DATA_VA + start, len(data)))
                self.assertEqual(record.value.to_bytes(record.size, "little"), data)

    def test_store_guard_rejects_before_memory_changes(self) -> None:
        code = words(0xAC820000, 0x03E00008, 0)  # sw v0,0(a0)
        symbols = GameSymbols({"store": (LOAD_VA, len(code))}, {})
        machine = ParserMachine(synthetic_retail(code), RetailProgram.link(symbols, ["store"]))
        with self.assertRaisesRegex(ParserMachineFault, "write outside allowed ranges"):
            machine.call(
                "store", [DATA_VA], initial_gprs={2: 0x44332211},
                memory=[MemoryInput(DATA_VA, b"\xA5" * 4)], allowed_writes=[],
            )
        self.assertEqual(machine._read(DATA_VA, 4), b"\xA5" * 4)

    def test_unsupported_store_cannot_bypass_write_audit(self) -> None:
        code = words(0xE8800000, 0x03E00008, 0)  # swc2 $0,0(a0)
        symbols = GameSymbols({"store": (LOAD_VA, len(code))}, {})
        with self.assertRaisesRegex(ParserMachineError, "unsupported.*store"):
            ParserMachine(synthetic_retail(code), RetailProgram.link(symbols, ["store"])).call(
                "store", [DATA_VA]
            )

    def test_jump_to_own_delay_slot_legally_executes_slot_twice(self) -> None:
        target = LOAD_VA + 8
        code = words(
            0x24020000,
            0x08000000 | ((target >> 2) & 0x03FFFFFF),
            0x24420001,
            0x03E00008,
            0,
        )
        symbols = GameSymbols({"delay_target": (LOAD_VA, len(code))}, {})
        result = ParserMachine(
            synthetic_retail(code), RetailProgram.link(symbols, ["delay_target"])
        ).call("delay_target")
        self.assertEqual(result.v0, 2)

    def test_candidate_links_distinct_functions_and_internal_call(self) -> None:
        # root saves ra, calls helper, restores ra with one load-delay filler,
        # and helper writes its result in the jr delay slot.
        root = words(
            0x27BDFFF0,
            0xAFBF000C,
            0x0C000000,
            0,
            0x8FBF000C,
            0x27BD0010,
            0x03E00008,
            0,
        )
        helper = words(0x24820007, 0x03E00008, 0xACA20000)
        blob = write_mips_elf(
            root + helper,
            "root",
            len(root),
            [MipsRelocation(8, "R_MIPS_26", "helper")],
            [DefinedSymbol("helper", len(root), len(helper), STT_FUNC)],
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "closure.o"
            path.write_bytes(blob)
            symbols = GameSymbols(
                {"root": (LOAD_VA, len(root)), "helper": (LOAD_VA + len(root), len(helper))},
                {},
            )
            program = CandidateProgram.link(
                symbols,
                [CandidateFunction("root", path), CandidateFunction("helper", path)],
            )
            self.assertNotEqual(program.address_of("root"), program.address_of("helper"))
            result = ParserMachine(synthetic_retail(b""), program).call(
                "root",
                [4, DATA_VA],
                memory=[MemoryInput(DATA_VA, words(0))],
                capture=[MemoryRange("out", DATA_VA, 4)],
            )
        self.assertEqual(result.v0, 11)
        self.assertEqual(result.memory_by_name()["out"], words(11))
        self.assertEqual([item.name for item in result.trace], ["root", "helper"])
        self.assertTrue(all(item.kind == "candidate" for item in result.trace))

    def test_unselected_function_relocations_are_not_resolved(self) -> None:
        selected = words(0x03E00008, 0)
        unselected = words(0x0C000000, 0, 0x03E00008, 0)
        blob = write_mips_elf(
            selected + unselected,
            "copy_words",
            len(selected),
            [MipsRelocation(len(selected), "R_MIPS_26", "OpenTIM")],
            [DefinedSymbol("unused_tim", len(selected), len(unselected), STT_FUNC)],
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "multi.o"
            path.write_bytes(blob)
            symbols = GameSymbols({"copy_words": (LOAD_VA, len(selected))}, {})
            program = CandidateProgram.link(
                symbols, [CandidateFunction("copy_words", path)]
            )
        self.assertEqual(program.address_of("copy_words"), CODE_BASE)

    def test_candidate_relocates_bss_rodata_and_r32_text(self) -> None:
        # Two HI relocations sharing one LO exercise the System V MIPS REL
        # pairing rule.  The second address pair loads private .rodata.
        code = words(
            0x3C080000,
            0x3C080000,
            0x25080000,
            0x8D020000,
            0,
            0x3C090000,
            0x25290000,
            0x8D230000,
            0,
            0x03E00008,
            0,
        )
        relocs = [
            MipsRelocation(0, "R_MIPS_HI16", "state"),
            MipsRelocation(4, "R_MIPS_HI16", "state"),
            MipsRelocation(8, "R_MIPS_LO16", "state"),
            MipsRelocation(20, "R_MIPS_HI16", RODATA_SECTION_SYMBOL),
            MipsRelocation(24, "R_MIPS_LO16", RODATA_SECTION_SYMBOL),
        ]
        rodata = words(0xCAFEBABE, 0)
        blob = write_mips_elf(
            code,
            "read_owned",
            len(code),
            relocs,
            bss_size=16,
            bss_symbols=[DefinedSymbol("state", 4, 4, STT_OBJECT)],
            rodata=rodata,
            rodata_relocations=[MipsRelocation(4, "R_MIPS_32", SECTION_SYMBOL)],
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "owned.o"
            path.write_bytes(blob)
            symbols = GameSymbols(
                {"read_owned": (LOAD_VA, len(code))}, {"state": (DATA_VA + 4, 4)}
            )
            program = CandidateProgram.link(
                symbols, [CandidateFunction("read_owned", path)]
            )
            rodata_patch = next(item for item in program.patches if ".rodata" in item.name)
            self.assertEqual(struct.unpack_from("<I", rodata_patch.data, 4)[0], CODE_BASE)
            result = ParserMachine(synthetic_retail(b""), program).call(
                "read_owned",
                memory=[MemoryInput(DATA_VA + 4, words(0x12345678))],
            )
        self.assertEqual(result.v0, 0x12345678)
        self.assertEqual(result.v1, 0xCAFEBABE)

    def test_candidate_hook_receives_o32_register_and_stack_args(self) -> None:
        # Tail-call the hook so the original return address and stack arguments
        # remain the incoming O32 call frame.
        code = words(0x08000000, 0)
        blob = write_mips_elf(
            code,
            "dispatch",
            len(code),
            [MipsRelocation(0, "R_MIPS_26", "service")],
        )
        observed = []

        def service(context):
            observed.append((context.args, context.stack_args))
            return HookReturn(v0=0x55, v1=0x66)

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "hook.o"
            path.write_bytes(blob)
            symbols = GameSymbols({"dispatch": (LOAD_VA, len(code))}, {})
            program = CandidateProgram.link(
                symbols,
                [CandidateFunction("dispatch", path)],
                hooks=[ExternalHook("service", service, stack_words=2)],
            )
            result = ParserMachine(synthetic_retail(b""), program).call(
                "dispatch", [1, 2, 3, 4, 5, 6]
            )
        self.assertEqual(observed, [((1, 2, 3, 4), (5, 6))])
        self.assertEqual((result.v0, result.v1), (0x55, 0x66))
        self.assertEqual([item.kind for item in result.trace], ["candidate", "hook"])

    def test_same_unit_text_offset_resolves_declared_hook_identity(self) -> None:
        selected = words(0x08000002, 0)  # j .text+8; nop
        unused_service = words(0x03E00008, 0)
        blob = write_mips_elf(
            selected + unused_service,
            "dispatch",
            len(selected),
            [MipsRelocation(0, "R_MIPS_26", SECTION_SYMBOL)],
            [DefinedSymbol("service", len(selected), len(unused_service), STT_FUNC)],
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "section-call.o"
            path.write_bytes(blob)
            symbols = GameSymbols({"dispatch": (LOAD_VA, len(selected))}, {})
            program = CandidateProgram.link(
                symbols,
                [CandidateFunction("dispatch", path)],
                hooks=[ExternalHook("service", lambda _context: 0x77)],
            )
            result = ParserMachine(synthetic_retail(b""), program).call("dispatch")
        self.assertEqual(result.v0, 0x77)
        self.assertEqual([item.name for item in result.trace], ["dispatch", "service"])

    def test_retail_hook_stub_does_not_execute_original_target_prologue(self) -> None:
        service = LOAD_VA + 0x40
        root = words(
            0x27BDFFF0,
            0xAFBF000C,
            0x0C000000 | ((service >> 2) & 0x03FFFFFF),
            0xAC800000,  # store in JAL delay exposed Unicorn's PC-zero fault
            0x8FBF000C,
            0x27BD0010,
            0x03E00008,
            0,
        )
        payload = root + bytes(0x40 - len(root)) + words(0x27BDFFB0, 0x03E00008, 0)
        symbols = GameSymbols(
            {"root": (LOAD_VA, len(root)), "service": (service, 12)}, {}
        )
        program = RetailProgram.link(
            symbols,
            ["root"],
            hooks=[ExternalHook("service", lambda _context: 0x44)],
        )
        result = ParserMachine(synthetic_retail(payload), program).call(
            "root", [DATA_VA], memory=[MemoryInput(DATA_VA, bytes(4))]
        )
        self.assertEqual(result.v0, 0x44)
        self.assertEqual(result.pc, 0x8017FFF0)

    def test_retail_cannot_fall_into_undeclared_function(self) -> None:
        target = LOAD_VA + 16
        root = words(0x0C000000 | ((target >> 2) & 0x03FFFFFF), 0, 0x03E00008, 0)
        target_code = words(0x03E00008, 0)
        symbols = GameSymbols({"root": (LOAD_VA, len(root))}, {})
        program = RetailProgram.link(symbols, ["root"])
        with self.assertRaisesRegex(ParserMachineFault, "outside declared"):
            ParserMachine(synthetic_retail(root + target_code), program).call("root")


if __name__ == "__main__":
    unittest.main()
