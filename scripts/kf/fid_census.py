"""Build and query a deterministic function-ID corpus from Psy-Q objects.

This is the PS-X/Psy-Q analogue of the Gruntz/Rage masked-library census.  It
does not require Ghidra: ``psy-k`` exposes the object bytes, XDEF/local names,
debug function extents, and linker patch records.  Patch-owned instruction
bits are wildcarded before each reference function is compared at every
manually admitted retail function start.

The output is evidence, not an automatic edit to ``config/retail``.  HIGH
requires a substantial function, one corpus identity at the retail address,
and no more than one match for that identity in each linked image.  GAME.EXE
and OPEN.EXE are separate programs, so the same SDK function may legitimately
match once in each overlay.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import tempfile
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

from scripts.kf.retail import IMAGE_ORDER, parse_int, read_tsv, write_tsv
from scripts.kf.seed_vendored_functions import (
    ObjectSymbols,
    load_retail_payloads,
    parse_object_symbols,
    run_psyk,
)


MIN_SIZE = 12
MIN_FIXED_BITS = 48
HIGH_SIZE = 16
HIGH_FIXED_BITS = 80

SECTION_RE = re.compile(r"Section symbol number ([0-9a-f]+) '([^']+)'", re.IGNORECASE)
LOCAL_RE = re.compile(
    r"Local symbol '([^']+)' at offset ([0-9a-f]+) in section ([0-9a-f]+)",
    re.IGNORECASE,
)
FUNCTION_RE = re.compile(
    r"Function start\s*:\s*\n"
    r"\s*section\s+([0-9a-f]+)\s*\n"
    r"\s*offset\s+\$([0-9a-f]+).*?"
    r"\n\s*name\s+([^\r\n]+)\s*\n"
    r"\s*\d+\s*:\s*Function end\s*:\s*\n"
    r"\s*section\s+([0-9a-f]+)\s*\n"
    r"\s*offset\s+\$([0-9a-f]+)",
    re.IGNORECASE | re.DOTALL,
)

CORPUS_FIELDS = (
    "library",
    "module",
    "member_offset",
    "size",
    "names",
    "boundary",
    "fixed_bits",
    "relocation_bytes",
    "library_sha256",
    "object_sha256",
    "fid_sha256",
)

MATCH_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "aliases",
    "library",
    "module",
    "member_offset",
    "boundary",
    "fixed_bits",
    "identity_image_matches",
    "va_identities",
    "library_sha256",
    "object_sha256",
    "fid_sha256",
    "confidence",
    "notes",
)


@dataclass(frozen=True)
class RetailFunction:
    image: str
    va: int
    size: int
    body_size: int
    name: str
    labels: tuple[str, ...]


@dataclass(frozen=True)
class FidSignature:
    library: str
    module: str
    member_offset: int
    data: bytes
    mask: bytes
    names: tuple[str, ...]
    boundary: str
    library_sha256: str
    object_sha256: str

    @property
    def size(self) -> int:
        return len(self.data)

    @property
    def fixed_bits(self) -> int:
        return sum(value.bit_count() for value in self.mask)

    @property
    def relocation_bytes(self) -> int:
        return sum(value != 0xFF for value in self.mask)

    @property
    def identity(self) -> tuple[str, str, int, tuple[str, ...]]:
        return self.library, self.module, self.member_offset, self.names

    @property
    def fid_sha256(self) -> str:
        digest = hashlib.sha256()
        digest.update(self.data)
        digest.update(self.mask)
        return digest.hexdigest()


@dataclass(frozen=True)
class FidHit:
    function: RetailFunction
    signature: FidSignature


def sha256_path(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def split_names(value: str) -> tuple[str, ...]:
    return tuple(part for part in value.split(";") if part)


def load_retail_functions(path: Path) -> list[RetailFunction]:
    _fields, rows = read_tsv(path)
    return [
        RetailFunction(
            row["image"],
            parse_int(row["va"]),
            parse_int(row["size"]),
            parse_int(row["body_size"]),
            row["name"],
            split_names(row["labels"]),
        )
        for row in rows
        if row["kind"] == "function"
    ]


def _text_section(listing: str) -> int:
    sections = {
        int(match.group(1), 16): match.group(2)
        for match in SECTION_RE.finditer(listing)
    }
    text = [section for section, name in sections.items() if name == ".text"]
    if len(text) != 1:
        raise ValueError(f"object has {len(text)} .text sections")
    return text[0]


def _debug_functions(listing: str, text_section: int) -> list[tuple[int, int, str]]:
    result = []
    for match in FUNCTION_RE.finditer(listing):
        start_section = int(match.group(1), 16)
        end_section = int(match.group(4), 16)
        if start_section != text_section or end_section != text_section:
            continue
        start = int(match.group(2), 16)
        end = int(match.group(5), 16)
        name = match.group(3).strip()
        if end > start:
            result.append((start, end, name))
    return result


def _local_symbols(listing: str, text_section: int) -> list[tuple[int, str]]:
    return [
        (int(match.group(2), 16), match.group(1))
        for match in LOCAL_RE.finditer(listing)
        if int(match.group(3), 16) == text_section
    ]


def signatures_from_listing(
    listing: str,
    symbols: ObjectSymbols,
    *,
    library: str,
    module: str,
    library_sha256: str,
    object_sha256: str,
) -> list[FidSignature]:
    """Split one object into named function signatures.

    Compiler debug start/end records are authoritative when present.  Stripped
    objects fall back to XDEF/local-symbol starts and the next named start (or
    object end).  The boundary source is retained so a reviewer can distinguish
    those two cases.
    """
    text_section = _text_section(listing)
    names_by_offset: dict[int, set[str]] = defaultdict(set)
    for offset, name in symbols.xdefs:
        names_by_offset[offset].add(name)
    for offset, name in _local_symbols(listing, text_section):
        names_by_offset[offset].add(name)

    debug = _debug_functions(listing, text_section)
    for start, _end, name in debug:
        names_by_offset[start].add(name)

    result: list[FidSignature] = []
    covered = set()
    for start, end, debug_name in debug:
        if not (0 <= start < end <= symbols.text_size):
            continue
        covered.add(start)
        names = tuple(sorted(names_by_offset.get(start, {debug_name})))
        result.append(FidSignature(
            library,
            module,
            start,
            symbols.text_data[start:end],
            symbols.text_mask[start:end],
            names,
            "debug-function",
            library_sha256,
            object_sha256,
        ))

    starts = sorted(offset for offset in names_by_offset if 0 <= offset < symbols.text_size)
    for index, start in enumerate(starts):
        if start in covered:
            continue
        end = starts[index + 1] if index + 1 < len(starts) else symbols.text_size
        if end <= start:
            continue
        result.append(FidSignature(
            library,
            module,
            start,
            symbols.text_data[start:end],
            symbols.text_mask[start:end],
            tuple(sorted(names_by_offset[start])),
            "symbol-next" if index + 1 < len(starts) else "symbol-object-end",
            library_sha256,
            object_sha256,
        ))

    return sorted(result, key=lambda item: (item.member_offset, item.names))


def _object_signatures(
    psyk: Path,
    path: Path,
    library: str,
    library_sha256: str,
) -> list[FidSignature]:
    listing = run_psyk(psyk, "list", "--code", str(path))
    symbols = parse_object_symbols(listing)
    return signatures_from_listing(
        listing,
        symbols,
        library=library,
        module=path.stem.upper(),
        library_sha256=library_sha256,
        object_sha256=sha256_path(path),
    )


def build_corpus(psyk: Path, sdk_lib_dir: Path) -> list[FidSignature]:
    """Extract every supplied SDK archive and build the function corpus."""
    result: list[FidSignature] = []
    for archive in sorted(sdk_lib_dir.glob("*.LIB")):
        archive_hash = sha256_path(archive)
        with tempfile.TemporaryDirectory(prefix="kf-fid-") as directory:
            root = Path(directory)
            run_psyk(psyk, "extract", str(archive.resolve()), cwd=root)
            for obj in sorted(root.glob("*.OBJ")):
                result.extend(_object_signatures(
                    psyk, obj, archive.name.upper(), archive_hash
                ))
    for obj in sorted(sdk_lib_dir.glob("*.OBJ")):
        object_hash = sha256_path(obj)
        result.extend(_object_signatures(
            psyk, obj, obj.name.upper(), object_hash
        ))
    return [
        item for item in result
        if item.size >= MIN_SIZE and item.fixed_bits >= MIN_FIXED_BITS
    ]


def _matches(actual: bytes, signature: FidSignature) -> bool:
    return len(actual) == signature.size and all(
        (left & mask) == (right & mask)
        for left, right, mask in zip(actual, signature.data, signature.mask, strict=True)
    )


def find_hits(
    corpus: Iterable[FidSignature],
    functions: Iterable[RetailFunction],
    images: dict[str, tuple[int, bytes]],
) -> list[FidHit]:
    by_size: dict[int, list[FidSignature]] = defaultdict(list)
    for signature in corpus:
        by_size[signature.size].append(signature)

    hits = []
    for function in functions:
        load_address, payload = images[function.image]
        offset = function.va - load_address
        for extent in sorted({function.size, function.body_size}):
            if extent <= 0 or extent not in by_size:
                continue
            actual = payload[offset:offset + extent]
            for signature in by_size[extent]:
                if _matches(actual, signature):
                    hits.append(FidHit(function, signature))
    return hits


def _preferred_name(function: RetailFunction, names: tuple[str, ...]) -> tuple[str, str]:
    preferred = (function.name, *function.labels)
    name = next((item for item in preferred if item and item in names), "")
    if not name and names:
        name = names[0]
    return name, ";".join(item for item in names if item != name)


def classify_hits(hits: Sequence[FidHit]) -> list[dict[str, str]]:
    identity_locations: dict[
        tuple[str, str, int, tuple[str, ...]], dict[str, set[int]]
    ] = defaultdict(lambda: defaultdict(set))
    va_identities: dict[tuple[str, int], set[tuple[str, str, int, tuple[str, ...]]]] = (
        defaultdict(set)
    )
    for hit in hits:
        identity_locations[hit.signature.identity][hit.function.image].add(hit.function.va)
        va_identities[(hit.function.image, hit.function.va)].add(hit.signature.identity)

    rows = []
    for hit in hits:
        signature = hit.signature
        function = hit.function
        image_matches = max(
            len(locations) for locations in identity_locations[signature.identity].values()
        )
        identities = len(va_identities[(function.image, function.va)])
        substantial = (
            signature.size >= HIGH_SIZE and signature.fixed_bits >= HIGH_FIXED_BITS
        )
        ambiguous = image_matches > 1 or identities > 1
        if substantial and not ambiguous:
            confidence = "HIGH"
        elif not ambiguous:
            confidence = "SHORT"
        elif substantial:
            confidence = "AMBIG"
        else:
            confidence = "LOW"
        notes = []
        if image_matches > 1:
            notes.append(f"identity_image_multimatch={image_matches}")
        if identities > 1:
            notes.append(f"va_multiidentity={identities}")
        if signature.size < HIGH_SIZE:
            notes.append(f"short={signature.size}")
        if signature.fixed_bits < HIGH_FIXED_BITS:
            notes.append(f"fewfixedbits={signature.fixed_bits}")
        name, aliases = _preferred_name(function, signature.names)
        rows.append({
            "image": function.image,
            "va": f"0x{function.va:08x}",
            "size": f"0x{signature.size:x}",
            "name": name,
            "aliases": aliases,
            "library": signature.library,
            "module": signature.module,
            "member_offset": f"0x{signature.member_offset:x}",
            "boundary": signature.boundary,
            "fixed_bits": str(signature.fixed_bits),
            "identity_image_matches": str(image_matches),
            "va_identities": str(identities),
            "library_sha256": signature.library_sha256,
            "object_sha256": signature.object_sha256,
            "fid_sha256": signature.fid_sha256,
            "confidence": confidence,
            "notes": ";".join(notes) if notes else "-",
        })
    return sorted(rows, key=lambda row: (
        IMAGE_ORDER[row["image"]], parse_int(row["va"]), row["library"],
        row["module"], parse_int(row["member_offset"]), row["name"],
    ))


def corpus_rows(corpus: Iterable[FidSignature]) -> list[dict[str, str]]:
    return [
        {
            "library": item.library,
            "module": item.module,
            "member_offset": f"0x{item.member_offset:x}",
            "size": f"0x{item.size:x}",
            "names": ";".join(item.names),
            "boundary": item.boundary,
            "fixed_bits": str(item.fixed_bits),
            "relocation_bytes": str(item.relocation_bytes),
            "library_sha256": item.library_sha256,
            "object_sha256": item.object_sha256,
            "fid_sha256": item.fid_sha256,
        }
        for item in sorted(corpus, key=lambda candidate: candidate.identity)
    ]


def resolve_tool(value: Path | None, executable: str) -> Path:
    if value is not None:
        return value
    found = shutil.which(executable)
    if found is None:
        raise SystemExit(f"{executable} not found; enter nix develop or pass --{executable}")
    return Path(found)


def parse_args(arguments: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--functions", type=Path, default=Path("config/retail/functions.tsv"))
    parser.add_argument("--psyk", type=Path)
    parser.add_argument(
        "--sdk-lib-dir",
        type=Path,
        default=Path(os.environ["PSYQ_LIB"]) if "PSYQ_LIB" in os.environ else None,
    )
    parser.add_argument("--exe-dir", required=True, type=Path)
    parser.add_argument(
        "--output", type=Path, default=Path("build/fid-census/matches.tsv")
    )
    parser.add_argument(
        "--corpus-output", type=Path, default=Path("build/fid-census/corpus.tsv")
    )
    return parser.parse_args(arguments)


def _refuse_retail_output(path: Path) -> None:
    try:
        path.resolve().relative_to(Path("config/retail").resolve())
    except ValueError:
        return
    raise SystemExit("refusing to overwrite hand-owned config/retail; review build output")


def main(arguments: Sequence[str] | None = None) -> int:
    args = parse_args(arguments)
    if args.sdk_lib_dir is None:
        raise SystemExit("Psy-Q library directory missing; enter nix develop or pass --sdk-lib-dir")
    _refuse_retail_output(args.output)
    _refuse_retail_output(args.corpus_output)

    psyk = resolve_tool(args.psyk, "psyk")
    corpus = build_corpus(psyk, args.sdk_lib_dir)
    images = load_retail_payloads(args.exe_dir)
    functions = load_retail_functions(args.functions)
    rows = classify_hits(find_hits(corpus, functions, images))

    archive_set = hashlib.sha256("".join(sorted(
        {item.library_sha256 for item in corpus}
    )).encode()).hexdigest()
    banner = (
        "Generated Psy-Q Release 2.5 masked function-ID evidence; review before promotion.",
        f"archive_set_sha256={archive_set}",
        f"signatures={len(corpus)} admitted_functions={len(functions)} "
        f"min_size={MIN_SIZE} min_fixed_bits={MIN_FIXED_BITS} "
        f"high_size={HIGH_SIZE} high_fixed_bits={HIGH_FIXED_BITS}",
    )
    write_tsv(args.corpus_output, CORPUS_FIELDS, corpus_rows(corpus), banner)
    write_tsv(args.output, MATCH_FIELDS, rows, banner)
    tiers = Counter(row["confidence"] for row in rows)
    print(
        f"[fid-census] {len(corpus)} signatures; {len(rows)} retail hits "
        f"{dict(sorted(tiers.items()))}; wrote {args.output}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
