# GAME save confirmation and root-menu results

## Function Match Plan

At `237f8c4`, inspect the prompt, save/load panels, System hub, root menu,
save-confirmation caller, mode dispatcher and player-update consumer. Capture
all six image-qualified semantic views, source history and adjacent helpers,
all 112 object files and all 484 strict scores before editing. The eight
GAME functions are game policy, absent from the vendored inventory; SDK pad,
audio and memory-card implementation remains outside this naming work.

Use the existing signed-word `KfMenuConfirmResult` through the prompt's
result local, both save/load result locals and all three return signatures.
The two panel callers retain the enum directly. At the System hub, encode
the response into the existing integer transport, which also carries root
navigation values. Give those negative root-control values names and follow
their actual meanings through the root menu and player update. Do not make
the combined item-ID/control channel a confirmation enum.

Name the System/save/load row indices and counts from the decoded loaded
window records, retaining integer cursor arithmetic and every comparison.
Preserve all authored frame durations, texture IDs, pad waits, operation
status checks and the save panel's explicit format-row guard. Count three
save slots separately from the three-draw animation and three System rows.

The prompt begins pending (-99), toggles selected row 0/1, writes `-selected`
on confirm and -1 on cancel, presents its final frame, then returns after
button release. Its three direct callers constrain this as accepted/cancelled,
not a selected slot number. Save/load rejection returns to pending; success
retains accepted only after the operation succeeds. Catalogue failure or
panel exit returns cancelled. Formatting stays in the save panel.

The System hub converts accepted load result zero into -3. Its accepted quit
confirmation enters a non-returning redraw loop; do not claim it produces -2.
The root menu combines nonnegative item IDs with -99 (keep menu open), -1
(no item), and the forwarded -3. A completed magic panel also produces -1,
so naming the root value "cancelled" would be wrong. Player update treats -3
as loaded-state reconstruction and -2 as return to the opening intro. The
-2 consumer is proven, but no reconstructed producer has been established.

Force affected compiles and compare all object sections and strict scores,
then compare every captured function's linked words and ordered referents.
Require every prior exact to stay exact. Run modern type checks, inventory,
Ruff, existing tests, whitespace checks and full `kf build`. No size assertions,
new permanent tests, compiler-profile changes or banking are proposed.

## Resource and boundary evidence

The [window-kind review](game-menu-window-kinds.md) decodes `COM/STAT.DAT`
records 2, 4 and 5 with the shipped font: System is load/quit/return; save
has three slot rows, card initialization and return; load has three slot
rows and return. The separate fourth directory record is not an additional
visible save slot. Row names are indices in these records, not memory-card
file slot IDs: `cursor + 1` remains the one-based file-slot conversion.

All three confirmation APIs return full signed words in v0. The prompt
initializes s4 to -99 at `80028720`, copies the result to v0 in the release
wait's branch slot at `800287f0`, and preserves arithmetic negation on
acceptance. Save and load preserve the returned word in s2 at `80025290`
and `80025660`; their final return copies are at `80025504` and `80025870`.
The hub captures the load result at `80024f1c`, tests it against zero and
sets -3 in its following jump slot. Root returns through its release wait;
`menu_enter_mode` transports those bits through its existing u32 result.

The lower-level card/slot operation status is separate from confirmation.
A failed operation can keep the panel pending after the user already said
yes. No automatic cast from card status into a confirmation result is valid.

## Message textures

Fresh decoding of the five actual TIM resources confirms the message meanings.
`menu_load_item_texture` formats `TIM\\Mddd.` from `id + 1`, unlike the
save subsystem's direct message-number API. The new local constants retain
that zero-based index; they do not replace the other API's numeric domain.

| Index | File | Decoded text meaning |
| --- | --- | --- |
| `0x67` | `KF/TIM/M104.` | Data is loading. |
| `0x68` | `KF/TIM/M105.` | Data is saving. |
| `0x69` | `KF/TIM/M106.` | Initializing the memory card in slot 1. |
| `0x72` | `KF/TIM/M115.` | Game data exists; may the card be initialized? |
| `0x3e6` | `KF/TIM/M999.` | The system is ending; remove the CD and turn off the power. |

All five files are 4-bit TIMs with a 256-by-256 image uploaded at VRAM
(960,256) and the sampled palette row at (0,501). The first four have
palette data at byte 20 and image data at 544; M999 has one palette row
and image data at 64. SHA-256 hashes:

- `M104.`: `a77c113cdc7ab94bc9cac5b08594b6fb69a343c1a16498f48642224450efcb76`
- `M105.`: `bbee3ce86d81bee7a87d9f8f492146d4686667548141fe434bac2f7887d476bc`
- `M106.`: `1882c415f6751eef1a6fbc5f69cca130b4509b52e538e71a9ed795d3244d26da`
- `M115.`: `44b0fec3a297a159c93dd2ca6088165ae261cdaa031ad859c2590c29337bf610`
- `M999.`: `2c3a8f86d13ae11c46051be6dc286adf4f9536846d21388b817a5710186051e6`

The two yes/no labels use atlas codes 0x59/0x41 and 0x41/0x41/0x43,
respectively (はい / いいえ). The same shipped font and voiced-mark decoding
used by the linked window review establish their meanings. The prompt
selects row zero or one and negates it; the enum decode at this arithmetic
boundary preserves the retail `negu` in the jump delay slot at `80028874`.

## Kept distinctions and retained literals

Root controls are plain integer constants because that channel also carries
item IDs. Pending is internal; no-item covers cancellation and completed
magic use; game-loaded requests loaded-state reconstruction; return-to-intro
names the existing player consumer without asserting a reachable producer.
The System hub is the explicit boundary where typed confirmations enter that
combined channel. Save/load locals, returns and their direct prompt calls
retain `KfMenuConfirmResult` throughout.

The new row constants describe table positions, not file-slot IDs. The
three actual save slots determine the format and exit positions; the
System quit row is not a format command. Three message draws remain
authored timing, independent of these equal-valued counts.

The [complete ledger](game-menu-save-literal-ledger.md) covers all 140
retained occurrences in the four prompt/System/save/load bodies, down from
213. Including root controls, the save-confirmation window row count and the
player consumer, this batch replaces **84 source literals**. It adds four
root controls, nine row/count constants and five local texture IDs, and
reuses the existing three-value confirmation enum. The wider caller bodies
still require their own complete literal audits.

The save catalogue guard still accepts one or three. The reviewed header
wrapper maps raw three to one; the guard alone is not evidence for a
separate reachable catalogue-result state. Lower-level operation result one
is kept distinct from accepted confirmation zero. The ledger also preserves
the partial catalogue clearing, the exact zero-HP rejection, and the cleanup
helper's open-result check without claiming stronger file-validity guarantees.

## Verification and function verdicts

All five affected image/source units were forced to compile. Each captured
function preserves its complete linked words, ordered calls and resolved
data addresses. The six exact bodies reproduce all 885 retail words.
The root and player bodies retain their existing partial results. Private
player data references were resolved through their individual declared
data owners; its initialized objects do not share one contiguous retail
section base. No new match is banked.

| GAME VA / retail bytes | Function | Before = after % | Source words / calls / data-address pairs |
| --- | --- | --- | --- |
| `0x80018880 / 6684` | `player_update` | 96.9455 | 1658 / 66 / 204 |
| `0x800222b4 / 148` | `menu_save_confirm` | 100 | 37 / 7 / 2 |
| `0x80022348 / 704` | `menu_root` | 96.8636 | 175 / 28 / 1 |
| `0x80024e64 / 608` | `menu_save_load_hub` | 100 | 152 / 20 / 2 |
| `0x800250c4 / 1128` | `menu_save_panel` | 100 | 282 / 58 / 0 |
| `0x8002552c / 880` | `menu_load_panel` | 100 | 220 / 42 / 0 |
| `0x800286d4 / 576` | `menu_two_option_prompt` | 100 | 144 / 16 / 0 |
| `0x80036e38 / 200` | `menu_enter_mode` | 100 | 50 / 8 / 0 |

Across all 112 objects, this batch changes only `game.menu_item_drop`
`.debug_line`. A concurrent list-renderer reconstruction changes its object
and its partial score (88.25261% to 87.67223% at verification); the other
483 strict scores are unchanged. An independent compile of the saved
pre-edit list source against the new header preserves all code, data, runtime
symbols and their relocations, isolating the header from that concurrent work.

Modern checking retains the identical 320-error multiset: 64/112 variants
pass, 48 fail for existing reasons. Temporary checks using the actual
compilation database accept all three APIs as confirmation results, reject
all three implicit conversions to integer variables, and reject raw
-99/-1/0 initializers for the scoped enum. These probes add no permanent
tests or size assertions.

Ruff, inventory and whitespace checks pass. The final repository run passes
**678 tests in 80.379 seconds**. Full `kf build` retains the existing source-data
results PSX 0/1, GAME 9/42 and OPEN 2/19, target relink PSX 1/1, GAME 75/77
and OPEN 34/38, two GAME/four OPEN section-base conflicts and zero artifact
failures. No tooling or flake implementation changed in this naming batch.
