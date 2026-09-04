# GAME parser failure paths and matching investigation

## Function Match Plan

This campaign extends the existing three-way parser oracles and investigates
the seven non-exact GAME functions explicitly listed in the
[parser census](../game-resource-parser-coverage.md). It does not replace more
SDK/hardware hooks, introduce formal proofs, or count Sony providers as game
reconstruction. The input worktree was clean at `2819a1c`.

Retail was initialized and hash-verified with `kf init`. Before edits, each
function's image-qualified address, disassembly/CFG, xrefs/callees, strings,
match report, source/history and existing inventory evidence were inspected.
Fresh candidates use the current manifest's `gcc257-native` probe; neither
that profile nor a matching percentage establishes historical attribution.

| GAME function / VA / body size | Initial strict score | Evidence snapshot and focused hypothesis |
| --- | ---: | --- |
| `tmd_prepare_primitive_indices`, `8001c2b0`, `300` | 94.135414% | Three GAME registry callers; no calls/strings. Two `lhu` count loads, nested masked countdowns, eight packet modes, `0xfd` mask and wrapping halfword shifts. Four reviewed address pairs; seven absolute internal jumps are still withheld candidates. Decode their raw targets/delay slots and curate those relocations before investigating the missing eight-byte frame and entry/zero-test scheduling. Do not invent a local to create the frame. |
| `render_bind_animated_instance`, `800205d4`, `3a4` | 90.793990% | Five render callers; O32 fifth argument at entry stack+16, four narrow integer parameters, pointer/sentinel return. Pool/allocation services, exact GAME selectors and Sony GTE call sites are identified. Incoming `s5` supplies the uninitialized keyframe index. Initial differences start at the allocation retry reload, followed by keyframe/morph zero guards, reverse selection and copy-register assignment. Preserve lifecycle semantics and the inherited value; no artificial initialization. |
| `item_load_floor_placements`, `80020b4c`, `1b0` | 97.731480% | Called by MIX loader; sole callee `rand`; three reviewed data pairs, no strings. `u16` sentinel/count, 12-byte input/24-byte output, signed offsets, 2000-unit tiles and 100-unit floor heights. Initial difference is count/original-input pointer allocation; expansion body agrees. Check whether the temporary global-count pointer obscures ordinary source structure, not a width correction. |
| `item_load_database`, `80020cfc`, `5dc` | 99.746666% | Called by game initialization. Six fixed copies, allocation/load/exit/release, 80 `CdSearchFile` calls; both literal paths and data banks agree. Full instruction comparison isolates quotient-register bindings in otherwise identical signed divide-by-30/100/10 and filename stores. No independently supported semantic correction yet; do not permute locals to assign registers. |
| `map_event_pool_load`, `800338b8`, `22c` | 97.215830% | Called by MIX loader; one occupancy callback, two globals and two reviewed internal jumps; no strings. Eight slots, byte sentinel/exhaustion flag, five-byte tag block copy, signed offsets, unsigned rotation. Source increments definitions then uses `definitions[-1]`; retail keeps the field cursor at current `initial_rotation` until that load. Try a direct current-record access and advance after it. |
| `map_world_state_persist`, `80035b5c`, `2b8` | 94.160920% | Death restart, interaction and unload callers; no callees/strings. Eight event records, 128 actor scan, 190 IDs, sparse links, two effect pools. Shared world/event base and 1700-byte stride are explicit. Differences include base expression scheduling and a separately materialized definition-table address. Preserve modeled object ownership; no overlapping globals/raw aliases to suppress the extra relocation. |
| `map_restore_floor_state`, `80035e44`, `69c` | 93.631200% | Called by floor loader; four `rand` sites and per-floor copy/link/death services; no strings. Marker guard, inverse record traversal, five-way floor switch. Retail predecrements sparse counts and tests against -1; source postdecrements against zero. Examine direct predecrement spelling and evidenced rotation-store order while preserving zero-count behavior. |

These are game-specific record/state policies, not vendor bodies: none of the
seven appears in `functions_vendored.tsv` or the FID match output. Existing
Psy-Q attribution covers the SDK callees; the TMD format/type evidence is
documented in [tmd-counts.md](tmd-counts.md). Adjacent functions and linked
ownership stay in their existing units except for the independently committed
animation-cache consolidation (`6f862d9`), which moved the binder into
`game.pool` while this campaign was running. That concurrent work is not counted
as a result of this campaign.

## Verification plan

- Failure controls retain full poisoned output regions and exact ordered
  service arguments. Add busy/allocation/validation failures and active fades
  to audio, repeated allocation failures to animation, and boundary inputs
  relevant to the investigated loaders. Explicitly distinguish safe Rust
  rejection from retail's unchecked malformed-memory behavior.
- After each focused source/config change, rebuild and inspect the first real
  instruction/relocation divergence. Run the corresponding retail/C/Rust
  oracle before keeping the change.
- Finish with all aggregate oracles, repository tests/lint, Rust checks,
  `git diff --check`, and a full `kf build`. Record each function's final
  verdict; bank only newly verified 100% functions, never a near match.

## Results

### Seven-function verdict

Scores below are strict objdiff reports after a full reconfigure, fresh
compilation and delinking. Non-100% numbers are similarity, not exactness.

| Function | Before | After | Kept change / remaining evidence |
| --- | ---: | ---: | --- |
| `tmd_prepare_primitive_indices` | 94.135414% | 94.317710% | Reviewed seven directly decoded internal `j` relocations. Raw targets and halfword-store delay slots have regression controls. No C change; entry/object-pointer ordering and the absent eight-byte frame remain unexplained. |
| `render_bind_animated_instance` | 90.793990% | 90.793990% | No campaign C edit. Allocation retry reload, keyframe/morph guards, reverse-branch shape and copy-register differences remain unattributed. Inherited keyframe index is preserved. |
| `item_load_floor_placements` | 97.731480% | 97.731480% | Direct global-count access was tried and rejected: it added repeated address materialization unlike retail's retained count pointer. Original source restored; initial pointer assignment differences remain. |
| `item_load_database` | 99.746666% | 99.746666% | No source change. Constants, copies, calls and divide checks agree; quotient-register assignment differs. All four source alignments exercise the aligned/unaligned copy paths. |
| `map_event_pool_load` | 97.215830% | **100%** | Load `initial_rotation` from the current definition before advancing; initialize exhaustion before the pool cursor. Complete instructions and relocations now agree. |
| `map_world_state_persist` | 94.160920% | 94.160920% | No source change. Base-expression scheduling and separate definition-address materialization remain; no overlapping alias was added to force a shared register. |
| `map_restore_floor_state` | 93.631200% | 97.052010% | Sparse loops now predecrement the shared loop counter and compare with -1; the first effect pool clears rotations in retail's z/y/x order. Remaining differences include actor-base hoisting, base arithmetic association, link-copy registers, second-pool cursor offsets and read/write order. |

The 29 explicit GAME functions in the census move from **22 to 23 exact**
(75.86% to 79.31% by function count). The resulting size-weighted similarity is
98.291691%; the unweighted mean is 99.096647%. These are not test coverage or a
probability of correctness. Sony providers remain excluded. Only the newly
exact event loader is eligible for banking in this campaign.

### Added controls and actual bugs

- VAB: 16 runtime controls cover normal success, busy versus other transfer
  flags, allocation failure, SPU end-limit rejection/equality, zero/short/error
  transfers, active fade, fade before rejection, nonboolean active state,
  exhausted bank slots, invalid magic, excessive program count and a nonzero
  unused sample-length tail. All mutated regions remain poisoned initially;
  comparisons include partial state, the adjacent cleanup byte, immutable VB,
  and complete ordered service arguments. Negative controls corrupt VH state,
  service trace and cleanup byte and must fail.
- Animation: seven lifecycle controls, adding three consecutive failed
  allocations and retries during different-asset reinitialization. The full
  animation census is now 1,136 cases.
- STAT: source addresses at all four byte alignments, with all six output
  banks, 80 directory entries and filename buffers checked each time.
- World restore: 15 cases, adding zero sparse counts, missing/skipped floor-one
  actor lookup, inactive floor overrides and both alternate floor-five link
  conditions. All complete output pools and deterministic helper calls agree.

The nonzero unused VAB length uncovered a **Rust state-model bug**: the old
success model summed all 256 entries, but retail advances across the whole
table while reading only `low_byte(sample_count) + 1` entries. Shipped unused
entries were zero, concealing the mistake. Both Rust runtime APIs now ignore
the unused tail; the test retains `0xffff` in the final unused entry.

The TMD fix corrects a **comparison inventory omission**, not C semantics:
withheld internal absolute jumps were being compared as unrelocated operands.
No new C parser behavioral mismatch was demonstrated. The two kept source
changes recover instruction-level structure without changing tested outputs.

Actual Sony code also exposes unusual failure behavior. With automatic bank
ID `-1`, header-error cleanup clears `bank_status[-1]` (`GAME:8005b01f`), not
the selected slot. A newly reserved slot and some header/program state can
therefore survive failure. The Rust API accepts that preceding byte explicitly
and never performs a host out-of-bounds write. When all slots are occupied,
Sony leaves its selected-bank register inherited; the control supplies 16
explicitly, reproducing rejection and the open-count decrement. Arbitrary
inherited values or unsafe memory extents are not claimed equivalent.

### How PS1 code runs on Linux

The Python harness loads the hash-verified GAME image into an isolated Unicorn
MIPS little-endian address space backed by 2 MiB of emulated RAM. O32 arguments,
stack, selected inherited registers and poisoned input/output regions are
seeded explicitly. It calls one retail routine and stops at a synthetic return
address. A second run links freshly compiled pinned-toolchain C objects into
private emulated code space, applying their MIPS relocations and resolving
symbols from the image-qualified inventories. Rust runs as a native Linux
driver with a length-prefixed byte-block protocol; its library remains
dependency-free, `no_std` and `forbid(unsafe_code)`.

The harness compares full captured data and ordered service calls across all
three implementations. SDK parser bodies such as VAB OpenHead/TransBody are
explicit shared retail providers; lower SPU/BIOS/GPU and selected GAME helper
services are deterministic hooks. This is isolated machine-code execution,
not a PS1 hardware emulator or a full-game playthrough. Instruction/store
instrumentation and separate load-delay audits are described in
[mips-parser-instrumentation.md](mips-parser-instrumentation.md).

### Remaining limits

This extends finite failure-path coverage; it does not prove all possible
inputs or all failures. In particular, arbitrary malformed pointers/counts,
unbounded allocation failure, all inherited-register values, additional SEQ
failure policies, hardware timing/interrupt interleavings and omitted service
bodies remain outside this campaign. No additional SDK/hardware boundary was
claimed implemented, and no bytemuck dependency was added.

### Verification

All commands ran in `nix develop` (flake checks use `nix flake check -L`).

- `python -m scripts.kf.codec_oracle`: all ten full suites pass, with fresh
  candidates and no case/event limits: 250 TMD payloads, 461 resource cases,
  five outer floor walks, 1,136 animation cases, five VAB banks, nine SEQs /
  15,880 events, five full-state VAB banks plus 16 runtime controls, 15 save-read
  cases, 14 save-write cases, 15 world-restore cases and five world-persist cases.
- `python -m unittest discover -s tests`: 351 tests pass against local retail.
- `cargo fmt --manifest-path tools/Cargo.toml --all -- --check` and
  `cargo check --offline --manifest-path tools/Cargo.toml --lib`: pass.
- `cargo test --offline --manifest-path tools/Cargo.toml`: 96 tests pass;
  five proprietary-corpus tests are separately enabled with `KF_RETAIL_DIR`
  and `-- --ignored`, and all five pass.
- `ruff check scripts tests`, `git diff --check`, and staged
  `nix flake check -L`: pass. The isolated Nix build runs 351 Python tests,
  skipping 22 tests requiring local retail/generated artifacts.
- Full reconfigured `kf build`, focused event/restore matches and final full
  `kf build`: pass, with no lost banked functions and all data-owning units
  matching. Bank only `game:0x800338b8` at 100%.

Generated logs and proprietary files stay under ignored local/build paths;
source, regression controls and this evidence report are the durable artifacts.
