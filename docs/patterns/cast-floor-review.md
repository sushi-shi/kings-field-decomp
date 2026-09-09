# Remaining casts: first decision pass

This is the historical 542-site baseline. The subsequent
[owner and storage reduction](cast-owner-reduction.md) removes fifteen more
casts, leaving **527 total / 430 pointer / 97 scalar** at `4938eb81`.
Its master-integration section records the later combined census. That report supersedes
the counts, dispositions and stopping decision below. The earlier experiments
remain useful evidence, but did not establish an irreducible cast floor.

The final decision pass at `48ff2dc8` closes the complete remaining-cast
review. **542 written casts remain: 445 pointer and 97 scalar. There are
zero undecided sites and no deferred research queue.** The fifteen tested
source alternatives all move away from retail; none is retained. The final
production function instructions, data and ordered relocations are unchanged.

| Final disposition | Pointer | Scalar | Total |
| --- | ---: | ---: | ---: |
| Accepted conversion boundaries | 348 | 64 | **412** |
| Parked for this cleanup campaign | 97 | 33 | **130** |
| Undecided | 0 | 0 | **0** |
| **Total** | **445** | **97** | **542** |

The pointer regression ratchet is tightened from **882 to 445**. The current
working floor is **542 total / 445 pointer / 97 scalar**. This is a completed
campaign decision, not a mathematical claim about every possible C spelling.
No casts are excluded from counting, and no other cleanliness floor is changed.

## Programming judgment before reopening source work

Some parked casts are still desirable cleanup. The disposition above records
the decision to retain them in this matching campaign; it does not establish
that all 130 are equally good source code. Reviewing the actual expressions
gives the following priorities:

| Sites | Source-quality judgment | Decision for this campaign |
| ---: | --- | --- |
| 79 owner-recovery casts | Clear cleanup target. Repeatedly converting a field address to an integer, subtracting its offset and recovering the known owner obscures a simple member access. Prefer the complete owner directly. | Retain: the tested direct-owner forms regress banked functions. |
| 11 registry/projection/morph storage casts | Type-model debt. The source would be clearer with correctly typed storage and one scratch view. Deleting conversions while retaining byte arrays would not solve the modeling problem. | Retain: the available evidence does not establish the complete subdivision and capacities; the shared scratch control also regresses an exact function. |
| 8 packed sprite XY casts | Small worthwhile cleanup. The existing `prim->packed.xyN` members express the word store more clearly than casting the address of `prim->sdk.xN`. | Retain: those member substitutions regress both exact sprite functions. |
| 24 packet-body casts | Reasonable C. Each switch case introduces its own typed packet local. Sharing the conversion could reduce repetition, but the current scopes are easy to understand. | Accept the current per-case style; no further standalone cleanup effort. |
| 8 width/alignment casts | Low-priority cleanup at most. Five caller casts make truncation explicit, two fade casts specify signed narrowing, and the copy-source cast supplies alignment information. | Accept the explicit conversions; no further standalone cleanup effort. |

Thus **98 sites belong to source-quality cleanup families**, while **32 do
not justify further standalone effort**. These are counts of existing sites,
not a promise that 98 casts can be removed. The 79 owner-recovery sites in
particular include several casts within each repeated expression.

The recommendation at this stage was to keep the current implementation,
with those quality compromises stated plainly. There is no additional probe
queue. The value **445 is the measured pointer-cast count and regression
ceiling**; calling it a working floor means this campaign has stopped there.
It does not establish an irreducible minimum or erase the remaining type debt.

## Function Match Plan, executed

Use the existing hash-checked retail files and pinned toolchain in an isolated
worktree. Refresh image-qualified disassembly/CFG, callers, callees, strings,
data references, current match state and source history for the registry,
projection/animation consumers and all previously tested substitution families.
Keep proven calls, validated references and candidate hypotheses distinct.

Resolve the eleven storage sites by checking shipped registry/archive ranges,
projected object counts and animation scratch ranges against decoded accesses
and callers. If the complete subobject extent cannot be established, park the
cast without inventing an array capacity. A byte-identical address expression
does not establish an allocation's original declared size.

Reproduce fifteen finite controls using temporary source copies: GAME/OPEN
direct graphics owners, the OPEN arena owner, two shared TMD packet views,
two packed sprite XY forms, two caller-width families, the darkness fade
local, growth-table copy alignment, the common animation byte base, shared
morph scratch view and OPEN projected byte base. Each control changes one
source cause and receives its own instruction, extent, ordered-call,
relocation and CFG-clue verdict. This is not a source-permutation campaign.

Keep a humane reduction only when the shared model is supported and banked
exact functions survive. For existing partials, inspect the actual width and
control-flow differences instead of treating a percentage as the verdict.
Finish every site disposition, set the measured pointer ratchet, run the full
build, repository tests, lint and whitespace checks, then commit only the
campaign. Generated objects, sources and reports stay under `build/`.

## Accepted boundaries: 412 sites

| Family | Sites | Final reason |
| --- | ---: | --- |
| Serialized records and parsed packet bodies | 74 | File bytes and variable asset-relative offsets acquire their record type at decoding. Smaller dispatchers already share one parsed body. A conversion-hiding helper or extra union would move the same boundary. |
| Prepared projected-vertex and normal byte offsets | 150 | Prepared fields contain byte offsets. Typed access and the authentic GTE vector interface require the conversion. Element indexing would introduce an offset-to-index operation. |
| Heterogeneous GPU storage | 54 | Primitive mode selects the packet type and allocation size; fields after the boundary already use shared packet types. |
| Generic allocations entering typed consumers | 12 | The allocator returns `void *`; each consumer establishes its concrete type. The checked modern view also requires this conversion. |
| Native O32 varargs and format payloads | 24 | Argument homes carry heterogeneous words and deliberately narrowed values. Moving casts into argument macros does not remove that ABI boundary. |
| Authentic SDK boundaries | 26 | Six CD command payloads, eight word buffers, nine matrix outputs, one CD-record revision boundary and two overlay argument conversions. |
| Addresses, allocator words, whole-object bytes and cursor publication | 12 | Seven literal/sentinel addresses, three allocator word conversions, one complete-object clear and one typed-to-byte cursor publication. The representations differ. |
| Numeric operations and enum domains | 60 | Signed extension, logical shifts, wrapped ranges, indexing widths and enum encoding affect arithmetic before its destination. These are real conversions. |
| **Total** | **412** | |

The SDK contracts were checked against bodies as well as headers. `RotTrans`
writes three words into twelve-byte `MATRIX.t`, whereas `VECTOR` has sixteen
bytes. Retail `CdSearchFile` copies five words into the game's twenty-byte
record; the supplied header's `CdlFILE` has twenty-four bytes. These explicit
representation/version boundaries are retained. See the
[earlier SDK audit](typed-cast-audit.md).

The accepted byte-offset family includes the OPEN projected base; the accepted
serialized family includes animation byte-base conversions. Their alternative
forms are also reproduced below. Their primary classification remains the
actual representation boundary, so they are not counted twice.

## Final parked decisions: 130 sites

| Family | Sites | Final decision |
| --- | ---: | --- |
| Complete-owner recovery | 79 | Retain the current expressions. Direct owner substitutions change retail base selection, addressing, extent or frame use even though the complete owner is already known. |
| Large TMD dispatcher body views | 24 | Retain per-case views. One parsed view grows both frames by eight bytes and introduces packet-cursor spills. |
| Sprite packed XY stores | 8 | Retain packed SDK word casts. Direct existing members change instruction order and body extent in both images. |
| Explicit narrow caller arguments | 5 | Retain the retail caller masks. Three damage-index and two fade-argument cast deletions change `andi` to `move`. |
| Darkness fade local | 2 | Retain the word local with explicit narrowing. The alternative changes the signed branch/shift sequence and worsens the existing partial; it fixes no wrong object model or numerical operation. |
| Growth-table copy alignment | 1 | Retain the typed source boundary. Its deletion loses alignment information and adds an alignment test plus an unaligned copy path. |
| GAME registry storage | 5 | Retain typed accesses into the unresolved interval. All shipped slots are accounted for, but neither those slots nor neighbor distance establishes a complete pointer-array declaration. |
| GAME projected storage | 3 | Retain the three typed producer views. The complete shipped object census establishes actual counts, not a declared projection capacity or complete subdivision. |
| GAME morph scratch | 3 | Retain the explicit views. All referenced morph ranges fit; the original complete scratch extent is still not established. The single-view control is also non-exact. |
| **Total** | **130** | **No pending investigation remains in this campaign.** |

### Registry and projection/morph decision evidence

The read-only corpus check visits **246 TMD payloads and 1,911 object tables**,
checks their vertex arrays against the enclosing payload, and parses all
**169 asset-header resources**, including **70 animated assets**, **813
keyframes** and **827 morph ranges**. It uses the repository's existing
bounded chunk, archive and animation readers; it does not run the game.

All five floor effect archives contain eighteen assets and occupy registry
slots **30..47**. Map-event archive counts are **6/4/2/2/2**, starting at ten;
ordinary actor counts are **7/8/6/7**, starting at zero. B5's three alternate
actor archives each contain six assets. Weapon/common assets occupy slots
20/21. This exhausts the relevant shipped registrations and confirms slot
47 as the highest used slot. It cannot distinguish a 48-entry array plus
unrelated storage from other declarations in the 240-byte interval.

The largest projected object is **B1 MIXB raw chunk 0, object 123: 664
vertices**. The largest animated copy is **B5 CHR3 asset 5: 651 vertices**,
written from scratch element one through element 651. Every parsed morph
range satisfies `base_vertex + vertex_count <= vertex_count_of_asset`.
The extra prefix blend therefore ends no later than scratch element 651;
its prefix entry at `base_vertex` is saved/restored. These checks account for
the `+0x1f40` scratch base, copy start, prefix entry and count-plus-one access.

Decoded projection loops accept caller-supplied counts and advance eight
bytes; they supply no independent capacity guard. Shipped maxima likewise
supply no complete-object bound. A declaration of 60 pointers, 1000 projected
vertices or 1001 scratch vectors would still be a guess. A new union,
overlapping extern or conversion helper would conceal the same uncertainty.
The enclosing graphics owner remains supported. These eleven casts are now
**parked**, rather than left awaiting another investigation.

### Reproduced controls

Every row below preserves its ordered external call sequence. Strict scores
are diagnostic for rejected alternatives; none is banked. All unaffected
sibling listings remain unchanged. Complete source and instruction diffs,
independent ELF extents, relocations and CFG clues are retained per control.

| Control | Body bytes before/after | Strict % before/after | First observed difference |
| --- | --- | --- | --- |
| GAME direct graphics owner | 3896 / 3900 | 100 / 99.536964 | Exit displacement `+0xf08` to `+0xf0c`; base/register/address changes follow. |
| OPEN TMD direct owner | 3320 / 3324 | 100 / 99.456630 | Exit displacement `+0xcc8` to `+0xccc`. |
| OPEN unlit direct owner | 676 / 692 | 100 / 97.597630 | Exit displacement `+0x274` to `+0x284`; separate owner address appears. |
| OPEN complete arena pointer | 532 / 532 | 100 / 99.902250 | Base addend eight becomes zero, with compensating field offsets. |
| GAME shared TMD body | 3896 / 3940 | 100 / 98.625260 | Frame 88 to 96 bytes; packet spills follow. |
| OPEN shared TMD body | 3320 / 3364 | 100 / 98.393974 | Frame 96 to 104 bytes; packet spills follow. |
| GAME packed sprite XY | 592 / 584 | 100 / 93.770270 | Exit displacement `+0x230` to `+0x228`; XY load/store schedule changes. |
| OPEN packed sprite XY | 540 / 532 | 100 / 93.170370 | Material address register `a1` becomes `a3`. |
| GAME damage caller widths | 6156 / 6156 | 99.827810 / 99.710850 | Caller `andi a0,s2,0xffff` becomes `move a0,s2`. |
| OPEN fade caller widths | 172 / 172 | 100 / 97.209305 | Caller `andi a0,s0,0xff` becomes `move a0,s0`. |
| GAME signed-halfword fade local | 6684 / 6684 | 99.967090 / 99.599045 | The first sign-extension sequence gains a saved value; branches and later shifts change. |
| GAME growth copy without typed source | 528 / 628 | 100 / 80.863640 | Source-address register changes; alignment branching and unaligned copying appear. |
| GAME single morph scratch view | 932 / 928 | 100 / 97.532190 | Initial record load selects `s3` instead of `s4`; later extent changes. |
| GAME common animation byte base | 932 / 944 | 100 / 97.403435 | Frame 72 to 80 bytes. |
| OPEN projected byte base | 3320 / 3320 | 100 / 99.506030 | First of 41 `addu` operand-order changes. |

The two already-partial functions are evaluated separately from the exact
controls. Damage cast deletion removes three retail argument masks. Fade
narrowing still exists when folded into the local declaration, but its emitted
signed tests and shifts fit retail less closely. Neither alternative supplies
a better supported representation, so both are parked. This is a final
source decision, not a claim that any partial score drop would be forbidden.

Known CFG successor lists are diagnostic, and unresolved switch transfers
remain unresolved. No compiler wall or optimizer mechanism is inferred from
these results. Earlier observations remain indexed in
[the reduction review](cast-reduction-review.md),
[projected addresses](typed-projected-addresses.md),
[GAME packet views](typed-storage-cast-campaign.md), and
[arena ownership](typed-memory-owner.md).

## Verification and retained evidence

The fresh target-C AST census covers all 112 source/image variants and every
project header. Written origins are deduplicated while retaining their
image/function contexts. Three scalar sites are enum macros; the other 539
casts are in C files. The final per-site ledger records **412 accepted, 130
parked and zero undecided** entries, with each original review ID, source,
types, family and final reason.

All 484 production function listings and strict scores remain unchanged,
including **453/471 exact game functions** and thirteen exact vendored
controls. All 112 objects retain their section bytes, extents and ordered
relocation targets. No new exact function is claimed or banked.

The repository suite passes **769 tests, with nine skips**. Ruff, all 112
cast-census parses, the tightened cleanliness gate and whitespace checks pass.
The fresh full baseline build and final full build retain the existing
failures: incomplete data ownership and relink/placement coverage, with
42/65 data-owning units matching and zero data artifact failures. No new
build failure is introduced and full-build success is not claimed.

Generated evidence is under the isolated worktree's `build/cast-floor-close/`:
`evidence/` holds image-qualified dossiers; `corpus-bounds.json` records the
complete resource census; `probes/` and `probe-verdicts.json` hold all fifteen
controls; `current-casts.json`, `current-sites.txt` and `site-verdicts.{tsv,json}`
cover every written cast; final function/object verdicts and build/test logs
record verification. Generated files and retail resources are not committed.
