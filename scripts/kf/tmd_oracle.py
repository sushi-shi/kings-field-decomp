"""Run GAME's retail and candidate TMD packet parser on identical payloads.

This is an isolated machine-code oracle, not a PlayStation boot harness.  It
maps GAME.EXE's load image and enough zeroed RAM for one call to
``tmd_prepare_primitive_indices``.  The candidate run replaces only that
function and its compiler-emitted jump table with relocated bytes from the
current reconstruction object.  Both runs start from the same pristine TMD
payload and the complete mutated payloads are compared.

The default corpus is read from the locally configured retail disc: the two
raw TMD chunks and all asset archives in B1..B5/MIXB.DAT, plus B5/CHR*.MIM.
No retail resource bytes are written to the repository.
"""

from __future__ import annotations

import argparse
import struct
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Mapping, Sequence

from elftools.elf.elffile import ELFFile

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.parser_machine import (
    LinkedFunction, LinkedProgram, MemoryInput, MemoryPatch, MemoryRange, ParserMachine,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import BUILD, REPO
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.relocations import (
    decode_hi_lo_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
)
from scripts.kf.sema.image import RetailImage
from scripts.kf.rust_codec import RustCodec, build_driver


PARSER_VA = 0x8001C2B0
CURRENT_ASSET_PTR_VA = 0x80090FC8
CANDIDATE_RODATA_VA = 0x80100000
PAYLOAD_VA = 0x80120000
STACK_VA = 0x801FF000
DEFAULT_INSTRUCTION_LIMIT = 20_000_000
DEFAULT_CANDIDATE = BUILD / "objdiff/game/base/8001b7b0_render.o"

# The current source migration names the complete owner; older candidate
# objects name its interior TMD subobject.  Both resolve to the retail BSS span
# and, after their implicit field addends, to CURRENT_ASSET_PTR_VA.
GAME_EXTERNALS = {
    "graphics_context": 0x80090EBC,
    "tmd_state": 0x80090FA8,
}

EXPECTED_SHIPPED_OBJECTS = 1_911
EXPECTED_SHIPPED_PACKETS = 117_119
EXPECTED_SHIPPED_MODES = {
    0x20: 27_216,
    0x24: 16_889,
    0x28: 2_407,
    0x2C: 19_844,
    0x30: 31_150,
    0x34: 16_440,
    0x38: 3_129,
    0x3C: 44,
}

# Halfword fields rewritten by each masked packet mode, relative to the packet
# body after its four-byte olen/ilen/flag/mode header.
INDEX_FIELDS = {
    0x20: (4, 6, 8, 10),
    0x24: (12, 14, 16, 18),
    0x28: (4, 6, 8, 10, 12),
    0x2C: (16, 18, 20, 22, 24),
    0x30: (4, 6, 8, 10, 12, 14),
    0x34: (12, 14, 16, 18, 20, 22),
    0x38: (4, 6, 8, 10, 12, 14, 16, 18),
    0x3C: (16, 18, 20, 22, 24, 26, 28, 30),
}
MODE_INPUT_WORDS = {
    0x20: 3,
    0x24: 5,
    0x28: 4,
    0x2C: 7,
    0x30: 4,
    0x34: 6,
    0x38: 5,
    0x3C: 8,
}


@dataclass(frozen=True)
class TmdCase:
    label: str
    payload: bytes
    tmd_offset: int


@dataclass(frozen=True)
class TmdScan:
    objects: int
    packets: int
    modes: Counter[int]


@dataclass(frozen=True)
class MachineResult:
    payload: bytes
    instructions: int


@dataclass(frozen=True)
class CandidatePatch:
    text_va: int
    text: bytes
    rodata_va: int
    rodata: bytes


@dataclass(frozen=True)
class Relocation:
    offset: int
    kind: int
    symbol: str
    symbol_value: int


def _u16(data: bytes | bytearray, offset: int, context: str) -> int:
    if offset < 0 or offset + 2 > len(data):
        raise ValueError(f"{context}: missing u16 at {offset:#x}")
    return struct.unpack_from("<H", data, offset)[0]


def _u32(data: bytes | bytearray, offset: int, context: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValueError(f"{context}: missing u32 at {offset:#x}")
    return struct.unpack_from("<I", data, offset)[0]


def length_prefixed_chunks(data: bytes, label: str) -> tuple[bytes, ...]:
    """Split a GAME DAT stream whose chunks are ``u32 length; payload``."""
    chunks: list[bytes] = []
    cursor = 0
    while cursor < len(data):
        size = _u32(data, cursor, label)
        end = cursor + 4 + size
        if end <= cursor + 4 or end > len(data):
            raise ValueError(
                f"{label}: chunk {len(chunks)} at {cursor:#x} has invalid size {size:#x}"
            )
        chunks.append(data[cursor + 4:end])
        cursor = end
    if cursor != len(data):
        raise ValueError(f"{label}: chunk walk stopped at {cursor:#x} of {len(data):#x}")
    return tuple(chunks)


def asset_archive_cases(data: bytes, label: str) -> tuple[TmdCase, ...]:
    """Split the archive grammar consumed by asset_registry_load_tmd_archive."""
    count = _u16(data, 0, label)
    cursor = 4
    result: list[TmdCase] = []
    for index in range(count):
        size = _u32(data, cursor, f"{label} asset {index}")
        if size < 20 or cursor + size > len(data):
            raise ValueError(
                f"{label}: asset {index} at {cursor:#x} has invalid size {size:#x}"
            )
        asset = data[cursor:cursor + size]
        tmd_offset = _u32(asset, 8, f"{label} asset {index}")
        if tmd_offset > size - 12:
            raise ValueError(
                f"{label}: asset {index} TMD offset {tmd_offset:#x} exceeds size {size:#x}"
            )
        result.append(TmdCase(f"{label}/asset-{index:02d}", asset, tmd_offset))
        cursor += size
    if cursor != len(data):
        raise ValueError(f"{label}: asset walk stopped at {cursor:#x} of {len(data):#x}")
    return tuple(result)


def single_asset_case(data: bytes, label: str) -> TmdCase:
    """Validate one KfAssetHeader-prefixed resource and expose its TMD."""
    size = _u32(data, 0, label)
    if size != len(data) or size < 20:
        raise ValueError(f"{label}: asset size {size:#x} does not equal file size {len(data):#x}")
    tmd_offset = _u32(data, 8, label)
    if tmd_offset > size - 12:
        raise ValueError(f"{label}: TMD offset {tmd_offset:#x} exceeds size {size:#x}")
    return TmdCase(label, data, tmd_offset)


def shipped_cases(retail_dir: Path) -> tuple[TmdCase, ...]:
    """Enumerate the fixed SLPS-00017 TMD corpus without copying it in-tree."""
    root = retail_dir / "KF"
    result: list[TmdCase] = []
    for floor in range(1, 6):
        path = root / f"B{floor}" / "MIXB.DAT"
        chunks = length_prefixed_chunks(path.read_bytes(), f"B{floor}/MIXB.DAT")
        if len(chunks) < 4:
            raise ValueError(f"{path}: expected at least four chunks, found {len(chunks)}")
        result.extend(
            TmdCase(f"B{floor}/MIXB.DAT/raw-{index}", chunk, 0)
            for index, chunk in enumerate(chunks[:2])
        )
        for index, chunk in enumerate(chunks[2:], start=2):
            result.extend(asset_archive_cases(chunk, f"B{floor}/MIXB.DAT/chunk-{index}"))
    for path in sorted((root / "B5").glob("CHR*.MIM")):
        result.extend(asset_archive_cases(path.read_bytes(), f"B5/{path.name}"))
    common_chunks = length_prefixed_chunks(
        (root / "COM" / "COM.DAT").read_bytes(), "COM/COM.DAT"
    )
    result.append(single_asset_case(common_chunks[0], "COM/COM.DAT/chunk-0"))
    for path in sorted((root / "WEPON").glob("WEP*.MIM")):
        result.append(single_asset_case(path.read_bytes(), str(path.relative_to(root))))
    for path in sorted(root.glob("ITEM*/I*.TMD")):
        result.append(TmdCase(str(path.relative_to(root)), path.read_bytes(), 0))
    return tuple(result)


def scan_tmd(data: bytes | bytearray, tmd_offset: int, label: str) -> TmdScan:
    """Validate the parser's bounded walk and count its masked packet modes."""
    object_count = _u16(data, tmd_offset + 8, label)
    object_table = tmd_offset + 12
    if object_table + object_count * 28 > len(data):
        raise ValueError(f"{label}: {object_count} TMD objects exceed the payload")
    packets = 0
    modes: Counter[int] = Counter()
    for object_index in range(object_count):
        object_at = object_table + object_index * 28
        primitive_offset = _u32(data, object_at + 16, label)
        primitive_count = _u16(data, object_at + 20, label)
        packet = tmd_offset + 12 + primitive_offset
        for primitive_index in range(primitive_count):
            if packet + 4 > len(data):
                raise ValueError(
                    f"{label}: object {object_index} packet {primitive_index} header is truncated"
                )
            packet_size = 4 + data[packet + 1] * 4
            mode = data[packet + 3] & 0xFD
            indices = INDEX_FIELDS.get(mode, ())
            if (packet + packet_size > len(data)
                    or (indices and indices[-1] + 2 > packet_size - 4)):
                raise ValueError(
                    f"{label}: object {object_index} packet {primitive_index} "
                    f"has invalid input length {data[packet + 1]}"
                )
            modes[mode] += 1
            packets += 1
            packet += packet_size
    return TmdScan(object_count, packets, modes)


def reference_prepare(data: bytes, tmd_offset: int, label: str) -> bytes:
    """Small format model used only as a synthetic-case third-party check."""
    result = bytearray(data)
    object_count = _u16(result, tmd_offset + 8, label)
    object_table = tmd_offset + 12
    for object_index in range(object_count):
        object_at = object_table + object_index * 28
        primitive_offset = _u32(result, object_at + 16, label)
        primitive_count = _u16(result, object_at + 20, label)
        packet = tmd_offset + 12 + primitive_offset
        for _ in range(primitive_count):
            body = packet + 4
            mode = result[packet + 3] & 0xFD
            for relative in INDEX_FIELDS.get(mode, ()):  # unknown modes are unchanged
                value = _u16(result, body + relative, label)
                struct.pack_into("<H", result, body + relative, (value << 3) & 0xFFFF)
            packet += 4 + result[packet + 1] * 4
    return bytes(result)


def synthetic_cases() -> tuple[TmdCase, ...]:
    zero_objects = bytearray(12)
    struct.pack_into("<I", zero_objects, 0, 0x41)

    zero_primitives = bytearray(12 + 28)
    struct.pack_into("<I", zero_primitives, 0, 0x41)
    struct.pack_into("<H", zero_primitives, 8, 1)
    struct.pack_into("<I", zero_primitives, 12 + 16, 28)

    packets = bytearray()
    seed = 1
    for base_mode in INDEX_FIELDS:
        for mode in (base_mode, base_mode | 2):  # ABE bit must be ignored
            body_size = MODE_INPUT_WORDS[base_mode] * 4
            packet = bytearray(4 + body_size)
            packet[0] = MODE_INPUT_WORDS[base_mode]
            packet[1] = MODE_INPUT_WORDS[base_mode]
            packet[3] = mode
            for relative in INDEX_FIELDS[base_mode]:
                struct.pack_into("<H", packet, 4 + relative, seed)
                seed += 1
            packets.extend(packet)
    all_modes = bytearray(12 + 28)
    struct.pack_into("<I", all_modes, 0, 0x41)
    struct.pack_into("<H", all_modes, 8, 1)
    struct.pack_into("<I", all_modes, 12 + 16, 28)
    struct.pack_into("<H", all_modes, 12 + 20, 16)
    all_modes.extend(packets)

    # Defaults inside and outside the switch range must preserve their bytes
    # and still advance by ilen. A following known packet catches a bad cursor.
    defaults = bytearray(zero_primitives)
    struct.pack_into("<H", defaults, 12 + 20, 4)
    for mode, length in ((0x21, 0), (0x10, 1), (0x40, 2)):
        defaults.extend(bytes((length, length, 0x77, mode)) + bytes([0xa5]) * (length * 4))
    body = bytearray(MODE_INPUT_WORDS[0x20] * 4)
    for offset in INDEX_FIELDS[0x20]:
        struct.pack_into("<H", body, offset, 0xffff)
    defaults.extend(bytes((MODE_INPUT_WORDS[0x20], MODE_INPUT_WORDS[0x20], 0, 0x20)) + body)

    return (
        TmdCase("synthetic/zero-objects", bytes(zero_objects), 0),
        TmdCase("synthetic/one-object-zero-primitives", bytes(zero_primitives), 0),
        TmdCase("synthetic/unknown-modes-and-wrapping", bytes(defaults), 0),
        TmdCase("synthetic/all-modes-and-abe", bytes(all_modes), 0),
    )


def apply_relocations(data: bytes, relocations: Sequence[Relocation]) -> bytes:
    """Apply the MIPS REL forms needed by GAME parser code and jump tables."""
    result = bytearray(data)
    ordered = sorted(relocations, key=lambda item: item.offset)
    consumed_lows: set[int] = set()
    for index, relocation in enumerate(ordered):
        if relocation.kind == 6 and index in consumed_lows:  # R_MIPS_LO16
            continue
        if relocation.offset < 0 or relocation.offset + 4 > len(result):
            raise ValueError(f"relocation at {relocation.offset:#x} exceeds section")
        word = struct.unpack_from("<I", result, relocation.offset)[0]
        if relocation.kind == 2:  # R_MIPS_32
            struct.pack_into(
                "<I", result, relocation.offset, (word + relocation.symbol_value) & 0xFFFFFFFF
            )
        elif relocation.kind == 4:  # R_MIPS_26
            addend = (word & 0x03FFFFFF) << 2
            target = (relocation.symbol_value + addend) & 0xFFFFFFFF
            struct.pack_into("<I", result, relocation.offset, encode_mips26_addend(word, target))
        elif relocation.kind == 5:  # R_MIPS_HI16, paired with the next same-symbol LO16
            pair_index = next(
                (
                    candidate_index
                    for candidate_index in range(index + 1, len(ordered))
                    if candidate_index not in consumed_lows
                    and ordered[candidate_index].kind == 6
                    and ordered[candidate_index].symbol == relocation.symbol
                    and ordered[candidate_index].symbol_value == relocation.symbol_value
                ),
                None,
            )
            if pair_index is None:
                raise ValueError(f"unpaired R_MIPS_HI16 for {relocation.symbol}")
            low = ordered[pair_index]
            low_word = struct.unpack_from("<I", result, low.offset)[0]
            addend = decode_hi_lo_target(word, low_word)
            high_word, low_word = encode_hi_lo_addend(
                word, low_word, (relocation.symbol_value + addend) & 0xFFFFFFFF
            )
            struct.pack_into("<I", result, relocation.offset, high_word)
            struct.pack_into("<I", result, low.offset, low_word)
            consumed_lows.add(pair_index)
        elif relocation.kind == 6:
            raise ValueError(f"unpaired R_MIPS_LO16 for {relocation.symbol}")
        else:
            raise ValueError(
                f"unsupported MIPS relocation {relocation.kind} for {relocation.symbol}"
            )
    return bytes(result)


def _symbol_address(symbol, elf: ELFFile, section_bases: Mapping[str, int]) -> int:
    section_index = symbol["st_shndx"]
    if not isinstance(section_index, int):
        raise ValueError(f"symbol {symbol.name!r} is not section-defined")
    section = elf.get_section(section_index)
    try:
        base = section_bases[section.name]
    except KeyError as error:
        raise ValueError(f"no mapped base for section {section.name!r}") from error
    return base + int(symbol["st_value"])


def _elf_relocations(
    elf: ELFFile,
    section_name: str,
    section_bases: Mapping[str, int],
    externals: Mapping[str, int],
    included_range: range | None = None,
) -> tuple[Relocation, ...]:
    relocation_section = elf.get_section_by_name(f".rel{section_name}")
    if relocation_section is None:
        return ()
    symbols = elf.get_section(relocation_section["sh_link"])
    result: list[Relocation] = []
    for item in relocation_section.iter_relocations():
        offset = int(item["r_offset"])
        if included_range is not None and offset not in included_range:
            continue
        symbol = symbols.get_symbol(item["r_info_sym"])
        if isinstance(symbol["st_shndx"], int):
            value = _symbol_address(symbol, elf, section_bases)
            name = symbol.name or elf.get_section(symbol["st_shndx"]).name
        else:
            name = symbol.name
            try:
                value = externals[name]
            except KeyError as error:
                raise ValueError(f"unresolved candidate symbol {name!r}") from error
        result.append(Relocation(offset, int(item["r_info_type"]), name, value))
    return tuple(result)


def load_candidate_patch(
    path: Path,
    symbol_name: str = "tmd_prepare_primitive_indices",
) -> CandidatePatch:
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        symbols = elf.get_section_by_name(".symtab")
        matches = symbols.get_symbol_by_name(symbol_name) if symbols is not None else None
        if not matches or len(matches) != 1:
            raise ValueError(f"{path}: expected one symbol {symbol_name!r}")
        symbol = matches[0]
        if symbol["st_shndx"] != elf.get_section_index(".text"):
            raise ValueError(f"{path}: {symbol_name!r} is not defined in .text")
        function_offset = int(symbol["st_value"])
        function_size = int(symbol["st_size"])
        if function_size <= 0:
            raise ValueError(f"{path}: {symbol_name!r} has no extent")
        text_section = elf.get_section_by_name(".text")
        rodata_section = elf.get_section_by_name(".rodata")
        if text_section is None or rodata_section is None:
            raise ValueError(f"{path}: parser candidate needs .text and .rodata")
        text_base = PARSER_VA - function_offset
        section_bases = {".text": text_base, ".rodata": CANDIDATE_RODATA_VA}
        # A unit that owns the reconstructed aggregate defines graphics_context
        # in .bss; older objects referenced the interior tmd_state as undefined.
        # Derive an owned section's base from either known symbol before asking
        # the generic relocation resolver to place it.
        for known_name, known_va in GAME_EXTERNALS.items():
            known = symbols.get_symbol_by_name(known_name)
            if not known:
                continue
            for known_symbol in known:
                section_index = known_symbol["st_shndx"]
                if not isinstance(section_index, int):
                    continue
                section_name = elf.get_section(section_index).name
                inferred = known_va - int(known_symbol["st_value"])
                existing = section_bases.setdefault(section_name, inferred)
                if existing != inferred:
                    raise ValueError(
                        f"{path}: inconsistent {section_name} base from {known_name}"
                    )
        selected = range(function_offset, function_offset + function_size)
        text_relocations = _elf_relocations(
            elf, ".text", section_bases, GAME_EXTERNALS, selected
        )
        relocated_text = apply_relocations(text_section.data(), text_relocations)
        rodata_relocations = _elf_relocations(
            elf, ".rodata", section_bases, GAME_EXTERNALS
        )
        relocated_rodata = apply_relocations(rodata_section.data(), rodata_relocations)
        return CandidatePatch(
            PARSER_VA,
            relocated_text[function_offset:function_offset + function_size],
            CANDIDATE_RODATA_VA,
            relocated_rodata,
        )


def execute_case(
    retail: RetailImage,
    case: TmdCase,
    patch: CandidatePatch | None,
    instruction_limit: int,
) -> MachineResult:
    if PAYLOAD_VA + len(case.payload) > STACK_VA - 0x1000:
        raise ValueError(f"{case.label}: payload is too large for isolated RAM layout")
    name = "tmd_prepare_primitive_indices"
    patches = () if patch is None else (
        MemoryPatch("candidate TMD", patch.text_va, patch.text),
        MemoryPatch("candidate TMD switch table", patch.rodata_va, patch.rodata),
    )
    program = LinkedProgram((LinkedFunction(name, PARSER_VA, 0x300 if patch is None else len(patch.text),
                                           "retail" if patch is None else "candidate"),), patches)
    capture = MemoryRange("payload", PAYLOAD_VA, len(case.payload))
    result = ParserMachine(retail, program).call(
        name, memory=[MemoryInput(PAYLOAD_VA, case.payload),
                      MemoryInput(CURRENT_ASSET_PTR_VA, struct.pack("<I", PAYLOAD_VA + case.tmd_offset))],
        capture=[capture], allowed_writes=[capture], instruction_limit=instruction_limit,
    )
    return MachineResult(result.memory[0].data, result.instructions)


def _first_difference(left: bytes, right: bytes) -> int | None:
    for index, (a, b) in enumerate(zip(left, right, strict=True)):
        if a != b:
            return index
    return None


def compare_cases(
    retail: RetailImage,
    candidate: CandidatePatch,
    cases: Iterable[TmdCase],
    instruction_limit: int = DEFAULT_INSTRUCTION_LIMIT,
    rust: RustCodec | None = None,
) -> tuple[int, int, int]:
    count = 0
    retail_instructions = 0
    candidate_instructions = 0
    for case in cases:
        # The static walk makes malformed offsets fail with a useful resource
        # label before either machine can leave its input buffer.
        scan_tmd(case.payload, case.tmd_offset, case.label)
        expected = (
            reference_prepare(case.payload, case.tmd_offset, case.label)
            if case.label.startswith("synthetic/")
            else None
        )
        retail_result = execute_case(retail, case, None, instruction_limit)
        candidate_result = execute_case(retail, case, candidate, instruction_limit)
        if retail_result.payload != candidate_result.payload:
            offset = _first_difference(retail_result.payload, candidate_result.payload)
            assert offset is not None
            raise RuntimeError(
                f"{case.label}: retail/candidate differ at payload+{offset:#x}: "
                f"{retail_result.payload[offset]:#04x} != {candidate_result.payload[offset]:#04x}"
            )
        if expected is not None and retail_result.payload != expected:
            offset = _first_difference(retail_result.payload, expected)
            assert offset is not None
            raise RuntimeError(
                f"{case.label}: retail differs from the synthetic model at payload+{offset:#x}"
            )
        if rust is not None:
            (rust_result,) = rust.call("tmd", case.payload, struct.pack("<I", case.tmd_offset))
            if rust_result != retail_result.payload:
                offset = _first_difference(retail_result.payload, rust_result)
                raise RuntimeError(f"{case.label}: retail/Rust differ at payload+{offset:#x}")
        count += 1
        retail_instructions += retail_result.instructions
        candidate_instructions += candidate_result.instructions
    return count, retail_instructions, candidate_instructions


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--candidate-object", type=Path, default=DEFAULT_CANDIDATE)
    parser.add_argument("--rust-driver", type=Path, help="use an already-built Rust host driver")
    parser.add_argument("--no-rebuild", action="store_true", help="reuse the existing C object explicitly")
    parser.add_argument(
        "--synthetic-only",
        action="store_true",
        help="run only zero-object, zero-primitive, and all-mode generated cases",
    )
    parser.add_argument("--instruction-limit", type=int, default=DEFAULT_INSTRUCTION_LIMIT)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        retail_dir = configured_retail_dir(args.retail_dir)
        retail = RetailImage.load("GAME.EXE") if args.retail_dir is None else RetailImage(
            "GAME.EXE",
            IMAGE_LAYOUTS["GAME.EXE"],
            (retail_dir / "GAME.EXE").read_bytes(),
            retail_dir / "GAME.EXE",
        )
        candidate_path = args.candidate_object.resolve()
        if not args.no_rebuild and candidate_path == DEFAULT_CANDIDATE.resolve():
            rebuild_units(["game.render"])
        candidate = load_candidate_patch(candidate_path)
        rust = RustCodec(args.rust_driver or build_driver())
        generated = synthetic_cases()
        shipped = () if args.synthetic_only else shipped_cases(retail_dir)

        if shipped:
            census = TmdScan(0, 0, Counter())
            for case in shipped:
                scan = scan_tmd(case.payload, case.tmd_offset, case.label)
                census = TmdScan(
                    census.objects + scan.objects,
                    census.packets + scan.packets,
                    census.modes + scan.modes,
                )
            if (
                len(shipped) != 246
                or census.objects != EXPECTED_SHIPPED_OBJECTS
                or census.packets != EXPECTED_SHIPPED_PACKETS
                or dict(census.modes) != EXPECTED_SHIPPED_MODES
            ):
                raise ValueError(
                    "shipped TMD census differs from SLPS-00017 expectations: "
                    f"payloads={len(shipped)} objects={census.objects} "
                    f"packets={census.packets} modes={dict(sorted(census.modes.items()))}"
                )
            print(
                f"[tmd-oracle] corpus: {len(shipped)} shipped payloads, "
                f"{census.objects} objects, {census.packets} packets"
            )

        count, retail_steps, candidate_steps = compare_cases(
            retail,
            candidate,
            (*generated, *shipped),
            args.instruction_limit,
            rust,
        )
        print(
            f"[tmd-oracle] PASS: {count} payloads; retail/C/Rust complete mutated bytes agree "
            f"(retail {retail_steps} instructions, candidate {candidate_steps})"
        )
        try:
            display_path = candidate_path.relative_to(REPO)
        except ValueError:
            display_path = candidate_path
        print(f"[tmd-oracle] candidate: {display_path}")
        return 0
    except (OSError, RuntimeError, ValueError) as error:
        print(f"tmd-oracle: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
