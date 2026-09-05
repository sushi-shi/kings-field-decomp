# GAME parser inline-helper investigation

## Function Match Plan

The active goal remains strict 100% for all 29 GAME parser/serializer functions.
This continuation starts at clean `110ec5b`, with 23 exact and six remaining.
The user proposes source helpers that were expanded inline. This is a source
hypothesis, not permission to insert artificial register/frame carriers.
The previous [closure attempt](game-parser-exact-closure.md) records the full
per-function evidence and residual instruction streams. Retail initialization,
image-qualified evidence and caller/adjacency checks are refreshed before edits.

| GAME function / VA / size | Starting strict score | Meaningful helper hypothesis |
| --- | ---: | --- |
| `map_restore_floor_state` / `80035e44` / `69c` | 99.964540% | A shared per-floor record-address helper; both reader and writer use the same world anchor and 1700-byte stride. Only the final two address-arithmetic instructions currently differ. Preserve the complete restore, floor switch and call set. |
| `map_world_state_persist` / `80035b5c` / `2b8` | 94.821840% | The same record-address helper, then the eight-byte sparse-link copy as a distinct operation. Preserve the 8/128/190/160/10/20 loop extents, shared definition/object owner and all byte-store order. No calls in retail. |
| `render_bind_animated_instance` / `800205d4` / `3a4` | 98.454930% | The two identical aligned two-word-per-vertex copy loops are one candidate helper. Preserve the halfword predecrement counter, inherited keyframe index, allocation/release calls and GTE boundary. |
| `tmd_prepare_primitive_indices` / `8001c2b0` / `300` | 98.333336% | Typed per-mode packet preparation helpers within the eight-way switch. Preserve the independent word header load, halfword index stores, wrapping shifts and zero/countdown guards. Check whether actual inlining explains the unused eight-byte frame. |
| `item_load_floor_placements` / `80020b4c` / `1b0` | 98.888885% | The sentinel-count pass as a helper separate from placement expansion. Preserve the halfword global stores, saved input start and `rand` request order. |
| `item_load_database` / `80020cfc` / `5dc` | 99.746666% | The numbered lookup-name construction as a helper; division by 30/100/10 and the exact path bytes are already established. Preserve signed division and all SDK/CD calls. |

All six are game-specific format/state policies, not vendored implementations.
No compiler/profile changes are planned. For each trial, check actual emitted
helpers/calls as well as instruction and relocation differences: a successful
`inline` declaration alone does not prove that expansion happened. Retain only
humane source that is supported by the original operations, and verify any
improvement with strict objdiff, the corresponding retail/C/Rust oracle, full
build and repository checks. Bank only verified new 100% functions.

## Compiler capability

A small C control compiled with the pinned `cc1psx-257 -O2 -G0 -mcpu=r2000
-Winline` accepts `static inline`, emits the caller's shift/add multiplication
directly, and emits neither a helper body nor a call. The local GCC 2.5.8
`extend.texi`, section `Inline`, documents the same GNU C extension, including
`__inline__`, static helper omission and optimization requirements. This proves
the hypothesis is supported by the probe; it does not attribute the original
game compiler or establish that any specific helper existed.

The control is:

```c
static inline unsigned floor_offset(unsigned floor)
{
    return floor * 1700;
}

unsigned parser_inline_support(unsigned floor)
{
    return floor_offset(floor) + 10;
}
```

Compile it inside `nix develop` with
`cc1psx-257 -quiet -O2 -G0 -mcpu=r2000 -Winline control.c -o control.s`.
The [later GNU C manual](https://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html)
also describes static inline expansion in section 4.31; the pinned compiler
experiment, rather than that later manual, establishes support in this probe.

## Results

The record-address helper and a narrower multiply/offset helper both inline
without changing the restore candidate. The full address helper likewise
leaves persistence unchanged. Those helper trials are not retained.

The first vertex-copy helper used word-pointer parameters and added a load
delay. Giving the helper the actual `SVECTOR` array types, then forming its
two word cursors locally, reproduces the first retail copy loop completely.
The second copy still differs around metadata stores and scratch setup.
A cache-to-scratch operation taking record, clip, keyframe and vertex count
tests whether its inline arguments account for the retail clip reload before
metadata stores. This is a cohesive operation, not a placeholder argument
added to reserve a register.

The cache-metadata/copy wrapper emits identical code to the typed copy helper
alone and is removed. A separate cohesive record-reinitialization helper is
then tested against the remaining retry-parameter reload: its four inputs are
the record, owner slot, asset ID and vertex count, with the original allocation
failure/release loop and owner publication unchanged.

The reinitialization wrapper also leaves the typed-copy result unchanged and
is removed. The sparse-link helper adds two instructions and cursor moves,
so persistence keeps its original byte-copy loop. Eight typed TMD mode helpers
inline without changing the instruction stream; they are removed. A single
packet-preparation helper taking and returning the packet cursor also inlines,
but changes the independent header word load/shift to a byte load, swaps the
cursor/count registers and still does not reproduce the eight-byte frame.
It is removed.

The placement-count helper changes the count zeroing to an independent store
and no longer walks the caller's saved placement register. Its expansion is
less like retail and is removed. The complete model-filename formatter and
a narrower three-decimal-digit helper each inline without changing the STAT
candidate; neither is retained.

### Strict matching result

The explicit census remains **23/29 strict-exact (79.31%)**. No new function
is banked. Size-weighted objdiff similarity over the same 13,672 retail bytes
rises from 99.470158% to **99.493564%**; the unweighted mean rises from
99.662421% to 99.674261%. These are similarity measures, not fractions of
behavior proved correct. The sole retained source change is the typed
`SVECTOR` copy helper in `game.pool`.

| Function | Final strict score | Verdict |
| --- | ---: | --- |
| `map_restore_floor_state` | 99.964540% | Unchanged. Full address and multiply helpers do not alter the two remaining base-arithmetic instructions. |
| `map_world_state_persist` | 94.821840% | Unchanged. Address helper has no effect; sparse-link helper adds instructions. Base arithmetic, constant placement and register choices remain different. |
| `render_bind_animated_instance` | 98.798290% | Improved from 98.454930%. Typed copy helper reproduces the first copy loop completely and preserves the 932-byte extent. Allocation retry reload/backedge and scratch-copy setup/registers still differ. |
| `tmd_prepare_primitive_indices` | 98.333336% | Unchanged. Neither mode nor packet helpers explain the frame and loop-entry schedule. |
| `item_load_floor_placements` | 98.888885% | Unchanged. The count-helper expansion does not reproduce the retail count-pointer move. |
| `item_load_database` | 99.746666% | Unchanged. Both filename helper boundaries preserve the existing quotient-register differences. |

The final pool object has no `copy_vertices` symbol or added call: both uses
are expanded inline. Its lifecycle functions retain their addresses within
the unit. No compiler flags, function/data/relocation inventories, shared
types or Rust codecs change in this pass. The remaining differences are still
unattributed codegen residues, not demonstrated compiler limitations. These
experiments establish that inline helpers can matter to this probe, not that
the original source used this particular helper.

## Verification and Linux execution

The pool candidate is explicitly rebuilt with the pinned compiler, followed
by a full reconfigured `kf build`. All six focused image-qualified matches
reproduce the final scores above. No banked function is lost, and all 39 GAME
data-owning units still match. Repository lint and whitespace checks pass;
Python passes 359 tests plus 130 subtests. Rust passes 96 ordinary tests and
all five explicitly enabled proprietary-corpus tests; its library-only check
and formatting check also pass.

`python -m scripts.kf.codec_oracle` passes all ten complete suites with fresh
candidates and no case/event limits: 250 TMD payloads, 461 resource cases,
five outer floor walks, 1,136 animation cases, five VAB banks and nine SEQs /
15,880 events, five full-state VAB banks plus 16 runtime controls, 15 save-read
cases, 14 save-write cases, 15 world-restore cases and five world-persist cases.
The complete declared byte states and ordered service requests agree. No new
behavioral reconstruction bug is demonstrated by this pass.

PS1 code runs on Linux through the existing isolated Unicorn little-endian
MIPS harness. It maps hash-verified retail GAME bytes for one invocation and
relocates freshly compiled C functions into private emulated code slots for
another. Both runs receive the same input bytes, O32 arguments, stack,
inherited register seeds and poisoned output state. Complete declared output
regions and ordered service requests are compared with each other and with
the independent native Linux Rust driver.

This is an isolated function-call comparison, not a PS1 boot or playthrough.
Sony providers and deterministic SDK, allocation and GTE hooks remain explicit
boundaries. The runner audits executed load-delay dependencies separately
because Unicorn is not an R3000 timing model. Finite agreement does not prove
arbitrary malformed inputs, all inherited machine states or hardware timing.
See [instrumentation](mips-parser-instrumentation.md) and the
[coverage census](../game-resource-parser-coverage.md). The Rust library stays
dependency-free, `no_std` and `forbid(unsafe_code)`; bytemuck is not added.

## Remaining source and toolchain questions

Exact matching is still the objective; unsuccessful helper trials are not an
impossibility result. The manifest pins a reproducible Decompals old-gcc 0.17
GCC 2.5.7 PSX rebuild, not a proven identical retail compiler build. Source
semantics alone also do not fix the compiler-visible types, expression shape,
declarations or translation-unit context. No flag or compiler change follows
merely from that uncertainty.

A concrete remaining ownership question is the world-state declaration:
`map_world_state_base` is still a four-byte anchor identity, while both save
directions copy 8,500 bytes and the floor routines use a 1,700-byte stride.
Curated interior identities at offsets 3,400 and 6,800 through 6,803 also lie
inside that copied region. Their xrefs and the adjacent event globals need a
joint layout audit before replacing anchors with a shared owner. This is a
source-evidence avenue, not a promise that changing the type will close either
world-state function; no such ownership edit is included in this commit.
