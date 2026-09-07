# GAME HUD, notification, and floor-item materials

The later [HUD/rendering constant audit](game-render-hud-constants.md) names
the atlas slots and six lighting consumers and supplies a complete literal
ledger for the frame, geometry, model and scene modules.

## Function Match Plan

At `13e081a`, name the cached GPU selectors and the HUD colour intensity from
their initializer and every known consumer. This campaign uses the project
matcher workflow, the pinned GCC 2.5.7 probe, and GAME-qualified semantic
queries. Six-view dossiers, adjacent claims, source history, and data xrefs
were captured before editing. These routines implement game rendering and
death policy; the vendor inventory instead attributes their external SDK
operations, including `GetTPage`, `GetClut`, and the matrix calls.

| GAME function | Hex extent; initial strict match | Evidence and preserved behaviour |
| --- | --- | --- |
| `800154b0 player_death_restart` | `19c`; 100% | No arguments, called by death update. Six proven calls and 33 validated references; `sb zero` at `80015604` clears HUD intensity. Keep restart selection, calls, branches and complete return. |
| `80016ee8 player_sync_position_to_map` | `158`; 100% | No arguments; position synchronization restores the HUD byte to 86 with `sb` at `8001701c`. Preserve signed map coordinates, camera/state copies and all caller-visible effects. |
| `800184b0 player_death_apply_visual_fade` | `90`; 100% | MATRIX pointer and signed word Q12 blend; three calls. Store `86 + ((-86 * blend) >> 12)` as a byte, after colour/fog updates. |
| `800186c4 player_death_update_reverse_fade` | `e0`; 100% | No arguments; signed halfword blend. Restore intensity using `(86 * blend) >> 12`, then advance the blend and retain both completion paths. |
| `8001bce0 render_initialize` | `2d8`; 100% | No arguments from display initialization; 32-byte frame, fifteen proven calls and 41 validated references. GPU results use `sh`; palette coordinates use `lh` into the two GetClut arguments. Preserve allocation, matrix setup, queue reset and delay slots. |
| `8001f218 render_entities` | `580`; 91.25% | No arguments from frame rendering; nine proven calls and 25 validated references. `lhu` loads the floor-item pair immediately before its draw loop. Keep visibility, iteration and all other materials unchanged. Signature remains a curated candidate. |
| `8001fde4 render_frame` | `518`; 100% | Nullable VECTOR/SVECTOR pointers forwarded to the view routine. Twenty proven calls and 68 validated references including internal branches. HUD pair and byte intensity precede HUD drawing; text pair precedes notification rows 0/1, digit pair precedes rows 2..5. |

For every function, retain its full instruction extent, branches, load and
control-transfer delay slots, ordered calls and real relocation targets.
No algorithm or SDK boundary type changes are proposed. Compare forced
compiles, all object sections with renamed symbols accounted for, and every
strict function score. Resolve complete exact-control words to retail VAs;
do not use a rounded percentage or a Ninja no-op as evidence.

## Cached values

| GAME VA | Name; storage | Producer and consumer |
| --- | --- | --- |
| `80095060` | `hud_clut`; u16 | GetClut of palette coordinates at `80055dac/ae`; active CLUT for HUD gauges. |
| `80095062` | `hud_tpage`; u16 | 4-bit, average-blend page at VRAM `(896,256)`; HUD drawing. |
| `80095064` | `hud_brightness`; u8 | Reset/fade and position synchronization; copied to active red, green and blue for HUD drawing. |
| `80095066` | `notification_text_clut`; u16 | GetClut of `80055db4/b6`; first two notification sprites. |
| `80095068` | `notification_text_tpage`; u16 | 4-bit, average-blend page `(832,256)`; notification text. |
| `8009506a` | `notification_digit_clut`; u16 | Same initializer result as text, separately stored and consumed for four numeric sprites. |
| `8009506c` | `notification_digit_tpage`; u16 | Packed selector `1c`: 4-bit, average-blend page `(768,256)`. |
| `8009508c` | `floor_item_clut`; u16 | Packed selector `7a40`: palette at `(0,489)`. |
| `8009508e` | `floor_item_tpage`; u16 | 8-bit, average-blend page `(896,0)`; floor-item draw loop. |

`LIBGPU.H` declares unsigned-halfword GetClut/GetTPage results. Its CLUT
encoding is `(y << 6) | ((x >> 4) & 0x3f)`; texture pages encode X in units
of 64 VRAM words and Y's 256-row bank along with depth/blend bits. These
values are SDK bitfields, not categorical game enums. The 86 intensity is
a per-channel texture-modulation value; its original artistic rationale
is unknown. It is not an opacity percentage.

The semantic names do not settle the complete BSS owner. These locations
are inside the [larger graphics clear and owner hypothesis](game-graphics-owner-pilot.md).
Keep their existing unsigned widths and shared rendering declarations,
without introducing independently allocated objects or overlapping owners.
Update the existing complete-owner fixture's member names and consumer
mapping so its checks continue to exercise the same source accesses.

## Palette-record hypothesis

Retail halfwords at `80055dac` are `(0,500,16,1)`; those at `80055db4`
are `(0,499,16,1)`. Each eight-byte interval exactly precedes the next
record or the colour-matrix table at `80055dbc`. SDK `RECT` has four signed
short members `x,y,w,h`; initializer loads prove the first two members'
coordinate roles. The trailing `(16,1)` is consistent with a single 16-colour
palette row, but has no separate code consumer and does not prove the
historical C declaration by itself.

The hash-checked MIX.TIM atlas corroborates the VRAM rows. Notification
text is its image 5, with 4-bit texture at `(832,256)` and CLUT at `(0,499)`;
see [the atlas decoding](game-notification-identities.md). Its TIM CLUT
block contains sixteen rows, whereas this candidate record selects one row.
Do not confuse the TIM block height with this record's stored height of one.
HUD image 6 likewise supplies texture `(896,256)` and CLUT `(0,500)`.
Floor-item image 1 is 8-bit texture `(896,0)` with CLUT `(0,489)`, agreeing
with the separately packed floor-item selector. Digit image 4 supplies
texture `(768,256)`; the notification initializer deliberately shares the
text palette for its digit material instead of selecting that TIM's own
CLUT row 498. Preserve this distinction.

First compile temporary sources using two authentic SDK RECT declarations
and `.x/.y` references, then the initialized owners. Compare all 182
initializer words, ordered calls and resolved addresses with retail. A
supported ownership model must also preserve other banked functions and
data bytes when integrated; record any contrary result before choosing
the retained source representation.

Both temporary initializer forms reproduce all 728 bytes, fifteen calls and
41 addresses. Initial integration in `render.c` exposes a new data-section
base conflict with its distant error-screen path at `80057b50`; it cannot
explain both initialized ranges as one linked section.

Refine the working data owner to `render_frame.c`: this routine consumes both
cached palette selectors and already owns the immediately preceding effect
sprite table (`80055d74..80055dac`). The two palette records extend that
presentation-data run through `80055dbc`, before the colour-matrix table.
This combines consumer-family evidence and a consistent linked data run;
it remains a WIP module assignment, not proof of the original TU boundary.
Keep the initializer's references external, preserve the complete frame
function and existing sprite bytes, and recheck the whole build's placement
diagnostics as well as the exact functions.

## Retained palette literals

The two initializers contain eight numeric occurrences, all authored resource
coordinates or dimensions. X `0` is the left VRAM origin; Y `500` and `499`
select the HUD and text palette rows corroborated by MIX.TIM. Width `16`
is the palette's colour count in VRAM words, and height `1` selects one row.
Keep these values together in their SDK records rather than introduce an
alias for each component. Their meaning is known; why the original assets
were packed at these particular VRAM positions is not recovered.

The existing [render.c literal audit](source-constant-names.md#gpu-setup-system-screens-and-transition-fades)
still covers its remaining inline values. The packed floor-item CLUT and
digit page already have domain names; their decoded coordinates are recorded
above. The light-matrix coefficients remain exact numeric matrix data.

## Final verdicts

| Reviewed function | Final strict match | Complete raw control |
| --- | --- | --- |
| `player_death_restart` | 100% unchanged | 103 words, six calls, 32 address pairs. |
| `player_sync_position_to_map` | 100% unchanged | 86 words, two calls, seventeen pairs. |
| `player_death_apply_visual_fade` | 100% unchanged | 36 words, three calls, five pairs. |
| `player_death_update_reverse_fade` | 100% unchanged | 56 words, five calls, nine pairs. |
| `render_initialize` | 100% unchanged | 182 words, fifteen calls, 41 pairs. |
| `render_entities` | 91.25% unchanged | All 354 source words and ordered references agree with the prior source; retail remains 352 words. No exact claim. |
| `render_frame` | 100% unchanged | 326 words, twenty calls, 64 pairs. |

Every function in `render.c` is also compared before/after, covering its
existing error-path datum and all TMD operations. Together these are 24 raw
controls: 21 exact functions reproduce 1,374 complete retail words, and the
three partials retain their prior complete words/calls/referents. Besides
`render_entities`, the partial controls are `display_initialize` (94.25301%)
and `tmd_prepare_primitive_indices` (99.375%). No match is newly banked.

All 112 objects and 484 strict function scores were checked against the
saved `13e081a` baseline. Four changed objects contain only symbol/string
table renames. The initializer's raw REL operands now express Y as an
addend of two to each RECT root; all resolved instructions agree. The frame
object adds exactly the sixteen retail palette bytes after the existing
56 sprite bytes; its instructions remain unchanged. Symbol-index changes
in relocations and debug-line changes are accounted for explicitly. The
other 106 objects are unchanged.

The existing complete-graphics-owner controls still pass with the new member
names; no new per-field tests or static size assertions were added. The
inventory count control now expects 2,952 data identities after four
interior-coordinate identities became two complete palette records.
There are 252 semantically named data identities, eleven more than before.

Validation passes: all 657 repository tests, inventory checks, Ruff and
whitespace checks. Modern diagnostics are the same multiset of 320 errors
(64 of 112 source/image variants pass). The full `kf build` retains its
existing failures: source data checks pass for PSX 0/1, GAME 8/41 and OPEN
2/19 units; target relinks remain 1/1, 75/77 and 34/38. The temporary extra
render section-base conflict is eliminated; the existing six conflicts
remain. GAME's known-reference accounting moves from 92 source-owned / 347
config-only ranges to 94 / 343, without claiming exhaustive byte coverage.
All sixteen new initialized bytes match, while original section alignment
and broader graphics ownership remain open.
