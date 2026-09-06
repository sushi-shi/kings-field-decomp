# GAME map-object forward collision probe

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x80030eb8 map_object_probe_forward`, **196 bytes / 49
words**, unit `game.map_object_pool`, initially **1.8163265%** strict objdiff.
This extends the related map-object clearing campaign on clean-base
`0ab6950` (272/362 GAME exact). Hash-identical retail was initialized and all
six semantic evidence views were read with `--image game`. The original
source in `7c7c843:src/game/map.c`, consolidation history, both neighbors and
the current comparison were inspected before editing.

- A 32-byte frame with saved `$ra`, 13 CFG blocks and one proven direct
  call, `collision_query_world` at `80030f04`. A validated HI16/LO16 pair
  `80030ecc/ed0` selects the shared definition owner; `lbu object+0` times
  eight selects its behavior byte. No strings or candidate outgoing refs.
- Five validated internal absolute jumps need `R_MIPS_26` even though they
  stay inside the function: `80030f0c`, `80030f38`, `80030f54` to the return
  join (+`0xb4`), and `80030f5c`, `80030f64` to the call block (+`0x3c`).
  Together with the two address entries and one call, there are eight ordered
  relocations. No indirect control transfers occur in the target.
- Word X/Z coordinates are read at object+8/+16. Yaw is masked by `0xfff`,
  then narrowed to `u16` in the type-0 dispatch. Type **2** calls with the
  unchanged point. Type **0** adjusts +X, +Z, -X or -Z by 2000 for yaw
  `0`, `0x400`, `0x800`, `0xc00`, respectively, then reaches the same call.
  The call passes point-Y sentinel `0xffff` in its delay slot, radius 3000
  through `$a3`, height zero at caller stack+16, and flags `0x21` at +20.
  The sole callee's complete retail body and shared signature were reviewed.
- The type-2 call block is physically before the cardinal-yaw dispatch.
  The successful result is moved to a common result register in the jump
  delay slot. All exits share `lw ra; move v0,result; jr ra; addiu sp,32`.
  Unsupported types and non-cardinal type-0 yaws never initialize that result
  register. The existing C already leaves those paths indeterminate; do not
  manufacture a fallback value or imply their C behavior is defined.
- Both proven call sites are in `map_object_pool_update`: `80031e38`
  passes `lh rotation.y - 1024` (subtraction in call delay slot), while
  `80031f38` passes `lh rotation.y` and the object pointer in the delay slot.
  Each compares the word result with -1. Source action-0/action-2 branches
  and surrounding timer guards were inspected. These are the intended
  cardinal/type-2 uses; no universal guarantee for arbitrary malformed
  placements or unsupported input is claimed. The caller's unresolved
  switch-table control is not promoted by this direct-call audit.
- Neighbors: complete retail `map_object_mark_collision_edge` before it,
  and the reset loop after it (whose unsuccessful fixed-size memset trial
  was reverted). No neighbor signature or data owner is changed.
- Vendor negative control: absent from supplied SDK/FID inventories; this
  is custom map-object metadata/cardinal-coordinate policy and calls a game
  collision helper, not a libc/GTE body. No vendored code is reconstructed.

The original early-rejection C puts the cardinal dispatch before the call,
inverts the type-0 branch and copies the uninitialized result into several
early exits. The first coordinate register difference follows that changed
CFG; do not attribute it to a register allocator limit.

First hypothesis: an outer behavior `switch` with type **2** written first
owns the shared collision block. Type **0** has the four cardinal cases,
each adjusting one coordinate then jumping to that shared block. Both the
successful call and unsupported exits reach one final return. Keep all
initializers, types, predicates, constants, referents and actual call set.
Require strict 100%, all 49 encoded words and all eight ordered relocations
before claiming closure. A matching source shape does not prove unique
historical syntax or compiler attribution.

## Focused control-structure results

The outer switch plus shared probe label restores the retail block layout
and common result return. The remaining raw differences are the X/result
register roles and radius setup: the current C keeps X in `$a3`, forcing
the 3000 radius load into the call delay slot; retail keeps X in `$t0`
and sets `$a3` before the common call or cardinal dispatch. This is not
grounds for forced registers or a synthetic radius local.

The second bounded source hypothesis expresses the two genuine operations
separately: type 2 queries its current point; type 0 adjusts the cardinal
point and then queries it. Keep one final return and a default exit for
unsupported yaws. This replaces the four explicit jumps to a cross-case
label with ordinary switch breaks and two mutually exclusive call sites;
the compiler may share their identical call tail. The dynamic call set and
all argument values stay the same. Compare the emitted call topology and
first remaining divergence; do not retain additional compiled calls.

That trial merged the calls at the end, moving the call block away from
retail again (focused raw similarity 38.3%, versus 82.5% for the shared
label). It was reverted. The shared-block source is kept, and canonical
strict objdiff confirms **93.755104%**, up from **1.8163265%**. Both bodies
remain 49 words, with the same eight ordered relocation kinds and referents;
internal `.text` addends still require comparison relative to each function.

The first raw difference is the X load into `$a3` rather than `$t0` at
function+`0xc`. Its consequences remain visible in call setup, the result
register, and the cardinal-dispatch entry (+`0x60` versus retail +`0x5c`).
Retail's radius load occupies the type-2 guard's delay slot and is repeated
in the first cardinal comparison's delay slot. The reconstruction loads it
in the call delay slot, placing the Y sentinel before the call instead.
No independent evidence currently justifies forced register assignments or
artificial locals. This is a substantial CFG recovery, **not exact**, and
is not banked. No historical compiler cause is assigned to the residue.

The unit stays 4/8 exact; the reverted pool-reset trial remains 78.25%.
Full-build, regression, lint and test results are recorded with the
[map-object clearing campaign](game-map-object-clearing.md).

## Function Match Plan at `3b1c834`

Resume on clean `master` after map-copy closure; this unit is now 7/8 exact.
The forward probe remains **93.755104%**, 196 bytes. The pinned profile is
still `probe-gcc257-o2-g0`, with no historical attribution claim. Retail was
hash-validated again. All six semantic views, both external caller windows,
the complete world-collision callee, both neighboring bodies, source history,
shared headers and the eight ordered relocation entries were re-read. A fresh
focused compile reproduces the prior difference at +0xc and all seven exact
siblings. No references, signatures, object fields, data or claims change.

The radius literal occurs twice in retail: once in the type-2 guard delay
slot (`80030ef0`) and once in the first cardinal compare delay slot
(`80030f20`). Both paths reach the single call at `80030f04`, whose delay
slot loads Y=65535. Our explicitly shared source call instead loads radius
at the call and Y beforehand. Test an alternative source-level sharing
boundary: one ordinary query expression in each of the four cardinal cases,
with the +/-2000 adjustment directly in its X or Z argument, plus the type-2
unchanged-point query. This differs from the earlier two-call trial, which
adjusted coordinates in the inner switch and called once after that switch.

Each admitted input still performs exactly one world query, and unsupported
type/yaw paths still do not initialize the return local. Preserve this
existing caller-domain limitation rather than inventing a fallback value.
The two runtime callers consume only the query result's equality with -1.
The function remains game object/cardinal policy, absent from the vendored
inventory; the world query is a separate game ABI, not a new SDK body.

Check whether normal compilation shares these mutually exclusive calls into
retail's single static call and early radius setup. Reject added machine call
sites or a wrong shared-return CFG. No explicit radius local, forced register,
inline assembly, dummy statement, new compiler flag or inventory workaround
is allowed. Verify all 49 raw words and eight physical relocation entries,
the complete 30-byte descriptor owner and 84 switch rows, and all 484 function
scores before banking any new exact result. Run the required full build,
lint, tests and diff checks before commit.

The direct-argument trial correctly loads X into t0 but produces a 268-byte
body. It merges the actual JAL into one site at +0xf0 while retaining five
stack-argument setup blocks and placing the call after the cardinal cases.
The first difference is the type-0 destination (+0x50 rather than +0x5c),
and the radius still occupies the call delay slot. Reject that form.

Retail's X updates write the captured X register itself, then its common
call block copies X into a0. The direct-argument trial instead computes
adjusted X directly in a0. Test separate coordinate updates followed by the
same query in each admitted arm; this preserves the observed X/Z dataflow
while still testing source call sharing. All constants and return guards
remain unchanged.

The separate-update/per-case-call trial emits 272 bytes with the same wrong
late call/duplicated stack setup. Its first difference is again X at +0xc,
now loaded into v1; Z also moves before the definition-byte lookup. Reject
both per-case-call forms and restore the original shared label.

Next test the lifetime of the one-use definition address. Retail captures
X before loading the behavior byte, then loads Z into the byte-load delay
slot; no definition pointer survives the byte read. Keep captured X/Z and
the shared query, but place the direct shared-owner behavior expression in
the outer switch rather than defining a pointer before either coordinate
capture. This preserves the same complete definition type and field, and
does not introduce a synthetic value solely to occupy a register.

The direct owner lookup reproduces the original 196-byte non-exact object;
it is not a discriminator. Restore the pointer and test the outer dispatch
as its directly decoded predicate chain: if behavior is nonzero, query only
when it equals two; otherwise perform the cardinal switch. Retain the shared
query label inside the type-2 block. Retail's BEQZ then BNE implement exactly
these two guards; unsupported types still reach the common uninitialized
return. This tests source control structure, not arbitrary register steering.

The explicit predicate chain also reproduces the baseline instruction
sequence. Restore the bank-safe original switch source; all four bounded
trials are rejected. The unchanged forward-probe residue is still
unattributed, not a proved compiler limitation. These controls rule out the
tested per-case-call, one-use-pointer and outer-predicate spellings as the
missing source fact; no new exact result or source change is claimed here.

The final all-484-row comparison confirms forward probe is unchanged at
**93.755104%**, and its source file has no diff. The sole score movement
is the independently closed effect scatter helper. Required full-build,
lint, 649-test and raw verification results are recorded with the
[scatter closure](game-effect-scatter.md#strict-and-raw-verification).
