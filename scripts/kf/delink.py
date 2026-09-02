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
    STT_FUNC, DefinedSymbol, MipsRelocation, SECTION_SYMBOL, write_mips_elf,
)
from scripts.kf.relocations import (
    decode_hi_lo_target,
    decode_mips26_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
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
) -> tuple[str, int]:
    if function.contains(target):
        return SECTION_SYMBOL, target - function.va
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
            owner = next(
                (item for item in catalog.data[function.image] if item.va == owner_va),
                None,
            )
            if owner is not None:
                return owner.symbol, target - owner_va
            return symbol, target - owner_va
        # A reviewed row may name the owning datum even when the address lies
        # outside its extent: `&table[index - 1]` folds to `table - stride`.
        # The addend is measured from that owner, wherever the target lies.
        named = next(
            (item for item in catalog.data[function.image] if item.symbol == symbol),
            None,
        )
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
) -> tuple[list[MipsRelocation], dict[str, object]]:
    validation = validate_relocation(blob, function, row, catalog, policy)
    target = validation.target
    offset = validation.offset
    symbol, addend = _resolve_symbol(catalog, function, target, row["target_name"])

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
    elif row["kind"] == "mips32_candidate" and policy == "all":
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

    @property
    def object_name(self) -> str:
        return f"{self.vas[0]:08x}_{self.stem}.o"


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


def _module_object(
    module: Module,
    functions: dict[int, Function],
    carved: dict[int, tuple[bytes, list[MipsRelocation]]],
) -> tuple[bytes, list[MipsRelocation], int, int]:
    """Concatenate a module's carved functions into one .text section."""
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
    return (
        write_mips_elf(bytes(text), first.name, first.size, relocations, symbols[1:]),
        relocations,
        len(text),
        body_total,
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
    selected_images = tuple(dict.fromkeys(images))
    selected_vas = set(vas)
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
        carved: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
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
                "confidence": function.confidence,
                "provenance": function.provenance,
            })

        live_objects = {Path(str(row["object"])).name for row in object_rows}
        for old_object in object_output.glob("*.o"):
            if old_object.name not in live_objects:
                old_object.unlink()

        # Manifested units are carved again as one contiguous section each, so
        # objdiff compares a translation unit against the retail run it claims.
        module_output = image_output / "modules"
        module_output.mkdir(parents=True, exist_ok=True)
        live_modules: set[str] = set()
        for module in modules_by_image[image]:
            if selected_vas and not set(module.vas) <= selected_vas:
                continue
            if any(va not in carved for va in module.vas):
                missing = [f"{va:#x}" for va in module.vas if va not in carved]
                raise ValueError(f"module {module.unit}: functions not carved: {missing}")
            data, module_relocations, size, body_total = _module_object(
                module, catalog.function_starts[image], carved
            )
            _write_bytes_if_changed(module_output / module.object_name, data)
            live_modules.add(module.object_name)
            first = catalog.function_starts[image][module.vas[0]]
            object_rows.append({
                "image": image,
                "va": format_hex(module.vas[0]),
                "size": format_size(size),
                "body_size": format_size(body_total),
                "name": module.stem,
                "unit": module.unit,
                "scope": "module",
                "provider": "",
                "library": "",
                "object": (Path("modules") / module.object_name).as_posix(),
                "relocations": len(module_relocations),
                "confidence": first.confidence,
                "provenance": "config/units.toml",
            })
        if not selected_vas:
            for old_object in module_output.glob("*.o"):
                if old_object.name not in live_modules:
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
