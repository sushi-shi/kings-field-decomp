"""Compare GAME's retail/C audio loaders with the Rust VAB and SEQ codecs.

The GAME wrappers are executed around the real vendored Sony Release 2.5
parsers.  Their lower SPU/CD services are deterministic hooks.  Rust does not
reuse those parsers: its typed, allocation-free readers are compared against
an independent bounded Python normalization of every shipped file.

Sony's private VAB work areas remain outside this oracle.  For SEQ, the oracle
captures all 176 caller-owned score bytes after initialization and after every
event, plus the complete ordered argument words sent to observable SPU hooks.
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


VAB_FUNCTION = "audio_load_vab"
SEQ_FUNCTION = "SsSeqOpen"
SEQ_WRAPPER_FUNCTION = "audio_play_map_sequence"
EVENT_FUNCTION = "GetSeqData"
CANDIDATE_OBJECT = BUILD / "objdiff/game/base/800328e0_audio.o"

VH_VA = 0x800B0000
# Largest shipped VB ends below the parser-machine stack from this address and
# cannot overwrite the relocated candidate arena at 0x80100000.
VB_VA = 0x80180000
SEQ_VA = 0x800C0000
SCORE_RECORD_VA = 0x800B0000
AUDIO_STATE_VA = 0x80095868
AUDIO_STATE_SIZE = 0x90
SCORE_POINTER_VA = 0x800A06E0
SCORE_RECORD_SIZE = 176
OPEN_FLAG_VA = 0x8009A728
VBLANK_MINUS_VA = 0x8009FF00

VAB_HEADER_SIZE = 32
VAB_PROGRAM_SLOTS = 128
VAB_PROGRAM_SIZE = 16
VAB_TONES_PER_PROGRAM = 16
VAB_TONE_SIZE = 32
VAB_OFFSET_ENTRIES = 256
VAB_OFFSET_TABLE_SIZE = VAB_OFFSET_ENTRIES * 2

SEQ_HEADER_SIZE = 15
SEQ_PATHS = (
    "B1/SND0.SEQ",
    "B1/SND1.SEQ",
    "B1/SND2.SEQ",
    "B2/SND0.SEQ",
    "B2/SND1.SEQ",
    "B3/SND0.SEQ",
    "B4/SND0.SEQ",
    "B5/SND0.SEQ",
    "B5/SND1.SEQ",
)


@dataclass(frozen=True)
class VabCase:
    label: str
    vh: bytes
    vb: bytes


@dataclass(frozen=True)
class SeqCase:
    label: str
    data: bytes


@dataclass(frozen=True)
class SeqEvent:
    offset: int
    delta: int
    delta_len: int
    kind: int
    status: int
    flags: int
    data1: int
    data2: int
    payload_len: int
    encoded: bytes


@dataclass(frozen=True)
class VabMachineResult:
    audio_state: bytes
    vh: bytes
    trace: tuple[tuple[str, tuple[int, ...], tuple[int, ...]], ...]
    instructions: int


@dataclass(frozen=True)
class SeqMachineResult:
    initialized: bytes
    record: bytes
    open_flag: bytes
    trace: tuple[str, ...]
    instructions: int


@dataclass(frozen=True)
class SeqWrapperResult:
    audio_state: bytes
    record: bytes
    open_flag: bytes
    loaded_sequence: bytes
    trace: tuple[tuple[str, tuple[int, ...], tuple[int, ...]], ...]
    instructions: int


@dataclass(frozen=True)
class EventMachineResult:
    record: bytes
    trace: tuple[tuple[str, tuple[int, ...], tuple[int, ...]], ...]
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
            raise ValueError(f"{label}: chunk at {at:#x} exceeds file")
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


def seq_cases(retail_dir: Path) -> tuple[SeqCase, ...]:
    root = _root(retail_dir)
    return tuple(SeqCase(path, (root / path).read_bytes()) for path in SEQ_PATHS)


def normalize_vab(vh: bytes, vb: bytes) -> tuple[bytes, ...]:
    if len(vh) < VAB_HEADER_SIZE or vh[:4] != b"pBAV":
        raise ValueError("VAB: invalid or truncated header")
    file_size, programs, tones, samples = struct.unpack_from("<I2x3H", vh, 12)
    if programs > VAB_PROGRAM_SLOTS or tones > programs * VAB_TONES_PER_PROGRAM:
        raise ValueError("VAB: invalid program/tone counts")
    if samples >= VAB_OFFSET_ENTRIES:
        raise ValueError("VAB: invalid sample count")
    tone_at = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE
    offsets_at = tone_at + programs * VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE
    if len(vh) != offsets_at + VAB_OFFSET_TABLE_SIZE:
        raise ValueError("VAB: header extent differs from count-derived extent")
    if file_size != len(vh) + len(vb):
        raise ValueError("VAB: declared file size differs from VH plus VB")
    if struct.unpack_from("<H", vh, offsets_at)[0] != 0:
        raise ValueError("VAB: first sample length entry is nonzero")
    sample_rows = bytearray()
    body_at = 0
    for index in range(samples):
        units = struct.unpack_from("<H", vh, offsets_at + (index + 1) * 2)[0]
        size = units * 8
        sample_rows.extend(struct.pack("<HHII", index, units, body_at, size))
        body_at += size
    if body_at != len(vb):
        raise ValueError("VAB: sample lengths do not span VB")
    return (
        vh[:VAB_HEADER_SIZE],
        vh[VAB_HEADER_SIZE:tone_at],
        vh[tone_at:offsets_at],
        bytes(sample_rows),
        vh,
        vb,
    )


def _vlq(data: bytes, at: int) -> tuple[int, int, int]:
    start = at
    value = 0
    for _index in range(4):
        if at == len(data):
            raise ValueError(f"SEQ: truncated VLQ at {start:#x}")
        byte = data[at]
        at += 1
        value = (value << 7) | (byte & 0x7F)
        if byte & 0x80 == 0:
            return value, at - start, at
    raise ValueError(f"SEQ: VLQ at {start:#x} exceeds four bytes")


def parse_seq_events(data: bytes) -> tuple[SeqEvent, ...]:
    if len(data) < SEQ_HEADER_SIZE or data[:4] != b"pQES":
        raise ValueError("SEQ: invalid or truncated header")
    result = []
    at = SEQ_HEADER_SIZE
    running: int | None = None
    while at < len(data):
        start = at
        delta, delta_len, at = _vlq(data, at)
        if at == len(data):
            raise ValueError(f"SEQ: missing status at {at:#x}")
        first = data[at]
        at += 1
        if first < 0x80:
            if running is None:
                raise ValueError(f"SEQ: running data without status at {at - 1:#x}")
            status = running
            first_data = first
            flags = 1
        else:
            status = first
            first_data = None
            flags = 0
        data1 = 0
        data2 = 0xFF
        if 0x80 <= status <= 0xEF:
            running = status
            size = 1 if status >> 4 in (0xC, 0xD) else 2
            if first_data is None:
                if at == len(data):
                    raise ValueError("SEQ: truncated channel event")
                data1 = data[at]
                at += 1
            else:
                data1 = first_data
            if data1 >= 0x80:
                raise ValueError("SEQ: channel data byte has status bit")
            if size == 2:
                if at == len(data):
                    raise ValueError("SEQ: truncated channel event")
                data2 = data[at]
                at += 1
                if data2 >= 0x80:
                    raise ValueError("SEQ: channel data byte has status bit")
            kind = 0
            payload_len = size
        elif status == 0xFF:
            running = None
            if at == len(data):
                raise ValueError("SEQ: truncated meta type")
            data1 = data[at]
            at += 1
            payload_len, _length_len, at = _vlq(data, at)
            if at + payload_len > len(data):
                raise ValueError("SEQ: truncated meta payload")
            at += payload_len
            kind = 1
        elif status in (0xF0, 0xF7):
            running = None
            payload_len, _length_len, at = _vlq(data, at)
            if at + payload_len > len(data):
                raise ValueError("SEQ: truncated system-exclusive payload")
            at += payload_len
            kind = 2
        elif status in (0xF1, 0xF2, 0xF3, 0xF6, 0xF8, 0xFA, 0xFB, 0xFC, 0xFE):
            if status < 0xF8:
                running = None
            payload_len = {0xF1: 1, 0xF2: 2, 0xF3: 1}.get(status, 0)
            if at + payload_len > len(data):
                raise ValueError("SEQ: truncated system payload")
            if any(byte >= 0x80 for byte in data[at : at + payload_len]):
                raise ValueError("SEQ: system data byte has status bit")
            at += payload_len
            kind = 3
        else:
            raise ValueError(f"SEQ: unsupported status {status:#x}")
        result.append(
            SeqEvent(
                start,
                delta,
                delta_len,
                kind,
                status,
                flags,
                data1,
                data2,
                payload_len,
                data[start:at],
            )
        )
    return tuple(result)


def normalize_seq(data: bytes) -> tuple[bytes, ...]:
    events = parse_seq_events(data)
    header = data[:SEQ_HEADER_SIZE]
    resolution = struct.unpack_from(">H", header, 8)[0]
    tempo = int.from_bytes(header[10:13], "big")
    if tempo == 0 or not events:
        raise ValueError("SEQ: zero tempo or empty event stream")
    quotient, remainder = divmod(60_000_000, tempo)
    bpm = quotient + (remainder > tempo // 2)
    rows = bytearray(struct.pack("<I", len(events)))
    for event in events:
        rows.extend(
            struct.pack(
                "<IIBBBBBBH I",
                event.offset,
                event.delta,
                event.delta_len,
                event.kind,
                event.status,
                event.flags,
                event.data1,
                event.data2,
                event.payload_len,
                len(event.encoded),
            )
        )
        rows.extend(event.encoded)
    initialized = struct.pack(
        "<4I",
        resolution,
        bpm,
        events[0].delta * 10,
        events[0].offset + events[0].delta_len,
    )
    return header, bytes(rows), initialized, data


def _trace(result) -> tuple[tuple[str, tuple[int, ...], tuple[int, ...]], ...]:
    return tuple((call.name, call.args, call.stack_args) for call in result.trace)


def _vab_hooks() -> tuple[ExternalHook, ...]:
    def read(context: HookContext) -> int:
        return context.args[1]

    return (
        ExternalHook("_spu_getInTransfer", lambda _context: 0),
        ExternalHook("_spu_setInTransfer", lambda _context: 0),
        ExternalHook("SpuMalloc", lambda _context: 0x1010),
        ExternalHook("SpuSetTransferMode", lambda _context: 0),
        ExternalHook("SpuSetTransferStartAddr", lambda context: context.args[0]),
        ExternalHook("SpuRead", read),
        ExternalHook("SpuIsTransferCompleted", lambda _context: 1),
        ExternalHook("VSync", lambda _context: 0),
        ExternalHook("SsSeqSetVol", lambda _context: 0),
        ExternalHook("SsSeqStop", lambda _context: 0),
        ExternalHook("SsSeqClose", lambda _context: 0),
        ExternalHook("printf", lambda _context: 0),
    )


def _vab_programs(symbols: GameSymbols):
    providers = tuple(
        RetailProvider(name)
        for name in (
            "SsVabOpenHead",
            "SsVabTransBody",
            "SsVabTransCompleted",
        )
    )
    retail = RetailProgram.link(
        symbols,
        [VAB_FUNCTION, "audio_stop_sequence_fade"],
        hooks=_vab_hooks(),
        providers=providers,
    )
    candidate = CandidateProgram.link(
        symbols,
        [
            CandidateFunction(VAB_FUNCTION, CANDIDATE_OBJECT),
            CandidateFunction("audio_stop_sequence_fade", CANDIDATE_OBJECT),
        ],
        hooks=_vab_hooks(),
        providers=providers,
    )
    return retail, candidate


def execute_vab(retail: RetailImage, program, case: VabCase) -> VabMachineResult:
    state = MemoryRange("audio state", AUDIO_STATE_VA, AUDIO_STATE_SIZE)
    header = MemoryRange("VH", VH_VA, len(case.vh))
    result = ParserMachine(retail, program).call(
        VAB_FUNCTION,
        (VH_VA, VB_VA),
        memory=(
            MemoryInput(AUDIO_STATE_VA, bytes(AUDIO_STATE_SIZE)),
            MemoryInput(VH_VA, case.vh),
            MemoryInput(VB_VA, case.vb),
        ),
        capture=(state, header),
    )
    memory = result.memory_by_name()
    return VabMachineResult(memory["audio state"], memory["VH"], _trace(result), result.instructions)


def _assert_dense_program_map(label: str, original: bytes, opened: bytes) -> None:
    dense = 0
    declared = struct.unpack_from("<H", original, 18)[0]
    for slot in range(VAB_PROGRAM_SLOTS):
        at = VAB_HEADER_SIZE + slot * VAB_PROGRAM_SIZE
        actual = struct.unpack_from("<I", opened, at + 8)[0]
        if actual != dense:
            raise AssertionError(
                f"{label}: opened program slot {slot} maps to {actual}, expected {dense}"
            )
        dense += original[at] != 0
    if dense != declared:
        raise AssertionError(f"{label}: {dense} live slots differ from declared {declared}")


def compare_vabs(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[VabCase],
) -> tuple[int, int, int]:
    retail_program, candidate_program = _vab_programs(symbols)
    retail_steps = 0
    candidate_steps = 0
    for case in cases:
        expected = normalize_vab(case.vh, case.vb)
        actual = rust.call("audio-vab", case.vh, case.vb)
        if actual != expected:
            raise AssertionError(f"{case.label}: Rust VAB normalized bytes differ")
        retail_result = execute_vab(retail, retail_program, case)
        candidate_result = execute_vab(retail, candidate_program, case)
        if candidate_result.audio_state != retail_result.audio_state:
            raise AssertionError(f"{case.label}: retail/C audio state differs")
        if candidate_result.vh != retail_result.vh:
            raise AssertionError(f"{case.label}: retail/C opened VH differs")
        if candidate_result.trace != retail_result.trace:
            raise AssertionError(f"{case.label}: retail/C VAB call trace differs")
        if struct.unpack_from("<I", retail_result.audio_state)[0] != VH_VA:
            raise AssertionError(f"{case.label}: GAME did not retain the VH pointer")
        if struct.unpack_from("<h", retail_result.audio_state, 4)[0] != 0:
            raise AssertionError(f"{case.label}: GAME did not retain VAB id zero")
        _assert_dense_program_map(case.label, case.vh, retail_result.vh)
        retail_steps += retail_result.instructions
        candidate_steps += candidate_result.instructions
    return len(cases), retail_steps, candidate_steps


def _seq_program(symbols: GameSymbols):
    return RetailProgram.link(
        symbols,
        [SEQ_FUNCTION],
        providers=(RetailProvider("InitSoundSeq"), RetailProvider("ReadDeltaValue")),
    )


def execute_seq(
    retail: RetailImage,
    program,
    case: SeqCase,
    seed: bytes = bytes(SCORE_RECORD_SIZE),
) -> SeqMachineResult:
    if len(seed) != SCORE_RECORD_SIZE:
        raise ValueError(f"score seed is {len(seed)} bytes, expected {SCORE_RECORD_SIZE}")
    record = MemoryRange("score", SCORE_RECORD_VA, SCORE_RECORD_SIZE)
    open_flag = MemoryRange("open flag", OPEN_FLAG_VA, 4)
    result = ParserMachine(retail, program).call(
        SEQ_FUNCTION,
        (SEQ_VA, 0),
        memory=(
            MemoryInput(SEQ_VA, case.data),
            MemoryInput(SCORE_POINTER_VA, struct.pack("<I", SCORE_RECORD_VA)),
            MemoryInput(SCORE_RECORD_VA, seed),
            MemoryInput(OPEN_FLAG_VA, bytes(4)),
            MemoryInput(VBLANK_MINUS_VA, struct.pack("<I", 60)),
        ),
        capture=(record, open_flag),
        allowed_writes=(record, open_flag),
    )
    if result.v0 != 0:
        raise AssertionError(f"{case.label}: SsSeqOpen returned {result.v0}, expected slot zero")
    memory = result.memory_by_name()
    data = memory["score"]
    initialized = struct.pack(
        "<4I",
        struct.unpack_from("<H", data, 108)[0],
        struct.unpack_from("<I", data, 160)[0],
        struct.unpack_from("<I", data, 152)[0],
        struct.unpack_from("<I", data, 48)[0] - SEQ_VA,
    )
    if struct.unpack_from("<I", data, 52)[0] != struct.unpack_from("<I", data, 48)[0]:
        raise AssertionError(f"{case.label}: InitSoundSeq cursor copies differ")
    if struct.unpack_from("<I", data, 56)[0] != struct.unpack_from("<I", data, 48)[0]:
        raise AssertionError(f"{case.label}: InitSoundSeq loop cursor differs")
    if struct.unpack_from("<I", data, 164)[0] != struct.unpack_from("<I", data, 152)[0]:
        raise AssertionError(f"{case.label}: InitSoundSeq delta copies differ")
    if struct.unpack_from("<I", data, 168)[0] != struct.unpack_from("<I", data, 160)[0]:
        raise AssertionError(f"{case.label}: InitSoundSeq tempo copies differ")
    return SeqMachineResult(
        initialized,
        data,
        memory["open flag"],
        tuple(call.name for call in result.trace),
        result.instructions,
    )


def _c_string(context: HookContext, address: int, maximum: int = 64) -> bytes:
    result = bytearray()
    for index in range(maximum):
        byte = context.read(address + index, 1)[0]
        if byte == 0:
            return bytes(result)
        result.append(byte)
    raise AssertionError("unterminated hooked CD path")


def _seq_wrapper_program(symbols: GameSymbols, case: SeqCase, *, candidate: bool):
    expected_path = case.label.replace("/", "\\").encode("ascii")

    def load(context: HookContext) -> int:
        destination, path, _unused2, _unused3 = context.args
        if destination != SEQ_VA:
            raise AssertionError(f"sequence loader destination is {destination:#x}")
        actual_path = _c_string(context, path)
        if actual_path != expected_path:
            raise AssertionError(
                f"sequence loader path is {actual_path!r}, expected {expected_path!r}"
            )
        context.write(destination, case.data)
        return 0

    def memset(context: HookContext) -> int:
        destination, value, size, _unused3 = context.args
        context.write(destination, bytes((value & 0xFF,)) * size)
        return destination

    hooks = (
        ExternalHook("memset", memset),
        ExternalHook("cd_file_load_into", load),
        ExternalHook("VSync", lambda _context: 0),
        ExternalHook("SsSeqSetVol", lambda _context: 0),
        ExternalHook("SsSeqPlay", lambda _context: 0),
        ExternalHook("SsSeqStop", lambda _context: 0),
        ExternalHook("SsSeqClose", lambda _context: 0),
    )
    providers = tuple(
        RetailProvider(name)
        for name in (
            "SsSeqOpen",
            "InitSoundSeq",
            "ReadDeltaValue",
        )
    )
    if candidate:
        return CandidateProgram.link(
            symbols,
            [
                CandidateFunction(SEQ_WRAPPER_FUNCTION, CANDIDATE_OBJECT),
                CandidateFunction("audio_stop_sequence_fade", CANDIDATE_OBJECT),
            ],
            hooks=hooks,
            providers=providers,
        )
    return RetailProgram.link(
        symbols,
        [SEQ_WRAPPER_FUNCTION, "audio_stop_sequence_fade"],
        hooks=hooks,
        providers=providers,
    )


def execute_seq_wrapper(
    retail: RetailImage,
    symbols: GameSymbols,
    case: SeqCase,
    *,
    candidate: bool,
    audio_seed: bytes | None = None,
    score_seed: bytes = bytes(SCORE_RECORD_SIZE),
    open_flags: int = 0,
) -> SeqWrapperResult:
    floor = int(case.label[1])
    sequence_id = int(case.label[6])
    player = bytearray(0xE0)
    player[10] = floor
    player[151] = 1
    if audio_seed is None:
        audio = bytearray(AUDIO_STATE_SIZE)
        struct.pack_into("<Ih", audio, 8, SEQ_VA, 0)
    else:
        if len(audio_seed) != AUDIO_STATE_SIZE:
            raise ValueError(
                f"audio seed is {len(audio_seed)} bytes, expected {AUDIO_STATE_SIZE}"
            )
        audio = bytearray(audio_seed)
    if len(score_seed) != SCORE_RECORD_SIZE:
        raise ValueError(
            f"score seed is {len(score_seed)} bytes, expected {SCORE_RECORD_SIZE}"
        )
    state = MemoryRange("audio state", AUDIO_STATE_VA, AUDIO_STATE_SIZE)
    record = MemoryRange("score", SCORE_RECORD_VA, SCORE_RECORD_SIZE)
    open_flag = MemoryRange("open flag", OPEN_FLAG_VA, 4)
    sequence = MemoryRange("sequence", SEQ_VA, len(case.data))
    result = ParserMachine(
        retail, _seq_wrapper_program(symbols, case, candidate=candidate)
    ).call(
        SEQ_WRAPPER_FUNCTION,
        (sequence_id,),
        memory=(
            MemoryInput(AUDIO_STATE_VA, bytes(audio)),
            MemoryInput(0x800A0780, bytes(player)),
            MemoryInput(SCORE_POINTER_VA, struct.pack("<I", SCORE_RECORD_VA)),
            MemoryInput(SCORE_RECORD_VA, score_seed),
            MemoryInput(OPEN_FLAG_VA, struct.pack("<I", open_flags)),
            MemoryInput(VBLANK_MINUS_VA, struct.pack("<I", 60)),
        ),
        capture=(state, record, open_flag, sequence),
        allowed_writes=(state, record, open_flag, sequence),
    )
    memory = result.memory_by_name()
    return SeqWrapperResult(
        memory["audio state"],
        memory["score"],
        memory["open flag"],
        memory["sequence"],
        _trace(result),
        result.instructions,
    )


def _event_program(symbols: GameSymbols):
    providers = tuple(
        RetailProvider(name)
        for name in (
            "NoteOn",
            "SetProgramChange",
            "SetControlChange",
            "SetPitchBend",
            "GetMetaEvent",
            "ReadDeltaValue",
            "Snd_nextseq",
        )
    )
    hooks = (
        ExternalHook("SpuVmKeyOn", lambda _context: 0, stack_words=2),
        ExternalHook("SpuVmKeyOff", lambda _context: 0),
        ExternalHook("SpuVmGetProgVol", lambda _context: 0),
        ExternalHook("SpuVmPitchBend", lambda _context: 0),
        ExternalHook("SpuVmSeqKeyOff", lambda _context: 0),
        ExternalHook("SpuVmSetSeqVol", lambda _context: 0),
    )
    return RetailProgram.link(symbols, [EVENT_FUNCTION], hooks=hooks, providers=providers)


def execute_event(
    retail: RetailImage, program, sequence: bytes, record_bytes: bytes
) -> EventMachineResult:
    record = MemoryRange("score", SCORE_RECORD_VA, SCORE_RECORD_SIZE)
    result = ParserMachine(retail, program).call(
        EVENT_FUNCTION,
        (0, 0),
        memory=(
            MemoryInput(SEQ_VA, sequence),
            MemoryInput(SCORE_POINTER_VA, struct.pack("<I", SCORE_RECORD_VA)),
            MemoryInput(SCORE_RECORD_VA, record_bytes),
            MemoryInput(VBLANK_MINUS_VA, struct.pack("<I", 60)),
        ),
        capture=(record,),
        allowed_writes=(record,),
    )
    return EventMachineResult(
        result.memory[0].data,
        _trace(result),
        result.instructions,
    )


def compare_event_stream(
    retail: RetailImage,
    symbols: GameSymbols,
    case: SeqCase,
    initial_record: bytes,
    expected: Sequence[SeqEvent],
    rust_states: bytes,
    rust_callbacks: bytes,
    *,
    event_limit: int | None = None,
) -> tuple[int, int]:
    program = _event_program(symbols)
    record = initial_record
    instructions = 0
    count = len(expected) if event_limit is None else min(len(expected), event_limit)
    handlers = {
        0x90: "NoteOn",
        0xB0: "SetControlChange",
        0xC0: "SetProgramChange",
        0xE0: "SetPitchBend",
        0xF0: "GetMetaEvent",
    }
    state_count = struct.unpack_from("<I", rust_states)[0]
    if len(rust_states) != 4 + state_count * SCORE_RECORD_SIZE:
        raise AssertionError(f"{case.label}: malformed Rust score-state stream")
    callback_at = 0
    callback_count = struct.unpack_from("<I", rust_callbacks, callback_at)[0]
    callback_at += 4
    decoded_callbacks = []
    for callback_event in range(callback_count):
        if callback_at + 4 > len(rust_callbacks):
            raise AssertionError(
                f"{case.label}/event-{callback_event}: truncated Rust callback count"
            )
        per_event = struct.unpack_from("<I", rust_callbacks, callback_at)[0]
        callback_at += 4
        callbacks = []
        for _index in range(per_event):
            if callback_at + 28 > len(rust_callbacks):
                raise AssertionError(
                    f"{case.label}/event-{callback_event}: truncated Rust callback"
                )
            row = struct.unpack_from("<7I", rust_callbacks, callback_at)
            callback_at += 28
            callbacks.append((row[0], row[1:]))
        decoded_callbacks.append(tuple(callbacks))
    if callback_at != len(rust_callbacks):
        raise AssertionError(f"{case.label}: trailing Rust callback bytes")
    if state_count != len(expected) or callback_count != len(expected):
        raise AssertionError(
            f"{case.label}: Rust runtime counts states={state_count}, "
            f"callbacks={callback_count}, events={len(expected)}"
        )

    callback_tags = {
        "SpuVmKeyOn": 1,
        "SpuVmKeyOff": 2,
        "SpuVmPitchBend": 3,
    }
    for index, event in enumerate(expected[:count]):
        cursor_before = struct.unpack_from("<I", record, 48)[0] - SEQ_VA
        expected_cursor = event.offset + event.delta_len
        if cursor_before != expected_cursor:
            raise AssertionError(
                f"{case.label}/event-{index}: retail cursor {cursor_before:#x}, "
                f"Rust event status {expected_cursor:#x}"
            )
        result = execute_event(retail, program, case.data, record)
        handler = handlers.get(event.status & 0xF0)
        trace_names = tuple(call[0] for call in result.trace)
        if handler is None or handler not in trace_names:
            raise AssertionError(
                f"{case.label}/event-{index}: status {event.status:#x} trace "
                f"{trace_names!r} lacks expected handler {handler!r}"
            )
        expected_callbacks = []
        for name, args, stack_args in result.trace:
            tag = callback_tags.get(name)
            if tag is None:
                continue
            words = tuple(args) + tuple(stack_args)
            if len(words) > 6:
                raise AssertionError(
                    f"{case.label}/event-{index}: {name} has {len(words)} argument words"
                )
            expected_callbacks.append((tag, words + (0,) * (6 - len(words))))
        if decoded_callbacks[index] != tuple(expected_callbacks):
            raise AssertionError(
                f"{case.label}/event-{index}: retail SPU callbacks "
                f"{tuple(expected_callbacks)!r} != Rust {decoded_callbacks[index]!r}"
            )
        state_at = 4 + index * SCORE_RECORD_SIZE
        rust_record = rust_states[state_at : state_at + SCORE_RECORD_SIZE]
        if result.record != rust_record:
            differences = [
                offset
                for offset, (left, right) in enumerate(zip(result.record, rust_record))
                if left != right
            ]
            raise AssertionError(
                f"{case.label}/event-{index}: retail/Rust score record differs at "
                f"{differences[:16]!r}"
            )
        if index + 1 < len(expected):
            following = expected[index + 1]
            cursor_after = struct.unpack_from("<I", result.record, 48)[0] - SEQ_VA
            expected_after = following.offset + following.delta_len
            if cursor_after != expected_after:
                raise AssertionError(
                    f"{case.label}/event-{index}: retail next cursor {cursor_after:#x}, "
                    f"Rust next status {expected_after:#x}"
                )
            delta_after = struct.unpack_from("<I", result.record, 164)[0]
            expected_delta = following.delta * 10
            if delta_after != expected_delta:
                raise AssertionError(
                    f"{case.label}/event-{index}: retail next delta {delta_after}, "
                    f"Rust next scaled delta {expected_delta}"
                )
        record = result.record
        instructions += result.instructions
    return count, instructions


def compare_sequences(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
    cases: Sequence[SeqCase],
    *,
    event_limit: int | None = None,
) -> tuple[int, int, int, int, int, int]:
    program = _seq_program(symbols)
    initializer_instructions = 0
    wrapper_retail_instructions = 0
    wrapper_candidate_instructions = 0
    event_instructions = 0
    event_count = 0
    running_count = 0
    for case in cases:
        expected = normalize_seq(case.data)
        initialized = execute_seq(retail, program, case)
        if initialized.initialized != expected[2]:
            raise AssertionError(f"{case.label}: retail/Rust SEQ initializer fields differ")
        if initialized.trace != ("SsSeqOpen", "InitSoundSeq", "ReadDeltaValue"):
            raise AssertionError(f"{case.label}: unexpected SEQ initializer call trace")
        parsed = parse_seq_events(case.data)
        retail_wrapper = execute_seq_wrapper(retail, symbols, case, candidate=False)
        candidate_wrapper = execute_seq_wrapper(retail, symbols, case, candidate=True)
        if candidate_wrapper.audio_state != retail_wrapper.audio_state:
            raise AssertionError(f"{case.label}: retail/C sequence audio state differs")
        if candidate_wrapper.record != retail_wrapper.record:
            raise AssertionError(f"{case.label}: retail/C initialized score record differs")
        if candidate_wrapper.open_flag != retail_wrapper.open_flag:
            raise AssertionError(f"{case.label}: retail/C sequence open flag differs")
        if candidate_wrapper.loaded_sequence != retail_wrapper.loaded_sequence:
            raise AssertionError(f"{case.label}: retail/C loaded sequence differs")
        if candidate_wrapper.trace != retail_wrapper.trace:
            raise AssertionError(f"{case.label}: retail/C sequence call trace differs")
        if retail_wrapper.record != initialized.record:
            raise AssertionError(f"{case.label}: wrapper/direct Sony initializer record differs")

        initial_audio = bytearray(AUDIO_STATE_SIZE)
        struct.pack_into("<Ih", initial_audio, 8, SEQ_VA, 0)
        allocated_sequence_id = struct.unpack_from(
            "<h", retail_wrapper.audio_state, 12
        )[0]
        provider = struct.pack("<hI", allocated_sequence_id, 0)
        actual = rust.call(
            "audio-seq",
            case.data,
            bytes(SCORE_RECORD_SIZE),
            bytes(initial_audio),
            provider,
        )
        if len(actual) != 9:
            raise AssertionError(
                f"{case.label}: Rust SEQ returned {len(actual)} blocks, expected nine"
            )
        if actual[:4] != expected:
            raise AssertionError(f"{case.label}: Rust SEQ normalized bytes differ")
        if not (
            retail_wrapper.loaded_sequence
            == candidate_wrapper.loaded_sequence
            == actual[3]
            == case.data
        ):
            raise AssertionError(f"{case.label}: wrapper/Rust loaded SEQ differs")
        if initialized.open_flag != actual[8]:
            raise AssertionError(f"{case.label}: retail/Rust open flag differs")
        if actual[4] != initialized.record:
            raise AssertionError(f"{case.label}: retail/Rust full initialized score differs")
        if actual[7] != retail_wrapper.audio_state:
            raise AssertionError(f"{case.label}: retail/Rust complete GAME audio state differs")
        if actual[8] != retail_wrapper.open_flag:
            raise AssertionError(f"{case.label}: retail/Rust wrapper open flag differs")

        # InitSoundSeq deliberately leaves some private bytes untouched.  A
        # nonzero seed proves that Rust changes exactly the same byte set.
        seed = bytearray((index * 37 + 11) & 0xFF for index in range(SCORE_RECORD_SIZE))
        seed[104:106] = b"\0\0"
        audio_seed = bytearray(
            (index * 53 + 17) & 0xFF for index in range(AUDIO_STATE_SIZE)
        )
        struct.pack_into("<h", audio_seed, 4, 0)
        struct.pack_into("<I", audio_seed, 8, SEQ_VA)
        struct.pack_into("<I", audio_seed, 16, 0)
        seeded_direct = execute_seq(retail, program, case, bytes(seed))
        seeded_retail = execute_seq_wrapper(
            retail,
            symbols,
            case,
            candidate=False,
            audio_seed=bytes(audio_seed),
            score_seed=bytes(seed),
        )
        seeded_candidate = execute_seq_wrapper(
            retail,
            symbols,
            case,
            candidate=True,
            audio_seed=bytes(audio_seed),
            score_seed=bytes(seed),
        )
        allocated_sequence_id = struct.unpack_from("<h", seeded_retail.audio_state, 12)[0]
        provider = struct.pack("<hI", allocated_sequence_id, 0)
        seeded_rust = rust.call(
            "audio-seq", case.data, bytes(seed), bytes(audio_seed), provider
        )
        if seeded_rust[4] != seeded_direct.record:
            raise AssertionError(
                f"{case.label}: seeded retail/Rust initialized score differs"
            )
        if seeded_retail.record != seeded_direct.record:
            raise AssertionError(f"{case.label}: seeded wrapper/direct score differs")
        if seeded_candidate.record != seeded_retail.record:
            raise AssertionError(f"{case.label}: seeded retail/C score differs")
        if seeded_candidate.audio_state != seeded_retail.audio_state:
            raise AssertionError(f"{case.label}: seeded retail/C audio state differs")
        if not (
            seeded_retail.loaded_sequence
            == seeded_candidate.loaded_sequence
            == seeded_rust[3]
            == case.data
        ):
            raise AssertionError(f"{case.label}: seeded wrapper/Rust loaded SEQ differs")
        if seeded_rust[7] != seeded_retail.audio_state:
            raise AssertionError(f"{case.label}: seeded retail/Rust audio state differs")
        if not (
            seeded_direct.open_flag
            == seeded_retail.open_flag
            == seeded_candidate.open_flag
            == seeded_rust[8]
        ):
            raise AssertionError(f"{case.label}: seeded sequence open flags differ")
        pumped, pump_steps = compare_event_stream(
            retail,
            symbols,
            case,
            retail_wrapper.record,
            parsed,
            actual[5],
            actual[6],
            event_limit=event_limit,
        )
        event_count += pumped
        running_count += sum(event.flags & 1 for event in parsed[:pumped])
        initializer_instructions += initialized.instructions
        initializer_instructions += seeded_direct.instructions
        wrapper_retail_instructions += retail_wrapper.instructions
        wrapper_candidate_instructions += candidate_wrapper.instructions
        wrapper_retail_instructions += seeded_retail.instructions
        wrapper_candidate_instructions += seeded_candidate.instructions
        event_instructions += pump_steps
    return (
        len(cases),
        event_count,
        running_count,
        initializer_instructions,
        wrapper_retail_instructions + wrapper_candidate_instructions,
        event_instructions,
    )


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    parser.add_argument("--max-cases", type=int)
    parser.add_argument("--max-events", type=int)
    args = parser.parse_args(argv)
    try:
        retail_dir = args.retail_dir or configured_retail_dir()
        vabs = vab_cases(retail_dir)
        sequences = seq_cases(retail_dir)
        if args.max_cases is not None:
            if args.max_cases <= 0:
                raise ValueError("--max-cases must be positive")
            vabs = vabs[: args.max_cases]
            sequences = sequences[: args.max_cases]
        if not args.no_rebuild:
            rebuild_units(["game.audio"])
        retail = RetailImage.load("GAME.EXE")
        symbols = GameSymbols.load()
        rust = RustCodec(args.rust_driver or build_driver())
        vab_count, vab_retail_steps, vab_candidate_steps = compare_vabs(
            retail, symbols, rust, vabs
        )
        if args.max_events is not None and args.max_events <= 0:
            raise ValueError("--max-events must be positive")
        seq_count, events, running, seq_init_steps, seq_wrapper_steps, seq_event_steps = (
            compare_sequences(
                retail,
                symbols,
                rust,
                sequences,
                event_limit=args.max_events,
            )
        )
        print(
            f"[audio-oracle] PASS: {vab_count} VAB banks retail/C/Rust; "
            f"{seq_count} SEQ files/{events} events/{running} running-status events "
            "retail/C/Rust score records, GAME state, open flags, and Sony pump "
            "SPU arguments agree"
        )
        print(
            f"[audio-oracle] instructions: VAB retail {vab_retail_steps}, "
            f"candidate {vab_candidate_steps}; SEQ init {seq_init_steps}, "
            f"wrapper retail+C {seq_wrapper_steps}, event pump {seq_event_steps}"
        )
        print(
            "[audio-oracle] shared boundary: real Sony VAB/SEQ parser providers; SPU "
            "service bodies are explicit hooks; compared SEQ state is the 176-byte "
            "score, 0x90-byte GAME audio state, and 4-byte open flag"
        )
        return 0
    except (OSError, RuntimeError, ValueError, AssertionError) as error:
        print(f"audio-oracle: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
