"""Calibrate LNK v2 alignment tags against the supplied PSYLINK 1.17.

These are synthetic data-only objects, not reconstructed game assembly. No
production alignment conversion, retail address or delinker output supplies
the expected result. Check the original linker's symbols and complete CPE.
"""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from psylink_order_smoke import assert_link_succeeded, run, symbol_address


def data_object(name: str, tag: int, payload: bytes) -> bytes:
    """LNK v2: CPU, section definition/switch, code record, XDEF, EOF."""
    symbol = name.encode("ascii")
    return (b"LNK\x02\x2e\x07"
            + struct.pack("<BHHBB", 16, 1, 0, tag, 5) + b".data"
            + struct.pack("<BH", 6, 1)
            + struct.pack("<BH", 2, len(payload)) + payload
            + struct.pack("<BHHIB", 12, 2, 1, 0, len(symbol)) + symbol + b"\x00")


def main() -> int:
    emulator = os.environ.get("PSYLINK_DOSBOX") or shutil.which("dosbox-x")
    if emulator is None or Path(emulator).name != "dosbox-x":
        raise RuntimeError("this original-linker control requires the flake's DOSBox-X")
    # Explicit observed pairs, not tag // 2: higher bits do not generalize.
    tag_alignment = {2: 1, 4: 2, 8: 4, 16: 16}
    pairs = ((2, 2), (4, 4), (8, 8), (16, 16), (8, 4), (4, 8), (16, 8), (8, 16))
    # Unknown bits may link without enforcing alignment. The production
    # importer must reject them, not extrapolate an ELF alignment from them.
    pairs += ((8, 1), (8, 32), (8, 64))
    observed_alignment = {**tag_alignment, 1: 1, 32: 1, 64: 1}
    with tempfile.TemporaryDirectory(prefix="kf-psylink-alignment-") as directory:
        root = Path(directory)
        shutil.copy2(Path(os.environ["PSYQ_BIN"]) / "PSYLINK.EXE", root / "PSYLINK.EXE")
        cases, commands = [], []
        for first_tag, second_tag in pairs:
            for size in (1, 3, 9, 17):
                for origin in (0x80010000, 0x80010003):
                    number = len(cases)
                    name = f"P{number}"
                    records = []
                    # PSYLINK's initial /o origin is rounded to four bytes,
                    # separately from the following input-section alignment.
                    cursor = (origin + 3) // 4 * 4
                    for symbol, tag, payload in (
                        (f"A{number}", first_tag, bytes(range(1, size + 1))),
                        (f"B{number}", second_tag, b"\xa5\x00\x5a"),
                    ):
                        path = root / f"{symbol}.OBJ"
                        path.write_bytes(data_object(symbol, tag, payload))
                        listing = subprocess.run(["psyk", "list", "--code", str(path)],
                                                 capture_output=True, text=True, check=True).stdout
                        for expected in ("Header : LNK version 2", "46 : Processor type 7",
                                         f"'.data' in group 0 alignment {tag}",
                                         f"2 : Code {len(payload)} bytes",
                                         f"'{symbol}' at offset 0 in section 1"):
                            if expected not in listing:
                                raise RuntimeError(f"synthetic LNK input changed: {expected}")
                        alignment = observed_alignment[tag]
                        cursor = (cursor + alignment - 1) // alignment * alignment
                        records.append((symbol, cursor, payload))
                        cursor += len(payload)
                    cases.append((name, records))
                    inputs = "+".join(f"{symbol}.OBJ" for symbol, _, _ in records)
                    commands.append(f"psylink /o${origin:x} {inputs},"
                                    f"{name}.CPE,{name}.SYM,{name}.MAP > {name}.TXT")
        # DOSBox limits repeated -c options. One bounded batch runs every case.
        (root / "PROBE.BAT").write_bytes(("\r\n".join(commands) + "\r\n").encode("ascii"))
        environment = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
                           XDG_CONFIG_HOME=str(root / "config"))
        run([emulator, "-silent", "-fastlaunch", "-set", "sdl output=surface",
             "-c", f"mount c {root}", "-c", "c:", "-c", "PROBE.BAT", "-exit"],
            cwd=root, env=environment)
        for name, records in cases:
            transcript = root / f"{name}.TXT"
            assert_link_succeeded(transcript)
            if "PSYLINK version 1.17" not in transcript.read_text():
                raise RuntimeError("alignment calibration used a different linker")
            expected_cpe = bytearray(b"CPE\x01\x08\x00")
            for symbol, address, payload in records:
                actual = symbol_address(root / f"{name}.SYM", symbol)
                if actual != address:
                    raise RuntimeError(f"{name}: {symbol}={actual:#x}, expected {address:#x}")
                expected_cpe.extend(struct.pack("<BII", 1, address, len(payload)) + payload)
            expected_cpe.append(0)
            if (root / f"{name}.CPE").read_bytes() != expected_cpe:
                raise RuntimeError(f"{name}: linked payload, extent or record placement changed")
    print(f"PSYLINK 1.17 alignment: {len(cases)} full-CPE controls pass; "
          "LNK v2 tags 2/4/8/16 select 1/2/4/16-byte alignment")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
