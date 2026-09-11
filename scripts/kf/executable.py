"""Compile C and link original Psy-Q objects/libraries without output rewriting."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from scripts.kf.sdk import compile_c, dos_run, dos_text, tool_succeeded
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
OVERLAY_STARTUP = 'NONE2.OBJ'
# LIBAPI A74/A75/A76/A69: identical BIOS B0 selectors 4a/4b/4c/45.
GAME_SDK_ALIASES = {'InitCARD2': 'InitCARD', 'StartCARD2': 'StartCARD',
                    'StopCARD2': 'StopCARD', 'erase': 'delete'}


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compile_unit(unit: Unit, profile: Profile, root: Path, index: int) -> dict:
    if profile.language != 'c' or profile.aspsx_version != '1.07':
        raise ValueError(f'{unit.unit}: executable build requires C and ASPSX 1.07')
    if profile.optimization is None:
        raise ValueError(f'{unit.unit}: missing compiler optimization profile')
    result = compile_c(
        unit.source_path, root, f'U{index:04d}', compiler=profile.compiler,
        optimization=profile.optimization, small_data=profile.small_data,
        include_dirs=(REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
        cc1_flags=profile.cc1_flags, defines=unit.defines)
    dependencies = [unit.source, *IncludeScanner().headers(unit.source)]
    return {**result, 'unit': unit.unit, 'source': unit.source,
            'source_sha256': {name: file_hash(REPO / name) for name in dependencies}}


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
            assembler = Path(os.environ['PSYQ_ASMPSX'])
            shutil.copyfile(assembler, root / 'ASMPSX.EXE')
            report['tools']['ASMPSX.EXE'] = {
                'path': str(assembler), 'sha256': file_hash(assembler)}
            source = REPO / 'config/link/overlay_bounds.asm'
            shutil.copyfile(source, root / 'BOUNDS.ASM')
            dos_text(root / 'BOUNDS.ASM')
            report['boundaries'] = {
                'source': str(source.relative_to(REPO)), 'source_sha256': file_hash(source),
                'object': 'BOUNDS.OBJ',
                'assembler_command': 'asmpsx /l /oc+ BOUNDS.ASM,BOUNDS.OBJ > BOUNDS.TXT'}
            assembler_commands.append(report['boundaries']['assembler_command'])
            source = Path(os.environ['PSYQ_H2000_LIB']) / OVERLAY_STARTUP
            shutil.copyfile(source, root / OVERLAY_STARTUP)
            report['startup'] = {
                'file': OVERLAY_STARTUP,
                'path': str(source),
                'sha256': file_hash(source),
                'provenance': 'exact Release 2.5 H2000/LIB2000 object',
            }
        report['phase'] = 'assemble'
        dos_run(root, assembler_commands, 'asm')
        for unit in report['units']:
            tool_succeeded(root, unit['log'], unit['object'], b'LNK\x02')
            unit['object_sha256'] = file_hash(root / unit['object'])
        if report['startup'] and not (root / OVERLAY_STARTUP).read_bytes().startswith(b'LNK\x02'):
            raise ValueError(f'{OVERLAY_STARTUP}: expected a Psy-Q LNK object')
        if report['startup']:
            tool_succeeded(root, 'BOUNDS.TXT', 'BOUNDS.OBJ', b'LNK\x02')
            report['boundaries']['object_sha256'] = file_hash(root / 'BOUNDS.OBJ')
        for library in LIBRARIES[name]:
            filename = library + '.LIB'
            source = Path(os.environ['PSYQ_LIB']) / filename
            shutil.copyfile(source, root / filename)
            report['libraries'].append({'file': filename, 'path': str(source),
                                        'sha256': file_hash(source)})
        report['phase'] = 'link'
        stem = name.removesuffix('.EXE')
        commands = [f'\torg ${IMAGE_LAYOUTS[name].load_address:08x}',
                    *(f'\tinclude "{u["object"]}"' for u in report['units']),
                    *([f'\tinclude "{OVERLAY_STARTUP}"'] if report['startup'] else []),
                    *(['\tinclude "BOUNDS.OBJ"'] if report['startup'] else []),
                    *(f'\tinclib "{library["file"]}"' for library in report['libraries']),
                    *(f'{alias} alias {symbol}' for alias, symbol in
                      (GAME_SDK_ALIASES.items() if name == 'GAME.EXE' else ())),
                    'bssdata group bss', '\tsection .sbss,bssdata',
                    *(['\tsection .bss_start,bssdata'] if report['startup'] else []),
                    '\tsection .bss,bssdata',
                    *(['\tsection .bss_end,bssdata'] if report['startup'] else []),
                    f'\tregs pc={ENTRY}']
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        report['linker_command'] = f'psylink /c @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT'
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')
        report['phase'] = 'convert'
        report['converter_command'] = f'cpe2x {stem}.CPE > CONVERT.TXT'
        dos_run(root, [report['converter_command']], 'convert')
        executable = root / name
        if not executable.is_file():
            raise RuntimeError('CPE2X produced no executable; see CONVERT.TXT')
        actual = executable.read_bytes()
        if len(actual) < 2048 or actual[:8] != b'PS-X EXE':
            raise ValueError('CPE2X produced an invalid executable')
        report.update(linked=True, phase='complete', executable=str(executable),
                      cpe=str(root / (stem + '.CPE')))
    except (KeyError, OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        (root / name).unlink(missing_ok=True)
        report['error'] = str(error)
    (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
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
        verified = {name: RetailImage.load(name) for name in names} if args.compare_only else {}
        reports = []
        for name in names:
            root = BUILD / 'link' / name[:-4].lower()
            report = ({'image': name, 'mode': 'compare-existing', 'executable': str(root / name)}
                      if args.compare_only else build_image(name, manifest, root))
            if args.compare_only:
                actual = (root / name).read_bytes()
                report['comparison'] = compare(actual, verified[name])
                report['layout_tolerant'] = compare_executables(verified[name].data, actual)
                diff = report['comparison']
                print(f'{name}: existing EXE has {diff["linked_size"]} bytes; '
                      f'{diff["differing_bytes"]} file bytes differ from retail')
                fuzzy = report['layout_tolerant']
                print(f'  island-aligned byte similarity: {fuzzy["byte_similarity_percent"]:.2f}%; '
                      f'nonzero bytes: {fuzzy["nonzero_byte_similarity_percent"]:.2f}%; '
                      f'{fuzzy["island_count"]} islands (heuristic, not exactness)')
                html = root / 'fuzzy-comparison.html'
                html.write_text(render_html(name, diff, fuzzy))
                print(f'  report: {html}')
            elif report['linked']:
                print(f'{name}: built {report["executable"]}')
            else:
                print(f'{name}: {report["phase"]} failed; see {root}: {report["error"]}')
            filename = 'fuzzy-comparison.json' if args.compare_only else 'build.json'
            (root / filename).write_text(json.dumps(report, indent=2) + '\n')
            reports.append(report)
        (BUILD / 'link' / filename).write_text(json.dumps(reports, indent=2) + '\n')
        if args.compare_only:
            refresh_executable(BUILD / 'link')
        return int(any(not report.get('linked', args.compare_only) for report in reports))
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
