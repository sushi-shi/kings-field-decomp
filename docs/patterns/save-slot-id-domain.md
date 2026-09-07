# Save-slot IDs and directory tags

## Function Match Plan

Carry one-based logical slot IDs through the four save read/write APIs and the
byte directory tags. `KfSaveSlotId` uses the APIs' signed-halfword representation;
`KF_ENUM_STORAGE(KfSaveSlotId, u8)` preserves directory and catalogue-local byte
storage. Give slots one/two/three names alongside the existing empty/spare tags.
Name the three conversions between zero-based menu/summary rows and logical IDs.

All addresses below identify GAME.EXE. Current source/history and cached retail
disassembly/CFG, calls, strings, data references and historical match dossiers
were reviewed. Adjacent header read, workspace allocation and initialization
copy or clear the existing whole records; they do not reinterpret slot tags.
The [save status review](game-save-status-domain.md) records the surrounding
card/file contracts. SDK file functions keep their authentic integer arguments.

| Function | Address / extent | Evidence snapshot and intended change |
| --- | --- | --- |
| `menu_save_panel` | `0x800250c4 / 0x468` | Call `0x800252f8` passes cursor plus one in its delay slot. Name the origin and decode to the halfword domain at this boundary. |
| `menu_load_panel` | `0x8002552c / 0x370` | Call `0x800256c0` likewise passes cursor plus one. Keep cursor traversal and overlay selection independent. |
| `save_system_read_catalog` | `0x8002b078 / 0xd8` | `lbu` at `0x8002b0c4` reads header +0x200; zero and four are skipped, and `0x8002b0d8` subtracts one for the summary row. Type the byte local and name that origin. |
| `save_system_write_slot` | `0x8002b648 / 0xf4` | Signed-halfword normalization at `0x8002b6b0`/`0x8002b6b8` precedes the file writer. Type the API and preserve direct domain forwarding. |
| `save_file_write_slot` | `0x8002b73c / 0x4f4` | Byte scans first seek tag four, then zero, then the requested signed-halfword ID (`0x8002b8b0..c4`). The new tag is stored as a byte at `0x8002baf4`; the previous entry becomes spare. Type the parameter and inherited tag operations. |
| `save_system_read_slot` | `0x8002bde4 / 0xcc` | Signed-halfword normalization at `0x8002be34`/`0x8002be3c` precedes the file reader. Type the API and direct forwarding. |
| `save_file_read_slot` | `0x8002beb0 / 0x3cc` | Normalizes the argument to signed halfword at `0x8002bee0..e4`, compares it with unsigned byte tags at `0x8002bef8..bf00`, and uses the matching physical index for the payload offset. Type the parameter; preserve that search. |

## Encoded meanings and boundaries

There are three visible logical slots and four physical directory entries. Tags
one/two/three identify the visible slots; zero marks an empty entry and four
marks the spare physical entry. The writer prefers the spare, falls back to an
empty entry, then changes the previous occurrence of the requested logical ID
to spare. Thus the spare tag is not the fourth visible slot. The physical
directory index, byte offset, cursor and search-failure sentinel remain integer
values in their own roles.

The enum represents the full signed-halfword argument domain. The byte storage
wrapper preserves the old narrowing on writes, and comparisons still distinguish
a byte tag from a requested halfword outside that byte range. No new bounds
checks, clamping, directory repair or changes to the unguarded previous-entry
store are introduced. Whole-header clears/reads and existing save payload copies
retain their byte representation.

The first-slot constant replaces the one-based origin in two menu calls and
one catalogue expression. Menu addition still occurs before halfword conversion;
catalogue subtraction still promotes the unsigned byte to integer arithmetic.
The count of three visible slots and count of four directory entries remain
integer extents, separate from slot/tag values.

Stored match results describe older source snapshots. Builds, compiler checks,
tests, post-edit matches and banking remain deferred until naming is finished.

## Source result

All seven planned functions have the same final verdict: source propagation
reviewed, compiler and post-edit match verification deferred. The directory
field and catalogue local keep byte storage; the four APIs use the signed
halfword enum. Directory comparisons and assignments now carry the same domain
directly. All four current read/write call sites are typed: the two menu calls
decode the computed ID, and the two wrappers forward it to their file helpers.
Four curated signatures and one field row reflect these types without changing
their evidence tiers or field extent.

The two affected ledgers account for 132 save-system and 254 root/list/save
occurrences after removing the three newly named origin tokens. Full source
accounting covers all 111 C files and 5,997 retained numeric/character occurrences
with reasons. No builds, compiler checks, tests, post-edit matches or banking
ran; source review and documentation coverage do not establish a binary match.
The broader naming goal remains open.
