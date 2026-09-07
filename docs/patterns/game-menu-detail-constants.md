# GAME item-detail and save-summary constants

## Function Match Plan

Baseline `d0fd06df1d75c800a45a1b8f0604b7e6387f8b71`, image **GAME.EXE**, unit `game.menu_item_detail`, profile `probe-gcc257-o2-g0`. The linked run and identity signatures remain curated working models.

The six semantic views were captured for each function, together with source history and retail save/load panels, two-option prompt, save confirmation, list interaction, database loading and number formatting/rendering consumers. Calls are decoded direct/proven; data address pairs retain their validated status. Preserve every branch, load and transfer delay slot, signed width, call and ordered referent. These game-specific menu and save-resource consumers are not vendored library bodies; their GPU/GTE SDK callees remain external.

Name the shared marker/dialog ordering depths, three slot-overlay quad indices, summary spacing and four-digit status width. Propagate the existing number advance from the runtime private enum into the shared menu header so four-digit current values, slash placement and maximum values express the same seven-pixel pitch. Name the repeated item-detail line spacing. Rename the dialog index parameter to `highlighted_slot`, preserving its signed integer type and all branches, and use the save-slot count for its upper boundary.

| Function | GAME VA / bytes | Strict baseline | Retail calls / conditional branches / returns | Hypothesis |
| --- | --- | ---: | --- | --- |
| `menu_draw_item_detail` | `80027b7c / 732` | 92.207650% | 17 / 3 / 1 | Preserve price-table selection and preview transforms; name the repeated 18-pixel baseline advance. |
| `menu_add_marker_quad` | `80027e58 / 72` | 100.000000% | 1 / 0 / 1 | Name the mid-depth quad bucket 500; retain active-buffer selection. |
| `menu_add_frame_quad` | `80027ea0 / 68` | 100.000000% | 1 / 0 / 1 | Retain the first-bucket pointer directly; no artificial zero constant is needed. |
| `menu_draw_dialog_frame` | `80027ee4 / 1180` | 100.000000% | 27 / 7 / 1 | Interpret the signed selector as a highlighted slot, name the packet and number geometry, and retain exact summary traversal. |

## Resource and caller evidence

`KF/COM/STAT.DAT` (5,708 bytes; SHA-256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`)
is copied from file offset zero into `menu_assets`. Each dialog packet is a
24-byte POLY_F4. In both buffers, indices 2/3/4 have RGB (15,15,15), command 0x2a,
X bounds 174..301, and Y bounds 24..88 / 88..153 / 153..217 respectively.
These are overlays on the three summary regions. Their storage is BSS;
`item_load_database` supplies the packet bytes before drawing.

| Highlighted slot | Enqueued indices, in order |
| --- | --- |
| 0 | 3,4 |
| 1 | 2,4 |
| 2 | 2,3 |
| at least `KF_SAVE_SLOT_COUNT` | 2,3,4 |
| negative | none |

All fifteen direct call sites are in save/load panels, save confirmation or
the two-option prompt. Save/load forward their cursor, including the format
or return rows. The prompt initializes its forwarded selector to -1 and only
replaces it for save/load windows. Save confirmation passes 3 with null rows.
A null summary pointer suppresses text independently of the overlay choice.
Thus `highlighted_slot` is a signed position with out-of-range behavior, not
a closed enum of mutually exclusive dialog kinds. The old parameter name and
border description obscured this relationship.

Retail `menu_add_marker_quad` adds 2,000 bytes to the OT pointer (500 four-byte
entries). Dialog AddPrim calls add 4,000 bytes (1,000 entries). This bucket is
separate from the marker and the foreground helper's direct first-entry pointer.
Preserve call order because AddPrim inserts at the head of its bucket.

Summary baselines start at 30 and advance 65 pixels per slot. Labels advance 14
pixels per line. HP and MP each format four digits; the slash follows 28 pixels
later, then the maximum begins another 7 pixels later. The existing number
renderer advances 7 pixels per digit. These widths explain 28 = 4×7 without
claiming a rationale for the authored label origins or inter-slot gap.

Verification will force both affected units, compare isolated before/after
runtime sections and ordered relocations, inspect exact functions against
retail, and run repository tests, lint, inventory, modern checking and full build.

## Final verification

Eleven private definitions and propagation of the existing shared number
advance name 40 inline uses. The complete
[retained-literal ledger](game-menu-detail-literal-ledger.md) accounts for all
100 remaining occurrences; the preview slice was refreshed to 158 occurrences.
The source-wide census is 6,985 occurrences, with complete token/expression
accounting verified for 20 files / 1,909 occurrences. No missing-name count can
be inferred from that total.

The captured worktree also included the stats-header index-lifetime change
subsequently committed as `abd85dc`. Its source was identical in both frozen
controls and at initial live-object verification.

Forced compilation of the detail and runtime units and `kf build compare`
passed. All 112 isolated before/after source/image variants have identical
runtime sections, alignment, runtime symbols and ordered relocations. Initial
live objects equalled the isolated results; only the two edited C files' debug
line sections changed, and all 484 strict function scores were unchanged.

A subsequent concurrent edit to `menu_draw_status_details` changed its score
from 98.161130% to 98.296430%. Its current source was compiled independently
with both header versions: runtime bytes and relocations are identical and
match its live object. The other 111 live objects and 483 scores still match
the frozen baseline. This one source/VA-specific control separates that
reconstruction from the naming changes; it is not a general exception.

All four functions preserve 513 resolved instruction words, 46 calls and 52
address pairs. The three exact functions also match their target objects and
all 330 retail words. Each final score equals its baseline in the plan above;
item detail remains 92.207650%. Its first raw difference remains at
`80027b9c`: candidate `move s4,a1`, retail `move s2,a1`. This is an
unattributed source/codegen residue, not a proved compiler mechanism.
The number renderer retains the same bytes
when its pitch definition moves to the shared header. There is no new exact
function or banking claim.

Inventory checks, Ruff and `git diff --check` pass. All 680 existing tests pass
(86.721 seconds); no tests were added. Modern checking retains the same 300
diagnostics and 65/112 passing variants. Full `kf build` still fails on existing
data ownership/section-placement and relink problems: source-data matches
PSX 0/1, GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77, OPEN 34/38.
