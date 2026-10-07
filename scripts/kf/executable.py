"""Compile C and link original Psy-Q objects/libraries without output rewriting."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

from scripts.kf.sdk import compile_c
from scripts.psxbuild.link import build_image as link_image
from scripts.psxbuild.link import OVERLAY_STARTUP as OVERLAY_STARTUP
from scripts.kf.executable_diff import compare_executables, render_html
from scripts.kf.graph import IncludeScanner
from scripts.kf.manifest import Manifest, Profile, Unit, load as load_manifest
from scripts.kf.paths import BUILD, REPO
from scripts.kf.readme import refresh_executable
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.sema.image import RetailImage


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
    units = [u for u in manifest.units if u.image == name and u.scope != 'vendored']
    report = link_image(
        name, root, units,
        lambda unit, output, index: compile_unit(unit, manifest.profiles[unit.profile], output, index),
        repo=REPO, load_address=IMAGE_LAYOUTS[name].load_address,
        bounds_source='config/link/overlay_bounds.asm')
    report.update(output_rewritten=False, retail_payload_inputs=[],
                  game_execution_tested=False, historical_toolchain_proven=False)
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
