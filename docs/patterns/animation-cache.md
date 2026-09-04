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

`pool.h` owns the shared record and lifecycle API and imports the SDK's
`SVECTOR`. The morph object is forward-declared there, with its private format
definition in the owner source. The binder retains its public rendering API
in `game_render.h`; its opaque anchor parameter remains unchanged because
the five caller object families still require their own field recovery.
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
