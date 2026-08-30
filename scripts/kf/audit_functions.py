"""Find plausible MIPS function starts missed by the admitted retail census.

This analyzer is deliberately independent of Ghidra.  It combines compiler-like
stack frames, matching return sequences, cross-function tail jumps, and stored
code pointers.  Its TSV is review evidence; it never edits ``config/retail``.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import struct
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.retail import IMAGE_LAYOUTS, IMAGE_ORDER, parse_int, read_tsv


FIELDS = (
    "image",
    "va",
    "file_offset",
    "owner_va",
    "frame_size",
    "ra_save_offset",
    "return_va",
    "pointer_refs",
    "tail_jump_refs",
    "reloc_refs",
    "valid_first_12",
    "boundary",
    "confidence",
    "note",
)


@dataclass(frozen=True)
class FunctionExtent:
    start: int
    end: int


@dataclass(frozen=True)
class Frame:
    size: int
    ra_offset: int


def signed16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def opcode(word: int) -> int:
    return word >> 26


def rs(word: int) -> int:
    return (word >> 21) & 0x1F


def rt(word: int) -> int:
    return (word >> 16) & 0x1F


def function_target(site: int, word: int) -> int:
    return ((site + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def is_jr_ra(word: int) -> bool:
    return word == 0x03E00008


def stack_adjust(word: int) -> int | None:
    if opcode(word) != 0x09 or rs(word) != 29 or rt(word) != 29:
        return None
    return signed16(word & 0xFFFF)


def stack_store(word: int) -> tuple[int, int] | None:
    if opcode(word) != 0x2B or rs(word) != 29:
        return None
    return rt(word), signed16(word & 0xFFFF)


def valid_instruction(word: int) -> bool:
    """Conservative R3000A/GTE encoding check, sufficient for ranking bytes."""
    op = opcode(word)
    if op == 0:
        return (word & 0x3F) in {
            0x00,
            0x02,
            0x03,
            0x04,
            0x06,
            0x07,
            0x08,
            0x09,
            0x0C,
            0x0D,
            0x10,
            0x11,
            0x12,
            0x13,
            0x18,
            0x19,
            0x1A,
            0x1B,
            0x20,
            0x21,
            0x22,
            0x23,
            0x24,
            0x25,
            0x26,
            0x27,
            0x2A,
            0x2B,
        }
    if op == 1:
        return rt(word) in {0x00, 0x01, 0x10, 0x11}
    return op in {
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x07,
        0x08,
        0x09,
        0x0A,
        0x0B,
        0x0C,
        0x0D,
        0x0E,
        0x0F,
        0x10,
        0x12,
        0x20,
        0x21,
        0x22,
        0x23,
        0x24,
        0x25,
        0x26,
        0x28,
        0x29,
        0x2A,
        0x2B,
        0x2E,
        0x32,
        0x3A,
    }


class ImageWords:
    def __init__(self, path: Path, image: str):
        self.layout = IMAGE_LAYOUTS[image]
        raw = path.read_bytes()
        self.payload = raw[0x800 : 0x800 + self.layout.load_size]

    def contains_word(self, address: int) -> bool:
        return self.layout.contains(address, 4) and not (address & 3)

    def word(self, address: int) -> int:
        offset = address - self.layout.load_address
        return struct.unpack_from("<I", self.payload, offset)[0]

    def addresses(self):
        return range(self.layout.load_address, self.layout.load_end - 3, 4)


def load_extents(config_dir: Path) -> dict[str, list[FunctionExtent]]:
    _fields, rows = read_tsv(config_dir / "functions.tsv")
    result: dict[str, list[FunctionExtent]] = defaultdict(list)
    for row in rows:
        start = parse_int(row["va"])
        result[row["image"]].append(
            FunctionExtent(start, start + parse_int(row["size"]))
        )
    return {image: sorted(values, key=lambda item: item.start)
            for image, values in result.items()}


def extent_owner(extents: list[FunctionExtent], address: int) -> FunctionExtent | None:
    starts = [extent.start for extent in extents]
    index = bisect.bisect_right(starts, address) - 1
    if index >= 0 and address < extents[index].end:
        return extents[index]
    return None


def frame_at(words: ImageWords, address: int) -> Frame | None:
    adjust = stack_adjust(words.word(address))
    if adjust is None or adjust >= 0 or adjust < -0x4000 or adjust % 4:
        return None
    frame_size = -adjust
    for index in range(1, 13):
        site = address + index * 4
        if not words.contains_word(site):
            break
        word = words.word(site)
        store = stack_store(word)
        if store and store[0] == 31 and 0 <= store[1] < frame_size:
            return Frame(frame_size, store[1])
        if opcode(word) in {0x02, 0x04, 0x05, 0x06, 0x07} or is_jr_ra(word):
            break
    return None


def matching_return(
    words: ImageWords,
    address: int,
    frame: Frame,
    limit: int = 0x4000,
) -> int | None:
    end = min(words.layout.load_end - 4, address + limit)
    for site in range(address + 4, end, 4):
        if not is_jr_ra(words.word(site)) or not words.contains_word(site + 4):
            continue
        if stack_adjust(words.word(site + 4)) == frame.size:
            return site
    return None


def first_window_score(words: ImageWords, address: int, count: int = 12) -> str:
    sample = [
        words.word(site)
        for site in range(address, address + count * 4, 4)
        if words.contains_word(site)
    ]
    valid = sum(valid_instruction(word) for word in sample)
    return f"{valid}/{len(sample)}"


def linear_return(
    words: ImageWords,
    address: int,
    stop: int,
    limit: int = 0x1000,
) -> int | None:
    end = min(words.layout.load_end - 4, address + limit, stop)
    for site in range(address, end, 4):
        if is_jr_ra(words.word(site)) and words.contains_word(site + 4):
            return site
    return None


def valid_span(words: ImageWords, start: int, end: int) -> bool:
    return all(valid_instruction(words.word(site)) for site in range(start, end, 4))


def boundary_before(words: ImageWords, address: int) -> str:
    if address == words.layout.load_address:
        return "load-start"
    if words.contains_word(address - 8) and is_jr_ra(words.word(address - 8)):
        return "previous-return"
    preceding = [
        words.word(site)
        for site in (address - 8, address - 4)
        if words.contains_word(site)
    ]
    if preceding and all(word == 0 for word in preceding):
        return "zero-padding"
    return ""


def reference_maps(
    words: ImageWords,
    extents: list[FunctionExtent],
) -> tuple[Counter[int], Counter[int]]:
    pointer_refs: Counter[int] = Counter()
    tail_refs: Counter[int] = Counter()
    for site in words.addresses():
        word = words.word(site)
        if words.contains_word(word):
            pointer_refs[word] += 1
        owner = extent_owner(extents, site)
        if owner is None or opcode(word) != 0x02:
            continue
        target = function_target(site, word)
        if words.contains_word(target) and not (owner.start <= target < owner.end):
            tail_refs[target] += 1
    return pointer_refs, tail_refs


def relocation_refs(config_dir: Path) -> dict[tuple[str, int], int]:
    _fields, rows = read_tsv(config_dir / "relocs.tsv")
    result: Counter[tuple[str, int]] = Counter()
    for row in rows:
        if row["status"] != "rejected" and row["target_region"] == "load":
            result[(row["image"], parse_int(row["target_va"]))] += 1
    return dict(result)


def audit_image(
    image: str,
    words: ImageWords,
    extents: list[FunctionExtent],
    relocations: dict[tuple[str, int], int],
) -> list[dict[str, object]]:
    known_starts = {extent.start for extent in extents}
    pointer_refs, tail_refs = reference_maps(words, extents)
    candidates = set(pointer_refs) | set(tail_refs)
    candidates.update(
        address for address in words.addresses() if frame_at(words, address) is not None
    )
    rows: list[dict[str, object]] = []
    for address in sorted(candidates - known_starts):
        frame = frame_at(words, address)
        owner = extent_owner(extents, address)
        boundary = boundary_before(words, address)
        return_site = matching_return(words, address, frame) if frame else None
        score = first_window_score(words, address)
        valid, total = (int(part) for part in score.split("/"))
        refs = pointer_refs[address] + tail_refs[address]
        reloc_count = relocations.get((image, address), 0)

        confidence = "weak"
        if frame and return_site is not None and valid >= max(8, total - 2):
            if owner is None or boundary or refs or reloc_count:
                confidence = "strong"
            else:
                confidence = "frame-return"
        elif refs and valid >= max(8, total - 2):
            confidence = "referenced-code"
        if confidence == "weak":
            continue

        evidence = []
        if frame:
            evidence.append("stack-frame+ra-save")
        if return_site is not None:
            evidence.append("matching-stack-return")
        if pointer_refs[address]:
            evidence.append("stored-pointer")
        if tail_refs[address]:
            evidence.append("cross-extent-jump")
        if reloc_count:
            evidence.append("relocation-candidate")
        rows.append({
            "image": image,
            "va": f"0x{address:08x}",
            "file_offset": f"0x{words.layout.file_offset(address):x}",
            "owner_va": f"0x{owner.start:08x}" if owner else "",
            "frame_size": f"0x{frame.size:x}" if frame else "",
            "ra_save_offset": f"0x{frame.ra_offset:x}" if frame else "",
            "return_va": f"0x{return_site:08x}" if return_site is not None else "",
            "pointer_refs": pointer_refs[address],
            "tail_jump_refs": tail_refs[address],
            "reloc_refs": reloc_count,
            "valid_first_12": score,
            "boundary": boundary,
            "confidence": confidence,
            "note": "+".join(evidence),
        })

    # A compiler-generated leaf routine need not allocate a frame or save RA.
    # Carve only clean sequences between an observed return and the next return,
    # and do not cross a known or strong frame-based start. This finds routines
    # stranded between linked functions without treating every code-like word as
    # a new entry point.
    strong_ranges = [
        (parse_int(str(row["va"])), parse_int(str(row["return_va"])) + 8)
        for row in rows
        if row["confidence"] == "strong"
    ]
    boundaries = sorted(known_starts | {start for start, _end in strong_ranges})
    rows_by_start = {parse_int(str(row["va"])): row for row in rows}
    for return_site in words.addresses():
        if not is_jr_ra(words.word(return_site)):
            continue
        address = return_site + 8
        padding_words = 0
        while (
            padding_words < 3
            and words.contains_word(address)
            and words.word(address) == 0
        ):
            address += 4
            padding_words += 1
        existing = rows_by_start.get(address)
        if (
            not words.contains_word(address)
            or address in known_starts
            or extent_owner(extents, address) is not None
            or any(start <= address < end for start, end in strong_ranges)
            or (existing is not None and existing["confidence"] != "referenced-code")
        ):
            continue
        index = bisect.bisect_right(boundaries, address)
        stop = boundaries[index] if index < len(boundaries) else words.layout.load_end
        next_return = linear_return(words, address, stop)
        if next_return is None or not valid_span(words, address, next_return + 8):
            continue
        size = next_return + 8 - address
        if size < 8:
            continue
        reloc_count = relocations.get((image, address), 0)
        evidence = ["previous-return", "return-delimited-valid-code"]
        if padding_words:
            evidence.append(f"skipped-{padding_words}-padding-nop")
        if pointer_refs[address]:
            evidence.append("stored-pointer")
        if tail_refs[address]:
            evidence.append("cross-extent-jump")
        if reloc_count:
            evidence.append("relocation-candidate")
        leaf_row = {
            "image": image,
            "va": f"0x{address:08x}",
            "file_offset": f"0x{words.layout.file_offset(address):x}",
            "owner_va": "",
            "frame_size": "",
            "ra_save_offset": "",
            "return_va": f"0x{next_return:08x}",
            "pointer_refs": pointer_refs[address],
            "tail_jump_refs": tail_refs[address],
            "reloc_refs": reloc_count,
            "valid_first_12": first_window_score(words, address),
            "boundary": (
                f"previous-return+{padding_words}-padding-nop"
                if padding_words
                else "previous-return"
            ),
            "confidence": "strong-leaf",
            "note": "+".join(evidence),
        }
        if existing is None:
            rows.append(leaf_row)
            rows_by_start[address] = leaf_row
        else:
            existing.update(leaf_row)
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe-dir", type=Path, required=True)
    parser.add_argument("--config-dir", type=Path, default=Path("config/retail"))
    parser.add_argument(
        "--output", type=Path, default=Path("build/function-audit/candidates.tsv")
    )
    args = parser.parse_args()

    extents = load_extents(args.config_dir)
    relocations = relocation_refs(args.config_dir)
    rows: list[dict[str, object]] = []
    for image in IMAGE_LAYOUTS:
        words = ImageWords(args.exe_dir / image, image)
        image_rows = audit_image(image, words, extents.get(image, []), relocations)
        rows.extend(image_rows)
        counts = Counter(row["confidence"] for row in image_rows)
        summary = ", ".join(f"{key}={counts[key]}" for key in sorted(counts))
        print(f"[function-audit] {image}: {summary or 'no candidates'}")

    rows.sort(key=lambda row: (IMAGE_ORDER[str(row["image"])], parse_int(str(row["va"]))))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8", newline="") as stream:
        stream.write("# REVIEW CANDIDATES ONLY; this analyzer never edits config/retail.\n")
        writer = csv.DictWriter(stream, FIELDS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print(f"[function-audit] total={len(rows)} output={args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
