"""Carve conservative MIPS ELF target objects from the three retail programs.

The PS-X EXE format contains linked bytes, not an object-file relocation table.
This tool therefore consumes the hand-owned WIP inventories in ``config/retail``
and records every relocation decision it makes.  The default ``safe`` policy is
deliberately narrower than the candidate census.
"""

from __future__ import annotations

import argparse
import bisect
import re
import struct
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from scripts.kf.mips_elf import MipsRelocation, SECTION_SYMBOL, write_mips_elf
from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    format_hex,
    format_size,
    parse_int,
    parse_psx_exe,
    read_tsv,
    write_tsv,
)


OBJECT_FIELDS = (
    "image",
    "va",
    "size",
    "body_size",
    "name",
    "scope",
    "provider",
    "library",
    "object",
    "relocations",
    "confidence",
    "provenance",
)
USED_RELOCATION_FIELDS = (
    "image",
    "function_va",
    "site_va",
    "paired_site_va",
    "action",
    "relocation",
    "symbol",
    "addend",
    "source_kind",
    "source_channel",
    "confidence",
)
WITHHELD_RELOCATION_FIELDS = (
    "image",
    "function_va",
    "site_va",
    "paired_site_va",
    "kind",
    "channel",
    "target_va",
    "confidence",
    "status",
    "reason",
)
WITHHELD_FUNCTION_FIELDS = (
    "image",
    "va",
    "size",
    "body_size",
    "fragments",
    "name",
    "reason",
)

SIGNED_LOW_OPCODES = {
    0x08,  # addi
    0x09,  # addiu
    0x20,  # lb
    0x21,  # lh
    0x23,  # lw
    0x24,  # lbu
    0x25,  # lhu
    0x28,  # sb
    0x29,  # sh
    0x2B,  # sw
}


@dataclass(frozen=True)
class Function:
    image: str
    va: int
    size: int
    body_size: int
    fragments: int
    symbol: str
    confidence: str
    provenance: str
    provider: str = ""
    library: str = ""

    @property
    def end(self) -> int:
        return self.va + self.size

    def contains(self, address: int, size: int = 1) -> bool:
        return self.va <= address and address + size <= self.end

    @property
    def scope(self) -> str:
        return "vendored" if self.provider else "decomp"


@dataclass(frozen=True)
class DataObject:
    image: str
    va: int
    size: int
    symbol: str

    @property
    def end(self) -> int:
        return self.va + self.size


@dataclass(frozen=True)
class Catalog:
    functions: dict[str, tuple[Function, ...]]
    function_starts: dict[str, dict[int, Function]]
    data: dict[str, tuple[DataObject, ...]]


def sanitize_symbol(value: str, fallback: str) -> str:
    symbol = re.sub(r"[^0-9A-Za-z_$]", "_", value.strip())
    if not symbol:
        symbol = fallback
    if symbol[0].isdigit():
        symbol = f"_{symbol}"
    return symbol


def image_key(image: str) -> str:
    return image.removesuffix(".EXE").lower()


def decode_mips26_target(site_va: int, word: int) -> int:
    """Decode a J/JAL target at its linked virtual address."""
    return ((site_va + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def encode_mips26_addend(word: int, addend: int) -> int:
    if addend & 3:
        raise ValueError(f"unaligned MIPS26 addend {addend:#x}")
    return (word & 0xFC000000) | ((addend >> 2) & 0x03FFFFFF)


def decode_hi_lo_target(hi_word: int, lo_word: int) -> int:
    if hi_word >> 26 != 0x0F:
        raise ValueError("HI site is not LUI")
    hi_register = (hi_word >> 16) & 0x1F
    low_opcode = lo_word >> 26
    if (lo_word >> 21) & 0x1F != hi_register:
        raise ValueError("LO instruction does not consume the LUI register")
    low = lo_word & 0xFFFF
    if low_opcode == 0x0D:  # ori
        return ((hi_word & 0xFFFF) << 16) | low
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError(f"unsupported LO opcode {low_opcode:#x}")
    signed_low = low if low < 0x8000 else low - 0x10000
    return (((hi_word & 0xFFFF) << 16) + signed_low) & 0xFFFFFFFF


def encode_hi_lo_addend(hi_word: int, lo_word: int, addend: int) -> tuple[int, int]:
    """Encode the implicit addend used by an ELF HI16/LO16 REL pair."""
    low_opcode = lo_word >> 26
    if low_opcode not in SIGNED_LOW_OPCODES:
        raise ValueError("ELF HI16/LO16 requires a sign-extending low instruction")
    high = ((addend + 0x8000) >> 16) & 0xFFFF
    low = addend & 0xFFFF
    return (hi_word & 0xFFFF0000) | high, (lo_word & 0xFFFF0000) | low


def _word(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def _put_word(data: bytearray, offset: int, word: int) -> None:
    struct.pack_into("<I", data, offset, word & 0xFFFFFFFF)


def _vendored_functions(
    config_dir: Path,
) -> dict[tuple[str, int], dict[str, str]]:
    functions: dict[tuple[str, int], dict[str, str]] = {}
    path = config_dir / "functions_vendored.tsv"
    if path.is_file():
        _, rows = read_tsv(path)
        for row in rows:
            functions[(row["image"], parse_int(row["va"]))] = row
    return functions


def load_catalog(config_dir: Path) -> Catalog:
    vendored = _vendored_functions(config_dir)
    _, function_rows = read_tsv(config_dir / "functions.tsv")

    preferred: dict[tuple[str, int], str] = {}
    for row in function_rows:
        key = row["image"], parse_int(row["va"])
        provider_row = vendored.get(key, {})
        preferred[key] = sanitize_symbol(
            row["name"] or provider_row.get("name", ""),
            f"func_{key[1]:08x}",
        )
    duplicate_names = {
        image: Counter(
            name for (candidate_image, _), name in preferred.items()
            if candidate_image == image
        )
        for image in IMAGE_LAYOUTS
    }

    functions: dict[str, list[Function]] = {image: [] for image in IMAGE_LAYOUTS}
    starts: dict[str, dict[int, Function]] = {image: {} for image in IMAGE_LAYOUTS}
    for row in function_rows:
        image = row["image"]
        va = parse_int(row["va"])
        provider_row = vendored.get((image, va), {})
        symbol = preferred[(image, va)]
        if duplicate_names[image][symbol] > 1:
            symbol = f"{symbol}_{va:08x}"
        function = Function(
            image=image,
            va=va,
            size=parse_int(row["size"]),
            body_size=parse_int(row["body_size"]),
            fragments=int(row["fragments"]),
            symbol=symbol,
            confidence=row["confidence"],
            provenance=row["provenance"],
            provider=provider_row.get("provider", ""),
            library=provider_row.get("library", ""),
        )
        functions[image].append(function)
        starts[image][va] = function

    _, data_rows = read_tsv(config_dir / "data.tsv")
    data: dict[str, list[DataObject]] = {image: [] for image in IMAGE_LAYOUTS}
    for row in data_rows:
        va = parse_int(row["va"])
        data[row["image"]].append(DataObject(
            image=row["image"],
            va=va,
            size=parse_int(row["size"]),
            symbol=sanitize_symbol(row["name"], f"DAT_{va:08x}"),
        ))

    return Catalog(
        functions={image: tuple(sorted(rows, key=lambda item: item.va))
                   for image, rows in functions.items()},
        function_starts=starts,
        data={image: tuple(sorted(rows, key=lambda item: item.va))
              for image, rows in data.items()},
    )


def _containing_function(catalog: Catalog, image: str, address: int) -> Function | None:
    functions = catalog.functions[image]
    index = bisect.bisect_right([item.va for item in functions], address) - 1
    if index >= 0 and functions[index].contains(address):
        return functions[index]
    return None


def _containing_data(catalog: Catalog, image: str, address: int) -> DataObject | None:
    objects = catalog.data[image]
    index = bisect.bisect_right([item.va for item in objects], address) - 1
    if index >= 0 and objects[index].va <= address < objects[index].end:
        return objects[index]
    return None


def _resolve_symbol(
    catalog: Catalog,
    function: Function,
    target: int,
    target_name: str,
) -> tuple[str, int]:
    if function.contains(target):
        return SECTION_SYMBOL, target - function.va
    target_function = catalog.function_starts[function.image].get(target)
    if target_function is not None:
        return target_function.symbol, 0
    if target_name:
        return sanitize_symbol(target_name, f"DAT_{target:08x}"), 0
    target_data = _containing_data(catalog, function.image, target)
    if target_data is not None:
        return target_data.symbol, target - target_data.va
    return f"DAT_{target:08x}", 0


def _withheld(row: dict[str, str], owner: Function | None, reason: str) -> dict[str, object]:
    return {
        "image": row["image"],
        "function_va": format_hex(owner.va) if owner else "",
        "site_va": row["site_va"],
        "paired_site_va": row["paired_site_va"],
        "kind": row["kind"],
        "channel": row["channel"],
        "target_va": row["target_va"],
        "confidence": row["confidence"],
        "status": row["status"],
        "reason": reason,
    }


def _policy_reason(row: dict[str, str], policy: str) -> str | None:
    if row["status"] == "rejected":
        return "candidate-rejected"
    if policy == "reviewed" and row["status"] != "reviewed":
        return "not-manually-reviewed"
    if policy in {"safe", "reviewed"} and row["channel"] != "reachable-code":
        return "non-reachable-code-channel"
    return None


def _apply_relocation(
    blob: bytearray,
    function: Function,
    row: dict[str, str],
    catalog: Catalog,
    policy: str,
) -> tuple[list[MipsRelocation], dict[str, object]]:
    reason = _policy_reason(row, policy)
    if reason:
        raise ValueError(reason)

    site = parse_int(row["site_va"])
    target = parse_int(row["target_va"])
    offset = site - function.va
    symbol, addend = _resolve_symbol(catalog, function, target, row["target_name"])

    if row["kind"] == "mips26":
        word = _word(blob, offset)
        expected_opcode = {"j": 2, "jal": 3}.get(row["opcode"])
        if expected_opcode is None or word >> 26 != expected_opcode:
            raise ValueError("instruction-opcode-mismatch")
        if decode_mips26_target(site, word) != target:
            raise ValueError("decoded-target-mismatch")
        target_function = catalog.function_starts[function.image].get(target)
        local_target = function.contains(target)
        if not local_target and target_function is None and policy != "all":
            raise ValueError("target-is-not-a-function-start")
        _put_word(blob, offset, encode_mips26_addend(word, addend))
        relocation = MipsRelocation(offset, "R_MIPS_26", symbol)
        action = "local-section" if local_target else "external-symbol"
        kinds = "R_MIPS_26"
        relocations = [relocation]
    elif row["kind"] == "mips_hi16_lo16":
        if row["target_region"] != "load" and policy != "all":
            raise ValueError("target-outside-load-image")
        if not row["paired_site_va"]:
            raise ValueError("missing-paired-site")
        paired = parse_int(row["paired_site_va"])
        if not function.contains(paired, 4):
            raise ValueError("pair-crosses-function-extent")
        paired_offset = paired - function.va
        hi_word = _word(blob, offset)
        lo_word = _word(blob, paired_offset)
        try:
            decoded = decode_hi_lo_target(hi_word, lo_word)
        except ValueError as error:
            raise ValueError(f"instruction-pair-mismatch:{error}") from error
        if decoded != target:
            raise ValueError("decoded-target-mismatch")
        try:
            hi_word, lo_word = encode_hi_lo_addend(hi_word, lo_word, addend)
        except ValueError as error:
            raise ValueError("unsupported-unsigned-low-pair") from error
        _put_word(blob, offset, hi_word)
        _put_word(blob, paired_offset, lo_word)
        relocations = [
            MipsRelocation(offset, "R_MIPS_HI16", symbol),
            MipsRelocation(paired_offset, "R_MIPS_LO16", symbol),
        ]
        action = "paired-symbol"
        kinds = "R_MIPS_HI16+R_MIPS_LO16"
    elif row["kind"] == "mips32_candidate" and policy == "all":
        word = _word(blob, offset)
        if word != target:
            raise ValueError("decoded-target-mismatch")
        _put_word(blob, offset, addend)
        relocations = [MipsRelocation(offset, "R_MIPS_32", symbol)]
        action = "raw-word"
        kinds = "R_MIPS_32"
    else:
        raise ValueError("unsupported-relocation-kind")

    used = {
        "image": function.image,
        "function_va": format_hex(function.va),
        "site_va": row["site_va"],
        "paired_site_va": row["paired_site_va"],
        "action": action,
        "relocation": kinds,
        "symbol": symbol,
        "addend": format_hex(addend),
        "source_kind": row["kind"],
        "source_channel": row["channel"],
        "confidence": row["confidence"],
    }
    return relocations, used


def _comments(description: str, policy: str) -> tuple[str, ...]:
    return (
        "GENERATED - do not curate this build artifact by hand.",
        description,
        f"relocation policy: {policy}",
        "Inputs remain the manually curated WIP TSVs in config/retail.",
    )


def delink(
    exe_dir: Path,
    config_dir: Path,
    output_dir: Path,
    images: Iterable[str] = IMAGE_LAYOUTS,
    vas: Iterable[int] = (),
    policy: str = "safe",
) -> dict[str, dict[str, int]]:
    selected_images = tuple(dict.fromkeys(images))
    selected_vas = set(vas)
    catalog = load_catalog(config_dir)
    _, relocation_rows = read_tsv(config_dir / "relocs.tsv")
    rows_by_image: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in relocation_rows:
        if row["image"] in selected_images:
            rows_by_image[row["image"]].append(row)

    results: dict[str, dict[str, int]] = {}
    for image in selected_images:
        expected = IMAGE_LAYOUTS[image]
        exe_path = exe_dir / image
        actual = parse_psx_exe(exe_path)
        if actual != expected:
            raise ValueError(f"{exe_path}: retail identity/layout mismatch")
        executable = exe_path.read_bytes()
        image_output = output_dir / image_key(image)
        object_output = image_output / "objects"
        object_output.mkdir(parents=True, exist_ok=True)

        functions = [
            function for function in catalog.functions[image]
            if not selected_vas or function.va in selected_vas
        ]
        selected_function_starts = {function.va for function in functions}
        rows_by_owner: dict[int, list[dict[str, str]]] = defaultdict(list)
        withheld_rows: list[dict[str, object]] = []
        for row in rows_by_image[image]:
            owner = _containing_function(catalog, image, parse_int(row["site_va"]))
            if owner is None:
                if not selected_vas:
                    withheld_rows.append(_withheld(row, None, "site-outside-function"))
                continue
            if owner.va in selected_function_starts:
                rows_by_owner[owner.va].append(row)

        object_rows: list[dict[str, object]] = []
        used_rows: list[dict[str, object]] = []
        withheld_functions: list[dict[str, object]] = []
        for function in functions:
            if function.fragments != 1:
                withheld_functions.append({
                    "image": image,
                    "va": format_hex(function.va),
                    "size": format_size(function.size),
                    "body_size": format_size(function.body_size),
                    "fragments": function.fragments,
                    "name": function.symbol,
                    "reason": "fragment-ranges-not-yet-recorded",
                })
                for row in rows_by_owner[function.va]:
                    withheld_rows.append(_withheld(row, function, "function-is-fragmented"))
                continue

            start = expected.file_offset(function.va)
            blob = bytearray(executable[start:start + function.size])
            if len(blob) != function.size:
                raise ValueError(f"{exe_path}: truncated function {function.va:#x}")
            function_relocations: list[MipsRelocation] = []
            occupied = Counter()
            for row in rows_by_owner[function.va]:
                occupied[parse_int(row["site_va"])] += 1
                if row["paired_site_va"]:
                    occupied[parse_int(row["paired_site_va"])] += 1

            for row in rows_by_owner[function.va]:
                sites = [parse_int(row["site_va"])]
                if row["paired_site_va"]:
                    sites.append(parse_int(row["paired_site_va"]))
                if any(occupied[site] > 1 for site in sites):
                    withheld_rows.append(_withheld(
                        row, function, "ambiguous-overlapping-candidates"
                    ))
                    continue
                try:
                    relocations, used = _apply_relocation(
                        blob, function, row, catalog, policy
                    )
                except ValueError as error:
                    withheld_rows.append(_withheld(row, function, str(error)))
                    continue
                function_relocations.extend(relocations)
                used_rows.append(used)

            object_name = f"{function.va:08x}_{function.symbol}.o"
            object_relative = Path("objects") / object_name
            (object_output / object_name).write_bytes(write_mips_elf(
                bytes(blob),
                function.symbol,
                function.body_size,
                function_relocations,
            ))
            object_rows.append({
                "image": image,
                "va": format_hex(function.va),
                "size": format_size(function.size),
                "body_size": format_size(function.body_size),
                "name": function.symbol,
                "scope": function.scope,
                "provider": function.provider,
                "library": function.library,
                "object": object_relative.as_posix(),
                "relocations": len(function_relocations),
                "confidence": function.confidence,
                "provenance": function.provenance,
            })

        write_tsv(
            image_output / "objects.tsv",
            OBJECT_FIELDS,
            object_rows,
            _comments("One target MIPS ELF object per admitted contiguous function.", policy),
        )
        write_tsv(
            image_output / "relocations_used.tsv",
            USED_RELOCATION_FIELDS,
            used_rows,
            _comments("Relocations materialized in target objects.", policy),
        )
        write_tsv(
            image_output / "relocations_withheld.tsv",
            WITHHELD_RELOCATION_FIELDS,
            withheld_rows,
            _comments("Candidates intentionally not materialized, with a reason.", policy),
        )
        write_tsv(
            image_output / "functions_withheld.tsv",
            WITHHELD_FUNCTION_FIELDS,
            withheld_functions,
            _comments("Functions not carved into target objects.", policy),
        )
        results[image] = {
            "objects": len(object_rows),
            "relocations": sum(int(row["relocations"]) for row in object_rows),
            "withheld_relocations": len(withheld_rows),
            "withheld_functions": len(withheld_functions),
        }
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe-dir", required=True, type=Path)
    parser.add_argument("--config-dir", type=Path, default=Path("config/retail"))
    parser.add_argument("--output-dir", type=Path, default=Path("build/delink"))
    parser.add_argument(
        "--image",
        action="append",
        choices=tuple(IMAGE_LAYOUTS),
        help="repeat to select targets; default: all three",
    )
    parser.add_argument(
        "--va",
        action="append",
        type=parse_int,
        default=[],
        help="repeat to carve selected function starts only",
    )
    parser.add_argument(
        "--policy",
        choices=("safe", "reviewed", "all"),
        default="safe",
    )
    args = parser.parse_args()
    results = delink(
        args.exe_dir,
        args.config_dir,
        args.output_dir,
        args.image or IMAGE_LAYOUTS,
        args.va,
        args.policy,
    )
    for image, counts in results.items():
        print(image + ": " + ", ".join(
            f"{name}={count}" for name, count in counts.items()
        ))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
