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

from scripts.kf.mips_elf import (
    RODATA_SECTION_SYMBOL,
    STB_GLOBAL,
    STB_LOCAL,
    STT_FUNC,
    STT_OBJECT,
    DefinedSymbol,
    MipsRelocation,
    SECTION_SYMBOL,
    write_mips_elf,
)
from scripts.kf.relocations import (
    decode_hi_lo_target,
    decode_mips26_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
    named_data_referent,
    validate_relocation,
)
from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    format_hex,
    format_size,
    parse_int,
    parse_psx_exe,
    read_tsv,
    write_tsv,
)


__all__ = (
    "decode_hi_lo_target",
    "decode_mips26_target",
    "encode_hi_lo_addend",
    "encode_mips26_addend",
)


OBJECT_FIELDS = (
    "image",
    "va",
    "size",
    "body_size",
    "name",
    "unit",
    "scope",
    "provider",
    "library",
    "object",
    "relocations",
    "data_size",
    "bss_size",
    "rodata_size",
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
class Datum:
    """A global a unit claims with DATA(); ``storage`` is load or bss."""

    va: int
    size: int
    symbol: str
    storage: str
    scope: str = ""

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def binding(self) -> int:
        return STB_LOCAL if self.scope == "static" else STB_GLOBAL

    @property
    def alignment(self) -> int:
        # Working packing constraint bounded by the retail address, not proof
        # of the original section alignment or allocation class. The pinned
        # GCC/maspsx path gives file-static tentative BSS an eight-byte stride
        # even for one-word objects; other claims currently pack up to four.
        candidates = (
            (8, 4, 2)
            if self.storage == "bss" and self.scope == "static"
            else (4, 2)
        )
        for candidate in candidates:
            if self.va % candidate == 0:
                return candidate
        return 1


class _DataOwner:
    """Structural stand-in for a claimed datum while validating rows sited in it.

    ``contains`` always answers False so a target inside the datum resolves
    through the catalog to the datum's own symbol instead of a code section.
    """

    def __init__(self, image: str, datum: Datum) -> None:
        self.image = image
        self.va = datum.va
        self.size = datum.size

    def contains(self, address: int, size: int = 1) -> bool:
        return False


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


def _game_call_targets(
    config_dir: Path,
    function_rows: list[dict[str, str]],
    vendored: dict[tuple[str, int], dict[str, str]],
) -> dict[str, set[int]]:
    """Direct-call targets of admitted ``jal`` candidates sited in game code.

    The linked image resolves a duplicated library name to exactly one body.
    Calls from non-vendored functions are the retail evidence for which body
    that was; rejected rows and calls from inside vendored code do not count.
    """
    targets: dict[str, set[int]] = {image: set() for image in IMAGE_LAYOUTS}
    path = config_dir / "relocs.tsv"
    if not path.is_file():
        return targets
    extents: dict[str, list[tuple[int, int]]] = {image: [] for image in IMAGE_LAYOUTS}
    for row in function_rows:
        extents[row["image"]].append((parse_int(row["va"]), parse_int(row["size"])))
    for image in extents:
        extents[image].sort()
    _, rows = read_tsv(path)
    for row in rows:
        if row.get("kind") != "mips26" or row.get("opcode") != "jal":
            continue
        if row.get("status") == "rejected":
            continue
        image = row["image"]
        site = parse_int(row["site_va"])
        starts = extents.get(image, [])
        index = bisect.bisect_right(starts, (site, 0xFFFFFFFF)) - 1
        if index < 0:
            continue
        start, size = starts[index]
        if not start <= site < start + size or (image, start) in vendored:
            continue
        targets[image].add(parse_int(row["target_va"]))
    return targets


def _identity_names(config_dir: Path, filename: str) -> dict[tuple[str, int], str]:
    """Curated source-level names keyed by exact (image, va)."""
    path = config_dir / filename
    if not path.is_file():
        return {}
    _, rows = read_tsv(path)
    return {
        (row["image"], parse_int(row["va"])): row["name"]
        for row in rows
        if row.get("name")
    }


def load_catalog(config_dir: Path) -> Catalog:
    vendored = _vendored_functions(config_dir)
    _, function_rows = read_tsv(config_dir / "functions.tsv")
    # Reconstructed source spells labelled functions and data by their curated
    # identities, so target objects carry the same symbols. Address-derived
    # identities remain the explicit unresolved spelling.
    function_identities = _identity_names(config_dir, "function_identities.tsv")
    data_identities = _identity_names(config_dir, "data_identities.tsv")

    preferred: dict[tuple[str, int], str] = {}
    for row in function_rows:
        key = row["image"], parse_int(row["va"])
        provider_row = vendored.get(key, {})
        if key in vendored:
            name = row["name"] or provider_row.get("name", "")
        else:
            name = function_identities.get(key) or row["name"]
        preferred[key] = sanitize_symbol(name, f"func_{key[1]:08x}")
    duplicate_names = {
        image: Counter(
            name for (candidate_image, _), name in preferred.items()
            if candidate_image == image
        )
        for image in IMAGE_LAYOUTS
    }
    # Several bodies can share one library name (LIBAPI and LIBGPU both carry a
    # memset). Only the single instance that admitted game code calls keeps the
    # plain name; every other copy stays address-qualified so reconstructed
    # source can name the linked symbol without inventing one.
    game_calls = _game_call_targets(config_dir, function_rows, vendored)
    plain_duplicate: dict[tuple[str, str], int] = {}
    for (candidate_image, candidate_va), name in preferred.items():
        if duplicate_names[candidate_image][name] > 1 and candidate_va in game_calls[candidate_image]:
            plain_duplicate.setdefault((candidate_image, name), []).append(candidate_va)
    plain_duplicate = {
        key: called[0] for key, called in plain_duplicate.items() if len(called) == 1
    }

    functions: dict[str, list[Function]] = {image: [] for image in IMAGE_LAYOUTS}
    starts: dict[str, dict[int, Function]] = {image: {} for image in IMAGE_LAYOUTS}
    for row in function_rows:
        image = row["image"]
        va = parse_int(row["va"])
        provider_row = vendored.get((image, va), {})
        symbol = preferred[(image, va)]
        if duplicate_names[image][symbol] > 1 and plain_duplicate.get((image, symbol)) != va:
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
    data_starts: set[tuple[str, int]] = set()
    for row in data_rows:
        va = parse_int(row["va"])
        data_starts.add((row["image"], va))
        data[row["image"]].append(DataObject(
            image=row["image"],
            va=va,
            size=parse_int(row["size"]),
            symbol=sanitize_symbol(
                data_identities.get((row["image"], va)) or row["name"],
                f"DAT_{va:08x}",
            ),
        ))
    identity_path = config_dir / "data_identities.tsv"
    identity_rows = read_tsv(identity_path)[1] if identity_path.is_file() else []
    for row in identity_rows:
        image = row["image"]
        va = parse_int(row["va"])
        if row["storage"] != "bss" or (image, va) in data_starts:
            continue
        data[image].append(DataObject(
            image=image,
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
    rodata: tuple[int, int] | None = None,
    *,
    data_reference: bool = False,
) -> tuple[str, int]:
    named = named_data_referent(
        catalog, function.image, sanitize_symbol(target_name, "")
    ) if target_name else None
    if data_reference and named is not None:
        # A biased data base can coincide with unrelated code or RODATA. Use
        # the reviewed allocation, then express unit-owned literals/tables
        # through their actual read-only section contribution.
        if rodata is not None and rodata[0] <= named.va < rodata[0] + rodata[1]:
            return RODATA_SECTION_SYMBOL, target - rodata[0]
        return named.name, target - named.va
    if function.contains(target):
        return SECTION_SYMBOL, target - function.va
    if rodata is not None and rodata[0] <= target < rodata[0] + rodata[1]:
        # The owning module's read-only contribution: jump tables and string
        # literals compare as .rodata offsets, the way the compiler emits them.
        return RODATA_SECTION_SYMBOL, target - rodata[0]
    target_function = catalog.function_starts[function.image].get(target)
    if target_function is not None:
        return target_function.symbol, 0
    if target_name:
        symbol = sanitize_symbol(target_name, f"DAT_{target:08x}")
        address_name = re.fullmatch(r"DAT_([0-9A-Fa-f]{8})", symbol)
        if address_name is not None:
            # An address-derived owner spelling names the datum, not its
            # symbol: a curated identity at that address wins so the target
            # carries the same name reconstructed source uses.
            owner_va = int(address_name.group(1), 16)
            if named is not None:
                return named.name, target - owner_va
            return symbol, target - owner_va
        # A reviewed row may name the owning datum even when the address lies
        # outside its extent: `&table[index - 1]` folds to `table - stride`.
        # The addend is measured from that owner, wherever the target lies.
        if named is not None:
            return symbol, target - named.va
        return symbol, 0
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


def _competes_for_site(row: dict[str, str], policy: str) -> bool:
    if row["status"] == "rejected":
        return False
    if policy == "all":
        return True
    if policy == "reviewed":
        return row["status"] == "reviewed"
    return row["target_region"] == "load" or row["status"] == "reviewed"


def _apply_relocation(
    blob: bytearray,
    function: Function,
    row: dict[str, str],
    catalog: Catalog,
    policy: str,
    rodata: tuple[int, int] | None = None,
) -> tuple[list[MipsRelocation], dict[str, object]]:
    validation = validate_relocation(blob, function, row, catalog, policy)
    target = validation.target
    offset = validation.offset
    symbol, addend = _resolve_symbol(
        catalog, function, target, row["target_name"], rodata,
        data_reference=(row["status"] == "reviewed"
                        and row["kind"] in {"mips_hi16_lo16", "mips32_candidate"}),
    )

    if row["kind"] == "mips26":
        local_target = function.contains(target)
        _put_word(blob, offset, encode_mips26_addend(validation.word, addend))
        relocation = MipsRelocation(offset, "R_MIPS_26", symbol)
        action = "local-section" if local_target else "external-symbol"
        kinds = "R_MIPS_26"
        relocations = [relocation]
    elif row["kind"] == "mips_hi16_lo16":
        paired_offset = validation.paired_offset
        assert paired_offset is not None and validation.paired_word is not None
        hi_word, lo_word = encode_hi_lo_addend(
            validation.word,
            validation.paired_word,
            addend,
        )
        _put_word(blob, offset, hi_word)
        _put_word(blob, paired_offset, lo_word)
        relocations = [
            MipsRelocation(offset, "R_MIPS_HI16", symbol),
            MipsRelocation(paired_offset, "R_MIPS_LO16", symbol),
        ]
        action = "paired-symbol"
        kinds = "R_MIPS_HI16+R_MIPS_LO16"
    elif row["kind"] == "mips32_candidate" and (
        policy == "all" or row["status"] == "reviewed"
    ):
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
        "addend": format_hex(addend) if addend >= 0 else f"-{format_hex(-addend)}",
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


def _write_bytes_if_changed(path: Path, content: bytes) -> None:
    if path.is_file() and path.read_bytes() == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.tmp")
    temporary.write_bytes(content)
    temporary.replace(path)


@dataclass(frozen=True)
class Module:
    """A manifested unit: one contiguous run of admitted functions."""

    image: str
    unit: str
    stem: str
    vas: tuple[int, ...]
    data: tuple[Datum, ...] = ()
    rodata: tuple[int, int] | None = None

    @property
    def object_name(self) -> str:
        return f"{self.vas[0]:08x}_{self.stem}.o"

    @property
    def text_start(self) -> int:
        return self.vas[0]

    def owns_rodata(self, address: int) -> bool:
        return self.rodata is not None and self.rodata[0] <= address < self.rodata[0] + self.rodata[1]


def _get_word(blob: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", blob, offset)[0]


def _rebase_section_relocations(
    blob: bytearray, relocations: list[MipsRelocation], offset: int
) -> None:
    """Add ``offset`` to every implicit addend that is relative to ``.text``."""
    pending_hi: MipsRelocation | None = None
    for item in relocations:
        if item.symbol != SECTION_SYMBOL:
            continue
        if item.kind == "R_MIPS_26":
            word = _get_word(blob, item.offset)
            addend = (word & 0x03FFFFFF) << 2
            _put_word(blob, item.offset, encode_mips26_addend(word, addend + offset))
        elif item.kind == "R_MIPS_HI16":
            pending_hi = item
        elif item.kind == "R_MIPS_LO16" and pending_hi is not None:
            hi_word = _get_word(blob, pending_hi.offset)
            lo_word = _get_word(blob, item.offset)
            addend = decode_hi_lo_target(hi_word, lo_word)
            new_hi, new_lo = encode_hi_lo_addend(hi_word, lo_word, addend + offset)
            _put_word(blob, pending_hi.offset, new_hi)
            _put_word(blob, item.offset, new_lo)
            pending_hi = None


@dataclass(frozen=True)
class ModuleImage:
    """One module target object plus the sizes recorded in objects.tsv."""

    data: bytes
    relocations: list[MipsRelocation]
    size: int
    body_size: int
    data_size: int
    bss_size: int
    rodata_size: int = 0


def _module_rodata(
    module: Module, blob: bytes, text_size: int
) -> tuple[bytes, list[MipsRelocation]]:
    """Rebase a module's carved read-only range: in-module code pointers become
    .text-relative words with R_MIPS_32 relocations; everything else (string
    literals, foreign pointers) keeps its retail bytes."""
    start = module.text_start
    end = start + text_size
    data = bytearray(blob)
    relocations: list[MipsRelocation] = []
    for offset in range(0, len(data) - 3, 4):
        word = _get_word(data, offset)
        if start <= word < end:
            _put_word(data, offset, word - start)
            relocations.append(MipsRelocation(offset, "R_MIPS_32", SECTION_SYMBOL))
    return bytes(data), relocations


def _module_data(
    module: Module,
    data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]],
) -> tuple[bytes, list[DefinedSymbol], list[MipsRelocation], int, list[DefinedSymbol]]:
    """Lay out a module's claimed data (.data bytes) and bss (sizes only)."""
    data = bytearray()
    data_symbols: list[DefinedSymbol] = []
    data_relocations: list[MipsRelocation] = []
    bss_size = 0
    bss_symbols: list[DefinedSymbol] = []
    for datum in module.data:
        if datum.storage == "load":
            blob, relocations = data_blobs[datum.va]
            offset = (len(data) + datum.alignment - 1) & -datum.alignment
            data.extend(b"\0" * (offset - len(data)))
            data_relocations.extend(
                MipsRelocation(item.offset + offset, item.kind, item.symbol)
                for item in relocations
            )
            data_symbols.append(
                DefinedSymbol(datum.symbol, offset, datum.size, STT_OBJECT, datum.binding)
            )
            data.extend(blob)
        else:
            offset = (bss_size + datum.alignment - 1) & -datum.alignment
            bss_symbols.append(
                DefinedSymbol(datum.symbol, offset, datum.size, STT_OBJECT, datum.binding)
            )
            bss_size = offset + datum.size
    return bytes(data), data_symbols, data_relocations, bss_size, bss_symbols


def _module_object(
    module: Module,
    functions: dict[int, Function],
    carved: dict[int, tuple[bytes, list[MipsRelocation]]],
    data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]] | None = None,
    rodata_blob: bytes | None = None,
) -> ModuleImage:
    """Concatenate a module's carved functions into one .text section.

    Claimed data follows in ``.data``/``.bss`` so the object compares the whole
    translation-unit hypothesis, not only its code. Do not append the probe
    assembler's alignment tails: bytes outside the retail claims can belong to
    another object, and synthesized zeros are not recovered retail storage.
    """
    text = bytearray()
    relocations: list[MipsRelocation] = []
    symbols: list[DefinedSymbol] = []
    body_total = 0
    for va in module.vas:
        function = functions[va]
        if function.va != module.vas[0] + len(text):
            raise ValueError(
                f"module {module.unit}: {function.symbol} at {va:#x} is not contiguous "
                f"with the preceding function"
            )
        blob, function_relocations = carved[va]
        offset = len(text)
        rebased = bytearray(blob)
        # Section-relative addends were written relative to the function's own
        # object; inside the module section they are relative to the run start.
        if offset:
            _rebase_section_relocations(rebased, function_relocations, offset)
        relocations.extend(
            MipsRelocation(item.offset + offset, item.kind, item.symbol)
            for item in function_relocations
        )
        symbols.append(DefinedSymbol(function.symbol, offset, function.body_size, STT_FUNC))
        text += rebased
        body_total += function.body_size
    first = symbols[0]
    data, data_symbols, data_relocations, bss_size, bss_symbols = _module_data(
        module, data_blobs or {}
    )
    rodata, rodata_relocations = b"", []
    if module.rodata is not None:
        if rodata_blob is None:
            raise ValueError(f"module {module.unit}: RODATA range was not carved")
        rodata, rodata_relocations = _module_rodata(module, rodata_blob, len(text))
    # The section must carry the strongest constraint used to pack its members,
    # not an unrelated four-byte default. Relink validation still requires one
    # consistent base for every member; changing ELF alignment cannot fix gaps.
    return ModuleImage(
        write_mips_elf(
            bytes(text),
            first.name,
            first.size,
            relocations,
            symbols[1:],
            data=data,
            data_symbols=data_symbols,
            data_relocations=data_relocations,
            data_alignment=max((d.alignment for d in module.data if d.storage == "load"),
                               default=1),
            bss_size=bss_size,
            bss_symbols=bss_symbols,
            bss_alignment=max((d.alignment for d in module.data if d.storage == "bss"),
                              default=1),
            rodata=rodata,
            rodata_relocations=rodata_relocations,
        ),
        [*relocations, *data_relocations, *rodata_relocations],
        len(text),
        body_total,
        len(data),
        bss_size,
        len(rodata),
    )


def delink(
    exe_dir: Path,
    config_dir: Path,
    output_dir: Path,
    images: Iterable[str] = IMAGE_LAYOUTS,
    vas: Iterable[int] = (),
    policy: str = "safe",
    modules: Iterable[Module] = (),
) -> dict[str, dict[str, int]]:
    from scripts.kf.config_data import delink_object, load as load_contributions
    from scripts.kf.sema.image import RetailImage

    selected_images = tuple(dict.fromkeys(images))
    selected_vas = set(vas)
    modules = tuple(modules)
    contributions = load_contributions(config_dir, modules) if not selected_vas else ()
    modules_by_image: dict[str, list[Module]] = defaultdict(list)
    for module in modules:
        modules_by_image[module.image].append(module)
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
        selected_modules = [
            module for module in modules_by_image[image]
            if not selected_vas or set(module.vas) <= selected_vas
        ]
        claimed_load_data = sorted(
            (datum for module in selected_modules for datum in module.data
             if datum.storage == "load"),
            key=lambda item: item.va,
        )
        rows_by_owner: dict[int, list[dict[str, str]]] = defaultdict(list)
        rows_by_datum: dict[int, list[dict[str, str]]] = defaultdict(list)
        withheld_rows: list[dict[str, object]] = []
        for row in rows_by_image[image]:
            site = parse_int(row["site_va"])
            owner = _containing_function(catalog, image, site)
            if owner is None:
                datum = next(
                    (item for item in claimed_load_data if item.va <= site < item.end), None
                )
                if datum is not None:
                    rows_by_datum[datum.va].append(row)
                elif not selected_vas:
                    withheld_rows.append(_withheld(row, None, "site-outside-function"))
                continue
            if owner.va in selected_function_starts:
                rows_by_owner[owner.va].append(row)

        object_rows: list[dict[str, object]] = []
        used_rows: list[dict[str, object]] = []
        withheld_functions: list[dict[str, object]] = []
        carved: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
        # Members of a module that claims a RODATA range are carved a second
        # time with that range resolving to .rodata offsets.
        rodata_by_function = {
            va: module.rodata
            for module in selected_modules if module.rodata is not None
            for va in module.vas
        }
        carved_rodata: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
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
                if not _competes_for_site(row, policy):
                    continue
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
            if function.va in rodata_by_function:
                module_blob = bytearray(executable[start:start + function.size])
                module_relocations: list[MipsRelocation] = []
                for row in rows_by_owner[function.va]:
                    sites = [parse_int(row["site_va"])]
                    if row["paired_site_va"]:
                        sites.append(parse_int(row["paired_site_va"]))
                    if any(occupied[site] > 1 for site in sites):
                        continue
                    try:
                        relocations, _used = _apply_relocation(
                            module_blob, function, row, catalog, policy,
                            rodata_by_function[function.va],
                        )
                    except ValueError:
                        continue
                    module_relocations.extend(relocations)
                carved_rodata[function.va] = (bytes(module_blob), module_relocations)

            object_name = f"{function.va:08x}_{function.symbol}.o"
            object_relative = Path("objects") / object_name
            carved[function.va] = (bytes(blob), function_relocations)
            _write_bytes_if_changed(object_output / object_name, write_mips_elf(
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
                "unit": "",
                "scope": function.scope,
                "provider": function.provider,
                "library": function.library,
                "object": object_relative.as_posix(),
                "relocations": len(function_relocations),
                "data_size": "0x0",
                "bss_size": "0x0",
                "rodata_size": "0x0",
                "confidence": function.confidence,
                "provenance": function.provenance,
            })

        # Claimed load data is carved from the image the same way; every
        # relocation candidate sited inside it goes through the shared
        # validator. Raw pointer words stay withheld under the safe policy
        # unless their exact site, target, and semantic owner were reviewed.
        data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
        for datum in claimed_load_data:
            start = expected.file_offset(datum.va)
            blob = bytearray(executable[start:start + datum.size])
            if len(blob) != datum.size:
                raise ValueError(f"{exe_path}: truncated datum {datum.va:#x}")
            owner = _DataOwner(image, datum)
            datum_relocations: list[MipsRelocation] = []
            for row in rows_by_datum[datum.va]:
                try:
                    relocations, used = _apply_relocation(blob, owner, row, catalog, policy)
                except ValueError as error:
                    withheld_rows.append(_withheld(row, None, f"data:{error}"))
                    continue
                datum_relocations.extend(relocations)
                used_rows.append(used)
            data_blobs[datum.va] = (bytes(blob), datum_relocations)

        live_objects = {Path(str(row["object"])).name for row in object_rows}
        for old_object in object_output.glob("*.o"):
            if old_object.name not in live_objects:
                old_object.unlink()

        # Manifested units are carved again as one contiguous section each, so
        # objdiff compares a translation unit against the retail run it claims.
        module_output = image_output / "modules"
        module_output.mkdir(parents=True, exist_ok=True)
        live_modules: set[str] = set()
        for module in selected_modules:
            if any(va not in carved for va in module.vas):
                missing = [f"{va:#x}" for va in module.vas if va not in carved]
                raise ValueError(f"module {module.unit}: functions not carved: {missing}")
            module_carved = dict(carved)
            module_carved.update({va: carved_rodata[va] for va in module.vas if va in carved_rodata})
            rodata_blob = None
            if module.rodata is not None:
                rodata_start = expected.file_offset(module.rodata[0])
                rodata_blob = executable[rodata_start:rodata_start + module.rodata[1]]
                if len(rodata_blob) != module.rodata[1]:
                    raise ValueError(f"{exe_path}: truncated RODATA range for {module.unit}")
            built = _module_object(
                module, catalog.function_starts[image], module_carved, data_blobs, rodata_blob
            )
            _write_bytes_if_changed(module_output / module.object_name, built.data)
            live_modules.add(module.object_name)
            first = catalog.function_starts[image][module.vas[0]]
            object_rows.append({
                "image": image,
                "va": format_hex(module.vas[0]),
                "size": format_size(built.size),
                "body_size": format_size(built.body_size),
                "name": module.stem,
                "unit": module.unit,
                "scope": "module",
                "provider": "",
                "library": "",
                "object": (Path("modules") / module.object_name).as_posix(),
                "relocations": len(built.relocations),
                "data_size": format_size(built.data_size),
                "bss_size": format_size(built.bss_size),
                "rodata_size": format_size(built.rodata_size),
                "confidence": first.confidence,
                "provenance": "config/units.toml",
            })
        if not selected_vas:
            for old_object in module_output.glob("*.o"):
                if old_object.name not in live_modules:
                    old_object.unlink()

        data_output = image_output / "data"
        live_data = set()
        for contribution in contributions:
            if contribution.image != image:
                continue
            blob = delink_object(contribution, RetailImage(image, expected, executable, exe_path),
                                 relocation_rows)
            _write_bytes_if_changed(contribution.target_path(output_dir), blob)
            live_data.add(contribution.object_name)
            object_rows.append({
                "image": image, "va": format_hex(contribution.va),
                "size": format_size(contribution.size), "body_size": "0x0",
                "name": contribution.identity, "unit": contribution.unit,
                "scope": "config-data", "provider": contribution.provider,
                "library": contribution.library,
                "object": (Path("data") / contribution.object_name).as_posix(),
                "relocations": 0, "data_size": format_size(contribution.size),
                "bss_size": "0x0", "rodata_size": "0x0", "confidence": "supported",
                "provenance": contribution.evidence,
            })
        if not selected_vas and data_output.is_dir():
            for old_object in data_output.glob("*.o"):
                if old_object.name not in live_data:
                    old_object.unlink()

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
    parser.add_argument(
        "--manifest",
        type=Path,
        help="unit manifest whose modules are carved as well (default: config/units.toml)",
    )
    args = parser.parse_args()
    modules: tuple[Module, ...] = ()
    manifest_path = args.manifest or Path("config/units.toml")
    if manifest_path.is_file():
        from scripts.kf.manifest import load as load_manifest

        modules = load_manifest(manifest_path, config_dir=args.config_dir).modules()
    results = delink(
        args.exe_dir,
        args.config_dir,
        args.output_dir,
        args.image or IMAGE_LAYOUTS,
        args.va,
        args.policy,
        modules,
    )
    for image, counts in results.items():
        print(image + ": " + ", ".join(
            f"{name}={count}" for name, count in counts.items()
        ))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
