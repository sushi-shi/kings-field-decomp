# Visibility classes and system screens

## Function Match Plan

Promote the existing hidden/distant/near constants into a byte-backed
`KfCellVisibility`. Carry it through `KfCellWindow.cells`, both map traversal
pointers and the GAME/OPEN cell-rendering arguments. Preserve GAME's legacy
`char` parameter through `KF_ENUM_PARAM`; OPEN retains its u8 representation.
Replace the fixed window's 169 numeric classes and four GAME hidden-cell tests
with named members. Keep the authored grid, dimensions and origins intact.

Separately promote the four system-screen selectors to `KfSystemScreen`,
preserving the s32 argument and explicitly encoding it where it becomes the
filename's ASCII digit. All seven callers already use the supported names.
Compilation and match verification are deferred until the naming pass finishes,
at the user's request.

## Evidence and source result

The shared visibility resource consists of sixteen 204-byte records, with a
four-halfword header and 196 class bytes. GAME's second COM/COM.DAT chunk and
OPEN's B0/RTBL. carry the same data; all shipped classes are zero, one or two.
The [existing resource audit](source-constant-names.md#map-mesh-banks-and-visibility-classes)
establishes hidden/distant/near from both the authored spatial pattern and the
consumer: only class one adds 100 to select the distant map-mesh bank.

GAME's fixed window retains its 13-by-13 pattern, origin (6, 6), and the unused
zero-initialized tail. Table-local H/D/N macros alias the full enum names and
are undefined immediately after the initializer; they keep the grid readable.
The two loader copies remain raw resource boundaries into the same byte-backed
records. No per-cell conversion loop or runtime validation is introduced.

Reviewed retail snapshots (image is part of each identity):

| Image / function | VA / extent | Relevant evidence |
| --- | --- | --- |
| GAME `render_map_cell` | 0x8001e5ec / 0x250 | At 0x8001e6ec, masks the third argument to a byte; class one selects the additional 100-mesh bank. Staff remapping, orientation, wrapped coordinates and nine calls stay unchanged. |
| GAME `render_map_cells` | 0x8001e83c / 0x168 | Loads a class with lbu at 0x8001e928, skips zero and forwards it in a2. Steep-pitch fallback, yaw selection, header reads and row/column loops stay intact. Its pointer only reads cells and is now const-qualified. |
| GAME `render_entities` | 0x8001f218 / 0x580 | Four pool paths test class bytes against zero. The actor path joins a complete class byte loaded at 0x8001f3b0 with the alternative square-culling predicate at 0x8001f3f0. Keep the joined u8 local, explicitly encode the class and preserve its final nonzero test. |
| OPEN `render_map_cell` | 0x80018bbc / 0x1d0 | Masks a2 at 0x80018c3c; only class one selects the additional bank. Its caller, matrix calls and attribute/orientation/height referents agree with the shared domain. |
| OPEN `opening_render_map_cells` | 0x80018d8c / 0x140 | Traverses the same byte-backed record, skips hidden cells and passes their class to the emitter. Byte counters and full-halfword skipped-row strides stay distinct. |
| OPEN `opening_render_entities_and_items` | 0x80019240 / 0x298 | Both pool sweeps already compare the shared class bytes with the named hidden value. They acquire the enum through the shared field without body edits. |
| GAME `display_show_error_screen` | 0x8001b7b0 / 0x308 | Retains the full incoming word, adds ASCII zero at 0x8001b8a4 and stores the path byte at 0x8001b8ac. Seven calls select CD-search failure, CD-read failure, missing memory card or pause. Path handling, GPU/CD calls and press/release loops stay intact. |

The GAME dossiers and fresh OPEN disassembly/CFG, xrefs, strings and stored
match reports were reviewed alongside source history and curated evidence.
These are game-owned selection/traversal policies around separately attributed
SDK services. Address pairs are validated referents; direct calls are proven
control-flow edges. No relocation, claim or data owner changes.

Declarations, curated signatures, field metadata and existing inventory
expectations now agree on the two enum domains. The culling join's explicit
integer conversion preserves its actual mixed provenance rather than inventing
a near/distant interpretation for a boolean square test. The filename conversion
preserves the original full-width arithmetic before the byte store.

This names 173 inline values. The [map-cell remainder ledger](map-cell-literal-ledger.md)
accounts for all 30 retained occurrences in the two map-cell modules; the four
removed culling rows are reconciled in the existing HUD ledger. Builds, tests
and post-edit match checks have not run. Existing stored scores are baselines,
not verification of this change; nothing is banked.
