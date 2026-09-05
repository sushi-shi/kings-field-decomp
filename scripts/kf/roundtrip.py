"""Relink source/config target objects at their claimed retail section addresses.

This checks the delinker's bytes and placements with the independent GNU MIPS
linker, not the delinker's inverse encoding helpers. It is not reconstruction
progress or proof that all reachable bytes have been modeled. Packed DATA/BSS
claims must admit one base per ELF section; symbols are never scattered or
overridden to make an unplaceable section appear linkable.
"""

from __future__ import annotations

import argparse
import io
import json
import re
import shutil
import subprocess
import tempfile
from collections import Counter, defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path

from elftools.common.exceptions import ELFError
from elftools.elf.elffile import ELFFile

from scripts.kf.delink import load_catalog, sanitize_symbol
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv
from scripts.kf.sema.image import RetailImage


SECTIONS = {".text": "SHT_PROGBITS", ".rodata": "SHT_PROGBITS",
            ".data": "SHT_PROGBITS", ".bss": "SHT_NOBITS"}
LINKER = "mipsel-linux-gnu-ld"
SYMBOL_NAME = re.compile(r"[A-Za-z_$][A-Za-z0-9_$]*\Z")
ADDRESS_NAME = re.compile(r"DAT_([0-9a-fA-F]{8})\Z")


@dataclass
class SectionPlacement:
    name: str
    address: int
    size: int
    claimed_bytes: int
    storage: str


@dataclass
class UnitResult:
    image: str
    unit: str
    object: str
    sections: list[SectionPlacement] = field(default_factory=list)
    issues: list[dict] = field(default_factory=list)
    initialized_bytes_compared: int = 0
    relocation_rows: int = 0
    linker_ran: bool = False
    linker_diagnostics: str = ""

    def issue(self, kind: str, **details: object) -> None:
        self.issues.append({"kind": kind, **details})


def address_book(image: str, config_dir: Path = RETAIL_CONFIG) -> dict[str, set[int]]:
    """Image-local input identities only; never infer S from delinker output."""
    book: dict[str, set[int]] = defaultdict(set)
    catalog = load_catalog(config_dir)
    for item in (*catalog.functions[image], *catalog.data[image]):
        book[item.symbol].add(item.va)
    # Include curated BSS, overlay RAM and hardware identities outside load data.
    for row in read_tsv(config_dir / "data_identities.tsv")[1]:
        if row["image"] == image:
            va = int(row["va"], 0)
            book[sanitize_symbol(row["name"], f"DAT_{va:08x}")].add(va)
    return dict(book)


def plan(elf: ELFFile, unit: Unit, result: UnitResult) -> dict[str, int]:
    """Derive one section base from every claim and actual ELF symbol offset."""
    if (elf.elfclass != 32 or not elf.little_endian
            or elf['e_machine'] != 'EM_MIPS' or elf['e_type'] != 'ET_REL'):
        result.issue("unsupported-object-format")
        return {}
    symtab = elf.get_section_by_name(".symtab")
    if symtab is None:
        result.issue("missing-symbol-table")
        return {}
    symbols: dict[str, list] = defaultdict(list)
    for symbol in symtab.iter_symbols():
        if symbol.name:
            symbols[symbol.name].append(symbol)
    # name, address, extent, expected section. Function sizes exclude linker
    # padding, but .text owns the entire contiguous run, including delay slots.
    claims = [(f.symbol, f.va, f.body_size, ".text") for f in unit.functions]
    claims += [(d.symbol, d.va, d.size, ".data" if d.storage == "load" else ".bss")
               for d in unit.data]
    bases: dict[str, set[int]] = defaultdict(set)
    sizes: Counter = Counter()
    witnesses: dict[str, list[dict]] = defaultdict(list)
    for name, va, size, section_name in claims:
        matches = symbols.get(name, [])
        if len(matches) != 1:
            result.issue("missing-or-ambiguous-owned-symbol", symbol=name, count=len(matches))
            continue
        symbol = matches[0]
        index = symbol['st_shndx']
        actual_section = elf.get_section(index).name if isinstance(index, int) else index
        if actual_section != section_name or symbol['st_size'] != size:
            result.issue("owned-symbol-layout", symbol=name, expected_section=section_name,
                         actual_section=actual_section, expected_size=size,
                         actual_size=symbol['st_size'])
            continue
        offset = symbol['st_value']
        section = elf.get_section(index)
        if offset + size > section['sh_size']:
            result.issue("owned-symbol-out-of-bounds", symbol=name)
            continue
        bases[section_name].add(va - offset)
        sizes[section_name] += size
        witnesses[section_name].append({"symbol": name, "va": va, "offset": offset, "size": size,
                                        "implied_base": va - offset})
    text_extent = unit.functions[-1].end - unit.functions[0].va if unit.functions else 0
    if unit.functions:
        sizes[".text"] = text_extent
    if unit.rodata:
        bases[".rodata"].add(unit.rodata[0])
        sizes[".rodata"] = unit.rodata[1]
    placed = {}
    seen = set()
    for section in elf.iter_sections():
        name, size = section.name, section['sh_size']
        if not section['sh_flags'] & 2:  # SHF_ALLOC
            continue
        if name in seen:
            result.issue("duplicate-allocated-section", section=name)
            continue
        seen.add(name)
        # ELF ABI metadata, not PS-X runtime storage. Target objects currently
        # emit neither; recognizing them does not admit arbitrary extra data.
        if name in {".reginfo", ".MIPS.abiflags"}:
            continue
        if name not in SECTIONS or section['sh_type'] != SECTIONS[name]:
            result.issue("unsupported-allocated-section", section=name, size=size)
            continue
        if not size:
            if sizes[name]:
                result.issue("owned-section-extent", section=name, size=0, claimed=sizes[name])
            continue
        candidates = bases.get(name, set())
        if len(candidates) != 1:
            result.issue("conflicting-section-bases" if candidates else "unclaimed-section",
                         section=name, size=size, claims=witnesses[name])
            continue
        owned = sorted((s['offset'], s['offset'] + s['size'], s['symbol'])
                       for s in witnesses[name])
        for left, right in zip(owned, owned[1:]):
            if left[1] > right[0]:
                result.issue("overlapping-owned-symbols", section=name,
                             symbols=[left[2], right[2]])
        base = next(iter(candidates))
        if base < 0 or base + size > 0x100000000 or base % max(1, section['sh_addralign']):
            result.issue("invalid-section-placement", section=name, address=base, size=size,
                         alignment=section['sh_addralign'])
            continue
        if sizes[name] > size or (name == ".text" and size != text_extent):
            result.issue("owned-section-extent", section=name, size=size, claimed=sizes[name])
            continue
        placed[name] = base
        result.sections.append(SectionPlacement(
            name, base, size, sizes[name], "bss" if name == ".bss" else "load",
        ))
    for name in bases:
        if name not in seen:
            result.issue("missing-owned-section", section=name)
    return placed


def externals(elf: ELFFile, book: dict[str, set[int]], result: UnitResult) -> dict[str, int]:
    resolved = {}
    for section in elf.iter_sections():
        if section['sh_type'] not in {"SHT_REL", "SHT_RELA"}:
            continue
        target_section = elf.get_section(section['sh_info'])
        if target_section.name not in SECTIONS:
            continue
        if section['sh_type'] != "SHT_REL":
            result.issue("unsupported-relocation-section", section=section.name)
            continue
        symtab = elf.get_section(section['sh_link'])
        for row in section.iter_relocations():
            result.relocation_rows += 1
            if row['r_info_type'] not in {2, 4, 5, 6}:
                result.issue("unsupported-relocation", section=target_section.name,
                             offset=row['r_offset'], type=row['r_info_type'])
            if row['r_offset'] & 3 or row['r_offset'] + 4 > target_section['sh_size']:
                result.issue("relocation-out-of-bounds", section=target_section.name,
                             offset=row['r_offset'])
            symbol = symtab.get_symbol(row['r_info_sym'])
            if symbol['st_shndx'] != 'SHN_UNDEF':
                continue
            name = symbol.name
            candidates = book.get(name, set())
            address_name = ADDRESS_NAME.fullmatch(name)
            if not candidates and address_name:
                # This is the unresolved identity's literal address, not proof
                # of a datum's classification, extent or reachability.
                candidates = {int(address_name.group(1), 16)}
            if len(candidates) != 1 or not SYMBOL_NAME.fullmatch(name):
                result.issue("unresolved-external", symbol=name, candidates=sorted(candidates))
                continue
            resolved[name] = next(iter(candidates))
    return resolved


def compare_linked(elf: ELFFile, image: RetailImage, result: UnitResult) -> None:
    expected_sections = {s.name for s in result.sections}
    for section in elf.iter_sections():
        if (section['sh_flags'] & 2 and section['sh_size']
                and section.name not in expected_sections):
            result.issue("unexpected-linked-allocation", section=section.name)
    for placement in result.sections:
        section = elf.get_section_by_name(placement.name)
        if (section is None or section['sh_addr'] != placement.address
                or section['sh_size'] != placement.size):
            result.issue("linker-changed-section-layout", section=placement.name)
            continue
        if placement.storage == "bss":
            if section['sh_type'] != 'SHT_NOBITS':
                result.issue("initialized-bss", section=placement.name)
            continue
        expected = image.read(placement.address, placement.size)
        if expected is None:
            result.issue("section-outside-retail-load", section=placement.name,
                         address=placement.address, size=placement.size)
            continue
        actual = section.data()
        if len(actual) != len(expected):
            result.issue("truncated-linked-section", section=placement.name)
            continue
        result.initialized_bytes_compared += len(expected)
        if actual != expected:
            first = next(i for i, pair in enumerate(zip(expected, actual)) if pair[0] != pair[1])
            result.issue("retail-byte-mismatch", section=placement.name, offset=first,
                         va=placement.address + first, expected=expected[first], actual=actual[first],
                         differing_bytes=sum(a != b for a, b in zip(expected, actual)))


def verify_unit(unit: Unit, image: RetailImage, book: dict[str, set[int]],
                object_path: Path, scratch: Path, *, linker: str = LINKER) -> UnitResult:
    result = UnitResult(unit.image, unit.unit, str(object_path))
    try:
        if unit.image != image.image:
            raise ValueError("unit/image namespace mismatch")
        elf = ELFFile(io.BytesIO(object_path.read_bytes()))
        placed = plan(elf, unit, result)
        if result.issues:
            return result
        symbols = externals(elf, book, result)
        if result.issues:
            return result
        # Never scatter a packed section or override an internally defined
        # symbol to its desired VA. GNU ld must honor the entire input layout.
        with tempfile.TemporaryDirectory(prefix="link-", dir=scratch) as temporary:
            root = Path(temporary)
            script = root / "retail.ld"
            script.write_text("SECTIONS {\n" + "\n".join(
                f"  {name} {address:#x} : {{ *({name}) }}"
                for name, address in sorted(placed.items(), key=lambda item: item[1])
            ) + "\n  /DISCARD/ : { *(*) }\n}\n")
            linked = root / "linked.elf"
            entry = unit.functions[0].va if unit.functions else min(placed.values())
            command = [linker, "-EL", "--entry", hex(entry), "-T", str(script),
                       "-o", str(linked), str(object_path.resolve())]
            command += [f"--defsym={name}={address:#x}" for name, address in sorted(symbols.items())]
            process = subprocess.run(command, capture_output=True, text=True, timeout=30)
            result.linker_ran = True
            result.linker_diagnostics = process.stderr.strip()
            if process.returncode:
                result.issue("linker-failed", returncode=process.returncode)
                return result
            compare_linked(ELFFile(io.BytesIO(linked.read_bytes())), image, result)
    except (ELFError, OSError, ValueError, IndexError, subprocess.TimeoutExpired) as error:
        result.issue("artifact-or-link-error", detail=str(error))
    return result


def find_overlaps(results: list[UnitResult]) -> None:
    spans = sorted((result.image, s.address, s.address + s.size, i, s.name)
                   for i, result in enumerate(results) for s in result.sections if s.size)
    for pos, (image, start, end, index, name) in enumerate(spans):
        for other_image, other_start, other_end, other_index, other_name in spans[pos + 1:]:
            if other_image != image or other_start >= end:
                break
            details = {"section": name, "other_unit": results[other_index].unit,
                       "other_section": other_name, "address": other_start,
                       "size": min(end, other_end) - other_start}
            results[index].issue("overlapping-section-placements", **details)
            results[other_index].issue("overlapping-section-placements", **{
                **details, "section": other_name, "other_unit": results[index].unit,
                "other_section": name,
            })


def audit(image: RetailImage, units: tuple[Unit, ...], book: dict[str, set[int]],
          delink_dir: Path, scratch: Path) -> dict:
    selected = tuple(unit for unit in units if unit.image == image.image)
    results = [verify_unit(unit, image, book,
                           unit.target_path(delink_dir) if hasattr(unit, "target_path") else
                           delink_dir / unit.image_key / "modules" / unit.object_name, scratch)
               for unit in selected]
    find_overlaps(results)
    counts = Counter(issue['kind'] for result in results for issue in result.issues)
    if not selected:
        counts['empty-selection'] += 1
    return {
        "image": image.image, "scope": "source-and-config-target-objects",
        "linker": shutil.which(LINKER), "units": [asdict(result) for result in results],
        "verified_units": sum(not result.issues for result in results),
        "total_units": len(results), "issues": dict(sorted(counts.items())),
        "initialized_bytes_compared": sum(r.initialized_bytes_compared for r in results),
        "complete_reachable_bytes_proven": False, "linked_image_equality_proven": False,
    }


def run(images=IMAGE_LAYOUTS, *, output: Path | None = None, unit: str | None = None) -> int:
    from scripts.kf.config_data import load as load_contributions

    manifest = load_manifest()
    all_units = (*manifest.units, *load_contributions(modules=manifest.modules()))
    by_name = {u.unit: u for u in all_units}
    images = tuple(images)
    if unit is not None and unit not in by_name:
        raise ValueError(f"unknown unit {unit!r}")
    if unit is not None:
        image = by_name[unit].image
        if image not in images:
            raise ValueError(f"unit {unit!r} is not in the selected image(s)")
        images = (image,)
    if shutil.which(LINKER) is None:
        raise ValueError(f"{LINKER} is required; run in nix develop")
    units = tuple(u for u in all_units if unit is None or u.unit == unit)
    reports = []
    scratch = BUILD / "roundtrip"
    scratch.mkdir(parents=True, exist_ok=True)
    for name in images:
        report = audit(RetailImage.load(name), units, address_book(name), BUILD / "delink", scratch)
        reports.append(report)
        print(f"{name} target relink: {report['verified_units']}/{report['total_units']} units verified")
        for kind, count in report['issues'].items():
            print(f"  {kind}: {count}")
    if output is not None:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(reports, indent=2) + "\n")
    return int(any(report['issues'] for report in reports))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", choices=("psx", "game", "open"), action="append")
    parser.add_argument("--unit")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    images = tuple(name.upper() + ".EXE" for name in args.image) if args.image else tuple(IMAGE_LAYOUTS)
    try:
        return run(images, output=args.output, unit=args.unit)
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
