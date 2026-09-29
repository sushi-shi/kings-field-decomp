"""Prepare and launch a PlayStation disc containing the source-built programs."""

import argparse
import hashlib
import os
from pathlib import Path
import re
import struct

RETAIL_DISC_SHA256 = "ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9"
SECTOR_SIZE = 2352
PAYLOAD_OFFSET = 24
PAYLOAD_SIZE = 2048

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


def disc_path(requested: Path) -> Path:
    path = requested.expanduser().resolve()
    if path.is_dir():
        path = _disc_from_directory(path)
    elif path.suffix.lower() == '.cue':
        path = _cue_bin(path)
    if not path.is_file():
        raise ValueError(f'{path}: disc binary is missing')
    return path


def iso_files(raw: bytes, *, with_records=False) -> dict:
    """Read root file extents from the disc's ISO9660 primary volume descriptor."""
    def payload(lba, size):
        if lba < 0 or size < 0 or (lba + (size + 2047) // 2048) * SECTOR_SIZE > len(raw):
            raise ValueError('ISO9660 extent exceeds disc')
        return b''.join(raw[n * SECTOR_SIZE + PAYLOAD_OFFSET:
                            n * SECTOR_SIZE + PAYLOAD_OFFSET + PAYLOAD_SIZE]
                        for n in range(lba, lba + (size + 2047) // 2048))[:size]

    pvd = payload(16, PAYLOAD_SIZE)
    if pvd[:7] != b'\x01CD001\x01' or struct.unpack_from('<H', pvd, 128)[0] != PAYLOAD_SIZE:
        raise ValueError('expected ISO9660 with 2048-byte logical blocks')
    root_lba, root_size = struct.unpack_from('<I', pvd, 158)[0], struct.unpack_from('<I', pvd, 166)[0]
    directory = payload(root_lba, root_size)
    files = {}
    position = 0
    while position < len(directory):
        size = directory[position]
        if not size:
            position = (position // PAYLOAD_SIZE + 1) * PAYLOAD_SIZE
            continue
        record = directory[position:position + size]
        if size < 34 or len(record) != size or 33 + record[32] > size:
            raise ValueError('invalid ISO9660 directory record')
        if not record[25] & 2:
            name = record[33:33 + record[32]].decode('ascii').split(';')[0].upper()
            if name in files:
                raise ValueError(f'duplicate ISO9660 file: {name}')
            files[name] = (struct.unpack_from('<I', record, 2)[0],
                           struct.unpack_from('<I', record, 10)[0])
            if with_records:
                location = (root_lba + position // PAYLOAD_SIZE) * SECTOR_SIZE + \
                    PAYLOAD_OFFSET + position % PAYLOAD_SIZE
                files[name] += (location,)
        position += size
    return files


def prepare(disc: Path, executables: Path, cache: Path) -> Path:
    raw = bytearray(disc.read_bytes())
    if hashlib.sha256(raw).hexdigest() != RETAIL_DISC_SHA256:
        raise ValueError('disc SHA-256 does not match Japanese SLPS-00017')
    files = iso_files(raw, with_records=True)
    programs = {}
    digest = hashlib.sha256(raw)
    for name in ('PSX.EXE', 'GAME.EXE', 'OPEN.EXE'):
        path = executables / name
        if not path.is_file():
            raise ValueError(f'{path}: built executable is missing')
        program = path.read_bytes()
        if len(program) < 2048 or program[:8] != b'PS-X EXE':
            raise ValueError(f'{path}: invalid PS-X executable')
        if name not in files:
            raise ValueError(f'{name}: missing from the disc root')
        lba, capacity, _ = files[name]
        if capacity % PAYLOAD_SIZE or (lba + capacity // PAYLOAD_SIZE) * SECTOR_SIZE > len(raw):
            raise ValueError(f'{name}: invalid executable disc extent')
        programs[name] = program
        digest.update(name.encode())
        digest.update(hashlib.sha256(program).digest())
    # Include framing implementation in the cache key when it changes.
    digest.update(Path(__file__).read_bytes())
    output = cache / digest.hexdigest()
    output.mkdir(parents=True, exist_ok=True)
    target = output / 'game.bin'
    if not target.exists():
        changed = set()
        original_sectors = len(raw) // SECTOR_SIZE
        for name, program in programs.items():
            lba, capacity, record = files[name]
            if len(program) > capacity:
                lba = len(raw) // SECTOR_SIZE
                capacity = (len(program) + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE * PAYLOAD_SIZE
                for number in range(lba, lba + capacity // PAYLOAD_SIZE):
                    minute, remainder = divmod(number + 150, 75 * 60)
                    second, frame = divmod(remainder, 75)
                    if minute >= 100:
                        raise ValueError('rebuilt disc exceeds CD address range')
                    def bcd(n):
                        return (n // 10 << 4) | n % 10
                    sector = bytearray(SECTOR_SIZE)
                    sector[:12] = b'\0' + b'\xff' * 10 + b'\0'
                    sector[12:16] = bytes([bcd(minute), bcd(second), bcd(frame), 2])
                    sector[16:24] = bytes([0, 0, 8, 0]) * 2
                    raw.extend(sector)
                struct.pack_into('<I', raw, record + 2, lba)
                struct.pack_into('>I', raw, record + 6, lba)
                struct.pack_into('<I', raw, record + 10, len(program))
                struct.pack_into('>I', raw, record + 14, len(program))
                changed.add(record // SECTOR_SIZE)
            program += bytes(capacity - len(program))
            for offset in range(0, len(program), PAYLOAD_SIZE):
                start = (lba + offset // PAYLOAD_SIZE) * SECTOR_SIZE
                sector = raw[start:start + SECTOR_SIZE]
                sector[PAYLOAD_OFFSET:PAYLOAD_OFFSET + PAYLOAD_SIZE] = program[offset:offset + PAYLOAD_SIZE]
                raw[start:start + SECTOR_SIZE] = sector
                changed.add(start // SECTOR_SIZE)
        if len(raw) // SECTOR_SIZE != original_sectors:
            pvd = 16 * SECTOR_SIZE + PAYLOAD_OFFSET
            struct.pack_into('<I', raw, pvd + 80, len(raw) // SECTOR_SIZE)
            struct.pack_into('>I', raw, pvd + 84, len(raw) // SECTOR_SIZE)
            changed.add(16)
        for number in sorted(changed):
            start = number * SECTOR_SIZE
            sector = raw[start:start + SECTOR_SIZE]
            regenerate_sector(sector)
            raw[start:start + SECTOR_SIZE] = sector
        temporary = output / f'.game-{os.getpid()}.tmp'
        try:
            temporary.write_bytes(raw)
            temporary.replace(target)
        finally:
            temporary.unlink(missing_ok=True)
    return write_cue(output / 'game.cue', target)


def prepare_retail(disc: Path, cache: Path) -> Path:
    if hashlib.sha256(disc.read_bytes()).hexdigest() != RETAIL_DISC_SHA256:
        raise ValueError('disc SHA-256 does not match Japanese SLPS-00017')
    output = cache / 'retail' / hashlib.sha256(str(disc).encode()).hexdigest()
    output.mkdir(parents=True, exist_ok=True)
    return write_cue(output / 'game.cue', disc)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--disc', type=Path, default=os.environ.get('KF_RETAIL_DISC'))
    parser.add_argument('--executables', type=Path)
    parser.add_argument('--retail', action='store_true',
                        help='run the original disc, ignoring replacement executables')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('emulator_args', nargs=argparse.REMAINDER)
    args = parser.parse_args(argv)
    try:
        if not args.retail and not args.executables:
            raise ValueError('supply --executables DIR or --retail')
        if not args.disc:
            raise ValueError('supply --disc PATH or set KF_RETAIL_DISC')
        cache = Path(os.environ.get('XDG_CACHE_HOME', Path.home() / '.cache')) / 'kings-field'
        original = disc_path(args.disc)
        cue = prepare_retail(original, cache) if args.retail else prepare(original, args.executables, cache)
        print(f'Prepared {cue}', flush=True)
        if args.prepare_only:
            return 0
        data = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share')) / 'kings-field'
        data.mkdir(parents=True, exist_ok=True)
        os.chdir(data)
        extra = args.emulator_args
        if extra[:1] == ['--']:
            extra = extra[1:]
        emulator = os.environ['PCSX_REDUX_BIN']
        os.execv(emulator, [emulator, '-run', '-bios', os.environ['PCSX_REDUX_BIOS'],
                           '-iso', str(cue), *extra])
    except (OSError, ValueError, KeyError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
