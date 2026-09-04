from __future__ import annotations

import struct
import unittest
from pathlib import Path
from unittest.mock import patch
from subprocess import CompletedProcess

from scripts.kf.rust_codec import RustCodec, decode_blocks, encode_blocks


class RustCodecTransportTests(unittest.TestCase):
    def test_framing_preserves_empty_and_binary_blocks(self) -> None:
        blocks = (b"", b"\0\xff\x12", bytes(range(256)))
        self.assertEqual(decode_blocks(encode_blocks(blocks)), blocks)

    def test_rejects_missing_truncated_and_trailing_frames(self) -> None:
        bad_frames = (b"", struct.pack("<I", 129), struct.pack("<I", 1),
                      struct.pack("<II", 1, 3) + b"ab", struct.pack("<I", 0) + b"x")
        for frame in bad_frames:
            with self.subTest(frame=frame), self.assertRaises(ValueError):
                decode_blocks(frame)

    def test_executes_requested_operation_and_decodes_its_response(self) -> None:
        result = CompletedProcess([], 0, stdout=encode_blocks([b"result"]), stderr=b"")
        with patch("scripts.kf.rust_codec.subprocess.run", return_value=result) as run:
            output = RustCodec(Path("/test/driver")).call("tmd", b"payload", b"offset")
        self.assertEqual(output, (b"result",))
        self.assertEqual(run.call_args.args[0], ["/test/driver", "tmd"])
        self.assertEqual(run.call_args.kwargs["input"], encode_blocks([b"payload", b"offset"]))

    def test_reports_driver_failure_instead_of_accepting_its_output(self) -> None:
        result = CompletedProcess([], 1, stdout=encode_blocks([b"bad"]), stderr=b"truncated input")
        with patch("scripts.kf.rust_codec.subprocess.run", return_value=result):
            with self.assertRaisesRegex(RuntimeError, "Rust tmd: truncated input"):
                RustCodec().call("tmd", b"")


if __name__ == "__main__":
    unittest.main()
