# Shared menu confirmation state

## Function Match Plan

At `5f3eebf`, propagate one confirmation-request domain through the related
menu input loops and both drawing helpers. The confirmed call family spans
ten C files. The existing equipment-local enum describes the same state;
move that domain to the shared header rather than keeping duplicate types.

The draw helpers consume the fourth O32 argument as a full word and select
their confirmed background only for exactly one. GAME `80028978` preserves
a3 in s5 in a branch delay slot; `800289c4/800289c8` loads one and compares
that saved state. GAME `80029258/8002925c` likewise compares the two-option
highlight with one. Selection index and returned accept/cancel result remain
separate domains. The pickup caller supplies zero in each of the call delay
slots at `800220bc`, `800220e8` and `80022114`, then forwards its confirmation
state during interaction. A confirm edge also highlights a declined option
or a full-stack refusal: this state does not imply successful acceptance.

Preserve every zero initialization, reset and exact-one guard, frame/input
sequence, and direct/indirect referent. The list footer currently declares
its local state as u32; preserve that representation through the storage
wrapper while the other locals and drawing parameters retain s32 storage.
No value validation, new branch, cast between domains or size assertion is
part of this change. Raw PadRead and SDK declaration debt remains separate.

Each function has an image-qualified address, disassembly/CFG, incoming and
outgoing xref, string and match snapshot, with the source history captured
before editing. Raw instructions and the existing menu-window, choice, list,
save and presentation evidence constrain the plan. The game composition and
input policies are absent from the vendored inventory; SDK bodies remain
external. Neighbors in each selected module are included as unchanged controls.

| GAME VA / bytes | Function | Initial strict % | Source hypothesis |
|---|---|---:|---|
| `80020b4c / 0x1b0` | `item_load_floor_placements` | 98.888885 | Unchanged contiguous-module control. |
| `80020cfc / 0x5dc` | `item_load_database` | 99.746666 | Unchanged contiguous-module control. |
| `800212d8 / 0x260` | `item_menu_root` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `80021538 / 0x5c4` | `item_menu_buy` | 99.769646 | Type confirmation flow; retain every branch, frame and call. |
| `80021afc / 0x500` | `item_menu_sell` | 99.609375 | Type confirmation flow; retain every branch, frame and call. |
| `80021ffc / 0x2b8` | `item_pickup_confirm` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800222b4 / 0x94` | `menu_save_confirm` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `80022348 / 0x2c0` | `menu_root` | 96.86364 | Type confirmation flow; retain every branch, frame and call. |
| `80022608 / 0x774` | `menu_use_item_panel` | 99.48218 | Type confirmation flow; retain every branch, frame and call. |
| `8002317c / 0x530` | `menu_magic_panel` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800236ac / 0x22c` | `menu_option_root` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800238d8 / 0x5c4` | `menu_equip_select` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `80023e9c / 0x470` | `menu_spell_select` | 99.54225 | Type confirmation flow; retain every branch, frame and call. |
| `800249a8 / 0x4bc` | `menu_drop_item` | 99.54785 | Type confirmation flow; retain every branch, frame and call. |
| `80024e64 / 0x260` | `menu_save_load_hub` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800250c4 / 0x468` | `menu_save_panel` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `8002552c / 0x370` | `menu_load_panel` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `8002589c / 0x504` | `menu_config_panel` | 99.85981 | Type confirmation flow; retain every branch, frame and call. |
| `80025da0 / 0x198` | `menu_config_panel_draw` | 100.0 | Unchanged contiguous-module control. |
| `80028380 / 0x354` | `menu_list_interact` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800286d4 / 0x240` | `menu_two_option_prompt` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `80028914 / 0x15c` | `menu_draw_window` | 87.54023 | Type confirmation flow; retain every branch, frame and call. |
| `800291ec / 0x10c` | `menu_draw_two_option` | 100.0 | Type confirmation flow; retain every branch, frame and call. |
| `800292f8 / 0x7b8` | `menu_draw_item_name_frame` | 95.37652 | Unchanged contiguous-module control. |
| `80029ab0 / 0x1a0` | `menu_blit_sprite_translucent` | 100.0 | Unchanged contiguous-module control. |
| `80029c50 / 0x190` | `menu_blit_sprite` | 100.0 | Unchanged contiguous-module control. |
| `80029de0 / 0x530` | `menu_draw_string` | 100.0 | Unchanged contiguous-module control. |
| `8002a310 / 0x200` | `menu_draw_number` | 100.0 | Unchanged contiguous-module control. |
| `8002a510 / 0x6a4` | `menu_draw_window_backdrop` | 99.97176 | Unchanged contiguous-module control. |
| `8002abb4 / 0x80` | `menu_frame_begin` | 100.0 | Unchanged contiguous-module control. |
| `8002ac34 / 0x98` | `menu_present_frame` | 100.0 | Unchanged contiguous-module control. |
| `8002accc / 0x50` | `primitive_buffer_begin_poly_ft4` | 100.0 | Unchanged contiguous-module control. |
| `8002ad1c / 0x50` | `primitive_buffer_commit_poly_ft4` | 100.0 | Unchanged contiguous-module control. |
| `8002ad6c / 0x8c` | `menu_list_init` | 100.0 | Unchanged contiguous-module control. |
| `8002adf8 / 0xac` | `menu_format_number` | 100.0 | Unchanged contiguous-module control. |
| `8002aea4 / 0x68` | `menu_load_item_model` | 100.0 | Unchanged contiguous-module control. |
| `8002af0c / 0x3c` | `menu_release_item_model` | 100.0 | Unchanged contiguous-module control. |
| `8002af48 / 0x130` | `menu_load_item_texture` | 100.0 | Unchanged contiguous-module control. |

## Result and verification

`KfMenuConfirmState` now has `KF_MENU_CONFIRM_IDLE` (0) and
`KF_MENU_CONFIRM_REQUESTED` (1). Seventeen input-state locals and both drawing
parameters share the domain, including the former equipment-local state.
The list footer preserves its u32 local representation; the other local and
parameter representations remain s32. All 32 source calls to `menu_draw_window`
and ten source calls to `menu_draw_two_option` pass typed state. The curated
function signatures and the presentation evidence agree with the header.

The selected units contain 807 retained literals before the change and 740
after it: 67 raw state occurrences become enum members. The
[root/list/save ledger](menu-root-list-save-literal-ledger.md) accounts for
265 retained occurrences across four previously unverified files. The current
configuration, equipment, confirmation/item and runtime ledgers are reconciled
with the changed expressions. This is complete occurrence accounting, not a
claim that every remaining selector has its final domain: save operation
results and the composite dialog selector remain explicit follow-up work.

Ten ignored Clang controls accept the typed state chain and unsigned-storage
chain, and reject eight raw/cross-domain uses. In particular, a selected
accept/decline option and an accepted/cancelled result cannot be used as the
confirmation-request argument, even where their numeric encodings coincide.
No production tests or size assertions were added. Modern checking retains
300 errors and 65/112 passing source/image variants; existing PadRead, SDK and
missing-declaration debt is not hidden or waived.

All ten units were forced to compile. An isolated before/after build of all
112 variants changes no allocated section, runtime symbol or ordered
relocation; only the equipment selector's debug line section changes. The
38 functions retain all 7,094 candidate words, 529 direct calls and 436 data
address references. Twenty-six exact functions also agree with the complete
3,352 raw retail words and the delinked target words and referents.

| GAME VA | Function | Final strict % | Words / calls / references | Verdict |
|---|---|---:|---:|---|
| `80020b4c` | `item_load_floor_placements` | 98.888885 | 107 / 1 / 3 | Unchanged partial |
| `80020cfc` | `item_load_database` | 99.746666 | 375 / 5 / 10 | Unchanged partial |
| `800212d8` | `item_menu_root` | 100.0 | 152 / 25 / 0 | Exact retained |
| `80021538` | `item_menu_buy` | 99.769646 | 369 / 21 / 11 | Unchanged partial |
| `80021afc` | `item_menu_sell` | 99.609375 | 320 / 20 / 5 | Unchanged partial |
| `80021ffc` | `item_pickup_confirm` | 100.0 | 174 / 28 / 3 | Exact retained |
| `800222b4` | `menu_save_confirm` | 100.0 | 37 / 7 / 2 | Exact retained |
| `80022348` | `menu_root` | 96.86364 | 175 / 28 / 1 | Unchanged partial |
| `80022608` | `menu_use_item_panel` | 99.48218 | 477 / 24 / 27 | Unchanged partial |
| `8002317c` | `menu_magic_panel` | 100.0 | 332 / 20 / 20 | Exact retained |
| `800236ac` | `menu_option_root` | 100.0 | 139 / 20 / 2 | Exact retained |
| `800238d8` | `menu_equip_select` | 100.0 | 369 / 25 / 13 | Exact retained |
| `80023e9c` | `menu_spell_select` | 99.54225 | 284 / 20 / 3 | Unchanged partial |
| `800249a8` | `menu_drop_item` | 99.54785 | 303 / 20 / 3 | Unchanged partial |
| `80024e64` | `menu_save_load_hub` | 100.0 | 152 / 20 / 2 | Exact retained |
| `800250c4` | `menu_save_panel` | 100.0 | 282 / 58 / 0 | Exact retained |
| `8002552c` | `menu_load_panel` | 100.0 | 220 / 42 / 0 | Exact retained |
| `8002589c` | `menu_config_panel` | 99.85981 | 321 / 19 / 9 | Unchanged partial |
| `80025da0` | `menu_config_panel_draw` | 100.0 | 102 / 9 / 11 | Exact retained |
| `80028380` | `menu_list_interact` | 100.0 | 213 / 21 / 0 | Exact retained |
| `800286d4` | `menu_two_option_prompt` | 100.0 | 144 / 16 / 0 | Exact retained |
| `80028914` | `menu_draw_window` | 87.54023 | 88 / 6 / 8 | Unchanged partial |
| `800291ec` | `menu_draw_two_option` | 100.0 | 67 / 7 / 11 | Exact retained |
| `800292f8` | `menu_draw_item_name_frame` | 95.37652 | 498 / 23 / 112 | Unchanged partial |
| `80029ab0` | `menu_blit_sprite_translucent` | 100.0 | 104 / 3 / 9 | Exact retained |
| `80029c50` | `menu_blit_sprite` | 100.0 | 100 / 2 / 8 | Exact retained |
| `80029de0` | `menu_draw_string` | 100.0 | 332 / 6 / 24 | Exact retained |
| `8002a310` | `menu_draw_number` | 100.0 | 128 / 2 / 8 | Exact retained |
| `8002a510` | `menu_draw_window_backdrop` | 99.97176 | 425 / 16 / 107 | Unchanged partial |
| `8002abb4` | `menu_frame_begin` | 100.0 | 32 / 1 / 4 | Exact retained |
| `8002ac34` | `menu_present_frame` | 100.0 | 38 / 5 / 4 | Exact retained |
| `8002accc` | `primitive_buffer_begin_poly_ft4` | 100.0 | 20 / 1 / 4 | Exact retained |
| `8002ad1c` | `primitive_buffer_commit_poly_ft4` | 100.0 | 20 / 1 / 5 | Exact retained |
| `8002ad6c` | `menu_list_init` | 100.0 | 35 / 0 / 1 | Exact retained |
| `8002adf8` | `menu_format_number` | 100.0 | 43 / 0 / 0 | Exact retained |
| `8002aea4` | `menu_load_item_model` | 100.0 | 26 / 3 / 2 | Exact retained |
| `8002af0c` | `menu_release_item_model` | 100.0 | 15 / 1 / 2 | Exact retained |
| `8002af48` | `menu_load_item_texture` | 100.0 | 76 / 3 / 2 | Exact retained |

The twelve partials keep their first raw divergence:

| GAME VA | Candidate | Retail |
|---|---|---|
| `80020b84` | `beq $v1, $v0, 0x80020bb0` | `beq $v1, $v0, 0x80020bb4` |
| `800211b8` | `ori $v1, $zero, 0x1e` | `ori $a2, $zero, 0x1e` |
| `80021568` | `addiu $s2, $zero, -0x63` | `addiu $s1, $zero, -0x63` |
| `80021b28` | `move $s2, $zero` | `move $s3, $zero` |
| `8002234c` | `sw $ra, 0x28($sp)` | `sw $ra, 0x2c($sp)` |
| `80022ccc` | `lhu $v0, 0x792($v0)` | `addiu $v0, $v0, 0x792` |
| `80023ec0` | `move $s3, $zero` | `move $s4, $zero` |
| `800249cc` | `move $s2, $zero` | `move $s3, $zero` |
| `800258d0` | `addiu $s5, $zero, -0x63` | `addiu $s6, $zero, -0x63` |
| `80028918` | `sw $ra, 0x2c($sp)` | `sw $ra, 0x28($sp)` |
| `800292f8` | `addiu $sp, $sp, -0xa0` | `addiu $sp, $sp, -0xe0` |
| `8002a510` | `addiu $sp, $sp, -0x28` | `addiu $sp, $sp, -0x68` |

These remain unattributed residues; no function is newly closed or banked.
All 484 scores agree with the captured comparison snapshot. Inventory, Ruff,
whitespace and all 684 tests pass (119.204 seconds). Full `kf build` retains
existing data/ownership failures: source-data matches PSX 0/1, GAME 9/42,
OPEN 3/19; target relink PSX 1/1, GAME 75/77, OPEN 34/38, with six conflicting
section bases and zero data artifact failures.

The live source census is 111 C files and 6,448 retained occurrences. The
current coverage map verifies **82 files / 4,981 occurrences**. The evolving
`map_scripts.c` remains outside that subtotal until its expressions and
referent evidence are reconciled. Unrelated sources and the root README are
excluded from this commit.
