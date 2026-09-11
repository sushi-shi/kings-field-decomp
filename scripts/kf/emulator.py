"""Build and launch hash-controlled retail and reconstructed disc images."""

from __future__ import annotations

import argparse
import hashlib
import os
import re
from pathlib import Path

from scripts.kf.local_config import configured_retail_dir, validate_retail_dir


RETAIL_DISC_SHA256 = "ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9"
SECTOR_SIZE = 2352
PAYLOAD_OFFSET = 24
PAYLOAD_SIZE = 2048
PATCHES = (
    ("PSX.EXE", 38, Path("psx/PSX.EXE")),
    ("GAME.EXE", 40, Path("game/GAME.EXE")),
    ("OPEN.EXE", 181, Path("open/OPEN.EXE")),
)


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _cue_bin(cue: Path) -> Path:
    matches: list[str] = []
    pattern = re.compile(r'^\s*FILE\s+(?:"([^"]+)"|(\S+))\s+BINARY\s*$', re.IGNORECASE)
    for line in cue.read_text(encoding="utf-8-sig").splitlines():
        match = pattern.match(line)
        if match:
            matches.append(match.group(1) or match.group(2))
    if len(matches) != 1:
        raise ValueError(f"{cue}: expected exactly one binary FILE entry")
    path = Path(matches[0])
    return (path if path.is_absolute() else cue.parent / path).resolve()


def _disc_from_directory(directory: Path) -> Path:
    cues = sorted(directory.glob("*.cue"))
    if len(cues) == 1:
        return _cue_bin(cues[0])
    bins = sorted(directory.glob("*.bin"))
    if not cues and len(bins) == 1:
        return bins[0].resolve()
    raise ValueError(
        f"{directory}: expected exactly one .cue, or exactly one .bin when no cue exists"
    )


def retail_disc() -> Path:
    configured = os.environ.get("KF_RETAIL_DISC")
    if configured:
        path = Path(configured).expanduser().resolve()
    else:
        path = configured_retail_dir().parent
    if path.is_dir():
        path = _disc_from_directory(path)
    elif path.suffix.casefold() == ".cue":
        path = _cue_bin(path)
    if not path.is_file():
        raise ValueError(f"{path}: retail disc binary is missing")
    data = path.read_bytes()
    digest = _sha256(data)
    if digest != RETAIL_DISC_SHA256:
        raise ValueError(
            f"{path}: retail disc SHA-256 {digest} does not match SLPS-00017"
        )
    return path


def retail_resources(disc: Path) -> Path:
    try:
        return configured_retail_dir()
    except ValueError:
        return validate_retail_dir(disc.parent / "disc")


def _tables() -> tuple[list[int], list[int], list[int]]:
    edc: list[int] = []
    for value in range(256):
        result = value
        for _ in range(8):
            result = (result >> 1) ^ (0xD8018001 if result & 1 else 0)
        edc.append(result)

    forward: list[int] = []
    backward = [0] * 256
    for value in range(256):
        result = value << 1
        if result & 0x100:
            result ^= 0x11D
        forward.append(result)
        backward[value ^ result] = value
    return edc, forward, backward


EDC_TABLE, ECC_FORWARD, ECC_BACKWARD = _tables()


def _edc(data: bytes | bytearray) -> bytes:
    result = 0
    for value in data:
        result = (result >> 8) ^ EDC_TABLE[(result ^ value) & 0xFF]
    return result.to_bytes(4, "little")


def _ecc(
    source: bytes | bytearray,
    major_count: int,
    minor_count: int,
    major_multiplier: int,
    minor_increment: int,
) -> bytes:
    size = major_count * minor_count
    output = bytearray(major_count * 2)
    for major in range(major_count):
        index = (major >> 1) * major_multiplier + (major & 1)
        first = second = 0
        for _ in range(minor_count):
            value = source[index]
            index = (index + minor_increment) % size
            first ^= value
            second ^= value
            first = ECC_FORWARD[first]
        first = ECC_BACKWARD[ECC_FORWARD[first] ^ second]
        output[major] = first
        output[major + major_count] = first ^ second
    return bytes(output)


def regenerate_sector(sector: bytearray) -> None:
    if len(sector) != SECTOR_SIZE or sector[15] != 2 or sector[18] & 0x20:
        raise ValueError("candidate patch requires a raw Mode 2 Form 1 sector")
    sector[2072:2076] = _edc(sector[16:2072])
    header = sector[12:16]
    sector[12:16] = bytes(4)
    sector[2076:2248] = _ecc(sector[12:2248], 86, 24, 2, 86)
    sector[2248:2352] = _ecc(sector[12:2248], 52, 43, 86, 88)
    sector[12:16] = header


def write_cue(path: Path, binary: Path) -> Path:
    if '"' in str(binary):
        raise ValueError(f"{binary}: quotes are not supported in cue paths")
    path.write_text(
        f'FILE "{binary}" BINARY\n'
        "  TRACK 01 MODE2/2352\n"
        "    INDEX 01 00:00:00\n",
        encoding="utf-8",
    )
    return path


def build_candidate(repo: Path, disc: Path, output: Path) -> Path:
    resources = retail_resources(disc)
    candidate_root = Path(
        os.environ.get("KF_CANDIDATE_DIR", str(repo / "build/link"))
    ).expanduser().resolve()
    raw = bytearray(disc.read_bytes())

    selected_text = os.environ.get("KF_CANDIDATE_IMAGES", "PSX.EXE,GAME.EXE,OPEN.EXE")
    selected = {name.strip().upper() for name in selected_text.split(",") if name.strip()}
    known = {name for name, _, _ in PATCHES}
    if not selected or selected - known:
        choices = ", ".join(sorted(known))
        raise ValueError(f"KF_CANDIDATE_IMAGES must select from: {choices}")

    for name, lba, relative in PATCHES:
        if name not in selected:
            continue
        retail = (resources / name).read_bytes()
        candidate_path = candidate_root / relative
        if not candidate_path.is_file():
            raise ValueError(f"{candidate_path}: linked candidate executable is missing")
        candidate = candidate_path.read_bytes()
        if not candidate.startswith(b"PS-X EXE"):
            raise ValueError(f"{candidate_path}: not a PS-X EXE")
        if len(candidate) > len(retail):
            raise ValueError(
                f"{candidate_path}: {len(candidate)} bytes exceed the {len(retail)}-byte retail extent"
            )

        payload = candidate + bytes(len(retail) - len(candidate))
        for offset in range(0, len(payload), PAYLOAD_SIZE):
            sector_number = lba + offset // PAYLOAD_SIZE
            start = sector_number * SECTOR_SIZE
            sector = bytearray(raw[start : start + SECTOR_SIZE])
            retail_chunk = retail[offset : offset + PAYLOAD_SIZE]
            if sector[PAYLOAD_OFFSET : PAYLOAD_OFFSET + PAYLOAD_SIZE] != retail_chunk:
                raise ValueError(f"{name}: retail disc payload mismatch at LBA {sector_number}")
            sector[PAYLOAD_OFFSET : PAYLOAD_OFFSET + PAYLOAD_SIZE] = payload[
                offset : offset + PAYLOAD_SIZE
            ]
            regenerate_sector(sector)
            raw[start : start + SECTOR_SIZE] = sector

        print(
            f"[kf-run-candidate] {name}: {len(candidate)} linked bytes, "
            f"{len(retail) - len(candidate)} zero-filled bytes",
            flush=True,
        )

    temporary = output.with_name(f".{output.name}.{os.getpid()}.tmp")
    temporary.write_bytes(raw)
    temporary.replace(output)
    print(f"[kf-run-candidate] disc SHA-256: {_sha256(raw)}", flush=True)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("retail", "candidate"))
    parser.add_argument("--prepare-only", action="store_true")
    args, pcsx_args = parser.parse_known_args()

    repo = Path(os.environ.get("KINGS_FIELD_DIR", Path.cwd())).resolve()
    output = repo / "build/emulator"
    output.mkdir(parents=True, exist_ok=True)
    disc = retail_disc()
    if args.mode == "candidate":
        disc = build_candidate(repo, disc, output / "candidate.bin")
        cue = write_cue(output / "candidate.cue", disc)
    else:
        cue = write_cue(output / "retail.cue", disc)

    if args.prepare_only:
        print(f"[kf-run-{args.mode}] prepared {cue}", flush=True)
        return 0

    pcsx = Path(os.environ["PCSX_REDUX_BIN"])
    bios = Path(os.environ["PCSX_REDUX_BIOS"])
    extra = pcsx_args[1:] if pcsx_args[:1] == ["--"] else pcsx_args
    command = [str(pcsx), "-run", "-bios", str(bios), "-iso", str(cue), *extra]
    print(f"[kf-run-{args.mode}] launching {cue}", flush=True)
    os.execv(str(pcsx), command)


if __name__ == "__main__":
    raise SystemExit(main())
