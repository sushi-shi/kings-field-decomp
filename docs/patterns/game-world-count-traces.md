# GAME serialized-count compiler traces

## Function Match Plan

At `14f9686`, GAME `80035b5c map_world_state_persist`, 696 retail bytes,
remains strict **96.005745%**, 700 compiled bytes in `game.map_events`.
Fresh native/traced whole ELF parity holds for source SHA-256
`a008fa56ce21aa94dbd5fb1bca56f24510ef5f849525e48b001a939a978818e3`.
Compiler fingerprint is
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`;
isolated evidence lives under `build/gcc257/game-world-count-traces/`.

The six GAME views, complete retail and candidate, three caller windows,
both adjacent bodies, inverse reader, shared record layouts, history and
the [previous writer trials](game-world-state-layout.md#writer-base-expression-revisit-at-c3700f8)
were reviewed. This no-argument, void leaf serializes game-owned floor state;
there is no vendored body, call, string or indirect transfer. Seven address
pairs and one internal jump are validated, with no candidate outgoing rows.
Preserve all conditional edges, load delays, stores and the final `jr ra;nop`.
The five other unit functions reproduce all 297 retail words.

The first 49 words already match. At +0xc4, the active count initializes
`t0` instead of retail `t1`. Both sparse sections increment this count and
backfill exactly one byte. The actor section visits 128 records and the
object section 160, so neither count can exceed 255. The exact inverse
reader loads each count with `lbu`. Source currently models the shared
accumulator as `s32`; its upper bits have no consumer. Retail does not prove
a unique local type, but the byte stream and bounded counts support testing
an unsigned-byte accumulator.

Change only `active` from `s32` to `u8` in an isolated TU. Preserve the signed
index, complete owners, sparse predicates, eight-byte link-copy loop, output
order and 8/128/190/160/10/20 traversal bounds. Inspect whether narrowing
survives as an extra instruction or changes the actual count lifetime; do
not accept a higher score if the count or stream semantics differ. This is
independent of the rejected link postdecrement and promoted-behavior controls.
No artificial carrier, storage, alias, forced register or compiler change
is introduced.

Compare the first raw divergence, all numeric referents and all five exact
siblings. Validate any claimed compiler mechanism with a small control.
Require strict 100%, complete retail-word agreement, native/debug/traced
parity, focused and full builds, and repository verification before banking.
A negative trial leaves the production source and bank unchanged.

The byte-count trial produces the same 700-byte body and ordered referents
at 96.005745%; all five siblings remain raw exact. Narrowing the accumulator
therefore supplies no retained correction. Reject this no-effect edit.

Next test the decoded shared action-check block explicitly. Each of the five
behavior comparisons branches to `check_action`; unmatched behavior reaches
`serialize_object`, and a non-idle action falls through there. The idle edge
continues the same outer loop. Keep every comparison in its existing order.
These labels represent retail blocks `80035d24` and `80035d34`, not extra
operations. Test whether this common join changes the loop's retained RTL
before invariant motion; no changed predicate or duplicate serialization is
permitted. Start from the unchanged production source, without the byte-count
control, and preserve all caller, owner, extent and verification constraints.

The explicit action join is also byte-identical at 96.005745%, 700 bytes.
Both trial bodies preserve the baseline's seven ordered numeric references
and all five exact siblings. Reject the join; the production source remains
unchanged. Neither control recovers the retail constant placement or count
register, and no new optimizer mechanism is claimed from these negative results.
Native, traced and uninstrumented debug whole ELF objects agree for the
baseline and both rejected trials. No production input or bank row changes.
