"""Confidence-preserving reference graph for one PS-X executable."""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass, replace
from functools import lru_cache

from scripts.kf.delink import load_catalog
from scripts.kf.relocations import (
    decode_hi_lo_target,
    decode_mips26_target,
    validate_relocation,
)
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.cfg import build_graph
from scripts.kf.sema.image import RetailImage, retail
from scripts.kf.sema.index import Binding, Index, index


TIER_RANK = {"rejected": 0, "candidate": 1, "validated": 2, "proven": 3}


@dataclass(frozen=True)
class Origin:
    tier: str
    channel: str
    confidence: str
    status: str
    provenance: str
    reason: str = ""

    def as_dict(self) -> dict[str, object]:
        return {
            "tier": self.tier,
            "channel": self.channel,
            "confidence": self.confidence,
            "status": self.status,
            "provenance": self.provenance,
            "reason": self.reason or None,
        }


@dataclass(frozen=True)
class Reference:
    image: str
    site: int
    target: int | None
    kind: str
    owner: int | None
    paired_site: int | None = None
    tier: str = "candidate"
    origins: tuple[Origin, ...] = ()

    def as_dict(self) -> dict[str, object]:
        return {
            "image": self.image,
            "site": self.site,
            "paired_site": self.paired_site,
            "target": self.target,
            "kind": self.kind,
            "owner": self.owner,
            "tier": self.tier,
            "origins": [origin.as_dict() for origin in self.origins],
        }


def _reference_kind(row: dict[str, str], owner: Binding | None, idx: Index) -> str:
    if row["kind"] == "mips_hi16_lo16":
        return "address"
    if row["kind"] == "mips32_candidate":
        return "pointer"
    if row["opcode"] == "jal":
        return "call"
    target = parse_int(row["target_va"])
    if owner is not None and owner.contains_body(target):
        return "branch"
    return "tail" if idx.function(target) is not None else "jump"


class Evidence:
    def __init__(
        self,
        image: str,
        *,
        idx: Index | None = None,
        img: RetailImage | None = None,
        rows: list[dict[str, str]] | None = None,
    ):
        self.image = image
        self.idx = idx or index(image)
        self.img = img or retail(image)
        self.catalog = load_catalog(self.idx.config_dir)
        if rows is None:
            _, all_rows = read_tsv(self.idx.config_dir / "relocs.tsv")
            rows = [row for row in all_rows if row["image"] == image]
        self.rows = rows
        self._references: tuple[Reference, ...] | None = None
        self._by_site: dict[int, tuple[Reference, ...]] | None = None

    def _byte_reason(
        self,
        row: dict[str, str],
        owner: Binding | None,
    ) -> str | None:
        site = parse_int(row["site_va"])
        target = parse_int(row["target_va"])
        word = self.img.u32(site)
        if word is None:
            return "site-outside-load-image"
        if row["kind"] == "mips32_candidate":
            return None if word == target else "decoded-target-mismatch"
        if row["kind"] == "mips26":
            expected = {"j": 2, "jal": 3}.get(row["opcode"])
            if expected is None or word >> 26 != expected:
                return "instruction-opcode-mismatch"
            return (
                None
                if decode_mips26_target(site, word) == target
                else "decoded-target-mismatch"
            )
        if row["kind"] != "mips_hi16_lo16":
            return "unsupported-relocation-kind"
        if not row["paired_site_va"]:
            return "missing-paired-site"
        paired = parse_int(row["paired_site_va"])
        paired_word = self.img.u32(paired)
        if paired_word is None:
            return "paired-site-outside-load-image"
        if owner is not None and not owner.contains(paired, 4):
            return "pair-crosses-function-extent"
        try:
            decoded = decode_hi_lo_target(word, paired_word)
        except ValueError as error:
            return f"instruction-pair-mismatch:{error}"
        return None if decoded == target else "decoded-target-mismatch"

    def _relocation_references(self) -> list[Reference]:
        occupied = Counter()
        reviewed = Counter()
        for row in self.rows:
            if row["status"] == "rejected":
                continue
            sites = [parse_int(row["site_va"])]
            if row["paired_site_va"]:
                sites.append(parse_int(row["paired_site_va"]))
            for site in sites:
                occupied[site] += 1
                if row["status"] == "reviewed":
                    reviewed[site] += 1

        out: list[Reference] = []
        for row in self.rows:
            site = parse_int(row["site_va"])
            target = parse_int(row["target_va"])
            paired = parse_int(row["paired_site_va"]) if row["paired_site_va"] else None
            owner = self.idx.function_owner(site)
            site_owner = owner or self.idx.data_owner(site)
            reason = ""
            tier = "candidate"
            byte_reason = self._byte_reason(row, owner)
            overlap_counts = reviewed if row["status"] == "reviewed" else occupied
            overlaps = (
                overlap_counts[site] > 1
                or paired is not None and overlap_counts[paired] > 1
            )
            if row["status"] == "rejected":
                tier, reason = "rejected", "candidate-rejected"
            elif byte_reason:
                tier, reason = "rejected", byte_reason
            elif overlaps:
                reason = "ambiguous-overlapping-candidates"
            elif owner is not None and owner.fragments == 1:
                function = self.catalog.function_starts[self.image].get(owner.va)
                if function is not None:
                    blob = self.img.require(owner.va, owner.size)
                    try:
                        validate_relocation(blob, function, row, self.catalog, "safe")
                    except ValueError as error:
                        reason = str(error)
                    else:
                        tier = "validated"
            elif owner is None:
                reason = "site-outside-function"
            else:
                reason = "function-is-fragmented"

            origin = Origin(
                tier,
                row["channel"],
                row["confidence"],
                row["status"],
                row["provenance"],
                reason,
            )
            out.append(Reference(
                self.image,
                site,
                target,
                _reference_kind(row, owner, self.idx),
                site_owner.va if site_owner else None,
                paired,
                tier,
                (origin,),
            ))
        return out

    def _control_references(self) -> list[Reference]:
        out: list[Reference] = []
        for function in self.idx.functions:
            if function.fragments != 1:
                continue
            payload = self.img.read(function.va, function.body_size)
            if payload is None:
                continue
            graph = build_graph(function, payload, self.idx)
            for edge in graph.edges:
                if edge.kind not in {
                    "call",
                    "tail",
                    "jump-external",
                    "taken-external",
                    "indirect-call",
                    "indirect-jump",
                }:
                    continue
                target = edge.target
                if target is not None and function.contains_body(target):
                    continue
                kind = {
                    "jump-external": "jump",
                    "taken-external": "branch",
                }.get(edge.kind, edge.kind)
                origin = Origin(
                    "proven",
                    "decoded-control-flow",
                    "instruction-bytes",
                    "decoded",
                    "scripts/kf/sema/mips.py",
                )
                out.append(Reference(
                    self.image,
                    edge.site,
                    target,
                    kind,
                    function.va,
                    tier="proven",
                    origins=(origin,),
                ))
        return out

    @property
    def references(self) -> tuple[Reference, ...]:
        if self._references is None:
            merged: dict[tuple[int, int | None, str], Reference] = {}
            for reference in [
                *self._relocation_references(),
                *self._control_references(),
            ]:
                key = (reference.site, reference.target, reference.kind)
                previous = merged.get(key)
                if previous is None:
                    merged[key] = reference
                    continue
                tier = max(
                    (previous.tier, reference.tier),
                    key=lambda value: TIER_RANK[value],
                )
                origins = tuple(dict.fromkeys((*previous.origins, *reference.origins)))
                merged[key] = replace(previous, tier=tier, origins=origins)
            self._references = tuple(sorted(
                merged.values(),
                key=lambda row: (
                    row.site,
                    -1 if row.target is None else row.target,
                    row.kind,
                ),
            ))
        return self._references

    @staticmethod
    def visible(reference: Reference, *, confirmed_only: bool = False) -> bool:
        if reference.tier == "rejected":
            return False
        return not confirmed_only or reference.tier in {"proven", "validated"}

    def incoming(
        self,
        target: Binding | int,
        *,
        confirmed_only: bool = False,
    ) -> tuple[Reference, ...]:
        lo = target.va if isinstance(target, Binding) else target
        hi = target.end if isinstance(target, Binding) else target + 1
        return tuple(
            row for row in self.references
            if self.visible(row, confirmed_only=confirmed_only)
            and row.target is not None
            and lo <= row.target < hi
        )

    def outgoing(
        self,
        function: Binding,
        *,
        confirmed_only: bool = False,
    ) -> tuple[Reference, ...]:
        return tuple(
            row for row in self.references
            if self.visible(row, confirmed_only=confirmed_only)
            and function.contains(row.site)
        )

    def at_site(
        self,
        site: int,
        *,
        confirmed_only: bool = False,
    ) -> tuple[Reference, ...]:
        if self._by_site is None:
            grouped: dict[int, list[Reference]] = {}
            for row in self.references:
                grouped.setdefault(row.site, []).append(row)
                if row.paired_site is not None:
                    grouped.setdefault(row.paired_site, []).append(row)
            self._by_site = {
                va: tuple(rows) for va, rows in grouped.items()
            }
        return tuple(
            row for row in self._by_site.get(site, ())
            if self.visible(row, confirmed_only=confirmed_only)
        )


@lru_cache(maxsize=3)
def evidence(image: str) -> Evidence:
    return Evidence(image)
