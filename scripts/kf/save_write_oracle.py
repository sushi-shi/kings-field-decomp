"""Compare retail/C/Rust save serialization through an in-memory write sink."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Sequence

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import (
    CandidateFunction, CandidateProgram, ExternalHook, GameSymbols, HookContext,
    MemoryInput, MemoryRange, ParserMachine, RetailProgram,
)
from scripts.kf.resource_oracle import pattern, require_equal
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.save_oracle import (
    HEADER_SIZE, HEADER_VA, PAYLOAD_SIZE, PAYLOAD_VA, SAVE_OBJECT, _cstring, _header,
    _memset, _pointer_memory,
)
from scripts.kf.sema.image import RetailImage


def words(values: Sequence[int]) -> bytes:
    return b"".join(struct.pack("<i", value) for value in values)


@dataclass
class WriteSink:
    file: bytearray
    opens: list[int]
    results: list[int]
    icons: list[bytes]
    trace: bytearray = field(default_factory=bytearray)
    offset: int = 0

    def clear(self, _context: HookContext) -> int:
        self.trace.append(1)
        return 0

    def open(self, context: HookContext) -> int:
        path = _cstring(context, context.args[0])
        if path not in (b"bu00:BISLPS-00017KF      ", b"bu00:BISLPS-00017KFTMP   "):
            raise AssertionError(f"unexpected save path: {path!r}")
        result = self.opens.pop(0)
        self.trace.extend(struct.pack("<BBIi", 2, b"TMP" in path, context.args[1], result))
        return result

    def close(self, context: HookContext) -> int:
        self.trace.extend(struct.pack("<BI", 5, context.args[0]))
        return 0

    def seek(self, context: HookContext) -> int:
        if context.args[2] != 0:
            raise AssertionError("save writer must use absolute seeks")
        self.offset = context.args[1]
        self.trace.extend(struct.pack("<BII", 3, context.args[0], self.offset))
        return self.offset

    def write(self, context: HookContext) -> int:
        size = context.args[2]
        result = self.results.pop(0) if self.results else size
        data = context.read(context.args[1], size)
        self.trace.extend(struct.pack("<BIIi", 4, context.args[0], size, result))
        self.trace.extend(data)
        if result > 0:
            count = min(result, size)
            if self.offset + count > len(self.file):
                raise AssertionError("save write escapes simulated card file")
            self.file[self.offset:self.offset + count] = data[:count]
            self.offset += count
        return result

    def erase(self, context: HookContext) -> int:
        if _cstring(context, context.args[0]) != b"bu00:BISLPS-00017KFTMP   ":
            raise AssertionError("save writer must erase only temporary card file")
        self.trace.append(7)
        return 0

    def icon(self, context: HookContext) -> int:
        path = _cstring(context, context.args[1])
        expected = f"TIM\\ICO{4 - len(self.icons)}.TIM".encode()
        if path != expected:
            raise AssertionError(f"unexpected icon {path!r}, expected {expected!r}")
        context.write(context.args[0], self.icons.pop(0))
        self.trace.extend((6, len(path)))
        self.trace.extend(path)
        return 0

    def hooks(self) -> list[ExternalHook]:
        return [ExternalHook(name, method) for name, method in (
            ("memory_card_clear_events", self.clear), ("open", self.open),
            ("close", self.close), ("lseek", self.seek), ("write", self.write),
            ("erase", self.erase), ("cd_file_load_into", self.icon), ("memset", _memset),
        )]


def compare_write_slots(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    # Different positive descriptors expose accidental handle substitution.
    cases = [(f"rotate-{slot}", slot, bytes((1, 2, 3, 4)), [3, 5, 7], [], 1)
             for slot in range(1, 5)]
    cases += [
        ("empty-entry", 2, bytes((1, 2, 3, 0)), [3, 5, 7], [], 1),
        ("missing-previous", 3, bytes((1, 2, 0, 4)), [3, 5, 7], [], 1),
        ("create", 1, bytes((1, 2, 3, 4)), [-1, 4, 5, 7], [], 1),
        ("create-fails", 1, bytes((1, 2, 3, 4)), [-1, -1, -1], [], 3),
        ("no-space", 1, bytes((1, 2, 3, 4)), [-1, -1, 4], [], 5),
        ("payload-open-fails", 1, bytes((1, 2, 3, 4)), [3, -1], [], 14),
        ("header-open-fails", 1, bytes((1, 2, 3, 4)), [3, 5, -1], [], 14),
        ("retry-both", 1, bytes((1, 2, 3, 4)), [3, 5, 7], [-1, 1, PAYLOAD_SIZE, 0, HEADER_SIZE], 1),
        ("payload-five-short", 1, bytes((1, 2, 3, 4)), [3, 5], [1] * 5, 14),
        ("header-five-short", 1, bytes((1, 2, 3, 4)), [3, 5, 7], [PAYLOAD_SIZE] + [1] * 5, 14),
    ]
    icons = [(root / f"KF/TIM/ICO{index}.TIM").read_bytes() for index in range(1, 4)]
    runtime = [(name, symbols.datum(name)[0], size) for name, size in (
        ("player_state", 224), ("map_world_state_base", 8500),
        ("DAT_800652a8", 240), ("magic_records", 480),
    )]
    sources = [pattern(size, 91 + i) for i, (_, _, size) in enumerate(runtime)]
    captures = [MemoryRange("header", HEADER_VA, HEADER_SIZE + 16),
                MemoryRange("payload", PAYLOAD_VA, PAYLOAD_SIZE + 16)]
    names = ["save_file_write_slot", "save_file_initialize_buffers"]
    for label, slot, ids, opens, results, expected in cases:
        header = bytearray(_header() + pattern(16, 88))
        header[0x200:0x204] = ids
        payload = pattern(PAYLOAD_SIZE + 16, 89)
        disk = pattern(5 * 8192, 90)
        machine = []
        for candidate in (False, True):
            service = WriteSink(bytearray(disk), opens.copy(), results.copy(), icons.copy())
            program = (CandidateProgram.link(symbols, [CandidateFunction(name, SAVE_OBJECT) for name in names],
                                             hooks=service.hooks()) if candidate else
                       RetailProgram.link(symbols, names, hooks=service.hooks()))
            output = ParserMachine(retail, program).call("save_file_write_slot", (slot,), memory=[
                *_pointer_memory(symbols), MemoryInput(HEADER_VA, bytes(header)),
                MemoryInput(PAYLOAD_VA, payload),
                *[MemoryInput(address, data) for (_, address, _), data in zip(runtime, sources)],
            ], capture=captures, allowed_writes=captures)
            if service.opens or service.results:
                raise AssertionError(f"{label}: writer did not consume scenario")
            machine.append([struct.pack("<I", output.v0), *[item.data for item in output.memory],
                            bytes(service.trace), bytes(service.file)])
        decoded = rust.call("save-write-slot", struct.pack("<h", slot), bytes(header), payload,
                            *sources, disk, words(opens), words(results), *icons)
        for index, field_name in enumerate(("status", "header", "payload", "I/O", "file")):
            require_equal(f"write-slot/{label}/{field_name}", machine[0][index], machine[1][index], decoded[index])
        if struct.unpack("<i", decoded[0])[0] != expected:
            raise AssertionError(f"{label}: unexpected status")
        print(f"[save-write-oracle] PASS {label}: full buffers, file bytes and ordered I/O; status {expected}", flush=True)
    return len(cases)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    args = parser.parse_args(argv)
    if not args.no_rebuild:
        rebuild_units(["game.save_system"])
    count = compare_write_slots(RetailImage.load("GAME.EXE"), GameSymbols.load(),
                                RustCodec(args.rust_driver or build_driver()),
                                args.retail_dir or configured_retail_dir())
    print(f"[save-write-oracle] PASS: {count} deterministic cases", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
