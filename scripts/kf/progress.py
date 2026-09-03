"""Current progress, regression checking, and manual high-water banking."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

from scripts.kf.delink import image_key, load_catalog
from scripts.kf.graph import IncludeScanner, toolchain_identity
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import BASELINE, BUILD, REPO, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, format_hex, parse_int, read_tsv, write_tsv


BASELINE_FIELDS = (
    "image", "va", "unit", "name", "input_sha256", "best_pct",
    "hist_pct", "banked_pct", "code_size",
)
EPSILON = 0.000001
LOOSE_EXACT = 99.995


@dataclass(frozen=True)
class Target:
    image: str
    va: int
    name: str
    code_size: int


@dataclass(frozen=True)
class Current:
    target: Target
    unit: Unit
    input_sha256: str
    pct: float | None
    compiled: bool
    scored: bool


def _target_universe() -> dict[tuple[str, int], Target]:
    catalog = load_catalog(RETAIL_CONFIG)
    return {
        (function.image, function.va): Target(
            function.image,
            function.va,
            function.symbol,
            function.body_size,
        )
        for image in IMAGE_LAYOUTS
        for function in catalog.functions[image]
        if function.scope == "decomp" and function.fragments == 1
    }


def _base_path(unit: Unit) -> Path:
    return BUILD / "objdiff" / unit.image_key / "base" / unit.object_name


def input_hash(unit: Unit, manifest: Manifest, scanner: IncludeScanner | None = None) -> str:
    scanner = scanner or IncludeScanner()
    digest = hashlib.sha256()
    for relative in [unit.source, *scanner.headers(unit.source)]:
        path = REPO / relative
        digest.update(relative.encode("utf-8"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    profile = asdict(manifest.profiles[unit.profile])
    digest.update(json.dumps(profile, sort_keys=True).encode("utf-8"))
    digest.update(b"\0")
    digest.update(toolchain_identity().encode("utf-8"))
    return digest.hexdigest()


def _load_report(image: str) -> tuple[dict | None, str | None]:
    path = BUILD / "objdiff" / image_key(image) / "report.json"
    if not path.is_file():
        return None, None
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return None, f"{path.relative_to(REPO)} is not a readable report: {error}"
    if not isinstance(document, dict) or not isinstance(document.get("units"), list):
        return None, f"{path.relative_to(REPO)} is not an objdiff report"
    return document, None


def _percent(row: dict) -> float:
    # objdiff omits a default-valued fuzzy_match_percent; that means 0%, not unknown.
    if "fuzzy_match_percent" in row:
        return float(row.get("fuzzy_match_percent") or 0.0)
    return float(row.get("match_percent") or 0.0)


def _report_scores(document: dict | None) -> tuple[dict[str, float], list[str]]:
    """Size-weighted score per report unit (used for unit-level summaries)."""
    if document is None:
        return {}, []
    scores: dict[str, float] = {}
    failures: list[str] = []
    for unit in document.get("units", []):
        name = str(unit.get("name", "")).split("/")[-1]
        measures = unit.get("measures", {})
        totals = [int(measures.get(key) or 0) for key in (
            "total_code", "total_functions", "total_data"
        )]
        if not any(totals):
            failures.append(f"report unit {name!r} has zero code/function/data totals")
            continue
        functions = unit.get("functions", [])
        if functions:
            weighted = sum(
                int(function.get("size") or 0) * _percent(function)
                for function in functions
            )
            size = sum(int(function.get("size") or 0) for function in functions)
            scores[name] = weighted / size if size else _percent(measures)
        else:
            scores[name] = _percent(measures)
    return scores, failures


def _report_function_scores(document: dict | None) -> dict[str, dict[str, float]]:
    """Per-function fuzzy score keyed by report unit and function symbol."""
    if document is None:
        return {}
    scores: dict[str, dict[str, float]] = {}
    for unit in document.get("units", []):
        name = str(unit.get("name", "")).split("/")[-1]
        scores[name] = {
            str(function.get("name", "")): _percent(function)
            for function in unit.get("functions", [])
        }
    return scores


def _report_is_stale(image: str, manifest: Manifest) -> bool:
    key = image_key(image)
    report = BUILD / "objdiff" / key / "report.json"
    if not report.is_file():
        return False
    inputs = [
        BUILD / "objdiff" / key / "objdiff.json",
        BUILD / "delink" / key / ".delink.stamp",
        *[_base_path(unit) for unit in manifest.units if unit.image == image],
    ]
    newest = max((path.stat().st_mtime for path in inputs if path.exists()), default=0.0)
    return newest - report.stat().st_mtime > 1.0


def current_state(
    images: Iterable[str] = IMAGE_LAYOUTS,
) -> tuple[Manifest, dict[tuple[str, int], Target], list[Current], list[str]]:
    selected = tuple(images)
    manifest = load_manifest()
    universe = _target_universe()
    scanner = IncludeScanner()
    report_scores: dict[str, dict[str, dict[str, float]]] = {}
    failures: list[str] = []
    for image in selected:
        report, error = _load_report(image)
        if error:
            failures.append(error)
        _scores, report_failures = _report_scores(report)
        report_scores[image] = _report_function_scores(report)
        failures.extend(f"{image}: {failure}" for failure in report_failures)
        image_units = [unit for unit in manifest.units if unit.image == image]
        if image_units and any(_base_path(unit).is_file() for unit in image_units):
            if report is None and error is None:
                failures.append(f"{image}: no report; run `kf build --image {image_key(image)}`")
            elif _report_is_stale(image, manifest):
                failures.append(f"{image}: report is stale; run `kf build --image {image_key(image)}`")

    rows: list[Current] = []
    for unit in manifest.units:
        if unit.image not in selected:
            continue
        base = _base_path(unit)
        digest = input_hash(unit, manifest, scanner)
        unit_scores = report_scores.get(unit.image, {}).get(unit.unit)
        for function in unit.functions:
            target = universe[(unit.image, function.va)]
            pct = None if unit_scores is None else unit_scores.get(function.symbol)
            if unit_scores is not None and pct is None:
                failures.append(
                    f"{unit.image}: report unit {unit.unit!r} lacks function {function.symbol}"
                )
            rows.append(Current(
                target,
                unit,
                digest,
                pct,
                base.is_file(),
                pct is not None,
            ))
    return manifest, universe, rows, failures


def _summary(
    images: Iterable[str], universe: dict[tuple[str, int], Target], rows: list[Current],
    *, loose: bool,
) -> dict[str, dict[str, int | float]]:
    threshold = LOOSE_EXACT if loose else 100.0
    result: dict[str, dict[str, int | float]] = {}
    for image in images:
        targets = [target for target in universe.values() if target.image == image]
        current = [row for row in rows if row.target.image == image]
        scored = [row for row in current if row.pct is not None]
        exact = [row for row in scored if row.pct is not None and row.pct >= threshold]
        eligible_code = sum(target.code_size for target in targets)
        scored_code = sum(row.target.code_size for row in scored)
        exact_code = sum(row.target.code_size for row in exact)
        weighted = sum((row.pct or 0.0) * row.target.code_size for row in scored)
        result[image] = {
            "eligible_functions": len(targets),
            "manifested_functions": len(current),
            "compiled_functions": sum(row.compiled for row in current),
            "scored_functions": len(scored),
            "exact_functions": len(exact),
            "eligible_code": eligible_code,
            "scored_code": scored_code,
            "exact_code": exact_code,
            "coverage_percent": 100.0 * exact_code / eligible_code if eligible_code else 0.0,
            "fuzzy_started_percent": weighted / scored_code if scored_code else 0.0,
            "fuzzy_overall_percent": weighted / eligible_code if eligible_code else 0.0,
        }
    return result


def load_baseline() -> dict[tuple[str, int], dict[str, str]]:
    if not BASELINE.is_file():
        return {}
    fields, rows = read_tsv(BASELINE)
    if fields != BASELINE_FIELDS:
        raise ValueError(f"{BASELINE}: unexpected fields {fields!r}")
    return {(row["image"], parse_int(row["va"])): row for row in rows}


def classifications(
    rows: list[Current], baseline: dict[tuple[str, int], dict[str, str]],
) -> dict[str, list[tuple[Current | None, dict[str, str] | None]]]:
    buckets = {name: [] for name in (
        "REGRESS", "LOST", "IMPROVE", "CHANGED", "NEW", "RENAMED"
    )}
    current = {(row.target.image, row.target.va): row for row in rows if row.scored}
    for identity, old in baseline.items():
        row = current.get(identity)
        if row is None:
            buckets["LOST"].append((None, old))
            continue
        if row.unit.unit != old["unit"] or row.target.name != old["name"]:
            buckets["RENAMED"].append((row, old))
        pct = row.pct or 0.0
        if row.input_sha256 != old["input_sha256"]:
            buckets["CHANGED"].append((row, old))
        elif pct < float(old["best_pct"]) - EPSILON:
            buckets["REGRESS"].append((row, old))
        elif pct > float(old["best_pct"]) + EPSILON:
            buckets["IMPROVE"].append((row, old))
    for identity, row in current.items():
        if identity not in baseline:
            buckets["NEW"].append((row, None))
    return buckets


def snapshot(
    images: Iterable[str] = IMAGE_LAYOUTS, *, loose: bool = False,
) -> tuple[dict, list[str]]:
    selected = tuple(images)
    _manifest, universe, rows, failures = current_state(selected)
    baseline = {
        identity: row for identity, row in load_baseline().items()
        if identity[0] in selected
    }
    buckets = classifications(rows, baseline)
    summaries = _summary(selected, universe, rows, loose=loose)
    totals: dict[str, int | float] = {}
    integer_fields = (
        "eligible_functions", "manifested_functions", "compiled_functions",
        "scored_functions", "exact_functions", "eligible_code", "scored_code",
        "exact_code",
    )
    for field in integer_fields:
        totals[field] = sum(int(summary[field]) for summary in summaries.values())
    eligible_code = int(totals["eligible_code"])
    scored_code = int(totals["scored_code"])
    weighted = sum(
        float(summary["fuzzy_started_percent"]) * int(summary["scored_code"])
        for summary in summaries.values()
    )
    totals["coverage_percent"] = (
        100.0 * int(totals["exact_code"]) / eligible_code if eligible_code else 0.0
    )
    totals["fuzzy_started_percent"] = weighted / scored_code if scored_code else 0.0
    totals["fuzzy_overall_percent"] = weighted / eligible_code if eligible_code else 0.0
    document = {
        "exact_threshold": LOOSE_EXACT if loose else 100.0,
        "images": {image_key(image): summaries[image] for image in selected},
        "total": totals,
        "changes": {name.lower(): len(values) for name, values in buckets.items()},
        "failures": failures,
    }
    return document, failures


def print_status(
    images: Iterable[str] = IMAGE_LAYOUTS, *, loose: bool = False,
    as_json: bool = False, show_all: bool = False,
) -> int:
    selected = tuple(images)
    document, failures = snapshot(selected, loose=loose)
    if as_json:
        print(json.dumps(document, indent=2, sort_keys=True))
        return 0
    threshold_note = " (loose)" if loose else ""
    print(f"exact threshold: {document['exact_threshold']:.3f}%{threshold_note}")
    print(
        f"{'image':<6} {'eligible':>8} {'started':>8} {'built':>7} "
        f"{'scored':>7} {'exact':>7} {'coverage':>10} {'fuzzy/built':>12}"
    )
    for key, summary in document["images"].items():
        print(
            f"{key:<6} {summary['eligible_functions']:>8} "
            f"{summary['manifested_functions']:>8} {summary['compiled_functions']:>7} "
            f"{summary['scored_functions']:>7} {summary['exact_functions']:>7} "
            f"{summary['coverage_percent']:>9.3f}% "
            f"{summary['fuzzy_started_percent']:>11.3f}%"
        )
    total = document["total"]
    print(
        f"{'total':<6} {total['eligible_functions']:>8} "
        f"{total['manifested_functions']:>8} {total['compiled_functions']:>7} "
        f"{total['scored_functions']:>7} {total['exact_functions']:>7} "
        f"{total['coverage_percent']:>9.3f}% "
        f"{total['fuzzy_started_percent']:>11.3f}%"
    )
    changes = document["changes"]
    nonzero = [f"{name}={count}" for name, count in changes.items() if count]
    print("ledger: " + (", ".join(nonzero) if nonzero else "no changes"))
    for failure in failures:
        print(f"WARNING: {failure}", file=sys.stderr)
    if show_all:
        _manifest, _universe, rows, _failures = current_state(selected)
        for row in sorted(rows, key=lambda item: (item.target.image, item.target.va)):
            pct = "n/a" if row.pct is None else f"{row.pct:.6f}%"
            print(
                f"{image_key(row.target.image)}:{row.target.va:#010x} "
                f"{row.unit.unit} {pct}"
            )
    return 0


def _report_cleanliness() -> None:
    """Print the cleanliness ratchet status - informational, never fatal.

    The board is a ratchet enforced by `kf verify board --gate`; surfacing its
    delta here lets a matcher see a crutch it just added without making the
    shared build/check a hard failure (the orchestrator gates at integration).
    """
    try:
        from scripts.kf.cleanliness import gate
        findings = gate()
    except Exception:  # noqa: BLE001 - a reporting aid must never break check
        return
    if findings:
        print(f"cleanliness: {len(findings)} ratchet(s) above floor "
              f"(run `kf verify board`); informational, not gating build")
    else:
        print("cleanliness: no ratcheted crutch metric above its committed floor")


def check(
    images: Iterable[str] = IMAGE_LAYOUTS, *, strict: bool = False,
) -> int:
    selected = tuple(images)
    _manifest, _universe, rows, failures = current_state(selected)
    baseline = {
        identity: row for identity, row in load_baseline().items()
        if identity[0] in selected
    }
    buckets = classifications(rows, baseline)
    print_status(selected)
    bad = bool(failures or buckets["REGRESS"] or buckets["LOST"])
    strict_changed = [
        (row, old) for row, old in buckets["CHANGED"]
        if row is not None and old is not None
        and (row.pct or 0.0) < float(old["hist_pct"]) - EPSILON
    ]
    if strict:
        bad = bad or bool(strict_changed)
    from scripts.kf.readme import refresh as refresh_readme

    if refresh_readme():
        print("README match-status block refreshed")
    _report_cleanliness()
    if bad:
        reasons = []
        if failures:
            reasons.append(f"{len(failures)} report failure(s)")
        if buckets["REGRESS"]:
            reasons.append(f"{len(buckets['REGRESS'])} unchanged-input regression(s)")
        if buckets["LOST"]:
            reasons.append(f"{len(buckets['LOST'])} lost banked function(s)")
        if strict_changed:
            reasons.append(
                f"{len(strict_changed)} changed-input row(s) below historical best [--strict]"
            )
        print("check FAILED: " + ", ".join(reasons), file=sys.stderr)
        return 1
    print("check OK: no unchanged-input regressions or lost banked functions")
    return 0


def _dirty_inputs() -> list[str]:
    paths = ("src", "include", "config", "scripts/kf", "flake.nix", "flake.lock")
    commands = (
        ["git", "diff", "--name-only", "--", *paths],
        ["git", "ls-files", "--others", "--exclude-standard", "--", *paths],
    )
    dirty: set[str] = set()
    for command in commands:
        result = subprocess.run(command, cwd=REPO, capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError(result.stderr.strip() or "git status query failed")
        dirty.update(line for line in result.stdout.splitlines() if line)
    dirty.discard(str(BASELINE.relative_to(REPO)))
    return sorted(dirty)


def _bank_record(row: Current, previous: dict[str, str] | None) -> dict[str, str | int]:
    pct = row.pct or 0.0
    same_input = previous is not None and previous["input_sha256"] == row.input_sha256
    best = max(float(previous["best_pct"]), pct) if same_input else pct
    hist = max(float(previous["hist_pct"]), pct) if previous else pct
    return {
        "image": row.target.image,
        "va": format_hex(row.target.va),
        "unit": row.unit.unit,
        "name": row.target.name,
        "input_sha256": row.input_sha256,
        "best_pct": f"{best:.9f}",
        "hist_pct": f"{hist:.9f}",
        "banked_pct": f"{pct:.9f}",
        "code_size": row.target.code_size,
    }


def _bank_rows(
    rows: list[Current],
    old: dict[tuple[str, int], dict[str, str]],
    selected_units: Iterable[str] | None = None,
) -> list[dict[str, str | int]]:
    requested = set(selected_units or ())
    if not requested:
        output = {
            (row.target.image, row.target.va): _bank_record(row, old.get(
                (row.target.image, row.target.va)
            ))
            for row in rows
            if row.scored
        }
    else:
        present = {row.unit.unit for row in rows}
        missing = sorted(requested - present)
        if missing:
            raise ValueError("unknown bank unit(s): " + ", ".join(missing))
        selected = [row for row in rows if row.unit.unit in requested]
        unscored = [row for row in selected if not row.scored or row.pct is None]
        if unscored:
            raise ValueError(
                "selected bank unit has unscored function(s): "
                + ", ".join(row.target.name for row in unscored)
            )
        nonexact = [row for row in selected if row.pct != 100.0]
        if nonexact:
            raise ValueError(
                "selected bank unit is not exact: "
                + ", ".join(
                    f"{row.target.name}={row.pct:.9f}%" for row in nonexact
                )
            )
        output = dict(old)
        for row in selected:
            identity = row.target.image, row.target.va
            output[identity] = _bank_record(row, old.get(identity))
    image_order = {image: position for position, image in enumerate(IMAGE_LAYOUTS)}
    return sorted(
        output.values(),
        key=lambda row: (image_order[str(row["image"])], parse_int(str(row["va"]))),
    )


def bank(
    *, allow_dirty: bool = False, selected_units: Iterable[str] | None = None,
) -> int:
    dirty = _dirty_inputs()
    if dirty and not allow_dirty:
        shown = "\n".join(f"  {path}" for path in dirty[:12])
        raise ValueError(
            "refusing to bank from unstaged/untracked build inputs:\n"
            f"{shown}\nstage or commit them first (`kf bank --dirty` overrides)"
        )
    _manifest, _universe, rows, failures = current_state()
    if failures:
        raise ValueError("refusing to bank: " + "; ".join(failures))
    old = load_baseline()
    selected = tuple(dict.fromkeys(selected_units or ()))
    output = _bank_rows(rows, old, selected)
    write_tsv(
        BASELINE,
        BASELINE_FIELDS,
        output,
        (
            "Manually updated only by `kf bank`; (image, va) is the stable identity.",
            "Scores retain full precision; --loose affects display classification only.",
        ),
    )
    from scripts.kf.readme import refresh as refresh_readme

    refreshed = refresh_readme()
    if selected:
        count = sum(row.unit.unit in selected for row in rows)
        print(
            f"banked {count} selected exact function(s) in "
            f"{BASELINE.relative_to(REPO)}"
        )
    else:
        print(f"banked {len(output)} scored function(s) in {BASELINE.relative_to(REPO)}")
    print(f"README match-status block {'refreshed' if refreshed else 'unchanged'}")
    return 0
