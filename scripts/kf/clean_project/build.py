#!/usr/bin/env python3
"""Build all three PlayStation executables using the pinned SDK."""

import json
import os
from pathlib import Path
import sys

from scripts.psxbuild.link import build_image
from scripts.psxbuild.sdk import compile_c


def main():
    repo = Path(__file__).resolve().parent
    manifest = json.loads((repo / 'build.json').read_text())

    def compile_one(unit, output, index):
        return compile_c(
            repo / unit['source'], output, f'U{index:04d}',
            include_dirs=(repo / 'include', repo / 'vendor/include',
                          Path(os.environ['PSYQ_INCLUDE'])),
            **unit['options'])

    failed = False
    for image in manifest['images']:
        report = build_image(
            image['name'], repo / 'build' / image['name'][:-4].lower(), image['units'],
            compile_one, repo=repo, load_address=image['load_address'],
            bounds_source='link/overlay_bounds.asm')
        if report['linked']:
            print(report['executable'], flush=True)
        else:
            print(f"{image['name']}: {report['phase']} failed: {report['error']}", file=sys.stderr)
            failed = True
    return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
