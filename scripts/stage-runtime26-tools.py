"""Stage hash-verified host-tool candidates from the preserved Runtime 2.6 CD."""

from pathlib import Path
import hashlib
import shutil
import subprocess
import sys
import tempfile
import zipfile


EXPECTED = {
    'PSYQ/ASPSX.EXE': 'a0b22f4379e713f916b06c9c433ef806c5ed15487fce57bfca3e3bb1649ab740',
    'PSYQ/ASMPSX.EXE': '60be8a14a71d5e2643d2cee8894eaa82535b801587ae6ce0d07c05427f6e6f25',
    'PSXGRAPH/BIN/CPE2X.EXE': '641d95ebe8131c3503407518cb6110ed311cb5f87943d866296660ab98938af2',
    'PSXGRAPH/BIN/EXEHEADX.EXE': 'aade1fe284096e4d3f86a5a7fe19bcfa09ef77c771171e3625af97bbbbf527bb',
}


def main():
    archive, output = map(Path, sys.argv[1:])
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        with zipfile.ZipFile(archive) as package:
            names = [name for name in package.namelist() if name.lower().endswith('.bin')]
            if len(names) != 1:
                raise ValueError('expected one preserved CD track')
            track = package.read(names[0])
        if len(track) % 2352 or track[16 * 2352 + 16:16 * 2352 + 22] != b'\x01CD001':
            raise ValueError('expected the preserved MODE1/2352 ISO9660 track')
        iso = root / 'runtime26.iso'
        iso.write_bytes(b''.join(track[i + 16:i + 2064] for i in range(0, len(track), 2352)))
        subprocess.run(['7z', 'x', '-y', f'-o{root / "tools"}', str(iso), *EXPECTED],
                       check=True, stdout=subprocess.DEVNULL)
        (output / 'bin').mkdir(parents=True)
        rows = ['source\tsha256']
        for relative, digest in EXPECTED.items():
            source = root / 'tools' / relative
            if hashlib.sha256(source.read_bytes()).hexdigest() != digest:
                raise ValueError(f'{relative}: host-tool hash mismatch')
            shutil.copyfile(source, output / 'bin' / source.name)
            rows.append(relative + '\t' + digest)
        (output / 'MANIFEST.tsv').write_text('\n'.join(rows) + '\n')


if __name__ == '__main__':
    main()
