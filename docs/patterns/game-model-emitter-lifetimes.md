# GAME model-emitter scalar lifetimes

## Function Match Plan at `333f135`

The related campaign is the two pooled-model consumers in
`game.entity_model_render`, connected by the same frame traversal, view/light
matrices and TMD projection/enqueue interfaces. Their contiguous source run
also includes the exact 88-byte `menu_render_item_model` control. Read the six
GAME semantic views, sole caller sites, complete disassembly/CFG, adjacent
`render_map_cells` and item-preview boundaries, shared types, original source
commit `4accbc2`, compiler-attribution and animation-cache notes before editing.

| Function | Retail extent | Strict baseline | Retail controls |
| --- | --- | --- | --- |
| `render_actor` | `8001e9a4`, 532 bytes | 91.406010% | 7 blocks, 20 calls, 2 conditional branches, 2 internal jumps, 1 return |
| `render_map_object` | `8001ebb8`, 384 bytes | 95.604164% | 8 blocks, 15 calls, 3 conditional branches, 2 internal jumps, 1 return |

Both signatures are `void(record *)`: traversal sites `8001f3fc` and
`8001f2fc` pass the current 72-/44-byte pool record in the call delay slot
and do not consume a return value. Positions use low unsigned halfword
differences stored into SDK `SVECTOR`; rotations are signed halfwords. Keep
all SDK matrix types and calls. Retail `RotTrans` writes only three output
words plus its separate flag, validating the existing translation-column
output view without inventing frame padding. Neither emitter is in the
vendored inventory: their pool/definition/material policy is game code.
`LIBGTE` MTX/SMP calls remain vendored with the recorded Release 2.5 FID or
cross-overlay object-lineage evidence; their exact revision is not presumed.
There are no strings, candidate references or indirect transfers in either
emitter. All branch, call, jump and return slots remain part of the audit.

Actor evidence: ten retail address pairs, including the descriptor at
`actor_state+1`, and texture CLUT access derived from the active tpage base
at displacement -18. The current standalone globals add an independent
CLUT referent; do not delete the real CLUT identity to hide this discrepancy.
The current known CFG agrees, but the first raw difference is frame 160
versus 168. Retail later shifts the descriptor, decrements into its original
saved register in the guard's delay slot, and narrows that index to a byte.
Test an updated byte descriptor/index lifetime instead of the current
separate `int high` and two `high - 1` expressions. This is a real consumed
index, not a fake local or mask carrier. Separately test the unchanged
complete graphics-owner fixture for the proved cross-field address chain;
do not promote unresolved registry/scratch capacities or migrate production
storage. Inspect complete raw words, ordered calls/referents, predicates and
the exact sibling before retaining anything.

Map-object evidence: six address pairs and two separate `lbu` reads of the
object ID are retained; the behavior byte dispatch selects depth 15 for
0/1, 180 for 2/3 and zero otherwise. Current source has five blocks versus
retail's eight. Preserve the actual branch topology as an open question;
the historical claim that these CFGs agree is not current evidence. If
testing this function, compare retail's depth-body order (180, 15, zero)
with source case order before any more elaborate source hypothesis.

Only strict 100% plus a full raw/referent audit permits banking. Rebuild the
unit and full project before handoff; preserve all other functions and all
concurrent effect work. Record each kept or rejected trial here, not in C.

## Kept source and four-way result

The actor now updates its original byte descriptor instead of introducing
`int high`: shift right four, test the old value while post-decrementing,
then use the resulting byte index for both texture tables. The zero nibble
still selects the ordinary TMD path; nonzero nibbles select the same table
element as before. No source load, store, call, mask expression or padding
was added. Both pre-shift `andi 0xff` instructions, the separate decrement,
final byte index and descriptor/object saved-register roles now reproduce
retail under the unchanged pinned profile.

| Actor source | Storage declaration | Compiled bytes | Frame | Address pairs | Entire linked body exact |
| --- | --- | --- | --- | --- | --- |
| Old `int high` expressions | Canonical globals | 520 | 160 | 11 | No |
| Updated byte descriptor | Canonical globals | 532 | 160 | 11 | No |
| Old `int high` expressions | Existing complete-owner fixture | 516 | 168 | 10 | No |
| Updated byte descriptor | Existing complete-owner fixture | 532 | 168 | 10 | Yes |

Canonical actor strict score moves **91.406010% -> 94.751880%**. Its remaining
25 differing words are confined to frame setup/restore and material-reference
instructions at +1b4..+1e4. The entire +028..+1b0 interval agrees with retail.
The complete-owner trial preserves all 20 ordered calls, ten ordered address
pairs, both branch predicates, seven blocks and every return/delay-slot word.
Changing only the fixture root by four bytes breaks the raw result without
changing the calls. Its retail SHA-256 is
`19a2938f8f2a619971195147ef5dd84ac04395c98d912e1aac1011e5ee78d126`.
This is an owner-dependent raw-exact witness, **not a banked production
function**. It does not justify guessing registry/projection/morph capacities
or replacing the complete-owner declaration with incompatible local aliases.

The map-object switch now lists 2/3 before 0/1, matching the decoded body
order. This alone reproduces all **384 bytes**, fifteen ordered calls, six
address pairs, eight blocks, three conditional branches, both internal jumps
and the common return. Strict score moves **95.604164% -> 100%**. Both object-ID
loads, the apparently redundant masks, depth constants and 152-byte frame
remain intact. Its retail SHA-256 is
`27dd73c6a4afd8c89fcb2351eb7fbf6ff77cab62f7fa5b5c6632db7c58c9ab70`.
The two source corrections compose without changing either function's linked
result; the 88-byte exact item-preview sibling is unchanged in all trials.

These results supersede the earlier claims that the actor's masks/frame or
map-object switch could not be reproduced by source corrections. In
particular, the current original-case-order build differed first at the
switch, not the historical position-argument scheduling site. The historical
experiments remain measurements of their then-current inputs, not general
limits of this compiler.

## Verification

The focused fresh build records both strict scores. Only these two rows
change among all 484 report rows; all thirteen vendored controls remain exact.
GAME advances **311 -> 312 / 362** exact, OPEN remains **98 / 108**, and PSX
remains **1 / 1**. Sixty game functions remain partial across GAME and OPEN.
The existing graphics-owner control suite now preserves the independent
source/owner requirements and exact unit siblings in a four-way test.

Full `kf build` still fails its pre-existing data/ownership/placement gates:
source data PSX 0/1, GAME 9/42, OPEN 2/19; target relink 1/1, 75/77, 34/38;
zero artifact failures. No banked function regresses. This is not an all-green
project build or full graphics-owner migration.

Ruff and `git diff --check` pass; all 680 repository tests pass in 81.611
seconds. The final canonical objects equal the independently audited combined
trial in complete linked words, ordered calls and address pairs. Only the
production map-object renderer is eligible for the new exact bank entry.
