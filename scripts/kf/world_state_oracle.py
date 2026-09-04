"""Three-way GAME floor-state restoration on deterministic synthetic records."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from scripts.kf.codec_candidate import rebuild_units
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
)
from scripts.kf.paths import BUILD
from scripts.kf.resource_oracle import pattern, require_equal
from scripts.kf.rust_codec import RustCodec, build_driver
from scripts.kf.sema.image import RetailImage


EVENT_SIZE = 68
EVENT_COUNT = 8
ACTOR_HEAD_SIZE = 0x442
ACTOR_SIZE = 72
ACTOR_COUNT = 128
OBJECT_SIZE = 44
OBJECT_COUNT = 190
WORLD_SIZE = 8500
HEIGHT_SIZE = 10_000
PLAYER_SIZE = 0xE0
RANDOM_SEED = 0x12345678
RESTORE_OBJECT = BUILD / "objdiff/game/base/80035e44_map_load.o"


@dataclass(frozen=True)
class Conditions:
    current_floor: int
    highest_floor: int = 0
    world_byte_1: int = 0
    world_byte_2: int = 0
    flag_8009eafc: int = 0
    flag_8009f844: int = 0
    flag_8009f845: int = 0
    flag_8009f846: int = 0
    inventory_0a: int = 0
    inventory_0b: int = 0
    boss_defeat_complete: int = 0
    found_actor: int = -1

    def codec_bytes(self) -> bytes:
        return bytes((
            self.current_floor, self.highest_floor, self.world_byte_1,
            self.world_byte_2, self.flag_8009eafc, self.flag_8009f844,
            self.flag_8009f845, self.flag_8009f846, self.inventory_0a,
            self.inventory_0b, self.boss_defeat_complete,
        ))


def synthetic_record(marker: bool = True, *, sparse: bool = True) -> bytes:
    if not marker:
        return b"\0"
    output = bytearray((1,))
    for event in range(EVENT_COUNT):
        output.extend((event + 1, 9, 1, 2, 0x30 + event, 3, 4))
    output.extend((2, 3, 7, 127, 8) if sparse else (0,))
    object_ids_offset = len(output)
    output.extend(index % 150 for index in range(OBJECT_COUNT))
    output[object_ids_offset + 170:object_ids_offset + 173] = bytes((42, 45, 50))
    output.extend((1, 3, 10, 11, 12, 13, 14, 15, 16, 17) if sparse else (0,))
    for index in range(10):
        output.extend((index + 1, index + 2, 0x80 + index, 0x90 + index))
    for index in range(20):
        output.extend((index + 21, index + 31, index))
    assert len(output) == (362 if sparse else 349)
    return bytes(output)


class RestoreServices:
    def __init__(self, found_actor: int) -> None:
        self.found_actor = found_actor
        self.random = RANDOM_SEED
        self.trace = bytearray()

    def rand(self, _context: HookContext) -> int:
        self.random = (self.random * 1_103_515_245 + 12_345) & 0xFFFFFFFF
        value = (self.random >> 16) & 0x7FFF
        self.trace.append(1)
        self.trace.extend(struct.pack("<i", value))
        return value

    def copy_region(self, context: HookContext) -> int:
        self.trace.extend((2, context.args[0] & 255))
        return 0

    def find_actor(self, context: HookContext) -> int:
        self.trace.extend((3, context.args[0] & 255, context.args[1] & 255))
        self.trace.extend(struct.pack("<i", self.found_actor))
        return self.found_actor

    def clear_link(self, context: HookContext) -> int:
        self.trace.extend((4, context.args[0] & 255))
        return 0

    def trigger_link(self, context: HookContext) -> int:
        self.trace.extend((5, context.args[0] & 255))
        return 0

    def actor_death(self, context: HookContext) -> int:
        self.trace.append(6)
        self.trace.extend(struct.pack("<H", context.args[0] & 0xFFFF))
        return 0

    def hooks(self) -> list[ExternalHook]:
        return [
            ExternalHook("rand", self.rand),
            ExternalHook("map_apply_copy_region", self.copy_region),
            ExternalHook("actor_pool_find_at_tile", self.find_actor),
            ExternalHook("map_object_pool_clear_link", self.clear_link),
            ExternalHook("map_object_pool_trigger_link", self.trigger_link),
            ExternalHook("actor_pool_begin_death_by_definition", self.actor_death),
        ]


def _program(symbols: GameSymbols, candidate: bool, hooks: Sequence[ExternalHook]):
    if candidate:
        return CandidateProgram.link(
            symbols,
            [CandidateFunction("map_restore_floor_state", RESTORE_OBJECT)],
            hooks=hooks,
        )
    return RetailProgram.link(symbols, ["map_restore_floor_state"], hooks=hooks)


def _cases() -> list[tuple[str, bytes, Conditions]]:
    persisted = synthetic_record()
    return [
        ("floor1", persisted, Conditions(1, world_byte_2=1, flag_8009f845=1,
                                          found_actor=5)),
        ("floor2", persisted, Conditions(2, highest_floor=3, flag_8009f845=1)),
        ("floor3", persisted, Conditions(3, flag_8009eafc=4, flag_8009f845=1)),
        ("floor4", persisted, Conditions(4)),
        ("floor5-direct", persisted,
         Conditions(5, flag_8009f844=1, inventory_0a=1, boss_defeat_complete=1)),
        ("floor5-copy", persisted, Conditions(5, flag_8009f846=1)),
        ("marker-clear", synthetic_record(False), Conditions(4)),
        ("zero-sparse-counts", synthetic_record(sparse=False), Conditions(4)),
        ("floor1-missing-actor", persisted, Conditions(1)),
        ("floor1-skip-actor", persisted, Conditions(1, world_byte_1=2)),
        ("floor2-no-overrides", persisted, Conditions(2)),
        ("floor3-no-overrides", persisted, Conditions(3)),
        ("floor5-no-overrides", persisted, Conditions(5)),
        ("floor5-second-inventory", persisted, Conditions(5, inventory_0b=1)),
        ("floor5-link-flag", persisted, Conditions(5, flag_8009f845=1)),
    ]


def compare_world_state(retail: RetailImage, symbols: GameSymbols, rust: RustCodec) -> int:
    event_address = symbols.datum("map_event_pool")[0]
    actor_state_address = symbols.datum("actor_state")[0]
    actor_address = actor_state_address + 1824
    object_state_address = symbols.datum("map_object_state")[0]
    object_address = object_state_address + 1280
    world_address = symbols.datum("map_world_state_base")[0]
    player_address = symbols.datum("player_state")[0]
    height_address = symbols.datum("map_floor_height_grid")[0]
    ranges = [
        MemoryRange("events", event_address, EVENT_COUNT * EVENT_SIZE),
        MemoryRange("actor-head", actor_state_address, ACTOR_HEAD_SIZE),
        MemoryRange("actors", actor_address, ACTOR_COUNT * ACTOR_SIZE),
        MemoryRange("objects", object_address, OBJECT_COUNT * OBJECT_SIZE),
    ]
    heights = pattern(HEIGHT_SIZE, 91)
    for label, record, conditions in _cases():
        initial = [pattern(item.size, 100 + index) for index, item in enumerate(ranges)]
        world = bytearray(pattern(WORLD_SIZE, 110))
        world[1] = conditions.world_byte_1
        world[2] = conditions.world_byte_2
        record_at = -1690 + 1700 * conditions.current_floor
        world[record_at:record_at + len(record)] = record
        player = bytearray(pattern(PLAYER_SIZE, 111))
        player[10] = conditions.current_floor
        player[11] = conditions.highest_floor
        machine_memory = [
            MemoryInput(world_address, bytes(world)),
            MemoryInput(player_address, bytes(player)),
            MemoryInput(height_address, heights),
            MemoryInput(object_state_address, pattern(1280, 112)),
            MemoryInput(symbols.datum("DAT_8009eafc")[0], bytes((conditions.flag_8009eafc,))),
            MemoryInput(symbols.datum("DAT_8009f844")[0], bytes((conditions.flag_8009f844,))),
            MemoryInput(symbols.datum("DAT_8009f845")[0], bytes((conditions.flag_8009f845,))),
            MemoryInput(symbols.datum("DAT_8009f846")[0], bytes((conditions.flag_8009f846,))),
            MemoryInput(symbols.datum("boss_defeat_complete")[0],
                        bytes((conditions.boss_defeat_complete,))),
            MemoryInput(symbols.datum("DAT_800652a8")[0],
                        bytes(10) + bytes((conditions.inventory_0a, conditions.inventory_0b))),
        ]
        machine_memory.extend(
            MemoryInput(item.address, value) for item, value in zip(ranges, initial)
        )
        outputs = []
        for candidate in (False, True):
            services = RestoreServices(conditions.found_actor)
            result = ParserMachine(retail, _program(symbols, candidate, services.hooks())).call(
                "map_restore_floor_state", memory=machine_memory,
                capture=ranges, allowed_writes=ranges,
            )
            outputs.append(([item.data for item in result.memory], bytes(services.trace)))
        rust_blocks = rust.call(
            "world-restore", record, conditions.codec_bytes(), *initial, heights,
            struct.pack("<Ii", RANDOM_SEED, conditions.found_actor),
        )
        for index, name in enumerate(("events", "actor-head", "actors", "objects")):
            require_equal(f"world/{label}/{name}", outputs[0][0][index],
                          outputs[1][0][index], rust_blocks[index])
        require_equal(f"world/{label}/helpers", outputs[0][1], outputs[1][1], rust_blocks[4])
        print(f"[world-state-oracle] PASS {label}: full pools and helper trace", flush=True)
    return len(_cases())


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    args = parser.parse_args(argv)
    if not args.no_rebuild:
        rebuild_units(["game.map_load"])
    count = compare_world_state(
        RetailImage.load("GAME.EXE"), GameSymbols.load(),
        RustCodec(args.rust_driver or build_driver()),
    )
    print(f"[world-state-oracle] PASS: {count} deterministic cases", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
