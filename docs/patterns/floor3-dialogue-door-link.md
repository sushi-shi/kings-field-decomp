# Floor-3 dialogue door unlock

## Function Match Plan

The shipped placement records resolve the link-clear branch left open by the
[character-domain review](map-character-domain.md). Name character 12
`KF_CHARACTER_FLOOR3_DOOR_UNLOCKER` and its link 55
`MAP_FLOOR3_DIALOGUE_DOOR_LINK`. Preserve the two encoded values and the existing
call; no behavior or resource bytes change.

GAME `map_event_interact` (`0x80034a80 / 0x2d4`) compares its character byte
with 12 at `0x80034ac4..c8`, then calls `map_object_pool_clear_link` at
`0x80034ce0`, supplying 55 in the delay slot. Its complete cached dossier,
current source, callers and adjacent routines were reviewed in the preceding
character campaign. This is a game-specific dialogue action, not SDK code.

## Shipped placement evidence

The configured retail directory was validated before reading resources.
Decode all eight length-prefixed `KF/B1..B5/MIXA.DAT` chunks. Chunk 7 holds
24-byte event definitions and chunk 4 holds 20-byte object placements; stop each
list at its free/end marker. Common-resource chunk 5 supplies eight-byte object
definitions. These widths and chunk owners follow the existing resource loaders.

| Resource / slot | Decoded identity and role |
| --- | --- |
| B3 event 1 | Character 12; active; behavior 2; cell X 98, Z 58. This is the only live authored character-12 record across the five floors. |
| B3 object 19 | Model 120, the hinged-door partner; behavior 1; cell X 66, Z 64; yaw zero; link 55. |
| B3 object 23 | Model 119, the hinged door; behavior 0; cell X 64, Z 64; yaw zero; link 55. |
| B1 object 60 | Model 104; behavior 9; cell X 33, Z 86; link 55. This occurrence is outside the link-clear routine's eligible behavior range. |

No other live object placements carry link 55. B3 `MIXA.DAT` SHA-256 is
`3172588238d121fec8972b8aca03922f9bb1e652a956780a0bec033b2f97f7f2`;
B1 is `4b61e2e98348d43b21e9dc1ca793d2447fbfc5bbd0db59637572d456f70f035f`.
The ignored placement report also records the hashes of the other three floors.

## Consumer confirmation

Fresh image-qualified retail disassembly of GAME `map_object_pool_clear_link`
(`0x80031c44 / 0x84`) shows a leaf routine with no calls and one validated data
pair. It visits 190 records, accepts definition behavior below eight or exactly
eight at `0x80031c80..8c`, compares the link byte at `0x80031c94..9c`, and stores
255 at `0x80031ca4`. Thus it clears both B3 door records but excludes B1's
behavior-nine object. No new evidence tier is assigned to unrelated references.

The GAME interaction dispatcher tests a hinged door's link against 255 at
`0x80035438..50`; a remaining link on behavior zero selects the default
notification instead of the door action. Its paired-door search repeats that
test at `0x800354c4..dc`. Clearing the two links removes this interaction lock;
normal facing and interaction checks still apply. The name describes that
observed unlock role, without inventing the character's proper name or a room
name. Authored floor membership establishes context, not a new runtime guard.

Builds, compiler checks, tests, post-edit matches and banking remain deferred
until the naming pass finishes. Review the two substitutions and reconcile the
removed literal rows before committing; no new exact-match claim is made.

## Source review result

The character selector is now a named member of its existing byte enum, and the
link is a named local constant passed to the existing u8 API. Reversing those
substitutions and definitions recovers the previous header and C source exactly.
The complete source ledger matches all 111 C files and 6,087 retained occurrences.
The two names resolve previously documented semantic gaps; no unknown field is
renamed speculatively. `map_event_interact` remains unverified after this edit.
