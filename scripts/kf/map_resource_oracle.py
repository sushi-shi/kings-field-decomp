"""Three-way oracle for GAME's MIXA/MIXB outer resource walk.

Retail and reconstructed C execute ``map_resources_load`` plus its path and
word-copy helpers in isolated GAME RAM.  Inner format consumers and platform
services are deterministic callback hooks; their grammars are covered by the
dedicated resource oracles.  The independent no_std Rust implementation sees
the complete files and emits the same typed callback trace and five grids.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import (
    CallRecord,
    CandidateFunction,
    CandidateProgram,
    ExternalHook,
    GameSymbols,
    HookContext,
    LinkedProgram,
    MemoryInput,
    MemoryRange,
    ParserMachine,
    RetailProgram,
)
from scripts.kf.paths import BUILD
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage


FILE_VA = 0x800A8000
TIM_VA = 0x800A7000
VARIANT_VA = 0x801A0000
GRID_SIZE = 10_000
ALL_GRIDS_SIZE = GRID_SIZE * 5
RESOURCES_OBJECT = BUILD / "objdiff/game/base/8001b100_resources.o"


@dataclass(frozen=True)
class PayloadSpan:
    index: int
    header_offset: int
    payload_offset: int
    size: int


@dataclass(frozen=True)
class SparseResourceFile:
    """Complete host bytes plus only the ranges read by the outer MIPS code."""

    name: str
    data: bytes
    payloads: tuple[PayloadSpan, ...]
    header_count: int
    grid_chunk: int | None = None

    @classmethod
    def parse(
        cls,
        name: str,
        data: bytes,
        *,
        chunks: int,
        header_count: int,
        grid_chunk: int | None = None,
    ) -> "SparseResourceFile":
        at = 0
        payloads = []
        for index in range(chunks):
            if at + 4 > len(data):
                raise ValueError(f"{name}: chunk {index} has no complete length header")
            size = struct.unpack_from("<I", data, at)[0]
            end = at + 4 + size
            if end > len(data):
                raise ValueError(
                    f"{name}: chunk {index} declares {size:#x} bytes, "
                    f"only {len(data) - at - 4:#x} available"
                )
            payloads.append(PayloadSpan(index, at, at + 4, size))
            at = end
        if at != len(data):
            raise ValueError(f"{name}: {len(data) - at:#x} bytes remain after {chunks} chunks")
        if not 0 <= header_count <= chunks:
            raise ValueError(f"{name}: invalid sparse header count {header_count}")
        if grid_chunk is not None and payloads[grid_chunk].size < ALL_GRIDS_SIZE:
            raise ValueError(
                f"{name}: grid chunk has {payloads[grid_chunk].size} bytes, "
                f"needs {ALL_GRIDS_SIZE}"
            )
        return cls(name, data, tuple(payloads), header_count, grid_chunk)

    @property
    def consumed(self) -> int:
        return len(self.data)

    def sparse_inputs(self, base: int = FILE_VA) -> tuple[MemoryInput, ...]:
        inputs = [
            MemoryInput(base + span.header_offset, self.data[span.header_offset:span.payload_offset])
            for span in self.payloads[:self.header_count]
        ]
        if self.grid_chunk is not None:
            span = self.payloads[self.grid_chunk]
            inputs.append(
                MemoryInput(
                    base + span.payload_offset,
                    self.data[span.payload_offset:span.payload_offset + ALL_GRIDS_SIZE],
                )
            )
        return tuple(inputs)

    def sparse_ranges(self, base: int = FILE_VA) -> tuple[MemoryRange, ...]:
        return tuple(
            MemoryRange(f"{self.name}:load:{index}", item.address, len(item.data))
            for index, item in enumerate(self.sparse_inputs(base))
        )

    def describe_pointer(self, pointer: int, base: int = FILE_VA) -> PayloadSpan:
        offset = pointer - base
        for payload in self.payloads:
            if payload.payload_offset == offset:
                return payload
        raise AssertionError(f"{self.name}: callback pointer {pointer:#010x} is not a chunk payload")


def _append_payload(trace: bytearray, payload: PayloadSpan) -> None:
    trace.extend(struct.pack("<BII", payload.index, payload.payload_offset, payload.size))


def normalize_trace(
    calls: Iterable[CallRecord],
    mixa: SparseResourceFile,
    mixb: SparseResourceFile,
    loaded_files: Sequence[int],
) -> bytes:
    """Translate machine calls into the Rust driver's stable event protocol."""

    trace = bytearray()
    loads = iter(loaded_files)
    releases = 0
    payload_calls = {
        "item_load_floor_placements": (10, mixa),
        "map_object_pool_load": (11, mixa),
        "actor_pool_load_placements": (12, mixa),
        "actor_definitions_load": (13, mixa),
        "map_event_pool_load": (14, mixa),
    }
    simple = {
        "audio_stop_sequence_fade": 1,
        "effect_pool_reset": 2,
        "memory_allocation_reset": 3,
        "tim_upload_images": 6,
        "audio_play_current_map_sequence": 9,
        "map_variant_assets_load": 19,
        "player_sync_position_to_map": 20,
    }
    ignored = {
        "map_resources_load",
        "map_resource_load_file",
        "map_resource_copy_words",
        "strcpy",
    }
    for call in calls:
        if call.name in ignored:
            continue
        if call.name in simple:
            trace.append(simple[call.name])
        elif call.name == "map_resource_path_set_floor":
            trace.extend(struct.pack("<Bi", 4, _signed(call.args[0])))
        elif call.name == "cd_file_load_allocated":
            try:
                file_id = next(loads)
            except StopIteration as error:
                raise AssertionError("more file-load calls than observed file paths") from error
            trace.extend((5, file_id))
        elif call.name == "memory_release_last":
            releases += 1
            trace.append(7)
            if releases == 2:
                vab_body_header = mixa.payloads[1].header_offset
                trace.extend(struct.pack("<BI", 15, vab_body_header + 16))
        elif call.name == "audio_load_vab":
            trace.append(8)
            _append_payload(trace, mixa.describe_pointer(call.args[0]))
            _append_payload(trace, mixa.describe_pointer(call.args[1]))
        elif call.name in payload_calls:
            tag, resource = payload_calls[call.name]
            trace.append(tag)
            _append_payload(trace, resource.describe_pointer(call.args[0]))
        elif call.name == "tmd_register":
            trace.extend(struct.pack("<BH", 16, call.args[0] & 0xFFFF))
            _append_payload(trace, mixb.describe_pointer(call.args[1]))
        elif call.name == "asset_registry_load_tmd_archive":
            trace.extend(struct.pack("<BH", 17, call.args[0] & 0xFFFF))
            _append_payload(trace, mixb.describe_pointer(call.args[1]))
        elif call.name == "memory_allocate":
            trace.extend(struct.pack("<BI", 18, call.args[0]))
        elif call.name == "memory_set_allocation_mode":
            trace.extend(struct.pack("<Bi", 21, _signed(call.args[0])))
        else:
            raise AssertionError(f"unexpected outer-loader call {call.name!r}")
    try:
        extra = next(loads)
    except StopIteration:
        pass
    else:
        raise AssertionError(f"observed file load {extra} has no machine call")
    return bytes(trace)


def _signed(value: int) -> int:
    return value if value < 0x80000000 else value - 0x100000000


def _cstring(context: HookContext, address: int, limit: int = 64) -> bytes:
    value = context.read(address, limit)
    end = value.find(b"\0")
    if end < 0:
        raise AssertionError(f"unterminated string at {address:#010x}")
    return value[:end]


def _ranges_overlap(left: MemoryRange, right_address: int, right_size: int) -> bool:
    return left.address < right_address + right_size and right_address < left.address + left.size


def verify_sparse_layout(
    program: LinkedProgram,
    sparse_ranges: Iterable[MemoryRange],
) -> None:
    """Reject a corpus layout that would overwrite linked candidate code/data."""

    for sparse in sparse_ranges:
        for function in program.functions:
            if _ranges_overlap(sparse, function.address, function.size):
                raise AssertionError(
                    f"{sparse.name} overlaps linked {function.name} at {function.address:#010x}"
                )
        for patch in program.patches:
            if _ranges_overlap(sparse, patch.address, len(patch.data)):
                raise AssertionError(
                    f"{sparse.name} overlaps {patch.name} at {patch.address:#010x}"
                )


def _pattern(size: int, seed: int) -> bytes:
    return bytes((at * 73 + (at >> 3) * 19 + seed * 41 + 17) & 0xFF for at in range(size))


def _require_equal(label: str, retail: bytes, candidate: bytes, rust: bytes) -> None:
    for version, result in (("C", candidate), ("Rust", rust)):
        if result != retail:
            first = next(
                (index for index, pair in enumerate(zip(retail, result)) if pair[0] != pair[1]),
                min(len(retail), len(result)),
            )
            raise AssertionError(
                f"{label}: retail/{version} mismatch at byte {first:#x}; "
                f"lengths {len(retail)}/{len(result)}; "
                f"retail={retail[first:first + 16].hex()} "
                f"{version}={result[first:first + 16].hex()}"
            )


def compare_floor(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    resources_root: Path,
    floor: int,
) -> None:
    floor_root = resources_root / f"B{floor}"
    mixa = SparseResourceFile.parse(
        f"B{floor}/MIXA.DAT",
        (floor_root / "MIXA.DAT").read_bytes(),
        chunks=8,
        header_count=7,
        grid_chunk=2,
    )
    mixb_chunks = 4 if floor == 5 else 5
    mixb = SparseResourceFile.parse(
        f"B{floor}/MIXB.DAT",
        (floor_root / "MIXB.DAT").read_bytes(),
        chunks=mixb_chunks,
        header_count=4,
    )
    use_external_variant = floor == 5
    path_address, path_size = symbols.datum("map_resource_path")
    cursor_address, cursor_size = symbols.datum("memory_arena_cursor")
    variant_address, variant_size = symbols.datum("map_variant_asset_buffer")
    grid_names = (
        "map_cell_attribute_grid",
        "map_floor_height_grid",
        "map_cell_orientation_grid",
        "map_collision_flag_grid",
        "map_collision_grid",
    )
    captures = [MemoryRange(name, symbols.datum(name)[0], GRID_SIZE) for name in grid_names]
    captures += [
        MemoryRange("path", path_address, path_size),
        MemoryRange("arena cursor", cursor_address, cursor_size),
        MemoryRange("variant buffer", variant_address, variant_size),
    ]
    initial_grids = _pattern(ALL_GRIDS_SIZE, floor)
    path_initial = b"B0\\" + bytes(path_size - 3)
    cursor_initial = 0x1357_2468
    variant_initial = 0x2468_1357
    initial_memory = [
        *(
            MemoryInput(capture.address, initial_grids[index * GRID_SIZE:(index + 1) * GRID_SIZE])
            for index, capture in enumerate(captures[:5])
        ),
        MemoryInput(path_address, path_initial),
        MemoryInput(cursor_address, struct.pack("<I", cursor_initial)),
        MemoryInput(variant_address, struct.pack("<I", variant_initial)),
    ]
    functions = [
        CandidateFunction(name, RESOURCES_OBJECT)
        for name in (
            "map_resource_path_set_floor",
            "map_resource_load_file",
            "map_resource_copy_words",
            "map_resources_load",
        )
    ]
    outputs = []
    for candidate in (False, True):
        loads: list[int] = []

        def strcpy(context: HookContext) -> int:
            value = _cstring(context, context.args[1]) + b"\0"
            context.write(context.args[0], value)
            return context.args[0]

        def load(context: HookContext) -> int:
            path = _cstring(context, context.args[1])
            if path.endswith(b"MIX.TIM"):
                loads.append(0)
                pointer = TIM_VA
            elif path.endswith(b"MIXA.DAT"):
                loads.append(1)
                pointer = FILE_VA
                for item in mixa.sparse_inputs():
                    context.write(item.address, item.data)
            elif path.endswith(b"MIXB.DAT"):
                loads.append(2)
                pointer = FILE_VA
                for item in mixb.sparse_inputs():
                    context.write(item.address, item.data)
            else:
                raise AssertionError(f"B{floor}: unexpected resource path {path!r}")
            context.write_u32(context.args[0], pointer)
            return 0

        def allocate(context: HookContext) -> int:
            return VARIANT_VA

        def no_op(_context: HookContext) -> int:
            return 0

        hooks = [
            ExternalHook("strcpy", strcpy),
            ExternalHook("cd_file_load_allocated", load),
            ExternalHook("audio_stop_sequence_fade", no_op),
            ExternalHook("effect_pool_reset", no_op),
            ExternalHook("memory_allocation_reset", no_op),
            ExternalHook("tim_upload_images", no_op),
            ExternalHook("memory_release_last", no_op),
            ExternalHook("audio_load_vab", no_op),
            ExternalHook("audio_play_current_map_sequence", no_op),
            ExternalHook("item_load_floor_placements", no_op),
            ExternalHook("map_object_pool_load", no_op),
            ExternalHook("actor_pool_load_placements", no_op),
            ExternalHook("actor_definitions_load", no_op),
            ExternalHook("map_event_pool_load", no_op),
            ExternalHook("tmd_register", no_op),
            ExternalHook("asset_registry_load_tmd_archive", no_op),
            ExternalHook("memory_allocate", allocate),
            ExternalHook("map_variant_assets_load", no_op),
            ExternalHook("player_sync_position_to_map", no_op),
            ExternalHook("memory_set_allocation_mode", no_op),
        ]
        if candidate:
            program = CandidateProgram.link(symbols, functions, hooks=hooks)
            verify_sparse_layout(program, (*mixa.sparse_ranges(), *mixb.sparse_ranges()))
        else:
            program = RetailProgram.link(symbols, [item.name for item in functions], hooks=hooks)
        allowed = [*captures, *mixa.sparse_ranges(), *mixb.sparse_ranges()]
        result = ParserMachine(retail, program).call(
            "map_resources_load",
            (floor, int(use_external_variant)),
            memory=initial_memory,
            capture=captures,
            allowed_writes=allowed,
            instruction_limit=1_000_000,
        )
        outputs.append((result, normalize_trace(result.trace, mixa, mixb, loads)))

    params = struct.pack("<iI", floor, int(use_external_variant))
    rust_grids, rust_trace, rust_report = rust.call(
        "map-resources", mixa.data, mixb.data, initial_grids, params
    )
    retail_memory = outputs[0][0].memory_by_name()
    candidate_memory = outputs[1][0].memory_by_name()
    _require_equal(
        f"B{floor}/grids",
        b"".join(retail_memory[name] for name in grid_names),
        b"".join(candidate_memory[name] for name in grid_names),
        rust_grids,
    )
    _require_equal(f"B{floor}/callbacks", outputs[0][1], outputs[1][1], rust_trace)
    expected_path = f"B{floor}\\MIXB.DAT".encode() + b"\0"
    if retail_memory["path"] != expected_path or candidate_memory["path"] != expected_path:
        raise AssertionError(f"B{floor}: final path does not name MIXB.DAT")
    expected_cursor = FILE_VA + mixa.payloads[1].header_offset + 16
    expected_variant = VARIANT_VA if use_external_variant else variant_initial
    for name, expected in (("arena cursor", expected_cursor), ("variant buffer", expected_variant)):
        expected_bytes = struct.pack("<I", expected)
        if retail_memory[name] != expected_bytes or candidate_memory[name] != expected_bytes:
            raise AssertionError(f"B{floor}: {name} mismatch")
    expected_report = struct.pack(
        "<IIII", mixa.consumed, mixb.consumed, int(use_external_variant), ALL_GRIDS_SIZE
    )
    if rust_report != expected_report:
        raise AssertionError(f"B{floor}: Rust structural report mismatch")
    print(
        f"[map-resource-oracle] PASS B{floor}: {mixa.consumed + mixb.consumed:,} "
        f"input bytes, 50,000 copied grid bytes, {len(rust_trace)} trace bytes",
        flush=True,
    )


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true", help="reuse existing candidate object")
    args = parser.parse_args(argv)
    root = args.retail_dir or configured_retail_dir()
    resources_root = root / "KF" if (root / "KF").is_dir() else root
    if not args.no_rebuild:
        rebuild_units(["game.resources"])
    retail = RetailImage.load("GAME.EXE")
    symbols = GameSymbols.load()
    rust = RustCodec(args.rust_driver or build_driver())
    for floor in range(1, 6):
        compare_floor(retail, symbols, rust, resources_root, floor)
    print("[map-resource-oracle] PASS: all five floor outer walks", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
