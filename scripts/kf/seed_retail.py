"""Produce a review candidate for the hand-owned retail executable censuses.

This is intentionally a seed/audit tool. It writes to ``build/retail-seed`` by
default and never updates ``config/retail``. Once admitted, the TSVs are edited
manually; later runs are evidence for review rather than authoritative output.
"""

from __future__ import annotations

import argparse
import csv
import re
import tempfile
from pathlib import Path

from scripts.kf.retail import (
    DATA_FIELDS,
    FUNCTION_FIELDS,
    IMAGE_LAYOUTS,
    IMAGE_ORDER,
    RELOC_FIELDS,
    format_hex,
    format_size,
    parse_int,
    parse_psx_exe,
    write_tsv,
)


DEFAULT_GHIDRA_NAMES = re.compile(r"(?:FUN|LAB|SUB)_[0-9a-fA-F]+$")
GHIDRA_PROVENANCE = "Ghidra-12.0.4/ghidra_psx_ldr-2026.07.08"


def read_rows(path: Path) -> list[dict[str, str]]:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    return list(csv.DictReader(lines, delimiter="\t"))


def reviewed_symbols(path: Path) -> dict[str, list[dict[str, str]]]:
    by_image = {image: [] for image in IMAGE_LAYOUTS}
    for row in read_rows(path):
        if row["image"] in by_image:
            by_image[row["image"]].append(row)
    return by_image


def address_value(address) -> int:
    return int(address.getOffset())


def escape_trailing_spaces(text: str) -> str:
    """Keep byte-significant trailing spaces without trailing TSV whitespace."""
    trimmed = text.rstrip(" ")
    return trimmed + "\\x20" * (len(text) - len(trimmed))


def known_at(
    symbols: dict[str, list[dict[str, str]]], image: str, address: int, kind: str
) -> list[dict[str, str]]:
    return [
        row for row in symbols[image]
        if parse_int(row["va"]) == address and row["kind"] == kind
    ]


def function_rows(program, image: str, symbols: dict[str, list[dict[str, str]]]):
    layout = IMAGE_LAYOUTS[image]
    rows: list[dict[str, object]] = []
    for function in program.getFunctionManager().getFunctions(True):
        address = address_value(function.getEntryPoint())
        if not layout.contains(address, 1):
            continue
        body = function.getBody()
        body_min = max(address_value(body.getMinAddress()), layout.load_address)
        body_max = min(address_value(body.getMaxAddress()), layout.load_end - 1)
        size = body_max - body_min + 1
        body_size = int(body.getNumAddresses())
        fragments = int(body.getNumAddressRanges())
        exact = known_at(symbols, image, address, "function")
        aliases = known_at(symbols, image, address, "alias")
        if exact:
            primary = exact[0]
            size = parse_int(primary["size"])
            name = primary["name"]
            confidence = primary["confidence"]
            provenance = primary["source"]
            note = primary["evidence"]
        else:
            ghidra_name = str(function.getName())
            name = "" if DEFAULT_GHIDRA_NAMES.fullmatch(ghidra_name) else ghidra_name
            confidence = "entry" if address == layout.entry else "ghidra-analysis"
            provenance = GHIDRA_PROVENANCE
            note = "machine-assisted WIP boundary"
        label_items = [f"+0x0={row['name']}" for row in aliases]
        rows.append({
            "image": image,
            "va": format_hex(address),
            "file_offset": format_size(layout.file_offset(address)),
            "size": format_size(size),
            "body_size": format_size(min(body_size, size)),
            "fragments": fragments,
            "kind": "function",
            "confidence": confidence,
            "name": name,
            "labels": ";".join(label_items),
            "provenance": provenance,
            "note": note,
        })

    rows.sort(key=lambda row: parse_int(str(row["va"])))
    for symbol in symbols[image]:
        if symbol["kind"] != "label":
            continue
        address = parse_int(symbol["va"])
        owners = [row for row in rows
                  if parse_int(str(row["va"])) <= address <
                  parse_int(str(row["va"])) + parse_int(str(row["size"]))]
        if not owners:
            continue
        owner = max(owners, key=lambda row: parse_int(str(row["va"])))
        offset = address - parse_int(str(owner["va"]))
        label = f"+0x{offset:x}={symbol['name']}"
        owner["labels"] = ";".join(filter(None, (str(owner["labels"]), label)))
        if symbol["evidence"] not in str(owner["note"]):
            owner["note"] = f"{owner['note']}; {symbol['evidence']}"
    return rows


def classify_datatype(name: str) -> str:
    lowered = name.lower()
    if "string" in lowered or "unicode" in lowered:
        return "string"
    if "pointer" in lowered or lowered.endswith(" *"):
        return "pointer"
    if "[" in name and "]" in name:
        return "array"
    if "struct" in lowered:
        return "structure"
    return "defined"


def data_rows(
    program,
    image: str,
    symbols: dict[str, list[dict[str, str]]],
    strings: list[dict[str, str]],
    functions: list[dict[str, object]],
):
    layout = IMAGE_LAYOUTS[image]
    rows: dict[tuple[int, str], dict[str, object]] = {}
    listing = program.getListing()
    for datum in listing.getDefinedData(True):
        address = address_value(datum.getMinAddress())
        size = int(datum.getLength())
        if size <= 0 or not layout.contains(address, size):
            continue
        datatype = str(datum.getDataType().getDisplayName())
        row = {
            "image": image,
            "va": format_hex(address),
            "file_offset": format_size(layout.file_offset(address)),
            "size": format_size(size),
            "kind": classify_datatype(datatype),
            "confidence": "ghidra-defined",
            "name": "",
            "datatype": datatype.replace("\t", " ").replace("\n", " "),
            "provenance": GHIDRA_PROVENANCE,
            "note": "machine-assisted WIP datum",
            "_priority": 1,
        }
        rows[(address, str(row["kind"]))] = row

    for string in strings:
        if string["path"] != image or not string["va"]:
            continue
        address = parse_int(string["va"])
        size = int(string["byte_length"])
        if not layout.contains(address, size):
            continue
        for key in [key for key in rows if key[0] == address]:
            del rows[key]
        rows[(address, "string")] = {
            "image": image,
            "va": format_hex(address),
            "file_offset": format_size(layout.file_offset(address)),
            "size": format_size(size),
            "kind": "string",
            "confidence": "byte-pattern",
            "name": "",
            "datatype": string["encoding"],
            "provenance": "investigation:string-census",
            "note": "escaped text: " + escape_trailing_spaces(string["text"]),
            "_priority": 2,
        }

    for symbol in symbols[image]:
        if symbol["kind"] not in {"data", "pointer-table", "bss"}:
            continue
        address = parse_int(symbol["va"])
        size = parse_int(symbol["size"])
        if size <= 0 or not layout.contains(address, size):
            continue
        for key in [key for key in rows if key[0] == address]:
            del rows[key]
        kind = "pointer-table" if symbol["kind"] == "pointer-table" else symbol["kind"]
        rows[(address, kind)] = {
            "image": image,
            "va": format_hex(address),
            "file_offset": format_size(layout.file_offset(address)),
            "size": format_size(size),
            "kind": kind,
            "confidence": symbol["confidence"],
            "name": symbol["name"],
            "datatype": "",
            "provenance": symbol["source"],
            "note": symbol["evidence"],
            "_priority": 3,
        }

    # The Ghidra listing may expose array elements in addition to a reviewed
    # aggregate, and ASCII/Shift-JIS scans can overlap generic Ghidra data.
    # Keep one structural datum for each overlapping interval, preferring
    # reviewed rows, then byte-proven strings, then generic analyzer output.
    selected: list[dict[str, object]] = []
    candidates = sorted(
        rows.values(),
        key=lambda row: (
            -int(row["_priority"]),
            parse_int(str(row["va"])),
            -parse_int(str(row["size"])),
            str(row["kind"]),
        ),
    )
    for candidate in candidates:
        start = parse_int(str(candidate["va"]))
        end = start + parse_int(str(candidate["size"]))
        if any(
            start < parse_int(str(other["va"])) + parse_int(str(other["size"]))
            and parse_int(str(other["va"])) < end
            for other in selected
        ):
            continue
        selected.append(candidate)
    rows = {
        (parse_int(str(row["va"])), str(row["kind"])): row
        for row in selected
    }

    occupied: list[tuple[int, int]] = []
    for function in functions:
        start = parse_int(str(function["va"]))
        occupied.append((start, start + parse_int(str(function["size"]))))
    for row in rows.values():
        start = parse_int(str(row["va"]))
        occupied.append((start, start + parse_int(str(row["size"]))))
    merged: list[tuple[int, int]] = []
    for start, end in sorted(occupied):
        if merged and start <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(merged[-1][1], end))
        else:
            merged.append((start, end))
    cursor = layout.load_address
    for start, end in merged:
        if cursor < start:
            rows[(cursor, "unclassified")] = {
                "image": image,
                "va": format_hex(cursor),
                "file_offset": format_size(layout.file_offset(cursor)),
                "size": format_size(start - cursor),
                "kind": "unclassified",
                "confidence": "coverage-gap",
                "name": "",
                "datatype": "",
                "provenance": "payload-complement",
                "note": "may contain missed code, tables, constants, padding, or writable data",
                "_priority": 0,
            }
        cursor = max(cursor, end)
    if cursor < layout.load_end:
        rows[(cursor, "unclassified")] = {
            "image": image,
            "va": format_hex(cursor),
            "file_offset": format_size(layout.file_offset(cursor)),
            "size": format_size(layout.load_end - cursor),
            "kind": "unclassified",
            "confidence": "coverage-gap",
            "name": "",
            "datatype": "",
            "provenance": "payload-complement",
            "note": "may contain missed code, tables, constants, padding, or writable data",
            "_priority": 0,
        }
    result = []
    for row in rows.values():
        row = dict(row)
        del row["_priority"]
        result.append(row)
    return sorted(result, key=lambda row: (parse_int(str(row["va"])), str(row["kind"])))


def relocation_rows(
    path: Path,
    names: dict[tuple[str, int], str],
) -> list[dict[str, object]]:
    rows = []
    for source in read_rows(path):
        image = source["path"]
        if image not in IMAGE_LAYOUTS:
            continue
        target = parse_int(source["target_va"])
        rows.append({
            "image": image,
            "site_va": source["site_va"],
            "site_file_offset": source["site_exe_offset"],
            "paired_site_va": source["paired_site_va"],
            "kind": source["kind"],
            "channel": source["channel"],
            "target_va": source["target_va"],
            "target_region": source["target_region"],
            "target_name": names.get((image, target), ""),
            "opcode": source["opcode"],
            "register": source["register"],
            "confidence": source["confidence"],
            "status": "candidate",
            "provenance": "investigation:relocs.tsv",
        })
    return sorted(rows, key=lambda row: (
        IMAGE_ORDER[str(row["image"])],
        parse_int(str(row["site_va"])),
        parse_int(str(row["paired_site_va"])) if row["paired_site_va"] else -1,
        str(row["kind"]),
        parse_int(str(row["target_va"])),
    ))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe-dir", type=Path, required=True)
    parser.add_argument("--evidence-dir", type=Path, required=True)
    parser.add_argument("--reviewed-symbols", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, default=Path("build/retail-seed"))
    args = parser.parse_args()

    for image, expected in IMAGE_LAYOUTS.items():
        actual = parse_psx_exe(args.exe_dir / image)
        if actual != expected:
            raise ValueError(f"{args.exe_dir / image}: retail identity/layout mismatch")

    import pyghidra

    symbols = reviewed_symbols(args.reviewed_symbols)
    strings = read_rows(args.evidence_dir / "strings.tsv")
    all_functions: list[dict[str, object]] = []
    all_data: list[dict[str, object]] = []
    with tempfile.TemporaryDirectory(prefix="kf-retail-ghidra-") as project_root:
        for image in IMAGE_LAYOUTS:
            with pyghidra.open_program(
                args.exe_dir / image,
                project_location=Path(project_root) / image.lower(),
                project_name="analysis",
                analyze=True,
            ) as api:
                program = api.getCurrentProgram()
                if str(program.getLanguageID()) != "PSX:LE:32:default":
                    raise ValueError(f"{image}: PSX loader selected {program.getLanguageID()}")
                functions = function_rows(program, image, symbols)
                data = data_rows(program, image, symbols, strings, functions)
                all_functions.extend(functions)
                all_data.extend(data)
                print(f"[seed] {image}: functions={len(functions)} data={len(data)}")

    names = {
        (str(row["image"]), parse_int(str(row["va"]))): str(row["name"])
        for row in [*all_functions, *all_data] if row["name"]
    }
    all_relocs = relocation_rows(args.evidence_dir / "relocs.tsv", names)
    comments = (
        "INITIAL MACHINE-ASSISTED SEED; after admission this file is MANUALLY MANAGED.",
        "Never overwrite config/retail from this tool; diff build/retail-seed and review rows.",
        "Each address is qualified by image because GAME.EXE and OPEN.EXE are separate overlays.",
    )
    write_tsv(args.output_dir / "functions.tsv", FUNCTION_FIELDS, all_functions, comments + (
        "Function extents are WIP and may include owned jump tables/padding or miss indirect-only code.",
    ))
    write_tsv(args.output_dir / "data.tsv", DATA_FIELDS, all_data, comments + (
        "Defined rows plus unclassified gaps account for every byte of every loaded payload.",
    ))
    write_tsv(args.output_dir / "relocs.tsv", RELOC_FIELDS, all_relocs, comments + (
        "PS-X EXE retains no relocation directory: candidate does not mean linker truth.",
    ))
    print(f"[seed] relocations={len(all_relocs)} output={args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
