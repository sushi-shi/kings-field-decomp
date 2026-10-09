# GAME declaration header dependencies

Reviewed input: `91a8ecee`. This completes the 39 remaining consumers of
`kf/game/game.h` identified by the external cleanup plan, after the shared
memory implementation was corrected separately.

## Review and source verdict

Each source was compiled in isolation with the umbrella include removed.
Four already had sufficient declarations; the other 35 exposed specific
missing declarations. Those diagnostics were traced to the existing curated
owner headers, including the SDK adapters. The database loader is declared
in `game/menu.h`, despite its `item_` name; header ownership was checked rather
than inferred from the prefix. No local replacement declarations were added.

The table records each individual include decision. Existing primary and SDK
includes remain. Only the umbrella include is replaced; all function bodies,
ADDRESS/DATA/RODATA claims, signatures, widths, constants, and referents agree
with the reviewed input. Compilation alone does not establish semantic names
or new object ownership; this change reuses the established declarations.

| Source | Added declaration owners |
| --- | --- |
| [main.c](../../src/game/main.c) | `game/system.h`, `game/state.h`, `psyq/cd.h`, `psyq/pad.h` |
| [display_play_transition.c](../../src/game/display_play_transition.c) | `lib/cd_file.h`, `lib/resources.h` |
| [game.c](../../src/game/game.c) | `lib/memory.h`, `game/resources.h`, `game/system.h`, `game/menu.h`, `psyq/pad.h` |
| [player_death.c](../../src/game/player_death.c) | None; existing declarations suffice |
| [player_core.c](../../src/game/player_core.c) | `lib/cd_file.h`, `lib/memory.h` |
| [player_use_item.c](../../src/game/player_use_item.c) | `game/actor.h`, `game/render.h`, `game/notify.h`, `game/state.h` |
| [player_death_fade.c](../../src/game/player_death_fade.c) | None; existing declarations suffice |
| [player_update.c](../../src/game/player_update.c) | `game/actor.h`, `game/render.h`, `game/menu.h`, `game/animation_cache.h`, `game/state.h`, `psyq/pad.h` |
| [collision_grid.c](../../src/game/collision_grid.c) | None; existing declarations suffice |
| [collision.c](../../src/game/collision.c) | `game/player.h`, `game/actor.h`, `lib/map.h` |
| [cd_file.c](../../src/game/cd_file.c) | `game/render.h`, `lib/memory.h` |
| [resources.c](../../src/game/resources.c) | `lib/cd_file.h`, `lib/memory.h`, `game/asset.h` |
| [render.c](../../src/game/render.c) | `lib/resources.h`, `lib/memory.h`, `psyq/pad.h` |
| [render_map_cells.c](../../src/game/render_map_cells.c) | `game/player.h` |
| [item.c](../../src/game/item.c) | `game/menu.h`, `game/player.h`, `lib/memory.h`, `psyq/pad.h` |
| [menu.c](../../src/game/menu.c) | `game/player.h`, `psyq/pad.h` |
| [menu_map_viewer.c](../../src/game/menu_map_viewer.c) | `game/player.h`, `lib/resources.h`, `psyq/pad.h` |
| [menu_panels.c](../../src/game/menu_panels.c) | `game/player.h`, `game/effect.h`, `psyq/pad.h` |
| [menu_select.c](../../src/game/menu_select.c) | `game/player.h`, `game/effect.h`, `psyq/pad.h` |
| [menu_runtime.c](../../src/game/menu_runtime.c) | `game/player.h`, `game/cd.h`, `lib/resources.h`, `psyq/pad.h` |
| [save_system.c](../../src/game/save_system.c) | `game/menu.h`, `lib/memory.h`, `lib/resources.h`, `psyq/pad.h` |
| [actor.c](../../src/game/actor.c) | `game/player.h` |
| [actor_behavior.c](../../src/game/actor_behavior.c) | `game/player.h` |
| [actor_pool.c](../../src/game/actor_pool.c) | `game/collision.h` |
| [map_object_pool.c](../../src/game/map_object_pool.c) | `game/effect.h` |
| [map_object.c](../../src/game/map_object.c) | `game/state.h`, `lib/audio.h`, `game/collision.h`, `game/effect.h`, `game/player.h` |
| [audio.c](../../src/game/audio.c) | `lib/memory.h`, `game/player.h`, `lib/cd_file.h` |
| [map_event.c](../../src/game/map_event.c) | `game/player.h`, `game/render.h`, `game/system.h` |
| [lighting.c](../../src/game/lighting.c) | `game/player.h`, `game/system.h` |
| [map_scripts.c](../../src/game/map_scripts.c) | `game/player.h`, `game/state.h`, `game/system.h`, `game/menu.h` |
| [map_events.c](../../src/game/map_events.c) | `game/player.h`, `game/state.h`, `game/animation_cache.h` |
| [map_load.c](../../src/game/map_load.c) | `game/player.h` |
| [player_warp.c](../../src/game/player_warp.c) | `game/render.h`, `game/state.h`, `game/system.h`, `game/animation_cache.h`, `game/actor.h` |
| [menu_enter_mode.c](../../src/game/menu_enter_mode.c) | `game/animation_cache.h`, `lib/memory.h`, `game/player.h` |
| [effect_pool.c](../../src/game/effect_pool.c) | `game/player.h` |
| [effect_map_collision.c](../../src/game/effect_map_collision.c) | None; existing declarations suffice |
| [effect_update.c](../../src/game/effect_update.c) | `game/player.h`, `game/actor.h`, `game/state.h` |
| [effect_dispatch.c](../../src/game/effect_dispatch.c) | `game/player.h`, `game/actor.h` |
| [magic.c](../../src/game/magic.c) | `game/actor.h` |

Both C89 and C++20 checks pass for all 39 actual source files (78 checks),
with function declarations required. This includes the final top-level GAME
imports. The historical compiler additionally rebuilds every changed unit
with its existing manifest profile; no profile or SDK declaration changes.

`game/game.h` remains a convenience aggregate, with its comment updated to
reflect direct owner imports in implementation files. No implementation
source includes it after this review. The actual consumers are checked in
both language views and rebuilt with their pinned profiles.

## Verification scope

All 97 native-derived ELF comparison objects are byte-identical to the
captured `19df9a3e` baseline, including instructions, data and relocations.
The native debug records are outside that equality claim. All three linked
CPE/EXE pairs are also byte-identical. Refreshed strict function reports retain
the baseline total of 465/471, including the existing non-exact functions;
this is not a claim that every reviewed function was already exact.

The 39 reviewed units contain 284 functions: 280 remain strict 100%. Their
four existing residues retain these results:

| GAME function | Current strict result | Header verdict |
| --- | ---: | --- |
| `main` | 78.52941% | Complete comparison object unchanged |
| `player_move_horizontal` | 96.56896% | Complete comparison object unchanged |
| `map_object_spawn_drop` | 97.128716% | Complete comparison object unchanged |
| `effect_update_dispatch` | 99.82781% | Complete comparison object unchanged |

The executable build succeeds for all three images. Existing data and
section-placement analysis failures remain baseline debt. No new functions
are banked. All 885 local tests pass in 283.988 seconds with no skips;
all 97 type variants, Ruff, and diff whitespace checks pass.
