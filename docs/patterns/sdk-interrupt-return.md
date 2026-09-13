# SDK interrupt return and modal screens

The generated C game could stop accepting input after its starting-door plaque
remained open for several seconds. Sound voices continued looping. Retail did
not show the failure. The user also reproduced it on the previously published
classic snapshot, before the original GCC varargs-header change.

The captured candidate was executing the exact rebuilt GAME text. Its plaque
function, `screen_show_image_until_input` (retail GAME `0x8002c794`), is a banked
100% reconstruction. Debugger breakpoints showed that it still cleared and drew
its ordering table and reached `PadRead`; `PadRead` returned zero after a real
button press. Initial samples in `intDMA` and `_otc` were not deadlocks: both
returned. No game source change is justified by those samples.

The decisive observation was that the SDK `Vcount` continued advancing while
the game's BIOS-event-driven `frame_pacer_vsync_count` stayed at `0x2cff`.
Input, sequence playback and the BIOS VBlank event stopped updating together.

The pinned Release 2.5 `LIBETC.LIB/INTR.OBJ` dispatches IRQ slots, then tests
`I_STAT & I_MASK` and repeats the entire pass before returning to the BIOS.
Under continuous GPU DMA, VBlank can arrive inside that dispatcher. Its next
pass consumes and acknowledges VBlank before BIOS controller, audio and root
counter handlers can see it. Retail GAME's interrupt dispatcher (`0x8004fd30`)
returns after its peripheral pass; it has no corresponding outer retry branch.
This is a demonstrated SDK revision difference, not a game reconstruction fix.

A debugger-only experiment replaced the candidate's final backward `bnez`
with `nop`. From the same saved failure, the BIOS frame counter advanced from
`0x2cff` to `0x2d6d` in two seconds. The user confirmed that normal audio and
plaque dismissal worked again.

`scripts/psxbuild/sdk_compat.py` applies that one-instruction correction to the
build-directory copy of the SDK archive before PSYLINK reads it. It requires
the complete pinned archive's SHA-256 and checks the original branch bytes.
Archive offset `0x8b5` is INTR's `intInit + 0x190`; instruction `0x1440ffcc`
becomes zero. Its delay slot was already a `nop`. The object size, relocation
records, symbols and all other instructions remain intact. Pending interrupts
return through the BIOS normally instead of being consumed by another SDK pass.
The Nix SDK package and matching reference objects remain original.

The [function-level scope](../sdk-object-audit.md#interrupt-workaround-scope)
records the size of a potential retail-object replacement: INTR contains 18
inventoried routines in each overlay, with one existing exact source reference
and a separate SDK function match for `stopInit`. The full image/address list
is maintained in [functions_vendored.tsv](../../config/retail/functions_vendored.tsv);
filter `library = LIBETC.LIB` and `module = INTR`. The behavioral difference is
within `intInit`; matching the complete containing object is a broader campaign.

Every executable build report records the original archive hash, actual linker
input hash, member, offset, before/after bytes and reason. The shared native
builder applies the correction to GAME and OPEN for master, classic and source.
PSX does not link LIBETC. Generated branches contain the correction as ordinary
build support, without matching tests or inventories.

`tests/test_sdk_interrupts.py` builds a native control with the real archive,
loads its CPE into a MIPS interpreter, and introduces a VBlank after its slot
was checked. The original branch consumes that interrupt; the corrected
dispatcher returns with it pending for the BIOS. A no-pending-interrupt control
also returns normally. Unknown SDK archives are rejected.
