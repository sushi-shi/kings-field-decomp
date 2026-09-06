# GAME effect constructor

## Function Match Plan at `fdadfc0`

GAME `80036f44 effect_pool_construct` owns 2,092 retail bytes (`0x82c`) in
`game.effect_pool`, initially strict 52.130020% under the unchanged
`probe-gcc257-o2-g0` probe. Retail hashes and all six semantic views were
refreshed, including the complete disassembly/CFG and raw candidate reasons.
The adjacent free-slot search, typed constructor and current-record selector
are strict-exact controls. The spatial-audio callees and caller argument
windows constrain the interfaces; source history and the existing shared
effect-state ownership dossier were also inspected.

Retail has a 56-byte frame, seven direct calls, a 45-entry kind switch and
one shared return at `80037768`, including its frame-restoring delay slot.
Allocation failure reaches the epilogue without initializing a record.
The first three arguments are stored as bytes, the fourth addresses a full
16-byte VECTOR copy, and caller stack slot five addresses an eight-byte
SVECTOR copy. The remaining optional slots are consumed according to kind.
The effect policy is game-owned, with no vendored/FID identity or SDK wrapper
shape; the audio services are independently reconstructed game interfaces.

### First: faithful comparison references

The starting target lacks relocations for the 29 internal absolute jumps
and six sound calls after indirect dispatch: the rows remain `candidate`
on the `instruction-word` channel. The effect-state base pair at
`80036ffc/80037000` remains an unreviewed BSS candidate. Five fixed sound
address pairs inside switch arms are absent altogether. Validate each raw
opcode and numeric target, including the signed-low carry, against the
existing function/data owners before curating these rows. Preserve all
physical destinations and do not loosen shared validation policy.

The checked index is the reloaded unsigned record kind minus four, bounded
by 44 before indexing four-byte rows at `80012c28`. All 45 table words must
land on admitted instructions inside this body, not delay slots. Verify that
chain and all table destinations before reviewing the table candidates;
this is an owned RODATA claim, not 45 new data identities. The navigator's
unresolved indirect edge is a tooling limit, not evidence that the case
bodies are dead.

The five fixed addresses select existing magic-row SoundRef fields:
`8009ceb2` (row 4 sound 0), `8009ceb5` (row 4 sound 1),
`8009cfcd` (row 18 sound 1), `8009cfca` (row 18 sound 0), and
`8009cff2` (row 20 sound 0). They live inside the existing startup-cleared
`effect_state` owner; no additional globals or storage are justified.

### Then: directly observed source structure

After rebuilding the target, compare from the first real source divergence.
Retail reloads the stored kind at `80036ff8` after both aggregate copies;
the current source instead retains the input kind through indexing and
dispatch. Recover the record-kind read, preserving every case and its values.
The failure branch and successful initializer share one final record return;
test that ordinary source guard if the rebuilt diff still disagrees.

Further independent issues require their own focused source/layout audit:
the direction and optional rotation transfers are complete eight-byte
aggregate copies, not four scalar halfword assignments. Kind 0x24 writes
bytes at record+0x38/+0x39, whereas the current source writes halfwords at
+0x38/+0x3a. Kinds 0x13/0x14/0x18 also write a single byte at +0x38.
Do not conceal these width differences or add casts/padding to bank bytes.
Preserve the three exact siblings and all existing exact consumers.

## Reference and first source results

All 29 caller windows were inspected. Each supplies the fifth direction
pointer at caller sp+16; kind-dependent slots at +20/+24/+28 include rotation
pointers, halfword values, byte IDs and word flags. There is no uniform
eight-fixed-argument interface. The callee has seven direct calls in total;
the navigator's eighth proven outgoing control is the indirect switch jump.

Reviewing 82 existing candidate rows and adding the five decoded fixed sound
pairs makes the carved target relink all 523 instruction words exactly.
All 45 switch rows match the bounded table and avoid delay slots. Wrong
numeric-target controls fail shared validation. Without changing source,
strict comparison moves to 52.372850%; this is target repair, not a source
match. Reading `record->kind` replaces the input mask with a real byte reload
but leaves that percentage unchanged. A positive success guard with one
final record return removes the extra failure jump and reaches 53.070747%.

### Typed aggregate and variant-field plan

The complete eight-byte rotation region is already consumed through SVECTOR
SDK interfaces, and the constructor copies all four halfwords including pad.
Model it as `SVECTOR rotation`. Direction is both a complete SVECTOR copy
and unsigned kind-dependent halfword state; use one shared union exposing
the SDK vector and the existing unsigned halfwords. The two bytes at +0x38
also have genuine halfword consumers, so expose both views in one shared
union. Keep the 60-byte record, field offsets and unsigned direction/tail-word
views; rotation uses the SDK's signed components and copied pad halfword.

First migrate member paths mechanically, without changing the scalar
expressions; compare all affected compiled instruction/relocation streams.
Then replace only the evidenced eight-byte copies with aggregate assignments
and correct the constructor's +0x38/+0x39 byte writes. No overlapping globals,
per-file incompatible types, fake padding, volatile carriers or hand assembly
are permitted. Shared consumers keep their existing operations; their exact
rows must not regress. The retained fields stay address-derived where their
kind-specific meanings are not established.

The mechanical member migration leaves the complete compiled objects for
effect_pool, effect_update, effect_dispatch and player_warp byte-identical.
The inventory layout reader supports only named structures, so the shared
union views need a narrow extension: each union member starts at zero and
the extent is the largest member rounded to the largest alignment. Keep
named component structures and the existing fail-closed declaration parser;
do not invent inventory offsets to accommodate the parser. Validate both
union extents and nested record offsets before changing copy expressions.

The named-union extension also registers the existing flat notification digit
union, without changing its source. Anonymous nested packet unions remain
outside the flat layout inventory, as before. The complete layout census is
95 types / 759 fields, with 631 named fields; existing consumer objects stay
byte-identical after the named-member refactor.

Full vector assignments improve strict comparison from 53.070747% to
76.986620%. Correct byte stores then produce 76.602295%; keep the real width
correction despite the lower intermediate score. A fresh raw compile shows
the intended `sb` at +0x39/+0x38 and byte-only optional stores. All seven
ordered direct calls match retail, but two duplicated fixed sound addresses
remain, the body is 2,204 rather than 2,092 bytes, and the frame is 48 rather
than 56 bytes. All three exact siblings retain every linked word.

The next entry hypothesis follows the call delay slot literally: retail
initializes the optional-slot cursor before entering effect_pool_find_free,
while current C initializes it after the call. Move that real initialization
before allocation and compare. Then recover the individually decoded case
order and shared arms; no extra locals or artificial register constraints.

Initializing the cursor before allocation recovers the full 56-byte frame,
all entry saves/register roles and the call delay slot (79.414920%). Restoring
the decoded case order reaches 90.990440%. The remaining duplicated source
tails are the `0x17 -> 4` join at `800370cc` and `0x29 -> 0x20` join at
`80037130`; use explicit shared initialization labels at those proved joins.
Other identical arms already merge naturally and do not need forced sharing.
Kind 0x0a loads optional slot three once and publishes that halfword to
unknown_08, scale_z, scale_y and scale_x in that order. Replace four distinct
loads with a chained assignment preserving that publication order.

The two explicit joins reach 96.351814%; the single optional-scale read then
reaches 98.652010%. All seven ordered call targets and all eight ordered data
addresses now agree. The candidate is four bytes longer; the first non-target
divergence is kind 6 reading optional slot one after writing record+0x38,
where retail reads it first into v1, then writes +0x38 and +0x3a. Capture that
actual argument value before the two publications. Kind 0x13 likewise reads
its byte argument before clearing three rotation halfwords, then publishes
the byte in the exit delay slot. These are bounded input snapshots, not fake
locals. Finally, kind 0x15's current X/Z/Y assignment order disagrees with the
retail load-delay sequence; test ordinary X/Y/Z rotation publication.

## Exact closure and verification

The kind-6 argument capture removes the extra load-delay nop and recovers the
2,092-byte extent (99.235180%). The kind-0x13 byte capture reaches
99.617590%; natural X/Y/Z publication in kind 0x15 closes strict **100%**.
The pinned profile, optimization and assembler settings are unchanged.
Historical compiler attribution is still open; these are source/dataflow
corrections, not evidence for a compiler-wall taxonomy.

A separate fresh compile reproduces every one of the constructor's **523
linked instruction words**, all seven ordered direct calls, eight ordered
materialized addresses, branch destinations and delay slots. The complete
45-row compiled switch table independently relinks to the retail table.
The three adjacent controls preserve all their words: free-slot search
`80036f00/44`, typed constructor `80037770/ac`, and selector `8003781c/34`.
Thus all four functions in `game.effect_pool` are strict exact. Removing
obsolete pointer casts in favor of typed vector and SoundRef members preserves
the result; effect_update, effect_dispatch and player_warp retain their entire
original compiled-object hashes despite the member-path migration.

Only the constructor changes among all **484** reported function rows:
52.130020% to 100%. GAME moves **291/362 -> 292/362**, OPEN remains **98/108**,
PSX remains **1/1**, and the overall total is **391/471**. No existing exact
regresses, and no vendored function or denominator changes. Only the new exact
constructor is selected for banking.

The full `kf build` runs after fresh affected-unit compilation. Its unfinished
data/reference/placement gates remain red, with no artifact failures: source
data improves **5/60 -> 6/60** because the effect-pool unit now passes; the four
independent SDK data contributions pass; target relink remains **110/116**
with six conflicting section-base cases. Known-reference ownership remains
incomplete. These are not treated as whole-project closure.

`ruff check scripts tests`, `git diff --check`, and all **649** repository
tests pass (80.839 seconds). The four constructor controls include a fresh
compiled-unit/table comparison and retail-target negative checks. The narrow
named-union layout extension passes maximum-size/alignment, nested-offset and
unknown-member controls. `nix flake check -L` also passes; its isolated 649-test
run has the normal 135 unavailable-retail/tool skips. Generated probes,
objects, raw listings and logs remain uncommitted under `build/`.
