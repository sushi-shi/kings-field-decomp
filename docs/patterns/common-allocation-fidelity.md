# COMMON cannot disappear from data matching

## Campaign boundary

Baseline `d9b6b60`: 386/471 exact game functions and thirteen exact vendor
controls. Strict source data is 5/60; four independent SDK contributions match.
Target relinking passes 110/116, with six conflicting section bases. This
campaign changes no game/vendor C, claim, relocation, identity or probe flags.

Plan: reproduce an allocation omission with genuine GNU MIPS objects, require
the shared placement contract to reject it on both comparison sides, and
calibrate original PSYLINK reservations separately from ordinary section
contributions. Preserve every function score and input object's bytes. Do not
infer successful whole-program placement from a section-only comparison.

## Reproduced false pass

`tests/test_common_allocation.py` assembles a synthetic eight-byte `jr ra; nop`
control and the initialized word `0x12345678` using the pinned GNU assembler.
Adding `.comm unclaimed,8,8` leaves those twelve bytes unchanged but adds an
eight-byte allocation request. Its ELF symbol has `SHN_COMMON`, size 8 and
alignment 8; it has no input section and no relocation referring to it.

Before the correction, `roundtrip.plan` reported no issue. The generated GNU
linker script selected the normal sections and discarded everything else with
`/DISCARD/ : { *(*) }`. The real linker succeeded, retained all twelve initialized
bytes and silently removed the COMMON symbol and its storage. `verify_unit`
then reported success. The strict data gate also passed matching `.data`, or
returned no data result when the control had no initialized DATA claim.

This is not hypothetical unsupported syntax: the production adapter already
has an optional `--use-comm-section` path. Current default objects materialize
COMMON into `.bss`, so this hole did not change their present scores. It would
have made a future allocation-model migration falsely appear successful.

## Shared correction and controls

`roundtrip.plan` now scans every symbol for `SHN_COMMON` before examining claims
and allocated sections. It reports `unsupported-common-allocation` with the
symbol, requested size and alignment. Unnamed and zero-sized requests are not
silently exempted. A DATA claim does not turn an unallocated symbol into a
placed BSS object. The verifier rejects the input before launching GNU ld.

The data matcher already uses this same placement contract for both objects.
Its default gate therefore rejects source-only, target-only and identical
COMMON requests, including a unit with no DATA/RODATA claim. Equal requests
are not evidence of their final allocation addresses. No separate relaxed
comparison, guessed `.bss` base, address override or synthetic padding is added.

Five test methods include all those cases, clean positive controls and a
genuine GNU-as/GNU-ld reproduction. The latter retains an explicit old-script
control: the linker still demonstrably drops the unreferenced storage, while
the corrected verifier must refuse to run it. Before the production edit,
the suite reports twelve failing subcases. After it, these and the existing
data/relink controls pass.

## Original PSYLINK allocation calibration

`tests/psylink_bss_smoke.py` uses the supplied PSYLINK 1.17, SHA-256
`b98e7180fc33b4c94adf41dcf9cc73f469864d23aabb3f3fcedf5e9f2c1ca040`.
It constructs data-only LNK v2/processor-7 objects and independently decodes
each with `psyk list --code`. The records distinguish:

- a three-byte initialized `.data` contribution;
- record-8 fixed reservations in `.sbss` and `.bss`, with XDEF start/end labels;
- record-48 exported XBSS requests, with sizes but no fixed section offsets.

Eighteen cases check every label and the complete CPE stream: prefix, load
record address/extent/payload, record order and EOF. Expected offsets and
allocated sizes are explicit observed tables, not values produced by a new
allocator or inferred from retail addresses. No game code executes.

All offsets and range endpoints below are hexadecimal, relative to
`/o$80010000`; requested sizes and byte counts are decimal:

| Control | Observed placement |
| --- | --- |
| One input; 16 fixed small bytes; XBSS `one:8`, `two:4` | `.data` 0..3, `.sbss` 4..14, XBSS 14..1c and 1c..20 |
| Reverse those XBSS records or their numeric IDs | Same allocation addresses |
| Add 8 fixed `.bss` bytes | Fixed 14..1c; XBSS starts at 1c and 24 |
| Two inputs with 16/24 small bytes | Data at 0/3, small at 8/18, XBSS at 30/38 |
| Reverse those inputs | Their data/small contributions reverse; named XBSS remains at 30/38 |
| Duplicate `shared` requests of 4 and 16 bytes | One 16-byte allocation at 34, after `one` at 28 and `two` at 30; same result with input order reversed |
| Requests of 1, 3 and 9 bytes | Reservations are 4, 4 and 12 bytes |
| `.bss` raw tag 16 for those requests | Section begins at 10, but internal requests start at 10/14/18, not at 16-byte increments |
| Names `ZZZ:8`, `AAA:4`, in either record order | `AAA` at 14, `ZZZ` at 18 |
| Names `AB:8`, `BA:4` | `BA` at 14, `AB` at 18; reversing the records changes these to `AB` at 14, `BA` at 1c |
| 3 fixed `.bss` bytes, then requests of 3 and 5 | Fixed 10..13; XBSS at 13/17 with extents 4/8, without start realignment |
| Raw tag 2, one fixed byte, then requests 1/3/9 | Fixed d..e; XBSS at e/12/16 with extents 4/4/12 |
| Two fixed `.bss` contributions of 5 and 3 bytes | Contributions 24..29 and 2c..2f; XBSS begins at 2f, after both inputs |

The distinction is not simply “align every object to four bytes.” These inputs
show request-size rounding, input-section alignment, fixed reservations,
cross-input coalescing and name-sensitive ordering as different operations.
The `AB`/`BA` control does **not** prove a general symbol-hash algorithm.

Default PSYLINK emits zero-filled CPE load records for the fixed reservations
and XBSS in these synthetic inputs. Alignment holes have no load records.
Records for ordinary contributions follow input traversal rather than sorted
addresses; the XBSS records follow them. Thus a CPE zero record alone does not
prove an explicit C initializer. These default synthetic-link results do not
prove the game's original linker options or CPE conversion policy.

## Consequences for remaining ownership work

The PAD archive's private `.sbss` reservation and exported `PadIdentifier`
XBSS request are distinct records, as documented in
[the PAD evidence](../../config/evidence/pad_storage_and_linkage.md).
The archive's raw alignment tag 8 means four-byte input alignment under the
separate [tag calibration](psyq-section-alignment.md), not ELF alignment 8.
Its member remains revision-skewed relative to retail; it is not promoted to
an exact provider merely because the allocation records are understood.

Do not flatten those classes into one contiguous `.bss`, replace a four-byte
C integer with its rounded reservation size, or place exported symbols at
desired retail addresses independently of the allocation model. Also do not
enable G8 merely to produce `.sbss`: the adapter changes instruction selection.

The next required contract must preserve reservation class/size separately
from C object size, retain private contribution order and exported coalescing,
and establish whole-image allocation placement from independent evidence.
The measured cases constrain that work but do not establish the full original
allocator or assembler. Until it exists, COMMON must remain a visible failure,
not omitted storage. Existing PAD placement conflicts remain unresolved.

## Follow-up: the native symbol hash

The OPEN twenty-owner campaign added independent symbol-name controls and
inspected the same hash-pinned PSYLINK executable. Its MZ header is 512 bytes;
the offsets below refer to the executable file, not a game-image address.

At file `0x1451`, the symbol lookup routine saves the length-prefixed name
pointer, reads the length byte, clears AH and initializes both CX and DX with
that length. With case sensitivity disabled, `0x1464..0x146c` translates each
character through a case-fold table, adds the byte to DX and replaces it in
the name. With case sensitivity enabled, `0x1470..0x1473` adds the original
bytes. At `0x1478` it masks DX with `0x1ff`, doubles the index, and reads the
bucket head. Thus its hash includes the length byte:

```text
bucket = (name_length + sum(normalized_name_bytes)) & 511
```

The routine compares the stored length and all name bytes within a collision
chain. At `0x14bc..0x14ea`, insertion calls that lookup; a new symbol becomes
the bucket head and links to the previous head. An existing symbol is reused.
The exported-allocation pass at file `0x189b..0x19c1` traverses all 512 buckets
in increasing order and follows each symbol's next link. Entries with flag
`0x40` allocate their requested extent, advance the section contribution,
receive the previous section offset and become ordinary defined symbols.
The allocation loop uses the symbol table order, not C declaration order.

Four additional full-CPE cases in `tests/psylink_bss_smoke.py` preserve
observed placement tables independently of this formula. Nineteen varied
names are checked with and without `/c`; 179 longer names cross hash wraps.
Their complete CPE records, labels, request extents and EOF are verified.
The fourth case introduces an XREF to `BA` before the unchanged `AB`/`BA`
XBSS declarations. It changes their allocated order: the earlier reference
creates the symbol first, and its later allocation declaration reuses that
entry. All 22 cases pass, including the original private-reservation,
cross-input coalescing and collision controls.

A simpler sum-only hypothesis was falsified by `PadIdentifier`: in the
case-sensitive control it precedes `abc`, although its character sum is
larger. Including the length predicts the observed order and agrees with
the decoded routine. A separate earlier `and bx,0x1ff` occurrence belongs
to sector-size arithmetic, not the symbol hash; it is not evidence for the
allocator.

This does not establish original game symbol spellings. The current source
identities are curated names; changing them changes native placement. A
future COMMON contract must preserve all requests and actual symbol-interning
order and test the complete allocation against retail evidence. These
controls do not authorize scattering symbols to their desired addresses or
counting equal unallocated requests as a completed data-owner match.

## Reproduction

In `nix develop`, set `PSYLINK_DOSBOX` to the flake's DOSBox-X and run:

```sh
python3 -m unittest tests.test_common_allocation tests.test_roundtrip tests.test_data_match -v
python3 tests/psylink_bss_smoke.py
kf build
nix flake check -L
```

The flake runs the new allocation control alongside the existing original-linker
order and alignment controls. Synthetic OBJ/CPE/SYM files and generated reports
remain uncommitted.

## Verification

All 629 local tests pass without skips, along with Ruff and diff checks. The
flake passes its 629-test suite with 123 expected local-artifact skips, the
native compiler/objdiff controls, existing PSYLINK order/alignment checks and
all eighteen new full-CPE allocation controls.

The full build freshly recompiles all 112 source units. All 224 source-owned
source/target object SHA-256 values and every one of 484 function-score rows
remain unchanged, preserving 386/471 exact game functions and thirteen vendor
controls. No function is newly banked, no DATA claim is changed, and no new
whole-data match is claimed.

Strict source data remains 5/60 (GAME 3/40, OPEN 2/19, PSX 0/1); independent
SDK data remains 4/4. Target relinking remains 110/116 with the same six
conflicting bases. Reachability still includes 614 unmatched config-only
ranges across GAME and OPEN. The full default build remains red on those
data/ownership/placement failures. Exhaustive reachable-byte accounting and
linked-executable equality are not yet proved.
