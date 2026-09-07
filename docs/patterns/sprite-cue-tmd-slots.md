# Sprite depth cueing and TMD slots

## Function Match Plan

Name the projected-sprite renderer's full-word option and propagate a shared
`KfSpriteDepthCueMode` through both image implementations and every caller.
Preserve the exact-one predicate: only one enables `p + (p >> 1)` before
`NormalColorDpq`; every other value takes the normal path. Rename the local
factor to `depth_cue`, leaving the signed shift and addition intact.

Promote map/entities/menu-item TMD slots to `KfTmdSlot`, preserving the unsigned
halfword register/select arguments. Explicitly encode slots at pointer-array
indices. Carry the domain through the release wrapper with its existing s32
legacy parameter and retain that unused parameter's candidate confidence.
All existing slot callers already use named constants.

## Retail evidence and source result

| Image / function | VA / extent | Evidence and resulting source |
| --- | --- | --- |
| GAME `render_enqueue_sprite` | 0x8001e230 / 0x250 | Word comparison at 0x8001e400 selects the arithmetic half-term at 0x8001e410 before NormalColorDpq. Five calls pass zero or one. Typed mode and named cue factor; primitive allocation, material references, coordinates and biased depth unchanged. |
| OPEN `render_enqueue_sprite` | 0x800189a0 / 0x21c | Word comparison at 0x80018b3c selects the same shift/add at 0x80018b4c. Its sole caller passes one. Shared mode with the same predicate and signed arithmetic; the existing separate allocation and material owner remain. |
| GAME `render_floor_item` | 0x8001ed90 / 0x14c | Calls 0x8001e230 with a2=1 at 0x8001ee94. The floor-item argument now names boosted depth cueing; position, facing, bias and animation updates stay intact. |
| GAME `render_actor_sprite` | 0x8001eedc / 0x1e8 | Billboard path passes a2=0 at 0x8001effc. Names normal depth cueing without changing model-path selection or the zero depth bias. |
| GAME `render_frame` | 0x8001fde4 / 0x518 | Notification calls at 0x80020230, 0x80020254 and 0x800202a0 all clear a2 in their delay slots. All three now use normal depth cueing; depth biases remain zero. |
| OPEN `render_floor_item` | 0x800190f4 / 0x14c | Loads a2=1 at 0x800191dc before the call at 0x800191f8. Uses the boosted member while preserving its frame counter and facing branches. |
| GAME `tmd_select` | 0x8001c0e8 / 0x2c | Masks a0 to 16 bits, scales by four, loads a slot and writes current_asset. Typed slot with explicit u16 array-index encoding. |
| OPEN `tmd_select` | 0x80016e68 / 0x2c | Same halfword selection with OPEN's separate table and current_asset. Typed slot with explicit u16 array-index encoding. |
| GAME `tmd_register` | 0x8001c5b0 / 0x3c | Masks a0 at 0x8001c5c0, publishes the payload to the slot and current_asset, then prepares packet indices. Typed slot preserves the publication and call order. |
| OPEN `tmd_register` | 0x80017330 / 0x3c | Same slot masking and two stores before preparation. Typed slot preserves OPEN's table ownership. |
| GAME `tmd_release_last_allocation` | 0x8001c5ec / 0x20 | Ignores a0 and calls the LIFO allocator. The sole caller passes slot four. Keep the candidate argument, its legacy s32 representation and unchanged release behavior. |
| OPEN `tmd_release_last_allocation` | 0x8001736c / 0x20 | Identical ignored-argument body, with no decoded caller. The shared unused slot contract remains candidate; enum typing does not establish its historical purpose. |

Existing GAME dossiers, fresh OPEN disassembly/CFG, xrefs, strings and stored
match reports were reviewed with source history and curated evidence. Calls are
proven direct edges and the cited data pairs are validated referents. These
functions implement game selection and composition around separately attributed
GTE, GPU and memory callees; no SDK body or API is redefined.

The normal mode is used for effect billboards and notifications; floor items
use the boosted mode. The observed boost adds half the signed interpolation
factor. Its artistic rationale is not established, and the operation is not
rewritten as a multiplication or division that could alter rounding.

TMD slot zero contains map geometry, one contains scene entities and four is
GAME's menu-item model. These are registry slots, distinct from object indices
inside a selected TMD. The array storage remains pointers to payloads. The
enum names known roles; it adds neither an array bound check nor a claim that
OPEN's two-entry table supports GAME's menu slot. The lifetime wrapper still
releases the last allocation, not an allocation looked up by slot.

Eight inline mode values are now named. The [OPEN sprite ledger](open-sprite-literal-ledger.md)
documents its 23 remaining literals. Existing notification and OPEN floor-item
ledgers drop the newly named arguments and retain the independent zero biases.
Declarations, curated signatures and existing inventory expectations agree.

Builds, tests and post-edit match verification remain deferred until the naming
pass finishes, as requested. Stored scores are previous baselines, not evidence
that these edits have been compiled or matched. No new exact result is banked.
