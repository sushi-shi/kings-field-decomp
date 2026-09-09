"""Original PSYLINK controls for fixed reservations versus exported XBSS.

Only synthetic data-only LNK v2 objects run here. Expected symbol addresses and
allocation extents are observed tables, not a guessed global ordering algorithm.
Complete CPE bytes distinguish reservations, load records and alignment holes.
"""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from psylink_order_smoke import assert_link_succeeded, run, symbol_address


ORIGIN = 0x80010000
LINKER_SHA256 = 'b98e7180fc33b4c94adf41dcf9cc73f469864d23aabb3f3fcedf5e9f2c1ca040'


@dataclass(frozen=True)
class Input:
    label: str
    common: tuple[tuple[str, int], ...]
    small: int = 16
    fixed: int = 0
    tag: int = 8
    reverse_ids: bool = False
    references: tuple[str, ...] = ()


@dataclass(frozen=True)
class Case:
    name: str
    inputs: tuple[Input, ...]
    # label, data offset, small start/end, fixed start/end (all origin-relative).
    regular: tuple[tuple[str, int, int, int, int, int], ...]
    # name, observed origin-relative address, observed reserved extent.
    common: tuple[tuple[str, int, int], ...]
    case_sensitive: bool = False


SINGLE = (('A', 0, 4, 0x14, 0x14, 0x14),)
ODD = (('A', 0, 4, 0xD, 0x10, 0x10),)
PAIR = (('one', 8), ('two', 4))
PAIR_PLACED = (('one', 0x14, 8), ('two', 0x1C, 4))
ODD_COMMON = (('one', 1), ('two', 3), ('three', 9))
ODD_PLACED = (('one', 0x10, 4), ('two', 0x14, 4), ('three', 0x18, 12))
HASH_NAMES = ('one', 'two', 'shared', 'ZZZ', 'AAA', 'AB', 'BA',
              'opening_input_action', 'PadIdentifier', 'open_graphics_runtime',
              'audio_state', 'map_floor_height_grid', 'abc', 'bca', 'cab',
              'sn_long_name_with_a_prefix', 'X' * 20, 'Y' * 20, 'Z' * 20)
# Observed native allocation order. These tables do not call a proposed hash
# implementation to calculate their expectations.
HASH_FOLDED = (7, 15, 11, 9, 6, 5, 4, 14, 13, 12, 0, 16, 1, 17, 3, 18, 10, 2, 8)
HASH_SENSITIVE = (7, 2, 6, 5, 10, 11, 4, 15, 9, 16, 17, 3, 18, 8, 14, 13, 12, 0, 1)
HASH_LONG_ORDER = (
    154, 123, 92, 61, 30, 162, 131, 100, 69, 38, 7, 170, 139, 108, 77, 46, 15,
    178, 147, 116, 85, 54, 23, 155, 124, 93, 62, 31, 163, 132, 101, 70, 39, 8,
    171, 140, 109, 78, 47, 16, 179, 148, 117, 86, 55, 24, 156, 125, 94, 63, 32, 1,
    164, 133, 102, 71, 40, 9, 172, 141, 110, 79, 48, 17, 149, 118, 87, 56, 25,
    157, 126, 95, 64, 33, 2, 165, 134, 103, 72, 41, 10, 173, 142, 111, 80, 49, 18,
    150, 119, 88, 57, 26, 158, 127, 96, 65, 34, 3, 166, 135, 104, 73, 42, 11,
    174, 143, 112, 81, 50, 19, 151, 120, 89, 58, 27, 159, 128, 97, 66, 35, 4,
    167, 136, 105, 74, 43, 12, 175, 144, 113, 82, 51, 20, 152, 121, 90, 59, 28,
    160, 129, 98, 67, 36, 5, 168, 137, 106, 75, 44, 13, 176, 145, 114, 83, 52, 21,
    153, 122, 91, 60, 29, 161, 130, 99, 68, 37, 6, 169, 138, 107, 76, 45, 14,
    177, 146, 115, 84, 53, 22,
)
EMPTY_FIXED = (('A', 0, 4, 4, 4, 4),)
CASES = (
    Case('BASE', (Input('A', PAIR),), SINGLE, PAIR_PLACED),
    Case('REVREC', (Input('A', PAIR[::-1]),), SINGLE, PAIR_PLACED),
    Case('REVID', (Input('A', PAIR, reverse_ids=True),), SINGLE, PAIR_PLACED),
    Case('NAMES', (Input('A', (('zulu', 8), ('alpha', 4))),), SINGLE,
         (('zulu', 0x14, 8), ('alpha', 0x1C, 4))),
    Case('FIXED', (Input('A', PAIR, fixed=8),), (('A', 0, 4, 0x14, 0x14, 0x1C),),
         (('one', 0x1C, 8), ('two', 0x24, 4))),
    Case('TWOIN', (Input('A', (('one', 8),)), Input('B', (('two', 4),), small=24)),
         (('A', 0, 8, 0x18, 0x30, 0x30), ('B', 3, 0x18, 0x30, 0x30, 0x30)),
         (('one', 0x30, 8), ('two', 0x38, 4))),
    Case('REVIN', (Input('B', (('two', 4),), small=24), Input('A', (('one', 8),))),
         (('B', 0, 8, 0x20, 0x30, 0x30), ('A', 3, 0x20, 0x30, 0x30, 0x30)),
         (('one', 0x30, 8), ('two', 0x38, 4))),
    Case('COAL', (Input('A', (('shared', 4), ('one', 8))),
                  Input('B', (('shared', 16), ('two', 4)))),
         (('A', 0, 8, 0x18, 0x28, 0x28), ('B', 3, 0x18, 0x28, 0x28, 0x28)),
         (('one', 0x28, 8), ('two', 0x30, 4), ('shared', 0x34, 16))),
    Case('REVCOAL', (Input('B', (('shared', 16), ('two', 4))),
                     Input('A', (('shared', 4), ('one', 8)))),
         (('B', 0, 8, 0x18, 0x28, 0x28), ('A', 3, 0x18, 0x28, 0x28, 0x28)),
         (('one', 0x28, 8), ('two', 0x30, 4), ('shared', 0x34, 16))),
    Case('ODDS', (Input('A', ODD_COMMON, small=9),), ODD, ODD_PLACED),
    Case('ALIGN16', (Input('A', ODD_COMMON, small=9, tag=16),), ODD, ODD_PLACED),
    Case('RENAMED', (Input('A', (('ZZZ', 8), ('AAA', 4))),), SINGLE,
         (('AAA', 0x14, 4), ('ZZZ', 0x18, 8))),
    Case('REVNAME', (Input('A', (('AAA', 4), ('ZZZ', 8))),), SINGLE,
         (('AAA', 0x14, 4), ('ZZZ', 0x18, 8))),
    Case('COLLIDE', (Input('A', (('AB', 8), ('BA', 4))),), SINGLE,
         (('BA', 0x14, 4), ('AB', 0x18, 8))),
    Case('REVCOLL', (Input('A', (('BA', 4), ('AB', 8))),), SINGLE,
         (('AB', 0x14, 8), ('BA', 0x1C, 4))),
    Case('REFCOLL', (Input('A', (('AB', 8), ('BA', 4)), references=('BA',)),), SINGLE,
         (('AB', 0x14, 8), ('BA', 0x1C, 4))),
    Case('FIXODD', (Input('A', (('one', 3), ('two', 5)), small=9, fixed=3),),
         (('A', 0, 4, 0xD, 0x10, 0x13),), (('one', 0x13, 4), ('two', 0x17, 8))),
    Case('TAG2', (Input('A', ODD_COMMON, small=9, fixed=1, tag=2),),
         (('A', 0, 4, 0xD, 0xD, 0xE),),
         (('one', 0xE, 4), ('two', 0x12, 4), ('three', 0x16, 12))),
    Case('FIXTWO', (Input('A', (('one', 8),), fixed=5),
                    Input('B', (('two', 4),), small=9, fixed=3)),
         (('A', 0, 8, 0x18, 0x24, 0x29), ('B', 3, 0x18, 0x21, 0x2C, 0x2F)),
         (('one', 0x2F, 8), ('two', 0x37, 4))),
    Case('HASHFOLD', (Input('A', tuple((name, 4) for name in HASH_NAMES), small=0),),
         EMPTY_FIXED, tuple((HASH_NAMES[index], 4 + 4 * rank, 4)
                            for rank, index in enumerate(HASH_FOLDED))),
    Case('HASHCASE', (Input('A', tuple((name, 4) for name in HASH_NAMES), small=0),),
         EMPTY_FIXED, tuple((HASH_NAMES[index], 4 + 4 * rank, 4)
                            for rank, index in enumerate(HASH_SENSITIVE)), case_sensitive=True),
    Case('HASHLONG', (Input('A', tuple(('N' + 'A' * n, 4) for n in range(1, 180)), small=0),),
         EMPTY_FIXED, tuple(('N' + 'A' * n, 4 + 4 * rank, 4)
                            for rank, n in enumerate(HASH_LONG_ORDER)), case_sensitive=True),
)


def section(number: int, name: str, tag: int) -> bytes:
    encoded = name.encode('ascii')
    return struct.pack('<BHHBB', 16, number, 0, tag, len(encoded)) + encoded


def symbol(number: int, section: int, value: int, name: str, kind: int = 12) -> bytes:
    encoded = name.encode('ascii')
    return struct.pack('<BHHIB', kind, number, section, value, len(encoded)) + encoded


def data_object(item: Input) -> bytes:
    blob = bytearray(b'LNK\x02\x2e\x07')
    blob += section(1, '.data', 2) + section(2, '.sbss', 8) + section(3, '.bss', item.tag)
    payload = item.label.encode('ascii') + b'\0\xa5'
    blob += struct.pack('<BHBH', 6, 1, 2, len(payload)) + payload
    blob += symbol(10, 1, 0, item.label + '_data')
    for number, size, name in ((2, item.small, 'small'), (3, item.fixed, 'fixed')):
        blob += struct.pack('<BHBI', 6, number, 8, size)
        blob += symbol(11 + number, number, 0, item.label + '_' + name)
        blob += symbol(21 + number, number, size, item.label + '_' + name + '_end')
    for index, name in enumerate(item.references):
        encoded = name.encode('ascii')
        blob += struct.pack('<BHB', 14, 2000 + index, len(encoded)) + encoded
    for index, (name, size) in enumerate(item.common):
        blob += symbol(110 - index if item.reverse_ids else 100 + index, 3, size, name, 48)
    return bytes(blob) + b'\0'


def validate_input(path: Path, item: Input) -> None:
    listing = subprocess.run(['psyk', 'list', '--code', str(path)],
                             capture_output=True, text=True, check=True).stdout
    expected = ['Header : LNK version 2', '46 : Processor type 7',
                "'.data' in group 0 alignment 2", "'.sbss' in group 0 alignment 8",
                f"'.bss' in group 0 alignment {item.tag}", '2 : Code 3 bytes',
                f"'{item.label}_data' at offset 0 in section 1", '0 : End of file']
    for number, size, name in ((2, item.small, 'small'), (3, item.fixed, 'fixed')):
        expected += [f'8 : Uninitialized data, {size} bytes',
                     f"'{item.label}_{name}' at offset 0 in section {number}",
                     f"'{item.label}_{name}_end' at offset {size:x} in section {number}"]
    for index, (name, size) in enumerate(item.common):
        number = 110 - index if item.reverse_ids else 100 + index
        expected.append(f"48 : XBSS symbol number {number:x} '{name}' size {size:x} in section 3")
    for index, name in enumerate(item.references):
        expected.append(f"14 : XREF symbol number {2000 + index:x} '{name}'")
    for line in expected:
        if line not in listing:
            raise RuntimeError(f'{path.name}: missing decoded input {line!r}\n{listing}')
    if listing.count('48 : XBSS ') != len(item.common):
        raise RuntimeError(f'{path.name}: unexpected XBSS input records')


def verify_output(root: Path, case: Case) -> None:
    transcript = root / f'{case.name}.TXT'
    assert_link_succeeded(transcript)
    if 'PSYLINK version 1.17' not in transcript.read_text():
        raise RuntimeError('allocation calibration used a different linker')
    expected = bytearray(b'CPE\x01\x08\x00')

    def check_symbol(name, offset):
        actual = symbol_address(root / f'{case.name}.SYM', name,
                                case_sensitive=case.case_sensitive)
        if actual != ORIGIN + offset:
            raise RuntimeError(f'{case.name}: {name}={actual:#x}, expected {ORIGIN + offset:#x}')

    def load(offset, payload):
        if payload:
            expected.extend(struct.pack('<BII', 1, ORIGIN + offset, len(payload)) + payload)

    for label, data, small, small_end, fixed, fixed_end in case.regular:
        for name, offset in zip(('data', 'small', 'small_end', 'fixed', 'fixed_end'),
                                (data, small, small_end, fixed, fixed_end)):
            check_symbol(label + '_' + name, offset)
        # CPE records follow input traversal, not increasing address order.
        load(data, label.encode('ascii') + b'\0\xa5')
        load(small, bytes(small_end - small))
        load(fixed, bytes(fixed_end - fixed))
    for name, offset, size in case.common:
        check_symbol(name, offset)
        load(offset, bytes(size))
    expected.append(0)
    if (root / f'{case.name}.CPE').read_bytes() != expected:
        raise RuntimeError(f'{case.name}: full CPE allocation/placement/payload differs')


def main() -> int:
    emulator = os.environ.get('PSYLINK_DOSBOX') or shutil.which('dosbox-x')
    if emulator is None or Path(emulator).name != 'dosbox-x':
        raise RuntimeError("this original-linker control requires the flake's DOSBox-X")
    linker = Path(os.environ['PSYQ_BIN']) / 'PSYLINK.EXE'
    if hashlib.sha256(linker.read_bytes()).hexdigest() != LINKER_SHA256:
        raise RuntimeError('allocation calibration requires the hash-pinned PSYLINK 1.17')
    with tempfile.TemporaryDirectory(prefix='kf-psylink-bss-') as directory:
        root = Path(directory)
        shutil.copy2(linker, root / 'PSYLINK.EXE')
        commands = []
        for index, case in enumerate(CASES):
            inputs = []
            for item in case.inputs:
                path = root / f'{item.label}{index}.OBJ'
                path.write_bytes(data_object(item))
                validate_input(path, item)
                inputs.append(path.name)
            sensitivity = '/c ' if case.case_sensitive else ''
            commands.append(f'psylink {sensitivity}/o${ORIGIN:x} ' + '+'.join(inputs)
                            + f',{case.name}.CPE,{case.name}.SYM,{case.name}.MAP > {case.name}.TXT')
        (root / 'PROBE.BAT').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                           XDG_CONFIG_HOME=str(root / 'config'))
        run([emulator, '-silent', '-fastlaunch', '-set', 'sdl output=surface',
             '-c', f'mount c {root}', '-c', 'c:', '-c', 'PROBE.BAT', '-exit'],
            cwd=root, env=environment)
        for case in CASES:
            verify_output(root, case)
    print(f'PSYLINK 1.17 BSS/XBSS: {len(CASES)} full-CPE controls pass; '
          'fixed reservations, coalescing, size rounding and order remain distinct')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
