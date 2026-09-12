"""Bind reconstructed functions and data to retail addresses from source claims.

Each unit source annotates every function it reconstructs with
``ADDRESS(0xVA, size)`` on the line before the definition, and every global it
owns with ``DATA(0xVA, size)`` on the line before the definition. This module
extracts those claims; the manifest loader checks them against the admitted
retail census and the curated identities, enforces address-order
incrementalism inside a source, and writes ``build/gen/bindings.tsv`` for
audit. The claims are the only place a source file names a retail address;
``config/units.toml`` lists sources, not addresses.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.paths import REPO
from scripts.kf.retail import read_tsv, write_tsv


CLAIM_RE = re.compile(
    r"^\s*(ADDRESS|DATA)\(\s*(0x[0-9A-Fa-f]+)\s*,\s*(0x[0-9A-Fa-f]+|[0-9]+)\s*\)\s*"
    r"(?:/\*.*\*/\s*)?$"
)
ADDRESS_AT_RE = re.compile(
    r'^\s*ADDRESS_AT\(\s*"([A-Z]+)"\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*'
    r"(0x[0-9A-Fa-f]+|[0-9]+)\s*\)\s*(?:/\*.*\*/\s*)?$"
)
IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
FUNCTION_POINTER_RE = re.compile(r"\(\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)")
BINDING_FIELDS = ("image", "va", "kind", "name", "unit", "source", "line", "ordinal")


@dataclass(frozen=True)
class Claim:
    va: int
    size: int
    name: str
    line: int
    # Short image token ("GAME"/"OPEN"/"PSX") for an ADDRESS_AT() claim in a
    # shared source; None for a plain ADDRESS() claim, which binds to the
    # claiming unit's own image.
    image: str | None = None


@dataclass(frozen=True)
class DataClaim:
    va: int
    size: int
    name: str
    line: int


@dataclass(frozen=True)
class RodataClaim:
    va: int
    size: int
    line: int


RODATA_RE = re.compile(
    r"^\s*RODATA\(\s*(0x[0-9A-Fa-f]+)\s*,\s*(0x[0-9A-Fa-f]+|[0-9]+)\s*\)\s*"
    r"(?:/\*.*\*/\s*)?$"
)


def scan_rodata_claims(source: Path) -> tuple[RodataClaim, ...]:
    """Return the RODATA() claims of one source in file order."""
    claims: list[RodataClaim] = []
    for index, text in enumerate(source.read_text(encoding="utf-8").splitlines()):
        match = RODATA_RE.match(text)
        if match is not None:
            claims.append(RodataClaim(int(match.group(1), 16), int(match.group(2), 0), index + 1))
    return tuple(claims)


def _definition_after(lines: list[str], index: int) -> str:
    for following in lines[index + 1:index + 6]:
        stripped = following.strip()
        if not stripped or stripped.startswith(("/*", "//", "*")):
            continue
        # A shared function stacks several ADDRESS_AT()/ADDRESS() claims before
        # its definition; skip sibling claims to reach the declarator.
        if CLAIM_RE.match(following) or ADDRESS_AT_RE.match(following):
            continue
        return stripped
    return ""


def _data_name(definition: str) -> str | None:
    """Declarator identifier of a global definition line (``u32 x[4] = {``)."""
    if not definition or definition.startswith("extern ") or definition.startswith("#"):
        return None
    cut = len(definition)
    for stop in "=;[{":
        position = definition.find(stop)
        if position != -1:
            cut = min(cut, position)
    head = definition[:cut]
    pointer = FUNCTION_POINTER_RE.search(head)
    if pointer is not None:
        return pointer.group(1)
    if "(" in head:
        return None
    names = IDENTIFIER_RE.findall(head)
    return names[-1] if names else None


def _function_name(definition: str) -> str | None:
    """Return the declarator name without mistaking parameter macros for it."""
    head = definition.split("{", 1)[0]
    depth = 0
    candidates: list[str] = []
    for index, character in enumerate(head):
        if character == "(":
            if depth == 0:
                names = IDENTIFIER_RE.findall(head[:index])
                if names:
                    candidates.append(names[-1])
            depth += 1
        elif character == ")" and depth:
            depth -= 1
    return candidates[-1] if candidates else None


def scan_source(source: Path) -> tuple[tuple[Claim, ...], tuple[DataClaim, ...]]:
    """Return the ADDRESS()/ADDRESS_AT() and DATA() claims in file order.

    Function claims may stack (one ADDRESS_AT() per image before a shared
    definition); a run of consecutive function-claim lines shares the one
    definition that follows it. Each ADDRESS_AT() claim carries its image; a
    plain ADDRESS() claim carries ``image=None`` and binds to its unit's image.
    """
    lines = source.read_text(encoding="utf-8").splitlines()
    claims: list[Claim] = []
    data_claims: list[DataClaim] = []
    index = 0
    total = len(lines)
    while index < total:
        text = lines[index]
        plain = CLAIM_RE.match(text)
        at = ADDRESS_AT_RE.match(text)
        if plain is not None and plain.group(1) == "DATA":
            va = int(plain.group(2), 16)
            size = int(plain.group(3), 0)
            name = _data_name(_definition_after(lines, index))
            if name is None:
                raise ValueError(
                    f"{source}:{index + 1}: DATA({va:#x}) is not followed by a "
                    "global definition"
                )
            data_claims.append(DataClaim(va, size, name, index + 1))
            index += 1
            continue
        if at is None and plain is None:
            index += 1
            continue
        # A run of consecutive function claims (ADDRESS()/ADDRESS_AT()) that
        # share the single definition following the run.
        run: list[tuple[str | None, int, int, int]] = []
        first_line = index + 1
        while index < total:
            line_at = ADDRESS_AT_RE.match(lines[index])
            line_plain = CLAIM_RE.match(lines[index])
            if line_at is not None:
                run.append((line_at.group(1), int(line_at.group(2), 16),
                            int(line_at.group(3), 0), index + 1))
                index += 1
            elif line_plain is not None and line_plain.group(1) == "ADDRESS":
                run.append((None, int(line_plain.group(2), 16),
                            int(line_plain.group(3), 0), index + 1))
                index += 1
            else:
                break
        name = _function_name(_definition_after(lines, index - 1))
        if name is None:
            va = run[0][1]
            raise ValueError(
                f"{source}:{first_line}: ADDRESS({va:#x}) is not followed by a "
                "function definition"
            )
        for image, va, size, line in run:
            claims.append(Claim(va, size, name, line, image))
    return tuple(claims), tuple(data_claims)


def scan_claims(source: Path) -> tuple[Claim, ...]:
    """Return the ADDRESS() claims of one source in file order."""
    return scan_source(source)[0]


def scan_data_claims(source: Path) -> tuple[DataClaim, ...]:
    """Return the DATA() claims of one source in file order."""
    return scan_source(source)[1]


ADDRESS_NAME_RE = re.compile(r"\b(func|DAT)_([0-9a-fA-F]{8})\b")


def stale_address_names(
    source: Path,
    image: str,
    functions: dict[tuple[str, int], str],
    data: dict[tuple[str, int], str],
) -> list[tuple[str, str]]:
    """Address-derived spellings whose identity already has a curated name.

    Labelled functions and data are spelled by their identities in source; an
    address-derived token for a labelled item is stale and would no longer
    match the symbol the delinker emits.
    """
    stale: list[tuple[str, str]] = []
    seen: set[str] = set()
    for match in ADDRESS_NAME_RE.finditer(source.read_text(encoding="utf-8")):
        token = match.group(0)
        if token in seen:
            continue
        seen.add(token)
        table = functions if match.group(1) == "func" else data
        name = table.get((image, int(match.group(2), 16)))
        if name and name != token:
            stale.append((token, name))
    return stale


def identity_names(config_dir: Path) -> dict[tuple[str, int], str]:
    path = config_dir / "function_identities.tsv"
    if not path.is_file():
        return {}
    _, rows = read_tsv(path)
    return {(row["image"], int(row["va"], 0)): row["name"] for row in rows if row.get("name")}



@dataclass(frozen=True)
class DataIdentity:
    name: str
    size: int
    storage: str
    scope: str
    section: str = ""
    reservation_size: int = 0


def data_identities(config_dir: Path) -> dict[tuple[str, int], DataIdentity]:
    """Curated data identities keyed by exact (image, va)."""
    from scripts.kf.data_sections import load as load_sections

    from scripts.kf.data_reservations import load as load_reservations

    sections = load_sections(config_dir)
    reservations = load_reservations(config_dir)
    path = config_dir / "data_identities.tsv"
    if not path.is_file():
        return {}
    _, rows = read_tsv(path)
    return {
        (row["image"], int(row["va"], 0)): DataIdentity(
            row["name"], int(row["size"], 0), row.get("storage", ""), row.get("scope", ""),
            sections.get((row["image"], int(row["va"], 0)), ""),
            reservations.get((row["image"], int(row["va"], 0)), 0),
        )
        for row in rows
        if row.get("name")
    }


def write_bindings(rows: list[dict[str, object]], output: Path = REPO / "build/gen/bindings.tsv") -> Path:
    output.parent.mkdir(parents=True, exist_ok=True)
    write_tsv(
        output,
        BINDING_FIELDS,
        rows,
        ("GENERATED from ADDRESS()/DATA() claims in unit sources; do not edit.",),
    )
    return output
