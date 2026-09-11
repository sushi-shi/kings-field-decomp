"""Compile and strict-score a bounded matrix of source hypotheses for one function.

The JSON manifest describes exact, reviewable source substitutions.  Axes form a
Cartesian product, so five binary axes produce 32 independently compiled states.
Each state uses a disposable sibling source and an isolated object directory;
the configured source is never rewritten.
"""

from __future__ import annotations

import concurrent.futures
import hashlib
import itertools
import json
import os
import shutil
import subprocess
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path

from scripts.kf import compile as compiler
from scripts.kf import gcc_trace
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.paths import BUILD, REPO


@dataclass(frozen=True)
class Edit:
    start: int
    end: int
    replacement: bytes


@dataclass(frozen=True)
class Option:
    name: str
    edits: tuple[Edit, ...]


@dataclass(frozen=True)
class Axis:
    name: str
    options: tuple[Option, ...]


@dataclass(frozen=True)
class Variant:
    index: int
    name: str
    labels: dict[str, str]
    source: bytes


@dataclass(frozen=True)
class TraceOptions:
    compiler: Path
    features: list[dict]


def _unique_span(original: bytes, value: object, context: str) -> tuple[int, int]:
    if not isinstance(value, str) or not value:
        raise ValueError(f"{context}: find must be a non-empty string")
    needle = value.encode()
    count = original.count(needle)
    if count != 1:
        raise ValueError(f"{context}: exact find span occurs {count} times, expected 1")
    start = original.index(needle)
    return start, start + len(needle)


def _parse_edits(raw_edits: object, original: bytes, context: str) -> tuple[Edit, ...]:
    if not isinstance(raw_edits, list):
        raise ValueError(f"{context}: edits must be a list")
    edits = []
    for raw in raw_edits:
        if not isinstance(raw, dict):
            raise ValueError(f"{context}: every edit must be an object")
        start, end = _unique_span(original, raw.get("find"), context)
        replacement = raw.get("replace")
        if not isinstance(replacement, str):
            raise ValueError(f"{context}: replace must be a string")
        edits.append(Edit(start, end, replacement.encode()))
    edits.sort(key=lambda edit: (edit.start, edit.end))
    for left, right in zip(edits, edits[1:]):
        if left.end > right.start:
            raise ValueError(f"{context}: edits overlap")
    return tuple(edits)


def parse_manifest(path: Path, *, root: Path = REPO):
    payload = json.loads(path.read_text(encoding="utf-8"))
    if payload.get("schema") != 1:
        raise ValueError("manifest schema must be 1")
    unit_name = payload.get("unit")
    function_name = payload.get("function")
    if not isinstance(unit_name, str) or not unit_name:
        raise ValueError("manifest unit must be a non-empty string")
    if not isinstance(function_name, str) or not function_name:
        raise ValueError("manifest function must be a non-empty string")
    unit = load_manifest().by_name().get(unit_name)
    if unit is None:
        raise ValueError(f"unknown unit {unit_name!r}")
    if function_name not in {function.symbol for function in unit.functions}:
        raise ValueError(f"{function_name!r} is not claimed by {unit_name}")
    source = root / unit.source
    original = source.read_bytes()
    raw_axes = payload.get("axes")
    if not isinstance(raw_axes, list) or not raw_axes:
        raise ValueError("manifest axes must be a non-empty list")
    axes = []
    axis_names = set()
    all_axis_edits: list[tuple[str, Edit]] = []
    for raw_axis in raw_axes:
        if not isinstance(raw_axis, dict):
            raise ValueError("every axis must be an object")
        name = raw_axis.get("name")
        if not isinstance(name, str) or not name or name in axis_names:
            raise ValueError("axis names must be unique non-empty strings")
        axis_names.add(name)
        start, end = _unique_span(original, raw_axis.get("find"), f"axis {name}")
        find = original[start:end]
        raw_options = raw_axis.get("options")
        if not isinstance(raw_options, list) or not raw_options:
            raise ValueError(f"axis {name}: options must be a non-empty list")
        options = []
        option_names = set()
        for raw_option in raw_options:
            if not isinstance(raw_option, dict):
                raise ValueError(f"axis {name}: every option must be an object")
            option_name = raw_option.get("name")
            if (not isinstance(option_name, str) or not option_name
                    or option_name in option_names):
                raise ValueError(f"axis {name}: option names must be unique")
            option_names.add(option_name)
            replacement = raw_option.get("replace")
            if replacement is None:
                replacement_bytes = find
            elif isinstance(replacement, str):
                replacement_bytes = replacement.encode()
            else:
                raise ValueError(f"axis {name}/{option_name}: replace must be a string")
            edits = (Edit(start, end, replacement_bytes), *_parse_edits(
                raw_option.get("extra_edits", []), original,
                f"axis {name}/{option_name}",
            ))
            ordered = sorted(edits, key=lambda edit: (edit.start, edit.end))
            for left, right in zip(ordered, ordered[1:]):
                if left.end > right.start:
                    raise ValueError(f"axis {name}/{option_name}: edits overlap")
            options.append(Option(option_name, tuple(edits)))
            all_axis_edits.extend((name, edit) for edit in edits)
        axes.append(Axis(name, tuple(options)))
    for index, (left_name, left) in enumerate(all_axis_edits):
        for right_name, right in all_axis_edits[index + 1:]:
            if left_name != right_name and left.start < right.end and right.start < left.end:
                raise ValueError(f"axes {left_name} and {right_name} overlap")
    return payload, unit, function_name, source, original, tuple(axes)


def render(original: bytes, edits: tuple[Edit, ...]) -> bytes:
    result = original
    for edit in sorted(edits, key=lambda item: item.start, reverse=True):
        result = result[:edit.start] + edit.replacement + result[edit.end:]
    return result


def variants(original: bytes, axes: tuple[Axis, ...]) -> list[Variant]:
    result = []
    seen = {}
    for choices in itertools.product(*(axis.options for axis in axes)):
        labels = {axis.name: option.name for axis, option in zip(axes, choices)}
        candidate = render(original, tuple(edit for option in choices for edit in option.edits))
        digest = hashlib.sha256(candidate).hexdigest()
        if digest in seen:
            continue
        seen[digest] = labels
        name = ",".join(f"{key}={value}" for key, value in labels.items())
        result.append(Variant(len(result), name, labels, candidate))
    return result


def _compile(
    unit: Unit, source: Path, source_dir: Path, output: Path,
    *, trace: TraceOptions | None = None, symbol: str | None = None,
) -> None:
    profile = load_manifest().profiles[unit.profile]
    # Preserve quoted-include lookup from the configured source's directory
    # while compiling the disposable source outside the worktree's src tree.
    includes = [source_dir, REPO / "include", REPO / "vendor/include"]
    if os.environ.get("PSYQ_INCLUDE"):
        includes.append(Path(os.environ["PSYQ_INCLUDE"]))
    compiler.compile_source(
        source, unit.image, output, BUILD / "delink", profile.optimization,
        profile.small_data, profile.aspsx_version, tuple(includes), profile.cc1_flags,
        profile.compiler, defines=unit.defines,
        cc1_override=trace.compiler if trace else None,
        trace_path=output.with_suffix(".trace.jsonl") if trace else None,
        trace_function=symbol if trace else None,
    )


def strict_score(target: Path, candidate: Path, symbol: str) -> tuple[float, int | None, str]:
    executable = shutil.which("objdiff-cli")
    if executable is None:
        raise RuntimeError("objdiff-cli not found; enter nix develop")
    process = subprocess.run(
        [executable, "diff", "-1", str(target), "-2", str(candidate), symbol,
         "-o", "-", "--format", "json"],
        capture_output=True, text=True, check=False,
    )
    if process.returncode:
        return 0.0, None, (process.stderr or process.stdout).strip()
    try:
        payload = json.loads(process.stdout)
    except json.JSONDecodeError:
        return 0.0, None, "objdiff returned invalid JSON"
    rows = [row for row in payload.get("right", {}).get("symbols", [])
            if row.get("name") == symbol]
    if len(rows) != 1 or rows[0].get("match_percent") is None:
        return 0.0, None, "function missing or non-unique in objdiff output"
    row = rows[0]
    size = int(str(row["size"]), 0) if row.get("size") is not None else None
    return float(row["match_percent"]), size, ""


def _run_variant(
    variant: Variant, unit: Unit, symbol: str, source: Path, target: Path, scratch: Path,
    trace: TraceOptions | None = None,
) -> tuple[dict, bytes | None]:
    directory = scratch / f"{variant.index:04d}"
    directory.mkdir()
    candidate_source = directory / source.name
    output = directory / unit.object_name
    try:
        candidate_source.write_bytes(variant.source)
        _compile(unit, candidate_source, source.parent, output, trace=trace, symbol=symbol)
        if trace:
            normal = directory / "normal" / unit.object_name
            _compile(unit, candidate_source, source.parent, normal)
            if normal.read_bytes() != output.read_bytes():
                raise RuntimeError("instrumented compiler changes complete object bytes")
        score, size, error = strict_score(target, output, symbol)
        row = {
            "index": variant.index,
            "name": variant.name,
            "labels": variant.labels,
            "source_sha256": hashlib.sha256(variant.source).hexdigest(),
            "score": score,
            "exact": score == 100.0,
            "candidate_size": size,
            "error": error,
        }
        if trace:
            trace_path = output.with_suffix(".trace.jsonl")
            records = gcc_trace.read_trace(trace_path, symbol, source_sha256=row["source_sha256"])
            row["features"] = gcc_trace.summarize(records, trace.features, output, target)
            row["frontier_key"] = gcc_trace.frontier_key(row["features"])
            row["compiler_source_sha256"] = records.compiler_source_sha256
            metadata = json.loads(output.with_suffix(output.suffix + ".json").read_text())
            row["preprocessed_sha256"] = metadata["preprocessed_sha256"]
            row["trace_sha256"] = hashlib.sha256(trace_path.read_bytes()).hexdigest()
            row["trace_retained"] = None
        return row, variant.source
    except (OSError, RuntimeError, ValueError) as error:
        return {
            "index": variant.index, "name": variant.name, "labels": variant.labels,
            "source_sha256": hashlib.sha256(variant.source).hexdigest(),
            "score": 0.0, "exact": False, "candidate_size": None,
            "error": str(error),
        }, None
    finally:
        candidate_source.unlink(missing_ok=True)


def trace_frontier(rows: list[dict]) -> set[int]:
    """Keep the strict-best representative of each explicitly selected state."""
    retained = {row["index"] for row in rows if row.get("exact") and not row["error"]}
    seen = set()
    for row in sorted(rows, key=lambda item: (-item["score"], item["index"])):
        if row["error"] or "frontier_key" not in row:
            continue
        key = row["frontier_key"]
        if key not in seen:
            retained.add(row["index"])
            seen.add(key)
    return retained


def run(
    path: Path, *, jobs: int, limit: int, keep_top: int, output: Path | None,
    instrumented_compiler: Path | None = None, trace_dir: Path | None = None,
) -> int:
    payload, unit, symbol, source, original, axes = parse_manifest(path)
    trace = None
    if trace_dir is not None and instrumented_compiler is None:
        raise ValueError("--trace-dir requires --instrumented-compiler")
    if instrumented_compiler is not None:
        if load_manifest().profiles[unit.profile].compiler != "gcc257-native":
            raise ValueError("instrumented compiler requires the unit's GCC 2.5.7 profile")
        executable = shutil.which(str(instrumented_compiler))
        if executable is None:
            raise ValueError(f"instrumented compiler is not executable: {instrumented_compiler}")
        trace = TraceOptions(Path(executable).resolve(),
                             gcc_trace.validate_features(payload.get("trace_features")))
    candidates = variants(original, axes)
    combinations = 1
    for axis in axes:
        combinations *= len(axis.options)
    if combinations > limit:
        raise ValueError(f"manifest expands to {combinations} states, above --limit {limit}")
    target = BUILD / "delink" / unit.image_key / "modules" / unit.object_name
    if not target.is_file():
        raise ValueError(f"{target}: target is missing; run `kf analyze` first")
    target_digest = hashlib.sha256(target.read_bytes()).hexdigest()
    stamp = time.strftime("%Y%m%d-%H%M%S")
    output = output or BUILD / "hypotheses" / f"{stamp}-{unit.unit.replace('.', '-')}-{symbol}"
    if trace:
        trace_dir = (trace_dir or output / "traces").resolve()
        if not trace_dir.is_relative_to(BUILD.resolve()):
            raise ValueError("compiler traces must be stored under build/")
        if trace_dir.exists():
            raise ValueError(f"trace directory already exists: {trace_dir}")
    output.mkdir(parents=True, exist_ok=False)
    if trace:
        trace_dir.mkdir(parents=True, exist_ok=False)
    (output / "manifest.json").write_text(json.dumps(payload, indent=2) + "\n")
    print(f"[hypotheses] {len(candidates)} unique/{combinations} states; "
          f"{jobs} compile job(s); source remains {source.relative_to(REPO)}", flush=True)
    with tempfile.TemporaryDirectory(prefix="kf-hypotheses-", dir=output) as temporary:
        scratch = Path(temporary)
        baseline = None
        if trace:
            baseline, _ = _run_variant(
                Variant(-1, "baseline", {}, original), unit, symbol, source, target, scratch, trace,
            )
            if baseline["error"]:
                (output / "baseline-error.json").write_text(json.dumps(baseline, indent=2) + "\n")
                raise RuntimeError(f"baseline trace failed: {baseline['error']}")
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            completed = list(pool.map(
                lambda candidate: _run_variant(
                    candidate, unit, symbol, source, target, scratch, trace,
                ),
                candidates,
            ))
        if trace:
            traced_rows = [baseline, *(item[0] for item in completed)]
            fingerprints = {row["compiler_source_sha256"] for row in traced_rows
                            if "compiler_source_sha256" in row}
            if len(fingerprints) != 1:
                raise RuntimeError("compiler source changed during hypothesis batch")
            retained = trace_frontier(traced_rows) | {-1}
            sources_by_index = {-1: original, **{item[0]["index"]: item[1] for item in completed}}
            for row in traced_rows:
                if row["index"] not in retained or row["error"]:
                    continue
                directory = scratch / f"{row['index']:04d}"
                destination = trace_dir / ("baseline" if row["index"] == -1 else f"{row['index']:04d}")
                destination.mkdir()
                shutil.copyfile(directory / unit.object_name, destination / unit.object_name)
                shutil.copyfile((directory / unit.object_name).with_suffix(".trace.jsonl"),
                                destination / "trace.jsonl")
                (destination / source.name).write_bytes(sources_by_index[row["index"]])
                row["trace_retained"] = str((destination / "trace.jsonl").relative_to(REPO))
                (destination / "summary.json").write_text(json.dumps(row, indent=2) + "\n")
            shutil.copyfile(target, trace_dir / "baseline" / "target.o")
    if source.read_bytes() != original:
        raise RuntimeError(f"configured source changed during batch: {source}")
    if hashlib.sha256(target.read_bytes()).hexdigest() != target_digest:
        raise RuntimeError(f"strict target changed during batch: {target}")
    rows = [item[0] for item in completed]
    sources = {item[0]["index"]: item[1] for item in completed if item[1] is not None}
    ranked = sorted(rows, key=lambda row: (-row["score"], row["index"]))
    for rank, row in enumerate(ranked[:keep_top], 1):
        candidate = sources.get(row["index"])
        if candidate is not None:
            (output / f"rank-{rank:02d}-{row['index']:04d}{source.suffix}").write_bytes(candidate)
    report = {
        "schema": 1, "unit": unit.unit, "function": symbol,
        "target_sha256": target_digest,
        "source": str(source.relative_to(REPO)), "states": combinations,
        "unique_states": len(candidates), "results": ranked,
        **({"baseline": baseline, "trace_features": trace.features,
            "retained_traces": len(retained)} if trace else {}),
    }
    (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    for row in ranked[:keep_top]:
        suffix = f" error={row['error']}" if row["error"] else ""
        print(f"{row['score']:12.9f}%  [{row['index']:04d}] {row['name']}{suffix}")
    exact = [row for row in ranked if row["exact"]]
    try:
        output_label = output.relative_to(REPO)
    except ValueError:
        output_label = output
    print(f"[hypotheses] {len(exact)} exact; results: {output_label}")
    if trace:
        print(f"[hypotheses] retained {len(retained)} traces, including baseline; "
              "selected internal features are in results.json")
    return 0 if exact else 1


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("-j", "--jobs", type=int, default=min(8, os.cpu_count() or 1))
    parser.add_argument("--limit", type=int, default=256)
    parser.add_argument("--keep-top", type=int, default=8)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--instrumented-compiler", type=Path)
    parser.add_argument("--trace-dir", type=Path)
    args = parser.parse_args(argv)
    if args.jobs < 1 or args.limit < 1 or args.keep_top < 1:
        parser.error("--jobs, --limit, and --keep-top must be positive")
    return run(args.manifest, jobs=args.jobs, limit=args.limit,
               keep_top=args.keep_top, output=args.output,
               instrumented_compiler=args.instrumented_compiler, trace_dir=args.trace_dir)


if __name__ == "__main__":
    raise SystemExit(main())
