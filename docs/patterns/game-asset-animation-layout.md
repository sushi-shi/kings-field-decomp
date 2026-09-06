# GAME asset and animation-cache semantics

## Function Match Plan

Starting at `bee34ff`, inspect the shared model header, its registry setter,
the weapon allocation/save-preservation path and all animation-cache state
consumers. GAME retail hashes are freshly verified. Six-view snapshots,
source histories and the frozen object/report baseline are under
`build/constant-names/asset-animation-layout/`.

Rename the header's signed word at +4 to `animation_clip_count` only after
checking the shipped table extents independently of the existing parser.
Propagate `KfAssetHeader *` through the registry setter, weapon-buffer field,
allocation and save-load local. Raw archive bytes become a typed header at
the existing chunk boundary; allocation results become their actual object
or vertex-buffer type. Do not add casts between unrelated gameplay types.
Make the cache's s16 free/stale/live field a stored enum, preserving every
state read/write and the allocator's existing behavior. Audit the remaining
literals in the complete registry and pool modules.

These are game-specific asset/cache policy around the SDK's TMD and MIME
operations, outside the vendored inventory. The SDK `gteMIMefunc` boundary
continues to use `SVECTOR`; its body is not reconstructed. Preserve all
register-visible widths, calls, CFGs, delay slots and relocation addends.
Force affected compilation, compare complete objects and strict scores,
independently link exact functions against retail, then run Clang, repository
tests/lint and the full build before commit. No new size assertions.

## Independent resource evidence

The existing shipped corpus contains 169 header-prefixed assets: 99 static
and 70 animated. For each animated asset, read the clip-table start from +16
and its first offset entry. The distance between that start and the first
clip, divided by four, independently bounds the table's entry count. Every
one of the 70 results equals the signed word at header +4. All derived rows
point to bounded clips and keyframe tables. The 99 static headers instead
have zero at +4 and place their TMD at offset 20; their two unused animation
table offsets are also 20. This supports a count, not a pointer or byte offset.

| Clip count | Assets |
| --- | ---: |
| 0 | 99 |
| 1 | 33 |
| 2 | 3 |
| 3 | 1 |
| 4 | 5 |
| 5 | 17 |
| 6 | 10 |
| 7 | 1 |

Retail `render_bind_animated_instance` selects static rendering when the word
is zero. The interaction dispatcher tests it as a signed value below two
before choosing clip 1. Keep s32 storage: the data contains nonnegative
counts, but retail's signed predicate is direct evidence. The pre-existing
dispatcher registry-index expression is preserved; this field recovery does
not settle that separate referent question.

All 214 clip headers still contain 20 in their unconsumed halfword at +2.
That constancy does not distinguish a duration, format code or other authored
metadata. `unknown_02` remains unresolved; it is not renamed as padding.
Per-asset labels, SHA-256 hashes, table boundaries and clip details are in the
ignored resource evidence JSON. No retail resource bytes are committed.

## Ownership and constants

The registry already stores header pointers and selects TMD data using the
header's +8 offset. Its setter has two callers: the first common-resource
chunk and the weapon file loaded into the player's buffer. The weapon buffer
is allocated once with 49152 bytes (48 KiB) and preserved across player-state
save restoration. `KF_WEAPON_ASSET_BUFFER_BYTES` names that allocation
capacity, without claiming why the original chose that amount.

The vertex allocation holds exactly `vertex_count` SDK `SVECTOR` elements;
the former shift by three is their eight-byte stride. Multiplication by
`sizeof(SVECTOR)` expresses that ownership. The explicit allocator-result
casts are allocation boundaries, while the common-resource cast decodes an
already bounded asset chunk. Typed pointers flow through ordinary consumers.

Cache states are free 0, stale 1 and live 2 in signed halfword storage. Each
frame marks occupied records stale; rendering refreshes used records to live;
the sweep releases those left stale. `pool_allocate` only finds a free slot
and invalidates its cached clip—it does not itself mark the slot live.
The binder's mixed pointer/sentinel result and uninitialized incoming
keyframe-index behavior remain separate, explicitly recorded debt.

## Retained literal ledger

The two complete modules contain 36 ordinary literal occurrences after the
vertex-stride substitution, down from 37. The 20 function-claim and two
data-claim literals are image addresses/extents, not semantic constants.
Line numbers below refer to the final source. Repeated line numbers count
distinct tokens. The three trailing `[1]` declarations describe variable
serialized tails using the legacy C89 convention, not a one-element limit.

| Source / lines | Literal | Occurrences | Reason retained inline |
| --- | --- | ---: | --- |
| `pool.c:16,25,33` | `1` | 3 | Variable tails for keyframe offsets, morph indices and vertex deltas; their counts come from the corresponding header fields. |
| `pool.c:54` | `0` | 1 | `copy_vertices` terminates its element countdown; each iteration copies the two words of one SDK vector. |
| `pool.c:77` | `0` | 1 | Empty animation-clip count selects the static rendering path. |
| `pool.c:78,86,90` | `0` | 3 | Null cache-record pointer checks before release, reuse or allocation failure. |
| `pool.c:82,151` | `0` | 2 | First TMD object selected for both static and animated models; retail ignores the morph object's stored TMD selector. |
| `pool.c:91` | `0` | 1 | Null return on cache exhaustion; the separate static-success sentinel is already named. |
| `pool.c:99` | `0` | 1 | Null vertex allocation triggers cache release and retry. |
| `pool.c:115,116` | `0` | 2 | Origin of the accumulated clip-phase interval. |
| `pool.c:123` | `0` | 1 | Clip keyframe countdown terminates after the stored count is exhausted. |
| `pool.c:132` | `0` | 1 | Nonzero serialized reversal flag complements the blend fraction; retain the full nonzero predicate. |
| `pool.c:159` | `0` | 1 | Keyframe morph countdown terminates after its stored count is exhausted. |
| `pool.c:175,189` | `1` | 2 | Scratch vertices start one SDK vector after the extra slot used by the rest-morph blend. |
| `pool.c:180,186` | `0` | 2 | First word of the extra scratch vector, holding X/Y, saved and restored around the blend. |
| `pool.c:181,187` | `1` | 2 | Second word of the same vector, holding Z/pad, saved and restored. |
| `pool.c:185` | `1` | 1 | Include the header-sized extra vector in the SDK rest-morph call, then restore its scratch entry. |
| `pool.c:202,228` | `0` | 2 | Null owned-vertex pointer after reset or release. |
| `pool.c:204,218,267,282` | `0` | 4 | Twelve-record countdown termination in reset, mark, sweep and allocation scans. |
| `pool.c:225` | `0` | 1 | Clear the owner's live cache pointer on release. |
| `pool.c:226` | `0` | 1 | Free only a non-null owned vertex buffer. |
| `pool.c:243,243` | `1`, `-1` | 2 | Capacity minus one initializes the signed countdown; minus one terminates after all twelve records. Neither is a cache state. |
| `pool.c:283` | `0` | 1 | Null result when the scan finds no free record. |
| `asset_registry.c:16` | `0` | 1 | Archive asset countdown termination; the header supplies the count. |

Existing named constants retain their separate domains: pool capacity 12,
invalid cached clip 255, static-bind result 1, Q12 scale 12/4096, and archive
header length four bytes. The new weapon allocation capacity is 49152 bytes;
the cache-state values have moved into their stored enum without changing
their numbers. No additional tuning rationale is inferred from those values.

## Verification and function verdicts

Forced compilation of the six affected units passes. All sections of all
112 rebuilt objects are byte-identical to the `bee34ff` baseline, including
debug lines. All 484 strict function scores and the complete objdiff report
are unchanged. For the 14 exact functions in this campaign, independent
relocation application reproduces all 892 retail words, including delay slots
and ordered calls/data referents. The partial dispatcher retains its emitted
code and score. No functions are newly banked.

Modern Clang improves from 62 to 64 passing source/image variants out of 112.
The asset registry and animation pool now pass. Error diagnostics fall from
323 to 320: the three removed errors are the registry assignment and the
vertex/weapon allocator conversions; normalization of source lines/columns
finds no new errors. Other player-core errors remain. No new tests or size
assertions are added; the existing inventory test now expects the cache enum.

The full repository run exercised 656 tests in 82.307 seconds. It exposed
that stale inventory expectation and the known untracked save/load-hub
mismatch. All 110 inventory tests pass on the focused rerun after updating
the expectation, leaving only the unrelated save/load-hub failure.
Ruff and whitespace checks pass. Full `kf build` still fails on the existing
data ownership/placement and target relinking gaps: source data 7/60, SDK data
4/4, target relink PSX 1/1, GAME 75/77 and OPEN 34/38. Source/target matching
does not resolve those separate closure gaps.

All following rows select GAME.EXE and `probe-gcc257-o2-g0`; their six-view
snapshots capture pre-edit signatures, widths, CFGs, delay slots, callers,
callees and referents alongside source history and the resource evidence.

| GAME VA / bytes | Function | Initial = final strict % | Verdict / retail words |
| --- | --- | ---: | --- |
| `0x80016a30 / 244` | `player_equip_weapon` | 100 | Exact / 61 |
| `0x80016e24 / 148` | `game_initialize_session` | 100 | Exact / 37 |
| `0x8001b180 / 528` | `common_resources_load` | 100 | Exact / 132 |
| `0x800204c0 / 156` | `asset_registry_load_tmd_archive` | 100 | Exact / 39 |
| `0x8002055c / 64` | `asset_registry_set` | 100 | Exact / 16 |
| `0x8002059c / 56` | `asset_registry_select` | 100 | Exact / 14 |
| `0x800205d4 / 932` | `render_bind_animated_instance` | 100 | Exact / 233 |
| `0x80020978 / 48` | `pool_reset` | 100 | Exact / 12 |
| `0x800209a8 / 60` | `pool_mark_allocated` | 100 | Exact / 15 |
| `0x800209e4 / 72` | `pool_record_release` | 100 | Exact / 18 |
| `0x80020a2c / 108` | `pool_release_all` | 100 | Exact / 27 |
| `0x80020a98 / 108` | `pool_release_stale` | 100 | Exact / 27 |
| `0x80020b04 / 72` | `pool_allocate` | 100 | Exact / 18 |
| `0x8002beb0 / 972` | `save_file_read_slot` | 100 | Exact / 243 |
| `0x80034de4 / 2308` | `map_interaction_dispatch` | 83.04679 | Partial, unchanged |
