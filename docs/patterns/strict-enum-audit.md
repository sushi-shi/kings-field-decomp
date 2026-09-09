# Strict enum domain audit

## Function Match Plan

Baseline: `9e5985ab`, isolated branch `fix/strict-enum-domains`.
This is a source-wide type-propagation campaign, not new function reconstruction.
The campaign follows existing shared structures, field uses and confirmed calls.
All PSX/GAME/OPEN addresses retain their image identity.

1. Inspect every project structure, field, function declaration and definition.
   Classify finite selectors/states, counts and dimensions, packed flags,
   action-dependent storage, encoded input, and unresolved fields.
2. Use `KF_ENUM_BEGIN` for each closed semantic domain. Store the domain in
   fields and carry it through parameters, returns, locals and arrays. Keep
   literal dimensions and numeric quantities as constants. Consolidate values
   belonging to the same field in its owner header.
3. Preserve the C ABI with the existing storage/parameter macros. Clang's
   C++20 view must reject integer and foreign-enum assignments/arguments.
   Do not introduce integer assignment operators or generic enum conversions.
4. Model counted state machines explicitly: their named phases share one
   type; arithmetic is a documented counter boundary, not permission for
   implicit integer assignment. Do not invent separate enums for each action
   that reuses the same field.
5. Before edits, capture each source-owned function's retail address dossier,
   disassembly/CFG, incoming/outgoing references, strings and match status in
   `build/enum-audit/before/`, plus source history and baseline object hashes.
   Review the relevant records for each changed family, including adjacency,
   call-site widths, delay slots and constants. Existing game owners are not
   reclassified as library progress; SDK compatibility is a separate concern.
6. Rebuild affected objects and compare their sections and relocations with
   the pre-edit objects, then run full `kf build`, strict Clang, existing tests,
   lint and whitespace checks. No new enum tests or banking are needed.

The initial full build has pre-existing data ownership/relocation failures in
all three images. Initial strict checking passes 67/112 source/image variants;
45 fail, chiefly at K&R SDK declarations and C pointer conversions. Preserve
this baseline when reporting verification; a failed baseline is not a pass.

## Sibling comparison

`gruntz/include/Enums.h` and `homm2/homm2-buka/include/Ints.h` use scoped enums
for the modern view and ABI-compatible retail forms. KF already uses the same
principle. Coverage is the defect: anonymous constants and integer fields do
not participate. The sibling storage wrappers also accept raw storage integers;
KF's stricter enum-only storage wrapper is retained rather than copying that
escape hatch.

## Audit and verdicts

The field classifications and final verdicts below cover the complete checked source set.

## Coverage

The audit covers all 484 source-owned functions in 112 source/image variants
(111 unique C sources), the 125 checked aggregate layouts and their 854 fields,
and the remaining project headers/local aggregate definitions. The vendored
controller front ends are compatibility controls, not new game progress.

The campaign adds 26 shared enum declarations, including two replacements for
narrower duplicated domains. It changes 28 curated function signatures and 22
existing field declarations, and adds two typed views of the existing map-grid
layout. No addresses, extents, image ownership or relocation targets change.

| Owner / domain | Finding and retained model |
| --- | --- |
| Actor progress | `KfActorActionProgress` owns initialization, jump, collision, backoff, post-death, locked and completed values. Both the field and every named assignment retain that type. |
| Actor movement | Direction, collision policy and move result are distinct types propagated through the movement helpers and their callers. |
| Actor placement | The placement/runtime heading quadrant shares `KfActorHeadingQuadrant`; angle multiplication explicitly encodes it. |
| Player state | Update state, nearby spawn permission, full attack charge and map variant now have shared types. Damage ticks and recovery/death states share the actual update byte. |
| Effect phases | One `KfEffectPhase` owns the previously local projectile, ground branch, blast, spawner, hazard and floor-deformation phases, including helper limits. |
| Map object progress | Door/reveal ages and switch states share `KfMapObjectProgress`, including local copies and counter boundaries. Drop-source selection has its own type. |
| Animation | `KfAnimationClip` flows through actor definitions, actors, map events, effects, effect sprites and the vertex cache. The previous event-specific clip enum and repeated NONE/first-clip constants are removed. Clip meanings remain relative to the selected asset. |
| Display | GAME and OPEN use one `KfDisplayBuffer` domain, including the initial `0xff` value. The toggle retains the original equality expression in retail C. |
| Map grids | Collision kinds and one-based orientations have distinct byte types. Resource word-copy views retain the complete original grid. Attribute IDs, heights and occupancy bitfields remain numeric. |
| Items and magic | Container item arrays retain `KfItemId`. `KfMagicId` unifies utility and selected-spell record identities. The signed utility-panel result explicitly distinguishes spell outcomes and cancellation/pending controls. |
| Menu APIs | Padding reuses `KfFormatPaddingMode`; shop IDs remain typed through list confirmation. Texture IDs, model-allocation state, system actions and system results are typed. Modern declarations constrain mixed preview arguments to item or magic IDs and variadic menu arguments to item or shop IDs. |
| Audio / loading | Spatial playback returns and the effect's stored playback flag share `KfAudioPlaybackResult`. CD/menu load results share `KfResourceLoadResult`, preserving signed/unsigned retail return widths. |
| Save records | The icon type is typed; save-status values 9/10 and no-space result 3 use the owning domains instead of integer decodes. |

### Integer fields and boundaries

Every remaining integer field/parameter was classified by its uses. Numeric
quantities include coordinates, angles, interpolation phases, dimensions,
counts, row indexes, resource-relative indexes, lengths, speeds, damage,
prices, probabilities and countdowns. A bounded integer range alone does not
make such a quantity a semantic enum. In particular, weapon/animation phases
are fixed-point interpolation values, while the newly typed progress bytes
also carry named control states.

Packed words are retained for GPU commands, texture-page/CLUT coordinates,
controller masks, collision result payloads, actor/effect target flags, status
effect combinations and aligned resource/save copies. C predicate helpers
return truth values; SDK results and callback signatures retain their actual
API types. Generic menu-root/list results combine row/item payloads with signed
control values; conversion to a narrower caller domain is explicit.

Unknown fields remain explicitly opaque. Actor action parameters retain
kind-dependent numeric/pointer payloads; there is no invented enum for an
unresolved payload. The effect constructor uses eight typed checking argument
groups for its resolved variadic layouts, as recorded below. Encoded placement model
IDs that are interpreted differently by GAME and OPEN stay at the resource
boundary. No raw integer assignment operator was added to the enum storage
wrapper.

`KF_ENUM_COUNTER` is opt-in for the four progress types that retail increments
or decrements. It preserves the domain and original storage width. It does not
make integer or foreign-enum assignment valid. Explicit encode/decode operations
remain at table indexes, packed/serialized input, numeric arithmetic and
cross-domain dispatch boundaries; they are not runtime validators.

### Strict-check compatibility

Release 2.5's K&R SDK declarations are zero-argument declarations in C++.
Modern-only signatures supply the used GPU/libc/pad argument lists; the retail
headers remain authoritative for C. Missing direct SDK includes and the
exported `CdReadSync` declaration are supplied. C allocation/upload boundaries
now spell their pointer conversions. The six-byte opening path uses an exact
byte initializer because C++ rejects C's permitted omitted string terminator.
The two vendored pad stubs spell ignored arguments only in the modern view.

Mixed/variadic menu overloads belong to the strict checking view. Their owning
implementation sees its actual numeric/variadic ABI declaration; callers see
only the supported enum argument domains. This view is for syntax checking,
not a separately linked modern executable.

The inventory reader includes the shared animation and load-result headers.
Existing signature/layout assertions and final-signature evidence were updated
to the new types; no new tests were added.

## Verification and final verdict

All 112 source/image variants pass `kf check-types`, compared with 67 at the
baseline. All 116 preserved objects have identical allocated section contents,
section sizes/alignment and ordered relocation offsets/types/target symbols.
This comparison includes instructions, constants, delay slots, literal data,
and the vendored negative controls.

Each of the 484 functions has a before dossier under
`build/enum-audit/before/` and a final verdict in
`build/enum-audit/final-verdicts.tsv`. The object comparison is recorded in
`build/enum-audit/object-comparison.json`. These local evidence products are
not committed. Focused compiler/listing comparisons cover actor behavior,
effect dispatch, map objects, menu runtime/panels, animation cache and OPEN
spatial audio; the full objdiff report supplies the strict scores.

| Image | Game functions | Exact before and after | Verdict |
| --- | ---: | ---: | --- |
| PSX.EXE | 1 | 1 | Identical |
| GAME.EXE | 362 | 339 | Identical; existing residues retained |
| OPEN.EXE | 108 | 106 | Identical; existing residues retained |
| Vendored controls | 13 | 13 | Identical; excluded from game progress |

There is no exact-count movement attributable to this campaign and nothing to
bank. The checked-in README score was stale at the starting commit; the actual
baseline rebuild already produced 446/471 exact game functions. Its generated
score refresh is excluded from this change.

Full `kf build` compiles the affected sources and completes comparisons, but
fails the same pre-existing verification conditions in all three images:
incomplete known-reference data ownership and data-section comparison; GAME
and OPEN also have existing target-relink/section-placement mismatches. No
banked function regresses. This is not a successful full-build closure.

`ruff check scripts tests` and `git diff --check` pass. The final existing local
suite passes: 713 tests, nine skips and 9,173 subtests. The full hermetic flake
suite also passes (722 tests, 139 environment-dependent skips). No new tests
or banking entries were introduced.

## Follow-up Function Match Plan: variadic enum arguments

The completion audit found that `effect_pool_construct` still exposed its
ellipsis to strict callers. Its sixth argument is a ground-branch enum for
kind 6; its seventh argument is a homing selector for kinds 20/24. The existing
enum decodes in the body correctly describe the retail argument slots but did
not constrain callers. The sound-request slots likewise represent a finite
choice. This contradicts complete argument propagation.

The follow-up keeps the original symbol and retail variadic call sequence.
Strict callers construct typed argument groups; retail macros expand those
groups to the original argument expressions. The implementation alone sees
its raw ellipsis declaration. Ground-branch role, homing target and sound
request remain enums in the checking groups; duration, scale, generation and
parent indexes remain numeric. Inspect all 33 callers and the constructor's
kind switch against the existing GAME dossiers, then rebuild all affected
units and require the original object/relocation comparison to remain equal.
No new functions, tests, addresses or banking entries are planned.

### Variadic follow-up verdict

All 33 effect-constructor call sites were inspected. Eight checking argument
groups cover their tails; the seven calls without optional arguments use the
fixed five-argument overload. No strict caller sees the ellipsis. The pool
implementation retains the raw declaration because its body reads the original
argument homes, decoding sound-request words into the same domain used by
callers. `KfEffectGroundBranchRole`, `KfEffectHomingMode` and the new
`KfEffectSoundRequest` constrain the appropriate group members; integers and
foreign enums cannot initialize those members implicitly. Numeric tail fields
remain numbers. The eight C++ checking classes have no retail layout claims.

The player weapon burst's word-sized attachment selector now retains the
homing domain through the local and argument group. Its actor-index and parity
calculations are explicit encoding boundaries. Some selected effect kinds
ignore tail slots or interpret the selector's 0/1 parity as a sound gate; the
original ordered argument expressions remain intact. The groups check member
types, not the runtime relationship between a kind value and its payload.

The final strict check passes 112/112 source/image variants. The full rebuild
again compares all 116 preserved objects identically, covering all 484
functions; the pre-existing full-build verification failures are unchanged.
No new tests, target functions, data claims, or banking entries were added.

## Requirement completion audit

| Requirement | Current source evidence and result |
| --- | --- |
| Closed fields declare enum domains | The 125 target aggregates and 854 inventory fields agree with the checked header layouts. The audit table above records the changed field families and the uses that distinguish remaining numeric, packed and opaque storage. The additional checking argument groups also declare their enum members directly. |
| Assignment accepts only the same enum type | `KF_ENUM_BEGIN` expands to `enum class`; `KfEnumStorage` has an enum-typed constructor and assignment operator, with private integer storage. Neither accepts integers or foreign enum domains implicitly. `KfActor.action_progress` directly declares `KfActorActionProgress`. |
| Invalid assignments fail statically | `kf check-types` selects every manifest C source/image variant, uses `-x c++ -std=gnu++20 -fsyntax-only`, and fails on compiler errors. There is no permissive enum mode or integer assignment overload. This is a language-level constraint rather than a runtime assertion. |
| Audit every structure/field/function/parameter without new tests | All 484 function dossiers, 854 target field records, header declarations, local structures and 33 effect-constructor call sites were inspected. Existing inventory expectations were updated; no new test cases were added. |
| Function arguments retain their enum domains | Public enum parameters use their domain or `KF_ENUM_PARAM`; mixed menu calls expose typed overloads. Resolved variadic effect tails now expose typed groups rather than an ellipsis. Serialized/ABI decoding remains explicit inside the owners. |
| Consolidation and sibling comparison | Actor progress and effect phases each have one owner domain; animation and magic domains span all consumers. The local Gruntz and HoMM2 headers were inspected; their integer-accepting storage assignment operators were deliberately not adopted. |
| Preserve matching and isolation | The dedicated worktree retains 116 byte-identical objects and ordered relocations, with 446/471 exact game functions and 13/13 vendored controls. Existing full-build failures are documented above. The primary worktree is not modified by this campaign. |
