"""kf.cleanliness - the source cleanliness scoreboard (ported from gruntz).

Counts the address-derived spellings and the ``extern`` / cast / view crutches
that a clean King's Field reconstruction should trend to 0, measured
comment- and string-stripped over ``src/`` + ``include/`` and the curated
identity TSVs. Each count carries a delta against the committed floor in
``config/cleanliness/cleanliness-baseline.tsv`` - ``down = good``.

Gate semantics (ported from ``gruntz verify board``): a RATCHETED metric may
never RISE above its committed floor; the gate NEVER writes the floor. A floor
is only ever lowered by the manual ``--update`` bless (the orchestrator
refreshes it at integration, like ``match_baseline.tsv``). A metric that could
not be measured keeps its committed floor rather than being blessed away.

    python3 -m scripts.kf.cleanliness            # counts + delta vs the floors
    python3 -m scripts.kf.cleanliness --gate     # exit 1 on any ratchet rise
    python3 -m scripts.kf.cleanliness --update    # bless: rewrite the floor file
    python3 -m scripts.kf.cleanliness --externs   # list source-local extern crutches

The extern-disallow gates inspect translation units, not headers. ``GAME
extern decls`` counts declarations of curated game identities while
``source-local extern decls`` also catches SDK/libc declarations. Driving both
to 0 replaces local declaration crutches with the proper internal or vendor
header while allowing those headers to declare their interfaces normally.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import read_tsv


SOURCE_ROOTS = ("src", "include")
SOURCE_EXTS = {".c", ".h"}
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
#: a cast whose target is a pointer type - the byte-array/struct aliasing view
#: most tied to a wrong data model. Scalar width casts are intentionally not
#: counted here (they are ordinary width conversions, not view crutches).
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
        if path.suffix != ".c":
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


def count() -> list[tuple[str, int]]:
    """Every metric's live count, in report order."""
    game = game_symbols()
    totals = {label: 0 for label, _matcher, _c in SOURCE_METRICS}
    for path in _source_files():
        is_c = path.suffix == ".c"
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
    rows.append(("unresolved DAT_ identities",
                 _unresolved_identity_rows("data_identities.tsv", _DAT_NAME)))
    return rows


#: every metric is a ratchet - none may rise above its committed floor.
RATCHET = {label for label, _m, _c in SOURCE_METRICS} | {
    "unresolved func_ identities", "unresolved DAT_ identities",
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
        "# Never hand-edit to raise a floor - reconstruct/name the symbol instead.\n"
    )
    body = "".join(f"{label}\t{value}\n" for label, value in rows)
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
    lines = ["cleanliness (down = good; delta vs committed floor):"]
    width = max(len(label) for label, _n in rows)
    for label, n in rows:
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
    args = parser.parse_args(argv)

    if args.externs:
        sites = source_extern_sites()
        for path, line, symbol in sites:
            print(f"{path}:{line}\t{symbol}")
        print(f"# {len(sites)} source-local extern declaration(s)")
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
