"""King's Field umbrella command for initialization, builds, and match status."""

from __future__ import annotations

import argparse
import hashlib
import os
import sys
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.graph import PHASES, configure_if_needed, emit, run_ninja
from scripts.kf.local_config import initialize
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import BUILD, NINJA, REPO
from scripts.kf.progress import bank, check, current_state, print_status
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int


IMAGE_ALIASES = {image_key(image): image for image in IMAGE_LAYOUTS}


def _images(values: list[str] | None) -> tuple[str, ...]:
    return tuple(IMAGE_ALIASES[value] for value in values) if values else tuple(IMAGE_LAYOUTS)


def _bank_functions(values: list[str] | None) -> tuple[tuple[str, int], ...]:
    selected: list[tuple[str, int]] = []
    for value in values or ():
        image, separator, address = value.partition(":")
        if not separator or image not in IMAGE_ALIASES or not address:
            raise ValueError(
                f"invalid bank function {value!r}; expected IMAGE:VA "
                "(for example game:0x800346a8)"
            )
        selected.append((IMAGE_ALIASES[image], parse_int(address)))
    return tuple(selected)


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
        for row in (item for item in rows if item.unit.unit == args.unit):
            pct = "not scored" if row.pct is None else f"{row.pct:.9f}%"
            print(f"[match] {args.unit} {row.target.name}: {pct}")
    return print_status(selected, loose=args.loose, show_all=args.all)


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(prog="kf", description=__doc__)
    subs = root.add_subparsers(dest="command", required=True)

    init_parser = subs.add_parser("init", help="record and validate local retail files")
    init_parser.add_argument("--retail-dir", required=True, type=Path)

    configure = subs.add_parser("configure", help="generate build/build.ninja")
    configure.add_argument("--retail-dir", type=Path)

    clangd = subs.add_parser("clangd", help="refresh editor commands and select shared-source context")
    clangd.add_argument("--image", choices=tuple(IMAGE_ALIASES))
    clangd.add_argument("--mode", choices=("modern", "retail"))

    types = subs.add_parser("check-types", help="check C sources with modern scoped enum types")
    types.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    types.add_argument("--unit", action="append")
    types.add_argument("-j", "--jobs", type=int, default=4)

    casts = subs.add_parser(
        "casts", help="count deduplicated written C-style casts with target-C Clang"
    )
    from scripts.kf.casts import add_arguments as add_cast_arguments

    add_cast_arguments(casts)

    build = subs.add_parser("build", help="configure if needed and run the Ninja graph")
    build.add_argument("phase", nargs="?", choices=PHASES, default="all")
    build.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    build.add_argument("-j", "--jobs", type=int)
    build.add_argument("-v", "--verbose", action="store_true")
    build.add_argument("--reconfigure", action="store_true")
    build.add_argument("--retail-dir", type=Path)

    trial = subs.add_parser(
        "try", help="compile one unit and diff it per function against its module target"
    )
    trial.add_argument("--unit", required=True)
    trial.add_argument("--source", type=Path)
    trial.add_argument("--context", type=int, default=2)
    hypotheses = subs.add_parser(
        "hypotheses", help="compile and strict-score a matrix of source hypotheses"
    )
    hypotheses.add_argument("manifest", type=Path)
    hypotheses.add_argument("-j", "--jobs", type=int, default=min(8, os.cpu_count() or 1))
    hypotheses.add_argument("--limit", type=int, default=256)
    hypotheses.add_argument("--keep-top", type=int, default=8)
    hypotheses.add_argument("--output", type=Path)
    hypotheses.add_argument("--instrumented-compiler", type=Path)
    hypotheses.add_argument("--trace-dir", type=Path)
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

    check_parser = subs.add_parser(
        "check",
        help="fail on data mismatches, banked regressions, or losses",
    )
    check_parser.add_argument("--image", action="append", choices=tuple(IMAGE_ALIASES))
    check_parser.add_argument("--strict", action="store_true")

    bank_parser = subs.add_parser("bank", help="manually update the score high-water ledger")
    bank_parser.add_argument("--dirty", action="store_true")
    bank_parser.add_argument(
        "--unit", action="append",
        help="bank only this unit; every selected function must be exactly 100%%",
    )
    bank_parser.add_argument(
        "--function", action="append", metavar="IMAGE:VA",
        help="bank only this image/address function; it must be exactly 100%%",
    )

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
    verify_parser = subs.add_parser(
        "verify",
        add_help=False,
        help="cleanliness ratchet board and data-section matching vs retail",
    )
    verify_parser.add_argument("verify_args", nargs=argparse.REMAINDER)
    link_parser = subs.add_parser(
        "link", add_help=False,
        help="link executables with supplied libraries and compare complete retail files",
    )
    link_parser.add_argument("link_args", nargs=argparse.REMAINDER)
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
    if raw and raw[0] == "verify":
        from scripts.kf.verify import main as verify_main

        return verify_main(raw[1:])
    if raw and raw[0] == "link":
        from scripts.kf.executable import main as link_main

        return link_main(raw[1:])
    args = parser().parse_args(raw)
    try:
        if args.command == "init":
            retail = initialize(args.retail_dir)
            print(f"[init] retail executables verified at {retail}")
            return 0
        if args.command == "configure":
            return _configure(args)
        if args.command == "clangd":
            from scripts.kf.clangd import generate, selected_mode

            count, image = generate(image=args.image, mode=args.mode)
            print(f"[clangd] compile_commands.json: {count} C sources; "
                  f"shared-source context: {image}; type mode: {selected_mode()}")
            return 0
        if args.command == "check-types":
            from scripts.kf.check_types import check as check_types

            return check_types(images=_images(args.image), names=tuple(args.unit or ()),
                               jobs=args.jobs)
        if args.command == "casts":
            from scripts.kf.casts import run as run_cast_audit

            return run_cast_audit(args)
        if args.command == "build":
            return _build(args)
        if args.command == "try":
            from scripts.kf.trial import compare

            return compare(args.unit, args.source, args.context)
        if args.command == "hypotheses":
            from scripts.kf.hypotheses import run

            if args.jobs < 1 or args.limit < 1 or args.keep_top < 1:
                raise ValueError("--jobs, --limit, and --keep-top must be positive")
            return run(args.manifest, jobs=args.jobs, limit=args.limit,
                       keep_top=args.keep_top, output=args.output,
                       instrumented_compiler=args.instrumented_compiler, trace_dir=args.trace_dir)
        if args.command == "match":
            return _match(args)
        if args.command == "status":
            return print_status(
                _images(args.image), loose=args.loose,
                as_json=args.json, show_all=args.all,
            )
        if args.command == "check":
            return check(_images(args.image), strict=args.strict)
        return bank(
            allow_dirty=args.dirty,
            selected_units=args.unit,
            selected_functions=_bank_functions(args.function),
        )
    except (OSError, RuntimeError, ValueError) as error:
        print(f"kf {args.command}: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
