"""The shared compiler and native SDK assembler used by every source build."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

C_COMPILERS = {
    "gcc257-native": ("cpppsx-257", "cc1psx-257"),
    "gcc260-native": ("cpppsx-260", "cc1psx-260"),
}


def run_command(arguments, *, environment=None):
    result = subprocess.run(arguments, capture_output=True, env=environment, check=False)
    if result.returncode:
        detail = (result.stderr or result.stdout).decode(errors="replace")
        raise RuntimeError(f"command failed ({result.returncode}): {detail}")
    return result.stdout


def dos_run(root: Path, commands: list[str], phase: str) -> None:
    batch = phase.upper() + '.BAT'
    (root / batch).write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
    environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                       XDG_CONFIG_HOME=str(root / 'dosbox-config'))
    result = subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                             '-set', 'cpu cycles=max',
                             '-c', f'mount c "{root}"', '-c', 'c:', '-c', batch, '-exit'],
                            cwd=root, env=environment, capture_output=True, timeout=60)
    (root / (phase + '-dosbox.log')).write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'DOSBox failed during {phase}: {result.returncode}')


def tool_succeeded(root: Path, log: str, product: str, magic: bytes) -> None:
    transcript = (root / log).read_text(errors='replace')
    if not re.search(r'(?m)^0 error\(s\)', transcript):
        errors = [line.strip() for line in transcript.splitlines() if 'Error' in line]
        detail = errors[0] if errors else next(iter(transcript.strip().splitlines()), 'no diagnostic output')
        raise RuntimeError(f'{log}: {len(errors)} native tool errors; first: {detail}')
    path = root / product
    if not path.is_file() or not path.read_bytes().startswith(magic):
        raise RuntimeError(f'{log}: native tool produced no valid {product}')


def dos_text(path: Path) -> None:
    # The DOS reader needs CRLF. No directives, instructions or literals change.
    path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))


def compile_c(source: Path, root: Path, stem: str, *, compiler: str,
              optimization: str, small_data: int, include_dirs: tuple[Path, ...],
              cc1_flags: tuple[str, ...] = (), defines: tuple[str, ...] = (),
              cc1_override: Path | None = None,
              environment: dict[str, str] | None = None, run=run_command) -> dict:
    cpp, cc1 = C_COMPILERS[compiler]
    preprocessed, assembly = root / (stem + '.I'), root / (stem + '.S')
    includes = [argument for path in include_dirs for argument in ('-I', str(path.resolve()))]
    cpp_command = [cpp, '-lang-c', '-undef', '-nostdinc', *includes,
                   *(f'-D{value}' for value in defines), str(source.resolve())]
    preprocessed.write_bytes(run(cpp_command))
    cc1_command = [str(cc1_override) if cc1_override else cc1, '-quiet', '-g', f'-{optimization}',
                   f'-G{small_data}', *cc1_flags, str(preprocessed), '-o', str(assembly)]
    run(cc1_command, environment=environment)
    dos_text(assembly)
    return {'object': stem + '.OBJ', 'assembly': stem + '.S', 'log': stem + '.TXT',
            'preprocessor_command': cpp_command, 'compiler_command': cc1_command,
            'assembler_command': f'aspsx -g -G{small_data} -o {stem}.OBJ {stem}.S > {stem}.TXT',
            'assembly_sha256': hashlib.sha256(assembly.read_bytes()).hexdigest()}


def compile_classic(source: Path, root: Path, stem: str, *, include_dirs: tuple[Path, ...],
                    defines: tuple[str, ...] = (), **options) -> dict:
    """Use GCC's original MIPS headers and the driver definitions they require."""
    if options.get('compiler') != 'gcc257-native':
        raise ValueError('classic requires GCC 2.5.7 and its original headers')
    headers = Path(os.environ['PSYQ_C_INCLUDE'])
    provenance = {name: hashlib.sha256((headers / name).read_bytes()).hexdigest()
                  for name in ('stdarg.h', 'va-mips.h')}
    # gcc.c supplies the version; config/mips/psx.h selects little-endian MIPS.
    driver_defines = ('__GNUC__=2', '__GNUC_MINOR__=5', '__mips__', '__MIPSEL__')
    result = compile_c(source, root, stem, include_dirs=(headers, *include_dirs),
                       defines=(*driver_defines, *defines), **options)
    return {**result, 'compiler_headers': provenance}


def assemble(root: Path, stem: str, small_data: int) -> bytes:
    return assemble_many(root, (stem,), small_data)[stem]


def assemble_many(root: Path, stems: tuple[str, ...], small_data: int) -> dict[str, bytes]:
    shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
    for stem in stems:
        dos_text(root / (stem + '.S'))
        (root / (stem + '.OBJ')).unlink(missing_ok=True)
    dos_run(root, [f'aspsx -g -G{small_data} -o {stem}.OBJ {stem}.S > {stem}.TXT'
                   for stem in stems], 'asm')
    for stem in stems:
        tool_succeeded(root, stem + '.TXT', stem + '.OBJ', b'LNK\x02')
    return {stem: (root / (stem + '.OBJ')).read_bytes() for stem in stems}
