# Effect-kind domain

## Function Match Plan

Make the existing behavioral identities a `KfEffectKind` byte enum. Propagate
that domain through both views of effect-record byte +1, the general constructor,
the dispatcher local and the player's weapon-effect local. Decode the low five
bits of the actor's packed effect code into a separate typed selector after
checking the paired-spawn bit. Spell the magic-ID conversion explicitly; encode
effect kinds only at magic-row indexing. Keep unresolved numeric variants as
explicit enum decodes rather than inventing visual identities.

All addresses below identify GAME.EXE. Current source and history, cached retail
disassembly/CFG, callers, callees, strings and historical match records are the
evidence. The [constructor review](game-effect-constructor.md),
[pool review](game-effect-pool-constants.md) and
[dispatcher review](game-effect-dispatch-constants.md) establish this game-owned
family and adjacent helpers. Historical match percentages do not validate these
edits. Builds, compiler checks, tests, post-edit matches and banking are deferred
until naming is finished, as requested.

| Function / extent | Evidence snapshot and intended change |
| --- | --- |
| `effect_pool_construct`, `0x80036f44 / 0x82c` | A 56-byte frame; 29 proven incoming calls in the cached dossier. Stores input kind with `sb` at `0x80036f8c`, reloads with `lbu` at `0x80036ff8`, scales it by twenty for magic rows, then dispatches through the owned 45-entry switch. Type the argument/field and retain all constructor variant rewrites and optional slots. |
| `effect_pool_set_current`, `0x8003781c / 0x34` | Leaf, no strings or calls, two validated data pairs. `lbu` at `0x80037828` loads kind before twenty-byte row indexing. Encode this array index; retain the current-record-relative base. |
| `effect_update_dispatch`, `0x80038a38 / 0x180c` | A 168-byte frame and owned 49-entry switch. `lbu` at `0x80038a78` reads byte +1; source captures it separately from phase. Type that local, decode unresolved comparisons/cases and child constructor variants, encode fixed magic-row indices. Preserve every phase, collision and child-spawn path. |
| `actor_spawn_action_effect`, `0x8002edd4 / 0x454` | A 200-byte frame; paired bit tested at `0x8002ee1c`, low-five-bit mask at `0x8002ee2c`. Calls at `0x8002f12c`, `0x8002f184`, `0x8002f1bc`, `0x8002f1e0` consume the masked selector or explicit kinds. Separate the packed code from its decoded kind; retain the word local in legacy C. |
| `actor_update_boss_death_sequence`, `0x8002f8cc / 0x1bc` | The call at `0x8002fa18` receives kind 44 loaded at `0x8002f9fc`. Decode that literal at the typed constructor boundary; do not assign an unproven asset name. |
| `magic_cast`, `0x8003a2a0 / 0x4c0` | Four constructor calls at `0x8003a57c`, `0x8003a5a8`, `0x8003a600`, `0x8003a744` use the selected spell's encoded ID. Decode that byte into the effect domain; magic and effect IDs remain distinct types. |
| `player_update`, `0x80018880 / 0x1a1c` | Weapon selection assigns homing, fire-ball, Moonlight or Light-Needle kinds before constructor calls. Keep the existing signed-word local in legacy C while carrying the effect domain in modern checking. |
| `map_object_pool_update`, `0x80031cc8 / 0xc18` | The emitter action supplies raw kind 14, while neighboring fire-ball/wind-cutter actions already use names. Decode the unresolved selector and preserve the separate object/action domains. |
| `actor_try_select_profiled_action`, `0x8002e0f0 / 0x1f8` | A 40-byte frame, three proven callers, four calls and no strings. `andi` at `0x8002e108` extracts the same low-five-bit selector for the ten-byte action-profile table. Kind 9 adds live actors and pending spawners to its population check. Type the masked halfword local and encode only its table index; retain the packed input and both population scans. |

Other named callers, the specialized floor constructor, actor-spawner predicate
and harp floor-controller predicate inherit the enum directly. The rendering
view must use the same byte type, although current rendering does not read kind.
The type is not a range validator: constructor variants 23/24/41/42/44 and
unresolved stored kinds 14/22 remain representable. Their existing literal-ledger
reasons still apply. Effect type flags, renderer IDs, animation indices and
phase counters are separate domains and are not reinterpreted by this change.

## Source result

Both effect-record views and the constructor now share `KfEffectKind`. The
current source has 33 constructor calls: all third arguments are named kinds,
typed locals, or explicit decodes of packed/spell/unresolved values. This source
count is distinct from the cached navigator's 29 proven incoming references.
No unnamed variant is silently promoted to a semantic identity.

The actor action helper decodes only after testing the paired bit. Its five-bit
mask guarantees that converting to the byte domain retains the selected value.
Its typed selector and the player's weapon-effect selector retain signed-word
locals in the legacy C view; the profiled-action selector retains its unsigned
halfword. Table indices explicitly encode the enum, including fixed magic-row
references. Shared field metadata preserves byte widths and existing confidence.

Reversing only the type, local-name and boundary substitutions reproduces all
seven edited C files and the shared header exactly. Literal expressions were
reconciled without changing any of their reasons; all 111 C files and 6,073
retained occurrences remain accounted for. This review establishes source scope,
not compiler behavior. All nine edited functions remain unverified after editing:
no builds, compiler checks, tests, post-edit matches or banking were run. The
remaining unknown fields and unresolved selectors keep the naming goal open.
