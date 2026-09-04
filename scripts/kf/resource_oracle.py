"""Three-way GAME resource parser comparisons on initialized retail resources.

Only explicitly selected game functions execute. GPU/CD services are bounded
deterministic hooks; TIM's Sony decoder is an explicitly shared retail provider,
not a claimed C reconstruction. Rust decodes TIM independently.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import (
    CandidateFunction,
    CandidateProgram,
    ExternalHook,
    GameSymbols,
    HookContext,
    MemoryInput,
    MemoryRange,
    ParserMachine,
    RetailProgram,
    RetailProvider,
)
from scripts.kf.paths import BUILD
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage
from scripts.kf.tmd_oracle import length_prefixed_chunks


INPUT_VA = 0x800B0000
OUTPUT_VA = 0x80180000
RESOURCES_OBJECT = BUILD / "objdiff/game/base/8001b100_resources.o"


def pattern(size: int, seed: int = 0) -> bytes:
    return bytes((index * 73 + (index >> 3) * 19 + seed * 41 + 17) & 255 for index in range(size))


def require_equal(label: str, retail: bytes, candidate: bytes, rust: bytes) -> None:
    for version, result in (("C", candidate), ("Rust", rust)):
        if result != retail:
            first = next(
                (i for i, (a, b) in enumerate(zip(retail, result)) if a != b),
                min(len(retail), len(result)),
            )
            raise AssertionError(
                f"{label}: retail/{version} mismatch at byte {first:#x}; "
                f"lengths {len(retail)}/{len(result)}; "
                f"retail={retail[first : first + 16].hex()} "
                f"{version}={result[first : first + 16].hex()}"
            )


@dataclass(frozen=True)
class RecordCase:
    label: str
    function: str
    object_path: Path
    operation: str
    source: bytes
    destination: int
    size: int
    word_copy: bool = False


def record_cases(root: Path, symbols: GameSymbols) -> list[RecordCase]:
    common = (root / "KF/COM/COM.DAT").read_bytes()
    chunks = length_prefixed_chunks(common, "COM.DAT")
    offsets = []
    at = 0
    for chunk in chunks:
        offsets.append(at + 4)
        at += 4 + len(chunk)
    specs = (
        (
            1,
            "map_resource_copy_words",
            "8001b100_resources.o",
            "records-render",
            "render_cell_windows",
            3264,
            True,
        ),
        (
            2,
            "weapon_records_load_and_mirror_angles",
            "800150a8_equipment.o",
            "records-weapons",
            "weapon_records",
            704,
            False,
        ),
        (
            3,
            "armor_records_load",
            "800150a8_equipment.o",
            "records-armor",
            "armor_records",
            1176,
            False,
        ),
        (4, "magic_load_records", "8003a244_magic.o", "records-magic", "magic_records", 480, False),
        (
            5,
            "map_object_definitions_load",
            "80030a98_map_object_pool.o",
            "records-objects",
            "map_object_state",
            1280,
            False,
        ),
        (
            6,
            "map_resource_copy_words",
            "8001b100_resources.o",
            "records-growth",
            "player_level_growth_table",
            480,
            True,
        ),
    )
    cases = []
    for chunk, function, obj, operation, datum, size, word_copy in specs:
        cases.append(
            RecordCase(
                f"COM/{operation}",
                function,
                BUILD / "objdiff/game/base" / obj,
                operation,
                common[offsets[chunk] : offsets[chunk] + size],
                symbols.datum(datum)[0],
                size,
                word_copy,
            )
        )
    for floor in range(1, 6):
        label = f"B{floor}/MIXA.DAT"
        mixa = length_prefixed_chunks((root / "KF" / label).read_bytes(), label)
        cases.append(
            RecordCase(
                f"B{floor}/actor-definitions",
                "actor_definitions_load",
                BUILD / "objdiff/game/base/80030818_actor_pool.o",
                "records-actors",
                mixa[6],
                symbols.datum("actor_state")[0],
                1824,
            )
        )
    # These bytes exercise signed/wrapping halfwords, not only shipped values.
    for case in tuple(cases[:6]):
        cases.append(
            RecordCase(
                f"synthetic/{case.operation}",
                case.function,
                case.object_path,
                case.operation,
                pattern(case.size, 9),
                case.destination,
                case.size,
                case.word_copy,
            )
        )
    return cases


def compare_records(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    cases = record_cases(root, symbols)
    for case in cases:
        capture = MemoryRange("destination", case.destination, case.size + 32)
        initial = pattern(capture.size)
        memory = [MemoryInput(INPUT_VA, case.source), MemoryInput(capture.address, initial)]
        args = (case.destination, INPUT_VA, case.size // 4) if case.word_copy else (INPUT_VA,)
        retail_program = RetailProgram.link(symbols, [case.function])
        candidate_program = CandidateProgram.link(
            symbols,
            [CandidateFunction(case.function, case.object_path)],
        )
        results = [
            ParserMachine(retail, program).call(
                case.function,
                args,
                memory=memory,
                capture=[capture],
                allowed_writes=[capture],
            )
            for program in (retail_program, candidate_program)
        ]
        (rust_bytes,) = rust.call(case.operation, case.source, initial)
        require_equal(case.label, results[0].memory[0].data, results[1].memory[0].data, rust_bytes)
        if case.word_copy:
            assert results[0].v0 == results[1].v0 == INPUT_VA + case.size
        print(f"[resource-oracle] PASS {case.label}: {capture.size} destination bytes", flush=True)
    return len(cases)


def compare_archives(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    # Only TMD/registry globals below this address are live in this closure.
    # The largest archive fits below the separately relocated candidate code.
    payload_va = 0x80093000
    selected_initial = 0x13572468
    cases = []
    for floor in range(1, 6):
        label = f"KF/B{floor}/MIXB.DAT"
        chunks = length_prefixed_chunks((root / label).read_bytes(), label)
        for slot, chunk in enumerate(chunks[:2]):
            cases.append((f"{label}/raw-{slot}", chunk, slot, "register-tmd"))
        for chunk, slot in zip(chunks[2:], (10, 30, 0)):
            cases.append((f"{label}/archive-{slot}", chunk, slot, "register-archive"))
    for path in sorted((root / "KF/B5").glob("CHR*.MIM")):
        cases.append((str(path.relative_to(root)), path.read_bytes(), 0, "register-archive"))
    common = length_prefixed_chunks((root / "KF/COM/COM.DAT").read_bytes(), "COM")[0]
    cases.append(("COM/common-asset", common, 21, "register-asset"))
    weapons = sorted((root / "KF/WEPON").glob("WEP*.MIM"))
    items = sorted((root / "KF").glob("ITEM*/I*.TMD"))
    if len(weapons) != 16 or len(items) != 67:
        raise AssertionError(f"registry corpus changed: {len(weapons)} weapons, {len(items)} items")
    for path in weapons:
        cases.append((str(path.relative_to(root)), path.read_bytes(), 20, "register-asset"))
    for path in items:
        cases.append((str(path.relative_to(root)), path.read_bytes(), 2, "register-tmd"))
    cases.append(("synthetic/empty-archive", b"\0\0\x73\x19", 63, "register-archive"))
    for label, payload, first, operation in cases:
        if len(payload) > 0x6D000:
            raise AssertionError(f"{label}: archive exceeds isolated input arena")
        if operation == "register-tmd":
            entry = "tmd_register"
            table_va = symbols.datum("tmd_state")[0]
            table = pattern(32, 7)
            functions = [CandidateFunction(entry, BUILD / "objdiff/game/base/8001b7b0_render.o")]
        else:
            entry = (
                "asset_registry_load_tmd_archive"
                if operation == "register-archive"
                else "asset_registry_set"
            )
            table_va = symbols.datum("asset_registry_entries")[0]
            table = pattern(256, 7)
            functions = [
                CandidateFunction(name, BUILD / "objdiff/game/base/800204c0_asset_registry.o")
                for name in (entry, "asset_registry_select")
            ]
        functions.append(
            CandidateFunction(
                "tmd_prepare_primitive_indices", BUILD / "objdiff/game/base/8001b7b0_render.o"
            )
        )
        programs = [
            RetailProgram.link(symbols, [item.name for item in functions]),
            CandidateProgram.link(symbols, functions),
        ]
        captures = [
            MemoryRange("payload", payload_va, len(payload)),
            MemoryRange("registry", table_va, len(table)),
            MemoryRange("selected TMD", 0x80090FC8, 4),
        ]
        memory = [
            MemoryInput(payload_va, payload),
            MemoryInput(table_va, table),
            MemoryInput(0x80090FC8, struct.pack("<I", selected_initial)),
        ]
        outputs = [
            ParserMachine(retail, program).call(
                entry,
                (first, payload_va),
                memory=memory,
                capture=captures,
                allowed_writes=captures,
            )
            for program in programs
        ]
        rust_output = rust.call(
            operation, payload, table, struct.pack("<HII", first, payload_va, selected_initial)
        )
        for index, capture in enumerate(captures):
            require_equal(
                f"{label}/{capture.name}",
                outputs[0].memory[index].data,
                outputs[1].memory[index].data,
                rust_output[index],
            )
        print(
            f"[resource-oracle] PASS {label}: mutated asset bytes, {len(table) // 4}-slot registry window, selected pointer",
            flush=True,
        )
    return len(cases)


def compare_common(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    source = (root / "KF/COM/COM.DAT").read_bytes()
    record_specs = record_cases(root, symbols)[:6]
    captures = [MemoryRange(case.operation, case.destination, case.size) for case in record_specs]
    captures += [
        MemoryRange("registry", symbols.datum("asset_registry_entries")[0], 256),
        MemoryRange("COM payload", INPUT_VA, len(source)),
        MemoryRange("selected TMD", 0x80090FC8, 4),
        MemoryRange("arena cursor", symbols.datum("memory_arena_cursor")[0], 4),
    ]
    initial = [pattern(item.size, i + 11) for i, item in enumerate(captures[:7])]
    functions = [CandidateFunction("common_resources_load", RESOURCES_OBJECT)]
    functions += [
        CandidateFunction(case.function, case.object_path)
        for case in record_specs
        if not case.word_copy
    ]
    functions += [
        CandidateFunction(name, BUILD / "objdiff/game/base/800204c0_asset_registry.o")
        for name in ("asset_registry_set", "asset_registry_select")
    ]
    functions += [
        CandidateFunction(
            "tmd_prepare_primitive_indices", BUILD / "objdiff/game/base/8001b7b0_render.o"
        )
    ]
    results = []
    for candidate in (False, True):
        effects = []

        def load(context: HookContext) -> int:
            path = context.read(context.args[1], 20).split(b"\0")[0]
            effects.append(("load", path))
            if path == b"COM\\MIX.TIM":
                pointer = 0x800B8000
            elif path == b"COM\\COM.DAT":
                pointer = INPUT_VA
            else:
                raise AssertionError(f"unexpected common-resource path {path!r}")
            context.write_u32(context.args[0], pointer)
            return 0

        def upload(context: HookContext) -> int:
            effects.append(("upload", context.args[0]))
            return 0

        def release(_context: HookContext) -> int:
            effects.append(("release",))
            return 0

        hooks = [
            ExternalHook("cd_file_load_allocated", load),
            ExternalHook("tim_upload_images", upload),
            ExternalHook("memory_release_last", release),
        ]
        program = (
            CandidateProgram.link(symbols, functions, hooks=hooks)
            if candidate
            else RetailProgram.link(symbols, [f.name for f in functions], hooks=hooks)
        )
        result = ParserMachine(retail, program).call(
            "common_resources_load",
            memory=[
                MemoryInput(INPUT_VA, source),
                *[MemoryInput(item.address, data) for item, data in zip(captures[:7], initial)],
            ],
            capture=captures,
            allowed_writes=captures,
        )
        if effects != [
            ("load", b"COM\\MIX.TIM"),
            ("upload", 0x800B8000),
            ("release",),
            ("load", b"COM\\COM.DAT"),
            ("release",),
        ]:
            raise AssertionError(f"common resources: unexpected I/O order {effects}")
        results.append(result)
    rust_output = rust.call("common-load", source, *initial, struct.pack("<I", INPUT_VA))
    for index, item in enumerate(captures):
        require_equal(
            f"common/{item.name}",
            results[0].memory[index].data,
            results[1].memory[index].data,
            rust_output[index],
        )
    print(
        "[resource-oracle] PASS common outer loader: six tables, registry, mutated COM, pointers, I/O order",
        flush=True,
    )
    return 1


def tim_paths(root: Path) -> list[Path]:
    files = []
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if relative.as_posix() in {"KF/B0/MIX0.", "KF/B0/MIX3.", "KF/B0/MIX9.", "E3."}:
            continue
        with path.open("rb") as stream:
            if stream.read(4) == b"\x10\0\0\0":
                files.append(path)
    return files


def compare_tim(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    providers = [RetailProvider(name) for name in ("OpenTIM", "ReadTIM", "get_tim_addr")]
    trace = bytearray()
    descriptors = bytearray()
    descriptor_address = 0
    syncs: list[int] = []

    def debug(context: HookContext) -> int:
        nonlocal descriptor_address
        # The admitted SDK worker holds its caller's TIM_IMAGE in $s1
        # (move s1,a1 at GAME 0x80052ccc). This does not assume a C frame offset.
        descriptor_address = context.register(17)
        return 0

    def upload(context: HookContext) -> int:
        if descriptor_address == 0:
            raise AssertionError("TIM upload without a decoded descriptor")
        descriptors.extend(context.read(descriptor_address, 20))
        rectangle = context.read(context.args[0], 8)
        _x, _y, width, height = struct.unpack("<4h", rectangle)
        if width < 0 or height < 0:
            raise AssertionError("negative upload rectangle")
        pixels = context.read(context.args[1], width * height * 2)
        trace.extend(rectangle + struct.pack("<I", len(pixels)) + pixels)
        syncs.append(-1)
        return 0

    def sync(context: HookContext) -> int:
        if not syncs or syncs[-1] != -1 or context.args[0] != 0:
            raise AssertionError("DrawSync ordering or argument mismatch")
        syncs[-1] = 0
        return 0

    hooks = [
        ExternalHook("LoadImage", upload),
        ExternalHook("DrawSync", sync),
        ExternalHook("GetGraphDebug", debug),
    ]
    programs = [
        RetailProgram.link(symbols, ["tim_upload_images"], hooks=hooks, providers=providers),
        CandidateProgram.link(
            symbols,
            [CandidateFunction("tim_upload_images", RESOURCES_OBJECT)],
            hooks=hooks,
            providers=providers,
        ),
    ]
    paths = tim_paths(root)
    if len(paths) != 297:
        raise AssertionError(f"GAME TIM corpus changed: {len(paths)} files, expected 297")
    # The SDK cursor is the sole non-stack write by these admitted functions.
    cursor = MemoryRange("SDK TIM cursor", 0x80058030, 4)
    # TIM needs no game BSS. Its texture bundles can therefore use that area
    # without overlapping the load image, SDK cursor, or private candidate code.
    tim_input = 0x80060000
    uploads = 0
    cases = [(str(path.relative_to(root)), path.read_bytes()) for path in paths]
    # The shipped GAME records all have CLUTs. Exercise the zeroed descriptor
    # pointers, opaque mode bits, zero-area rectangles and word-truncated size.
    cases += [
        ("synthetic/no-clut", struct.pack("<5I", 0x10, 2, 16, 0, 0x00010002) + b"\x11\x22\x33\x44"),
        ("synthetic/opaque-mode-zero-area", struct.pack("<5I", 0x10, 0x80000002, 15, 0, 0)),
    ]
    for label, payload in cases:
        if len(payload) + 4 > 0xA0000:
            raise AssertionError(f"{label}: TIM exceeds reserved input arena")
        outputs = []
        descriptor_outputs = []
        for program in programs:
            trace.clear()
            descriptors.clear()
            descriptor_address = 0
            syncs.clear()
            result = ParserMachine(retail, program).call(
                "tim_upload_images",
                (tim_input,),
                memory=[MemoryInput(tim_input, payload + bytes(4))],
                capture=[cursor],
                allowed_writes=[cursor],
            )
            if any(value != 0 for value in syncs):
                raise AssertionError(f"{label}: upload without DrawSync")
            if struct.unpack("<I", result.memory[0].data)[0] != tim_input + len(payload):
                raise AssertionError(f"{label}: SDK did not consume the complete TIM stream")
            outputs.append(bytes(trace))
            descriptor_outputs.append(bytes(descriptors))
        rust_bytes, rust_descriptors = rust.call("tim-state", payload, struct.pack("<I", tim_input))
        require_equal(label + "/uploads", outputs[0], outputs[1], rust_bytes)
        require_equal(label + "/TIM_IMAGE", descriptor_outputs[0], descriptor_outputs[1], rust_descriptors)
        uploads += len(syncs)
    print(
        f"[resource-oracle] PASS TIM: {len(paths)} files + 2 controls, {uploads} ordered CLUT/image uploads; "
        "retail/C/Rust full descriptors, rectangle and pixel bytes agree (C uses shared Sony TIM decoder)",
        flush=True,
    )
    return len(cases)


GRID_NAMES = (
    "map_cell_attribute_grid",
    "map_floor_height_grid",
    "map_cell_orientation_grid",
    "map_collision_flag_grid",
    "map_collision_grid",
)


def compare_stat(retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path) -> int:
    specs = (
        ("DAT_800580e8", 912),
        ("menu_window_layouts", 2376),
        ("item_name_rows", 1600),
        ("magic_name_rows", 180),
        ("item_buy_prices", 320),
        ("item_sell_prices", 320),
        ("cd_file_table", 80 * 20),
    )
    captures = [MemoryRange(name, symbols.datum(name)[0], size) for name, size in specs]
    initial = [pattern(size, index + 5) for index, (_name, size) in enumerate(specs)]
    source = (root / "KF/COM/STAT.DAT").read_bytes()
    responses = bytearray()
    for index in range(80):
        path = root / f"KF/ITEM{1 + index // 30}/I{index + 1:03}.TMD"
        record = bytearray(pattern(20, index))
        found = path.is_file()
        size = path.stat().st_size if found else 0
        if index == 0:
            # Successful whole-sector control: retail must preserve an
            # already aligned CdSearchFile size instead of adding a sector.
            if not found:
                raise AssertionError("STAT aligned-size control requires ITEM1/I001.TMD")
            size = 2 * 2048
        struct.pack_into("<I", record, 4, size)
        responses.extend(bytes([found]) + record)
    outputs = []
    for candidate in (False, True):
        paths = bytearray()
        io_order = []

        def load(context: HookContext) -> int:
            name = context.read(context.args[1], 13).split(b"\0")[0]
            if name != b"COM\\STAT.DAT":
                raise AssertionError(f"unexpected STAT load path {name!r}")
            io_order.append("load")
            context.write_u32(context.args[0], INPUT_VA)
            return 0

        def release(_context: HookContext) -> int:
            io_order.append("release")
            return 0

        def search(context: HookContext) -> int:
            index = len(paths) // 40
            if index >= 80 or io_order != ["load", "release"]:
                raise AssertionError("STAT CD search ordering differs")
            if context.args[0] != captures[-1].address + index * 20:
                raise AssertionError(f"STAT CD entry {index} pointer differs: {context.args[0]:#x}")
            paths.extend(context.read(context.args[1], 40))
            at = index * 21
            if not responses[at]:
                return 0
            context.write(context.args[0], bytes(responses[at + 1 : at + 21]))
            return context.args[0]

        def unexpected_exit(_context: HookContext) -> int:
            raise AssertionError("successful STAT read unexpectedly called exit")

        def memset(context: HookContext) -> int:
            destination, value, size = context.args[:3]
            context.write(destination, bytes([value & 255]) * size)
            return destination

        hooks = [
            ExternalHook("cd_file_load_allocated", load),
            ExternalHook("memory_release_last", release),
            ExternalHook("CdSearchFile", search),
            ExternalHook("exit", unexpected_exit),
            ExternalHook("memset", memset),
        ]
        program = (
            CandidateProgram.link(
                symbols,
                [
                    CandidateFunction(
                        "item_load_database", BUILD / "objdiff/game/base/80020b4c_item.o"
                    )
                ],
                hooks=hooks,
            )
            if candidate
            else RetailProgram.link(symbols, ["item_load_database"], hooks=hooks)
        )
        result = ParserMachine(retail, program).call(
            "item_load_database",
            memory=[
                MemoryInput(INPUT_VA, source),
                *[MemoryInput(c.address, data) for c, data in zip(captures, initial)],
            ],
            capture=captures,
            allowed_writes=captures,
        )
        if len(paths) != 80 * 40:
            raise AssertionError("STAT did not query all 80 item files")
        outputs.append([*[s.data for s in result.memory], bytes(paths)])
    rust_output = rust.call("item-database", source, *initial, bytes(responses))
    if len(rust_output) != 8:
        raise AssertionError("Rust STAT output block count differs")
    for index, name in enumerate([*[name for name, _size in specs], "CD search paths"]):
        require_equal(f"STAT/{name}", outputs[0][index], outputs[1][index], rust_output[index])
    aligned_size = struct.unpack_from("<I", outputs[0][6], 4)[0]
    if aligned_size != 4096:
        raise AssertionError(f"STAT aligned-size control changed to {aligned_size}")
    print(
        "[resource-oracle] PASS STAT: six banks, 80 directory entries, ordered 40-byte paths, "
        "and preserved 2048-aligned success size",
        flush=True,
    )
    return 1


class PlacementServices:
    """Deterministic external context, not substituted placement parser code."""

    def __init__(self, symbols: GameSymbols, object_base: int):
        self.symbols = symbols
        self.object_base = object_base
        self.trace = bytearray()
        self.random = 0x12345678
        self.effects = 0

    def rand(self, _context: HookContext) -> int:
        self.random = (self.random * 1103515245 + 12345) & 0xFFFFFFFF
        value = (self.random >> 16) & 0x7FFF
        self.trace.extend(struct.pack("<Bi", 1, value))
        return value

    def floor_height(self, context: HookContext) -> int:
        x, y, z = struct.unpack("<3i", context.read(context.args[0], 12))

        def quotient(value: int) -> int:
            return (abs(value) // 2000) * (-1 if value < 0 else 1)

        cell = (quotient(z) * 100 + quotient(x)) & 0xFFFF
        height = -context.read(self.symbols.datum(GRID_NAMES[1])[0] + cell, 1)[0] * 100
        collision = context.read(self.symbols.datum(GRID_NAMES[4])[0] + cell, 1)[0]
        orientation = context.read(self.symbols.datum(GRID_NAMES[2])[0] + cell, 1)[0]
        rem_x, rem_z = x - quotient(x) * 2000, z - quotient(z) * 2000
        raised = {1: rem_x > 1000, 2: rem_z < 1000, 3: rem_x < 1000, 4: rem_z > 1000}
        if collision == 6 and raised.get(orientation, False):
            height += 300
        self.trace.extend(struct.pack("<B4i", 2, x, y, z, height))
        return height

    def occupancy(self, context: HookContext) -> int:
        x, z, delta = context.args[:3]
        delta = struct.unpack("<i", struct.pack("<I", delta))[0]
        self.trace.extend(struct.pack("<BHHi", 3, x, z, delta))
        base = self.symbols.datum(GRID_NAMES[3])[0]
        for row in range(max(z - 2, 0), min(z + 2, 99) + 1):
            for column in range(max(x - 2, 0), min(x + 2, 99) + 1):
                address = base + row * 100 + column
                old = context.read(address, 1)[0]
                context.write(address, bytes([(old & 0xE0) | ((old + delta) & 31)]))
        return 0

    def effect(self, context: HookContext) -> int:
        argument, kind, effect_id, position_ptr = context.args
        object_id = context.read(position_ptr - 8, 1)[0]
        has_rotation = object_id in (135, 138, 139)
        rotation = context.read(context.stack_args[1], 6) if has_rotation else bytes(6)
        index = (self.effects * 7 + 3) % 48
        self.effects += 1
        self.trace.extend(struct.pack("<5B", 4, argument, kind, effect_id, has_rotation))
        self.trace.extend(context.read(position_ptr, 12) + rotation + struct.pack("<i", index))
        # The caller only consumes the returned index. Scratch effect_output is
        # an opaque output argument and is intentionally not used by the loader.
        return self.symbols.datum("effect_pool_records")[0] + index * 60

    def mark(self, context: HookContext) -> int:
        address, value, yaw = context.args[:3]
        index = (address - self.object_base) // 44
        self.trace.extend(struct.pack("<BHBH", 5, index, value, yaw))
        object_id = context.read(address, 1)[0]
        x, z = struct.unpack("<2H", context.read(address + 2, 4))
        x, z = x & 255, z & 255
        behavior = context.read(self.symbols.datum("map_object_state")[0] + object_id * 8, 1)[0]
        yaw &= 0xFFF
        points = []
        if behavior in (2, 3):
            points = [
                (x, z),
                {
                    0: (x, (z + 1) & 255),
                    0x400: ((x + 1) & 255, z),
                    0x800: (x, (z - 1) & 255),
                    0xC00: ((x - 1) & 255, z),
                }.get(yaw, (x, z)),
            ]
        elif behavior == 0:
            points = {
                0: [(x + 1, z - 1), (x + 1, z)],
                0x400: [(x, z + 1), (x + 1, z + 1)],
                0x800: [(x - 1, z + 1), (x - 1, z)],
                0xC00: [(x, z - 1), (x - 1, z - 1)],
            }.get(yaw, [])
        for x, z in points:
            context.write(self.symbols.datum(GRID_NAMES[4])[0] + z * 100 + x, bytes([value]))
        return 0


def synthetic_actor_placements(count: int = 128) -> bytes:
    records = bytearray(count * 16)
    for index in range(count):
        at = index * 16
        records[at] = 1 + index % 4
        records[at + 1] = index & 0x3F
        records[at + 2] = index & 3
        records[at + 3] = 10 + index % 7
        records[at + 4] = 20 + index % 7
        records[at + 5] = index & 0xFF
        records[at + 6] = (index * 3) & 0xFF
        struct.pack_into("<hh", records, at + 10, index - 64, 63 - index)
    return bytes(records)


def synthetic_object_placements(object_id: int, count: int) -> bytes:
    records = bytearray(count * 20)
    for index in range(count):
        at = index * 20
        records[at] = object_id
        records[at + 1] = index & 0xFF
        records[at + 2] = 10 + index % 7
        records[at + 3] = 20 + index % 7
        struct.pack_into(
            "<Hhhh", records, at + 4, (index & 3) << 10, index - 95, 94 - index, index * 13 - 1000
        )
        struct.pack_into("<BBHh", records, at + 12, index & 0xFF, 0x5A, 0x1200 + index, index - 40)
        records[at + 18 : at + 20] = bytes((index & 0xFF, (~index) & 0xFF))
    return bytes(records)


def synthetic_event_definitions(count: int = 8) -> bytes:
    records = bytearray(count * 24)
    for index in range(count):
        at = index * 24
        records[at : at + 5] = bytes((1 + index, 2 + index, 3 + index, 10 + index, 20 + index))
        records[at + 5 : at + 10] = bytes((index + value for value in range(5)))
        records[at + 10 : at + 14] = bytes((30 + index, 40 + index, 50 + index, 60 + index))
        struct.pack_into(
            "<hhHHH",
            records,
            at + 14,
            index - 4,
            4 - index,
            index * 0x400,
            100 + index,
            200 + index,
        )
    return bytes(records)


def placement_synthetic_controls(
    specs: Sequence[tuple[int, str, str, str, int, int]],
    definitions: bytes,
) -> list[tuple[str, tuple[int, str, str, str, int, int], bytes, bytes, bytes, int]]:
    by_family = {spec[1]: spec for spec in specs}
    controlled_definitions = bytearray(definitions)
    for object_id in (1, 123):
        at = object_id * 8
        controlled_definitions[at] = 1
        controlled_definitions[at + 2 : at + 4] = b"\0\0"
    blank_grids = bytes(50_000)
    return [
        (
            "synthetic/object-id-123",
            by_family["objects"],
            synthetic_object_placements(123, 1) + b"\xff",
            blank_grids,
            bytes(controlled_definitions),
            1,
        ),
        (
            "synthetic/full-objects-no-sentinel",
            by_family["objects"],
            synthetic_object_placements(1, 190),
            blank_grids,
            bytes(controlled_definitions),
            190,
        ),
        (
            "synthetic/full-actors-no-sentinel",
            by_family["actors"],
            synthetic_actor_placements(),
            blank_grids,
            definitions,
            128,
        ),
        (
            "synthetic/full-events-no-sentinel",
            by_family["events"],
            synthetic_event_definitions(),
            blank_grids,
            definitions,
            8,
        ),
    ]


def compare_placements(
    retail: RetailImage, symbols: GameSymbols, rust: RustCodec, root: Path
) -> int:
    definitions = next(
        case.source for case in record_cases(root, symbols) if case.operation == "records-objects"
    )
    object_base = symbols.datum("map_object_state")[0] + 1280
    specs = (
        (
            3,
            "items",
            "item_load_floor_placements",
            "80020b4c_item.o",
            symbols.datum("floor_items")[0],
            64 * 24,
        ),
        (4, "objects", "map_object_pool_load", "80030a98_map_object_pool.o", object_base, 190 * 44),
        (
            5,
            "actors",
            "actor_pool_load_placements",
            "80030818_actor_pool.o",
            symbols.datum("actor_state")[0] + 1824,
            128 * 72,
        ),
        (
            7,
            "events",
            "map_event_pool_load",
            "8003379c_map_event.o",
            symbols.datum("map_event_pool")[0],
            8 * 68,
        ),
    )
    cases = []
    for floor in range(1, 6):
        label = f"B{floor}/MIXA.DAT"
        chunks = length_prefixed_chunks((root / "KF" / label).read_bytes(), label)
        for spec in specs:
            cases.append(
                (f"B{floor}/{spec[1]}", spec, chunks[spec[0]], chunks[2], definitions, None)
            )
    for spec in specs:
        cases.append(
            (f"synthetic/empty-{spec[1]}", spec, b"\xff\xff", bytes(50_000), definitions, 0)
        )
    cases.extend(placement_synthetic_controls(specs, definitions))
    for label, (
        _chunk,
        family,
        function,
        obj,
        address,
        size,
    ), source, grids, case_definitions, expected_active in cases:
        initial = pattern(size + 32, 3)
        output = MemoryRange("pool", address, len(initial))
        grid_ranges = [MemoryRange(name, symbols.datum(name)[0], 10_000) for name in GRID_NAMES]
        captures = [output, *grid_ranges]
        count = MemoryRange("floor_item_count", symbols.datum("floor_item_count")[0], 2)
        if family == "items":
            captures.append(count)
        memory = [
            MemoryInput(INPUT_VA, source),
            MemoryInput(address, initial),
            MemoryInput(symbols.datum("map_object_state")[0], case_definitions),
            *[
                MemoryInput(item.address, grids[i * 10_000 : (i + 1) * 10_000])
                for i, item in enumerate(grid_ranges)
            ],
        ]
        functions = [CandidateFunction(function, BUILD / "objdiff/game/base" / obj)]
        if family == "objects":
            functions.append(
                CandidateFunction(
                    "map_object_start_action_if_idle",
                    BUILD / "objdiff/game/base/800315c4_map_object.o",
                )
            )
        outputs = []
        for candidate in (False, True):
            context = PlacementServices(symbols, object_base)
            hooks = [
                ExternalHook("rand", context.rand),
                ExternalHook("map_floor_height_at_position", context.floor_height),
                ExternalHook("collision_adjust_cell_occupancy", context.occupancy),
                ExternalHook("effect_pool_construct", context.effect, stack_words=2),
                ExternalHook("map_object_mark_collision_edge", context.mark),
            ]
            program = (
                CandidateProgram.link(symbols, functions, hooks=hooks)
                if candidate
                else RetailProgram.link(symbols, [item.name for item in functions], hooks=hooks)
            )
            result = ParserMachine(retail, program).call(
                function,
                (INPUT_VA,),
                memory=memory,
                capture=captures,
                allowed_writes=captures,
            )
            outputs.append((result, bytes(context.trace)))
        rust_pool, rust_grids, rust_trace, rust_count = rust.call(
            f"placements-{family}",
            source,
            initial,
            grids,
            case_definitions,
        )
        require_equal(
            label + "/pool", outputs[0][0].memory[0].data, outputs[1][0].memory[0].data, rust_pool
        )
        require_equal(
            label + "/grids",
            b"".join(s.data for s in outputs[0][0].memory[1:6]),
            b"".join(s.data for s in outputs[1][0].memory[1:6]),
            rust_grids,
        )
        require_equal(label + "/effects", outputs[0][1], outputs[1][1], rust_trace)
        if family == "items":
            require_equal(
                label + "/count",
                outputs[0][0].memory[-1].data,
                outputs[1][0].memory[-1].data,
                rust_count,
            )
        if expected_active is not None and rust_count != struct.pack("<H", expected_active):
            raise AssertionError(
                f"{label}: Rust reports {rust_count.hex()}, expected {expected_active}"
            )
        if label == "synthetic/object-id-123":
            if any(output[0].memory[0].data[40] != 0x0B for output in outputs):
                raise AssertionError("object id 123 did not start action 0x0b")
        if "full-" in label:
            stride = {"objects": 44, "actors": 72, "events": 68}[family]
            for result, _trace in outputs:
                if any(
                    result.memory[0].data[index * stride] == 0xFF
                    for index in range(expected_active)
                ):
                    raise AssertionError(f"{label}: a full-capacity slot was marked free")
        print(
            f"[resource-oracle] PASS {label}: complete pool, grids, ordered service effects",
            flush=True,
        )
    return len(cases)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--family",
        choices=("records", "tim", "placements", "stat", "archives", "common", "all"),
        default="all",
    )
    parser.add_argument("--retail-dir", type=Path)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument(
        "--no-rebuild", action="store_true", help="reuse existing C objects explicitly"
    )
    args = parser.parse_args(argv)
    root = args.retail_dir or configured_retail_dir()
    retail = RetailImage.load("GAME.EXE")
    symbols = GameSymbols.load()
    if not args.no_rebuild:
        units = []
        if args.family in ("records", "all"):
            units += [
                "game.resources",
                "game.equipment",
                "game.magic",
                "game.map_object_pool",
                "game.actor_pool",
            ]
        if args.family in ("tim", "all"):
            units += ["game.resources"]
        if args.family in ("placements", "all"):
            units += [
                "game.item",
                "game.actor_pool",
                "game.map_object_pool",
                "game.map_object",
                "game.map_event",
            ]
        if args.family in ("stat", "all"):
            units += ["game.item"]
        if args.family in ("archives", "all"):
            units += ["game.asset_registry", "game.render"]
        if args.family in ("common", "all"):
            units += [
                "game.resources",
                "game.equipment",
                "game.magic",
                "game.map_object_pool",
                "game.asset_registry",
                "game.render",
            ]
        rebuild_units(units)
    rust = RustCodec(args.rust_driver or build_driver())
    count = 0
    if args.family in ("records", "all"):
        count += compare_records(retail, symbols, rust, root)
    if args.family in ("tim", "all"):
        count += compare_tim(retail, symbols, rust, root)
    if args.family in ("placements", "all"):
        count += compare_placements(retail, symbols, rust, root)
    if args.family in ("stat", "all"):
        count += compare_stat(retail, symbols, rust, root)
    if args.family in ("archives", "all"):
        count += compare_archives(retail, symbols, rust, root)
    if args.family in ("common", "all"):
        count += compare_common(retail, symbols, rust, root)
    print(f"[resource-oracle] PASS: {count} cases across requested families", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
