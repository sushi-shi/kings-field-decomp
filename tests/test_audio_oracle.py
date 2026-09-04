from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.audio_oracle import (
    AUDIO_STATE_SIZE,
    CANDIDATE_OBJECT,
    SCORE_RECORD_SIZE,
    SEQ_VA,
    VAB_HEADER_SIZE,
    VAB_OFFSET_TABLE_SIZE,
    VAB_PROGRAM_SIZE,
    VAB_PROGRAM_SLOTS,
    VAB_TONE_SIZE,
    VAB_TONES_PER_PROGRAM,
    compare_event_stream,
    compare_sequences,
    compare_vabs,
    execute_seq_wrapper,
    normalize_seq,
    normalize_vab,
    parse_seq_events,
    seq_cases,
    vab_cases,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import LOCAL_CONFIG
from scripts.kf.rust_codec import DEFAULT_DRIVER, RustCodec
from scripts.kf.sema.image import RetailImage


def synthetic_vab() -> tuple[bytes, bytes]:
    programs = 1
    samples = 2
    tone_at = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE
    offsets_at = tone_at + programs * VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE
    vh = bytearray(offsets_at + VAB_OFFSET_TABLE_SIZE)
    vb = bytes(range(48))
    struct.pack_into(
        "<4sII I HHHH4BI",
        vh,
        0,
        b"pBAV",
        6,
        0,
        len(vh) + len(vb),
        0,
        programs,
        1,
        samples,
        127,
        64,
        0,
        0,
        0,
    )
    vh[VAB_HEADER_SIZE] = 1
    vh[VAB_HEADER_SIZE + 70 * VAB_PROGRAM_SIZE] = 1
    vh[tone_at : tone_at + VAB_TONE_SIZE] = bytes(range(VAB_TONE_SIZE))
    struct.pack_into("<3H", vh, offsets_at, 0, 2, 4)
    return bytes(vh), vb


def synthetic_sequence() -> bytes:
    return (
        b"pQES"
        + struct.pack(">IH", 1, 480)
        + bytes((0x07, 0xA1, 0x20, 4, 2))
        + bytes(
            (
                0x00,
                0xC1,
                0x1D,
                0x81,
                0x00,
                0x20,
                0x00,
                0xFF,
                0x2F,
                0x00,
            )
        )
    )


class AudioOracleTests(unittest.TestCase):
    def test_vab_normalization_covers_fixed_slots_tones_samples_and_roundtrip(self) -> None:
        vh, vb = synthetic_vab()

        header, programs, tones, samples, encoded_vh, encoded_vb = normalize_vab(vh, vb)

        self.assertEqual(header, vh[:32])
        self.assertEqual(len(programs), 128 * 16)
        self.assertEqual(programs[70 * 16], 1)
        self.assertEqual(len(tones), 16 * 32)
        self.assertEqual(
            [struct.unpack_from("<HHII", samples, at) for at in (0, 12)],
            [(0, 2, 0, 16), (1, 4, 16, 32)],
        )
        self.assertEqual((encoded_vh, encoded_vb), (vh, vb))

    def test_seq_normalization_tracks_running_status_boundaries_and_initializer(self) -> None:
        data = synthetic_sequence()

        events = parse_seq_events(data)
        header, rows, initialized, roundtrip = normalize_seq(data)

        self.assertEqual(len(events), 3)
        self.assertEqual((events[0].status, events[0].flags), (0xC1, 0))
        self.assertEqual((events[1].delta, events[1].delta_len), (128, 2))
        self.assertEqual((events[1].status, events[1].flags), (0xC1, 1))
        self.assertEqual((events[2].kind, events[2].data1), (1, 0x2F))
        self.assertEqual(struct.unpack_from("<I", rows)[0], 3)
        self.assertEqual(struct.unpack("<4I", initialized), (480, 120, 0, 16))
        self.assertEqual((header, roundtrip), (data[:15], data))

    def test_seq_rejects_running_data_without_a_status(self) -> None:
        data = synthetic_sequence()[:15] + bytes((0, 0x40))

        with self.assertRaisesRegex(ValueError, "running data without status"):
            parse_seq_events(data)

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and CANDIDATE_OBJECT.is_file()
        and DEFAULT_DRIVER.is_file(),
        "local retail image, candidate object, and Rust driver are required",
    )
    def test_one_retail_candidate_and_rust_vab_and_seq(self) -> None:
        retail_dir = configured_retail_dir()
        retail = RetailImage.load("GAME.EXE")
        symbols = GameSymbols.load()
        rust = RustCodec()

        vab_count, _retail_steps, _candidate_steps = compare_vabs(
            retail, symbols, rust, vab_cases(retail_dir)[:1]
        )
        seq_count, events, running, *_steps = compare_sequences(
            retail, symbols, rust, seq_cases(retail_dir)[:1], event_limit=10
        )

        self.assertEqual((vab_count, seq_count), (1, 1))
        self.assertEqual(events, 10)
        self.assertGreaterEqual(running, 0)

        case = seq_cases(retail_dir)[0]
        parsed = parse_seq_events(case.data)
        wrapper = execute_seq_wrapper(retail, symbols, case, candidate=False)
        initial_audio = bytearray(AUDIO_STATE_SIZE)
        struct.pack_into("<Ih", initial_audio, 8, SEQ_VA, 0)
        provider = struct.pack(
            "<hI", struct.unpack_from("<h", wrapper.audio_state, 12)[0], 0
        )
        rust_blocks = rust.call(
            "audio-seq",
            case.data,
            bytes(SCORE_RECORD_SIZE),
            bytes(initial_audio),
            provider,
        )
        initialized = wrapper.record

        bad_states = bytearray(rust_blocks[5])
        bad_states[4] ^= 1
        with self.assertRaisesRegex(AssertionError, "score record differs"):
            compare_event_stream(
                retail,
                symbols,
                case,
                initialized,
                parsed,
                bytes(bad_states),
                rust_blocks[6],
                event_limit=1,
            )

        bad_callbacks = bytearray(rust_blocks[6])
        callback_at = 4
        callback_event = None
        for index in range(struct.unpack_from("<I", bad_callbacks)[0]):
            per_event = struct.unpack_from("<I", bad_callbacks, callback_at)[0]
            callback_at += 4
            if per_event:
                bad_callbacks[callback_at + 4] ^= 1
                callback_event = index
                break
            callback_at += per_event * 28
        self.assertIsNotNone(callback_event)
        with self.assertRaisesRegex(AssertionError, "retail SPU callbacks"):
            compare_event_stream(
                retail,
                symbols,
                case,
                initialized,
                parsed,
                rust_blocks[5],
                bytes(bad_callbacks),
                event_limit=callback_event + 1,
            )


if __name__ == "__main__":
    unittest.main()
