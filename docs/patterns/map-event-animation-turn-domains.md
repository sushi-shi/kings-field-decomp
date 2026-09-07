# Map-event animation and collision-turn domains

## Function Match Plan

Give `KfMapEvent.animation_clip` and `collision_turn_pending` distinct byte enum
types. Propagate their values through pool initialization, interaction, wandering
and rendering. The generic animation binder accepts clip indices from several
asset families, so explicitly encode the event-specific clip at that call.
Preserve all existing predicates, field widths, values and call sequences.

All addresses below select GAME.EXE. Complete cached retail/CFG, caller, callee,
string and match dossiers were reviewed alongside current source, history and
the adjacent map-event routines. The [dialogue/animation evidence](game-map-dialogue-state.md)
establishes these as game policies around separately attributed SDK operations.

| Function / extent | Evidence and intended source change |
| --- | --- |
| `map_event_pool_load`, `0x800338b8 / 0x22c` | The 56-byte-frame loader initializes clip +0x0f at `0x80033a8c` and turn flag +0x10 in the `collision_adjust_cell_occupancy` call's delay slot at `0x80033a9c`. Name both zero stores; preserve the one proven call, two data pairs and two internal jumps. |
| `map_interaction_dispatch`, `0x80034de4 / 0x904` | The shop path explicitly selects base clip zero. The animation-loop path checks the signed asset clip count, selects clip one for interaction if available, then restores clip zero. Name all three writes; keep the count threshold and animation-phase arithmetic distinct from clip identity. The prior [map review](map-image-groups.md) records its current dispatcher evidence. |
| `map_event_update_wander`, `0x80035708 / 0x1d8` | The 56-byte-frame update clears +0x10 after successful motion at `0x80035840`. Collision reads the byte at `0x80035874`; zero or completed yaw allows a random target, then stores one at `0x800358ac`. Name the pending-turn predicate and both stores; preserve nine proven calls, the current-event data pair and internal jump. |
| `render_map_event`, `0x8001f0c4 / 0x154` | The 120-byte-frame renderer loads +0x0f with `lbu a2` at `0x8001f1a4`, then calls the generic binder at `0x8001f1b4`. Encode the byte enum as its existing u16 parameter type. Preserve fifteen proven calls, five data pairs and the internal jump. |

## Domain meaning

`KF_MAP_EVENT_CLIP_BASE` is the default animation and the one used by the shop
pose sequence. `KF_MAP_EVENT_CLIP_INTERACTION` is the optional second clip used
by the animation-loop interaction path. These names express consumer roles,
without claiming every base clip is an idle animation or assigning actor action
names to unrelated asset clips. The stored clip byte is initialized at runtime;
it is not one of the unresolved definition bytes copied from the resource.

`KF_MAP_EVENT_COLLISION_TURN_NONE` means the collision response has no pending
turn. `KF_MAP_EVENT_COLLISION_TURN_PENDING` means it selected a heading; the
existing yaw-completion comparison also permits choosing another heading while
the flag remains set. Keep the zero/nonzero distinction: no new validation or
requirement that every nonzero value equal one is introduced.

The unresolved animation-header halfword, floor-item byte, map-event definition
bytes and OPEN reset-only words still lack evidence for semantic labels. This
domain refinement does not resolve them or hide their identifiers.

## Verification boundary

The existing project enum macros retain u8 storage for the retail C view and
distinct scoped types for modern checking. No compiler check, build, test,
post-edit match or bank operation is run during this naming pass, as requested.
Source review must establish the bounded substitutions, metadata types and
complete retained-literal accounting; binary verification remains outstanding.

## Source review result

All nine direct uses of the two fields now follow their domains: four clip
writes, three turn-flag writes, its one predicate and the renderer's numeric
encoding boundary. Reversing the eight value substitutions and the one explicit
argument encoding recovers the four prior C files exactly. The header adds the
two domains and changes only those field types; the two curated structure rows
retain their byte extents and supported status.

The current ledgers match all 111 C files and 6,092 retained occurrences,
including duplicate tokens and their full expressions. Eight values were named;
no unknown field acquired an unsupported identity. Pool loading, wandering,
interaction and rendering all remain unverified after this edit. The naming
goal remains open.
