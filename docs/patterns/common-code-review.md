# Common-code review and applied extractions

The standalone review below predates the PR stack. See
[stack integration](#stack-integration) for validation against the shared-enum
base and the conflict resolutions retained in the combined source.

The review and application cover **90 candidate families**. All 35 further
recommendations have been tested: 33 have retained extractions, sometimes
narrowed to the independently useful operation; two retain their original
source after failed trials. Together with the initial four macros, the result
is **39 helper definitions: 17 static inlines and 22 macros**.

The [candidate catalogue](../common-code-candidates.tsv) records the form,
consumer boundary, trial failures and final decision for every family:

| Verdict | Families |
| --- | ---: |
| Retained | 36 |
| Keep existing source/helpers after review | 52 |
| Rejected extraction after compilation | 2 |
| Pending | 0 |

The [function checklist](../common-code-functions.tsv) contains **522 read
entries, none unread**: 471 image-qualified retail functions, 34 previously
reviewed local/header/modern-support bodies, and 17 new inline bodies. Its
`decision` column preserves the source-reading findings; `retained_helpers`,
`extraction_verdict`, `strict_before` and `strict_after` record the application.
The strict columns come from the complete objdiff project. They do not use
`--loose`; support bodies without a separate retail binding use `n/a`.
Macros are counted as definitions in the catalogue, rather than
as additional function bodies in the checklist.

## Retained source operations

| Family | Retained boundary |
| --- | --- |
| Input and equipment | `PAD_PRESSED` keeps two short-circuited tests; `PLAYER_ITEM_IS_EQUIPPED` retains the seven ordered inventory slots. |
| Menu selection | `menu_list_previous` and `menu_list_next` own the byte selection/cursor/scroll state; empty-list handling, sound and resource loads remain callers. |
| Distance and angles | Signed shifted 3D length, signed-halfword angle-error folding, and actor-to-player bearing. The bearing keeps its particular Z subtraction. |
| Radial damage | `radial_damage_attenuated_scale` keeps u16 ratio/weight truncations and u32 Q12 products. Full-strength bypass and component routing remain callers. |
| Camera and audio | Q4 pose publication/advancement, descending voice reset, ring/keyoff/keyon publication, and ordered sequence stop/close. Image-specific policies remain outside. |
| CD and resource formats | Three location bytes with track untouched, signed three-character decimal path writes, and length-prefixed stream advancement. Two-digit and noncontiguous path fields remain explicit. |
| TMD format | Typed object/packet boundaries, encoded packet body length and prepared-vertex byte offsets. Packet header/body advancement remains split at its original sites. |
| Map and interaction | FLOOR-or-STEP predicate, typed u8 tile plus s16 local-coordinate conversion, XZ yaw probes, and X/Z/floor-Y player snapshots. |
| Actor initialization | Local home-position initialization and ten local bounded player-distance calls. Activation ranges and lifecycle policy remain callers. |
| Rendering and lighting | Floor-item byte-wrapped animation advance, transition RGB step, and a local current-color-matrix snapshot/blend. |
| Menu drawing | Nine preview-light coefficients, glyph packet setup, list tiles, reflected backdrop tiles, reverse background insertion, and six current/slash/maximum draws. |
| Saves and PSX loader | Two-cycle new-card handshake, six-field summary copy/equality, and the ordered overlay launch sequence with caller-owned EXEC/argument storage. |

Further consumers found while applying the review are included. Placement
arithmetic now covers seven GAME/OPEN TUs, actor bearing spans both actor
modules, and background insertion includes the map viewer. The earlier
52 keep decisions remain in force. Related policy code was not combined
merely because it occupied nearby addresses.

All macros require the simple, side-effect-free expressions documented at
their definitions. Several deliberately reread a global or argument after an
SDK call. Sequenced expression macros preserve those reads without adding a
caller block. The pinned SDK's `setXYWH` and `setUVWH` are themselves comma
expressions; composed menu macros retain those authentic APIs. No library
body, compiler flag, ownership inventory, assembly, artificial local or
volatile carrier was added to obtain a score.

These names and forms are reconstruction hypotheses. Neither matching bytes
nor the unit's pinned compiler profile establishes original source spelling
or historical compiler attribution.

## Trial findings and rejected scope

Every focused trial was rebuilt using its unit's pinned profile. Retail
instructions, delay slots, call targets and ordered referents were compared
from the first real divergence. The catalogue records individual scores and
symptoms; ignored trial objects and disassembly differences remain under
`build/helper-extraction/`.

Two complete recommendations were rejected:

- **Bounded XZ distance:** moving the arithmetic and return into an inline
  changed shifted-value destinations, multiply/sum order and the result
  branch tail. Actor/player also interleave the X shift with their vertical
  interval checks. Their original bounds and arithmetic remain explicit.
- **Save I/O retry loops:** the inline captured a buffer pointer before event
  clearing instead of reloading it at each I/O call, and changed saved
  registers and counters. The five loops retain their buffer evaluation and
  caller-owned close/error policy. A macro exposing both result and counter
  would make the interface harder to read.

Other candidates retained a smaller operation after a broader trial failed:
radial attenuation leaves its full-strength bypass outside; preview drawing
shares the light initializer while preserving matrix ownership; text glyphs
share packet setup while retaining atlas lookup and commit order. Menu
vital fractions were extracted, while class-tier and status-icon mutation
blocks were restored after their inline trials changed instructions.

The observed differences include loss of shared address bases, earlier
argument evaluation, different register/frame use and reordered stores.
They are **unattributed codegen residues**, not a compiler-wall taxonomy.
No register permutations or backend-mechanism claims were used.

## Verification and per-function results

A fresh full `kf analyze` rebuilt the source and refreshed comparisons.
**All 101 source/image comparison objects are byte-identical to the baseline
saved after the initial four macros**, including symbol and relocation tables.
All 471 retail function scores are unchanged. No exact-count movement or
new banking is claimed.

The checklist identifies 112 direct helper consumers: 108 are strict 100%;
four retain their existing full-project results:

| GAME function | Before and after |
| --- | ---: |
| `effect_projectile_update_2d` | 99.93421% |
| `menu_draw_item_detail` | 97.80875% |
| `player_warp_to_floor_entry` | 99.87654% |
| `player_update` | 99.96948% |

All other preexisting residues, including those in indirect/local-helper
consumers, are recorded individually in the checklist. Isolated trial scores
can differ from complete-project scores: the warp entry reports 100% in its
isolated comparison but retains 99.87654% in the complete project. The latter
is the handoff verdict. Identical complete objects establish nonregression;
isolated scores are not used to promote a function.

`kf build` succeeds for **PSX.EXE, GAME.EXE and OPEN.EXE** through the native
Psy-Q executable chain. All 101 source/image variants pass modern type
checking (100 in the full run, then the save TU after adding its missing
forward declaration). All **798 repository tests pass with retail inputs and
no skips**. Repository lint and `git diff --check` pass. The graphics-owner
control was updated for the shared vertex macro while retaining its
linked-byte and referent assertions. `nix flake check -L` also passes; its
isolated test run skips the 143 tests that require local retail inputs.

The analysis command still exits nonzero on the preexisting data-section and
ownership gates. Target relink remains PSX 1/1, GAME 58/65 and OPEN 35/39.
A successful native build does not claim those gates or whole-image matching
are closed. Retail inputs were hash-validated with `kf init`; the game was
not run. Generated reports and retail paths are not committed.

## Reconciliation

The checklist's retail identity set must equal the manifest's
`(source, image, VA, symbol)` bindings, with no duplicate retail row. Every
observed candidate ID must resolve to exactly one catalogue row, and every
row must have a reading decision. New local/header functions require a body
reading and a separate checklist row; generated enum instances share their
source definition's row. The completed application reconciles 471 retail
bindings, 522 readings, 90 candidate families, 17 new inline definitions and
22 retained macro definitions.

## Stack integration

Review in order: [PR #2](https://github.com/sushi-shi/kings-field-decomp/pull/2)
(native referents), [PR #3](https://github.com/sushi-shi/kings-field-decomp/pull/3)
(shared enums), then [PR #4](https://github.com/sushi-shi/kings-field-decomp/pull/4)
(these helpers). Each PR targets the preceding branch; the first targets master.

Ten source files needed explicit conflict resolution. Menu input/navigation
helpers retain the shared result and trade types. The floor-item animation
helper uses the canonical packed appearance mask. The distance helper retains
the common orbit sound range; the new-card helper retains `SAVE_STATUS_OK`.
Resource stream helpers use the shared OPEN placement height, and the overlay
launch macro keeps the direct shared-mode assignment. A whole-source check
finds no retired enum aliases reintroduced by clean merges or helper bodies.

Fresh comparison of PR #3 against #2 and PR #4 against #3 finds all 101 objects
unchanged at each step: allocated bytes, BSS/COMMON extents, alignments, symbol
values/sizes and ordered relocations. All three native executable hashes are
identical across the stack. All 101 modern type checks pass in both source
refactor layers; the complete enum ledger still covers every current member.
The combined comparison has 458/471 game functions exact and 13/13 vendored
verification functions exact. `player_warp_to_floor_entry` is now exact due to
PR #2; the standalone checklist above preserves its original before/after
snapshot. Existing data ownership and placement gates remain open.

The first PR passes its independent 800-test repository suite; the combined
stack passes 808 tests. Both local runs skip nine tests. Ruff, whitespace
checks and `nix flake check -L` pass at the first and final layers; the isolated
flake suites skip 143 checks requiring local retail/build artifacts.
