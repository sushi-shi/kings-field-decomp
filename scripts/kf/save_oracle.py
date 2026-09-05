"""Three-way GAME save parser comparison with deterministic memory-card I/O.

This executes only the selected retail or freshly compiled candidate function.
BIOS/file services are explicit hooks whose ordered effects are compared with
the independent no_std Rust codec.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import (
    CandidateFunction,
    CandidateProgram,
    ExternalHook,
    GameSymbols,
    HookContext,
    MemoryInput,
    MemoryRange,
    ParserMachine,
    RetailProgram,
)
from scripts.kf.paths import BUILD
from scripts.kf.resource_oracle import pattern, require_equal
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage


HEADER_SIZE = 0x280
PAYLOAD_SIZE = 0x2580
PLAYER_SIZE = 0xE0
WORLD_SIZE = 8500
AUX_SIZE = 240
MAGIC_SIZE = 24 * 20
HEADER_VA = 0x800B0000
PAYLOAD_VA = 0x800B1000
OUTPUT_VA = 0x800B4000
SAVE_OBJECT = BUILD / "objdiff/game/base/8002b078_save_system.o"

TRACE_CLEAR = 1
TRACE_OPEN = 2
TRACE_SEEK = 3
TRACE_READ = 4
TRACE_CLOSE = 5
TRACE_ICON = 6


def io_scenario(opened: bool, results: Sequence[int]) -> bytes:
    return bytes([opened]) + struct.pack("<I", len(results)) + b"".join(
        struct.pack("<i", result) for result in results
    )


def _cstring(context: HookContext, address: int, maximum: int = 128) -> bytes:
    data = context.read(address, maximum)
    end = data.find(b"\0")
    if end == -1:
        raise AssertionError("unterminated hooked C string")
    return data[:end]


def _memset(context: HookContext) -> int:
    destination, value, size, _ = context.args
    context.write(destination, bytes([value & 255]) * size)
    return destination


@dataclass
class DeterministicIo:
    file: bytes
    opened: bool
    results: list[int]
    offset: int = 0
    result_index: int = 0
    trace: bytearray | None = None

    def __post_init__(self) -> None:
        self.trace = bytearray()

    def clear(self, _context: HookContext) -> int:
        assert self.trace is not None
        self.trace.append(TRACE_CLEAR)
        return 0

    def open(self, _context: HookContext) -> int:
        assert self.trace is not None
        self.trace.extend((TRACE_OPEN, int(self.opened)))
        return 3 if self.opened else -1

    def seek(self, context: HookContext) -> int:
        assert self.trace is not None
        self.offset = context.args[1]
        self.trace.append(TRACE_SEEK)
        self.trace.extend(struct.pack("<I", self.offset))
        return self.offset

    def read(self, context: HookContext) -> int:
        assert self.trace is not None
        requested = context.args[2]
        if self.result_index < len(self.results):
            returned = self.results[self.result_index]
        else:
            returned = requested
        self.result_index += 1
        if returned > 0:
            count = min(returned, requested, max(0, len(self.file) - self.offset))
            context.write(context.args[1], self.file[self.offset:self.offset + count])
        self.trace.append(TRACE_READ)
        self.trace.extend(struct.pack("<Ii", requested, returned))
        return returned

    def close(self, _context: HookContext) -> int:
        assert self.trace is not None
        self.trace.append(TRACE_CLOSE)
        return 0

    def hooks(self) -> list[ExternalHook]:
        return [
            ExternalHook("memory_card_clear_events", self.clear),
            ExternalHook("open", self.open),
            ExternalHook("lseek", self.seek),
            ExternalHook("read", self.read),
            ExternalHook("close", self.close),
            ExternalHook("memset", _memset),
        ]


def _program(symbols: GameSymbols, function: str, candidate: bool,
             hooks: Sequence[ExternalHook]):
    if candidate:
        return CandidateProgram.link(
            symbols, [CandidateFunction(function, SAVE_OBJECT)], hooks=hooks
        )
    return RetailProgram.link(symbols, [function], hooks=hooks)


def _pointer_memory(symbols: GameSymbols) -> list[MemoryInput]:
    return [
        MemoryInput(symbols.datum("save_header_buffer")[0], struct.pack("<I", HEADER_VA)),
        MemoryInput(symbols.datum("save_payload_buffer")[0], struct.pack("<I", PAYLOAD_VA)),
    ]


def compare_read_header(retail: RetailImage, symbols: GameSymbols, rust: RustCodec) -> int:
    file = pattern(HEADER_SIZE, 21)
    cases = [
        ("success", True, [HEADER_SIZE], 1),
        ("retry-short", True, [1, HEADER_SIZE - 1, HEADER_SIZE], 1),
        ("five-short", True, [1] * 5, 13),
        ("open-failure", False, [], 7),
    ]
    for label, opened, results, expected_status in cases:
        initial = pattern(HEADER_SIZE + 16, 3)
        capture = MemoryRange("header", HEADER_VA, len(initial))
        outputs = []
        for candidate in (False, True):
            service = DeterministicIo(file, opened, list(results))
            result = ParserMachine(
                retail, _program(symbols, "save_file_read_header", candidate, service.hooks())
            ).call(
                "save_file_read_header",
                memory=[*_pointer_memory(symbols), MemoryInput(HEADER_VA, initial)],
                capture=[capture], allowed_writes=[capture],
            )
            outputs.append((result.v0, result.memory[0].data, bytes(service.trace or b"")))
        rust_status, rust_header, rust_trace = rust.call(
            "save-read-header", initial, file, io_scenario(opened, results)
        )
        status = struct.unpack("<i", rust_status)[0]
        if outputs[0][0] != outputs[1][0] or outputs[0][0] != status or status != expected_status:
            raise AssertionError(f"read-header/{label}: status {outputs[0][0], outputs[1][0], status}")
        require_equal(f"read-header/{label}/buffer", outputs[0][1], outputs[1][1], rust_header)
        require_equal(f"read-header/{label}/I/O", outputs[0][2], outputs[1][2], rust_trace)
        print(f"[save-oracle] PASS read-header/{label}: status {status}", flush=True)
    return len(cases)


def _header() -> bytes:
    header = bytearray(pattern(HEADER_SIZE, 31))
    header[0x200:0x204] = bytes((1, 2, 3, 4))
    return bytes(header)


def _saved_payload(seed: int) -> bytes:
    return pattern(PAYLOAD_SIZE, seed)


def compare_read_slots(retail: RetailImage, symbols: GameSymbols, rust: RustCodec) -> int:
    header = _header()
    payloads = [_saved_payload(40 + index) for index in range(4)]
    file = header + b"".join(payloads)
    scenarios = [(f"slot-{slot}", slot, True, [], False, 1) for slot in range(1, 5)]
    scenarios += [
        ("retry", 2, True, [1, HEADER_SIZE, 2, PAYLOAD_SIZE], False, 1),
        ("open-failure", 1, False, [], False, 7),
        ("summary-mismatch", 1, True, [HEADER_SIZE], True, 12),
        ("missing-slot", 9, True, [], False, 7),
    ]
    player_address = symbols.datum("player_state")[0]
    world_address = symbols.datum("map_runtime_state")[0] + 0x22C
    auxiliary_address = symbols.datum("item_stock")[0]
    magic_address = symbols.datum("effect_state")[0]
    ranges = [
        MemoryRange("payload", PAYLOAD_VA, PAYLOAD_SIZE + 16),
        MemoryRange("player", player_address, PLAYER_SIZE + 16),
        MemoryRange("world", world_address, WORLD_SIZE),
        MemoryRange("aux", auxiliary_address, AUX_SIZE),
        MemoryRange("magic", magic_address, MAGIC_SIZE),
    ]
    for label, slot, opened, results, mismatch, expected_status in scenarios:
        case_file = bytearray(file)
        if mismatch:
            case_file[0x220] ^= 1
        initial = [pattern(item.size, 50 + index) for index, item in enumerate(ranges)]
        outputs = []
        for candidate in (False, True):
            service = DeterministicIo(bytes(case_file), opened, list(results))
            memory = [*_pointer_memory(symbols), MemoryInput(HEADER_VA, header)]
            memory.extend(MemoryInput(item.address, data) for item, data in zip(ranges, initial))
            result = ParserMachine(
                retail, _program(symbols, "save_file_read_slot", candidate, service.hooks())
            ).call(
                "save_file_read_slot", (slot,), memory=memory, capture=ranges,
                allowed_writes=ranges,
            )
            outputs.append((result.v0, [item.data for item in result.memory], bytes(service.trace or b"")))
        rust_blocks = rust.call(
            "save-read-slot", struct.pack("<h", slot), header,
            initial[0], initial[1], initial[2], initial[3], initial[4],
            bytes(case_file), io_scenario(opened, results),
        )
        rust_status = struct.unpack("<i", rust_blocks[0])[0]
        if outputs[0][0] != outputs[1][0] or outputs[0][0] != rust_status or rust_status != expected_status:
            raise AssertionError(f"read-slot/{label}: status {outputs[0][0], outputs[1][0], rust_status}")
        for index, name in enumerate(("payload", "player", "world", "aux", "magic")):
            require_equal(f"read-slot/{label}/{name}", outputs[0][1][index],
                          outputs[1][1][index], rust_blocks[index + 1])
        require_equal(f"read-slot/{label}/I/O", outputs[0][2], outputs[1][2], rust_blocks[-1])
        print(f"[save-oracle] PASS read-slot/{label}: status {rust_status}", flush=True)
    return len(scenarios)


def compare_catalog(retail: RetailImage, symbols: GameSymbols, rust: RustCodec) -> int:
    header = _header()
    output = MemoryRange("summaries", OUTPUT_VA, 3 * 24 + 16)
    cases = [("cached", 1), ("unavailable", 7)]
    for label, status in cases:
        initial = pattern(output.size, 71)
        machine_outputs = []
        for candidate in (False, True):
            def read_header(_context: HookContext, value: int = status) -> int:
                return value

            hooks = [
                ExternalHook("save_system_read_header", read_header),
                ExternalHook("memset", _memset),
            ]
            result = ParserMachine(
                retail, _program(symbols, "save_system_read_catalog", candidate, hooks)
            ).call(
                "save_system_read_catalog", (OUTPUT_VA,),
                memory=[*_pointer_memory(symbols), MemoryInput(HEADER_VA, header),
                        MemoryInput(OUTPUT_VA, initial)],
                capture=[output], allowed_writes=[output],
            )
            machine_outputs.append((result.v0, result.memory[0].data))
        rust_status, rust_output = rust.call("save-catalog", struct.pack("<i", status), header, initial)
        decoded = struct.unpack("<i", rust_status)[0]
        if machine_outputs[0][0] != machine_outputs[1][0] or decoded != machine_outputs[0][0]:
            raise AssertionError(f"catalog/{label}: status mismatch")
        require_equal(f"catalog/{label}", machine_outputs[0][1], machine_outputs[1][1], rust_output)
        print(f"[save-oracle] PASS catalog/{label}", flush=True)
    return len(cases)


def compare_initialize(retail: RetailImage, symbols: GameSymbols, rust: RustCodec,
                       root: Path) -> int:
    icons = [(root / f"KF/TIM/ICO{index}.TIM").read_bytes() for index in range(1, 4)]
    initial_header = pattern(HEADER_SIZE + 16, 81)
    initial_payload = pattern(PAYLOAD_SIZE + 16, 82)
    captures = [MemoryRange("header", HEADER_VA, len(initial_header)),
                MemoryRange("payload", PAYLOAD_VA, len(initial_payload))]
    machine_outputs = []
    for candidate in (False, True):
        trace = bytearray()

        def load_icon(context: HookContext) -> int:
            path = _cstring(context, context.args[1])
            expected = f"TIM\\ICO{len(trace) // 14 + 1}.TIM".encode()
            if path != expected:
                raise AssertionError(f"unexpected icon path {path!r}")
            image = icons[len(trace) // 14]
            context.write(context.args[0], image)
            trace.extend((TRACE_ICON, len(path)))
            trace.extend(path)
            return 0

        hooks = [ExternalHook("memset", _memset),
                 ExternalHook("cd_file_load_into", load_icon)]
        result = ParserMachine(
            retail, _program(symbols, "save_file_initialize_buffers", candidate, hooks)
        ).call(
            "save_file_initialize_buffers",
            memory=[*_pointer_memory(symbols), MemoryInput(HEADER_VA, initial_header),
                    MemoryInput(PAYLOAD_VA, initial_payload)],
            capture=captures, allowed_writes=captures,
        )
        machine_outputs.append(([item.data for item in result.memory], bytes(trace)))
    rust_header, rust_payload, rust_trace = rust.call(
        "save-initialize", initial_header, initial_payload, *icons
    )
    require_equal("initialize/header", machine_outputs[0][0][0], machine_outputs[1][0][0], rust_header)
    require_equal("initialize/payload", machine_outputs[0][0][1], machine_outputs[1][0][1], rust_payload)
    require_equal("initialize/icons", machine_outputs[0][1], machine_outputs[1][1], rust_trace)
    print("[save-oracle] PASS initialize: actual ICO1/2/3 extraction", flush=True)
    return 1


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--family", choices=("header", "slot", "catalog", "initialize", "all"),
                        default="all")
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    args = parser.parse_args(argv)
    if not args.no_rebuild:
        rebuild_units(["game.save_system"])
    retail = RetailImage.load("GAME.EXE")
    symbols = GameSymbols.load()
    rust = RustCodec(args.rust_driver or build_driver())
    root = args.retail_dir or configured_retail_dir()
    count = 0
    if args.family in ("header", "all"):
        count += compare_read_header(retail, symbols, rust)
    if args.family in ("slot", "all"):
        count += compare_read_slots(retail, symbols, rust)
    if args.family in ("catalog", "all"):
        count += compare_catalog(retail, symbols, rust)
    if args.family in ("initialize", "all"):
        count += compare_initialize(retail, symbols, rust, root)
    print(f"[save-oracle] PASS: {count} deterministic cases", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
