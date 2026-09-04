"""Compare GAME's retail, reconstructed C, and Rust animation binders.

The oracle executes ``render_bind_animated_instance`` directly across existing,
missing, replaced, exhausted, retried, and static pool-record states. Retail
and candidate code share exact GAME helpers for TMD selection and the checked
allocator; pool/heap outcomes and Sony's ``gteMIMefunc`` are explicit
deterministic services. Rust independently parses the asset and produces the
same record, cached vertices, scratch state, TMD state, and lifecycle effects.

Retail inherits its keyframe cache index from uninitialized register ``$s5``.
Every machine call and Rust request therefore receives the same explicit seed.
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
    RetailProvider,
)
from scripts.kf.paths import BUILD
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage
from scripts.kf.tmd_oracle import shipped_cases


FUNCTION = "render_bind_animated_instance"
CANDIDATE_UNIT = "game.pool"
CANDIDATE_OBJECT = BUILD / "objdiff/game/base/800205d4_pool.o"
ASSET_VA = 0x80060000
SCRATCH_VA = 0x800930F0
ASSET_REGISTRY_VA = 0x80090FCC
TMD_STATE_VA = 0x80090FA8
CURRENT_VERTICES_VA = 0x800910BC
ALLOCATION_VA = 0x800A0000
RECORD_VA = 0x800B0000
ANCHOR_VA = 0x800B0100
ASSET_ID = 7
INITIAL_KF_INDEX = 0x4100
RECORD_SIZE = 20
VERTEX_SIZE = 8
TMD_STATE_SIZE = 36

TRACE_ALLOCATE_RECORD = 1
TRACE_RELEASE_RECORD = 2
TRACE_ALLOCATE_VERTICES = 3
TRACE_RELEASE_ALL = 4

SHARED_HELPERS = (
    "pool_record_release",
    "asset_registry_select",
    "tmd_select_object_vertices",
    "tmd_get_object",
    "pool_allocate",
    "memory_malloc_checked",
    "pool_release_all",
    "tmd_set_current_vertices",
)


@dataclass(frozen=True)
class MorphObject:
    index: int
    offset: int
    base_vertex: int
    vertex_count: int


@dataclass(frozen=True)
class Keyframe:
    ordinal: int
    offset: int
    reverse: int
    duration: int
    rest_index: int
    morph_indices: tuple[int, ...]


@dataclass(frozen=True)
class Clip:
    tag: int
    offset: int
    unknown_02: int
    keyframes: tuple[Keyframe, ...]


@dataclass(frozen=True)
class AnimationAsset:
    bytes: bytes
    tmd_offset: int
    object_table_offset: int
    vertex_count: int
    clips: tuple[Clip, ...]
    objects: tuple[MorphObject, ...]


@dataclass(frozen=True)
class AnimationCase:
    label: str
    asset: AnimationAsset
    clip: Clip
    keyframe: Keyframe
    phase: int
    initial_kf_index: int
    cache_hit: bool

    @property
    def keyframe_index(self) -> int:
        return (self.initial_kf_index + self.keyframe.ordinal) & 0xFFFF


@dataclass(frozen=True)
class AnimationResult:
    record: bytes
    cache: bytes
    output: bytes
    scratch_guard: bytes
    tmd_state: bytes
    current_vertices: int
    gte_calls: tuple[tuple[int, int, int, int], ...]
    instructions: int


@dataclass(frozen=True)
class LifecycleCase:
    label: str
    asset: AnimationAsset
    record_present: bool
    pool_record_available: bool
    record_asset_id: int
    allocation_results: tuple[int, ...]


@dataclass(frozen=True)
class LifecycleResult:
    status: int
    anchor: bytes
    record: bytes
    cache: bytes
    scratch: bytes
    tmd_state: bytes
    current_vertices: bytes
    trace: bytes
    gte_calls: tuple[tuple[int, int, int, int], ...]
    instructions: int


def _unpack(fmt: str, data: bytes, offset: int, context: str) -> tuple[int, ...]:
    size = struct.calcsize(fmt)
    if offset < 0 or offset + size > len(data):
        raise ValueError(f"{context}: missing {fmt} at {offset:#x}")
    return struct.unpack_from(fmt, data, offset)


def _u16(data: bytes, offset: int, context: str) -> int:
    return _unpack("<H", data, offset, context)[0]


def _u32(data: bytes, offset: int, context: str) -> int:
    return _unpack("<I", data, offset, context)[0]


def _i32(data: bytes, offset: int, context: str) -> int:
    return _unpack("<i", data, offset, context)[0]


def parse_asset(data: bytes, label: str) -> AnimationAsset:
    """Bound the table grammar read by ``render_bind_animated_instance``."""
    declared = _u32(data, 0, label)
    if declared != len(data) or declared < 20:
        raise ValueError(
            f"{label}: declared asset size {declared:#x} differs from {len(data):#x}"
        )
    clip_count = _i32(data, 4, label)
    if clip_count < 0:
        raise ValueError(f"{label}: negative clip count {clip_count}")
    tmd_offset = _u32(data, 8, label)
    object_table = _u32(data, 12, label)
    clip_table = _u32(data, 16, label)
    tmd_object = tmd_offset + 12
    vertex_count = _u32(data, tmd_object + 4, label) & 0xFFFF
    vertex_offset = _u32(data, tmd_object, label)
    if tmd_object + vertex_offset + vertex_count * VERTEX_SIZE > len(data):
        raise ValueError(f"{label}: base TMD vertex array exceeds asset")

    clips = []
    referenced_objects: set[int] = set()
    if clip_count and clip_table + clip_count * 4 > len(data):
        raise ValueError(f"{label}: clip table exceeds asset")
    for tag in range(clip_count):
        clip_at = _u32(data, clip_table + tag * 4, label)
        keyframe_count = _u16(data, clip_at, label)
        unknown_02 = _u16(data, clip_at + 2, label)
        if clip_at + 4 + keyframe_count * 4 > len(data):
            raise ValueError(f"{label}: clip {tag} keyframe table exceeds asset")
        keyframes = []
        for ordinal in range(keyframe_count):
            keyframe_at = _u32(data, clip_at + 4 + ordinal * 4, label)
            reverse, duration, rest_index, morph_count = _unpack(
                "<4H", data, keyframe_at, label
            )
            if duration == 0:
                raise ValueError(f"{label}: clip {tag} keyframe {ordinal} has zero duration")
            indices_at = keyframe_at + 8
            morph_indices = _unpack(f"<{morph_count}H", data, indices_at, label)
            referenced_objects.add(rest_index)
            referenced_objects.update(morph_indices)
            keyframes.append(
                Keyframe(
                    ordinal,
                    keyframe_at,
                    reverse,
                    duration,
                    rest_index,
                    tuple(morph_indices),
                )
            )
        clips.append(Clip(tag, clip_at, unknown_02, tuple(keyframes)))

    objects = []
    for index in range(max(referenced_objects, default=-1) + 1):
        object_at = _u32(data, object_table + index * 4, label)
        base_vertex = _u32(data, object_at + 4, label)
        count = _u32(data, object_at + 8, label)
        if object_at + 12 + count * VERTEX_SIZE > len(data):
            raise ValueError(f"{label}: morph object {index} delta array exceeds asset")
        if base_vertex + count > vertex_count:
            raise ValueError(
                f"{label}: morph object {index} range {base_vertex}.."
                f"{base_vertex + count} exceeds {vertex_count} vertices"
            )
        objects.append(MorphObject(index, object_at, base_vertex, count))
    return AnimationAsset(
        data,
        tmd_offset,
        object_table,
        vertex_count,
        tuple(clips),
        tuple(objects),
    )


def animation_cases(retail_dir: Path) -> tuple[AnimationCase, ...]:
    """Exercise every shipped keyframe miss plus one cache hit per clip."""
    result = []
    asset_count = 0
    animated_count = 0
    for case in shipped_cases(retail_dir):
        if case.tmd_offset == 0:
            continue
        asset_count += 1
        asset = parse_asset(case.payload, case.label)
        if not asset.clips:
            continue
        animated_count += 1
        for clip in asset.clips:
            elapsed = 0
            for keyframe in clip.keyframes:
                phase = elapsed + keyframe.duration // 2
                result.append(
                    AnimationCase(
                        f"{case.label}/clip-{clip.tag}/kf-{keyframe.ordinal}/miss",
                        asset,
                        clip,
                        keyframe,
                        phase,
                        INITIAL_KF_INDEX,
                        False,
                    )
                )
                if keyframe.ordinal == 0:
                    result.append(
                        AnimationCase(
                            f"{case.label}/clip-{clip.tag}/kf-0/hit",
                            asset,
                            clip,
                            keyframe,
                            phase,
                            INITIAL_KF_INDEX,
                            True,
                        )
                    )
                elapsed += keyframe.duration
            if elapsed != 4096:
                raise ValueError(f"{case.label}: clip {clip.tag} duration is {elapsed}, not 4096")
    if asset_count != 169 or animated_count != 70:
        raise ValueError(
            f"animation corpus changed: {asset_count} assets/{animated_count} animated"
        )
    if len(result) != 813 + 214:
        raise ValueError(f"animation case census changed: {len(result)}, expected 1027")
    return tuple(result)


def static_assets(retail_dir: Path) -> tuple[tuple[str, AnimationAsset], ...]:
    result = []
    for case in shipped_cases(retail_dir):
        if case.tmd_offset == 0:
            continue
        asset = parse_asset(case.payload, case.label)
        if not asset.clips:
            result.append((case.label, asset))
    if len(result) != 99:
        raise ValueError(f"static animation corpus changed: {len(result)}, expected 99")
    return tuple(result)


def synthetic_cases() -> tuple[AnimationCase, ...]:
    """Controls for retail branches absent from the shipped animation tables."""
    data = bytearray(176)
    struct.pack_into("<IiIII", data, 0, len(data), 1, 120, 56, 20)
    struct.pack_into("<I", data, 20, 24)
    struct.pack_into("<2H2I", data, 24, 2, 20, 36, 46)
    struct.pack_into("<5H", data, 36, 1, 2048, 0, 1, 1)
    struct.pack_into("<4H", data, 46, 0, 2048, 0, 0)
    struct.pack_into("<2I", data, 56, 64, 92)
    struct.pack_into("<3I4h4h", data, 64, 0x11111111, 0, 2, 2, 4, 6, 99, 8, 10, 12, 99)
    struct.pack_into(
        "<3I4h4h", data, 92, 0x22222222, 0, 2, 10, 20, 30, 99, 20, 40, 60, 99
    )
    struct.pack_into("<3I", data, 120, 0x41, 0, 1)
    struct.pack_into("<2I", data, 132, 28, 2)
    struct.pack_into("<4h4h", data, 160, 100, 200, 300, 7, -100, -200, -300, 8)
    asset = parse_asset(bytes(data), "synthetic/edge-asset")
    clip = asset.clips[0]
    return (
        AnimationCase(
            "synthetic/reverse",
            asset,
            clip,
            clip.keyframes[0],
            512,
            0x1234,
            False,
        ),
        AnimationCase(
            "synthetic/kf-index-wrap",
            asset,
            clip,
            clip.keyframes[1],
            3000,
            0xFFFF,
            False,
        ),
        AnimationCase(
            "synthetic/phase-at-total-duration",
            asset,
            clip,
            clip.keyframes[1],
            4096,
            0x1234,
            False,
        ),
    )


def lifecycle_cases() -> tuple[LifecycleCase, ...]:
    """Controls for every record ownership and allocation branch."""
    animated = synthetic_cases()[0].asset
    static_bytes = bytearray(animated.bytes)
    struct.pack_into("<i", static_bytes, 4, 0)
    static = parse_asset(bytes(static_bytes), "synthetic/static-release")
    return (
        LifecycleCase(
            "lifecycle/null-record-success",
            animated,
            False,
            True,
            ASSET_ID,
            (ALLOCATION_VA,),
        ),
        LifecycleCase(
            "lifecycle/pool-unavailable",
            animated,
            False,
            False,
            ASSET_ID,
            (),
        ),
        LifecycleCase(
            "lifecycle/malloc-retry",
            animated,
            False,
            True,
            ASSET_ID,
            (0, ALLOCATION_VA),
        ),
        LifecycleCase(
            "lifecycle/different-asset-reinit",
            animated,
            True,
            True,
            ASSET_ID - 1,
            (ALLOCATION_VA,),
        ),
        LifecycleCase(
            "lifecycle/static-record-release",
            static,
            True,
            True,
            ASSET_ID,
            (),
        ),
        LifecycleCase(
            "lifecycle/malloc-three-retries",
            animated,
            False,
            True,
            ASSET_ID,
            (0, 0, 0, ALLOCATION_VA),
        ),
        LifecycleCase(
            "lifecycle/reinit-malloc-retries",
            animated,
            True,
            True,
            ASSET_ID - 1,
            (0, 0, ALLOCATION_VA),
        ),
    )


def _pattern(size: int, seed: int) -> bytes:
    return bytes((index * 73 + (index >> 3) * 19 + seed * 41 + 17) & 0xFF for index in range(size))


def _clamp_i16(value: int) -> int:
    return max(-0x8000, min(0x7FFF, value))


def apply_gte_mime(destination: bytes, source: bytes, count: int, weight: int) -> bytes:
    """Independent byte model of Release 2.5 ``gteMIMefunc``.

    GPF uses a 12-bit fractional shift and signed IR saturation; the following
    CPU adds are stored as wrapping halfwords. Vector padding is untouched.
    """
    extent = count * VERTEX_SIZE
    if len(destination) != extent or len(source) != extent:
        raise ValueError("gteMIMefunc model requires exactly count complete vectors")
    output = bytearray(destination)
    for index in range(count):
        at = index * VERTEX_SIZE
        old = struct.unpack_from("<3h", output, at)
        delta = struct.unpack_from("<3h", source, at)
        values = [
            (component + _clamp_i16((change * weight) >> 12)) & 0xFFFF
            for component, change in zip(old, delta)
        ]
        struct.pack_into("<3H", output, at, *values)
    return bytes(output)


class GteMimeService:
    """Explicit shared Sony service boundary for the two machine-code runs."""

    def __init__(self) -> None:
        self.calls: list[tuple[int, int, int, int]] = []

    def reset(self) -> None:
        self.calls.clear()

    def __call__(self, context: HookContext) -> int:
        destination, source, count, weight = context.args
        extent = count * VERTEX_SIZE
        self.calls.append(context.args)
        context.write(
            destination,
            apply_gte_mime(
                context.read(destination, extent),
                context.read(source, extent),
                count,
                weight,
            ),
        )
        return 0


class LifecycleService:
    """Deterministic boundary for the pool and heap helpers.

    The binder's writes around these calls remain real machine instructions.
    Release models the exact pool helper's record/anchor mutations; freeing an
    allocation and flushing unrelated pool records are represented in the
    ordered trace because neither has parser-owned bytes to compare here.
    """

    def __init__(self) -> None:
        self.case: LifecycleCase | None = None
        self.allocation_index = 0
        self.trace: list[tuple[int, int, int, int]] = []

    def reset(self, case: LifecycleCase) -> None:
        self.case = case
        self.allocation_index = 0
        self.trace.clear()

    def allocate_record(self, context: HookContext) -> int:
        assert self.case is not None
        available = self.case.pool_record_available
        self.trace.append((TRACE_ALLOCATE_RECORD, int(available), 0, 0))
        if not available:
            return 0
        context.write(RECORD_VA + 4, struct.pack("<H", 0xFF))
        return RECORD_VA

    def allocate_vertices(self, context: HookContext) -> int:
        assert self.case is not None
        if self.allocation_index >= len(self.case.allocation_results):
            raise AssertionError(f"{self.case.label}: unexpected extra allocation attempt")
        result = self.case.allocation_results[self.allocation_index]
        self.allocation_index += 1
        self.trace.append((TRACE_ALLOCATE_VERTICES, context.args[0], result, 0))
        return result

    def release_record(self, context: HookContext) -> int:
        record_address = context.args[0]
        record = context.read(record_address, RECORD_SIZE)
        allocation = _u32(record, 12, "pool record")
        backlink = _u32(record, 16, "pool record")
        self.trace.append((TRACE_RELEASE_RECORD, allocation, backlink, 0))
        context.write(record_address, bytes(2))
        if backlink != 0:
            context.write(backlink, bytes(4))
        if allocation != 0:
            context.write(record_address + 12, bytes(4))
        return 0

    def release_all(self, _context: HookContext) -> int:
        self.trace.append((TRACE_RELEASE_ALL, 0, 0, 0))
        return 0

    def encoded_trace(self) -> bytes:
        return b"".join(struct.pack("<4I", *event) for event in self.trace)


def _record_and_cache(case: AnimationCase) -> tuple[bytes, bytes, bytes]:
    cache = _pattern(case.asset.vertex_count * VERTEX_SIZE, 11)
    if case.cache_hit:
        value_04 = case.clip.tag
        unknown_06 = case.keyframe_index
        rest_pointer = ASSET_VA + case.asset.objects[case.keyframe.rest_index].offset
        previous_present = 1
    else:
        value_04 = case.clip.tag ^ 0xFFFF
        unknown_06 = case.keyframe_index ^ 0xFFFF
        rest_pointer = ASSET_VA
        previous_present = 0
    record = struct.pack(
        "<hHHHIII",
        0x1357,
        ASSET_ID,
        value_04,
        unknown_06,
        rest_pointer,
        ALLOCATION_VA,
        ANCHOR_VA,
    )
    parameters = struct.pack(
        "<6H",
        ASSET_ID,
        case.clip.tag,
        case.phase,
        case.initial_kf_index,
        previous_present,
        case.keyframe.rest_index,
    )
    return record, cache, parameters


def execute_case(
    retail: RetailImage,
    program,
    service: GteMimeService,
    case: AnimationCase,
    *,
    instruction_limit: int = 1_000_000,
) -> AnimationResult:
    record, cache, _parameters = _record_and_cache(case)
    scratch_size = (case.asset.vertex_count + 1) * VERTEX_SIZE
    tmd_state = MemoryRange("tmd state", TMD_STATE_VA, 36)
    current = MemoryRange("current vertices", CURRENT_VERTICES_VA, 4)
    record_range = MemoryRange("pool record", RECORD_VA, RECORD_SIZE)
    cache_range = MemoryRange("cache", ALLOCATION_VA, len(cache))
    scratch_range = MemoryRange("scratch", SCRATCH_VA, scratch_size)
    scratch_initial = _pattern(scratch_size, 17)
    service.reset()
    result = ParserMachine(retail, program).call(
        FUNCTION,
        (
            ANCHOR_VA,
            ASSET_ID,
            case.clip.tag,
            case.phase,
            case.asset.vertex_count,
        ),
        initial_gprs={21: case.initial_kf_index},
        memory=(
            MemoryInput(ASSET_VA, case.asset.bytes),
            MemoryInput(ASSET_REGISTRY_VA + ASSET_ID * 4, struct.pack("<I", ASSET_VA)),
            MemoryInput(TMD_STATE_VA, bytes(tmd_state.size)),
            MemoryInput(CURRENT_VERTICES_VA, bytes(current.size)),
            MemoryInput(RECORD_VA, record),
            MemoryInput(ANCHOR_VA, struct.pack("<I", RECORD_VA)),
            MemoryInput(ALLOCATION_VA, cache),
            MemoryInput(SCRATCH_VA, scratch_initial),
        ),
        capture=(record_range, cache_range, scratch_range, tmd_state, current),
        allowed_writes=(record_range, cache_range, scratch_range, tmd_state, current),
        instruction_limit=instruction_limit,
    )
    if result.v0 != RECORD_VA:
        raise AssertionError(f"{case.label}: binder returned {result.v0:#x}, expected record")
    memory = result.memory_by_name()
    return AnimationResult(
        memory["pool record"],
        memory["cache"],
        memory["scratch"][VERTEX_SIZE:],
        memory["scratch"][:VERTEX_SIZE],
        memory["tmd state"],
        struct.unpack("<I", memory["current vertices"])[0],
        tuple(service.calls),
        result.instructions,
    )


def _programs(symbols: GameSymbols) -> tuple[tuple[object, GteMimeService], ...]:
    providers = tuple(RetailProvider(name) for name in SHARED_HELPERS)
    retail_service = GteMimeService()
    candidate_service = GteMimeService()
    retail_program = RetailProgram.link(
        symbols,
        [FUNCTION],
        hooks=[ExternalHook("gteMIMefunc", retail_service)],
        providers=providers,
        stack_words={FUNCTION: 1},
    )
    candidate_program = CandidateProgram.link(
        symbols,
        [CandidateFunction(FUNCTION, CANDIDATE_OBJECT, stack_words=1)],
        hooks=[ExternalHook("gteMIMefunc", candidate_service)],
        providers=providers,
    )
    return (retail_program, retail_service), (candidate_program, candidate_service)


def _lifecycle_programs(
    symbols: GameSymbols,
) -> tuple[tuple[object, GteMimeService, LifecycleService], ...]:
    providers = tuple(
        RetailProvider(name)
        for name in (
            "asset_registry_select",
            "tmd_select_object_vertices",
            "tmd_get_object",
            "tmd_set_current_vertices",
            "memory_malloc_checked",
        )
    )
    result = []
    for candidate in (False, True):
        gte = GteMimeService()
        lifecycle = LifecycleService()
        hooks = (
            ExternalHook("gteMIMefunc", gte),
            ExternalHook("pool_allocate", lifecycle.allocate_record),
            ExternalHook("malloc", lifecycle.allocate_vertices),
            ExternalHook("pool_record_release", lifecycle.release_record),
            ExternalHook("pool_release_all", lifecycle.release_all),
        )
        if candidate:
            program = CandidateProgram.link(
                symbols,
                [CandidateFunction(FUNCTION, CANDIDATE_OBJECT, stack_words=1)],
                hooks=hooks,
                providers=providers,
            )
        else:
            program = RetailProgram.link(
                symbols,
                [FUNCTION],
                hooks=hooks,
                providers=providers,
                stack_words={FUNCTION: 1},
            )
        result.append((program, gte, lifecycle))
    return tuple(result)


def _require_equal(label: str, expected: bytes, actual: bytes, version: str) -> None:
    if actual == expected:
        return
    first = next(
        (index for index, pair in enumerate(zip(expected, actual)) if pair[0] != pair[1]),
        min(len(expected), len(actual)),
    )
    raise AssertionError(
        f"{label}: retail/{version} mismatch at {first:#x}; "
        f"lengths {len(expected)}/{len(actual)}; "
        f"retail={expected[first:first + 16].hex()} "
        f"{version}={actual[first:first + 16].hex()}"
    )


def compare_cases(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[AnimationCase],
    *,
    instruction_limit: int = 1_000_000,
) -> tuple[int, int, int]:
    (retail_program, retail_service), (candidate_program, candidate_service) = _programs(symbols)
    retail_instructions = 0
    candidate_instructions = 0
    for case in cases:
        retail_result = execute_case(
            retail,
            retail_program,
            retail_service,
            case,
            instruction_limit=instruction_limit,
        )
        candidate_result = execute_case(
            retail,
            candidate_program,
            candidate_service,
            case,
            instruction_limit=instruction_limit,
        )
        record, cache, parameters = _record_and_cache(case)
        rust_output = rust.call("animation-bind", case.asset.bytes, parameters, record, cache)
        if len(rust_output) != 3:
            raise AssertionError(f"{case.label}: Rust returned {len(rust_output)} blocks, expected 3")
        for name, retail_bytes, candidate_bytes, rust_bytes in zip(
            ("record", "cache", "vertices"),
            (retail_result.record, retail_result.cache, retail_result.output),
            (candidate_result.record, candidate_result.cache, candidate_result.output),
            rust_output,
        ):
            _require_equal(f"{case.label}/{name}", retail_bytes, candidate_bytes, "C")
            _require_equal(f"{case.label}/{name}", retail_bytes, rust_bytes, "Rust")
        if retail_result.current_vertices != SCRATCH_VA + VERTEX_SIZE:
            raise AssertionError(f"{case.label}: retail installed the wrong scratch pointer")
        if candidate_result.current_vertices != retail_result.current_vertices:
            raise AssertionError(f"{case.label}: candidate installed a different scratch pointer")
        expected_guard = _pattern((case.asset.vertex_count + 1) * VERTEX_SIZE, 17)[:VERTEX_SIZE]
        _require_equal(
            f"{case.label}/scratch-guard",
            expected_guard,
            retail_result.scratch_guard,
            "retail",
        )
        _require_equal(
            f"{case.label}/scratch-guard",
            expected_guard,
            candidate_result.scratch_guard,
            "C",
        )
        expected_tmd_state = bytearray(TMD_STATE_SIZE)
        if not case.cache_hit:
            struct.pack_into("<I", expected_tmd_state, 32, ASSET_VA + case.asset.tmd_offset)
        _require_equal(
            f"{case.label}/tmd-state",
            bytes(expected_tmd_state),
            retail_result.tmd_state,
            "retail",
        )
        _require_equal(
            f"{case.label}/tmd-state",
            bytes(expected_tmd_state),
            candidate_result.tmd_state,
            "C",
        )
        if candidate_result.gte_calls != retail_result.gte_calls:
            raise AssertionError(f"{case.label}: retail/C gteMIMefunc call trace differs")
        expected_calls = 1 if case.cache_hit else len(case.keyframe.morph_indices) + 1
        if len(retail_result.gte_calls) != expected_calls:
            raise AssertionError(
                f"{case.label}: observed {len(retail_result.gte_calls)} GTE calls, "
                f"expected {expected_calls}"
            )
        retail_instructions += retail_result.instructions
        candidate_instructions += candidate_result.instructions
    return len(cases), retail_instructions, candidate_instructions


def compare_static_assets(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[tuple[str, AnimationAsset]],
    *,
    instruction_limit: int = 1_000_000,
) -> tuple[int, int, int]:
    (retail_program, retail_service), (candidate_program, candidate_service) = _programs(symbols)
    totals = [0, 0]
    for label, asset in cases:
        outputs = []
        for program, service, total_index in (
            (retail_program, retail_service, 0),
            (candidate_program, candidate_service, 1),
        ):
            service.reset()
            tmd_state = MemoryRange("tmd state", TMD_STATE_VA, 36)
            current = MemoryRange("current vertices", CURRENT_VERTICES_VA, 4)
            result = ParserMachine(retail, program).call(
                FUNCTION,
                (ANCHOR_VA, ASSET_ID, 0, 0, asset.vertex_count),
                initial_gprs={21: INITIAL_KF_INDEX},
                memory=(
                    MemoryInput(ASSET_VA, asset.bytes),
                    MemoryInput(
                        ASSET_REGISTRY_VA + ASSET_ID * 4,
                        struct.pack("<I", ASSET_VA),
                    ),
                    MemoryInput(TMD_STATE_VA, bytes(tmd_state.size)),
                    MemoryInput(CURRENT_VERTICES_VA, bytes(current.size)),
                    MemoryInput(ANCHOR_VA, bytes(4)),
                ),
                capture=(current,),
                allowed_writes=(tmd_state, current),
                instruction_limit=instruction_limit,
            )
            if result.v0 != 1 or service.calls:
                raise AssertionError(f"{label}: static binder return/GTE behavior differs")
            outputs.append(result.memory[0].data)
            totals[total_index] += result.instructions
        (rust_output,) = rust.call("animation-static", asset.bytes)
        _require_equal(f"{label}/static-pointer", outputs[0], outputs[1], "C")
        _require_equal(f"{label}/static-pointer", outputs[0], rust_output, "Rust")
    return len(cases), totals[0], totals[1]


def _lifecycle_inputs(
    case: LifecycleCase,
) -> tuple[bytes, bytes, bytes, bytes, bytes, bytes, bytes, bytes]:
    if case.record_present:
        anchor = struct.pack("<I", RECORD_VA)
        record = struct.pack(
            "<hHHHIII",
            0x1357,
            case.record_asset_id,
            0x2468,
            0x369A,
            ASSET_VA,
            ALLOCATION_VA,
            ANCHOR_VA,
        )
    else:
        anchor = bytes(4)
        record = struct.pack(
            "<hHHHIII",
            0,
            case.record_asset_id,
            0x2468,
            0x369A,
            0x12345678,
            0,
            0,
        )
    parameters = struct.pack(
        "<8H",
        ASSET_ID,
        0,
        1024,
        INITIAL_KF_INDEX,
        case.asset.vertex_count,
        int(case.record_present),
        int(case.pool_record_available),
        0,
    )
    cache = _pattern(case.asset.vertex_count * VERTEX_SIZE, 211)
    scratch = _pattern((case.asset.vertex_count + 1) * VERTEX_SIZE, 212)
    tmd_state = _pattern(TMD_STATE_SIZE, 213)
    current = struct.pack("<I", 0x81234567)
    allocations = b"".join(struct.pack("<I", result) for result in case.allocation_results)
    return parameters, anchor, record, cache, scratch, tmd_state, current, allocations


def execute_lifecycle_case(
    retail: RetailImage,
    program,
    gte: GteMimeService,
    lifecycle: LifecycleService,
    case: LifecycleCase,
    *,
    instruction_limit: int = 1_000_000,
) -> LifecycleResult:
    parameters, anchor, record, cache, scratch, tmd_bytes, current_bytes, _ = (
        _lifecycle_inputs(case)
    )
    record_range = MemoryRange("pool record", RECORD_VA, RECORD_SIZE)
    anchor_range = MemoryRange("anchor", ANCHOR_VA, 4)
    cache_range = MemoryRange("cache", ALLOCATION_VA, len(cache))
    scratch_range = MemoryRange("scratch", SCRATCH_VA, len(scratch))
    tmd_range = MemoryRange("tmd state", TMD_STATE_VA, TMD_STATE_SIZE)
    current_range = MemoryRange("current vertices", CURRENT_VERTICES_VA, 4)
    gte.reset()
    lifecycle.reset(case)
    result = ParserMachine(retail, program).call(
        FUNCTION,
        (ANCHOR_VA, ASSET_ID, 0, 1024, case.asset.vertex_count),
        initial_gprs={21: INITIAL_KF_INDEX},
        memory=(
            MemoryInput(ASSET_VA, case.asset.bytes),
            MemoryInput(ASSET_REGISTRY_VA + ASSET_ID * 4, struct.pack("<I", ASSET_VA)),
            MemoryInput(TMD_STATE_VA, tmd_bytes),
            MemoryInput(CURRENT_VERTICES_VA, current_bytes),
            MemoryInput(RECORD_VA, record),
            MemoryInput(ANCHOR_VA, anchor),
            MemoryInput(ALLOCATION_VA, cache),
            MemoryInput(SCRATCH_VA, scratch),
        ),
        capture=(
            anchor_range,
            record_range,
            cache_range,
            scratch_range,
            tmd_range,
            current_range,
        ),
        allowed_writes=(
            anchor_range,
            record_range,
            cache_range,
            scratch_range,
            tmd_range,
            current_range,
        ),
        instruction_limit=instruction_limit,
    )
    if lifecycle.allocation_index != len(case.allocation_results):
        raise AssertionError(
            f"{case.label}: consumed {lifecycle.allocation_index} of "
            f"{len(case.allocation_results)} allocation results"
        )
    memory = result.memory_by_name()
    return LifecycleResult(
        result.v0,
        memory["anchor"],
        memory["pool record"],
        memory["cache"],
        memory["scratch"],
        memory["tmd state"],
        memory["current vertices"],
        lifecycle.encoded_trace(),
        tuple(gte.calls),
        result.instructions,
    )


def compare_lifecycle_cases(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[LifecycleCase],
    *,
    instruction_limit: int = 1_000_000,
) -> tuple[int, int, int]:
    programs = _lifecycle_programs(symbols)
    totals = [0, 0]
    for case in cases:
        results = [
            execute_lifecycle_case(
                retail,
                program,
                gte,
                lifecycle,
                case,
                instruction_limit=instruction_limit,
            )
            for program, gte, lifecycle in programs
        ]
        rust_blocks = rust.call(
            "animation-instance",
            case.asset.bytes,
            *_lifecycle_inputs(case),
        )
        if len(rust_blocks) != 8:
            raise AssertionError(
                f"{case.label}: Rust returned {len(rust_blocks)} blocks, expected 8"
            )
        retail_blocks = (
            struct.pack("<I", results[0].status),
            results[0].anchor,
            results[0].record,
            results[0].cache,
            results[0].scratch,
            results[0].tmd_state,
            results[0].current_vertices,
            results[0].trace,
        )
        candidate_blocks = (
            struct.pack("<I", results[1].status),
            results[1].anchor,
            results[1].record,
            results[1].cache,
            results[1].scratch,
            results[1].tmd_state,
            results[1].current_vertices,
            results[1].trace,
        )
        for name, retail_bytes, candidate_bytes, rust_bytes in zip(
            ("status", "anchor", "record", "cache", "scratch", "tmd", "current", "trace"),
            retail_blocks,
            candidate_blocks,
            rust_blocks,
        ):
            _require_equal(f"{case.label}/{name}", retail_bytes, candidate_bytes, "C")
            _require_equal(f"{case.label}/{name}", retail_bytes, rust_bytes, "Rust")
        if results[0].gte_calls != results[1].gte_calls:
            raise AssertionError(f"{case.label}: retail/C GTE call traces differ")
        expected_gte_calls = 0 if results[0].status in (0, 1) else 2
        if len(results[0].gte_calls) != expected_gte_calls:
            raise AssertionError(
                f"{case.label}: observed {len(results[0].gte_calls)} GTE calls, "
                f"expected {expected_gte_calls}"
            )
        totals[0] += results[0].instructions
        totals[1] += results[1].instructions
        print(f"[animation-oracle] PASS {case.label}", flush=True)
    return len(cases), totals[0], totals[1]


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    parser.add_argument("--max-cases", type=int)
    parser.add_argument("--instruction-limit", type=int, default=1_000_000)
    args = parser.parse_args(argv)
    try:
        retail_dir = args.retail_dir or configured_retail_dir()
        cases = (*synthetic_cases(), *animation_cases(retail_dir))
        static_cases = static_assets(retail_dir)
        lifecycle = lifecycle_cases()
        if args.max_cases is not None:
            if args.max_cases <= 0:
                raise ValueError("--max-cases must be positive")
            cases = cases[: args.max_cases]
            static_cases = static_cases[: args.max_cases]
            lifecycle = lifecycle[: args.max_cases]
        if not args.no_rebuild:
            rebuild_units([CANDIDATE_UNIT])
        retail = RetailImage.load("GAME.EXE")
        rust = RustCodec(args.rust_driver or build_driver())
        symbols = GameSymbols.load()
        count, retail_steps, candidate_steps = compare_cases(
            retail,
            symbols,
            rust,
            cases,
            instruction_limit=args.instruction_limit,
        )
        static_count, static_retail_steps, static_candidate_steps = compare_static_assets(
            retail,
            symbols,
            rust,
            static_cases,
            instruction_limit=args.instruction_limit,
        )
        count += static_count
        retail_steps += static_retail_steps
        candidate_steps += static_candidate_steps
        lifecycle_count, lifecycle_retail_steps, lifecycle_candidate_steps = (
            compare_lifecycle_cases(
                retail,
                symbols,
                rust,
                lifecycle,
                instruction_limit=args.instruction_limit,
            )
        )
        count += lifecycle_count
        retail_steps += lifecycle_retail_steps
        candidate_steps += lifecycle_candidate_steps
        print(
            f"[animation-oracle] PASS: {count} cases; retail/C/Rust record, cache, "
            f"vertex, lifecycle, and static-pointer bytes agree "
            f"(retail {retail_steps} instructions, candidate {candidate_steps})"
        )
        print(
            "[animation-oracle] shared boundary: exact GAME TMD selectors and explicit "
            "Release 2.5 gteMIMefunc fixed-point service"
        )
        return 0
    except (OSError, RuntimeError, ValueError, AssertionError) as error:
        print(f"animation-oracle: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
