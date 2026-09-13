"""Source launcher controls using a synthetic Mode 2 ISO9660 disc."""

import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

from scripts.psxbuild import disc


def directory_record(name, lba, size, *, directory=False):
    record = bytearray(33 + len(name) + (len(name) % 2 == 0))
    record[0] = len(record)
    struct.pack_into('<I', record, 2, lba)
    struct.pack_into('>I', record, 6, lba)
    struct.pack_into('<I', record, 10, size)
    struct.pack_into('>I', record, 14, size)
    record[25] = 2 if directory else 0
    record[32] = len(name)
    record[33:33 + len(name)] = name
    return record


def synthetic_disc():
    raw = bytearray(26 * disc.SECTOR_SIZE)
    for lba in range(26):
        raw[lba * disc.SECTOR_SIZE + 15] = 2
    pvd = bytearray(2048)
    pvd[:7] = b'\x01CD001\x01'
    struct.pack_into('<H', pvd, 128, 2048)
    pvd[156:190] = directory_record(b'\0', 17, 2048, directory=True)
    start = 16 * disc.SECTOR_SIZE + 24
    raw[start:start + 2048] = pvd
    records = b''.join(directory_record(name.encode() + b';1', lba, 4096)
                       for name, lba in (('PSX.EXE', 18), ('GAME.EXE', 20), ('OPEN.EXE', 22)))
    start = 17 * disc.SECTOR_SIZE + 24
    raw[start:start + len(records)] = records
    return bytes(raw)


class SourceDiscControls(unittest.TestCase):
    def test_extents_are_read_from_iso_directory(self):
        self.assertEqual(disc.iso_files(synthetic_disc()), {
            'PSX.EXE': (18, 4096), 'GAME.EXE': (20, 4096), 'OPEN.EXE': (22, 4096),
        })

    def test_all_programs_replaced_original_unchanged_and_cache_reusable(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            input_path = root / 'original disc.bin'
            input_path.write_bytes(original)
            programs = root / 'programs'
            programs.mkdir()
            for index, name in enumerate(('PSX.EXE', 'GAME.EXE', 'OPEN.EXE')):
                (programs / name).write_bytes(b'PS-X EXE' + bytes([index + 1]) * (2048 - 8))
            with mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                cue = disc.prepare(input_path, programs, root / 'cache')
                output = cue.with_suffix('.bin')
                stamp = output.stat().st_mtime_ns
                self.assertEqual(disc.prepare(input_path, programs, root / 'cache'), cue)
                self.assertEqual(output.stat().st_mtime_ns, stamp)
            self.assertEqual(input_path.read_bytes(), original)
            rebuilt = output.read_bytes()
            for name, (lba, _) in disc.iso_files(original).items():
                start = lba * disc.SECTOR_SIZE
                sector = bytearray(rebuilt[start:start + disc.SECTOR_SIZE])
                self.assertEqual(sector[24:2072], (programs / name).read_bytes())
                control = bytearray(sector)
                disc.regenerate_sector(control)
                self.assertEqual(control, sector)
                next_start = start + disc.SECTOR_SIZE
                self.assertEqual(rebuilt[next_start + 24:next_start + 2072], bytes(2048))
            self.assertEqual(rebuilt[:18 * disc.SECTOR_SIZE], original[:18 * disc.SECTOR_SIZE])

    def test_rejects_wrong_disc_missing_program_and_oversize(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            path = root / 'input.bin'
            path.write_bytes(original)
            with self.assertRaisesRegex(ValueError, 'SHA-256'):
                disc.prepare(path, root, root / 'cache')
            with mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                with self.assertRaisesRegex(ValueError, 'missing'):
                    disc.prepare(path, root, root / 'cache')
                (root / 'PSX.EXE').write_bytes(b'PS-X EXE' + bytes(4096))
                with self.assertRaisesRegex(ValueError, 'exceed'):
                    disc.prepare(path, root, root / 'cache')
            self.assertFalse((root / 'cache').exists())

    def test_cue_relative_path_with_spaces_and_ambiguous_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'my disc.bin').write_bytes(b'disc')
            cue = root / 'disc.cue'
            cue.write_text('FILE "my disc.bin" BINARY\n  TRACK 01 MODE2/2352\n')
            self.assertEqual(disc.disc_path(cue), root / 'my disc.bin')
            self.assertEqual(disc.disc_path(root), root / 'my disc.bin')
            (root / 'another.cue').write_text(cue.read_text())
            with self.assertRaises(ValueError):
                disc.disc_path(root)


if __name__ == '__main__':
    unittest.main()
