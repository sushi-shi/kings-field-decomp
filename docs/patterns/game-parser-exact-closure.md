# GAME parser exact-closure campaign

## Function Match Plan

Starting at clean `fa4c113`, the user requested an attempt to make all 29
explicit GAME parser/serializer functions strict-exact. Twenty-three already
match; the six below are the entire remaining set. The previous
[verification campaign](game-parser-verification.md) supplies the detailed
evidence snapshots, vendor exclusions and finite differential boundaries.
Retail is revalidated with `kf init`; evidence and focused raw comparisons
are refreshed before each edit. The current manifest uses `gcc257-native`,
not a historically proved compiler attribution.

| GAME function / VA / size | Starting score | Evidence-led experiment |
| --- | ---: | --- |
| `item_load_floor_placements` / `80020b4c` / `1b0` | 97.731480% | One MIX caller, one `rand` call, three reviewed data pairs; signed offsets, byte attributes, halfword ID/count. Retail walks the incoming pointer and saves its start only after zeroing the count. Make the argument the cursor and preserve a named start for the second pass. |
| `item_load_database` / `80020cfc` / `5dc` | 99.746666% | Main-loop caller; exact six copies, strings, SDK/CD calls and signed divide checks. Retail evaluates directory quotient before computing the one-based file number. Move that real calculation to its observed position; do not permute declarations to assign registers. |
| `tmd_prepare_primitive_indices` / `8001c2b0` / `300` | 94.317710% | No calls; eight packet modes, halfword consumer counts and reviewed internal jumps. Inspect object/cursor setup, countdown shape and sibling OPEN consumer; no dummy local for the eight-byte frame. |
| `render_bind_animated_instance` / `800205d4` / `3a4` | 90.793990% | Five render callers, fifth O32 argument, typed animation cache, exact selectors and explicit SDK services. Inspect allocation-result reload, narrow countdowns and reverse selection. Preserve inherited keyframe index. |
| `map_world_state_persist` / `80035b5c` / `2b8` | 94.160920% | Shared typed world/event and definition/object families, eight events, 128 actors, 190 IDs and two effect pools. Inspect typed base/cursor lifetime; do not invent overlapping global aliases. |
| `map_restore_floor_state` / `80035e44` / `69c` | 97.052010% | Inverse record traversal and five-way floor dispatch, four `rand` calls, shared actor/object families. Inspect typed actor-array hoisting and second effect-pool read/store sequence. |

All six remain game policies, not Sony/Psy-Q bodies. No vendor source, profile
change, raw assembly, volatile carrier or fake local may be used to bank them.
Each focused edit is compiled and compared at the first instruction/relocation
divergence, then tested against retail and independent Rust. Final verification
includes all ten oracles, full build, tests/lint, strict data checks, and banking
only new 100% rows. Unresolved results get explicit final verdicts.

## Results

The attempt improves five of the six functions, but does **not** close another
function: the explicit census remains **23/29 strict-exact (79.31%)**. No new
function is banked. Across the same 13,672 target bytes, size-weighted objdiff
similarity rises from **98.291691% to 99.470158%**; the unweighted mean rises
from 99.096647% to 99.662421%. These are similarity measures, not percentages
of proven behavior or test coverage.

### Per-function verdict

These scores come from the full build's objdiff report and image-qualified
focused matches. The line-similarity percentage printed by `kf try` is a
different measure and is not used here.

| Function | Before | After | Retained source / final residue |
| --- | ---: | ---: | --- |
| `item_load_floor_placements` | 97.731480% | 98.888885% | Walk the incoming pointer, preserve its start after zeroing the count, and use a postincrement sentinel test. The expansion body agrees. Retail moves the count pointer from `a0` to `v1` before the counting loop; the candidate retains `a0` and is one instruction shorter. |
| `item_load_database` | 99.746666% | 99.746666% | No retained edit. Moving the one-based file-number calculation after the directory quotient, and spelling its digit expressions directly, both emitted identical candidate code. Signed division constants/checks, copies, paths and calls agree; quotient-register bindings remain different. |
| `tmd_prepare_primitive_indices` | 94.317710% | 98.333336% | Compute the object countdown and object pointer before the zero guard, as supported by the retail entry and OPEN sibling. Packet bodies and internal jump referents agree. Retail still reserves eight stack bytes and schedules the primitive countdown in the entry branch slot; the candidate has no frame and puts the packet addition in that slot. |
| `render_bind_animated_instance` | 90.793990% | 98.454930% | Use a typed clip-table local, direct postdecrement keyframe/morph loops, and an explicit reverse override after the forward fraction. Keyframe/morph and reverse-selection control flow, plus the 932-byte extent, now agree. Retry-parameter reload and the two word-copy loops' registers/order remain different. The inherited `s5` keyframe index is preserved. |
| `map_world_state_persist` | 94.160920% | 94.821840% | Keep a typed object-definition table view across the sparse-object walk. Its address is now derived from the object-pool base outside the loop, matching the shared-owner relationship. Base-expression scheduling, constant hoisting and scratch-register choices remain different. |
| `map_restore_floor_state` | 97.052010% | 99.964540% | Guard the actor-array setup, share the object/actor lookup index, and read second-pool yaw before clearing link fields. All remaining instructions and relocations agree except the two base-arithmetic instructions shown below. |

The remaining restore difference is exactly:

```text
retail                         candidate
addiu v1,a0,-1690              addiu v0,v0,-1690
addu  s0,v0,v1                 addu  s0,v0,a0
```

At this point `a0` is the world-state anchor and `v0` is the floor times 1700.
The sequences compute the same address modulo 32 bits, but they are not
byte-identical. Splitting the base adjustment into a separate source statement
changed register lifetime and moved the adjustment earlier; commuting the
addition emitted the same original candidate sequence. Neither experiment is
retained.

Other rejected experiments are also explicit: direct global count access in
the revised item walk reintroduced repeated address materialization; moving
the inner TMD decrement before its guard exchanged packet/count registers;
nested serializer guards and a two-step output-base expression did not recover
retail. The animation allocation loop was simplified without changing its
emitted retry code. There are no dummy locals, volatile carriers, artificial
frames, forced registers, assembly bodies or toolchain-flag sweeps.

The matcher evidence rule stops further edits at these **unattributed codegen
residues** until another independently supported source fact is available.
This is not proof of a compiler limitation or of historical compiler identity.
In particular, the old source-shape notes' optimizer explanations are not
promoted to evidence for these six functions.

Concurrent commits `65f06d4`/`69ca614` (OPEN rendering), `3d0adb4` (animation
owner-slot types) and `2c071e7` (port roadmap) landed during this campaign.
They are preserved and are not counted as parser-closure results. Verification
uses the resulting shared headers and sources.

### Verification and Linux execution

The full reconfigured build passes with no lost banked functions and all 39
GAME data-owning units matching retail. All six image-qualified focused
matches reproduce the scores above. Repository lint and whitespace checks
pass; `python -m pytest -q` passes 359 tests and 130 subtests. Rust's 96
ordinary tests and five separately enabled proprietary-corpus tests pass;
`cargo check --offline --manifest-path tools/Cargo.toml --lib` and the Rust
format check pass.

`python -m scripts.kf.codec_oracle` passes **all ten complete suites**, with
freshly rebuilt candidates and no case/event limits: 250 TMD payloads, 461
resource cases, five outer floor walks, 1,136 animation cases, five VAB banks,
nine SEQs / 15,880 events, five full-state VAB banks plus 16 runtime controls,
15 save-read cases, 14 save-write cases, 15 world-restore cases and five
world-persist cases. The full declared byte states and ordered service
requests still agree with retail and Rust. No new behavioral reconstruction
bug is demonstrated; this pass improves the C source's instruction-level
correspondence. No additional cases, service implementations or proof claims
are introduced by this matching-only campaign.

PS1 routines are executed on Linux in Unicorn's isolated little-endian MIPS
address space. One run enters hash-verified retail GAME code; another enters
freshly compiled Psy-Q probe objects after their MIPS relocations are applied.
The harness seeds O32 arguments, stack, inherited registers and poisoned state,
then captures complete declared output regions and ordered service requests.
An independent native Linux Rust driver computes the corresponding results.
The Rust library remains dependency-free, `no_std` and `forbid(unsafe_code)`;
this campaign adds no bytemuck dependency.

Sony parser providers and deterministic SDK/hardware/helper hooks retain their
explicit boundaries. This does not boot a PS1 or run a full-game playthrough,
and finite differential agreement does not prove arbitrary malformed inputs,
load-delay behavior beyond the separate audits, or hardware timing. See
[instrumentation](mips-parser-instrumentation.md) and the
[previous verification report](game-parser-verification.md) for those limits.
