"""Export image-qualified SDK reference witnesses, including executable startup.

Run with ``nix develop -c python3 -m scripts.kf.sdk_usage``. Unreached inventory
rows remain unresolved: missing indirect targets prevent an unused-code proof.
"""

from __future__ import annotations

import argparse
from collections import Counter
import csv
import json
from pathlib import Path

from scripts.kf.data_reachability import audit, data_extents
from scripts.kf.manifest import load
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv
from scripts.kf.sema import Context


def usage_rows(inventory: list[dict], reports: list[dict]) -> list[dict]:
    reached = {(r["image"], r["va"]): r for report in reports
               for r in report["ranges"] if r["kind"] == "function"}
    result = []
    for row in inventory:
        witness = reached.get((row["image"], parse_int(row["va"])))
        result.append({
            **row,
            "reference_status": witness["reachability"] if witness else "unreached",
            "via_owner": (witness.get("via_owner") or "entry") if witness else "",
            "via_reference": witness["via_reference"] if witness else "",
        })
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=BUILD / "sdk-usage")
    args = parser.parse_args(argv)
    output = args.output_dir.resolve()
    if not output.is_relative_to(BUILD.resolve()):
        raise ValueError("SDK usage reports must be written under this worktree's build/")
    extents = data_extents(load())
    reports = []
    for image in IMAGE_LAYOUTS:
        ctx = Context(image)
        reports.append(audit(image, ctx.idx.functions, extents, ctx.refs.references,
                             ctx.img, include_entry=True))
    fields, inventory = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
    rows = usage_rows(inventory, reports)
    output.mkdir(parents=True, exist_ok=True)
    with (output / "functions.tsv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=[*fields, "reference_status",
                                                   "via_owner", "via_reference"], delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)
    (output / "witnesses.json").write_text(json.dumps(reports, indent=2) + "\n")
    for image in IMAGE_LAYOUTS:
        counts = Counter(r["reference_status"] for r in rows if r["image"] == image)
        print(image, dict(counts))
    print(f"Reports: {output}; unreached does not mean unused.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
