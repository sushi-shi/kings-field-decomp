#!/usr/bin/env python3
"""Build the C++ game for PlayStation."""

import json
from pathlib import Path

from scripts.psxbuild.clang import compile_program
from scripts.psxbuild.link import build_image


def main():
    repo = Path(__file__).resolve().parent
    manifest = json.loads((repo / 'build.json').read_text())
    for image in manifest['images']:
        report = build_image(
            image['name'], repo / 'build' / image['name'][:-4].lower(), [image['units']],
            lambda units, root, index: compile_program(units, root, index, repo=repo),
            repo=repo, load_address=image['load_address'], bounds_source='link/overlay_bounds.asm')
        if not report['linked']:
            raise RuntimeError(f"{image['name']}: {report['error']}")
        print(report['executable'], flush=True)


if __name__ == '__main__':
    main()
