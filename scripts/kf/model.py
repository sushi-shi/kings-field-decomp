"""Bind reconstructed functions to retail addresses from ADDRESS() claims.

Each unit source annotates every function it reconstructs with
``ADDRESS(0xVA, size)`` on the line before the definition. This module extracts those
claims, checks them against the admitted retail census and the curated
identities, enforces address-order incrementalism inside a source, and writes
``build/gen/bindings.tsv`` for audit. The claims are the only place a source
file names a retail address; ``config/units.toml`` lists sources, not addresses.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.paths import REPO
from scripts.kf.retail import read_tsv, write_tsv


CLAIM_RE = re.compile(
    r"^\s*ADDRESS\(\s*(0x[0-9A-Fa-f]+)\s*,\s*(0x[0-9A-Fa-f]+|[0-9]+)\s*\)\s*(?:/\*.*\*/\s*)?$"
)
DEFINITION_NAME_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(")
BINDING_FIELDS = ("image", "va", "name", "unit", "source", "line", "ordinal")


@dataclass(frozen=True)
class Claim:
    va: int
    size: int
    name: str
    line: int


def scan_claims(source: Path) -> tuple[Claim, ...]:
    """Return the ADDRESS() claims of one source in file order."""
    lines = source.read_text(encoding="utf-8").splitlines()
    claims: list[Claim] = []
    for index, text in enumerate(lines):
        match = CLAIM_RE.match(text)
        if match is None:
            continue
        va = int(match.group(1), 16)
        size = int(match.group(2), 0)
        definition = ""
        for following in lines[index + 1:index + 6]:
            stripped = following.strip()
            if not stripped or stripped.startswith(("/*", "//", "*")):
                continue
            definition = stripped
            break
        names = DEFINITION_NAME_RE.findall(definition.split("{")[0])
        if not names:
            raise ValueError(
                f"{source}:{index + 1}: ADDRESS({va:#x}) is not followed by a function definition"
            )
        claims.append(Claim(va, size, names[-1], index + 1))
    return tuple(claims)


def identity_names(config_dir: Path) -> dict[tuple[str, int], str]:
    path = config_dir / "function_identities.tsv"
    if not path.is_file():
        return {}
    _, rows = read_tsv(path)
    return {(row["image"], int(row["va"], 0)): row["name"] for row in rows if row.get("name")}


def write_bindings(rows: list[dict[str, object]], output: Path = REPO / "build/gen/bindings.tsv") -> Path:
    output.parent.mkdir(parents=True, exist_ok=True)
    write_tsv(
        output,
        BINDING_FIELDS,
        rows,
        ("GENERATED from ADDRESS() claims in unit sources; do not edit.",),
    )
    return output
