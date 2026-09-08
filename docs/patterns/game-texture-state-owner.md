# GAME texture-state ownership campaign

These are historical controls preceding the adopted complete graphics owner.
For the subsequent migration and current model, see
[game-graphics-runtime-owner.md](game-graphics-runtime-owner.md).

## Function Match Plan

Continue the remaining GAME/OPEN matching goal from `1dcfcce`. The complete
graphics-owner pilot already demonstrates linked retail agreement for several
functions, but its registry and projection/morph extents are still unresolved.
Test the connected texture/cache/notification/floor-item field family as a
smaller shared declaration before attempting a production migration.

The candidate spans GAME `80095038..80095698`: three effect texture pages,
three CLUTs, active material, HUD and notification selectors/intensity, queue
state and the floor-item material/count/table. Existing typed fields retain
their independent addresses and extents; the two ten-byte intervals, one byte
after HUD intensity and six bytes before floor items remain unresolved. The
start is the first effect-cache field, and the end is the existing 64-record
floor-item table's end. This is a WIP grouping, not proof of an original
declaration boundary or permission to guess scratch capacities.

The grouping has evidence beyond adjacency: `render_actor` derives its CLUT
cache from the active tpage address minus 18; `render_entities` derives floor
items from active tpage plus 62; `render_frame` publishes the HUD/text/digit
selectors into that same active material. `render_initialize` initializes
the queue and floor/HUD materials, while the floor loader owns the two-pass
placement expansion. These decoded chains constrain a shared field family.

| GAME function | Bytes; strict baseline | Evidence and first source hypothesis |
| --- | --- | --- |
| `80020b4c item_load_floor_placements` | 432; 98.888885% | Pointer from MIX payload+4; 40-byte frame, one rand call, three validated pairs, unsigned count/sentinel and 12/24-byte strides. Use direct count-member accesses as in the exact OPEN homolog and complete-owner control, preserving reset, saved start, postincrement scan and expansion. |
| `8001e230 render_enqueue_sprite` | 592; 93.972980% | Sprite pointer, signed halfword depth bias, word depth-cue mode; 136-byte frame, five SDK calls and seven pairs. Keep all projection/packet operations; express CLUT/page and RGB/command bytes as shared material fields, preserving the CLUT+4 color pointer. |
| `8001e9a4 render_actor` | 532; 94.751880% | Actor pointer from the scene traversal; 168-byte frame, twenty calls, ten address pairs plus two validated internal jumps. Keep byte descriptor shift/postdecrement and texture publication; express both effect caches and active material under the shared owner. |
| `8001fde4 render_frame` | 1304; 100% | Nullable VECTOR/SVECTOR forwarding, 80-byte frame, twenty calls, 64 pairs and four internal jumps. All HUD/notification loads, stores and reloads must remain exact. The earlier eight-byte material owner lost eight body bytes and is a negative control. |
| `8001bce0 render_initialize` | 728; 100% | No arguments; 32-byte frame, fifteen calls, 41 pairs. Preserve allocations, independent matrix addresses, SDK selectors, queue byte clearing and return delay slot. |
| `8001bae4 effect5_texture_cache_prepare` | 176; 100% | Floor byte passed as the existing promoted domain; 24-byte frame, six calls/pairs, floor-five guard with ra save in its delay slot. Keep all three page and CLUT values and their independent stores. |

The six image-qualified views, complete retail bodies, constraining callers,
adjacent source claims and history are reviewed before the temporary compile.
No outgoing candidate reference or string belongs to these functions. They
implement game rendering/resource policy; SDK GPU/GTE APIs and rand retain
their archive attribution and authentic declarations. No vendored body is
reconstructed or counted as progress.

Initially change only these selected function references in disposable TU
copies and one shared candidate header. Measure every field offset and the
complete 0x660-byte extent with the pinned compiler. Compare whole linked
bodies, ordered referents, calls and delay slots against hash-checked retail,
including a wrong-root control. Trace native/instrumented parity and relevant
address lifetimes for any successful candidate. A raw temporary result is
evidence for migration, not a banked objdiff result.

If the grouping preserves the exact controls, audit every remaining consumer
before migrating its shared header, sole DATA definition and curated interior
identities together. Keep all numeric destinations and real types. Rebuild
every affected unit, require strict objdiff 100% for each newly closed
function, run the full build and repository checks, and bank only verified
exact rows. Reject the grouping if it requires incompatible per-file views,
false extents, fake padding or any banked function regression.

## Smaller-owner verdict

The pinned compiler measures all 25 field offsets and the complete 0x660-byte
extent correctly. The loader, sprite emitter and actor renderer each reproduce
every linked retail word; shifted-root controls fail. Effect-cache preparation
also remains exact. However, frame rendering becomes 1296 instead of 1304 bytes,
with a different saved-register set, and initialization becomes 732 instead of
728 bytes with a larger frame. The grouping is rejected; no production source,
inventory or bank entry changes.

## Next full-owner control

Return to the complete startup-clear owner and test an explicit typed layout
for its unresolved array spans in disposable source. The decoded starts bound
the registry interval to 0xf0 bytes, projection interval to 0x1f40 bytes and
morph interval to 0x1f48 bytes. At the observed pointer/record widths these
would hold 60 pointers, 1000 screen vertices and 1001 SVECTORs. These are
candidate storage extents, not recovered declaration capacities: the shipped
resource census establishes only registry use through 47 and vertex counts
through 664. Agreement with compiled code must not promote those bounds.

This control asks whether replacing opaque casts with typed member accesses
preserves the complete-owner results across all 80 direct consumers in 35
units. Keep all arithmetic, source lifetimes, signatures, calls and numeric
referents unchanged, except the independently reviewed direct count-member
form for the placement loader. Preserve every exact function in each compiled
unit, including functions without an owner reference. A typed-layout success
still requires a reviewed ownership model, all-consumer evidence, consistent
curated relocation/data migration, strict objdiff and full verification before
banking. A failed typed layout is evidence against this proposed declaration.

## Typed six-function control result

The candidate compiler layout agrees at all 40 measured field offsets and the
complete 0x249cc-byte extent. All six reviewed witnesses reproduce every linked
retail word, including the three previously exact controls. Moving the candidate
owner by four breaks each comparison without changing call targets. This is a
six-function result: the 80-consumer census and generated dossiers do not mean
that the other consumers have been reviewed or compiled with this declaration.

Native and instrumented compiles of both owner forms also have identical whole
ELF bytes for initialization and frame rendering. The observational traces show
an additional address lifetime in the rejected smaller-owner form:

| Function | Smaller-owner address lifetime | Complete-owner control |
| --- | --- | --- |
| `render_initialize` | The count address, pseudo 83, crosses 13 calls, has live length 260 and receives hard register 17. The frame is 40 bytes and saves s0/s1/ra. | The corresponding pseudo has no allocation references and crosses no calls. The 32-byte frame saves s0/ra, agreeing with retail. |
| `render_frame` | The active-tpage address, pseudo 187, crosses eight calls, has live length 136 and receives hard register 18. The 80-byte frame saves s0..s4/ra. | The corresponding pseudo has no allocation references and crosses no calls. The 80-byte frame saves s0..s3/ra, agreeing with retail. |

The smaller-owner address assignments survive both CSE stages and allocation;
the complete-owner assignments do not survive to allocation. These are observed
probe states, not proof of the original declaration or an attribution to a
particular optimizer cost rule. Trace provenance identifies compiler source
`ea9c07d5f5a67e28a6e3bfd21c5c53607543832a9be6561329591d0433f521fa`.

Generated controls remain under `build/gcc257/full-typed-owner/` and
`build/gcc257/texture-owner/`. No production C, curated data identity, relocation
or bank entry changes. GAME remains 317/362 and OPEN 106/108 strict exact;
the candidate array capacities and complete-owner migration remain open.

## Consumer-census correction after `ce37455`

A fresh manifest/source scan excludes comments before recording direct field
uses. It finds **79 direct consumers in 35 units**, including 61 currently
exact functions. The earlier count of eighty included `render_weapon`, whose
comment mentioned the state but whose body has no direct owner reference;
its indirect consumer/control role is unchanged. No actual consumer is newly
added or lost. Current function extents, source/input hashes, field sets and
strict scores are recorded in `build/gcc257/full-typed-owner/current-consumers.json`.
The three speculative typed array capacities remain unproved. Continue with
the explicit unresolved representation and per-consumer evidence audit; the
six raw-exact typed witnesses do not authorize replacing unknown storage with
asserted original capacities.
