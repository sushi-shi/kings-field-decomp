"""kf.data_match - data-section matching of reconstruction vs retail.

objdiff scores each unit's ``.text`` against the object carved out of retail;
it also reports a per-section ``.data``/``.rodata`` percentage, but not WHERE a
data section diverges, nor the ``None``-scored case where the reconstruction
emits no section at all. This module supplies that: for every manifested unit
that owns data it compares the reconstruction object's initialized data
sections against the retail-delinked target object, reloc-masked (like the code
objdiff), and reports the first divergence and any referent mismatch. ``.bss``
is compared by size/ownership only - uninitialized storage has no bytes to
match.

    kf verify data                 # per-image summary + the diverging units
    kf verify data --image game    # one image
    kf verify data --coverage      # add the owned-vs-total data coverage gap
    kf verify data --detail        # per-diverging-unit byte/referent detail

Retail ground truth is the delinked object under ``build/delink/<img>/modules``
whose ``.data``/``.rodata`` bytes are copied from the retail image; the
reconstruction is ``build/objdiff/<img>/base/<obj>``. Both are built by
``kf build``; this reads them, so run a build first.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path

from scripts.kf.delink import image_key
from scripts.kf.manifest import Manifest, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv


INIT_SECTIONS = (".data", ".rodata", ".sdata")
BSS_SECTIONS = (".bss", ".sbss")


# --------------------------------------------------------------------------- #
# Minimal ELF32 little-endian reader (the delinker's writer is one-directional)
# --------------------------------------------------------------------------- #
@dataclass(frozen=True)
class Reloc:
    offset: int
    type: int
    symbol: str


@dataclass(frozen=True)
class Section:
    name: str
    type: int
    size: int
    data: bytes


class Elf:
    """Just enough of an ELF32-LE relocatable object to read data + relocs."""

    def __init__(self, path: Path) -> None:
        blob = path.read_bytes()
        if blob[:4] != b"\x7fELF":
            raise ValueError(f"{path}: not an ELF object")
        e_shoff = struct.unpack_from("<I", blob, 0x20)[0]
        e_shentsize = struct.unpack_from("<H", blob, 0x2E)[0]
        e_shnum = struct.unpack_from("<H", blob, 0x30)[0]
        e_shstrndx = struct.unpack_from("<H", blob, 0x32)[0]
        raw = []
        for i in range(e_shnum):
            base = e_shoff + i * e_shentsize
            (name, typ, _flags, _addr, offset, size, link, info,
             _align, entsize) = struct.unpack_from("<10I", blob, base)
            raw.append((name, typ, offset, size, link, info, entsize))
        shstr = raw[e_shstrndx][2]

        def name_at(table_off: int, rel: int) -> str:
            end = blob.index(b"\x00", table_off + rel)
            return blob[table_off + rel:end].decode("ascii", "replace")

        self.sections: dict[str, Section] = {}
        self._sec_by_index: dict[int, str] = {}
        symtab_idx = None
        for idx, (name, typ, offset, size, _link, _info, _ent) in enumerate(raw):
            sname = name_at(shstr, name)
            self._sec_by_index[idx] = sname
            body = b"" if typ == 8 else blob[offset:offset + size]  # SHT_NOBITS=8
            self.sections[sname] = Section(sname, typ, size, body)
            if typ == 2:  # SHT_SYMTAB
                symtab_idx = idx

        self._symbols: list[str] = []
        if symtab_idx is not None:
            _, _, sym_off, sym_size, link, _info, entsize = raw[symtab_idx]
            str_off = raw[link][2]
            entsize = entsize or 16
            for pos in range(sym_off, sym_off + sym_size, entsize):
                st_name = struct.unpack_from("<I", blob, pos)[0]
                st_shndx = struct.unpack_from("<H", blob, pos + 14)[0]
                if st_name:
                    self._symbols.append(name_at(str_off, st_name))
                else:
                    # a section symbol has no name; resolve it to the section it
                    # points at so a jump-table reloc compares as `.text`/`.rodata`
                    # rather than an indistinguishable empty string.
                    self._symbols.append(self._sec_by_index.get(st_shndx, ""))

        self._relocs: dict[str, list[Reloc]] = {}
        for idx, (_name, typ, offset, size, _link, info, entsize) in enumerate(raw):
            if typ != 9:  # SHT_REL
                continue
            target = self._sec_by_index.get(info)
            if target is None:
                continue
            entsize = entsize or 8
            entries: list[Reloc] = []
            for pos in range(offset, offset + size, entsize):
                r_offset, r_info = struct.unpack_from("<II", blob, pos)
                sym_index = r_info >> 8
                r_type = r_info & 0xFF
                symbol = (self._symbols[sym_index]
                          if sym_index < len(self._symbols) else "")
                entries.append(Reloc(r_offset, r_type, symbol))
            self._relocs[target] = entries

    def relocations(self, section: str) -> list[Reloc]:
        return self._relocs.get(section, [])


def _masked(section: Section, relocs: list[Reloc]) -> bytes:
    """Section bytes with every 4-byte relocation field blanked."""
    out = bytearray(section.data)
    for reloc in relocs:
        for k in range(4):
            if reloc.offset + k < len(out):
                out[reloc.offset + k] = 0
    return bytes(out)


# --------------------------------------------------------------------------- #
# Per-unit data diff
# --------------------------------------------------------------------------- #
@dataclass
class SectionDiff:
    name: str
    retail_size: int
    recon_size: int
    status: str            # match | missing | extra | size | bytes | referent
    detail: str = ""


@dataclass
class UnitDataDiff:
    image: str
    unit: str
    diffs: list[SectionDiff] = field(default_factory=list)

    @property
    def matches(self) -> bool:
        return all(d.status == "match" for d in self.diffs)

    @property
    def divergent(self) -> list[SectionDiff]:
        return [d for d in self.diffs if d.status != "match"]


def _diff_init_section(name: str, retail: Elf, recon: Elf) -> SectionDiff | None:
    rt = retail.sections.get(name)
    rc = recon.sections.get(name)
    rt_size = rt.size if rt else 0
    rc_size = rc.size if rc else 0
    # A section counts as present only when it carries bytes; a compiler-emitted
    # zero-length `.data` is not ownership.
    rt_present = rt is not None and rt_size > 0
    rc_present = rc is not None and rc_size > 0
    if not rt_present and not rc_present:
        return None
    if rt_present and not rc_present:
        return SectionDiff(name, rt_size, rc_size, "missing",
                           f"retail has {rt_size} B ({len(retail.relocations(name))} "
                           f"reloc), reconstruction emits none")
    if rc_present and not rt_present:
        return SectionDiff(name, rt_size, rc_size, "extra",
                           f"reconstruction has {rc_size} B, retail has none")
    rt_rel = retail.relocations(name)
    rc_rel = recon.relocations(name)
    # The retail-delinked module packs exactly the claimed bytes; GNU `as` pads
    # the reconstruction's section to its 16-byte alignment. So compare content
    # over the shared extent (reloc-masked) and treat a zero-only tail on the
    # longer side as benign alignment padding, mirroring objdiff's symbol-level
    # data scoring rather than flagging every alignment pad as a mismatch.
    overlap = min(rt_size, rc_size)
    rt_bytes = _masked(rt, rt_rel)
    rc_bytes = _masked(rc, rc_rel)
    first = next((i for i in range(overlap) if rt_bytes[i] != rc_bytes[i]), None)
    if first is not None:
        return SectionDiff(name, rt_size, rc_size, "bytes",
                           f"masked content diverges at +{first:#x} "
                           f"(retail {rt_bytes[first]:#04x} vs "
                           f"reconstruction {rc_bytes[first]:#04x})")
    if rc_size > rt_size and any(rc_bytes[rt_size:]):
        extra = next(i for i in range(rt_size, rc_size) if rc_bytes[i])
        return SectionDiff(name, rt_size, rc_size, "extra-tail",
                           f"reconstruction emits {rc_size - rt_size} extra byte(s) "
                           f"past the retail extent, non-zero at +{extra:#x}")
    if rt_size > rc_size and any(rt_bytes[rc_size:]):
        short = next(i for i in range(rc_size, rt_size) if rt_bytes[i])
        return SectionDiff(name, rt_size, rc_size, "short",
                           f"reconstruction is {rt_size - rc_size} byte(s) short; "
                           f"retail has content at +{short:#x}")
    rt_key = [(r.offset, r.symbol) for r in rt_rel if r.offset < overlap]
    rc_key = [(r.offset, r.symbol) for r in rc_rel if r.offset < overlap]
    if rt_key != rc_key:
        rt_by_off = dict(rt_key)
        rc_by_off = dict(rc_key)
        same_referents = sorted(rt_by_off.values()) == sorted(rc_by_off.values())
        if set(rt_by_off) != set(rc_by_off) and same_referents:
            detail = (f"same {len(rt_key)} referent(s), shifted layout: "
                      f"retail reloc offsets {sorted(rt_by_off)[:4]} vs "
                      f"reconstruction {sorted(rc_by_off)[:4]}")
        else:
            pos = next((off for off in sorted(set(rt_by_off) | set(rc_by_off))
                        if rt_by_off.get(off) != rc_by_off.get(off)), None)
            detail = (f"reloc +{pos:#x} retail->{rt_by_off.get(pos, '(none)')} "
                      f"vs reconstruction->{rc_by_off.get(pos, '(none)')}")
        return SectionDiff(name, rt_size, rc_size, "referent", detail)
    return SectionDiff(name, rt_size, rc_size, "match")


def _diff_bss(retail: Elf, recon: Elf) -> SectionDiff | None:
    rt = sum(retail.sections[s].size for s in BSS_SECTIONS if s in retail.sections)
    rc = sum(recon.sections[s].size for s in BSS_SECTIONS if s in recon.sections)
    if rt == 0 and rc == 0:
        return None
    # `.bss` has no bytes to match; only its size/ownership is checkable, and
    # the reconstruction's is padded to 16-byte section alignment.
    padded = (rt + 15) & ~15
    if rc < rt:
        return SectionDiff(".bss", rt, rc, "size",
                           f"reconstruction owns {rc} B, retail {rt} B "
                           f"(missing uninitialized storage)")
    if rc > padded:
        return SectionDiff(".bss", rt, rc, "size",
                           f"reconstruction owns {rc} B, retail {rt} B "
                           f"(extra uninitialized storage beyond alignment)")
    return SectionDiff(".bss", rt, rc, "match")


def diff_unit(image: str, object_name: str,
              delink_dir: Path, objdiff_dir: Path) -> UnitDataDiff | None:
    key = image_key(image)
    target = delink_dir / key / "modules" / object_name
    base = objdiff_dir / key / "base" / object_name
    if not target.is_file() or not base.is_file():
        return None
    retail = Elf(target)
    recon = Elf(base)
    result = UnitDataDiff(image, object_name)
    for name in INIT_SECTIONS:
        diff = _diff_init_section(name, retail, recon)
        if diff is not None:
            result.diffs.append(diff)
    bss = _diff_bss(retail, recon)
    if bss is not None:
        result.diffs.append(bss)
    return result if result.diffs else None


def diff_image(image: str, manifest: Manifest,
               delink_dir: Path, objdiff_dir: Path) -> list[UnitDataDiff]:
    results: list[UnitDataDiff] = []
    for unit in manifest.units:
        if unit.image != image:
            continue
        diff = diff_unit(image, unit.object_name, delink_dir, objdiff_dir)
        if diff is not None:
            diff.unit = unit.unit
            results.append(diff)
    return results


# --------------------------------------------------------------------------- #
# Coverage: how much retail initialized data any unit owns
# --------------------------------------------------------------------------- #
def coverage(image: str, delink_dir: Path) -> tuple[int, int, list[tuple[int, int, str]]]:
    """(owned_init_bytes, total_init_bytes, largest unowned data.tsv extents)."""
    _fields, data_rows = read_tsv(RETAIL_CONFIG / "data.tsv")
    total = 0
    extents: list[tuple[int, int, str]] = []
    for row in data_rows:
        if row["image"] != image:
            continue
        size = int(row["size"], 0)
        total += size
        extents.append((int(row["va"], 0), size, row.get("name") or row.get("kind", "")))
    key = image_key(image)
    objects = delink_dir / key / "objects.tsv"
    owned = 0
    if objects.is_file():
        _f, rows = read_tsv(objects)
        for row in rows:
            if row.get("scope") != "module":
                continue
            owned += int(row["data_size"], 0) + int(row["rodata_size"], 0)
    extents.sort(key=lambda item: item[1], reverse=True)
    return owned, total, extents[:15]


# --------------------------------------------------------------------------- #
def run(images: tuple[str, ...], *, show_detail: bool, show_coverage: bool,
        delink_dir: Path, objdiff_dir: Path) -> int:
    manifest = load_manifest()
    total_units = total_match = total_diverge = 0
    for image in images:
        # diff_image returns every data-owning unit: a matching one carries only
        # `match` SectionDiffs, a diverging one carries at least one other.
        results = diff_image(image, manifest, delink_dir, objdiff_dir)
        diverging = [r for r in results if not r.matches]
        matched = len(results) - len(diverging)
        print(f"\n{image}: {len(results)} unit(s) own data; "
              f"{matched} match retail, {len(diverging)} diverge")
        total_units += len(results)
        total_match += matched
        total_diverge += len(diverging)
        for result in diverging:
            summary = ", ".join(f"{d.name} [{d.status}]" for d in result.divergent)
            print(f"  DIVERGE {result.unit:<40} {summary}")
            if show_detail:
                for d in result.divergent:
                    print(f"          {d.name:<9} retail={d.retail_size:<6} "
                          f"recon={d.recon_size:<6} {d.detail}")
        if show_coverage:
            owned, total, extents = coverage(image, delink_dir)
            pct = (100.0 * owned / total) if total else 0.0
            print(f"  coverage: units own {owned}/{total} B "
                  f"({pct:.1f}%) of retail initialized data")
            print("  largest unowned/curated data extents (extern-crutch backlog):")
            for va, size, name in extents[:10]:
                print(f"    {va:#010x}  {size:>6} B  {name}")
    print(f"\ndata-match: {total_match}/{total_units} data-owning unit(s) match retail"
          f" ({total_diverge} diverge)")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="kf verify data", description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    aliases = {image_key(image): image for image in IMAGE_LAYOUTS}
    parser.add_argument("--image", action="append", choices=tuple(aliases))
    parser.add_argument("--detail", action="store_true",
                        help="print per-section byte/referent detail for diverging units")
    parser.add_argument("--coverage", action="store_true",
                        help="add the owned-vs-total initialized-data coverage gap")
    parser.add_argument("--delink-dir", type=Path, default=BUILD / "delink")
    parser.add_argument("--objdiff-dir", type=Path, default=BUILD / "objdiff")
    args = parser.parse_args(argv)
    images = tuple(aliases[value] for value in args.image) if args.image \
        else tuple(IMAGE_LAYOUTS)
    return run(images, show_detail=args.detail, show_coverage=args.coverage,
               delink_dir=args.delink_dir, objdiff_dir=args.objdiff_dir)


if __name__ == "__main__":
    raise SystemExit(main())
