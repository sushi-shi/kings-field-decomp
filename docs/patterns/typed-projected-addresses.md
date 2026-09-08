# Projected vertex addresses and sprite packet views

## Function Match Plan

The OPEN projected array has a proved complete owner and an eight-byte element
stride. Prepared TMD indices contain byte offsets. The renderer can therefore
carry the base as a byte pointer instead of an integer address. Its active
ordering table belongs to the same `open_graphics_runtime` object. Probe direct
access through that known owner to replace repeated container recovery.

For OPEN sprites, reuse the existing `KfGpuFT4` union, whose SDK and packed
layouts are already checked against the authentic `POLY_FT4`. First replace
the color reinterpretation with `packed.color0`; independently probe the four
packed XY stores. The GAME sprite's prior packed-field probe changed emitted
words; use it as a documented control, not a reason to assume OPEN behaves
identically. Preserve all banked exact functions.

| Image / function | VA / bytes | Strict before | Evidence and first hypothesis |
| --- | --- | ---: | --- |
| OPEN.EXE `render_enqueue_tmd` | `8001764c / cf8` | 100% | `lhu` prepared offsets add directly to the projected base; word XY and signed halfword depth/p2 reads; twelve packet cases converge on `AddPrim`. Carry addresses as pointers, then probe the shared owner. |
| OPEN.EXE `render_enqueue_unlit_triangles` | `80018344 / 2a4` | 100% | Same projected owner and prepared offsets, two packet cases, signed averaged depth, one final `AddPrim`. Probe direct ordering-table ownership. |
| OPEN.EXE `render_enqueue_sprite` | `800189a0 / 21c` | 100% | Four SDK packed projection outputs copied by `sw` at packet offsets 8/16/24/32; SDK color result at +4. Use the existing packet union. |
| GAME.EXE `render_enqueue_sprite` | `8001e230 / 250` | 100% | Same packet word layout; previous typed XY experiment has an unattributed instruction-order residue. Keep this exact control. |

Six-view image-specific dossiers, current match state, source history and
pre-edit objects are under `build/cast-model/typed-projected-addresses/`.
The adjacent renderers and `opening_entity_render` call sites establish
object-index narrowing, signed depth bias and the shared packet/vertex state.
No signature changes are proposed. Calls to `NormalClip`, projection/color
operations and GPU constructors are separately vendored SDK functions; these
bodies contain game packet dispatch and ordering policy. They reference no
strings. The TMD switch rows remain the existing unit RODATA claim; an indirect
jump is not promoted to a newly proven target.

Each focused probe must compare raw linked words and ordered physical
referents, starting at the first divergence. Resolve referents, calls and CFG
before attributing any instruction difference. Do not invent fields, packet
padding, locals or alternate SDK types to steer emission. Keep explicit
decoding at genuine serialized byte-offset boundaries. After the final kept
source, rebuild all affected units, run the full build, target-C census,
repository tests, Ruff and diff checks, then bank only verified exact changes.

## Probe results

The byte-pointer change in OPEN TMD retains all 3,320 bytes of extent, the
call sequence, physical referents and known CFG successor lists. Exactly 41
words change: each is an `addu` with its two source registers exchanged. The
first is at +0xc8. Strict objdiff is 99.50603%, so the integer address form
is retained at this prepared-byte-offset boundary. No compiler mechanism is
claimed from the operand-order observation.

Direct access to the global ordering table emits a separate address instead
of the projected-base-relative load. OPEN TMD grows from 3,320 to 3,324 bytes
(99.45663%); its first changed word is the branch displacement at +0x6c.
OPEN unlit grows from 676 to 680 bytes (99.37278%); its first displacement
changes at +0x58 and the table load becomes a standalone address at +0x24c.
The ordered explicit referent lists consequently differ, while the table's
physical target, calls and known CFG successors agree. A local shared owner
also changes address generation and register/frame use: 3,328 bytes and
99.00843% for TMD, 704 bytes and 92.86391% for unlit. Neither form is retained.
The existing complete owner remains correct; removing its container recovery
still has an unattributed code-generation residue.

OPEN sprite color access through `KfGpuFT4.packed.color0` is fully exact and
is retained. The independent XY-member probe changes 540 bytes to 532 bytes,
with strict objdiff 93.17037%. Its first changed instruction at +0xf4 chooses
a different material-base register; subsequent scalar XY loads are reordered.
Calls, physical referents and known CFG successors agree. The four original
packed-word casts remain, alongside the four already documented GAME casts.

## Final verdict

| Function | Final strict result | Kept source |
| --- | ---: | --- |
| OPEN `render_enqueue_tmd` | 100% | Existing source restored; byte-pointer and direct-owner residues recorded. |
| OPEN `render_enqueue_unlit_triangles` | 100% | Existing source restored; direct-owner residue recorded. |
| OPEN `render_enqueue_sprite` | 100% | Shared packet union and typed color result; all 540 linked bytes unchanged. |
| GAME `render_enqueue_sprite` | 100% | Unchanged control, including all 592 linked bytes. |

All seven functions across the four selected units retain their complete
linked words and ordered referents. Six are exact; the existing GAME TMD
partial is unchanged. All 112 target-C variants parse without errors. The
written census is 717 casts, including 42 in headers; C pointer casts total
455, down 351 from the initial 806. All 713 repository tests pass (nine skips),
as do Ruff and diff checks. The full build retains 439/471 exact functions,
the existing closure/placement gates and zero artifact failures. Only the
edited exact OPEN sprite is selected for banking.
