# Source cleanup review

Reviewed source baseline: `19df9a3e` (2026-10-09). The cleanup plan is maintained
outside the repository. This review records source findings and implementation
verdicts; generated syntax counts do not prove object bounds or ownership.

## Current findings

| Finding | Source evidence | Current verdict |
| --- | --- | --- |
| Third actor attachment | Retail indexes three triples; the third x/y lanes are also special-attack chance/range | Candidate corrected to one three-element array with named signed readers; forced actor behavior and loader objects are byte-identical; see [layout evidence](patterns/actor-attachment-layout.md) |
| Saved-floor owner | `map_load.c` previously subtracted 1690 bytes before adding the floor stride | Candidate corrected to `world_state.floors[current_floor - 1].records`; forced `game.map_load` object is byte-identical to baseline |
| Audio resource sequencing | Five GAME/OPEN loaders read and advance `stream` in one call | Corrected: capture the header before advancing `stream`; all five calls and 18 resource-unit functions remain strict 100%; see [sequencing evidence](patterns/resource-vab-sequencing.md) |
| Effect arguments | `effect_pool_construct` walks raw stack slots from `&direction` | Open: documented typed-varargs candidates are non-exact |
| Stack carriers | Unused vectors/matrices in GAME, two OPEN reservations, one-shot menu loop | Five individual open verdicts: ordinary forms change frame instructions or glyph-loop registers; see [paired controls and SDK evidence](patterns/stack-carrier-review.md); exact bytes do not establish these source forms |
| Shared memory dependency | `src/lib/memory.c` included `kf/game/game.h` | Corrected to `kf/lib/memory.h`; all 39 remaining GAME consumers now import reviewed owner headers; see [dependency verdicts](patterns/game-header-dependencies.md) |
| Map-link word view | No direct `.words` consumer; retail copies the payload with two aligned word loads/stores | Retain: it supplies four-byte alignment for the aggregate copy; original declaration form remains unknown |
| Animation binding result | Returns NULL, pointer-shaped static sentinel 1, or a live record | Retain mixed pointer contract; five direct callers and stored-pointer evidence reviewed; public ownership comment clarified; see [contract evidence and indirect limits](patterns/animation-and-inherited-contracts.md) |
| Inherited behavior | Uninitialized animation read, missing returns, and an unwritten direction buffer | Retain four individually evidenced questions; original source explanations remain open; see [retention evidence](patterns/animation-and-inherited-contracts.md); intentional repairs belong in the port |

The clean baseline builds all three executables and reports **465/471 strict
exact functions**. Analysis still fails existing data and section-placement
checks: GAME 23/41 data modules exact, OPEN 10/20, PSX 0/1. A successful
executable build does not close those failures.

The third-slot asset census found floor 5 definition 7 uses effect code 56
and animation 3. Its shared triplet is `(0, -1000, -2200)`. This supports a
consuming third-slot representation; it does not prove an original typedef.

## Map-link alignment verdict

`KfMapObjectLink` is an eight-byte union. Its other members require at most
halfword alignment; `words[2]` supplies four-byte alignment. In GAME
`map_object_pool_load`, the complete assignment `object->link = placement->link`
corresponds to `lw v0,0(s2)` / `lw v1,4(s2)` at `80031170`/`80031174`, then
`sw` at `80031178`/`8003117c`. The placement link begins at offset 12;
the live-object link begins at offset 32. Placement stride is 20 and live
stride 44. The pinned fixture checks these sizes, offsets, and alignment.

OPEN advances the same placement owner by its complete 20-byte stride;
it reads the position/model fields and leaves the link unused. The saved-floor
reader and writer preserve all eight link bytes through byte cursors.
Keeping the word member supplies a supported aggregate-copy representation
without changing either image's model. The linked bytes establish aligned
accesses, not whether the original programmer used this union spelling or
another alignment mechanism.

## First implementation slice

The saved-floor reader uses the same complete floor owner as its writer.
The shared allocator imports `kf/lib/memory.h` directly. Forced compilation
with the manifest's pinned `gcc257-native` profiles leaves all 97 native-derived
ELF comparison objects byte-identical to the clean baseline, including instructions,
relocations, and data. Refreshed strict results for each affected function:

| Function | GAME address | OPEN address | Verdict |
| --- | --- | --- | --- |
| `map_restore_floor_state` | `80035e44` | — | 100%, owner access corrected |
| `map_refresh_dialogue_stages` | `800364e0` | — | 100%, unchanged |
| `map_load_floor` | `80036554` | — | 100%, unchanged |
| `map_load_floor_wrapper` | `800365f8` | — | 100%, unchanged |
| `memory_malloc_checked` | `8001aab0` | `80015dd4` | Both 100%, body unchanged |
| `memory_allocation_reset` | `8001aae8` | `80015e0c` | Both 100%, body unchanged |
| `memory_set_allocation_mode` | `8001ab08` | `80015e2c` | Both 100%, body unchanged |
| `memory_capture_system_heap_start` | `8001abb0` | `80015ed4` | Both 100%, body unchanged |
| `memory_reset_system_heap` | `8001abd0` | `80015ef4` | Both 100%, body unchanged |
| `memory_allocate` | `8001ac0c` | `80015f30` | Both 100%, body unchanged |
| `memory_release_last` | `8001ac8c` | `80015fb0` | Both 100%, body unchanged |

The existing retail/C/Rust restore comparison passes all 15 cases, covering
every floor, both floor-five paths, sparse overrides, full pools, and helper
traces. Nine focused layout/restore tests pass, including the deliberately
wrong size control and all 85 owner-relocation referents. Full `kf build`
produces all three executables. `kf check-types` passes 97/97 image variants
with zero enum-domain literals. Ruff and `git diff --check` pass.
`nix flake check -L` passes (883 tests, 146 expected skips in its isolated
suite). The full local repository suite passes all 883 tests with no skips.
The strict total remains 465/471; no functions are newly banked.
The existing data/link analysis failures are unchanged.

## Earlier review checklist

Historical checklist from the pre-cleanup README. Counts describe the earlier
review scope; they are not a whole-source safety verdict:

- Inspect unresolved data ownership with `kf verify board --data`.
  The board separates ownership candidates from the informational raw `DAT_`
  count; see [inventory metric rules](function-and-data-inventory.md).
- [ ] Close [SDK object evidence](sdk-object-audit.md): **29 lineage
  module identities**; the last ambiguous audio helper may belong to `SSCALL`.
  Match the functions the game needs: **607 SDK occurrences** have proven or
  validated reference paths, with **26 candidate-only** occurrences to review;
  see the [usage TSV command and scope](sdk-object-audit.md#matching-scope-and-function-list).
  The [complete SDK function TSV](../config/retail/functions_vendored.tsv) lists all
  **1,138** known linked SDK occurrences; the **505 unreached** rows are not
  automatically required reconstruction work or proven unused code.
  Search missing 1994 SDK archives/source before reconstructing them.
- [ ] Resolve the [SDK interrupt compatibility workaround](patterns/sdk-interrupt-return.md).
  Leaving the starting-door plaque open could stop BIOS input and sound updates
  in rebuilt games under PCSX-Redux/OpenBIOS; retail worked. The build currently
  applies a hash-guarded, one-instruction patch to the pinned SDK's interrupt
  dispatcher. It resolves the reproduced failure, but is construction debt,
  not a proven general SDK fix; original-BIOS/hardware validation remains open.
  Preserve original game logic in `master` and `classic`: do not change plaque
  timing, draw synchronization, or callback scheduling to avoid this failure.
  Resolve the SDK/BIOS compatibility cause with evidence; intentional game-side
  adaptations belong in `port`.
  Replacing the containing `INTR` object would cover **18 functions per overlay**;
  its [matching scope](sdk-object-audit.md#interrupt-workaround-scope) is smaller
  than rebuilding all unresolved SDK modules.
- [ ] Review casts and remove avoidable conversions: **625 written casts**
  (**544 pointer**, **81 scalar**; 19 of them in headers). The [cast/union debt campaign](patterns/cast-union-debt.md)
  clarifies grid, copy, and asset-offset access and requires explicit void-pointer boundaries;
  raw counts remain review inputs, not a measure of incorrect types. Its
  [open decisions](patterns/cast-union-debt.md#open-decisions-from-the-per-site-review)
  include the instruction-neutral `AddPrim`/`SetSemiTrans` erasures.
- [ ] Review unions and simplify avoidable alternate views: **31 union definitions**;
  seven wrappers replaced with canonical structs or SDK types. Four grids retain
  typed coordinate/linear views plus the loader's word view; `KfMapObjectLink.words`
  is declared but never accessed.
- [x] Review gotos: **72 statements** (**66 GAME**, **4 OPEN**, **2** in shared
  `src/lib/format.inc`); each joins a retail block with several predecessors.
- [ ] Revisit compiler-steering source forms: the OPEN emitters retain
  never-read stack carriers; GAME retains two unused SDK aggregates and a
  one-shot menu loop. Their source explanations remain unresolved.
- [x] Review owner recovery from member pointers: **0 sites**.
- [ ] Revisit owner and array boundaries: the actor third-attachment access
  exceeds its declared two-element array. The saved-floor owner expression
  has a byte-identical correction in the current cleanup worktree.
- [ ] Review manual varargs: **1 function**; `effect_pool_construct` still
  reads argument slots directly. The [argument-access audit](patterns/effect-constructor-varargs.md)
  records the typed `va_arg` candidate and its remaining non-exact code.
- [x] Review unrelated variable reuse: **0 functions**; working values are
  named locals initialised from their parameters at declaration.
- [ ] Review stack aggregates and unused members: the carriers above remain
  open; the
  unread second `ReadSZ2` argument is the SDK call sequence retail makes.
- [x] Review unresolved buffer bounds: **0 regions**; both formatter scratch
  buffers are claimed as the aligned 24-byte reservation their code
  accesses and match the data gate; the original declaration extents are
  unknowable from the image and are noted in source.
- [x] [Triage compiler warning families](patterns/compiler-warning-triage.md):
  15 safe cleanup sites retain identical code/data; **3,845 Clang C++20**,
  **2,719 Clang C89**, **631 GCC** unique diagnostic lines remain, including
  SDK/compatibility diagnostics. Per-site type/buffer work remains open.
- [ ] Resolve five resource-cursor sequencing sites and **2 data preconditions**
  narrowed by shipped-data checks. Preserve **4 inherited scalar uninitialized
  reads** and three passed-buffer warnings on decomp; intentional behavior repairs
  belong in `port`.
- [x] Search for inline functions and apply the review; see the [39 retained helpers](patterns/common-code-review.md).
- [x] Search for macros for common code and apply the review; [522 function entries read](common-code-functions.tsv), [90 candidate verdicts](common-code-candidates.tsv).

Preserve banked matches. Cast/union/goto counts cover the tree; other counts
cover audited cases. See the
[cast review and matching constraints](patterns/cast-owner-reduction.md)
and the [reconstruction debt review](patterns/reconstruction-debt-review.md).
