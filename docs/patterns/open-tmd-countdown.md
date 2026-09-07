# OPEN TMD preparation countdown

## Exact packet-header aggregate closure

OPEN `tmd_prepare_primitive_indices` at `80017030` is now strict
**100.000000000%** under `probe-gcc257-o2-g0`. The retained source models the
real four-byte primitive header as `KfTmdPacketHeader`, with a packed-word view
for mode extraction and a byte view for the input-length field. A by-value
header local preserves that authored relationship:

- `header.word = *(u32 *)packet` emits retail's full-word load;
- `header.bytes.input_length` emits retail's independent byte load;
- shifting `header.word` emits the existing mode extraction unchanged;
- the optimized aggregate home accounts for retail's otherwise empty
  eight-byte leaf frame.

All 768 bytes now agree, including the entry stack decrement, return delay-slot
restore, 18 known CFG blocks, five branches, four address pairs, seven internal
jumps, 29-row switch, and all index loads/stores. The type expresses actual TMD
packet data; no unused local, padding, volatile carrier, assembly, or compiler
change is involved. The other thirteen functions in `open.render` remain
instruction-identical.

## Related-address and debug-lifetime controls

The exact `tmd_select` sibling exposes the old-GCC mechanism behind its empty
eight-byte frame: after CSE and combine fold the indexed source address into a
load, one related-address pseudo remains in global allocation without a hard
register disposition and receives an unused stack slot. The preparation
function has no equivalent unallocated pseudo under the retained source.

Natural packet-format expressions do not transfer that effect. Spelling the
header word as `*(u32 *)(body - 4)`, spelling `ilen` as `body[-3]`, stepping
from a typed TMD header to the object table, and omitting explicit `!= 0` from
both post-decrement loop conditions are each byte-identical to the canonical
candidate and retain the two-word frame residue. Reusing the packet cursor for
the initial asset load removes the independently observed asset reload and
rotates the packet/count registers. Chaining the two outer counters adds eight
real instructions and changes their register allocation. Those forms are
removed.

Compiling the TU with GCC 2.5.7 `-O2 -mcpu=r2000 -g` also leaves the frame
absent. Debug-local branch labels create relocation-identity differences in
six exact sibling functions, so this is not a viable TU profile and is
removed. Neither source-relative address syntax nor debug pseudo lifetime
explains retail's empty frame.

## Equivalent decrement-expression controls (`82320b9` follow-up)

Writing the proven outer countdown as two statements (`left = count; left--`)
changes the live halfword and object-pointer registers and inserts eight bytes
of real instructions; it does not create retail's leaf frame. Explicitly narrowing
`count - 1` to `u16`, and spelling the same wrap as `count + 0xffff`, both emit
the canonical body byte for byte and retain only the two missing frame words.
The direct subtraction is restored. Equivalent arithmetic syntax is not the
unattributed stale pseudo that owns this frame.

A plain `int` outer countdown with an explicit `u16` observation retains the
guard but emits `andi t0` directly and `addiu -1`, losing retail's promoted
copy and shared `0xffff` decrement. A genuine local `KfTmdHeader *` for the
first count read lets CSE reuse the selected-asset load and removes four bytes;
retail independently reloads that global before forming the object table.
Both are reverted. Neither known leaf-frame witness transfers without changing
substantive instructions.

## Scheduler-profile control (`d8f448e`)

The existing `probe-gcc257-o2-plain` profile does not recover the unused
eight-byte leaf frame. It instead changes the switch-arm scheduling broadly,
reduces `tmd_prepare_primitive_indices` listing similarity to 15.9%, and
regresses seven of the unit's thirteen exact sibling controls. The configured
R3000 scheduling profile is retained. This rules out the repository's plain
GCC 2.5.7 profile as an explanation for the two remaining frame instructions;
it does not identify the historical compiler or justify source padding.

The repository's `probe-gcc260-o2-g0` profile is also negative. It still
emits `nop` at both frame positions, changes the switch CFG from 18 to 16
known blocks, and broadly reorders the otherwise exact case bodies. Only two
of the unit's fourteen function listings remain identical. The GCC 2.5.7
profile is restored; compiler-version substitution does not explain the
retail leaf frame.

## Function Match Plan (`8ac47fe`)

OPEN `tmd_prepare_primitive_indices`, `0x80017030`, owns 768 bytes and is
strict 98.333336% under `probe-gcc257-o2-g0`. The six semantic queries,
complete eight-mode CFG, sole caller `tmd_register`, preceding view-transform
body, source history, shared format widths, prior packet-header experiments
and vendor negative controls were inspected. The separately attributed Sony
TMD reader/provider is not this custom in-place conversion of indices to
eight-byte offsets. No direct calls or strings occur. Three address pairs
load the current asset; the fourth addresses the 29-row switch. Seven
validated internal jumps are distinct from its indirect dispatch and the
candidate table-pointer rows.

The no-argument caller stores the selected asset in its call delay slot.
Both on-disk word counts are explicitly narrowed to unsigned halfwords.
Packet length is loaded as a byte independently of the whole header word;
mode extraction shifts 24 and masks `0xfd`. Vertex and normal halfwords
shift by three and store with wrapping truncation. Every case body and
resolved referent agrees with current C. The first difference is retail's
unused eight-byte frame, which this experiment does not try to manufacture.

At `80017070`, the primitive zero guard owns `addu a1,a0,t1` in its delay
slot, with `t1=0xffff`: the countdown is computed on both outcomes. Current
C initializes and decrements it only inside the nonempty branch; the probe
puts packet-address addition in the branch slot and computes the countdown
afterward. Move the actual unsigned countdown initialization/decrement before
the zero guard, retaining the packet calculation and all loop/body code.
This follows the observed unconditional operation without fabricated locals,
frame padding, helper calls or compiler changes. Preserve all thirteen exact
siblings in `open.render`; change no GAME source or shared type.

## Retained result

Moving only the real countdown preparation before the guard recovers the
retail zero-test position and its countdown-add delay slot. Strict objdiff
rises to **99.375000%**, with both bodies 768 bytes. After resolving numeric
relocations, exactly two aligned instruction words differ:

| Offset | Retail | Probe |
| --- | --- | --- |
| `+0x8` | `addiu sp,sp,-8` | `nop` |
| `+0x2fc` | `addiu sp,sp,8` (return delay slot) | `nop` |

Every other instruction, all four address pairs, seven internal jump targets,
and the switch-table comparison agree. The thirteen sibling functions remain
strict 100%. The unaccessed frame has no independently established source
owner; no local or storage is added to create it. This result remains partial
and is not banked.

## Verification

The canonical focused match really rebuilt `open.render` and all OPEN units
touched by the reverted header trial. Camera step, FT4 and floor-item strict
scores are unchanged; no source edit from those trials remains. OPEN stays
95/108 exact, all 108 started, with size-weighted fuzzy progress 99.113%.
All 358 existing exact functions and 13 vendor-source controls are preserved.

The subsequent full `kf build` still fails only the known OPEN TMD-emitter
default-row addends (`+0xca0` versus `+0xc9c`), thirteen pre-existing GAME
RODATA comparisons, and four GAME historical-best deficits. OPEN preparation's
own switch/data comparison passes. Ruff, the existing 401 repository tests
(17.279 seconds), and `git diff --check` pass; no new test work was introduced.
Generated logs and objects are not committed. The accompanying emitter-note
correction records the actual configured `probe-gcc257-o2-g0` name; no profile
or build option was changed.
