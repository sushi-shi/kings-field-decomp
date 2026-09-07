# Allocation mode domain and startup literal coverage

## Function Match Plan

Make the shared GAME/OPEN allocation-mode parameter a signed-word enum while
preserving its three existing values, all nine source call sites, and default
no-op behavior. The modes select creation, rebasing or heap use; they are not
allocation sizes or stack indices. Propagate the declaration, shared definition
and both inventories' final signatures. No caller requires an integer cast.

Review the allocator together with startup and resource-loading callers. Capture
image-qualified disassembly/CFG, callers, callees, strings, history and strict
scores before editing. The allocator's seven functions are game policy around
separately vendored malloc/free/InitHeap; the existing allocator dossier records
archive/header evidence. The repeated-store startup helpers do not advance their
destination and must not be described as BSS-clearing loops. Do not infer the
original purpose of those repeated stores or claim original symbols/TU bounds.

Publish complete literal rows for the shared allocator and three entry-point
files. The four other caller files already have complete ledgers; verify their
current expressions rather than counting them again. Preserve pointer-range
wraparound, inclusive arena ends, exclusive heap ends, signed alignment
arithmetic, unguarded LIFO depth and mode-dependent stack-entry interpretation.

Force all nine selected source/image variants. Compare all 112 isolated
before/after objects for the transitive header change and independently check
retail words, call sets and ordered references in the 42 selected functions.
Use Clang controls to reject raw/mixed modes and check signed decoded defaults;
run modern checking, inventory, Ruff, repository tests, whitespace and full build.
Add no production tests or size assertions for this type-only change; update
existing inventory expectations when their signature spelling changes.

## Starting evidence snapshots

All 42 functions start at strict 100%. GAME and OPEN share `memory.c`, with
the OPEN profile rebasing its address claims and selecting its arena offset.
The two mode functions each use a 24-byte frame, signed `slti` against two,
exact zero/one/two branches, two calls and five global address materializations.
Values outside the named three return through the common epilogue without
changing state. The create call supplies 0x100000 in its delay slot; both
create and rebase store an inclusive end and call the reset helper. Heap mode
only nulls the cursor, retaining depth and existing entries.

| Image | Function | VA / bytes | Source constraint |
| --- | --- | --- | --- |
| PSX.EXE | `main` | `0x80010028 / 208` | Preserve ordered startup calls, SDK arguments and image-specific entry/return protocol. |
| GAME.EXE | `repeat_store_word` | `0x80014268 / 36` | Non-advancing destination; repeat-count post-decrement and zero termination. |
| GAME.EXE | `main` | `0x8001428c / 136` | Preserve ordered startup calls, SDK arguments and image-specific entry/return protocol. |
| GAME.EXE | `game_main_loop` | `0x800146b8 / 740` | Pass create then rebase; capture/reset system heap after persistent resources are loaded. |
| GAME.EXE | `game_shutdown` | `0x8001499c / 56` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `frame_pacer_vsync_callback` | `0x800149d4 / 32` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `frame_pacer_wait` | `0x800149f4 / 112` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `memory_malloc_checked` | `0x8001aab0 / 56` | Preserve unsigned cached-RAM range guard around vendored malloc; signed byte count and pointer result. |
| GAME.EXE | `memory_allocation_reset` | `0x8001aae8 / 32` | Word stores clear stack depth and copy arena start to cursor; no allocation or free. |
| GAME.EXE | `memory_set_allocation_mode` | `0x8001ab08 / 168` | Typed signed-word selector; preserve three branches, default no-op and image-specific rebase offset. |
| GAME.EXE | `memory_capture_system_heap_start` | `0x8001abb0 / 32` | Add one byte to the inclusive arena end. |
| GAME.EXE | `memory_reset_system_heap` | `0x8001abd0 / 60` | Subtract captured start from exclusive 801f8000 end and pass signed byte count to InitHeap. |
| GAME.EXE | `memory_allocate` | `0x8001ac0c / 128` | Null cursor selects heap; otherwise round bytes to four and bump cursor, then push pointer/size without a depth guard. |
| GAME.EXE | `memory_release_last` | `0x8001ac8c / 100` | Pre-decrement depth; current cursor selects free versus rewind. No per-entry mode tag or underflow guard. |
| GAME.EXE | `tim_upload_images` | `0x8001b100 / 128` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `common_resources_load` | `0x8001b180 / 528` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `map_resource_path_set_floor` | `0x8001b390 / 20` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `map_resource_load_file` | `0x8001b3a4 / 64` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `map_resource_copy_words` | `0x8001b3e4 / 48` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `map_variant_assets_load` | `0x8001b414 / 136` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `audio_play_current_map_sequence` | `0x8001b49c / 188` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| GAME.EXE | `map_resources_load` | `0x8001b558 / 600` | Pass heap mode after map/variant resource setup; separate integer variant parameter remains outside this domain. |
| OPEN.EXE | `func_80013734` | `0x80013734 / 36` | Same repeated-store behavior; broader purpose and original name remain unresolved. |
| OPEN.EXE | `main` | `0x80013758 / 108` | Preserve ordered startup calls, SDK arguments and image-specific entry/return protocol. |
| OPEN.EXE | `opening_run` | `0x800156bc / 532` | Pass create then rebase; preserve later loading, display and scene decisions. |
| OPEN.EXE | `memory_malloc_checked` | `0x80015dd4 / 56` | Preserve unsigned cached-RAM range guard around vendored malloc; signed byte count and pointer result. |
| OPEN.EXE | `memory_allocation_reset` | `0x80015e0c / 32` | Word stores clear stack depth and copy arena start to cursor; no allocation or free. |
| OPEN.EXE | `memory_set_allocation_mode` | `0x80015e2c / 168` | Typed signed-word selector; preserve three branches, default no-op and image-specific rebase offset. |
| OPEN.EXE | `memory_capture_system_heap_start` | `0x80015ed4 / 32` | Add one byte to the inclusive arena end. |
| OPEN.EXE | `memory_reset_system_heap` | `0x80015ef4 / 60` | Subtract captured start from exclusive 801f8000 end and pass signed byte count to InitHeap. |
| OPEN.EXE | `memory_allocate` | `0x80015f30 / 128` | Null cursor selects heap; otherwise round bytes to four and bump cursor, then push pointer/size without a depth guard. |
| OPEN.EXE | `memory_release_last` | `0x80015fb0 / 100` | Pre-decrement depth; current cursor selects free versus rewind. No per-entry mode tag or underflow guard. |
| OPEN.EXE | `cd_file_load_allocated` | `0x80016014 / 328` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| OPEN.EXE | `cd_file_load_into` | `0x8001615c / 316` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| OPEN.EXE | `tim_upload_images` | `0x80016298 / 128` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| OPEN.EXE | `resource_stream_copy_words` | `0x80016318 / 48` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| OPEN.EXE | `opening_resources_load_scene0` | `0x80016348 / 456` | Pass heap mode with a0=2 prepared before the final chunk-pointer update. |
| OPEN.EXE | `opening_resources_load_scene1` | `0x80016510 / 180` | Pass heap mode in the call delay slot after saving the arena cursor. |
| OPEN.EXE | `opening_resources_load_scene3` | `0x800165c4 / 240` | Pass heap mode in the delay slot after releasing temporary resources. |
| OPEN.EXE | `opening_resources_load_ending` | `0x800166b4 / 308` | Pass heap mode with a0=2 prepared before saving the ending cursor. |
| OPEN.EXE | `opening_resources_load_ending_entities` | `0x800167e8 / 88` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |
| OPEN.EXE | `opening_resources_load_ending_sequence` | `0x80016840 / 156` | Unedited caller-unit control: preserve existing signature, resource/frame behavior, calls and data references. |

## Mode call-site evidence

| Image | Caller sites | Values and evidence |
| --- | --- | --- |
| GAME.EXE | `80014750`, `800147a0` | `game_main_loop` passes zero then one in the respective delay slots. |
| GAME.EXE | `8001b790` | `map_resources_load` passes two in the delay slot. |
| OPEN.EXE | `80015708`, `80015728` | `opening_run` passes zero then one in the respective delay slots. |
| OPEN.EXE | `800164f8` | Scene zero prepares a0=2 at `800164ec`; the delay slot stores its advanced chunk pointer. |
| OPEN.EXE | `8001659c`, `8001669c` | Scene one and three pass two in the respective delay slots. |
| OPEN.EXE | `800167c8` | Ending prepares a0=2 at `800167b4`; subsequent stores retain it through the call. |

The proven call rows exhaust the admitted incoming calls for both mode
functions. Internal validated branch rows are not extra callers. Neither body
has strings or an unresolved indirect call. Confidence remains supported;
enum typing does not promote evidence or prove historical compiler attribution.

## Implemented interface

`KfMemoryAllocationMode` now owns the existing create/rebase/heap members.
Both the shared definition and declaration accept it. All nine admitted source
calls already use these members and now receive compiler enforcement without
casts. The two identity rows and matching evidence final signatures use the
same type; their supported confidence, raw signed-width evidence and historical
starting-match fields are unchanged. The existing inventory test's two expected
parameter strings were updated after it correctly caught the old spelling.

Clang uses a scoped enum with signed-word storage. The legacy compiler sees
the same signed-word alias and values. Stack extent/index constants remain in
their own anonymous enum. A positive control checks every valid mode, assignment,
comparison, a typed function pointer and explicit decoding. Twelve negative
controls reject raw initialization/assignment/comparison/arguments, stack-index
and extent arguments, foreign-enum arguments/assignment/comparison, implicit
integer encoding, an integer-parameter callback and mode arithmetic. Decoded
minus one retains its sign; decoded three remains distinct from heap mode.
Neither decoding nor the enum declaration adds runtime validation.

## Allocator and startup contracts

The [complete ledger](allocator-startup-literal-ledger.md) accounts for the
allocator's eleven occurrences and fourteen more across the three entry-point
files. This adds four complete files, covering five source/image variants.
The mode members were already named; this pass strengthens their type contract
without claiming a reduction in the inline-token count.

The malloc guard relies on unsigned 32-bit wraparound to admit the cached
2 MiB address interval. Arena allocation rounds signed byte counts to four,
stores the rounded count, and advances the cursor. Heap allocation stores the
returned pointer in the same word entries. Release chooses its interpretation
from the current cursor, not a per-entry tag. Reset clears depth without freeing
entries. There is no new capacity, underflow or arena-end check, and enum typing
does not establish safe allocator-mode transitions for arbitrary callers.

Both repeated-store helpers have identical 36-byte bodies and repeatedly write
the same address. GAME passes zero to address `80058060` for `67fe8` iterations;
OPEN passes zero to `800377a0` for `70218`. The destination does not advance,
so neither count is described as a BSS byte extent. Their broader repetition
purpose and original symbol names remain unresolved. Both initial heap ranges
end at exclusive address `801f8000`: GAME starts at `800a0980` with `157680`
bytes; OPEN starts at `80080100` with `177f00` bytes.

PSX's forty pathname bytes match retail and both objects. Each twenty-byte
declaration holds sixteen characters, a NUL and three trailing zeros; the
original declaration/alignment rationale is unproven. The retail eight-byte
pointer table orders OPEN (`80010014`) before GAME (`80010000`). Load's integer
success result and the zeroed SDK EXEC stack fields remain vendor-boundary
values, separate from project overlay and allocation modes. The BIOS Load/Exec
wrappers are independently exact Release 2.5 LIBAPI C66/C67 members and are not
counted as game reconstruction.

## Per-function final verdicts

All starting scores remain strict 100%. Words, call targets and ordered address
references agree with the previous objects, delinked targets and raw retail.

| Image | Function | VA / bytes | Final verdict |
| --- | --- | --- | --- |
| PSX.EXE | `main` | `0x80010028 / 208` | 100%; unchanged words and references. |
| GAME.EXE | `repeat_store_word` | `0x80014268 / 36` | 100%; unchanged words and references. |
| GAME.EXE | `main` | `0x8001428c / 136` | 100%; unchanged words and references. |
| GAME.EXE | `game_main_loop` | `0x800146b8 / 740` | 100%; unchanged words and references. |
| GAME.EXE | `game_shutdown` | `0x8001499c / 56` | 100%; unchanged words and references. |
| GAME.EXE | `frame_pacer_vsync_callback` | `0x800149d4 / 32` | 100%; unchanged words and references. |
| GAME.EXE | `frame_pacer_wait` | `0x800149f4 / 112` | 100%; unchanged words and references. |
| GAME.EXE | `memory_malloc_checked` | `0x8001aab0 / 56` | 100%; unchanged words and references. |
| GAME.EXE | `memory_allocation_reset` | `0x8001aae8 / 32` | 100%; unchanged words and references. |
| GAME.EXE | `memory_set_allocation_mode` | `0x8001ab08 / 168` | 100%; unchanged words and references. |
| GAME.EXE | `memory_capture_system_heap_start` | `0x8001abb0 / 32` | 100%; unchanged words and references. |
| GAME.EXE | `memory_reset_system_heap` | `0x8001abd0 / 60` | 100%; unchanged words and references. |
| GAME.EXE | `memory_allocate` | `0x8001ac0c / 128` | 100%; unchanged words and references. |
| GAME.EXE | `memory_release_last` | `0x8001ac8c / 100` | 100%; unchanged words and references. |
| GAME.EXE | `tim_upload_images` | `0x8001b100 / 128` | 100%; unchanged words and references. |
| GAME.EXE | `common_resources_load` | `0x8001b180 / 528` | 100%; unchanged words and references. |
| GAME.EXE | `map_resource_path_set_floor` | `0x8001b390 / 20` | 100%; unchanged words and references. |
| GAME.EXE | `map_resource_load_file` | `0x8001b3a4 / 64` | 100%; unchanged words and references. |
| GAME.EXE | `map_resource_copy_words` | `0x8001b3e4 / 48` | 100%; unchanged words and references. |
| GAME.EXE | `map_variant_assets_load` | `0x8001b414 / 136` | 100%; unchanged words and references. |
| GAME.EXE | `audio_play_current_map_sequence` | `0x8001b49c / 188` | 100%; unchanged words and references. |
| GAME.EXE | `map_resources_load` | `0x8001b558 / 600` | 100%; unchanged words and references. |
| OPEN.EXE | `func_80013734` | `0x80013734 / 36` | 100%; unchanged words and references. |
| OPEN.EXE | `main` | `0x80013758 / 108` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_run` | `0x800156bc / 532` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_malloc_checked` | `0x80015dd4 / 56` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_allocation_reset` | `0x80015e0c / 32` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_set_allocation_mode` | `0x80015e2c / 168` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_capture_system_heap_start` | `0x80015ed4 / 32` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_reset_system_heap` | `0x80015ef4 / 60` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_allocate` | `0x80015f30 / 128` | 100%; unchanged words and references. |
| OPEN.EXE | `memory_release_last` | `0x80015fb0 / 100` | 100%; unchanged words and references. |
| OPEN.EXE | `cd_file_load_allocated` | `0x80016014 / 328` | 100%; unchanged words and references. |
| OPEN.EXE | `cd_file_load_into` | `0x8001615c / 316` | 100%; unchanged words and references. |
| OPEN.EXE | `tim_upload_images` | `0x80016298 / 128` | 100%; unchanged words and references. |
| OPEN.EXE | `resource_stream_copy_words` | `0x80016318 / 48` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_scene0` | `0x80016348 / 456` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_scene1` | `0x80016510 / 180` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_scene3` | `0x800165c4 / 240` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_ending` | `0x800166b4 / 308` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_ending_entities` | `0x800167e8 / 88` | 100%; unchanged words and references. |
| OPEN.EXE | `opening_resources_load_ending_sequence` | `0x80016840 / 156` | 100%; unchanged words and references. |

## Verification

Nine selected variants were force-rebuilt. All 112 isolated before/after
objects retain every section's contents, including debug information, and all
112 live objects agreed with the isolated results at the initial verification.
A subsequent concurrent death-camera experiment replaces one `*bob` assignment
with its `player_state.view_bob_offset` lvalue. Final verification records that
source difference separately; unchanged live artifacts do not verify an unbuilt
concurrent edit. The 42 selected functions retain 1,777 instruction
words, 250 direct calls and 156 address materializations, including
delay slots. No new function was banked.

Inventory, Ruff, whitespace and all 684 repository tests pass (115.951 seconds).
The final test run includes the updated enum signature expectations. Modern
checking retains the same 300 errors and 65/112 passing variants. Full
`kf build` retains source-data mismatches (PSX 0/1, GAME 9/42, OPEN 2/19) and
target-relink gaps (PSX 1/1, GAME 75/77, OPEN 34/38), with six conflicting
section bases and zero artifact failures. No size assertions were added.

The refreshed source census remains 6,509 inline occurrences, with 73 complete
file ledgers covering 4,422 occurrences. These are coverage totals, not a count
of constants still needing names. Source `unknown_` identifiers remain on ten
lines, with fourteen identifier tokens; no unsupported field identity was
invented to reduce that count.
