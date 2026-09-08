# Psy-Q CPE residue at initialized-data boundaries

`GAME.EXE` contains these bytes at virtual address `0x80057e68` (file offset
`0x46668`):

```text
43 50 45 01 08 00 03 90 00 00 00 00
```

They decode as the standard Psy-Q CPE v1 prefix: `CPE\x01`, select unit zero,
then the start of a 32-bit register-value record. The controlled PSYLINK 1.17
probe in `tests/psylink_order_smoke.py` now checks that every produced CPE has
the same six-byte magic/select-unit prefix. These are conversion residue in
the final partially occupied PS-X EXE load page, not a meaningful source
initializer.

This matters because referenced storage can begin inside that padded load
page. `player_update` treats `0x80057e68` and `0x80057e70` as signed motion
limits, writes both on every ordinary-update path before reading either, and
is their only referencing function. The two four-byte objects are eight bytes
apart, matching the old GCC/maspsx common allocation observed by the build
pipeline. They are therefore modeled as separate BSS objects even though their
addresses fall below the PS-X EXE header's page-rounded load end. All of their
references are confined to `player_update`, and file-static tentative BSS words
receive the observed eight-byte alignment in the pinned GCC/maspsx pipeline.
The source therefore defines both limits privately in `player_update.c` rather
than leaking declarations through a shared header.

Do not classify a referenced address as initialized source data merely because
it falls inside the page-rounded PS-X EXE payload. At a suspected tail
boundary, inspect the raw bytes, prove write-before-read behavior and xref
ownership, and compare the spacing with the compiler's common allocation.

OPEN has the same twelve-byte CPE prefix at `0x800375d8` (file offset
`0x25dd8`), followed by page zeros through its header load end `0x80037800`.
The private controller words at `0x80037760` and `0x80037768` are inside that
page, not initialized source data. Their consuming PAD routine overwrites
them before the first branch/call, and the supplied Sony PAD.OBJ records their
`.sbss` allocation separately from exported `PadIdentifier` in external BSS.
The [PAD review](../../config/evidence/pad_storage_and_linkage.md) corrects the
earlier zero-initializer and private-identifier assumptions. It does not promote
every zero in the page to a known allocation or silently flatten the two BSS
classes into a placeable section.

The [CD/resource storage review](../../config/evidence/cd_resource_bss.md)
provides another boundary control: OPEN's CdlLOC at `0x800375d8` overlaps the
four-byte CPE magic itself, but both CD helpers overwrite its three command
bytes before use. Retail cd_setloc copies only those three bytes, and CD_cw's
command-count table independently limits CdlSetloc to three. GAME's homolog
at `0x80057e80` has the same write-before-use behavior, including its separate
error-screen consumer. The real four-byte SDK type is retained, without an
explicit initializer. OPEN's nearby arena pointers are saved before their
scene/ending consumers restore them. This proves only the reviewed objects;
it does not classify every zero or gap following the CPE prefix.

## PSX and converter provenance

PSX has the identical twelve-byte prefix at `0x80010230`, file offset `0xa30`,
immediately after the pointer table and SDK `_stacksize` word. Supplied
`LIBSN.LIB/SNMAIN.OBJ` allocates four uninitialized `.sbss` bytes there. The
retail startup zeros `[80010230, 80010234)` before using that slot to save
`$ra`; the bytes are not an initialized SDK datum. The following four bytes
are still an address-only census hypothesis, not a proved source allocation.

The [PSX layout experiment](../executable-linking.md#psx-difference-investigation)
matches all code and initialized data with complete SDK objects and a scratch
source alignment change. Its seven remaining differing bytes are exactly the
nonzero bytes of this prefix; all following page bytes agree as zeros.

The supplied native `CPE2X.EXE` identifies itself as **CPE2X 1.3**. A control
encodes the diagnostic linked `.rodata`, `.text` and `.data` as CPE v1 load
records, with select-unit-zero and a 32-bit PC record for `80010100`, and runs
that executable under the pinned DOSBox-X. It creates a 4096-byte PS-X EXE with
the correct entry and load range, but emits **zeros** at the tail. Its default
header also differs from retail in 59 bytes. Consequently the supplied
converter does not reproduce the retail tail under this control. The shared
CPE signature supports container residue; the exact historical converter,
record ordering/options and mechanism remain unproved. Do not copy the prefix
into generated output and call the result independently reproduced.

The later [PSX executable closure](psx-exact-link.md) also tests the distinct
Runtime 2.6 CPE2X 1.3 binary; it emits zeros too. The normal packer now has an
explicit compatibility model that encodes the observed CPE prefix at the linked
load end and zero-fills the remaining sector. Its report labels the mechanism
as inferred and keeps historical-converter reproduction false. This gives
complete-file equality for PSX while preserving the BSS model and the native
negative controls; it does not independently prove the historical mechanism.
