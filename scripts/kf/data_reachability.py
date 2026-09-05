"""Audit the known code/data reference closure of all admitted game functions.

This is a conservative worklist over the shared semantic evidence, not a proof
that the reference census is exhaustive. Candidate paths stay candidate; a TSV
gap or referenced prefix never becomes a complete object just by being reached.
The report separates source ownership, configuration-only extents, and missing
models. Configuration-only bytes are accounted in the inventory but do not yet
have a reconstruction comparison, so they remain a default-build diagnostic.
"""

from __future__ import annotations

import argparse
import bisect
import json
from collections import Counter, deque
from dataclasses import asdict, dataclass
from pathlib import Path

from scripts.kf.manifest import Manifest, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.evidence import Reference, TIER_RANK
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


@dataclass(frozen=True)
class DataExtent:
    image: str
    va: int
    size: int
    name: str
    storage: str
    source: str
    unit: str = ""
    confidence: str = ""
    evidence: str = ""
    note: str = ""
    datatype: str = ""
    classification: str = ""

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def claimed(self) -> bool:
        return self.source in {"DATA", "RODATA"}

    @property
    def key(self) -> str:
        return f"{self.source}:{self.va:08x}:{self.size:x}:{self.name}"


def data_extents(
    manifest: Manifest, config_dir: Path = RETAIL_CONFIG,
) -> tuple[DataExtent, ...]:
    out = []
    for unit in manifest.units:
        out.extend(DataExtent(
            unit.image, datum.va, datum.size, datum.symbol, datum.storage,
            "DATA", unit.unit, "source-claim", unit.source,
        ) for datum in unit.data)
        if unit.rodata:
            va, size = unit.rodata
            out.append(DataExtent(
                unit.image, va, size, f"{unit.unit}.rodata", "load", "RODATA",
                unit.unit, "source-claim", unit.source,
            ))
    for filename in ("data_identities.tsv", "data.tsv"):
        _, rows = read_tsv(config_dir / filename)
        for row in rows:
            out.append(DataExtent(
                row["image"], int(row["va"], 0), int(row["size"], 0),
                row["name"] or row.get("kind", ""),
                row.get("storage", "load"), filename,
                confidence=row["confidence"],
                evidence=row.get("evidence", row.get("provenance", "")),
                note=row.get("note", ""),
                datatype=row.get("datatype", ""),
                classification=row.get("kind", row.get("storage", "")),
            ))
    return tuple(out)


def _function_key(function: Binding) -> str:
    return f"function:{function.va:08x}"


def access_span(reference: Reference, img: RetailImage) -> tuple[int, int]:
    """Minimum directly observed memory span, or one anchor byte for an address.

    An address-taking expression does not establish an object extent. The
    returned span is only a boundary-crossing control, never a size proposal.
    LWL/LWR/SWL/SWR operate on the containing aligned word.
    """
    assert reference.target is not None
    target = reference.target
    word = img.u32(reference.paired_site) if reference.paired_site is not None else None
    if reference.referent is not None and (word is None or word >> 26 in {8, 9}):
        # Taking an address (including a biased base or one-past pointer) does
        # not read S+A. Follow the named allocation; memory-op lows below still
        # check their real access against its extent.
        return reference.referent.va, 1
    if word is None:
        return target, 1
    opcode = word >> 26
    if opcode in {0x22, 0x26, 0x2A, 0x2E}:
        return target & ~3, 4
    width = {
        0x20: 1, 0x24: 1, 0x28: 1,
        0x21: 2, 0x25: 2, 0x29: 2,
        0x23: 4, 0x2B: 4,
    }.get(opcode, 1)
    return target, width


def audit(
    image: str,
    functions: tuple[Binding, ...],
    extents: tuple[DataExtent, ...],
    references: tuple[Reference, ...],
    img: RetailImage,
) -> dict:
    """Lossless reference worklist with weakest-edge reachability witnesses.

    Roots are selected by admitted non-vendor identity, not manifestation or
    fuzzy score. A reached vendor routine is traversed without adding it to the
    game root/progress denominator. Entire admitted function/object extents are
    scanned conservatively; candidate discovery does not assert execution.
    """
    if img.image != image:
        raise ValueError("reference audit image does not match retail bytes")
    functions = tuple(f for f in functions if f.image == image)
    extents = tuple(d for d in extents if d.image == image)
    references = tuple(sorted(
        (r for r in references if r.image == image),
        key=lambda r: (r.site, r.target if r.target is not None else -1, r.kind),
    ))
    sites = [r.site for r in references]
    roots = tuple(f for f in functions if not f.vendored)
    nodes: dict[str, Binding | DataExtent] = {}
    visits: dict[str, tuple[int, str | None, int | None]] = {}
    outgoing: dict[str, list[int]] = {}
    destinations: dict[int, list[str]] = {}
    issues: dict[tuple, dict] = {}
    queue: deque[str] = deque()

    def issue(code: str, key: str, ref: int | None = None, **detail) -> None:
        issues[code, key, ref] = {"code": code, "owner": key, "reference": ref, **detail}

    def visit(node: Binding | DataExtent, rank: int, parent: str | None, via: int | None):
        key = _function_key(node) if isinstance(node, Binding) else node.key
        if rank > visits.get(key, (0, None, None))[0]:
            nodes[key] = node
            visits[key] = rank, parent, via
            queue.append(key)
        return key

    def owners(va: int) -> list[Binding | DataExtent]:
        code = [f for f in functions if f.va <= va < f.body_end]
        data = [d for d in extents if d.va <= va < d.end]
        # The source owner replaces its finer census/identity rows, including
        # interior string and switch-table entries. Config rows remain attached
        # to the selected range as evidence rather than duplicate allocations.
        claimed = [d for d in data if d.claimed]
        if claimed:
            return [*code, *claimed]
        identities = [d for d in data if d.source == "data_identities.tsv"]
        return [*code, *(identities or data)]

    for root in roots:
        visit(root, TIER_RANK["proven"], None, None)
    if not roots:
        issue("no-game-roots", image)
    while queue:
        key = queue.popleft()
        node = nodes[key]
        rank = visits[key][0]
        end = node.body_end if isinstance(node, Binding) else node.end
        if isinstance(node, Binding) and node.fragments != 1:
            issue("fragmented-function", key, fragments=node.fragments)
            continue
        if key not in outgoing:
            lo, hi = bisect.bisect_left(sites, node.va), bisect.bisect_left(sites, end)
            outgoing[key] = list(range(lo, hi))
        for number in outgoing[key]:
            reference = references[number]
            if reference.site + 4 > end:
                issue("relocation-site-crosses-owner", key, number)
            if reference.paired_site is not None and not (
                node.va <= reference.paired_site and reference.paired_site + 4 <= end
            ):
                issue("relocation-pair-crosses-owner", key, number)
            if reference.tier == "rejected":
                # Explicitly rejected hypotheses are retained but not followed.
                # A byte-invalid live TSV claim is a distinct curation failure.
                if not any(o.status == "rejected" for o in reference.origins):
                    issue("invalid-live-reference", key, number)
                continue
            if reference.target is None:
                issue("unresolved-indirect-control", key, number)
                continue
            path_rank = min(rank, TIER_RANK[reference.tier])
            found = owners(reference.destination)
            if not found:
                issue("unmodeled-target", key, number, target=reference.target)
                destinations[number] = []
                continue
            if len(found) > 1:
                issue("ambiguous-target-owner", key, number, target=reference.target)
                path_rank = min(path_rank, TIER_RANK["candidate"])
            destinations[number] = []
            for owner in found:
                target_key = visit(owner, path_rank, key, number)
                destinations[number].append(target_key)
                start, width = access_span(reference, img)
                owner_end = owner.body_end if isinstance(owner, Binding) else owner.end
                if start < owner.va or start + width > owner_end:
                    issue("access-crosses-owner", key, number, target=reference.target,
                          access_start=start, access_size=width)

    tier = {rank: name for name, rank in TIER_RANK.items()}
    range_rows = []
    for key, node in sorted(nodes.items()):
        rank, parent, via = visits[key]
        row = {
            "key": key, "reachability": tier[rank], "via_owner": parent,
            "via_reference": via,
        }
        if isinstance(node, Binding):
            row.update({**node.as_dict(), "kind": "function", "function_kind": node.kind})
            # A reached instruction range cannot silently coexist with a data
            # claim merely because no reference targets the overlap. Preserve
            # the conflicting metadata for review; do not reclassify either
            # owner from a byte-pattern scan or an admitted extent alone.
            if node.fragments == 1:
                overlaps = [
                    {**asdict(d), "data_owner": d.key,
                     "overlap_va": max(node.va, d.va),
                     "overlap_size": min(node.body_end, d.end) - max(node.va, d.va)}
                    for d in extents if d.va < node.body_end and node.va < d.end
                ]
                if overlaps:
                    issue("code-data-owner-overlap", key, overlaps=overlaps)
        else:
            row.update({"kind": "data", **asdict(node)})
            row["comparison"] = "source-owned" if node.claimed else "config-only"
            row["inventory"] = [
                asdict(d) for d in extents if not d.claimed
                and d.va < node.end and node.va < d.end
            ]
            if not node.claimed:
                issue("config-only-data-not-compared", key)
        range_rows.append(row)

    reference_rows = []
    for key in sorted(outgoing):
        rank = visits[key][0]
        for number in outgoing[key]:
            reference = references[number]
            path_rank = min(rank, TIER_RANK[reference.tier])
            if len(destinations.get(number, [])) > 1:
                path_rank = min(path_rank, TIER_RANK["candidate"])
            if path_rank == TIER_RANK["candidate"]:
                issue("candidate-reference-path", key, number)
            reference_rows.append({
                "id": number, **reference.as_dict(), "source_owner": key,
                "path_tier": tier[path_rank], "target_owners": destinations.get(number, []),
            })
    issue_rows = sorted(issues.values(), key=lambda r: (
        r["code"], r["owner"], -1 if r["reference"] is None else r["reference"],
    ))
    return {
        "schema": "kf-data-reachability-v1", "image": image,
        "scope": "known-reference-census", "complete_reachable_bytes_proven": False,
        "limitations": [
            "Computed/GP-relative addresses and references absent from the census need audit.",
            "Config extents may be referenced prefixes, not complete allocations.",
            "Source ownership does not prove matched bytes; the data gate checks objects separately.",
            "Whole admitted extents are traversed conservatively, not dynamic execution paths.",
        ],
        "roots": [_function_key(f) for f in roots],
        "summary": {
            "game_roots": len(roots),
            "reached_vendor_functions": sum(
                isinstance(n, Binding) and n.vendored for n in nodes.values()
            ),
            "source_owned_data_ranges": sum(
                isinstance(n, DataExtent) and n.claimed for n in nodes.values()
            ),
            "config_only_data_ranges": sum(
                isinstance(n, DataExtent) and not n.claimed for n in nodes.values()
            ),
            "reference_occurrences": len(reference_rows),
            "issues": dict(sorted(Counter(r["code"] for r in issue_rows).items())),
        },
        "ranges": range_rows, "references": reference_rows, "issues": issue_rows,
    }


def run(images: tuple[str, ...], *, output: Path | None = None) -> int:
    manifest = load_manifest()
    extents = data_extents(manifest)
    reports = []
    for image in images:
        ctx = Context(image)
        report = audit(image, ctx.idx.functions, extents, ctx.refs.references, ctx.img)
        reports.append(report)
        summary = report["summary"]
        print(f"{image} known-reference closure: {summary['game_roots']} game roots, "
              f"{summary['reached_vendor_functions']} reached vendor functions, "
              f"{summary['source_owned_data_ranges']} source-owned / "
              f"{summary['config_only_data_ranges']} config-only data ranges")
        for code, count in summary["issues"].items():
            print(f"  {code}: {count}")
        print("  exhaustive reachable-byte coverage is not yet proven")
    if output is not None:
        # Reports are build products, never curated inventory updates.
        resolved = output.resolve()
        if not resolved.is_relative_to(BUILD.resolve()):
            raise ValueError("reachability reports must be written under this worktree's build/")
        resolved.parent.mkdir(parents=True, exist_ok=True)
        resolved.write_text(json.dumps(reports, indent=2, sort_keys=True) + "\n")
    return int(any(report["issues"] for report in reports))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="kf verify reachability", description=__doc__)
    aliases = {image.removesuffix(".EXE").lower(): image for image in IMAGE_LAYOUTS}
    parser.add_argument("--image", action="append", choices=tuple(aliases))
    parser.add_argument("--output", type=Path,
                        help="write full witness/evidence JSON under build/")
    args = parser.parse_args(argv)
    images = tuple(aliases[i] for i in args.image) if args.image else tuple(IMAGE_LAYOUTS)
    return run(images, output=args.output)


if __name__ == "__main__":
    raise SystemExit(main())
