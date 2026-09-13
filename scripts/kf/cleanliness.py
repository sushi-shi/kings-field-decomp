"""kf.cleanliness - the source cleanliness scoreboard (ported from gruntz).

Counts address-derived spellings, ``extern`` declarations and cast/view syntax, measured
comment- and string-stripped over ``src/`` + ``include/`` and the curated
identity TSVs. Source metrics carry deltas against committed floors in
``config/cleanliness/cleanliness-baseline.tsv``. Data inventory counts are
informational: discovering an unknown datum is not a source regression.

Gate semantics (ported from ``gruntz verify board``): a RATCHETED metric may
never RISE above its committed floor; the gate NEVER writes the floor. A floor
is only ever lowered by the manual ``--update`` bless (the orchestrator
refreshes it at integration, like ``match_baseline.tsv``). A metric that could
not be measured keeps its committed floor rather than being blessed away.

    python3 -m scripts.kf.cleanliness            # counts + delta vs the floors
    python3 -m scripts.kf.cleanliness --gate     # exit 1 on any ratchet rise
    python3 -m scripts.kf.cleanliness --update    # bless: rewrite the floor file
    python3 -m scripts.kf.cleanliness --externs   # list source-local extern crutches
    python3 -m scripts.kf.cleanliness --data      # list unresolved data ownership
    python3 -m scripts.kf.cleanliness --data all  # include owners and their evidence

Pointer-cast and byte-array-view syntax counts are informational: exposing a
complete-object access boundary can increase them while removing an artificial
union or propagating an authentic SDK type. They remain visible review inputs;
neither count establishes whether a conversion is avoidable or well-typed.

The extern-disallow gates inspect translation units, not headers. ``GAME
extern decls`` counts declarations of curated game identities while
``source-local extern decls`` also catches SDK/libc declarations. Driving both
to 0 replaces local declaration crutches with the proper internal or vendor
header while allowing those headers to declare their interfaces normally.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import re
import sys
from pathlib import Path
from typing import Iterable

from scripts.kf.inventory import DataIdentity, load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import read_tsv


SOURCE_ROOTS = ("src", "include")
SOURCE_EXTS = {".c", ".h", ".inc"}
BASELINE = CONFIG / "cleanliness" / "cleanliness-baseline.tsv"

_BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.S)
_LINE_COMMENT = re.compile(r"//[^\n]*")
_STRING = re.compile(r'"(?:\\.|[^"\\\n])*"')
_CHAR = re.compile(r"'(?:\\.|[^'\\\n])*'")


def strip_source(text: str) -> str:
    """Blank comments and string/char literals so prose never inflates a count."""
    text = _BLOCK_COMMENT.sub(" ", text)
    text = _LINE_COMMENT.sub(" ", text)
    text = _STRING.sub(" ", text)
    text = _CHAR.sub(" ", text)
    return text


# --- address-derived spellings --------------------------------------------- #
FUNC_REF = re.compile(r"\bfunc_[0-9a-fA-F]{8}\b")
DAT_REF = re.compile(r"\bDAT_[0-9a-fA-F]{8}\b")
ADDRESS_DERIVED = re.compile(r"\b(?:func|DAT)_[0-9a-fA-F]{8}\b")

# --- cast / view crutches -------------------------------------------------- #
#: A cast whose target is a pointer type. This syntax-only count cannot distinguish
#: resource parsing, SDK representation boundaries and incorrect object models.
#: Scalar width casts are reported separately by the target-C AST census.
POINTER_CAST = re.compile(
    r"(?<![\w>)])\(\s*(?:const\s+|unsigned\s+|signed\s+|volatile\s+|struct\s+)*"
    r"(?:u8|u16|u32|s8|s16|s32|int|char|short|long|void|float|double"
    r"|u_char|u_short|u_long|Kf\w+|[A-Z]\w+)\s*\*+\s*\)")
VOID_STAR = re.compile(r"\bvoid\s*\*")
#: ``(u8 *)&x`` / ``(u8 *)(...)`` - a byte-window over a typed object.
BYTE_ARRAY_VIEW = re.compile(r"\(\s*u8\s*\*\s*\)\s*[&(]")
#: a struct/union body defined inside a .c - a per-TU fabricated view instead
#: of a shared header declaration.
CPP_LOCAL_AGGREGATE = re.compile(r"\b(?:struct|union)\s+\w+\s*\{")

# --- extern parsing -------------------------------------------------------- #
EXTERN_DECL = re.compile(r"\bextern\b\s+([^;{}]*);", re.S)
_IDENT = re.compile(r"[A-Za-z_]\w*")
_ARRAY = re.compile(r"\[[^\]]*\]")
_FUNC_PTR = re.compile(r"\(\s*\*\s*([A-Za-z_]\w*)")


def _extern_symbol(decl: str) -> str | None:
    """The declared identifier of one ``extern`` declaration body."""
    decl = decl.split("=", 1)[0]
    decl = _ARRAY.sub(" ", decl)
    pointer = _FUNC_PTR.search(decl)
    if pointer is not None:
        return pointer.group(1)
    head = decl.split("(", 1)[0]
    names = _IDENT.findall(head)
    return names[-1] if names else None


def game_symbols() -> set[str]:
    """GAME (non-SDK) symbol names: every curated function/global identity.

    A symbol is game state exactly when the project has claimed a semantic or
    address-derived identity for it. Vendored/SDK names (``memset``, Psy-Q
    APIs) have no identity row and so never enter this set.
    """
    names: set[str] = set()
    for filename in ("function_identities.tsv", "data_identities.tsv"):
        path = RETAIL_CONFIG / filename
        if not path.is_file():
            continue
        _fields, rows = read_tsv(path)
        for row in rows:
            name = row.get("name")
            if name:
                names.add(name)
    return names


def _is_game_extern(symbol: str, game: set[str]) -> bool:
    return symbol in game or ADDRESS_DERIVED.fullmatch(symbol) is not None


def source_extern_sites() -> list[tuple[str, int, str]]:
    """Every source-local ``extern`` as ``(relative path, line, symbol)``."""
    sites: list[tuple[str, int, str]] = []
    for path in _source_files():
        if path.suffix not in {".c", ".inc"}:
            continue
        raw = path.read_text(errors="ignore")
        code = strip_source(raw)
        for match in EXTERN_DECL.finditer(code):
            symbol = _extern_symbol(match.group(1))
            if symbol is None:
                continue
            line = code.count("\n", 0, match.start()) + 1
            sites.append((str(path.relative_to(REPO)), line, symbol))
    return sites


def game_extern_sites() -> list[tuple[str, int, str]]:
    """Every source-local ``extern`` of a curated GAME symbol."""
    game = game_symbols()
    return [
        site for site in source_extern_sites()
        if _is_game_extern(site[2], game)
    ]


def _source_files() -> list[Path]:
    files: list[Path] = []
    for root in SOURCE_ROOTS:
        base = REPO / root
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix in SOURCE_EXTS and path.is_file():
                files.append(path)
    return files


# --- metric table ---------------------------------------------------------- #
#: label -> (regex-or-callable, c_only). Callables take stripped code and the
#: game-symbol set; regexes are counted with findall.
def _count_game_externs(code: str, game: set[str]) -> int:
    total = 0
    for match in EXTERN_DECL.finditer(code):
        symbol = _extern_symbol(match.group(1))
        if symbol is not None and _is_game_extern(symbol, game):
            total += 1
    return total


SOURCE_METRICS: tuple[tuple[str, object, bool], ...] = (
    ("src func_ refs", FUNC_REF, False),
    ("src DAT_ refs", DAT_REF, False),
    ("GAME extern decls", _count_game_externs, True),
    ("source-local extern decls", EXTERN_DECL, True),
    ("pointer casts", POINTER_CAST, True),
    ("void* views", VOID_STAR, True),
    ("byte-array views", BYTE_ARRAY_VIEW, True),
    (".c-local aggregates", CPP_LOCAL_AGGREGATE, True),
)


def _unresolved_identity_rows(filename: str, pattern: re.Pattern[str]) -> int:
    path = RETAIL_CONFIG / filename
    if not path.is_file():
        return 0
    _fields, rows = read_tsv(path)
    return sum(1 for row in rows if pattern.fullmatch(row.get("name", "")))


_FUNC_NAME = re.compile(r"func_[0-9a-fA-F]{8}")
_DAT_NAME = re.compile(r"DAT_[0-9a-fA-F]{8}")


@dataclass(frozen=True)
class DataOwner:
    image: str
    va: int
    size: int
    storage: str
    owner: str
    evidence: str


def classify_data_ownership(
    identities: Iterable[DataIdentity], owners: Iterable[DataOwner],
) -> list[tuple[DataIdentity, DataOwner | None]]:
    """Only complete, image-local coverage establishes an identity's owner.

    A semantic name, nearby owner, or pointer into SDK code proves no ownership.
    Candidate owner annotations remain candidates until supported or proven.
    """
    identities = tuple(identities)
    owners = (
        *owners,
        *(DataOwner(row.image, row.va, row.size, row.storage, row.owner, row.evidence)
          for row in identities
          if row.owner and row.evidence and row.confidence in {"supported", "proven"}),
    )
    return [
        (row, next((owner for owner in owners
                    if owner.image == row.image and owner.storage == row.storage
                    and owner.va <= row.va
                    and row.va + row.size <= owner.va + owner.size), None))
        for row in identities
    ]


def data_ownership_rows() -> list[tuple[DataIdentity, DataOwner | None]]:
    """Use curated identities and validated source claims; no binary heuristics."""
    owners = []
    for unit in load_manifest().units:
        if unit.rodata is not None:
            va, size = unit.rodata
            owners.append(DataOwner(unit.image, va, size, "load", unit.unit,
                                    f"{unit.source}:RODATA"))
        for datum in unit.data:
            owners.append(DataOwner(unit.image, datum.va, datum.size, datum.storage,
                                    unit.unit, f"{unit.source}:DATA({datum.symbol})"))
    return classify_data_ownership(load_data_identities(RETAIL_CONFIG).values(), owners)


def count() -> list[tuple[str, int]]:
    """Every metric's live count, in report order."""
    game = game_symbols()
    totals = {label: 0 for label, _matcher, _c in SOURCE_METRICS}
    for path in _source_files():
        is_c = path.suffix in {".c", ".inc"}
        code = strip_source(path.read_text(errors="ignore"))
        for label, matcher, c_only in SOURCE_METRICS:
            if c_only and not is_c:
                continue
            if callable(matcher):
                totals[label] += matcher(code, game)
            else:
                totals[label] += len(matcher.findall(code))
    rows = [(label, totals[label]) for label, _m, _c in SOURCE_METRICS]
    rows.append(("unresolved func_ identities",
                 _unresolved_identity_rows("function_identities.tsv", _FUNC_NAME)))
    data = data_ownership_rows()
    rows.append(("unresolved data ownership", sum(owner is None for _row, owner in data)))
    rows.append(("raw DAT_ identities", sum(bool(_DAT_NAME.fullmatch(row.name))
                                            for row, _owner in data)))
    return rows


#: A spelling count cannot distinguish representation access from a false owner.
#: Do not reward hiding the same access behind an alternate union member.
INFORMATIONAL = {
    "unresolved data ownership", "raw DAT_ identities", "byte-array views", "pointer casts",
}
RATCHET = ({label for label, _m, _c in SOURCE_METRICS} - INFORMATIONAL) | {
    "unresolved func_ identities",
}


def load_baseline() -> dict[str, int]:
    out: dict[str, int] = {}
    if not BASELINE.is_file():
        return out
    for line in BASELINE.read_text().splitlines():
        if line.startswith("#") or "\t" not in line:
            continue
        label, value = line.rsplit("\t", 1)
        try:
            out[label] = int(value)
        except ValueError:
            pass
    return out


def save_baseline(rows: list[tuple[str, int]]) -> None:
    BASELINE.parent.mkdir(parents=True, exist_ok=True)
    header = (
        "# kf.cleanliness - committed floors for the source cleanliness ratchet.\n"
        "# RATCHET: down-only. Bless a lower number with `kf verify board --update`;\n"
        "# a higher one is a regression and fails `kf verify board --gate`.\n"
        "# Data inventory counts are informational and have no floors.\n"
        "# Never hand-edit to raise a floor - reconstruct/name the symbol instead.\n"
    )
    body = "".join(f"{label}\t{value}\n" for label, value in rows if label in RATCHET)
    BASELINE.write_text(header + body)


def gate(rows: list[tuple[str, int]] | None = None) -> list[str]:
    """Ratchet findings: every RATCHETED metric above its committed floor.

    FAIL-CLOSED: a ratcheted metric with no committed floor is a finding, not a
    pass - deleting the baseline must never make the gate vacuously green.
    """
    rows = rows if rows is not None else count()
    base = load_baseline()
    bad: list[str] = []
    missing = sorted(label for label, _n in rows
                     if label in RATCHET and label not in base)
    if missing:
        bad.append(
            f"no committed floor for {len(missing)} ratcheted metric(s): "
            f"{', '.join(missing[:6])} - restore "
            f"config/cleanliness/cleanliness-baseline.tsv or bless with "
            f"`kf verify board --update`")
    for label, n in rows:
        if label in RATCHET and label in base and n > base[label]:
            bad.append(f"{label}: {base[label]} -> {n} "
                       f"(+{n - base[label]}; ratcheted, never rises)")
    return bad


def report_lines(rows: list[tuple[str, int]] | None = None) -> list[str]:
    rows = rows if rows is not None else count()
    base = load_baseline()
    lines = ["cleanliness (source deltas vs committed floors; inventory counts are informational):"]
    width = max(len(label) for label, _n in rows)
    for label, n in rows:
        if label in INFORMATIONAL:
            lines.append(f"    {label:<{width}}  {n:>6}  [informational]")
            continue
        delta = n - base[label] if label in base else None
        tag = "" if delta is None else (f"  ({delta:+d})" if delta else "  (=)")
        floor = "" if label in base else "  [NO FLOOR]"
        lines.append(f"    {label:<{width}}  {n:>6}{tag}{floor}")
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="kf verify board", description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--gate", action="store_true",
                        help="exit 1 when a ratcheted metric rose above its floor")
    parser.add_argument("--update", action="store_true",
                        help="MANUAL bless: rewrite the committed floor file")
    parser.add_argument("--externs", action="store_true",
                        help="list every source-local extern crutch")
    parser.add_argument("--data", nargs="?", const="unresolved", choices=("unresolved", "all"),
                        help="list data ownership candidates, or all identities and their evidence")
    args = parser.parse_args(argv)

    if args.externs:
        sites = source_extern_sites()
        for path, line, symbol in sites:
            print(f"{path}:{line}\t{symbol}")
        print(f"# {len(sites)} source-local extern declaration(s)")
        return 0

    if args.data:
        print("image\tva\tsize\tname\towner\tevidence")
        for row, owner in data_ownership_rows():
            if args.data == "unresolved" and owner is not None:
                continue
            print(f"{row.image}\t{row.va:#010x}\t{row.size:#x}\t{row.name}\t"
                  f"{owner.owner if owner else ''}\t{owner.evidence if owner else ''}")
        return 0

    rows = count()
    if args.update:
        save_baseline(rows)
        print(f"cleanliness baseline updated ({len(rows)} metrics)")
        return 0
    for line in report_lines(rows):
        print(line)
    bad = gate(rows)
    for finding in bad:
        print(f"cleanliness RATCHET VIOLATED: {finding}", file=sys.stderr)
    if bad and args.gate:
        return 1
    if not bad:
        print("cleanliness: no ratcheted metric above its committed floor")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
