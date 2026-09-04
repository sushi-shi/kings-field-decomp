"""Three-way GAME serialization of live pools into nested floor records."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from scripts.kf.codec_candidate import rebuild_units
from scripts.kf.parser_machine import (
    CandidateFunction,
    CandidateProgram,
    GameSymbols,
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
ACTOR_DEFINITION_BYTES = 1_824
ACTOR_SIZE = 72
ACTOR_COUNT = 128
OBJECT_DEFINITION_BYTES = 1_280
OBJECT_SIZE = 44
OBJECT_COUNT = 190
WORLD_SIZE = 8_500
PLAYER_SIZE = 0xE0
FLOOR_RECORD_STRIDE = 1_700
PERSIST_OBJECT = BUILD / "objdiff/game/base/800356e8_map_events.o"


@dataclass(frozen=True)
class PersistCase:
    label: str
    floor: int
    events: bytes
    actors: bytes
    definitions: bytes
    objects: bytes


def _base_runtime() -> tuple[bytearray, bytearray, bytearray, bytearray]:
    events = bytearray(pattern(EVENT_COUNT * EVENT_SIZE, 121))
    for event, image_index in enumerate((0, 1, 2, 3, 4, 5, 1, 5)):
        events[event * EVENT_SIZE + 9] = image_index

    actors = bytearray(pattern(ACTOR_COUNT * ACTOR_SIZE, 122))
    for actor in range(ACTOR_COUNT):
        actors[actor * ACTOR_SIZE] = 0xFF

    definitions = bytearray(pattern(OBJECT_DEFINITION_BYTES, 123))
    objects = bytearray(pattern(OBJECT_COUNT * OBJECT_SIZE, 124))
    for object_index in range(OBJECT_COUNT):
        objects[object_index * OBJECT_SIZE] = 0xFF
    return events, actors, definitions, objects


def _runtime(kind: str) -> tuple[bytes, bytes, bytes, bytes]:
    events, actors, definitions, objects = _base_runtime()
    if kind == "mixed":
        for actor, slot, lifecycle in ((0, 1, 3), (7, 3, 2), (127, 1, 0)):
            at = actor * ACTOR_SIZE
            actors[at] = slot
            actors[at + 6] = lifecycle
        for object_index, (object_id, behavior, action) in enumerate((
            (1, 0xFF, 0xFF),
            (2, 0x0D, 0xFF),
            (3, 0x40, 0xFF),
            (4, 0x0E, 0xFF),
            (5, 0x41, 0xFF),
            (6, 0xFF, 9),
            (7, 2, 0xFF),
        )):
            definitions[object_id * 8] = behavior
            at = object_index * OBJECT_SIZE
            objects[at] = object_id
            objects[at + 40] = action
    elif kind == "floor5-full":
        definitions[:] = bytes(len(definitions))
        for object_index in range(149):
            objects[object_index * OBJECT_SIZE] = 0
    elif kind != "empty":
        raise ValueError(f"unknown persistence runtime {kind!r}")
    return bytes(events), bytes(actors), bytes(definitions), bytes(objects)


def cases() -> tuple[PersistCase, ...]:
    mixed = _runtime("mixed")
    empty = _runtime("empty")
    floor5 = _runtime("floor5-full")
    return (
        PersistCase("floor1-mixed", 1, *mixed),
        PersistCase("floor2-empty", 2, *empty),
        PersistCase("floor3-mixed", 3, *mixed),
        PersistCase("floor4-empty", 4, *empty),
        PersistCase("floor5-exact-capacity", 5, *floor5),
    )


def _program(symbols: GameSymbols, candidate: bool):
    if candidate:
        return CandidateProgram.link(
            symbols,
            [CandidateFunction("map_world_state_persist", PERSIST_OBJECT)],
        )
    return RetailProgram.link(symbols, ["map_world_state_persist"])


def compare_world_persist(
    retail: RetailImage,
    symbols: GameSymbols,
    rust: RustCodec,
) -> int:
    event_address = symbols.datum("map_event_pool")[0]
    actor_address = symbols.datum("actor_state")[0] + ACTOR_DEFINITION_BYTES
    object_state_address = symbols.datum("map_object_state")[0]
    world_address = symbols.datum("map_world_state_base")[0]
    player_address = symbols.datum("player_state")[0]
    world_range = MemoryRange("world", world_address, WORLD_SIZE)

    for case in cases():
        initial_world = pattern(WORLD_SIZE, 125 + case.floor)
        record_offset = -1_690 + FLOOR_RECORD_STRIDE * case.floor
        record_size = min(FLOOR_RECORD_STRIDE, WORLD_SIZE - record_offset)
        record_range = MemoryRange(
            "selected floor record",
            world_address + record_offset,
            record_size,
        )
        player = bytearray(pattern(PLAYER_SIZE, 131))
        player[10] = case.floor
        memory = (
            MemoryInput(event_address, case.events),
            MemoryInput(actor_address, case.actors),
            MemoryInput(object_state_address, case.definitions + case.objects),
            MemoryInput(world_address, initial_world),
            MemoryInput(player_address, bytes(player)),
        )
        outputs = []
        for candidate in (False, True):
            result = ParserMachine(retail, _program(symbols, candidate)).call(
                "map_world_state_persist",
                memory=memory,
                capture=(world_range,),
                allowed_writes=(record_range,),
            )
            outputs.append(result.memory[0].data)

        (rust_record,) = rust.call(
            "world-persist",
            initial_world[record_offset:record_offset + record_size],
            case.events,
            case.actors,
            case.definitions,
            case.objects,
        )
        rust_world = bytearray(initial_world)
        rust_world[record_offset:record_offset + record_size] = rust_record
        require_equal(case.label, outputs[0], outputs[1], bytes(rust_world))
        print(
            f"[world-persist-oracle] PASS {case.label}: "
            f"full {WORLD_SIZE}-byte world state",
            flush=True,
        )
    return len(cases())


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rust-driver", type=Path)
    parser.add_argument("--no-rebuild", action="store_true")
    args = parser.parse_args(argv)
    if not args.no_rebuild:
        rebuild_units(["game.map_events"])
    count = compare_world_persist(
        RetailImage.load("GAME.EXE"),
        GameSymbols.load(),
        RustCodec(args.rust_driver or build_driver()),
    )
    print(f"[world-persist-oracle] PASS: {count} deterministic cases", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
