# Shared arena allocation owner

## Function Match Plan

The allocator and resource consumers already establish three arena pointers
and a 17-word allocation stack in both images. OPEN `opening_run` supplies
additional aggregate evidence: it retains the cursor address `80075850` in
`s0`, clears depth with `sw zero,4(s0)`, reads start with `lw v0,-8(s0)`, and
writes the cursor with `sw v0,0(s0)` in the resource-load call's delay slot.
This is evidence for one owner, beyond address adjacency. The allocator's
cross-overlay lineage and identical relative field layout support sharing
that model with GAME at `800a01f0`.

Promote the existing local opening views to shared `KfMemoryAllocationState`
and `KfMemoryArena` definitions in `memory.h`. Preserve the current 17-word
stack extent and its depth/entry convention; do not invent a new capacity
guard or claim that the original typedef spelling or TU boundary is known.
The aggregate spans 0x50 bytes: start +0, inclusive end +4, cursor +8 and
the stack +0xc. It ends at the existing independent system-heap start word.
Use one `memory_arena` datum per image, defined in the shared allocator module
with a DATA claim and rebound by identity for OPEN. Remove the four interior
identities and update their relocation owner names without changing target
addresses or evidence tiers. Every consumer uses the shared members.

First probe a complete-owner pointer in `opening_run`, replacing its cast and
container recovery with typed fields. Preserve the reset order and original
call delay slot. Keep all allocator sizes, four-byte rounding, return forms,
zero-cursor heap mode, stack entries and SDK calls. The words that hold either
rounded sizes or heap pointers remain explicit generic storage in this stage.

The image-specific six-view dossiers, source history, pre-edit objects and
old identity map are under `build/cast-model/typed-memory-owner/`. Callers
establish signed allocation sizes, pointer returns and allocation-mode values;
the resource loaders directly retain or reset the same cursor. Adjacent CD
loaders use the returned storage generically. The malloc/free/InitHeap calls
are independently vendored SDK APIs; the LIFO and arena policy is game code.
No library body is reconstructed here. Review strings and every ordered
reference from these dossiers before the focused source change.

Compare original and new objects with their respective physical data identity
maps. A symbol-owner rename must not be mistaken for an instruction or target
change. Inspect the first real divergence in referents, calls, CFG and field
widths. Preserve every banked exact function; record any remaining codegen
residue without inventing compiler attribution or artificial source carriers.
Update layout/identity assertions, run full target-C parsing, all repository
tests, Ruff, diff checks and a full build. Run the flake checks if inventory
header validation changes. Bank only independently verified exact consumers.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| GAME `memory_malloc_checked` | `8001aab0 / 38` | 100% | 1/1/1 | 100%; raw unchanged |
| GAME `memory_allocation_reset` | `8001aae8 / 20` | 100% | 1/0/1 | 100%; raw unchanged |
| GAME `memory_set_allocation_mode` | `8001ab08 / a8` | 100% | 11/2/1 | 100%; raw unchanged |
| GAME `memory_capture_system_heap_start` | `8001abb0 / 20` | 100% | 1/0/1 | 100%; raw unchanged |
| GAME `memory_reset_system_heap` | `8001abd0 / 3c` | 100% | 1/1/1 | 100%; raw unchanged |
| GAME `memory_allocate` | `8001ac0c / 80` | 100% | 4/1/1 | 100%; raw unchanged |
| GAME `memory_release_last` | `8001ac8c / 64` | 100% | 4/1/1 | 100%; raw unchanged |
| OPEN `memory_malloc_checked` | `80015dd4 / 38` | 100% | 1/1/1 | 100%; raw unchanged |
| OPEN `memory_allocation_reset` | `80015e0c / 20` | 100% | 1/0/1 | 100%; raw unchanged |
| OPEN `memory_set_allocation_mode` | `80015e2c / a8` | 100% | 11/2/1 | 100%; raw unchanged |
| OPEN `memory_capture_system_heap_start` | `80015ed4 / 20` | 100% | 1/0/1 | 100%; raw unchanged |
| OPEN `memory_reset_system_heap` | `80015ef4 / 3c` | 100% | 1/1/1 | 100%; raw unchanged |
| OPEN `memory_allocate` | `80015f30 / 80` | 100% | 4/1/1 | 100%; raw unchanged |
| OPEN `memory_release_last` | `80015fb0 / 64` | 100% | 4/1/1 | 100%; raw unchanged |
| OPEN `opening_run` | `800156bc / 214` | 100% | 16/31/1 | 100%; raw unchanged |
| GAME `common_resources_load` | `8001b180 / 210` | 100% | 7/10/1 | 100%; raw unchanged |
| GAME `map_resources_load` | `8001b558 / 258` | 100% | 4/31/1 | 100%; raw unchanged |
| OPEN `opening_resources_load_scene0` | `80016348 / 1c8` | 100% | 1/18/1 | 100%; raw unchanged |
| OPEN `opening_resources_load_scene1` | `80016510 / b4` | 100% | 1/8/1 | 100%; raw unchanged |
| OPEN `opening_resources_load_scene3` | `800165c4 / f0` | 100% | 1/12/1 | 100%; raw unchanged |
| OPEN `opening_resources_load_ending` | `800166b4 / 134` | 100% | 1/12/1 | 100%; raw unchanged |
| OPEN `opening_resources_load_ending_sequence` | `80016840 / 9c` | 100% | 1/7/1 | 100%; raw unchanged |


## Result and remaining residue

The shared types and one 0x50-byte `memory_arena` owner per image are retained.
All 36 curated relocation rows keep their original target addresses and
reviewed evidence; only their owner name changes. Six interior identities
are removed. All 33 compared function bodies retain every linked word, call
and ordered physical referent, and all 22 reviewed functions remain exact.
Eighteen directly edited exact consumers are selected for banking.

The opening controller now takes `&memory_arena.allocation` without a cast.
Its container recovery uses the shared arena type; the duplicate local views
are gone. A complete-owner pointer changed four words (+0xf8, +0x138, +0x140,
+0x150), selecting the arena base and compensating field displacements;
strict objdiff was 99.90225%. A typed allocation pointer with a direct global
start read added a separate address load and four bytes (99.20301%). Calls
and branch topology agreed. Both experiments preserved the physical fields,
but neither is exact. The retained container expression preserves the
retail cursor base; no compiler mechanism is attributed to this residue.

The existing generic size-or-heap-pointer stack entries remain explicit.
One C pointer cast disappears (454 remain, down 352 from the initial 806).
Four casts in new static offset checks increase the whole-source census to
720, including 46 header casts. These checks are counted rather than hidden.
All 112 target-C variants parse without errors.

The 714-test repository suite passes with nine skips, including a new test
that recompiles both allocator copies and the opening controller and compares
all fifteen bodies to retail. Moving the arena owner by four bytes fails
all affected instruction comparisons while preserving the call sequence.
Inventory checks reject the removed interior identities. Ruff and diff
checks pass, and the full build retains all 439/471 exact functions.

The newly claimed GAME BSS block passes the data check. OPEN has the correct
80-byte extent but exposes the existing compiler/placement uncertainty: its
retail address 0x80075848 does not meet the emitted section alignment of 16.
This is a newly visible placement gate for formerly unowned storage, not a
function mismatch. No padding or alignment override is added. The other
full-build closure gates remain, with zero artifact failures.

The final flake check also passes all 714 tests (147 skips in its isolated
environment); the local retail run above executes the new owner control.
