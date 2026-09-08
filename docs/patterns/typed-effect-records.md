# Shared effect record types

## Function Match Plan at `28ba9ae`

This stage of the [typed-storage campaign](typed-storage-cast-campaign.md)
removes the duplicate rendering definition of the GAME effect pool. The
constructors already establish one 60-byte `KfEffectRecord` with a complete
SDK `VECTOR` position, eight-byte rotation, direction and animation slot.
The renderer's `KfEffectRenderView` describes the same address and stride,
but marks known producer fields opaque and recasts position halves to words.

Hash-identical GAME retail was initialized. The six semantic views were
refreshed for each edited function, together with adjacent controls, the
rotation callee and constraining callers. Source history, the
[constructor](game-effect-constructor.md),
[scatter](game-effect-scatter.md) and existing shared-effect dossiers were
read. All selected units retain `probe-gcc257-o2-g0`; that is a matching
probe, not historical compiler attribution. No function in this campaign is
identified as vendored. Their pool, kind dispatch and rendering policy are
game-owned; `rand` and the matrix/GTE SDK providers remain independently
vendored controls.

| GAME function | VA / bytes | Initial strict % | Blocks / direct calls / branches / returns | Direct evidence for the change |
| --- | --- | ---: | --- | --- |
| `render_actor_sprite` | `8001eedc / 1e8` | 100 | 8 / 20 / 5 / 1 | Only caller passes the live pool record; `lhu` position +0c/+10/+14, `lh` scale +24/+26/+28, rotation pointer +1c, animation slot +34 |
| `render_entities` | `8001f218 / 580` | 96.667610 | 61 / 9 / 41 / 1 | Effect cursor is `effect_state+1e0`, advances 60 bytes; full signed position words feed division by 2000 |
| `player_warp_shimmer` | `80036618 / 238` | 100 | 23 / 9 / 14 / 1 | Constructor result supplies the shared record; yaw +1e is wrapped by 0xfff and stored in the loop delay slot |
| `effect_pool_construct` | `80036f44 / 82c` | 100 | 52 / 7 / 38 / 1 | All 29 callers constrain byte header inputs, full VECTOR position and stack SVECTOR arguments; optional rotation copies include pad |
| `effect_pool_spawn_typed` | `80037770 / ac` | 100 | 3 / 1 / 1 / 1 | Four initial halfword arguments and two caller-stack words; +1c/+1e/+20 are real halfword stores into reused rotation storage |
| `effect_projectile_update_3d` | `80037fe0 / 2b8` | 100 | 28 / 10 / 20 / 1 | `RotMatrix` receives +1c; signed pitch/yaw loads feed angle helpers and comparisons |
| `effect_scatter_triple` | `800386c4 / 68` | 100 | 1 / 3 / 0 / 1 | Both callers supply an eight-byte direction object; only unsigned halves +0/+2/+4 are mutated; pad is untouched |
| `effect_update_dispatch` | `80038a38 / 180c` | 96.939570 | 257 / 69 / 204 / 1 | Copies eight direction bytes to the scatter local; supplies its SDK vector to construction and the live direction fields to scatter |

Branch counts include internal absolute jumps and decoded conditional
branches. Constructor and dispatcher also each contain a switch-table
indirect jump; their reviewed inventories remain unchanged. All outgoing
references are proven control or validated targets; none is candidate.
The dispatcher's incoming candidate rows remain hypotheses. There are no
strings in these eight bodies. Every return includes its frame-restoring
delay slot. Preserve the constructors' unaligned eight-byte copies, the
scatter stores in successive `rand` call slots and all ordered references.

The source hypothesis is one record, with a shared `KfEffectRotation` union
containing authentic `SVECTOR vector` and six-byte `KfEulerAngles angles`.
The former preserves complete constructor/SDK transfers; the latter serves
the game rotation helper, which reads signed halves +0/+2/+4 and never pad.
The helper also accepts real six-byte objects elsewhere, so widening its
interface would lack evidence. Assert both views' offsets and the unchanged
record extent. Propagate the union member through every producer/consumer.

The renderer uses the owner's header identities, full position words for
culling, and explicit numeric narrowing for its low-halfword screen delta.
Scale storage remains unsigned: dispatcher and shimmer use unsigned
arithmetic/comparisons, while the renderer explicitly sign-extends it for
the SDK. Remove four redundant `MATRIX *` casts. Give scatter the existing
`KfEffectDirectionWords *` type and named x/y/z accesses; its caller's local
uses the existing direction union for the complete SDK copy. Preserve its
arithmetic, three independent RNG calls and untouched fourth halfword.

Rebuild affected units, compare linked words and ordered targets against
saved pre-edit objects, and inspect the first real divergence. Every exact
function must remain 100%; partial functions retain their honest verdicts.
Run inventory/layout controls, repository tests, Ruff, `git diff --check`
and a full build before banking the selected exact functions and committing.

## Verification and final verdicts

The shared owner and both union interfaces reproduce the original instruction
streams on the first focused build. The explicit signed scale conversions
retain the three retail `lh` instructions; numeric position narrowing retains
the `lhu`/subtract/halfword-store sequence. Removing the already-signed yaw
cast in the dispatcher also leaves its complete body unchanged. There is no
new first divergence or new code-generation residue in this stage.

| Function | Final strict % | Final verdict |
| --- | ---: | --- |
| `render_actor_sprite` | 100 | All 122 linked words unchanged and retail-exact |
| `render_entities` | 96.667610 | Every pre-edit linked word unchanged; existing partial match |
| `player_warp_shimmer` | 100 | All 142 linked words unchanged and retail-exact |
| `effect_pool_construct` | 100 | All 523 linked words unchanged and retail-exact |
| `effect_pool_spawn_typed` | 100 | All 43 linked words unchanged and retail-exact |
| `effect_projectile_update_3d` | 100 | All 174 linked words unchanged and retail-exact |
| `effect_scatter_triple` | 100 | All 26 linked words unchanged and retail-exact |
| `effect_update_dispatch` | 96.939570 | Every pre-edit linked word unchanged; existing partial match |

All 21 functions in the six affected units preserve their entire linked
bodies, ordered calls and numeric data targets: 18 exact and three existing
partials, including the unchanged 2D projectile control. Section-symbol data
bases were derived from the unit's claimed symbols and checked for agreement;
relocation bits were resolved, not masked. Overall strict counts remain
439/471: GAME 332/362, OPEN 106/108 and PSX 1/1.

Eight C-file pointer casts disappear, together with one redundant scalar
cast. Six genuine numeric conversions express the renderer's different
width/sign interpretation of the shared storage. The AST census now has
838 written casts: 598 pointer targets, of which 583 occur in C files, and
240 scalar targets. C-file pointer casts are down 223 from the starting 806.
The layout inventory has 106 types and 818 fields, 725 named: the duplicate
22-field rendering view is replaced by the two-member rotation union.

All 713 repository tests pass with nine skips, including shared union,
animation-slot and inventory checks. Ruff and `git diff --check` pass.
The final focused rebuild and subsequent full `kf build` preserve every
score; full-image verification still fails on the pre-existing data
ownership, reference-closure and section-placement gates, with no artifact
failures. Only the six directly edited exact functions are banked here.
Vector-prefix interfaces elsewhere in the effect dispatcher remain part of
the wider vector campaign; they are not resolved by this record-view change.
