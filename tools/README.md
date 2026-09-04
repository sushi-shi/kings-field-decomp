# GAME resource codecs

`kf-codec` is a dependency-free `#![no_std]` library with no allocator. Like
the Gruntz tools, readers borrow resource bytes, transformations use caller-owned
buffers, and multi-byte fields have explicit byte order. File access and the
binary transport live in the `kf-codec-oracle` host executable, which uses `std`.

Modules cover chunk streams, COM/MIX views, asset archives and registry state,
TMD packets, TIM images, fixed database records, placements, animation, VAB/SEQ,
save files, and nested world-state records. The complete scope and verification
status are tracked in [the parser census](../docs/game-resource-parser-coverage.md).
An implemented reader or a roundtrip test is not by itself retail agreement.

```sh
nix develop --command cargo check --offline --manifest-path tools/Cargo.toml --lib
nix develop --command cargo test --offline --manifest-path tools/Cargo.toml

# Requires the locally initialized, hash-verified SLPS-00017 retail disc.
# Complete census, rebuilding the C candidates and Rust host driver:
nix develop --command python -m scripts.kf.codec_oracle

# Individual suites:
nix develop --command python -m scripts.kf.tmd_oracle
nix develop --command python -m scripts.kf.resource_oracle
nix develop --command python -m scripts.kf.map_resource_oracle
nix develop --command python -m scripts.kf.animation_oracle
nix develop --command python -m scripts.kf.audio_oracle
nix develop --command python -m scripts.kf.vab_state_oracle
nix develop --command python -m scripts.kf.save_oracle
nix develop --command python -m scripts.kf.save_write_oracle
nix develop --command python -m scripts.kf.world_state_oracle
nix develop --command python -m scripts.kf.world_persist_oracle
```

The Python oracles enter isolated retail functions and separately relocated C
candidate functions, then compare their output with Rust. Candidate game
callees must be explicitly included; unresolved calls fail closed. Vendored
providers and deterministic I/O/GPU/RNG services are declared explicitly and
reported, never silently substituted for a candidate parser. Output comparisons
use PSX-visible bytes, not Rust host structure layout. Seeded destination bytes
make unintended writes and failure to preserve untouched fields visible.

The execution harness is not a game boot or cycle-accurate PlayStation model.
It audits executed load-delay dependencies and guards code fetches and writes.
Its pinned Unicorn backend has a memory-write-hook defect affecting delayed
control transfers. Stores are instead guarded at instruction boundaries and
their resulting bytes verified, without changing CPU instructions or skipping
repeated PCs. See the [instrumentation evidence](../docs/patterns/mips-parser-instrumentation.md).
These checks strengthen bounded semantic evidence, but do not replace hardware
validation or strict objdiff matching.

## Running PS1 functions on Linux

Linux does not run `GAME.EXE` as a native executable. Python drives Unicorn's
little-endian MIPS CPU emulator and supplies an isolated PlayStation RAM model:

1. `kf init` verifies the retail executables. The runner copies GAME's load
   image into 2 MiB of emulated RAM, with the PS1's KSEG0 address alias.
2. For the C run, the pinned compiler builds real MIPS object code. The runner
   relocates the selected ELF functions and data into separate code slots,
   resolving calls and HI16/LO16/26/32-bit references explicitly.
3. Each run receives identical input and seeded state. O32 argument registers,
   stack arguments, stack pointer and a sentinel return address are set before
   entering the selected function directly. No game boot or Linux MIPS process
   is involved.
4. Declared Sony parsers can execute as unchanged retail providers on both
   machine-code paths. Disc/card I/O, allocation, GPU/SPU submission and the
   selected GTE service use explicit deterministic hooks or models. Calls and
   arguments are recorded; these are not claims of full hardware emulation.
5. The independent Rust implementation receives the same bytes and service
   inputs. Oracles compare PSX-visible output buffers, pointers, flags, return
   values and ordered service effects. Code-fetch bounds, instruction limits,
   load-delay auditing and checked stores make unexpected execution fail.

The [census](../docs/game-resource-parser-coverage.md) lists the exact shared
provider boundaries and compared state for each format.

Malformed-input checks make the Rust API bounded. Retail often trusts lengths,
indices, and sentinel presence; rejecting an out-of-bounds resource safely is
not a claim to reproduce retail's unchecked memory access. No proprietary
resource bytes are stored in this workspace.
