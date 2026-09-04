"""Run the complete GAME retail/C/Rust resource-parser comparison census.

Every family runs its full shipped corpus and synthetic controls. There is no
case/event limit in this aggregate entry point. Results retain the explicit
provider and I/O boundaries documented by each constituent oracle.
"""

from __future__ import annotations

import argparse
from importlib import import_module
from typing import Sequence

from scripts.kf.rust_codec import build_driver


ORACLES = (
    "tmd",
    "resource",
    "map_resource",
    "animation",
    "audio",
    "vab_state",
    "save",
    "save_write",
    "world_state",
    "world_persist",
)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--no-rebuild", action="store_true", help="explicitly reuse existing C objects"
    )
    args = parser.parse_args(argv)
    driver = build_driver()
    forwarded = ["--rust-driver", str(driver)]
    if args.no_rebuild:
        forwarded.append("--no-rebuild")
    for name in ORACLES:
        print(f"[codec-oracle] checking {name}", flush=True)
        result = import_module(f"scripts.kf.{name}_oracle").main(list(forwarded))
        if result != 0:
            print(f"[codec-oracle] FAIL: {name}", flush=True)
            return 1
    print(f"[codec-oracle] PASS: all {len(ORACLES)} complete comparison suites", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
