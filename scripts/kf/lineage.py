"""Verify cross-overlay function order and SDK archive lineage evidence."""

from __future__ import annotations

import argparse
import json
import os
import struct
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import REPO, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, parse_psx_exe, read_tsv


EVIDENCE = REPO / "config/evidence/overlay_lineage.tsv"
SSCALL = {
    "GAME.EXE": (0x8004A52C, 0x8004A55C),
    "OPEN.EXE": (0x8002A300, 0x8002A330),
}
FIELDS = (
    "lineage",
    "left_image",
    "left_va",
    "right_image",
    "right_va",
    "size",
    "address_delta",
    "exact_words",
    "total_words",
    "shape_words",
    "archive_library",
    "archive_module",
    "archive_symbol",
    "note",
)


@dataclass(frozen=True)
class LineageRow:
    lineage: str
    left_image: str
    left_va: int
    right_image: str
    right_va: int
    size: int
    address_delta: int
    exact_words: int
    total_words: int
    shape_words: int
    archive_library: str
    archive_module: str
    archive_symbol: str


@dataclass(frozen=True)
class LineageResult:
    lineage: str
    rows: int
    bytes: int
    exact_words: int
    total_words: int
    shape_words: int
    address_delta: int


def instruction_shape(word: int) -> int:
    """Keep MIPS opcode/register shape while removing linked immediates."""
    opcode = word >> 26
    if opcode in {2, 3}:  # j / jal
        return word & 0xFC000000
    if opcode != 0:  # I-type: keep opcode, rs, and rt
        return word & 0xFFFF0000
    return word  # R-type and COP encodings have no linked immediate here


def load_rows(path: Path = EVIDENCE) -> tuple[LineageRow, ...]:
    fields, raw_rows = read_tsv(path)
    if fields != FIELDS:
        raise ValueError(f"{path}: fields {fields!r}, expected {FIELDS!r}")
    rows = tuple(
        LineageRow(
            lineage=row["lineage"],
            left_image=row["left_image"],
            left_va=parse_int(row["left_va"]),
            right_image=row["right_image"],
            right_va=parse_int(row["right_va"]),
            size=parse_int(row["size"]),
            address_delta=parse_int(row["address_delta"]),
            exact_words=int(row["exact_words"]),
            total_words=int(row["total_words"]),
            shape_words=int(row["shape_words"]),
            archive_library=row["archive_library"],
            archive_module=row["archive_module"],
            archive_symbol=row["archive_symbol"],
        )
        for row in raw_rows
    )
    if not rows:
        raise ValueError(f"{path}: no lineage rows")
    return rows


def verify_inventory(
    rows: Iterable[LineageRow],
    functions_path: Path = RETAIL_CONFIG / "functions.tsv",
) -> None:
    _fields, function_rows = read_tsv(functions_path)
    functions = {
        (row["image"], parse_int(row["va"])): parse_int(row["size"])
        for row in function_rows
    }
    groups: dict[str, list[LineageRow]] = defaultdict(list)
    for row in rows:
        groups[row.lineage].append(row)
        if row.left_image not in IMAGE_LAYOUTS or row.right_image not in IMAGE_LAYOUTS:
            raise ValueError(f"{row.lineage}: unknown image")
        if row.size <= 0 or row.size & 3:
            raise ValueError(f"{row.lineage}: invalid size {row.size:#x}")
        if row.left_va - row.right_va != row.address_delta:
            raise ValueError(f"{row.lineage}: incorrect address delta at {row.left_va:#x}")
        for identity in (
            (row.left_image, row.left_va),
            (row.right_image, row.right_va),
        ):
            if functions.get(identity) != row.size:
                raise ValueError(
                    f"{row.lineage}: function inventory differs at "
                    f"{identity[0]} {identity[1]:#x}"
                )
        if row.total_words != row.size // 4:
            raise ValueError(f"{row.lineage}: word count differs at {row.left_va:#x}")
        if not 0 <= row.exact_words <= row.shape_words <= row.total_words:
            raise ValueError(f"{row.lineage}: invalid match counts at {row.left_va:#x}")

    for name, group in groups.items():
        deltas = {row.address_delta for row in group}
        if len(deltas) != 1:
            raise ValueError(f"{name}: address delta is not constant")
        ordered = sorted(group, key=lambda row: row.left_va)
        for previous, current in zip(ordered, ordered[1:]):
            if previous.left_va + previous.size != current.left_va:
                raise ValueError(f"{name}: left-image function order is not contiguous")
            if previous.right_va + previous.size != current.right_va:
                raise ValueError(f"{name}: right-image function order is not contiguous")


def _load_payload(exe_dir: Path, image: str) -> bytes:
    path = exe_dir / image
    if parse_psx_exe(path) != IMAGE_LAYOUTS[image]:
        raise ValueError(f"{path}: retail identity/layout mismatch")
    return path.read_bytes()


def _words(payload: bytes, image: str, va: int, size: int) -> tuple[int, ...]:
    layout = IMAGE_LAYOUTS[image]
    start = layout.file_offset(va)
    data = payload[start:start + size]
    if len(data) != size:
        raise ValueError(f"{image}: truncated lineage range {va:#x}+{size:#x}")
    return struct.unpack(f"<{size // 4}I", data)


def verify_binaries(exe_dir: Path, rows: Iterable[LineageRow]) -> tuple[LineageResult, ...]:
    rows = tuple(rows)
    images = {
        image: _load_payload(exe_dir, image)
        for row in rows
        for image in (row.left_image, row.right_image)
    }
    totals: dict[str, list[int]] = defaultdict(lambda: [0, 0, 0, 0, 0])
    deltas: dict[str, int] = {}
    for row in rows:
        left = _words(images[row.left_image], row.left_image, row.left_va, row.size)
        right = _words(images[row.right_image], row.right_image, row.right_va, row.size)
        exact = sum(a == b for a, b in zip(left, right))
        shaped = sum(instruction_shape(a) == instruction_shape(b) for a, b in zip(left, right))
        if (exact, len(left), shaped) != (
            row.exact_words,
            row.total_words,
            row.shape_words,
        ):
            raise ValueError(
                f"{row.lineage}: binary evidence differs at {row.left_va:#x}: "
                f"got {exact}/{len(left)} exact, {shaped}/{len(left)} shape"
            )
        values = totals[row.lineage]
        values[0] += 1
        values[1] += row.size
        values[2] += exact
        values[3] += len(left)
        values[4] += shaped
        deltas[row.lineage] = row.address_delta
    return tuple(
        LineageResult(name, *totals[name], deltas[name])
        for name in sorted(totals)
    )


def signature_directory_from_environment() -> Path | None:
    if root := os.environ.get("GHIDRA_PSX_LOADER"):
        candidate = Path(root) / "data/psyq/260"
        if candidate.is_dir():
            return candidate
    return None


def verify_archive_anchors(signature_dir: Path, rows: Iterable[LineageRow]) -> int:
    anchors = {
        (row.archive_library, row.archive_module, row.archive_symbol)
        for row in rows
        if row.archive_library and row.archive_module
    }
    libraries: dict[str, list[dict[str, object]]] = {}
    for library, module, symbol in sorted(anchors):
        if library not in libraries:
            path = signature_dir / f"{library}.json"
            libraries[library] = json.loads(path.read_text(encoding="utf-8"))
        objects = [item for item in libraries[library] if item.get("name") == module]
        if len(objects) != 1:
            raise ValueError(f"{library}: expected one {module}, found {len(objects)}")
        if symbol:
            labels = {item.get("name") for item in objects[0].get("labels", [])}
            if symbol not in labels:
                raise ValueError(f"{library}/{module}: missing symbol {symbol}")
    return len(anchors)


def _jal_target(pc: int, word: int) -> int | None:
    if word >> 26 != 3:
        return None
    return ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def verify_sscall_boundary_and_calls(
    exe_dir: Path,
    functions_path: Path = RETAIL_CONFIG / "functions.tsv",
    vendored_path: Path = RETAIL_CONFIG / "functions_vendored.tsv",
) -> int:
    """Check the memcpy FID, SSCALL-shaped dispatcher, and direct calls."""
    _fields, raw_functions = read_tsv(functions_path)
    functions = [
        (row["image"], parse_int(row["va"]), parse_int(row["size"]))
        for row in raw_functions
    ]
    function_sizes = {(image, va): size for image, va, size in functions}
    _fields, raw_vendored = read_tsv(vendored_path)
    vendored = {
        (row["image"], parse_int(row["va"])): row
        for row in raw_vendored
    }

    direct_calls = 0
    for image, (helper, dispatcher) in SSCALL.items():
        if function_sizes.get((image, helper)) != 0x30:
            raise ValueError(f"{image}: SSCALL helper boundary differs")
        if function_sizes.get((image, dispatcher)) != 0x2F8:
            raise ValueError(f"{image}: SsSeqCalledTbyT boundary differs")
        helper_vendor = vendored.get((image, helper), {})
        if (
            helper_vendor.get("name"),
            helper_vendor.get("evidence"),
            helper_vendor.get("confidence"),
        ) != (
            "memcpy",
            "exact-function-id-multiobject",
            "fid-release25-ambiguous",
        ):
            raise ValueError(
                f"{image}: repeated memcpy helper FID evidence differs"
            )
        if (image, dispatcher) in vendored:
            raise ValueError(f"{image}: unmatched SSCALL dispatcher must stay in scope")

        payload = _load_payload(exe_dir, image)
        prefix = _words(payload, image, dispatcher, 8)
        first, second = prefix
        if not (
            first >> 26 == 0x0F
            and (first >> 16) & 0x1F == 3
            and second >> 26 == 0x23
            and (second >> 21) & 0x1F == 3
            and (second >> 16) & 0x1F == 3
        ):
            raise ValueError(f"{image}: corrected SSCALL LUI/LW prefix differs")

        callers = []
        for function_image, va, size in functions:
            if function_image != image:
                continue
            for index, word in enumerate(_words(payload, image, va, size)):
                pc = va + index * 4
                if _jal_target(pc, word) == helper:
                    callers.append(pc)
        expected = [dispatcher + 0x3C, dispatcher + 0x2B0]
        if callers != expected:
            raise ValueError(
                f"{image}: SSCALL helper direct callers differ: "
                f"{', '.join(f'{address:#x}' for address in callers)}"
            )
        direct_calls += len(callers)
    return direct_calls


def parse_args(arguments: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--evidence", type=Path, default=EVIDENCE)
    parser.add_argument(
        "--functions", type=Path, default=RETAIL_CONFIG / "functions.tsv"
    )
    parser.add_argument(
        "--vendored", type=Path, default=RETAIL_CONFIG / "functions_vendored.tsv"
    )
    parser.add_argument("--signature-dir", type=Path)
    return parser.parse_args(arguments)


def main(arguments: Sequence[str] | None = None) -> int:
    args = parse_args(arguments)
    rows = load_rows(args.evidence)
    verify_inventory(rows, args.functions)
    signature_dir = args.signature_dir or signature_directory_from_environment()
    if signature_dir is None:
        raise ValueError("Psy-Q signature directory is unavailable")
    anchors = verify_archive_anchors(signature_dir, rows)
    exe_dir = args.retail_dir or configured_retail_dir()
    results = verify_binaries(exe_dir, rows)
    direct_calls = verify_sscall_boundary_and_calls(
        exe_dir, args.functions, args.vendored
    )
    for result in results:
        print(
            f"{result.lineage}: {result.rows} contiguous function pairs, "
            f"delta={result.address_delta:#x}, {result.exact_words}/{result.total_words} "
            f"exact words, {result.shape_words}/{result.total_words} instruction shapes"
        )
    print(f"archive anchors verified: {anchors}")
    print(
        "ambiguous-provider memcpy FIDs and unpromoted SSCALL dispatcher "
        f"verified with direct helper calls: {direct_calls}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
