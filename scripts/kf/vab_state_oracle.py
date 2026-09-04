"""Compare VAB-load success and failure state across retail GAME, C, and Rust.

The GAME wrapper is executed as reconstructed candidate code.  Its three Sony
Release 2.5 VAB routines remain explicitly admitted shared retail providers;
their lower SPU services are deterministic hooks.  Rust independently models
the observable provider transformation: the full mutated VH, all touched Sony
tables and current pointers, GAME's audio state, and ordered service requests.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass, replace
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
    RetailProvider,
)
from scripts.kf.paths import BUILD
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage


VAB_FUNCTION = "audio_load_vab"
CANDIDATE_OBJECT = BUILD / "objdiff/game/base/800328e0_audio.o"

VH_VA = 0x800B0000
VB_VA = 0x80180000
SPU_ALLOCATION = 0x1010
AUDIO_STATE_VA = 0x80095868
AUDIO_STATE_SIZE = 0x90
MAX_PROGRAMS_VA = 0x80057CF4
OPEN_COUNT_VA = 0x80057FE8
BANK_STATUS_VA = 0x8005B020
VH_END_POINTERS_VA = 0x8005B030
HEADER_POINTERS_VA = 0x8005B0A8
PROGRAM_POINTERS_VA = 0x8005B0E8
TONE_POINTERS_VA = 0x8005B128
SPU_STARTS_VA = 0x8005B168
BODY_SIZES_VA = 0x8005B1A8
CURRENT_PROGRAM_VA = 0x80069008
CURRENT_HEADER_VA = 0x8006B728
CURRENT_TONE_VA = 0x8006BD70

VAB_PROGRAM_SIZE = 16
VAB_PROGRAM_SLOTS = 128
POINTER_TABLE_SIZE = 16 * 4

SERVICE_NAMES = (
    "_spu_getInTransfer",
    "_spu_setInTransfer",
    "SpuMalloc",
    "SpuSetTransferMode",
    "SpuSetTransferStartAddr",
    "SpuRead",
    "SpuIsTransferCompleted",
)
SERVICE_ARITY = (0, 1, 1, 1, 1, 2, 1)

_MUTABLE_LAYOUT = (
    ("VH", VH_VA, None),
    ("GAME audio", AUDIO_STATE_VA, AUDIO_STATE_SIZE),
    ("maximum programs", MAX_PROGRAMS_VA, 2),
    ("open count", OPEN_COUNT_VA, 2),
    ("bank status", BANK_STATUS_VA, 16),
    ("VH-end pointers", VH_END_POINTERS_VA, POINTER_TABLE_SIZE),
    ("header pointers", HEADER_POINTERS_VA, POINTER_TABLE_SIZE),
    ("program pointers", PROGRAM_POINTERS_VA, POINTER_TABLE_SIZE),
    ("tone pointers", TONE_POINTERS_VA, POINTER_TABLE_SIZE),
    ("SPU starts", SPU_STARTS_VA, POINTER_TABLE_SIZE),
    ("body sizes", BODY_SIZES_VA, POINTER_TABLE_SIZE),
    ("current header", CURRENT_HEADER_VA, 4),
    ("current program", CURRENT_PROGRAM_VA, 4),
    ("current tone", CURRENT_TONE_VA, 4),
)


@dataclass(frozen=True)
class VabCase:
    label: str
    vh: bytes
    vb: bytes


@dataclass(frozen=True)
class VabRuntimeCase:
    asset: VabCase
    in_transfer: int = 0
    allocation: int = SPU_ALLOCATION
    read_result: int | None = None
    active_sequence: int = 0
    all_banks_busy: bool = False
    incoming_bank_id: int = 16


def runtime_cases(asset: VabCase) -> tuple[VabRuntimeCase, ...]:
    """Real providers see byte mutations and injected service results."""
    def case(label: str, **kwargs) -> VabRuntimeCase:
        return VabRuntimeCase(replace(asset, label=label), **kwargs)

    bad_magic = bytearray(asset.vh)
    bad_magic[:4] = b"BAD!"
    bad_programs = bytearray(asset.vh)
    struct.pack_into("<H", bad_programs, 18, 129)
    unused_length = bytearray(asset.vh)
    programs = struct.unpack_from("<H", unused_length, 18)[0]
    lengths_at = 32 + 128 * 16 + programs * 16 * 32
    struct.pack_into("<H", unused_length, lengths_at + 255 * 2, 0xFFFF)
    return (
        case("runtime/success"),
        case("runtime/transfer-busy", in_transfer=1),
        case("runtime/transfer-other-value", in_transfer=2),
        case("runtime/malloc-failed", allocation=0xFFFFFFFF),
        case("runtime/spu-end-overflow", allocation=0x80000),
        case("runtime/spu-end-exact", allocation=0x80000 - len(asset.vb)),
        case("runtime/read-zero", read_result=0),
        case("runtime/read-short", read_result=len(asset.vb) - 1),
        case("runtime/read-error", read_result=0xFFFFFFFF),
        case("runtime/active-fade", active_sequence=1),
        case("runtime/active-fade-then-busy", active_sequence=1, in_transfer=1),
        case("runtime/nonboolean-active", active_sequence=2),
        case("runtime/all-banks-busy", all_banks_busy=True),
        VabRuntimeCase(replace(asset, label="runtime/bad-magic", vh=bytes(bad_magic))),
        VabRuntimeCase(replace(asset, label="runtime/too-many-programs", vh=bytes(bad_programs))),
        VabRuntimeCase(replace(asset, label="runtime/unused-length-tail", vh=bytes(unused_length))),
    )


@dataclass(frozen=True)
class VabStateResult:
    blocks: tuple[bytes, ...]
    instructions: int


def _root(retail_dir: Path) -> Path:
    nested = retail_dir / "KF"
    return nested if nested.is_dir() else retail_dir


def _chunks(data: bytes, label: str) -> tuple[bytes, ...]:
    result = []
    at = 0
    while at < len(data):
        if at + 4 > len(data):
            raise ValueError(f"{label}: truncated chunk header at {at:#x}")
        size = struct.unpack_from("<I", data, at)[0]
        end = at + 4 + size
        if end > len(data):
            raise ValueError(f"{label}: chunk at {at:#x} exceeds the file")
        result.append(data[at + 4 : end])
        at = end
    return tuple(result)


def vab_cases(retail_dir: Path) -> tuple[VabCase, ...]:
    root = _root(retail_dir)
    result = []
    for floor in range(1, 6):
        label = f"B{floor}/MIXA.DAT"
        chunks = _chunks((root / label).read_bytes(), label)
        if len(chunks) < 2:
            raise ValueError(f"{label}: expected VH and VB chunks")
        result.append(VabCase(label, chunks[0], chunks[1]))
    return tuple(result)


def _pattern(size: int, seed: int) -> bytes:
    return bytes((seed + index * 29) & 0xFF for index in range(size))


def seeded_request(case: VabCase) -> tuple[bytes, ...]:
    """Return deterministic inputs for every state byte read by the path."""

    audio = bytearray(_pattern(AUDIO_STATE_SIZE, 0x31))
    struct.pack_into("<I", audio, 0x10, 0)  # skip the unrelated SEQ fade path
    status = bytearray((1, 2, 0, *([1] * 13)))
    return (
        case.vh,
        case.vb,
        bytes(audio),
        struct.pack("<H", 0x5A5A),
        struct.pack("<H", 2),
        bytes(status),
        _pattern(POINTER_TABLE_SIZE, 0x10),
        _pattern(POINTER_TABLE_SIZE, 0x20),
        _pattern(POINTER_TABLE_SIZE, 0x30),
        _pattern(POINTER_TABLE_SIZE, 0x40),
        _pattern(POINTER_TABLE_SIZE, 0x50),
        _pattern(POINTER_TABLE_SIZE, 0x60),
        struct.pack("<I", 0x11111111),
        struct.pack("<I", 0x22222222),
        struct.pack("<I", 0x33333333),
        struct.pack("<IIIi", VH_VA, VB_VA, SPU_ALLOCATION, 0),
    )


def _hooks() -> tuple[ExternalHook, ...]:
    def read(context: HookContext) -> int:
        return context.args[1]

    return (
        ExternalHook("_spu_getInTransfer", lambda _context: 0),
        ExternalHook("_spu_setInTransfer", lambda _context: 0),
        ExternalHook("SpuMalloc", lambda _context: SPU_ALLOCATION),
        ExternalHook("SpuSetTransferMode", lambda _context: 0),
        ExternalHook("SpuSetTransferStartAddr", lambda context: context.args[0]),
        ExternalHook("SpuRead", read),
        ExternalHook("SpuIsTransferCompleted", lambda _context: 1),
        # These calls occur only when sequence_active == 1.  They are declared
        # so both full GAME functions link while the deterministic seed proves
        # the no-fade path.
        ExternalHook("VSync", lambda _context: 0),
        ExternalHook("SsSeqSetVol", lambda _context: 0),
        ExternalHook("SsSeqStop", lambda _context: 0),
        ExternalHook("SsSeqClose", lambda _context: 0),
        ExternalHook("printf", lambda _context: 0),
    )


def programs(symbols: GameSymbols, hooks: tuple[ExternalHook, ...] | None = None):
    providers = tuple(
        RetailProvider(name)
        for name in ("SsVabOpenHead", "SsVabTransBody", "SsVabTransCompleted")
    )
    functions = (VAB_FUNCTION, "audio_stop_sequence_fade")
    hooks = _hooks() if hooks is None else hooks
    retail = RetailProgram.link(symbols, functions, hooks=hooks, providers=providers)
    candidate = CandidateProgram.link(
        symbols,
        tuple(CandidateFunction(name, CANDIDATE_OBJECT) for name in functions),
        hooks=hooks,
        providers=providers,
    )
    return retail, candidate


def _machine_inputs(request: Sequence[bytes]) -> tuple[MemoryInput, ...]:
    addresses = (
        VH_VA,
        VB_VA,
        AUDIO_STATE_VA,
        MAX_PROGRAMS_VA,
        OPEN_COUNT_VA,
        BANK_STATUS_VA,
        VH_END_POINTERS_VA,
        HEADER_POINTERS_VA,
        PROGRAM_POINTERS_VA,
        TONE_POINTERS_VA,
        SPU_STARTS_VA,
        BODY_SIZES_VA,
        CURRENT_HEADER_VA,
        CURRENT_PROGRAM_VA,
        CURRENT_TONE_VA,
    )
    return tuple(MemoryInput(address, data) for address, data in zip(addresses, request))


def _ranges(vh_size: int) -> tuple[MemoryRange, ...]:
    return tuple(
        MemoryRange(name, address, vh_size if size is None else size)
        for name, address, size in _MUTABLE_LAYOUT
    )


def _service_trace(result) -> bytes:
    calls = [call for call in result.trace if call.name in SERVICE_NAMES]
    names = tuple(call.name for call in calls)
    if names != SERVICE_NAMES:
        raise AssertionError(f"unexpected VAB service trace: {names}")
    output = bytearray()
    for call, arity in zip(calls, SERVICE_ARITY):
        tag = SERVICE_NAMES.index(call.name) + 1
        args = (*call.args[:arity], *((0,) * (4 - arity)))
        output.extend(struct.pack("<5I", tag, *args))
    return bytes(output)


def _report(original_vh: bytes, blocks: Sequence[bytes]) -> bytes:
    audio = blocks[1]
    maximum = struct.unpack_from("<H", blocks[2])[0]
    bank_id = struct.unpack_from("<h", audio, 4)[0]
    if not 0 <= bank_id < 16:
        raise AssertionError(f"GAME retained invalid VAB id {bank_id}")
    program_count = struct.unpack_from("<H", original_vh, 18)[0]
    dense = sum(
        original_vh[32 + slot * VAB_PROGRAM_SIZE] != 0
        for slot in range(maximum)
    )
    tone_address = struct.unpack_from("<I", blocks[8], bank_id * 4)[0]
    length_address = tone_address + program_count * 16 * 32
    vh_end = struct.unpack_from("<I", blocks[5], bank_id * 4)[0]
    body_size = struct.unpack_from("<I", blocks[10], bank_id * 4)[0]
    return struct.pack(
        "<7I",
        bank_id,
        maximum,
        dense,
        tone_address,
        length_address,
        vh_end,
        body_size,
    )


def execute_machine(
    retail: RetailImage, program, case: VabCase, request: Sequence[bytes]
) -> VabStateResult:
    ranges = _ranges(len(case.vh))
    # The largest shipped VB overlaps the parser machine's conservative stack
    # allowance.  Capture it without admitting it as writable, then audit the
    # decoded stores too so that this read-only input cannot change silently.
    vb = MemoryRange("VB (immutable)", VB_VA, len(case.vb))
    result = ParserMachine(retail, program).call(
        VAB_FUNCTION,
        (VH_VA, VB_VA),
        memory=_machine_inputs(request),
        capture=(*ranges, vb),
        allowed_writes=ranges,
    )
    if result.memory[-1].data != case.vb:
        raise AssertionError(f"{case.label}: VAB body input was mutated")
    vb_end = VB_VA + len(case.vb)
    overlapping = tuple(
        write
        for write in result.writes
        if write.address < vb_end and VB_VA < write.address + write.size
    )
    if overlapping:
        first = overlapping[0]
        raise AssertionError(
            f"{case.label}: store at {first.pc:#010x} overlaps VAB body "
            f"{first.address:#010x}+{first.size:#x}"
        )
    mutable = tuple(item.data for item in result.memory[:-1])
    return VabStateResult(
        mutable + (_service_trace(result), _report(case.vh, mutable)),
        result.instructions,
    )


def _assert_result(label: str, blocks: Sequence[bytes]) -> None:
    if len(blocks) != 16:
        raise AssertionError(f"{label}: expected 16 output blocks, got {len(blocks)}")
    bank_id, maximum, dense, _tone, _length, _end, body = struct.unpack(
        "<7I", blocks[-1]
    )
    if bank_id != 2 or maximum != 128:
        raise AssertionError(f"{label}: seeded bank/max are {bank_id}/{maximum}")
    if blocks[4][bank_id] != 1:
        raise AssertionError(f"{label}: TransBody did not leave bank {bank_id} ready")
    if struct.unpack("<H", blocks[3])[0] != 3:
        raise AssertionError(f"{label}: open-bank count did not increment")
    if dense != struct.unpack_from("<H", blocks[0], 18)[0]:
        raise AssertionError(f"{label}: dense program count differs from VH declaration")
    if body == 0:
        raise AssertionError(f"{label}: parsed zero VAB body bytes")


def compare_vab_states(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[VabCase],
) -> tuple[int, int, int]:
    retail_program, candidate_program = programs(symbols)
    retail_steps = 0
    candidate_steps = 0
    for case in cases:
        request = seeded_request(case)
        retail_result = execute_machine(retail, retail_program, case, request)
        candidate_result = execute_machine(retail, candidate_program, case, request)
        rust_result = rust.call("audio-vab-state", *request)
        if candidate_result.blocks != retail_result.blocks:
            raise AssertionError(f"{case.label}: retail/C VAB runtime state differs")
        if rust_result != retail_result.blocks:
            for index, (expected, actual) in enumerate(
                zip(retail_result.blocks, rust_result)
            ):
                if expected != actual:
                    raise AssertionError(
                        f"{case.label}: retail/Rust VAB state block {index} differs"
                    )
            raise AssertionError(f"{case.label}: retail/Rust VAB block count differs")
        _assert_result(case.label, rust_result)
        retail_steps += retail_result.instructions
        candidate_steps += candidate_result.instructions
    return len(cases), retail_steps, candidate_steps


def _runtime_request(case: VabRuntimeCase) -> tuple[bytes, ...]:
    blocks = list(seeded_request(case.asset))
    audio = bytearray(blocks[2])
    struct.pack_into("<h", audio, 0x0C, -3)  # signed O32 service argument
    struct.pack_into("<I", audio, 0x10, case.active_sequence)
    blocks[2] = bytes(audio)
    if case.all_banks_busy:
        blocks[5] = bytes([1] * 16)
    blocks[15] = struct.pack("<IIIi", VH_VA, VB_VA, case.allocation, case.in_transfer)
    blocks.extend((b"\xA5", struct.pack(
        "<III", case.read_result is not None,
        case.read_result or 0, case.incoming_bank_id,
    )))
    return tuple(blocks)


def execute_runtime(retail: RetailImage, symbols: GameSymbols,
                    case: VabRuntimeCase, request: Sequence[bytes], *, candidate: bool):
    trace = bytearray()
    transfer = case.in_transfer

    def hook(name: str, tag: int, arity: int, response):
        def call(context: HookContext) -> int:
            nonlocal transfer
            args = (*context.args[:arity], *((0,) * (4 - arity)))
            trace.extend(struct.pack("<5I", tag, *args))
            if name == "_spu_setInTransfer":
                transfer = context.args[0]
            return response(context) if callable(response) else response
        return ExternalHook(name, call)

    def error(context: HookContext) -> int:
        message = bytearray()
        for offset in range(64):
            byte = context.read(context.args[0] + offset, 1)[0]
            if byte == 0:
                break
            message.append(byte)
        tag = {b"VAB headder open failed\n": 12, b"VAB body open failed\n": 13}.get(bytes(message))
        if tag is None:
            raise AssertionError(f"unexpected VAB error: {bytes(message)!r}")
        trace.extend(struct.pack("<5I", tag, 0, 0, 0, 0))
        return 0

    hooks = (
        hook("_spu_getInTransfer", 1, 0, lambda _ctx: transfer),
        hook("_spu_setInTransfer", 2, 1, 0),
        hook("SpuMalloc", 3, 1, case.allocation),
        hook("SpuSetTransferMode", 4, 1, 0),
        hook("SpuSetTransferStartAddr", 5, 1, lambda ctx: ctx.args[0]),
        hook("SpuRead", 6, 2, lambda ctx: ctx.args[1] if case.read_result is None else case.read_result),
        hook("SpuIsTransferCompleted", 7, 1, 1),
        hook("VSync", 8, 1, 0),
        hook("SsSeqSetVol", 9, 3, 0),
        hook("SsSeqStop", 10, 1, 0),
        hook("SsSeqClose", 11, 1, 0),
        ExternalHook("printf", error),
    )
    program = programs(symbols, hooks)[int(candidate)]
    prefix = MemoryRange("Sony automatic-id cleanup byte", BANK_STATUS_VA - 1, 1)
    ranges = (*_ranges(len(case.asset.vh)), prefix)
    immutable_vb = MemoryRange("immutable VB", VB_VA, len(case.asset.vb))
    result = ParserMachine(retail, program).call(
        VAB_FUNCTION, (VH_VA, VB_VA),
        initial_gprs={19: case.incoming_bank_id},
        memory=(*_machine_inputs(request[:15]), MemoryInput(prefix.address, request[16])),
        capture=(*ranges, immutable_vb), allowed_writes=ranges,
    )
    if result.memory[-1].data != case.asset.vb or any(
        write.address < VB_VA + len(case.asset.vb) and VB_VA < write.address + write.size
        for write in result.writes
    ):
        raise AssertionError(f"{case.asset.label}: VB modified")
    # The service flag is represented by its complete ordered get/set trace.
    return tuple(region.data for region in result.memory[:-2]) + (
        bytes(trace), result.memory[-2].data,
    )


def compare_runtime_cases(retail: RetailImage, symbols: GameSymbols, rust: RustCodec,
                          cases: Sequence[VabRuntimeCase]) -> int:
    for case in cases:
        request = _runtime_request(case)
        expected = execute_runtime(retail, symbols, case, request, candidate=False)
        candidate = execute_runtime(retail, symbols, case, request, candidate=True)
        actual = rust.call("audio-vab-runtime", *request)
        for version, result in (("C", candidate), ("Rust", actual)):
            if result != expected:
                index = next((i for i, (a, b) in enumerate(zip(expected, result)) if a != b), -1)
                raise AssertionError(f"{case.asset.label}: retail/{version} runtime block {index} differs")
    return len(cases)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    parser.add_argument("--max-cases", type=int)
    args = parser.parse_args(argv)
    try:
        cases = vab_cases(args.retail_dir or configured_retail_dir())
        if args.max_cases is not None:
            if args.max_cases <= 0:
                raise ValueError("--max-cases must be positive")
            cases = cases[: args.max_cases]
        if not args.no_rebuild:
            rebuild_units(["game.audio"])
        retail = RetailImage.load("GAME.EXE")
        symbols = GameSymbols.load()
        rust = RustCodec(args.rust_driver or build_driver())
        count, retail_steps, candidate_steps = compare_vab_states(
            retail, symbols, rust, cases
        )
        runtime_count = compare_runtime_cases(retail, symbols, rust, runtime_cases(cases[0]))
        print(
            f"[vab-state-oracle] PASS: {count} shipped VAB banks; retail/C/Rust "
            "mutated VH, GAME audio state, Sony tables/pointers, and ordered SPU "
            "service requests agree"
        )
        print(
            f"[vab-state-oracle] instructions: retail {retail_steps}, "
            f"candidate {candidate_steps}"
        )
        print(f"[vab-state-oracle] PASS: {runtime_count} success/failure/fade controls; "
              "full partial state, cleanup byte, immutable VB, and service arguments agree")
        print(
            "[vab-state-oracle] shared boundary: candidate GAME wrapper is actual; "
            "Sony OpenHead/TransBody/Completed use explicitly admitted retail providers"
        )
        return 0
    except (OSError, RuntimeError, ValueError, AssertionError) as error:
        print(f"vab-state-oracle: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
