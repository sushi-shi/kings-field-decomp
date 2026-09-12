# SDK data ownership

This campaign starts from 1,036 unresolved ownership entries after the first
LIBSPU table cleanup. Ownership evidence is separate from recovering an original
private symbol spelling, exact historical SDK revision, or executable placement.
No SDK payload is supplied to the native build by this inventory work.

## Switch tables

| Role | Owner | GAME table / consumer | OPEN table / consumer | Entries | GAME bound / base pair |
| --- | --- | --- | --- | ---: | --- |
| CD BIOS dispatcher | LIBCD/BIOS | `80012f24` / `8003b5ac` | `800123f0` / `8001b3cc` | 5 | `8003b6a0` / `8003b6b4` |
| SetControlChange | LIBSND/SEQREAD | `80013bfc` / `80047db8` | `800130c8` / `80027b8c` | 122 | `80047e34` / `80047e48` |
| Snd_setVabAttr | LIBSND/SEQREAD | `80013de4` / `80049480` | `800132b0` / `80029254` | 20 | `800494e0` / `800494f8` |
| EigenMatrix | LIBGTE/GEO | `80013e34` / `8004f518` | `80013300` / `8002f2ec` | 9 | `8004f7c4` / `8004f7d8` |
| sprintf format dispatcher | LIBGPU/SPRINTF | `800141b4` / `80054d5c` | `80013680` / `80034a40` | 45 | `80054ffc` / `80055010` |

Every consumer loads the listed entry count minus one, bounds the selector with
an unsigned comparison, indexes by four, loads the pointer and jumps through
it. Each base pair consists of `lui` and the following `addiu`. All 402 entries
are aligned internal addresses in their respective function extents, and the
complete relative destination sequence agrees between the overlays. OPEN's
bound and base-pair sites have the same function-relative offsets as GAME's.
The sprintf selector subtracts 76; the CD dispatcher subtracts one.

`functions_vendored.tsv` supplies the archive/member attribution and distinguishes
exact Release 2.5 matches from supported lineage. The complete table identities
use descriptive role names with unknown original linkage. Pointer relocations
and their numeric targets are preserved.

Two GAME scan identities are also removed: `800122c4` and `800122ec`. Each is
an all-zero word comprising a string's terminating byte and three alignment
bytes. The terminating byte already belongs to `game.item`'s `0x25`-byte RODATA
claim or `game.menu_map_viewer`'s nine-byte claim. Neither word has an independent
reference or source object; no source claim is enlarged to consume padding.

## Evidence boundaries

The generated dossier under `build/remaining-data-ownership/` keeps the initial
1,036 entries and references, native SDK relocation observations, section-base
candidates, complete table destination sequences and pre-change target hashes.
These generated proposals never overwrite the curated inventories.

An exact instruction match with relocation fields masked does not prove that
the SDK's local data offsets or complete section layout survive unchanged.
For example, the Release 2.5 INTR stack allocation and the retail allocation
differ. A derived section base that extends into unrelated load data is rejected.
Conflicting offsets in BIOS, VMANAGER and LIBGPU/SYS likewise require individual
object evidence; a large candidate interval cannot establish their ownership.

## Complete SDK objects

`SSINIT.OBJ` declares a `0x800`-byte COMMON allocation for `MarkCallback`.
Retail SsInit clears 32 rows of 16 words at GAME `800653e8` and OPEN `800431d8`:
GAME's base pair is `8003e3ec/8003e3f0`, its inner bound is loaded at `8003e3f4`,
and its outer comparison is at `8003e410`. OPEN has the same sequence at a
`-0x201e0` displacement. Three scanned interior words per image therefore belong
to this one complete callback-pointer array. The two decoded base pairs were
missing from the relocation census and are added as reviewed address references.

The same member declares `_ss_score` as a `0x80`-byte COMMON allocation. Its
32 pointer slots start at GAME `800a06e0` and OPEN `800758a0`; SsSetTableSize and
the sequence readers use this base. Expand the existing base identity and
remove the separately seeded second slot.

`S_INI.OBJ` owns `_spu_rev_attr`, bound at GAME `8009fee8` and OPEN `800757b8`.
The authentic `LIBSPU.H` type `SpuReverbAttr` has a mask, mode, signed left/right
depth halfwords, delay and feedback. These fields explain all five interior
identities per overlay. The C object is 20 bytes; the archive's 24-byte COMMON
reservation includes allocation padding and is not its C size.

## Startup

PSX's `_stacksize` at `8001022c` is SNDEF's initialized `0x8000` word. The
four-byte `SNMAIN_sbss` at `80010230` is SNMAIN's return-address save slot.
Both already have exact SDK evidence in `data.tsv` and
`docs/patterns/startup-address-origins.md`; their missing owner annotations are
filled without inventing game definitions.

`80010234` is SNMAIN's exclusive BSS end. The native object uses
`sectend(.bss)` at the two corresponding address pairs. Retail startup uses
it as a clear-loop limit and heap boundary, not as a static word to read or
write. Remove the scanned data identity while preserving the payload coverage
and numeric relocation targets.

## Initialized SDK tables

| Role | Owner | GAME | OPEN | Extent |
| --- | --- | --- | --- | --- |
| GPU driver operations | LIBGPU/SYS | `80057948` | `800370c0` | 16 pointers |
| GPU packet names | LIBGPU/OTAG | `80057a54` | `800371cc` | 24 pointers |
| Initial SPU voice registers | LIBSND/SSINIT | `80056520` | `80035c98` | 8 halfwords |
| Initial SPU global registers | LIBSND/SSINIT | `80056530` | `80035ca8` | 16 halfwords |

SYS's `.sdata` first word points to its operation table. Slot zero points to
its version string; the other 15 slots match the ordered native SYS `.data`
relocations at offsets `0x38` through `0x70`, using each target function's
curated archive member offset. The retail version string is four bytes longer
than the pinned SDK string, so the table is bound independently of an assumed
whole-section base.

OTAG's 24 native `.data` pointer relocations occupy offsets `0x34` through
`0x90`. Their complete target offsets in `.sdata` are
`44,40,3c,38,34,30,2c,28,24,24,24,24,20,20,20,20,1c,18,14,10,c,8,4,0`
(hexadecimal). The corresponding retail `.sdata` bases are GAME `80057de8`
and OPEN `80037558`. All fixed instructions in OTAG match the pinned member;
only version-text bytes differ in its initialized section.

SSINIT's entire `0x30`-byte initialized section matches the native bytes.
SsInit copies eight halfwords to each voice and sixteen halfwords to the global
SPU registers. These are two arrays, rather than four separately seeded
halfwords per image.

## Remaining field ownership

The reviewed [SDK data ledger](../../config/evidence/sdk_data_owners.tsv) records
523 remaining field entries. Its library/module columns identify the owner;
consumer columns identify the member containing the cited relocation.
`retail_site`, `paired_site`, `sdk_site` and `decoded_target` permit independent
comparison of the MIPS address pair and native LNK expression. Named retail
consumers in the small manually reviewed families are explained below.

The evidence routes are deliberately separate:

- Native relocations in completely matching functions bind the addressed object
  to its defining archive member, including external COMMON symbols.
- Unique matching instruction blocks in differing SDK revisions support the
  same module ownership. They do not promote the whole function to exact or
  establish an unchanged private field name or offset.
- Matching initialized bytes plus native relocations support the BIOS, CDROM,
  COR and SPRINTF initialized groups. They do not depend merely on where a
  stored pointer happens to point.
- Individually bound native objects support interior field ownership, including
  BIOS result/interrupt storage, ISO9660 work and load buffers, VMANAGER arrays,
  and interrupt callbacks. Their native whole-section bases are not assumed.

The retained DAT names, seed widths, unknown linkage and private types remain
WIP. `supported` on these entries qualifies module ownership only. This audit
neither supplies SDK bytes to the native executable nor claims game-source
progress, original file boundaries, or exact historical SDK attribution.

### Retail families and rejected inferences

| Family | GAME field range | OPEN field range | Evidence |
| --- | --- | --- | --- |
| Callback initialization flag | `80057d0c` | `8003747c` | ResetCallback and StopCallback read/write it |
| Interrupt register pointers | `80057d10..80057d1f` | `80037480..8003748f` | Four words equal native INTR `.sdata+0xc`: `1f801070`, `1f801074`, `1f8010f0`, `1f8010f4`; intInit consumers |
| Saved critical-section state | `80057d24` | `80037494` | Retail critical_section_set helper, already attributed to LIBETC/INTR |
| VSync runtime state | `80057e54..80057e67` | `800375c4..800375d7` | Retail VSync register reads, initialization flag and counter |
| Voice masks | `80057ff8`, `80058000`, `80058008` | `80037738`, `80037740`, `80037748` | SpuVmInit stores and SpuVmAlloc/SpuVmFlush consumers |
| Voice records | `8005acd8..8005af17` | `80038da8..80038fe7` | SpuVmInit caps the voice count at 24 and indexes records by 24 bytes; native `_svm_voice` references |
| Interrupt stack-pointer slot | `800632a4` | `80041374` | intInit stores the stack address and passes the containing context to HookEntryInt |
| Additional SPU state | `8006bd90` | `80049530` | SpuInit clears this retail state word |

The voice-array base also appears as CD_cachefile's exclusive `load_buf` end;
that boundary use does not transfer the sound object to LIBCD. ISO9660's native
file table has larger records than retail, so its full native extent is not
used as a retail object claim. The INTR native stack is 64 KiB while retail
uses 4 KiB; callback arrays are bound individually. VSync's native `.sdata`
contains more words than the retail group; extending it would incorrectly
consume neighboring game storage. VMANAGER's private current-state fields and
key-on/off masks differ between revisions, so their names are not copied from
a block alignment.

## Campaign verdict

All 1,036 previously unresolved ownership entries received a verdict. The
inventory now contains 748 identities and zero unresolved owners. It retains
530 DAT spellings; those are supported SDK owners with private names or layouts
still WIP, rather than 530 unknown game objects.

Across the campaign, 24 complete tables/objects replace 510 earlier identities
(including two already named `_ss_score` bases). Three false object identities
are removed: two string/padding words and the SNMAIN exclusive BSS boundary.
The remaining 523 field entries and two startup words gain supported owners.
No existing relocation status is promoted or numeric target changed. The two
SsInit callback-array pairs are newly curated. No SDK function is counted as
newly reconstructed game code.

Validation: all 101 delinked game target modules have identical hashes before
and after curation. Fresh strict objdiff comparisons for `game.item` and
`game.menu_map_viewer` retain six exact functions and the existing
`item_load_database` result of `99.746666%`. The 453 numeric ledger references
were checked against native patch sites/opcodes and decoded retail HI16/LO16
pairs; the remaining 70 rows use the reviewed retail families above. All three
native executables build successfully.
The full repository suite passes all 798 tests; lint and the cleanliness gate
also pass.
