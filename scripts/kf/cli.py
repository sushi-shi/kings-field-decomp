"""King's Field umbrella command for initialization, builds, and match status."""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.graph import PHASES, configure_if_needed, emit, run_ninja
from scripts.kf.local_config import initialize
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import BUILD, NINJA, REPO
from scripts.kf.progress import bank, check, current_state, print_status
from scripts.kf.retail import IMAGE_LAYOUTS


IMAGE_ALIASES = {image_key(image): image for image in IMAGE_LAYOUTS}


def _images(values: list[str] | None) -> tuple[str, ...]:
    return tuple(IMAGE_ALIASES[value] for value in values) if values else tuple(IMAGE_LAYOUTS)


def _configure(args: argparse.Namespace) -> int:
    units, pruned = emit(NINJA, args.retail_dir)
    suffix = f", pruned {pruned} orphan(s)" if pruned else ""
    print(f"[configure] wrote {NINJA.relative_to(REPO)} ({units} units{suffix})")
    return 0


def _build(args: argparse.Namespace) -> int:
    configure_if_needed(
        args.reconfigure or args.retail_dir is not None,
        args.retail_dir,
    )
    targets = [
        f"{args.phase}-{value}" for value in args.image
    ] if args.image else [args.phase]
    return run_ninja(targets, jobs=args.jobs, verbose=args.verbose)


def _object_census() -> dict[str, str]:
    result = {}
    for path in sorted((BUILD / "objdiff").glob("*/base/*.o")):
        result[str(path.relative_to(REPO))] = hashlib.sha256(path.read_bytes()).hexdigest()
    return result


def _match(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    selected = _images(args.image)
    if args.unit:
        unit = manifest.by_name().get(args.unit)
        if unit is None:
            raise ValueError(f"unknown unit {args.unit!r}")
        if args.image and unit.image not in selected:
            raise ValueError(f"unit {args.unit!r} does not belong to the selected image")
        selected = (unit.image,)
    before = _object_census()
    configure_if_needed(
        args.reconfigure or args.retail_dir is not None,
        args.retail_dir,
    )
    targets = [f"all-{image_key(image)}" for image in selected]
    result = run_ninja(targets, jobs=args.jobs, verbose=args.verbose)
    if result:
        return result
    after = _object_census()
    changed = sorted(path for path, digest in after.items() if before.get(path) != digest)
    print(
        f"[match] {len(changed)} reconstruction object(s) changed"
        + (": " + ", ".join(Path(path).name for path in changed[:12]) if changed else "")
    )
    if args.unit:
        _manifest, _universe, rows, _failures = current_state(selected)
        row = next(item for item in rows if item.unit.unit == args.unit)
        pct = "not scored" if row.pct is None else f"{row.pct:.9f}%"
        print(f"[match] {args.unit}: {pct}")
    return print_status(selected, loose=args.loose, show_all=args.all)


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(prog="kf", description=__doc__)
    subs = root.add_subparsers(dest="command", required=True)

    init_parser = subs.add_parser("init", help="record and validate local retail files")
    init_parser.add_argument("--retail-dir", required=True, type=Path)

    configure = subs.add_parser("configure", help="generate build/build.ninja")
    configure.add_argument("--retail-dir", type=Path)

    build = subs.add_parser("build", help="configure if needed and run the Ninja graph")
    build.add_argument("phase", nargs="?", choices=PHASES, default="all")
    build.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    build.add_argument("-j", "--jobs", type=int)
    build.add_argument("-v", "--verbose", action="store_true")
    build.add_argument("--reconfigure", action="store_true")
    build.add_argument("--retail-dir", type=Path)

    match = subs.add_parser("match", help="build and summarize changed reconstruction units")
    match.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    match.add_argument("--unit")
    match.add_argument("-j", "--jobs", type=int)
    match.add_argument("-v", "--verbose", action="store_true")
    match.add_argument("--reconfigure", action="store_true")
    match.add_argument("--retail-dir", type=Path)
    match.add_argument("--all", action="store_true")
    match.add_argument(
        "--loose", action="store_true",
        help="count scores >=99.995%% as display-exact (default requires 100%%)",
    )

    status = subs.add_parser("status", help="read current progress without building")
    status.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    status.add_argument("--all", action="store_true")
    status.add_argument("--json", action="store_true")
    status.add_argument(
        "--loose", action="store_true",
        help="count scores >=99.995%% as display-exact (default requires 100%%)",
    )

    check_parser = subs.add_parser("check", help="fail on banked regressions or losses")
    check_parser.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    check_parser.add_argument("--strict", action="store_true")

    bank_parser = subs.add_parser("bank", help="manually update the score high-water ledger")
    bank_parser.add_argument("--dirty", action="store_true")

    sema_parser = subs.add_parser(
        "sema",
        add_help=False,
        help="query one retail executable's semantic inventories",
    )
    sema_parser.add_argument("sema_args", nargs=argparse.REMAINDER)
    lineage_parser = subs.add_parser(
        "lineage",
        add_help=False,
        help="verify cross-overlay order and Psy-Q archive anchors",
    )
    lineage_parser.add_argument("lineage_args", nargs=argparse.REMAINDER)
    inventory_parser = subs.add_parser(
        "inventory",
        add_help=False,
        help="validate or propose function/global semantic identities",
    )
    inventory_parser.add_argument("inventory_args", nargs=argparse.REMAINDER)
    return root


def main(argv: list[str] | None = None) -> int:
    raw = list(sys.argv[1:] if argv is None else argv)
    if raw and raw[0] == "sema":
        from scripts.kf.sema import main as sema_main

        return sema_main(raw[1:])
    if raw and raw[0] == "lineage":
        from scripts.kf.lineage import main as lineage_main

        return lineage_main(raw[1:])
    if raw and raw[0] == "inventory":
        from scripts.kf.inventory import main as inventory_main

        return inventory_main(raw[1:])
    args = parser().parse_args(raw)
    try:
        if args.command == "init":
            retail = initialize(args.retail_dir)
            print(f"[init] retail executables verified at {retail}")
            return 0
        if args.command == "configure":
            return _configure(args)
        if args.command == "build":
            return _build(args)
        if args.command == "match":
            return _match(args)
        if args.command == "status":
            return print_status(
                _images(args.image), loose=args.loose,
                as_json=args.json, show_all=args.all,
            )
        if args.command == "check":
            return check(_images(args.image), strict=args.strict)
        return bank(allow_dirty=args.dirty)
    except (OSError, RuntimeError, ValueError) as error:
        print(f"kf {args.command}: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
