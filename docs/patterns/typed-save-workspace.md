# Allocated save workspace

## Function Match Plan

`save_workspace_allocate` allocates exactly 0x2800 bytes, publishes its
0x280-byte header and the following 0x2580-byte payload, and clears each
through separate SDK `memset` calls. The null path publishes a null header,
leaves the previous payload pointer unchanged and returns -1. The successful
path returns zero. Retail computes the payload address in the null branch's
delay slot without dereferencing it.

The shared header and payload pointers are also rebound to two independent
stack objects by `menu_save_confirm` and `menu_save_load_hub`. Their public
types must remain `KfSaveHeader *` and `KfSavePayload *`; neither pointer
universally denotes a complete allocated workspace. Do not combine the
stack objects or reinterpret those callers as a heap owner.

Introduce a complete `KfSaveWorkspace` containing the existing header and
payload for the allocated case only. Check total size 0x2800 and payload
offset 0x280. Probe a typed local allocation result, with an explicit cast
only at the generic allocator boundary. Publish `&workspace->header` on
success and null on failure, then derive `&workspace->payload` through its
real member. The conditional header publication must preserve the original
failed-allocation write and avoid addressing a member through a null pointer.
Keep both existing clear calls and their sizes; do not add carriers or pad
the model to influence code generation.

The six-view dossiers, allocation/reset callees, stack callers, source
history and pre-edit objects belong under
`build/cast-model/typed-save-workspace/`. All addresses are GAME-qualified.
The allocator and save layout are game policy; the memory-card and libc
services retain their independent vendored attribution. No SDK body or
signature changes. Compare from the first changed word after compiling:
physical referents, call sequence, CFG, then actual width/selection evidence.
Preserve every banked function and record any remaining unattributed residue.
Run layout/inventory controls, complete target-C parsing, all repository
tests, Ruff, diff checks and the full build before banking a retained change.

## Pre-edit snapshots

All three functions are GAME game-policy bodies, separately from the vendored
SDK services they call. All six query views are retained in the dossier.

| Function / VA / extent | Initial score | Instruction and reference evidence | Hypothesis |
| --- | --- | --- | --- |
| `save_workspace_allocate` / `8002c27c` / `68` | 100% | 26 instructions, three calls (`memory_allocate`, twice `memset`), five blocks, three data pairs, one internal jump relocation; null branch computes +`280` in its slot; return restores 24-byte frame in its slot; no strings or external confirmed callers | Give only the allocated `2800` bytes their complete header/payload layout; preserve failed-allocation global writes. |
| `menu_save_confirm` / `800222b4` / `94` | 100% | Seven calls, two data pairs, three-frame loop and pad-release loop; payload at `sp+290`, header at `sp+10`, `2818`-byte frame; return restores frame in its slot; no strings | Keep its existing separate typed stack objects and publication order as an aliasing control. |
| `menu_save_load_hub` / `80024e64` / `260` | 100% | Twenty call sites, two global pairs, seven validated internal jumps; payload at `sp+290`, header at `sp+10`, `2830`-byte frame; pad/menu branches and terminal redraw loop; return restores frame in its slot; no strings | Preserve independent stack buffers and the complete menu CFG as a second control. |

The public signatures remain `s32 save_workspace_allocate(void)`,
`void menu_save_confirm(void)` and `s32 menu_save_load_hub(void)`. Adjacent
release/initialization and panel bodies remain unit-wide comparison controls.
The pre-edit baseline is commit `8138a8e`.


## Result

The complete workspace type and member accesses are retained. The explicit
allocator-boundary cast replaces the former header-to-payload reinterpretation;
it also removes that allocation's implicit incompatible `void *` conversion
in the modern editor view. No public pointer, stack object, signature,
allocation size, clear size or physical referent changes.

| Function | Final verdict |
| --- | --- |
| GAME `save_workspace_allocate` | 100%; all 26 words unchanged, including both null-path global effects and every delay slot. |
| GAME `menu_save_confirm` | 100%; all 37 words unchanged, with independent stack buffers. |
| GAME `menu_save_load_hub` | 100%; all 152 words unchanged, with independent stack buffers. |

All 31 bodies across the three units retain their linked instructions, calls
and ordered referents. Twenty-nine remain exact; the two existing partials
(`memory_card_show_status_message` and `talk_show_dialogue_page`) are unchanged.
Only the edited allocator is selected for banking.

Layout validation covers the 0x2800-byte complete workspace and payload at
0x280. The inventory now has 123 types, 848 fields and 761 named fields.
All 112 target-C variants parse successfully, with no unlocated casts. There
are 713 written casts, including 48 in headers; C pointer casts remain 451,
down 355 from the initial 806. The allocation cast remains a real generic
storage boundary and is not treated as an avoidable field conversion.

All 714 repository tests pass (nine skips), as do Ruff and diff checks.
The full build retains 439/471 exact functions, zero artifact failures and
the documented data, closure and placement gates, including the OPEN arena
placement exposed by the preceding ownership stage.
