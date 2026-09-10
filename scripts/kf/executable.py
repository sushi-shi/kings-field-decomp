"""Compare retired executable experiments; coherent SDK linking is unavailable."""

from __future__ import annotations

import argparse
import hashlib
import json

from scripts.kf.executable_diff import compare_executables, render_html
from scripts.kf.paths import BUILD
from scripts.kf.readme import refresh_executable
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.sema.image import RetailImage

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
                        help='compare retired experiment EXEs without rebuilding or changing them')
    args = parser.parse_args(argv)
    if not args.compare_only:
        parser.error(
            "coherent Psy-Q Release 2.5 linking is unavailable: the SDK's "
            "assembler requires its original software key, and the former "
            "external-assembler/native-compiler substitute has been removed"
        )
    names = [key.upper() + '.EXE' for key in args.image] if args.image else list(IMAGE_LAYOUTS)
    try:
        verified = {name: RetailImage.load(name) for name in names}
        reports = []
        for name in names:
            root = BUILD / 'link' / name[:-4].lower()
            report = {'image': name, 'mode': 'compare-existing-retired-experiment',
                      'executable': str(root / name)}
            actual = (root / name).read_bytes()
            report['comparison'] = compare(actual, verified[name])
            report['layout_tolerant'] = compare_executables(verified[name].data, actual)
            diff = report['comparison']
            print(f'{name}: retired experiment has {diff["linked_size"]} bytes; '
                  f'{diff["differing_bytes"]} file bytes differ from retail')
            fuzzy = report['layout_tolerant']
            print(f'  island-aligned byte similarity: {fuzzy["byte_similarity_percent"]:.2f}%; '
                  f'nonzero bytes: {fuzzy["nonzero_byte_similarity_percent"]:.2f}%; '
                  f'{fuzzy["island_count"]} islands (heuristic, not exactness)')
            html = root / 'fuzzy-comparison.html'
            html.write_text(render_html(name, diff, fuzzy))
            print(f'  report: {html}')
            filename = 'fuzzy-comparison.json'
            (root / filename).write_text(json.dumps(report, indent=2) + '\n')
            reports.append(report)
        (BUILD / 'link' / filename).write_text(json.dumps(reports, indent=2) + '\n')
        refresh_executable(BUILD / 'link')
        return 0
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
