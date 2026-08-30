"""Merge strong function-audit candidates into a reviewable retail proposal.

The default output is below ``build``. This tool never overwrites the admitted
``config/retail`` censuses; applying its diff remains a deliberate review step.
"""

from __future__ import annotations

import argparse
import shutil
from collections import Counter, defaultdict
from pathlib import Path

from scripts.kf.retail import (
    DATA_FIELDS,
    FUNCTION_FIELDS,
    IMAGE_LAYOUTS,
    IMAGE_ORDER,
    format_hex,
    format_size,
    parse_int,
    read_tsv,
    write_tsv,
)


ADMITTED_CONFIDENCE = {"strong", "strong-leaf"}


def comments(path: Path) -> list[str]:
    result = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.startswith("#"):
            break
        result.append(line.removeprefix("#").lstrip())
    return result


def subtract_interval(
    start: int,
    end: int,
    cuts: list[tuple[int, int]],
) -> list[tuple[int, int]]:
    fragments = []
    cursor = start
    for cut_start, cut_end in cuts:
        if cut_end <= cursor or end <= cut_start:
            continue
        if cursor < cut_start:
            fragments.append((cursor, min(end, cut_start)))
        cursor = max(cursor, cut_end)
        if cursor >= end:
            break
    if cursor < end:
        fragments.append((cursor, end))
    return fragments


def selected_candidates(path: Path) -> list[dict[str, str]]:
    _fields, rows = read_tsv(path)
    return [row for row in rows if row["confidence"] in ADMITTED_CONFIDENCE]


def proposed_functions(
    config_dir: Path,
    candidates: list[dict[str, str]],
) -> list[dict[str, object]]:
    _fields, current = read_tsv(config_dir / "functions.tsv")
    intervals: dict[str, list[tuple[int, int]]] = defaultdict(list)
    for row in current:
        start = parse_int(row["va"])
        intervals[row["image"]].append((start, start + parse_int(row["size"])))

    added: list[dict[str, object]] = []
    for candidate in candidates:
        image = candidate["image"]
        layout = IMAGE_LAYOUTS[image]
        start = parse_int(candidate["va"])
        end = parse_int(candidate["return_va"]) + 8
        if any(start < other_end and other_start < end
               for other_start, other_end in intervals[image]):
            raise ValueError(f"candidate overlaps admitted function: {image} {start:#x}")
        intervals[image].append((start, end))
        confidence = (
            "mips-frame-carve"
            if candidate["confidence"] == "strong"
            else "mips-return-carve"
        )
        added.append({
            "image": image,
            "va": format_hex(start),
            "file_offset": format_size(layout.file_offset(start)),
            "size": format_size(end - start),
            "body_size": format_size(end - start),
            "fragments": 1,
            "kind": "function",
            "confidence": confidence,
            "name": "",
            "labels": "",
            "provenance": "scripts/kf/audit_functions.py",
            "note": f"anonymous WIP start; {candidate['note']}",
        })
    result: list[dict[str, object]] = [dict(row) for row in current]
    result.extend(added)
    return sorted(
        result,
        key=lambda row: (IMAGE_ORDER[str(row["image"])], parse_int(str(row["va"]))),
    )


def proposed_data(
    config_dir: Path,
    candidates: list[dict[str, str]],
) -> list[dict[str, object]]:
    _fields, current = read_tsv(config_dir / "data.tsv")
    cuts: dict[str, list[tuple[int, int]]] = defaultdict(list)
    for candidate in candidates:
        cuts[candidate["image"]].append(
            (parse_int(candidate["va"]), parse_int(candidate["return_va"]) + 8)
        )
    for image in cuts:
        cuts[image].sort()

    result: list[dict[str, object]] = []
    for row in current:
        if row["kind"] != "unclassified":
            result.append(dict(row))
            continue
        image = row["image"]
        layout = IMAGE_LAYOUTS[image]
        start = parse_int(row["va"])
        end = start + parse_int(row["size"])
        for fragment_start, fragment_end in subtract_interval(
            start, end, cuts.get(image, [])
        ):
            fragment = dict(row)
            fragment["va"] = format_hex(fragment_start)
            fragment["file_offset"] = format_size(layout.file_offset(fragment_start))
            fragment["size"] = format_size(fragment_end - fragment_start)
            result.append(fragment)
    return sorted(
        result,
        key=lambda row: (
            IMAGE_ORDER[str(row["image"])],
            parse_int(str(row["va"])),
            str(row["kind"]),
        ),
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config-dir", type=Path, default=Path("config/retail"))
    parser.add_argument(
        "--candidates",
        type=Path,
        default=Path("build/function-audit/candidates.tsv"),
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("build/function-audit/proposed-retail"),
    )
    args = parser.parse_args()

    candidates = selected_candidates(args.candidates)
    functions = proposed_functions(args.config_dir, candidates)
    data = proposed_data(args.config_dir, candidates)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_tsv(
        args.output_dir / "functions.tsv",
        FUNCTION_FIELDS,
        functions,
        comments(args.config_dir / "functions.tsv"),
    )
    write_tsv(
        args.output_dir / "data.tsv",
        DATA_FIELDS,
        data,
        comments(args.config_dir / "data.tsv"),
    )
    shutil.copy2(args.config_dir / "relocs.tsv", args.output_dir / "relocs.tsv")
    counts = Counter(candidate["image"] for candidate in candidates)
    print(
        "[function-admission] added="
        f"{len(candidates)} ({', '.join(f'{key}={counts[key]}' for key in counts)}) "
        f"total={len(functions)} output={args.output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
