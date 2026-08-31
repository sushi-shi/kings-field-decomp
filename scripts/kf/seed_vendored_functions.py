"""Propose Sony/Psy-Q ownership for functions in the retail census.

The structural function list remains the authority for starts and WIP extents.
This command layers three independent kinds of library evidence over it:

* relocation-masked exact Psy-Q Release 2.5 object-section matches, with XDEF
  names read from the matching archive members; and
* project-built, function-level Release 2.5 FIDs derived from every supplied
  archive member; and
* the wildcarded Psy-Q 2.60 object signatures bundled by ``ghidra_psx_ldr``.

Release 2.5 matches establish the actual linked input.  The later signatures
are deliberately marked as version-skewed candidates.  Output goes below
``build`` by default and never overwrites the hand-owned retail list.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    IMAGE_ORDER,
    parse_int,
    parse_psx_exe,
    read_tsv,
    write_tsv,
)


VENDORED_FUNCTION_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "aliases",
    "provider",
    "library",
    "module",
    "member_offset",
    "source_version",
    "evidence",
    "confidence",
    "provenance",
    "note",
)

OBJECT_EVIDENCE_FIELDS = (
    "library",
    "module",
    "block_size",
    "compared_bits",
    "relocations_masked",
    "image",
    "image_occurrences",
    "archive_candidate_count",
    "archive_candidates",
    "match_confidence",
    "ram_va",
)

COMPLETE_OBJECT_FIELDS = (
    "image",
    "va",
    "object_size",
    "library",
    "module",
    "name",
    "note",
)

SECTION_RE = re.compile(r"Section symbol number ([0-9a-f]+) '([^']+)'")
SWITCH_RE = re.compile(r"Switch to section ([0-9a-f]+)")
CODE_RE = re.compile(r"Code ([0-9]+) bytes")
HEX_RE = re.compile(r"^([0-9a-f]{4,}): ((?:[0-9a-f]{2}(?: |$))+)$")
PATCH_RE = re.compile(r"Patch type ([0-9]+) at offset ([0-9a-f]+)")
XDEF_RE = re.compile(
    r"XDEF symbol number [0-9a-f]+ '([^']+)' at offset ([0-9a-f]+) "
    r"in section ([0-9a-f]+)"
)
AUTOMATIC_LABEL_RE = re.compile(
    r"^(?:loc|text|lab|fun|sub|byte|word|dword|unk)_[0-9a-f]+$", re.IGNORECASE
)


@dataclass(frozen=True)
class Function:
    image: str
    va: int
    size: int
    census_name: str
    census_labels: tuple[str, ...]

    @property
    def end(self) -> int:
        return self.va + self.size


@dataclass(frozen=True)
class ObjectSymbols:
    text_data: bytes
    text_mask: bytes
    relocation_count: int
    xdefs: tuple[tuple[int, str], ...]

    @property
    def text_size(self) -> int:
        return len(self.text_data)

    @property
    def compared_bits(self) -> int:
        return sum(mask.bit_count() for mask in self.text_mask)


@dataclass
class ObjectRecord:
    section: int
    data: bytes
    mask: bytearray
    relocations: int = 0

    @classmethod
    def create(cls, section: int, data: bytes) -> "ObjectRecord":
        return cls(section, data, bytearray(b"\xff" * len(data)))

    def mask_patch(self, kind: int, offset: int) -> None:
        if not 0 <= offset <= len(self.data) - 4:
            raise ValueError(
                f"relocation {kind} at {offset:#x} exceeds {len(self.data)}-byte record"
            )
        masks = {
            74: (0x00, 0x00, 0x00, 0xFC),  # R_MIPS_26
            82: (0x00, 0x00, 0xFF, 0xFF),  # R_MIPS_HI16
            84: (0x00, 0x00, 0xFF, 0xFF),  # R_MIPS_LO16
        }.get(kind, (0x00, 0x00, 0x00, 0x00))
        for index, mask in enumerate(masks, offset):
            self.mask[index] &= mask
        self.relocations += 1


@dataclass(frozen=True)
class SignaturePattern:
    library: str
    module: str
    data: bytes
    mask: bytes
    labels: tuple[tuple[int, str], ...]

    @property
    def size(self) -> int:
        return len(self.data)

    @property
    def fixed_bytes(self) -> int:
        return self.mask.count(0xFF)

    @property
    def identity(self) -> tuple[bytes, bytes]:
        return self.data, self.mask


@dataclass(frozen=True)
class SignatureHit:
    image: str
    va: int
    patterns: tuple[SignaturePattern, ...]

    @property
    def size(self) -> int:
        return self.patterns[0].size

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def fixed_bytes(self) -> int:
        return self.patterns[0].fixed_bytes


def split_names(value: str) -> tuple[str, ...]:
    return tuple(part for part in value.split(";") if part)


def load_functions(path: Path) -> list[Function]:
    _fields, rows = read_tsv(path)
    return [
        Function(
            row["image"],
            parse_int(row["va"]),
            parse_int(row["size"]),
            row["name"],
            split_names(row["labels"]),
        )
        for row in rows
    ]


def run_psyk(psyk: Path, *arguments: str, cwd: Path | None = None) -> str:
    result = subprocess.run(
        (str(psyk), *arguments),
        cwd=cwd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return result.stdout


def parse_object_symbols(listing: str) -> ObjectSymbols:
    """Read joined .text bytes, relocation masks, and XDEFs from psy-k."""
    sections: dict[int, str] = {}
    xdefs: list[tuple[int, int, str]] = []
    records: list[ObjectRecord] = []
    current_section: int | None = None
    pending_size: int | None = None
    pending_hex = bytearray()

    def finish_code() -> None:
        nonlocal pending_size, pending_hex
        if pending_size is None:
            return
        if current_section is None:
            raise ValueError("psy-k listing has code before a section switch")
        if len(pending_hex) != pending_size:
            raise ValueError(
                f"psy-k listed {pending_size} bytes but emitted "
                f"{len(pending_hex)} hexdump bytes"
            )
        records.append(ObjectRecord.create(current_section, bytes(pending_hex)))
        pending_size = None
        pending_hex = bytearray()

    for raw_line in listing.splitlines():
        line = raw_line.strip()
        if match := SECTION_RE.search(line):
            sections[int(match.group(1), 16)] = match.group(2)
        elif match := XDEF_RE.search(line):
            xdefs.append(
                (int(match.group(3), 16), int(match.group(2), 16), match.group(1))
            )
        elif match := SWITCH_RE.search(line):
            finish_code()
            current_section = int(match.group(1), 16)
        elif match := CODE_RE.search(line):
            finish_code()
            pending_size = int(match.group(1))
        elif pending_size is not None and (match := HEX_RE.fullmatch(line)):
            offset = int(match.group(1), 16)
            if offset != len(pending_hex):
                raise ValueError(f"non-contiguous psy-k hexdump at {offset:#x}")
            pending_hex.extend(bytes.fromhex(match.group(2)))
        elif match := PATCH_RE.search(line):
            finish_code()
            if not records:
                raise ValueError("relocation precedes the first code record")
            records[-1].mask_patch(int(match.group(1)), int(match.group(2), 16))
    finish_code()

    text_ids = {section for section, name in sections.items() if name == ".text"}
    if len(text_ids) != 1:
        raise ValueError(f"object has {len(text_ids)} .text sections")
    text_id = next(iter(text_ids))
    text_records = [record for record in records if record.section == text_id]
    return ObjectSymbols(
        b"".join(record.data for record in text_records),
        b"".join(bytes(record.mask) for record in text_records),
        sum(record.relocations for record in text_records),
        tuple(sorted((offset, name) for section, offset, name in xdefs
                     if section == text_id)),
    )


def load_release25_symbols(
    psyk: Path,
    sdk_lib_dir: Path,
    wanted: Iterable[tuple[str, str]],
) -> dict[tuple[str, str], ObjectSymbols]:
    by_library: dict[str, set[str]] = {}
    for library, module in wanted:
        by_library.setdefault(library.upper(), set()).add(module.upper())

    result: dict[tuple[str, str], ObjectSymbols] = {}
    for library, modules in sorted(by_library.items()):
        archive = sdk_lib_dir / library
        if not archive.is_file():
            raise FileNotFoundError(f"missing Psy-Q library: {archive}")
        with tempfile.TemporaryDirectory(prefix="kf-vendored-") as directory:
            root = Path(directory)
            run_psyk(psyk, "extract", str(archive.resolve()), cwd=root)
            objects = {path.stem.upper(): path for path in root.glob("*.OBJ")}
            for module in sorted(modules):
                try:
                    obj = objects[module]
                except KeyError as error:
                    raise FileNotFoundError(
                        f"{archive}: missing member {module}.OBJ"
                    ) from error
                listing = run_psyk(psyk, "list", "--code", str(obj))
                result[(library, module)] = parse_object_symbols(listing)
    return result


def parse_archive_candidate(value: str) -> tuple[str, str]:
    parts = value.split("/")
    if len(parts) != 3 or parts[2] != ".text":
        raise ValueError(f"invalid object candidate {value!r}")
    return parts[0].upper(), parts[1].upper()


def _choose_names(function: Function, names: Iterable[str]) -> tuple[str, str]:
    candidates = sorted({name for name in names if name and not AUTOMATIC_LABEL_RE.match(name)})
    preferred = [function.census_name, *function.census_labels]
    primary = next((name for name in preferred if name in candidates), "")
    if not primary and candidates:
        primary = candidates[0]
    aliases = ";".join(name for name in candidates if name != primary)
    return primary, aliases


def exact_object_rows(
    functions: Sequence[Function],
    evidence_path: Path,
    symbols: dict[tuple[str, str], ObjectSymbols],
    images: dict[str, tuple[int, bytes]],
) -> dict[tuple[str, int], dict[str, object]]:
    fields, evidence = read_tsv(evidence_path)
    if fields != OBJECT_EVIDENCE_FIELDS:
        raise ValueError(
            f"{evidence_path}: fields {fields!r}, expected {OBJECT_EVIDENCE_FIELDS!r}"
        )

    for evidence_row in evidence:
        candidate = (
            evidence_row["library"].upper(), evidence_row["module"].upper()
        )
        obj = symbols[candidate]
        if obj.text_size != int(evidence_row["block_size"]):
            raise ValueError(f"{evidence_path}: wrong size for {candidate!r}")
        if obj.compared_bits != int(evidence_row["compared_bits"]):
            raise ValueError(f"{evidence_path}: wrong compared-bit count for {candidate!r}")
        if obj.relocation_count != int(evidence_row["relocations_masked"]):
            raise ValueError(f"{evidence_path}: wrong relocation count for {candidate!r}")
        load_address, payload = images[evidence_row["image"]]
        offsets = find_masked_bytes(obj.text_data, obj.text_mask, payload)
        expected_offset = parse_int(evidence_row["ram_va"]) - load_address
        if expected_offset not in offsets:
            raise ValueError(
                f"{evidence_path}: {candidate!r} no longer matches "
                f"{evidence_row['image']} at {evidence_row['ram_va']}"
            )
        if len(offsets) != int(evidence_row["image_occurrences"]):
            raise ValueError(
                f"{evidence_path}: occurrence count changed for {candidate!r} "
                f"in {evidence_row['image']}"
            )
        archive_candidates = evidence_row["archive_candidates"].split(",")
        if len(archive_candidates) != int(evidence_row["archive_candidate_count"]):
            raise ValueError(
                f"{evidence_path}: archive candidate count is inconsistent"
            )
        if candidate not in map(parse_archive_candidate, archive_candidates):
            raise ValueError(
                f"{evidence_path}: row member is absent from archive candidates"
            )

    grouped: dict[tuple[str, int, int, str], list[dict[str, str]]] = {}
    for row in evidence:
        key = (
            row["image"],
            parse_int(row["ram_va"]),
            int(row["block_size"]),
            row["archive_candidates"],
        )
        grouped.setdefault(key, []).append(row)

    rows: dict[tuple[str, int], dict[str, object]] = {}
    for (image, start, block_size, candidate_text), matches in grouped.items():
        candidates = tuple(
            parse_archive_candidate(value) for value in candidate_text.split(",")
        )
        for candidate in candidates:
            obj = symbols[candidate]
            if obj.text_size != block_size:
                raise ValueError(
                    f"{candidate[0]}/{candidate[1]} .text is {obj.text_size:#x}, "
                    f"evidence says {block_size:#x}"
                )
        end = start + block_size
        match_confidence = matches[0]["match_confidence"]
        confidence = {
            "strong_unique": "exact-release25",
            "short_unique": "exact-release25-short",
            "archive_ambiguous": "exact-release25-ambiguous",
        }[match_confidence]
        compared_bits = int(matches[0]["compared_bits"])
        relocations = int(matches[0]["relocations_masked"])
        for function in functions:
            if function.image != image or not (start <= function.va and function.end <= end):
                continue
            member_offset = function.va - start
            exact_names = [
                name
                for candidate in candidates
                for offset, name in symbols[candidate].xdefs
                if offset == member_offset
            ]
            name, aliases = _choose_names(function, exact_names)
            key = (image, function.va)
            if key in rows:
                raise ValueError(f"overlapping exact object ownership at {key!r}")
            rows[key] = {
                "image": image,
                "va": f"0x{function.va:08x}",
                "size": f"0x{function.size:x}",
                "name": name,
                "aliases": aliases,
                "provider": "Sony Computer Entertainment",
                "library": "|".join(sorted({candidate[0] for candidate in candidates})),
                "module": "|".join(sorted({candidate[1] for candidate in candidates})),
                "member_offset": f"0x{member_offset:x}",
                "source_version": "Psy-Q Release 2.5",
                "evidence": "relocation-masked-object-section",
                "confidence": confidence,
                "provenance": (
                    "psyq_release_25_text_sections.tsv;psy-k-0.4.0-XDEF"
                ),
                "note": (
                    f"function extent lies within exact {block_size:#x}-byte .text "
                    f"match ({compared_bits} compared bits; {relocations} relocations masked)"
                ),
            }
    return rows


def apply_complete_object_rows(
    functions: Sequence[Function],
    evidence_path: Path,
    symbols: dict[tuple[str, str], ObjectSymbols],
    images: dict[str, tuple[int, bytes]],
    rows: dict[tuple[str, int], dict[str, object]],
) -> None:
    fields, evidence = read_tsv(evidence_path)
    if fields != COMPLETE_OBJECT_FIELDS:
        raise ValueError(
            f"{evidence_path}: fields {fields!r}, expected {COMPLETE_OBJECT_FIELDS!r}"
        )
    starts = {(function.image, function.va): function for function in functions}
    for evidence_row in evidence:
        key = (evidence_row["image"], parse_int(evidence_row["va"]))
        try:
            function = starts[key]
        except KeyError as error:
            raise ValueError(f"{evidence_path}: no structural function at {key!r}") from error
        library = evidence_row["library"].upper()
        module = evidence_row["module"].upper()
        object_size = parse_int(evidence_row["object_size"])
        obj = symbols[(library, module)]
        if obj.text_size != object_size:
            raise ValueError(
                f"{library}/{module} .text is {obj.text_size:#x}, "
                f"evidence says {object_size:#x}"
            )
        load_address, payload = images[function.image]
        image_offset = function.va - load_address
        linked_bytes = payload[image_offset:image_offset + object_size]
        if linked_bytes != obj.text_data:
            raise ValueError(
                f"{evidence_path}: complete bytes differ at "
                f"{function.image} {function.va:#x}"
            )
        xdef_names = [name for offset, name in obj.xdefs if offset == 0]
        if evidence_row["name"] not in xdef_names:
            raise ValueError(
                f"{evidence_path}: {evidence_row['name']!r} is not an offset-zero "
                f"XDEF of {library}/{module}"
            )
        name, aliases = _choose_names(function, xdef_names)
        if key in rows:
            raise ValueError(f"duplicate complete-object ownership at {key!r}")
        rows[key] = {
            "image": function.image,
            "va": f"0x{function.va:08x}",
            "size": f"0x{function.size:x}",
            "name": name,
            "aliases": aliases,
            "provider": "Sony Computer Entertainment",
            "library": library,
            "module": module,
            "member_offset": "0x0",
            "source_version": "Psy-Q Release 2.5",
            "evidence": "complete-object-byte-match",
            "confidence": "exact-release25-complete",
            "provenance": "psyq_release_25_complete_objects.tsv;psy-k-0.4.0-XDEF",
            "note": evidence_row["note"],
        }


def parse_signature(signature: str) -> tuple[bytes, bytes]:
    data = bytearray()
    mask = bytearray()
    for token in signature.split():
        if token == "??":
            data.append(0)
            mask.append(0)
        elif re.fullmatch(r"[0-9A-Fa-f]{2}", token):
            data.append(int(token, 16))
            mask.append(0xFF)
        else:
            raise ValueError(f"invalid signature token {token!r}")
    return bytes(data), bytes(mask)


def load_signatures(directory: Path) -> list[SignaturePattern]:
    patterns: list[SignaturePattern] = []
    for path in sorted(directory.glob("*.json")):
        library = path.name.removesuffix(".json").upper()
        records = json.loads(path.read_text(encoding="utf-8"))
        for record in records:
            if "sig" not in record:
                continue
            data, mask = parse_signature(record["sig"])
            patterns.append(SignaturePattern(
                library,
                str(record["name"]).upper(),
                data,
                mask,
                tuple(
                    (int(label["offset"]), str(label["name"]))
                    for label in record.get("labels", [])
                ),
            ))
    return patterns


def longest_anchor(pattern: SignaturePattern) -> tuple[int, bytes] | None:
    best_start = best_end = start = 0
    for position, mask in enumerate(pattern.mask):
        if mask != 0xFF:
            if position - start > best_end - best_start:
                best_start, best_end = start, position
            start = position + 1
    if pattern.size - start > best_end - best_start:
        best_start, best_end = start, pattern.size
    if best_end - best_start < 8:
        return None
    return best_start, pattern.data[best_start:best_end]


def find_signature(pattern: SignaturePattern, payload: bytes) -> list[int]:
    anchor = longest_anchor(pattern)
    if anchor is None:
        return []
    anchor_offset, needle = anchor
    matches: list[int] = []
    cursor = 0
    while True:
        found = payload.find(needle, cursor)
        if found < 0:
            return matches
        start = found - anchor_offset
        if 0 <= start <= len(payload) - pattern.size:
            candidate = payload[start:start + pattern.size]
            if all(
                not mask or actual == expected
                for actual, expected, mask in zip(
                    candidate, pattern.data, pattern.mask, strict=True
                )
            ):
                matches.append(start)
        cursor = found + 1


def find_masked_bytes(data: bytes, mask: bytes, payload: bytes) -> list[int]:
    if len(data) != len(mask):
        raise ValueError("masked byte signature has unequal data/mask lengths")
    best_start = best_end = start = 0
    for position, value in enumerate(mask):
        if value != 0xFF:
            if position - start > best_end - best_start:
                best_start, best_end = start, position
            start = position + 1
    if len(mask) - start > best_end - best_start:
        best_start, best_end = start, len(mask)
    if best_end - best_start < 8:
        raise ValueError("masked object has no eight-byte exact anchor")

    needle = data[best_start:best_end]
    matches: list[int] = []
    cursor = 0
    while True:
        found = payload.find(needle, cursor)
        if found < 0:
            return matches
        candidate_start = found - best_start
        if 0 <= candidate_start <= len(payload) - len(data):
            candidate = payload[candidate_start:candidate_start + len(data)]
            if all(
                (actual & compared) == (expected & compared)
                for actual, expected, compared in zip(candidate, data, mask, strict=True)
            ):
                matches.append(candidate_start)
        cursor = found + 1


def read_payload(path: Path) -> tuple[int, bytes]:
    data = path.read_bytes()
    if len(data) < 0x800 or data[:8] != b"PS-X EXE":
        raise ValueError(f"{path}: not a PS-X EXE")
    load_address = int.from_bytes(data[0x18:0x1c], "little")
    load_size = int.from_bytes(data[0x1c:0x20], "little")
    if len(data) < 0x800 + load_size:
        raise ValueError(f"{path}: truncated load image")
    return load_address, data[0x800:0x800 + load_size]


def load_retail_payloads(exe_dir: Path) -> dict[str, tuple[int, bytes]]:
    images: dict[str, tuple[int, bytes]] = {}
    for image, expected in IMAGE_LAYOUTS.items():
        path = exe_dir / image
        if parse_psx_exe(path) != expected:
            raise ValueError(f"{path}: retail identity/layout mismatch")
        images[image] = read_payload(path)
    return images


def signature_hits(
    patterns: Sequence[SignaturePattern],
    images: dict[str, tuple[int, bytes]],
    minimum_fixed_bytes: int = 12,
) -> list[SignatureHit]:
    grouped_patterns: dict[tuple[bytes, bytes], list[SignaturePattern]] = {}
    for pattern in patterns:
        if pattern.size >= 16 and pattern.fixed_bytes >= minimum_fixed_bytes:
            grouped_patterns.setdefault(pattern.identity, []).append(pattern)

    hits: list[SignatureHit] = []
    for image in IMAGE_LAYOUTS:
        load_address, payload = images[image]
        for candidates in grouped_patterns.values():
            pattern = candidates[0]
            offsets = find_signature(pattern, payload)
            if len(offsets) != 1:
                continue
            offset = offsets[0]
            if offset & 3:
                continue
            hits.append(SignatureHit(
                image,
                load_address + offset,
                tuple(sorted(candidates, key=lambda item: (item.library, item.module))),
            ))
    return hits


def _signature_names(hit: SignatureHit, function: Function) -> set[str]:
    relative = function.va - hit.va
    return {
        name
        for pattern in hit.patterns
        for offset, name in pattern.labels
        if offset == relative and not AUTOMATIC_LABEL_RE.match(name)
    }


def apply_signature_rows(
    functions: Sequence[Function],
    hits: Sequence[SignatureHit],
    rows: dict[tuple[str, int], dict[str, object]],
) -> None:
    starts = {(function.image, function.va): function for function in functions}
    observations: dict[tuple[str, int], list[tuple[SignatureHit, set[str]]]] = {}
    for hit in hits:
        named_starts = {
            hit.va + offset
            for pattern in hit.patterns
            for offset, name in pattern.labels
            if not AUTOMATIC_LABEL_RE.match(name)
            and (hit.image, hit.va + offset) in starts
        }
        # A short byte pattern found in the middle of game code is not enough.
        # At least one meaningful signature label must land on an admitted start.
        if not named_starts:
            continue
        for function in functions:
            if function.image != hit.image:
                continue
            names = _signature_names(hit, function)
            wholly_contained = hit.va <= function.va and function.end <= hit.end
            if not names and not wholly_contained:
                continue
            observations.setdefault((function.image, function.va), []).append(
                (hit, names)
            )

    for key, candidates in observations.items():
        function = starts[key]
        hit, names = max(
            candidates,
            key=lambda item: (
                bool(item[1]), item[0].fixed_bytes, item[0].size,
                -item[0].va,
            ),
        )
        if key in rows:
            row = rows[key]
            if not row["name"] and names:
                name, aliases = _choose_names(function, names)
                row["name"] = name
                row["aliases"] = aliases
                row["provenance"] = str(row["provenance"]) + ";ghidra_psx_ldr-psyq260-label"
                row["note"] = str(row["note"]) + "; name corroborated by 2.60 signature"
            continue

        name, aliases = _choose_names(function, names)
        libraries = sorted({pattern.library for pattern in hit.patterns})
        modules = sorted({pattern.module.removesuffix(".OBJ") for pattern in hit.patterns})
        ambiguous = len(hit.patterns) > 1
        rows[key] = {
            "image": function.image,
            "va": f"0x{function.va:08x}",
            "size": f"0x{function.size:x}",
            "name": name,
            "aliases": aliases,
            "provider": "Sony Computer Entertainment",
            "library": "|".join(libraries),
            "module": "|".join(modules),
            "member_offset": f"0x{function.va - hit.va:x}",
            "source_version": "Psy-Q 2.60 signature corpus",
            "evidence": "wildcard-object-signature",
            "confidence": (
                "psyq260-signature-ambiguous" if ambiguous else "psyq260-signature"
            ),
            "provenance": "ghidra_psx_ldr-2026.07.08/psyq/260",
            "note": (
                f"unique image occurrence; {hit.fixed_bytes}/{hit.size} bytes fixed"
                + (f"; {len(hit.patterns)} archive candidates" if ambiguous else "")
            ),
        }


def apply_fid_rows(
    fid_rows: Sequence[dict[str, str]],
    rows: dict[tuple[str, int], dict[str, object]],
) -> None:
    """Promote strong project-built function IDs.

    Exact containing-object matches remain the primary row when both channels
    agree; the function ID is appended as independent provenance.  HIGH IDs are
    admitted directly.  A repeated AMBIG ID is admitted only when its entire
    body is fixed and every candidate has the same normalized function name;
    this preserves possible SDK member identities without confusing them with
    conflicting short-wrapper signatures.
    """
    for fid in fid_rows:
        if fid["confidence"] != "HIGH":
            continue
        key = (fid["image"], parse_int(fid["va"]))
        witness = (
            f"psyq-release25-fid:{fid['fid_sha256'][:12]}:"
            f"{fid['object_sha256'][:12]}"
        )
        if key in rows:
            row = rows[key]
            if (
                fid["library"] not in str(row["library"]).split("|")
                or fid["module"] not in str(row["module"]).split("|")
            ):
                continue
            provenance = str(row["provenance"])
            if witness not in provenance.split(";"):
                row["provenance"] = provenance + ";" + witness
                row["note"] = (
                    str(row["note"])
                    + f"; corroborated by unique {fid['boundary']} function FID"
                )
            continue

        rows[key] = {
            "image": fid["image"],
            "va": fid["va"],
            "size": fid["size"],
            "name": fid["name"],
            "aliases": fid["aliases"],
            "provider": "Sony Computer Entertainment",
            "library": fid["library"],
            "module": fid["module"],
            "member_offset": fid["member_offset"],
            "source_version": "Psy-Q Release 2.5",
            "evidence": "relocation-masked-function-id",
            "confidence": "fid-release25",
            "provenance": witness,
            "note": (
                f"unique substantial {fid['boundary']} FID; "
                f"{fid['fixed_bits']} fixed instruction bits"
            ),
        }

    ambiguous: dict[tuple[str, int], list[dict[str, str]]] = {}
    for fid in fid_rows:
        if fid["confidence"] != "AMBIG":
            continue
        key = (fid["image"], parse_int(fid["va"]))
        ambiguous.setdefault(key, []).append(fid)

    for key, candidates in ambiguous.items():
        if key in rows:
            continue
        sizes = {parse_int(fid["size"]) for fid in candidates}
        hashes = {fid["fid_sha256"] for fid in candidates}
        names = {
            name
            for fid in candidates
            for name in (fid["name"], *fid["aliases"].split(";"))
            if name
        }
        normalized_names = {name.removeprefix("_") for name in names}
        if (
            len(sizes) != 1
            or next(iter(sizes)) < 0x20
            or len(hashes) != 1
            or len(normalized_names) != 1
            or any(
                int(fid["fixed_bits"]) != parse_int(fid["size"]) * 8
                for fid in candidates
            )
        ):
            continue

        size = next(iter(sizes))
        normalized_name = next(iter(normalized_names))
        name = normalized_name if normalized_name in names else sorted(names)[0]
        identities = sorted({
            (
                fid["library"], fid["module"], fid["member_offset"],
                fid["object_sha256"],
            )
            for fid in candidates
        })
        witnesses = sorted({
            f"psyq-release25-fid:{fid['fid_sha256'][:12]}:"
            f"{fid['object_sha256'][:12]}"
            for fid in candidates
        })
        mappings = ", ".join(
            f"{library}/{module}@{offset}"
            for library, module, offset, _object_hash in identities
        )
        offsets = {offset for _library, _module, offset, _hash in identities}
        rows[key] = {
            "image": candidates[0]["image"],
            "va": candidates[0]["va"],
            "size": f"0x{size:x}",
            "name": name,
            "aliases": ";".join(sorted(names - {name})),
            "provider": "Sony Computer Entertainment",
            "library": "|".join(sorted({item[0] for item in identities})),
            "module": "|".join(sorted({item[1] for item in identities})),
            "member_offset": next(iter(offsets)) if len(offsets) == 1 else "0x0",
            "source_version": "Psy-Q Release 2.5",
            "evidence": "exact-function-id-multiobject",
            "confidence": "fid-release25-ambiguous",
            "provenance": ";".join(witnesses),
            "note": (
                f"fully fixed {size:#x}-byte FID shared by {len(identities)} "
                "SDK identities; candidate-specific offsets: " + mappings
            ),
        }


def signature_directory_from_environment() -> Path | None:
    if root := os.environ.get("GHIDRA_PSX_LOADER"):
        candidate = Path(root) / "data" / "psyq" / "260"
        if candidate.is_dir():
            return candidate
    if root := os.environ.get("NIX_GHIDRAHOME"):
        candidate = (
            Path(root) / "Extensions" / "ghidra_psx_ldr" / "data" / "psyq" / "260"
        )
        if candidate.is_dir():
            return candidate
    return None


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
    parser.add_argument(
        "--object-evidence",
        type=Path,
        default=Path("config/evidence/psyq_release_25_text_sections.tsv"),
    )
    parser.add_argument(
        "--complete-object-evidence",
        type=Path,
        default=Path("config/evidence/psyq_release_25_complete_objects.tsv"),
    )
    parser.add_argument("--psyk", type=Path)
    parser.add_argument(
        "--sdk-lib-dir",
        type=Path,
        default=Path(os.environ["PSYQ_LIB"]) if "PSYQ_LIB" in os.environ else None,
    )
    parser.add_argument("--exe-dir", type=Path)
    parser.add_argument("--signature-dir", type=Path, default=signature_directory_from_environment())
    parser.add_argument("--no-fids", action="store_true")
    parser.add_argument("--no-signatures", action="store_true")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("build/vendored-seed/functions_vendored.tsv"),
    )
    return parser.parse_args(arguments)


def main(arguments: Sequence[str] | None = None) -> int:
    args = parse_args(arguments)
    if args.sdk_lib_dir is None:
        raise SystemExit("Psy-Q library directory missing; enter nix develop or pass --sdk-lib-dir")
    if args.exe_dir is None:
        raise SystemExit("--exe-dir is required to verify object and signature matches")
    try:
        args.output.resolve().relative_to(Path("config/retail").resolve())
    except ValueError:
        pass
    else:
        raise SystemExit("refusing to overwrite hand-owned config/retail; seed below build first")

    functions = load_functions(args.functions)
    _fields, evidence = read_tsv(args.object_evidence)
    _complete_fields, complete_evidence = read_tsv(args.complete_object_evidence)
    wanted = {
        parse_archive_candidate(candidate)
        for row in evidence
        for candidate in row["archive_candidates"].split(",")
    }
    wanted.update(
        (row["library"].upper(), row["module"].upper())
        for row in complete_evidence
    )
    symbols = load_release25_symbols(
        resolve_tool(args.psyk, "psyk"), args.sdk_lib_dir, wanted
    )
    images = load_retail_payloads(args.exe_dir)
    rows = exact_object_rows(functions, args.object_evidence, symbols, images)
    apply_complete_object_rows(
        functions, args.complete_object_evidence, symbols, images, rows
    )

    if not args.no_fids:
        # Kept local to avoid making the standalone FID tool depend on this
        # command's CLI initialization path.
        from scripts.kf.fid_census import (
            build_corpus,
            classify_hits,
            find_hits,
            load_retail_functions,
        )

        corpus = build_corpus(resolve_tool(args.psyk, "psyk"), args.sdk_lib_dir)
        fid_rows = classify_hits(find_hits(
            corpus, load_retail_functions(args.functions), images
        ))
        apply_fid_rows(fid_rows, rows)

    if not args.no_signatures:
        if args.signature_dir is None:
            raise SystemExit(
                "signature matching needs --signature-dir "
                "(automatic inside nix develop)"
            )
        patterns = load_signatures(args.signature_dir)
        apply_signature_rows(functions, signature_hits(patterns, images), rows)

    ordered = sorted(
        rows.values(),
        key=lambda row: (IMAGE_ORDER[str(row["image"])], parse_int(str(row["va"]))),
    )
    write_tsv(
        args.output,
        VENDORED_FUNCTION_FIELDS,
        ordered,
        (
            "Candidate Sony/Psy-Q ownership layered over config/retail/functions.tsv.",
            "Release 2.5 exact objects and project-built FIDs outrank version-skewed Psy-Q 2.60 signatures.",
            "Review before mechanically admitting changes to config/retail/functions_vendored.tsv.",
        ),
    )
    by_confidence: dict[str, int] = {}
    for row in ordered:
        confidence = str(row["confidence"])
        by_confidence[confidence] = by_confidence.get(confidence, 0) + 1
    summary = ", ".join(f"{name}={count}" for name, count in sorted(by_confidence.items()))
    print(f"wrote {len(ordered)} vendored function candidates to {args.output} ({summary})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
