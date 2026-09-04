# GAME animation cache ownership

## Retail evidence and Function Match Plan

The seven functions in `GAME.EXE:0x800205d4..0x80020b4c` share an animation
vertex cache. The per-function pre-edit CFG, signature, caller, relocation,
vendor and match snapshots are in
[`game_semantic_animation_pool.tsv`](../../config/evidence/game_semantic_animation_pool.tsv).
The binder starts at 90.793990% objdiff; the six lifecycle functions start at
100%. The plan is to recover record semantics and storage first, compare that
change independently, then test consolidation of the contiguous run.

The binder is the only proven caller of `pool_allocate` and the only writer of
the asset, clip, keyframe and rest-object metadata. It owns the vertex
allocation and installs its record in a caller-provided slot. Release clears
that same slot before freeing the cached vertices. This shared state and
lifecycle support one reconstructed `game.pool` TU, beyond address adjacency.
The historical filename and original file boundaries remain unproven.
The adjacent asset registry and item-placement loader retain separate owners.

`render_frame` marks occupied records stale at `0x8001fe08`, renders entities
and the weapon, then releases still-stale records at `0x800202d8`. The binder
sets state 2 after producing scratch vertices. Reset and release clear state
to 0. Allocation only writes the invalid clip index; it does **not** reserve
the record by changing state.

## Record and complete storage extent

| Offset | Field | Evidence |
| --- | --- | --- |
| `0x00` | `s16 state` | `lh` tests; 0 free, 1 stale, 2 refreshed |
| `0x02` | `u16 asset_index` | Binder stores its registry index and compares it on reuse |
| `0x04` | `u16 clip_index` | Compared with the selected clip; `0xff` invalidates the cache |
| `0x06` | `u16 keyframe_index` | Compared with and updated from the selected keyframe index |
| `0x08` | `struct KfMorphObject *rest_morph` | Asset base plus the keyframe's rest-object table entry |
| `0x0c` | `SVECTOR *cached_vertices` | Allocation size is `vertex_count << 3`; passed to `gteMIMefunc` |
| `0x10` | `struct KfPoolRecord **owner_slot` | Binder stores the anchor address; release clears the pointer there |

Five lifecycle scans visit twelve records at a 20-byte stride. The last field
ends at byte 20. Thus `GAME.EXE:0x800910c0` owns a complete `KfPoolRecord[12]`,
size `0xf0`, ending at `0x800911b0`. This is direct loop/field evidence, not an
extent inferred solely from the next global. The adjacent projected-vertex
and morph-scratch arrays still have only referenced-prefix extents in the
inventory; this campaign does not invent their capacities.

`pool.h` owns the shared record, binder and lifecycle API and imports the SDK's
`SVECTOR`. The morph object is forward-declared there, with its private format
definition in the owner source. `game_render.h` includes the pool interface;
it no longer owns a duplicate binder declaration. The caller-slot campaign
below recovers the five object families and the typed `KfPoolRecord **` API.
The implementation includes the asset, rendering, memory, pool and vendor
interfaces directly, without the umbrella `game.h`.

## ABI, scheduling and vendor controls

The binder's five call sites at `0x8001eaf4`, `0x8001f05c`, `0x8001f1b4`,
`0x8001f858` and `0x8001f978` supply an instance slot, asset/clip/phase and a
fifth stack argument containing the TMD vertex count. The binder narrows the
count to `u16`. Its zero/one/record return convention is unchanged, as is the
uninitialized incoming `$s5` keyframe index visible in retail.

The two word-copy statements per vertex are retained, together with the
scratch-array plus-eight referent and the extra-vector save/blend/restore.
The lifecycle counters remain `u16`, except `pool_release_all`, whose
11-to-minus-one loop has explicit signed-halfword extension in retail.
Record advances in branch delay slots and framed return delay slots remain
part of the comparison, not detached padding.

With the supplied Psy-Q Release 2.5 archives and `psyk` XDEF listings:

- `GAME.EXE:0x8004c860 gteMIMefunc`: all 128 body bytes equal
  `LIBGTE.LIB/MSC.OBJ` at member offset `0x61c`, with no masked bytes.
  `LIBGTE.H` declares `SVECTOR *`, `SVECTOR *`, `long`, `long`.
- `GAME.EXE:0x8005048c free`: all twelve body bytes equal
  `LIBAPI.LIB/C52.OBJ` at member offset zero, with no masked bytes.
  `psyq_libc.h` imports the supplied `MALLOC.H` declaration.

These calls stay separately vendored. The asset-table interpretation and
twelve-record frame-cache policy are game code. Reproduce the body checks
with `load_release25_symbols`, the named XDEF offsets and
`RetailImage.load("GAME.EXE").require(va, body_size)` after `kf init`.

## Verification

The independently built field/type/storage change leaves both pre-merge
objects' entire `.text` sections unchanged: binder SHA-256
`8fbfe3a6e7832083c58ede4377da9f3f2e91c89f61124863b129d778e81246b8`,
pool SHA-256
`e7b2249e889ad4152a40bfd0c245255f509791b71d2995516e0f89cafa5171ff`.
All six lifecycle functions remain exact and the new BSS claim passes strict
data matching. The combined binder has the same 936-byte function body;
restoring its former standalone section's eight zero-padding bytes reproduces
the same binder hash. No instruction change is hidden by the ownership move.

The final combined TU keeps all six lifecycle functions at 100% and the binder
at 90.793990%. Its first real retail difference is still the early jump to
compiled epilogue offset `+0x378`, versus retail `+0x374`; the subsequent
count-register, keyframe-selection and loop differences remain unattributed.
No referent, CFG or scheduling workaround was added to raise a score.

The full isolated animation oracle passes 1,134 cases: 1,027 shipped animated
selections/cache hits, 99 static assets, three reverse/wrap/fallback controls
and five allocation/reinitialization/release controls. It compares retail,
compiled C and Rust record/cache/vertex bytes and lifecycle behavior. This
semantic agreement does not promote the fuzzy binder to exact.

Across all three retail programs, all 348 historically exact game functions
remain exact, all 13 vendored verification functions remain exact, and all
59 data-owning units match. Only the six exact pool lifecycle rows are banked.
The campaign removes one TU; the shared record layout remains 20 bytes and
three previously opaque fields acquire supported meanings. The oracle's
candidate path and rebuild unit now follow `game.pool`, with a regression
test against manifest ownership.

## Caller-slot type recovery

The follow-up campaign starts from `fa4c113`. Its nine per-function snapshots
and verdicts are in
[`game_semantic_animation_slots.tsv`](../../config/evidence/game_semantic_animation_slots.tsv).
Seven functions start exact; `render_actor` and the binder start at
91.406010% and 90.793990%. The Function Match Plan is to replace opaque slots
with the pointer type proved by the complete bind/release chain, without
changing record extents, argument widths, control flow, constants or returns.

| Shared object view | Slot offset | Recovered field |
| --- | --- | --- |
| `KfActor` | `0x34` | `animation_cache` |
| `KfEffectRecord` and `KfEffectRenderView` | `0x34` | `animation_cache` |
| `KfMapEvent` | `0x3c` | `animation_cache` |
| `KfPlayerState` | `0x74` | `weapon_animation_cache` |
| `KfEffectSprite` | `0x18` | `animation_cache` |

Every field is a `KfPoolRecord *`, and each binder call passes its address.
The binder loads the record pointer from that slot, stores the slot address
in `record->owner_slot`, and installs the new record through the slot.
`pool_record_release` clears it through the saved slot address. Thus these
are not private byte buffers, integers or pointers directly to vertex data.
The zero/one/record binder return remains unchanged; this campaign does not
pretend the tagged result is always a dereferenceable record pointer.

The two effect views retain the same field type and offset. Map events split
the old six-byte `unknown_3a` span into two opaque bytes and the pointer at
`0x3c`. The eight-byte rotation view at `0x34` ends exactly before that pointer;
`rotation_target` remains at `0x40`. All six structure sizes are unchanged.
Headers needing only the pointer forward-declare `struct KfPoolRecord`.

`player_equip_weapon` clears the weapon slot, and `actor_pool_clear` clears
each actor slot. `save_file_read_slot` preserves the live weapon buffer and
animation record across the player-state copy. Its old integer local
`saved_07f4` is now `saved_weapon_animation_cache`, with the pointer type.
This restoration is part of retail behavior: serialized pointer bytes must
not replace the live runtime pointers.

The `effect_sprites[2]` initializer changes from four zero bytes to a null
pointer initializer. Both slot words are zero in the retail 56-byte object;
neither the sentinel nor any other byte changes. No new overlapping datum,
storage claim or inferred array capacity is introduced.

`entity_render`, `map_event_render` and `geometry_render` now import their
asset/render/math/player/state dependencies and Psy-Q interface directly,
without `game.h` or local extern replacements. The unresolved sprite table
still uses its existing `game_state.h` declaration; no owner is invented.

Additional vendor controls compare twelve unmasked bytes at each Release 2.5
`LIBAPI.LIB` XDEF offset zero: `open` (`GAME:0x800504dc`, `A50.OBJ`), `lseek`
(`0x8005027c`, `A51.OBJ`), `read` (`0x8005047c`, `A52.OBJ`), `close`
(`0x8005029c`, `A54.OBJ`) and `exit` (`0x800504ec`, `A56.OBJ`). All match.
These remain separately vendored; equipment and save policy remain game code.

After rebuilding, all 112 units have identical allocated-section bytes and
ordered relocation tuples to the pre-edit build. The comparison hashes each
allocated ELF section's name, size and bytes, plus relocations' target section,
offset, type and symbol in source order (section symbols use the section name).
Debug-line metadata is excluded; instructions, data and relocation addends in
allocated bytes are not. This includes unchanged neighboring functions and
both effect-sprite initializers, not just the nine edited functions.

All seven exact campaign functions remain exact. The two fuzzy scores are
unchanged: `render_actor` still first differs at its 160-byte compiled frame
versus retail's 168 bytes; the binder still first differs at the jump to
epilogue `+0x378` versus `+0x374`. These are existing, unattributed residues,
not reasons to add padding, artificial locals or scheduling workarounds.
The complete three-image audit preserves all 349 historically exact game
functions and all 13 vendor verification functions; all 59 data owners match.

The full animation oracle passes 1,136 retail/C/Rust cases, including the
seven current allocation/reinitialization/release controls. All 354 repository
tests pass with the Rust driver built, with no skips; `ruff check scripts tests`
and `git diff --check` pass. Only the seven exact edited functions and the six
exact cache-lifecycle functions are banked. No TU or vendored denominator
changes belong to this follow-up campaign.

Integration onto `69ca614` (the separately committed OPEN map-render campaign)
rebuilds all 113 units and passes all 359 tests. All animation-slot objects
retain their verified allocated bytes and ordered relocations; the only
fingerprint differences from the original 112-unit comparison are the new
`open.render_map` and its renamed call target in `open.render_map_cells`.
The combined tree keeps all 349 historically exact functions and 13 vendor
controls exact, with 60 matching data owners. The exact-only bank inputs are
refreshed for this integrated base.
