# GAME parser matching checkpoint

Matching is paused at the user's request. This is a reproducible checkpoint,
not closure of the goal to make all 29 functions exact. The reconstruction
checkpoint is `5e32bd4`, including the retained source changes through
`c35fdfc`; this finalization changes documentation only and banks nothing.

## Matching result

The 29 explicit GAME parsers/serializers contain 13,672 retail code bytes.
Fresh strict objdiff reports reproduce **23/29 exact (79.31%)**, with
**99.493564% size-weighted similarity** and a 99.674261% unweighted mean.
Similarity is neither test coverage nor a probability of correctness.
Sony providers and auxiliary consumers are excluded from this denominator.
Across the broader reconstruction, GAME has 262/362 eligible functions exact;
the three images together have 350/485. These are function-code scores, not
a claim that every translation unit's code and data are byte-identical.

All addresses below belong to `GAME.EXE` (`SLPS-00017`).

| Function | GAME VA | Strict objdiff |
| --- | --- | ---: |
| `weapon_records_load_and_mirror_angles` | `800150a8` | 100% |
| `armor_records_load` | `800150fc` | 100% |
| `tim_upload_images` | `8001b100` | 100% |
| `common_resources_load` | `8001b180` | 100% |
| `map_resource_copy_words` | `8001b3e4` | 100% |
| `map_resources_load` | `8001b558` | 100% |
| `tmd_prepare_primitive_indices` | `8001c2b0` | 98.333336% |
| `tmd_register` | `8001c5b0` | 100% |
| `asset_registry_load_tmd_archive` | `800204c0` | 100% |
| `asset_registry_set` | `8002055c` | 100% |
| `asset_registry_select` | `8002059c` | 100% |
| `render_bind_animated_instance` | `800205d4` | 98.798290% |
| `item_load_floor_placements` | `80020b4c` | 98.888885% |
| `item_load_database` | `80020cfc` | 99.746666% |
| `save_system_read_catalog` | `8002b078` | 100% |
| `save_file_write_slot` | `8002b73c` | 100% |
| `save_file_read_header` | `8002bd08` | 100% |
| `save_file_read_slot` | `8002beb0` | 100% |
| `save_file_initialize_buffers` | `8002c304` | 100% |
| `actor_pool_load_placements` | `800308c0` | 100% |
| `actor_definitions_load` | `80030a6c` | 100% |
| `map_object_definitions_load` | `80030fdc` | 100% |
| `map_object_pool_load` | `80031008` | 100% |
| `audio_load_vab` | `80032984` | 100% |
| `audio_play_map_sequence` | `80032a4c` | 100% |
| `map_event_pool_load` | `800338b8` | 100% |
| `map_world_state_persist` | `80035b5c` | 94.821840% |
| `map_restore_floor_state` | `80035e44` | 99.964540% |
| `magic_load_records` | `8003a274` | 100% |

## What remains unresolved

The six differences are preserved as evidence, not hidden with loose scoring,
artificial locals/frames, forced registers or assembly:

- TMD: retail's unused eight-byte frame and primitive-loop entry schedule.
- Animation: allocation-retry reload/backedge and scratch-copy setup/registers.
- Floor items: count-pointer transfer; the candidate is one instruction shorter.
- STAT: quotient-register choices in filename construction.
- World persistence: base arithmetic, constant placement and register/order.
- World restore: two address-arithmetic instructions compute the same address
  but are not byte-identical.

The [source-shape campaign](patterns/game-parser-exact-closure.md),
[inline-helper investigation](patterns/game-parser-inline-helpers.md),
[shared map runtime owner](patterns/game-world-state-layout.md), and
[TMD header controls](patterns/tmd-packet-header.md) record the retained changes,
rejected experiments, raw residues and per-function verdicts. The shared owner
also made `map_event_update_spinner` exact, but that auxiliary consumer does
not increase the 29-function parser count.

These are unattributed code-generation differences. Failed helper experiments
do not prove 100% matching impossible or establish an optimizer limitation.
The pinned native GCC 2.5.7 PSX rebuild, maspsx and GNU assembler are reproducible
probes, not a proven identical historical toolchain. Original GCC 2.4.1 and
two GCC 2.6.0 candidates are staged; the newly proposed comparison of all six
current parsers with original compiler binaries was not executed before this
pause. No compiler/profile change is included in the checkpoint.

## Correctness findings and Rust codecs

All enumerated resource formats have independent Rust implementations and
comparison coverage at the boundaries in the
[complete parser census](game-resource-parser-coverage.md). The codec library
is allocation-free, dependency-free, `no_std`, and `forbid(unsafe_code)`.
It uses borrowed input, caller-owned output and explicit byte-order decoding.
No bytemuck dependency was added; PSX pointers remain 32-bit values rather
than host pointers, and comparisons use serialized bytes, not host layouts.

The work did find a real **Rust VAB state-model bug**: it summed unused
sample-length entries that retail skips. Shipped unused entries were zero;
a synthetic nonzero tail exposed the mistake. Both runtime APIs were fixed
and the regression control retained. The earlier TMD relocation omission was
a comparison-inventory defect, not a C semantic bug. The latest matching
passes demonstrated no new C parser behavioral mismatch. See the
[failure-control findings](patterns/game-parser-verification.md).

Finite differential agreement does not prove every malformed input, inherited
register value, allocation history, omitted helper body or hardware state.
Rust safely rejects some inputs for which retail performs unchecked accesses.
Identical code, correct ownership, tested behavior and hardware equivalence
remain distinct claims.

## How PS1 code ran on Linux

Linux did not execute `GAME.EXE` natively. Python drives Unicorn's isolated
little-endian MIPS CPU emulator, maps the hash-verified GAME load image into
2 MiB of emulated RAM with the KSEG0 alias, and enters a selected function
directly. O32 arguments, stack, inherited register seeds, input bytes and
poisoned destinations are explicit; a synthetic return address stops the call.

A separate invocation runs the pinned compiler's reconstructed C MIPS objects.
The harness relocates their functions/data into private emulated code slots,
resolves image-qualified symbols and applies MIPS relocations. Candidate calls
cannot silently fall through to undeclared retail functions. The native Linux
Rust driver receives the same inputs. Oracles compare complete declared output
regions, PSX pointer values, return state and ordered service requests across
retail, candidate and Rust.

Declared Sony parsers execute as shared retail providers where documented.
Disc/card I/O, allocation, GPU/SPU submission, selected GAME helpers and GTE
interpolation have explicit deterministic hooks or models. Code-fetch guards,
instruction limits, checked stores and separate R3000 load-delay audits make
unexpected execution fail. This is neither a game boot/playthrough nor a
cycle-accurate PS1 emulator. The
[instrumentation note](patterns/mips-parser-instrumentation.md) documents the
Unicorn delayed-control-transfer memory-hook defect and its guarded-store
workaround; retail instructions are not patched to obtain agreement.

## Final verification

All checks use the flake-supplied tools through `nix develop`; retail hashes
are verified again with `kf init` before binary queries or execution.

- An isolated `git archive 5e32bd4` is configured and fully rebuilt from
  scratch. Full `kf build --reconfigure`, `kf check`, Ruff and all 363 Python
  tests pass. The Rust oracle driver is built in that snapshot before the
  final Python run, so none of those tests is skipped.
- Rust formatting and the offline library-only check pass. All 96 ordinary
  tests and all five separately enabled `KF_RETAIL_DIR` corpus tests pass.
- Fresh shared-tree objdiff reports reproduce every score above. No parser C,
  Rust codec, harness, shared type, inventory or compiler profile changes in
  this finalization.
- After concurrent data-verifier test updates, shared-tree Ruff and all 372
  Python tests pass. These additional tests are not part of this report commit.

The committed snapshot's full build uses its **older relocation-masked data
gate**. Its 60/60 data-owning-unit pass is not proof of exact relocation
addends. Concurrent changes to `scripts/kf/data_match.py` and its tests remove
that masking and expose **13 GAME units with jump-table/addend differences**;
the shared-tree full build and `kf check` fail that stronger gate (47/60 units
pass). The mismatches are not erased by the successful historical-snapshot
check, and they are separate from the 23/29 function-code result. Those
verification changes and their accompanying documentation are preserved,
not reverted, banked or included in this matching-report commit.

The final `python -m scripts.kf.codec_oracle` run rebuilds all candidates and
passes **all ten complete suites**, without case/event limits:

| Suite | Compared corpus |
| --- | --- |
| TMD | 250 payloads: 246 shipped and four synthetic |
| Resources | 461 cases, including TIM, placements, archives, COM and STAT |
| MIX outer loader | All five floor walks and complete 50,000-byte grid outputs |
| Animation | 1,136 selection, cache, static and lifecycle cases |
| Audio | Five VAB banks; nine SEQs and 15,880 events, including 5,154 running-status events |
| VAB runtime state | Five shipped banks plus 16 success/failure/fade controls |
| Save reader | 15 deterministic cases |
| Save writer | 14 deterministic cases |
| World restore | 15 deterministic cases |
| World persistence | Five cases, including exact floor-five capacity |

Complete declared byte states and ordered service requests agree at the
documented provider boundaries. Whitespace checks pass. No additional exact
function is banked, and concurrent changes remain outside this report commit.

Reproduce the behavioral and code checks with:

```sh
nix develop --command python -m scripts.kf.codec_oracle
nix develop --command kf build --reconfigure
nix develop --command kf check
nix develop --command python -m unittest discover -s tests
nix develop --command ruff check scripts tests
nix develop --command cargo fmt --manifest-path tools/Cargo.toml --all -- --check
nix develop --command cargo check --offline --manifest-path tools/Cargo.toml --lib
nix develop --command cargo test --offline --manifest-path tools/Cargo.toml
```

The complete comparison runner rebuilds candidates and has no case/event
limit. The stricter shared-tree data gate must not be bypassed to obtain a
green full build. Generated logs, snapshot build products and proprietary
inputs are not committed.
