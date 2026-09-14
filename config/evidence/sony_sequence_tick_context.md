# Sony sequence callback: saved RAM and jump context

## Campaign plan and function snapshots

The four unmodelled GAME data-reference witnesses at the `8ad1fe4` checkpoint
all belong to the vendored sequence callback. Recover its complete context and
save-area extents from the retail consumers and authentic SDK ABI; inspect the
OPEN twin for missing address pairs. Do not reconstruct SDK source, fabricate
a stack allocation from an address gap, or count inventory ownership as a match.

| Image / function | VA / extent | Starting state and evidence |
| --- | --- | --- |
| GAME SsSeqCalledTbyT | `8004a55c / 2f8` | Vendor lineage, no source unit or objdiff function score. `SsStart` takes its address. Saves eight low-RAM bytes, calls setjmp, changes its saved SP, dispatches sequence tracks, restores RAM and SP, then longjmps back. |
| OPEN SsSeqCalledTbyT | `8002a330 / 2f8` | Same 190 instruction shapes and complete dispatch call topology. Context-root and alternate-SP pairs were absent; eight other relevant pairs were candidates. |
| GAME memcpy / OPEN memcpy | `8004a52c / 30`; `8002a300 / 30` | Adjacent vendor byte-copy helper; callback passes size eight in both call delay slots. Not game progress. |
| GAME setjmp / longjmp | `8005033c / c`; `800503dc / c` | LIBAPI BIOS thunks through `0xa0`, selectors `0x13` and `0x14` in the jump delay slot. No local BIOS implementation is present. |
| OPEN setjmp / longjmp | `800300a0 / c`; `80030120 / c` | Same complete three-word thunk bodies, not proof of BIOS reachability closure. |

The callback evidence pass includes image-qualified address/extent, complete
disassembly/CFG, incoming/outgoing references, strings (none) and match state.
Its frame is 56 bytes; the final stack restoration belongs to the `jr ra` delay
slot. The next function is Snd_decrescendo in both images. Existing
`overlay_lineage.tsv` and `functions_vendored.tsv` supply the archive export,
dispatch topology and cross-overlay attribution. No callback signature, call
set, control flow, code bytes or vendor classification is changed here.

## Data owners

These names describe observed roles, not recovered historical symbol spellings.
All eight identities retain unknown source linkage. `psyq_sscall` records the
consumer family, not a proved original data-section boundary.

| Identity | GAME VA | OPEN VA | Extent / storage / type |
| --- | --- | --- | --- |
| ss_tick_low_ram_pointer | `80057d08` | `80037478` | 4 bytes, load, `u8 *`, initialized word `0x00000100` |
| ss_tick_saved_low_ram | `80058010` | `80037750` | 8 bytes, BSS, `u8[8]` |
| ss_tick_saved_sp | `80058018` | `80037758` | 4 bytes, BSS, `u32` |
| ss_tick_context | `8005b270` | `80039340` | 48 bytes, BSS, authentic `jmp_buf` |

Pinned `SETJMP.H` defines `jmp_buf` as `int[JB_SIZE]`, `JB_SIZE = 12`, and
`JB_SP = 1`. The 32-bit target ABI therefore requires 48 bytes and places SP
at byte offset four. The former `DAT_8005b274` and `DAT_80039344` identities
were interior words, not independent globals, and are replaced by the roots.

Relative to each callback start, the retail sequence is:

- `+2c`: load the low-RAM pointer; `+34`: address the eight-byte save area.
  `+3c`: copy call, with `a2 = 8` in the `+40` delay slot.
- `+44`: address the context root; `+4c`: setjmp call. Its return dispatch
  distinguishes zero, one and two. Zero takes the stack-switch path.
- `+90`: address context +4; `+98`: read the saved SP; `+9c`: address the
  alternate stack pointer; `+a4`: install it into context +4. `+a8` subtracts
  four to recover the context root, `+ac/+b0` saves the original SP separately,
  and `+b4` calls longjmp with `a1 = 1` in its delay slot.
- After sequence dispatch, `+2a0/+2a8` reverse the copy arguments. `+2b0`
  calls the helper with size eight in its delay slot. `+2b8` addresses context
  +4, `+2c0` loads the original SP, and `+2c8` recovers the context root.
  `+2d0` calls longjmp with argument two; its delay slot restores the SP word.

This is saved stack state, not an abort-handler code pointer. The byte meaning
of the pointed-to low RAM is not established here; do not label it an interrupt
vector from the address alone. OPEN's save area and SP lie in the page-rounded,
zero-filled executable tail; their GAME twins are outside the load image, and
the copy/setjmp/save operations establish write-before-read use. `data.tsv`
retains all 16 bytes from `8003774c` through `8003775b`: four unresolved bytes,
eight saved bytes and four SP bytes. Zero-filled load padding is not a C
initializer for those BSS objects.

## Relocations and remaining uncertainty

Twenty HI16/LO16 pairs, ten per image, now carry reviewed raw-byte evidence.
OPEN adds the absent `8002a374/8002a378` context pair and
`8002a3cc/8002a3d0` alternate-stack pair. Context +4 references use the complete
owner plus addend four. Every pair is checked through the shared safe delinker
and restored to its original opcode, registers and absolute retail target.

The alternate-SP values are GAME `80063278` and OPEN `80041348`. They do not
prove the allocation's start, size, lifetime or original linkage. Both remain
address-derived references without invented data extents, and consequently
remain explicit ownership failures. In particular, the gap after jmp_buf does
not justify declaring a 32-KiB stack. Indirect BIOS control flow and memory
behind the `0x100` pointer also remain outside this campaign's completeness
proof.

The pinned Release 2.5 `LIBSND.LIB/SSCALL.OBJ` has SHA-256
`7dd7838ada671a3901ebbf8766f484d43d1a5204d274a5e6b11d9dd727e63d9c`.
Its exported callback and complete dispatch XREF family support the existing
vendor lineage, but its text is 580 bytes versus retail's 760. It has busy/error
guards rather than this setjmp/longjmp wrapper; track stride is 168 versus
retail's 172, and the status word is at +144 rather than +0. It cannot establish
these private data symbols, source linkage, alternate-stack extent or an exact
provider match. The SDK archive is a revision negative control, not a replacement
for the retail body.

Seven unused vendor-only declarations are removed from `game/state.h`, including
the incorrect handler descriptions. None has a reconstructed C consumer, so no
replacement game-header extern is warranted. No source DATA claims or SDK data
contributions are fabricated for these eight config-owned objects.

## Verification boundary

`tests/test_sequence_tick_context.py` checks the SDK header and archive,
non-overlapping complete owners, retained stack uncertainty, retail copy widths,
SP save/restore and delay slots, both BIOS thunks, census preservation, and all
twenty safe relocation round trips. These are static evidence controls, not
execution with invented BIOS semantics and not a claim of whole-object matching.

The fresh audit reduces GAME unmodelled-target witnesses from four to one;
OPEN goes from zero to one because its missing alternate-stack pair is now
visible. Eight OPEN candidate paths become validated references, not proven
control flow. Reached config ranges lacking independent comparisons increase
from 624 to 625 (GAME 358, OPEN 267): identifying a previously missing owner
does not make it matched. Source-owned counts remain GAME 79 and OPEN 55.

All 112 source objects and all 484 function-score rows are unchanged against the
pre-campaign snapshot. The 360/471 exact game functions and 13 exact vendor
controls remain intact. All 572 local tests pass without skips, as do Ruff and
diff checks. The required full build reruns delinking and retains its explicit
strict-data, ownership and placement failures: source data 6/59, SDK data 2/2,
target relinking 108/114. Nothing is banked or reported as a new data match.
Source/header literal DAT_ occurrences fall 177 to 170 by removing the unused,
incorrectly game-owned declarations, not by inventing semantic replacements.
