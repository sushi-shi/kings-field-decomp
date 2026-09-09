"""Compile C and link original Psy-Q objects/libraries without output rewriting."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

from scripts.kf.compile import C_COMPILERS, _run
from scripts.kf.executable_diff import compare_executables, render_html
from scripts.kf.graph import IncludeScanner
from scripts.kf.manifest import Manifest, Profile, Unit, load as load_manifest
from scripts.kf.paths import BUILD, REPO
from scripts.kf.readme import refresh_executable
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.sema.image import RetailImage


# Ordinary linker inputs. The native linker selects members from these archives.
LIBRARIES = {
    'PSX.EXE': ('LIBSN', 'LIBAPI'),
    'GAME.EXE': ('LIBSN', 'LIBCD', 'LIBSND', 'LIBSPU', 'LIBGTE', 'LIBGPU',
                 'LIBETC', 'LIBAPI', 'LIBPRESS'),
    'OPEN.EXE': ('LIBSN', 'LIBCD', 'LIBSND', 'LIBSPU', 'LIBGTE', 'LIBGPU',
                 'LIBETC', 'LIBAPI', 'LIBPRESS'),
}
ENTRY = '__SN_ENTRY_POINT'
OVERLAY_STARTUP = 'src/sdk/overlay_start.s'
OVERLAY_SDATA = 'src/sdk/overlay_sdata.s'
# LIBAPI A74/A75/A76/A69: identical BIOS B0 selectors 4a/4b/4c/45.
GAME_SDK_ALIASES = {'InitCARD2': 'InitCARD', 'StartCARD2': 'StartCARD',
                    'StopCARD2': 'StopCARD', 'erase': 'delete'}


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dos_run(root: Path, commands: list[str], phase: str) -> None:
    """Run original DOS tools; the batch file only invokes commands."""
    batch = phase.upper() + '.BAT'
    (root / batch).write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
    environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                       XDG_CONFIG_HOME=str(root / 'dosbox-config'))
    result = subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                             '-c', f'mount c {root}', '-c', 'c:', '-c', batch, '-exit'],
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


def compile_unit(unit: Unit, profile: Profile, root: Path, index: int) -> dict:
    if profile.language != 'c' or profile.aspsx_version != '1.07':
        raise ValueError(f'{unit.unit}: direct executable build requires C and pinned ASPSX 1.07')
    if profile.optimization is None:
        raise ValueError(f'{unit.unit}: missing compiler optimization profile')
    cpp, cc1, _ = C_COMPILERS[profile.compiler]
    stem = f'U{index:04d}'
    preprocessed = root / (stem + '.I')
    assembly = root / (stem + '.S')
    includes = ['-I', str(REPO / 'include'), '-I', os.environ['PSYQ_INCLUDE']]
    cpp_command = [cpp, '-lang-c', '-undef', '-nostdinc', *includes,
                   *(f'-D{value}' for value in unit.defines), str(unit.source_path)]
    preprocessed.write_bytes(_run(cpp_command))
    cc1_command = [cc1, '-quiet', f'-{profile.optimization}', f'-G{profile.small_data}',
                   *profile.cc1_flags, str(preprocessed), '-o', str(assembly)]
    _run(cc1_command)
    # ASPSX's DOS text reader requires CRLF. Only line endings change; every
    # compiler directive, instruction, label and literal passes through intact.
    assembly.write_bytes(assembly.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))
    dependencies = [unit.source, *IncludeScanner().headers(unit.source)]
    return {'unit': unit.unit, 'source': unit.source, 'object': stem + '.OBJ',
            'assembly': stem + '.S', 'log': stem + '.TXT',
            'preprocessor_command': cpp_command, 'compiler_command': cc1_command,
            'assembler_command': f'aspsx -G{profile.small_data} -o {stem}.OBJ {stem}.S > {stem}.TXT',
            'source_sha256': {name: file_hash(REPO / name) for name in dependencies},
            'assembly_sha256': file_hash(assembly)}


def cpe_loads(data: bytes) -> tuple[int | None, list[tuple[int, bytes]]]:
    """Read native CPE records for verification only; never produce output bytes."""
    if not data.startswith(b'CPE\x01'):
        raise ValueError('expected native CPE v1 output')
    position, entry, loads = 4, None, []
    while position < len(data):
        tag = data[position]
        position += 1
        if tag == 0:
            if position != len(data):
                raise ValueError('bytes after native CPE end record')
            return entry, loads
        if tag == 8:
            size = 1
        elif tag == 3:
            size = 6
            if position + size <= len(data):
                register, value = struct.unpack_from('<HI', data, position)
                if register == 0x90:
                    entry = value
        elif tag == 1:
            if position + 8 > len(data):
                raise ValueError('truncated CPE load record')
            address, length = struct.unpack_from('<II', data, position)
            size = 8 + length
            if position + size <= len(data):
                loads.append((address, data[position + 8:position + size]))
        else:
            raise ValueError(f'unsupported native CPE record {tag}')
        if position + size > len(data):
            raise ValueError('truncated native CPE record')
        position += size
    raise ValueError('missing native CPE end record')


def build_image(name: str, manifest: Manifest, root: Path) -> dict:
    """The EXE is CPE2X's unchanged output, with no retail bytes as inputs."""
    root = root.resolve()
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    report = {'image': name, 'linked': False, 'phase': 'compile', 'units': [],
              'libraries': [], 'startup': None, 'tools': {}, 'output_rewritten': False,
              'retail_payload_inputs': [], 'game_execution_tested': False,
              'historical_toolchain_proven': False}
    try:
        tools = {'ASPSX.EXE': Path(os.environ['PSYQ_ASPSX']),
                 **{tool: Path(os.environ['PSYQ_BIN']) / tool
                    for tool in ('PSYLINK.EXE', 'CPE2X.EXE')}}
        for tool, source in tools.items():
            shutil.copyfile(source, root / tool)
            report['tools'][tool] = {'path': str(source), 'sha256': file_hash(source)}
        units = [u for u in manifest.units if u.image == name and u.scope != 'vendored']
        if not units:
            raise ValueError(f'{name}: no C source units')
        for index, unit in enumerate(units):
            report['units'].append(compile_unit(unit, manifest.profiles[unit.profile], root, index))
        assembler_commands = [u['assembler_command'] for u in report['units']]
        if name != 'PSX.EXE':
            source = REPO / OVERLAY_STARTUP
            (root / 'START.S').write_bytes(source.read_bytes().replace(b'\n', b'\r\n'))
            command = 'aspsx -G0 -o START.OBJ START.S > START.TXT'
            report['startup'] = {'source': OVERLAY_STARTUP, 'source_sha256': file_hash(source),
                                 'object': 'START.OBJ', 'assembler_command': command,
                                 'provenance': 'reconstructed vendored NONE2 assembly family'}
            assembler_commands.append(command)
            source = REPO / OVERLAY_SDATA
            (root / 'SDATA.S').write_bytes(source.read_bytes().replace(b'\n', b'\r\n'))
            command = 'aspsx -G0 -o SDATA.OBJ SDATA.S > SDATA.TXT'
            report['startup']['anchor'] = {
                'source': OVERLAY_SDATA, 'source_sha256': file_hash(source),
                'object': 'SDATA.OBJ', 'assembler_command': command}
            assembler_commands.append(command)
        report['phase'] = 'assemble'
        dos_run(root, assembler_commands, 'asm')
        for unit in report['units']:
            tool_succeeded(root, unit['log'], unit['object'], b'LNK\x02')
            unit['object_sha256'] = file_hash(root / unit['object'])
        if report['startup']:
            tool_succeeded(root, 'START.TXT', 'START.OBJ', b'LNK\x02')
            report['startup']['object_sha256'] = file_hash(root / 'START.OBJ')
            tool_succeeded(root, 'SDATA.TXT', 'SDATA.OBJ', b'LNK\x02')
            report['startup']['anchor']['object_sha256'] = file_hash(root / 'SDATA.OBJ')
        for library in LIBRARIES[name]:
            filename = library + '.LIB'
            source = Path(os.environ['PSYQ_LIB']) / filename
            shutil.copyfile(source, root / filename)
            report['libraries'].append({'file': filename, 'path': str(source),
                                        'sha256': file_hash(source)})
        report['phase'] = 'link'
        stem = name.removesuffix('.EXE')
        commands = [f'\torg ${IMAGE_LAYOUTS[name].load_address:08x}',
                    *(['\tinclude "SDATA.OBJ"'] if report['startup'] else []),
                    *(f'\tinclude "{u["object"]}"' for u in report['units']),
                    *(['\tinclude "START.OBJ"'] if report['startup'] else []),
                    *(f'\tinclib "{library["file"]}"' for library in report['libraries']),
                    *(f'{alias} alias {symbol}' for alias, symbol in
                      (GAME_SDK_ALIASES.items() if name == 'GAME.EXE' else ())),
                    'bssdata group bss', '\tsection .sbss,bssdata',
                    '\tsection .bss,bssdata',
                    f'\tregs pc={ENTRY}']
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        report['linker_command'] = f'psylink /c @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT'
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')
        entry, loads = cpe_loads((root / (stem + '.CPE')).read_bytes())
        report['phase'] = 'convert'
        report['converter_command'] = f'cpe2x {stem}.CPE > CONVERT.TXT'
        dos_run(root, [report['converter_command']], 'convert')
        executable = root / name
        if not executable.is_file():
            raise RuntimeError('CPE2X produced no executable; see CONVERT.TXT')
        actual = executable.read_bytes()
        if len(actual) < 2048 or actual[:8] != b'PS-X EXE':
            raise ValueError('CPE2X produced an invalid executable')
        address = struct.unpack_from('<I', actual, 0x18)[0]
        if entry is None or struct.unpack_from('<I', actual, 0x10)[0] != entry:
            raise ValueError('native converter entry differs from linked CPE')
        for start, payload in loads:
            offset = 2048 + start - address
            if offset < 2048 or actual[offset:offset + len(payload)] != payload:
                raise ValueError('native converter load bytes differ from linked CPE')
        report.update(linked=True, phase='complete', executable=str(executable),
                      entry=entry, cpe=str(root / (stem + '.CPE')),
                      load_records=[{'address': start, 'size': len(payload)} for start, payload in loads],
                      native_load_bytes_verified=sum(len(payload) for _, payload in loads))
    except (KeyError, OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        (root / name).unlink(missing_ok=True)
        report['error'] = str(error)
    (root / 'comparison.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def compare(actual: bytes, image: RetailImage) -> dict:
    expected = image.data
    common = min(len(actual), len(expected))
    differences = [i for i in range(common) if actual[i] != expected[i]]
    differences.extend(range(common, max(len(actual), len(expected))))
    first = differences[0] if differences else None
    first_load = next((i for i in differences if i >= 0x800), None)
    return {'file_equal': actual == expected, 'retail_size': len(expected), 'linked_size': len(actual),
            'retail_sha256': hashlib.sha256(expected).hexdigest(),
            'linked_sha256': hashlib.sha256(actual).hexdigest(),
            'differing_bytes': len(differences), 'first_difference_file_offset': first,
            'first_difference_va': image.load_start + first - 0x800
            if first is not None and first >= 0x800 else None,
            'first_load_difference_file_offset': first_load,
            'first_load_difference_va': image.load_start + first_load - 0x800
            if first_load is not None else None,
            'first_load_difference_retail': expected[first_load:first_load+16].hex()
            if first_load is not None else None,
            'first_load_difference_linked': actual[first_load:first_load+16].hex()
            if first_load is not None else None,
            'header_differing_bytes': sum(i < 0x800 for i in differences),
            'load_differing_bytes': sum(i >= 0x800 for i in differences)}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image', choices=('psx', 'game', 'open'), action='append')
    parser.add_argument('--compare-only', action='store_true',
                        help='compare existing native EXEs without rebuilding or changing them')
    args = parser.parse_args(argv)
    names = [key.upper() + '.EXE' for key in args.image] if args.image else list(IMAGE_LAYOUTS)
    try:
        manifest = None if args.compare_only else load_manifest()
        verified = {name: RetailImage.load(name) for name in names}
        reports = []
        for name in names:
            root = BUILD / 'link' / name[:-4].lower()
            report = ({'image': name, 'mode': 'compare-existing', 'executable': str(root / name)}
                      if args.compare_only else build_image(name, manifest, root))
            if args.compare_only or report['linked']:
                actual = (root / name).read_bytes()
                report['comparison'] = compare(actual, verified[name])
                report['layout_tolerant'] = compare_executables(verified[name].data, actual)
                diff = report['comparison']
                mode = 'existing EXE has' if args.compare_only else 'native tools linked'
                print(f'{name}: {mode} {diff["linked_size"]} bytes; '
                      f'{diff["differing_bytes"]} file bytes differ from retail')
                fuzzy = report['layout_tolerant']
                print(f'  island-aligned byte similarity: {fuzzy["byte_similarity_percent"]:.2f}%; '
                      f'nonzero bytes: {fuzzy["nonzero_byte_similarity_percent"]:.2f}%; '
                      f'{fuzzy["island_count"]} islands (heuristic, not exactness)')
                html = root / ('fuzzy-comparison.html' if args.compare_only else 'comparison.html')
                html.write_text(render_html(name, diff, fuzzy))
                print(f'  report: {html}')
            else:
                print(f'{name}: {report["phase"]} failed; see {root}: {report["error"]}')
            filename = 'fuzzy-comparison.json' if args.compare_only else 'comparison.json'
            (root / filename).write_text(json.dumps(report, indent=2) + '\n')
            reports.append(report)
        (BUILD / 'link' / filename).write_text(json.dumps(reports, indent=2) + '\n')
        refresh_executable(BUILD / 'link')
        return int(any(not report.get('linked', args.compare_only) for report in reports))
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
