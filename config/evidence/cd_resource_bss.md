# CD location and opening-resource BSS ownership

Reviewed from `c127c33` on 2026-09-05 in the isolated data worktree. This is
source-data recovery; no function body, signature, literal, instruction,
function extent or relocation row is changed.

## Function Match Plan and pre-edit snapshots

Inspect image-qualified addr, disasm/CFG, incoming/outgoing xrefs, strings,
match state, shared consumers, adjacent boundaries and source history. Recover
the storage class and ownership first; retain the authentic four-byte `CdlLOC`
type. Rebuild the affected objects and compare all function scores, complete
data sections and independently relinked target objects. Never enlarge C
objects to imitate allocation padding or initialize CPE conversion residue.

| Image / function | VA / body size | Before | Evidence and retained source behavior |
| --- | --- | --- | --- |
| GAME cd_file_load_allocated | 0x8001acf0 / 0x170 | 100% | Four calls into it pass a pointer-output slot and path; strcat twice, CdSearchFile, allocation, CD command/read/sync, error display. Three location-byte stores at 0x8001adc0/c8/d0 precede CdControl at 0x8001ade0. Sector size 0x800, three attempts, zero return. |
| GAME cd_file_load_table_entry | 0x8001ae60 / 0x13c | 100% | menu_load_item_model passes output slot/index; allocation indexes 20-byte records. Location stores at 0x8001aec0/dc and 0x8001aef8 precede CdControl at 0x8001af08. Five attempts, CD read/sync and error display, zero return; existing signature inventory remains candidate. |
| GAME cd_file_load_into | 0x8001af9c / 0x164 | 100% | Eleven calls supply destination/path; same search/round/read family without allocation. Stores at 0x8001b060/68/70 precede CdControl at 0x8001b080. Three attempts and zero return. |
| GAME display_show_error_screen | 0x8001b7b0 / 0x308 | 97.922680% | Shared consumer in game.render, not edited. Builds stage-dependent error path, searches/loads/uploads TIM, displays it and waits for PAD. Stores at 0x8001b918/20/28 precede CdControl at 0x8001b938; 50 attempts. Shared ownership forbids a private GAME location. |
| OPEN cd_file_load_allocated | 0x80016014 / 0x148 | 100% | Thirteen calls pass output slot/path; search failure returns one, success rounds/allocates and tries reads up to 100 times, returning zero. Stores at 0x800160d4/dc/e4 precede CdControl at 0x800160f4. |
| OPEN cd_file_load_into | 0x8001615c / 0x13c | 100% | Three calls supply destination/path; same search/round/read family without allocation. Stores at 0x80016210/18/20 precede CdControl at 0x80016230. |
| OPEN opening_resources_load_scene1 | 0x80016510 / 0xb4 | 100% | Sole caller opening_scene1_run; stop audio, reset allocation, load MIXA1 VAB chunks, release, save post-VAB pointer with sw at 0x80016590, update arena, mode two and OPEN1 sequence. |
| OPEN opening_resources_load_scene3 | 0x800165c4 / 0xf0 | 100% | Sole caller opening_scene3_run; restore saved pointer with lw at 0x800165d8, then OPEN3, MIX3, MIXA3 placements at -10000 and MIXB3 TMD. |
| OPEN opening_resources_load_ending | 0x800166b4 / 0x134 | 100% | Sole caller opening_ending_scene_run; MIX9 TIM, MIXAE VAB/placements, END sequence, MIXBE TMD; save arena pointer with sw at 0x800167bc. |
| OPEN opening_resources_load_ending_sequence | 0x80016840 / 0x9c | 100% | Sole caller opening_ending_scroll_run; stop audio/reset allocation, restore pointer with lw at 0x80016878 and arena store in call delay slot 0x80016888; MIXAG VAB and ENDG sequence. |

All loops, branch/return delay slots, call ordering and signed-low HI/LO pairs
remain intact. The OPEN resource routines have no live-in arguments or escaping
results. Existing dossiers `open_semantic_cd_file.tsv` and
`open_semantic_resources.tsv` retain their earlier signature/caller reviews.
Source history includes `bed6922` (combined OPEN ownership) and `a5c8176`
(GAME-only CD table header). The GAME CD run lies between memory_release_last
and tim_upload_images; OPEN's combined run lies between memory_release_last
and lighting_set_active_color_matrix. Neither boundary moves.

Vendor negative control: these game-specific paths, allocation policy, error
display and scene transitions are absent from the vendor inventory. Their
Psy-Q services are separately identified in LIBCD.LIB SYS/BIOS and other SDK
objects. No vendor body is reconstructed or counted as game progress.

## Storage and SDK consumption evidence

GAME's location at `0x80057e80` follows CPE residue at `0x80057e68` and currently
has zero load-page bytes. OPEN's location at `0x800375d8` overlaps the literal
`43 50 45 01` CPE header. OPEN's two saved pointers at `0x800375e0/e8` follow
that header in the page tail. Neither the header nor surrounding zeros proves
C initializers. All four census ranges retain their addresses and four-byte
extents when classified as BSS; the loaded-byte partition is not reduced.

The GAME location has sixteen known references in four functions, including
game.render. OPEN has eight references, all in its two resource-unit CD
helpers. Each pointer has exactly one store and one load, both inside
open.resources; no address escapes. OPEN's controller routes scene three only
after scene one and calls the ending scene before ending scroll, establishing
the corresponding save-before-restore paths. The declarations become private
tentative storage, not initialized null pointers. The external CD-location
declaration belongs only in the GAME header; OPEN owns its location privately.

LIBCD.H declares CdlLOC as four unsigned bytes: minute, second, sector, track;
its command documentation lists only minute/second/sector for CdlSetloc (2).
Retail CdControl calls cd_setloc and CD_cw. The former copies only offsets
0, 1 and 2; the latter indexes a command parameter-count table whose entry 2
is three at GAME `0x800564a8` and OPEN `0x80035c20`. The parameter loop reads
one unsigned byte and increments the pointer by one in its branch delay slot.
Thus the unwritten track byte is not consumed by this command; do not shorten
the SDK type or fabricate a track initializer.

Pinned input hashes:

- LIBCD.H: `951f1ca8add88188d692ad13a6a86445ece5cc30c5e63dc32f846948d677b61d`
- LIBCD.LIB: `d05f3b1d730f4b8b63ba7231abaa7fb4289121902581f8649d411cdcbd854de3`

SYS's CdControl is already identified by an exact relocation-masked archive
section; BIOS's CD_cw has revision-skewed lineage, not exact archive identity.
The parameter-count proof above uses each retail image directly. Original
local/common allocation classes and any bytes beyond the C object extents
remain unresolved; target placement alone cannot prove them.

## Verification verdict

Every pre-edit function snapshot above retains its exact score: the nine
data-owning-unit consumers remain 100%, and the shared error-screen consumer
remains 97.922680%. All thirteen functions across game.cd_file/open.resources
remain 100%; the complete 484-row report is unchanged, preserving 354 exact
game functions. Only these two source objects and their two module targets
change; the other 115 source objects and 1,720 target objects are identical.
No function is newly claimed or banked, and no relocation row changes.

Both complete module targets now relink through GNU ld without overrides.
GAME compares 1,051 initialized bytes with 92 relocation rows, retaining
11-byte DATA and four-byte NOBITS BSS. OPEN compares 2,448 initialized bytes
with 192 rows, retaining 20-byte DATA, 180-byte RODATA and 20-byte NOBITS BSS
(three four-byte objects at offsets 0/8/16). This packing remains the current
target model, not proof of any unclaimed original allocation tails/classes.

Target verification rises from 107/117 to **109/117** (PSX 1/1, GAME 74/75,
OPEN 34/41). The eight other conflicting section-base failures remain.
OPEN's reached data is now 57 source-owned and 279 config-only ranges, down
from 280 config-only; the combined backlog is still 665. No candidate path,
code/data overlap or unresolved indirect-control diagnostic was suppressed.

Strict source data falls from 16/63 to **15/63**. GAME CD's prior apparent
data match flattened the location into a false initialized section. Its
source now has 16 DATA and 16 BSS bytes versus target 11 and 4. OPEN resources
has source DATA/BSS extents 32/32 versus target 20/20. These allocation/section
extent differences remain failures; no score-protecting tail was restored.

All 471 local tests and Ruff pass. `nix flake check -L` passes, with 53
local-retail/oracle skips in the sandbox; the actual SDK-header control runs
there, while all six new controls run with local retail. The full `kf build`
fails the strict data, eight placement conflicts and ownership/reference
gates as expected. Whole reachable-byte closure and linked-program equality
remain unproven.
