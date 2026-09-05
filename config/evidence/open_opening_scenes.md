# OPEN opening-scene ownership pilot

## Function Match Plan

Image: `OPEN.EXE`. Baseline: `c3bfa98`. Consolidate six granular source units
into `open.opening_scenes`, retaining the seven complete C bodies, all seventeen
data definitions, their types/linkage/initializers, and the one literal claim.
Keep `probe-gcc257-o2-g0`; this is neither historical compiler attribution nor
an attempt to eliminate the existing non-exact instruction residues.

Before editing, the image-qualified `addr`, `disasm --blocks`, both `xref`
directions, `strings`, and `match` queries were read for every function. The
dispatcher, adjacent camera/fade routines, four transition call sites, every
owned datum's xrefs, source history and the existing semantic dossiers were
also inspected. All seven functions have no string references. Their existing
Psy-Q/FID negative controls remain applicable: resource/camera/entity/scene
policy is game code, while SDK graphics, audio, trigonometry and timing calls
remain external providers. None is present in `functions_vendored.tsv`.

### Per-function snapshots

Every extent includes the return delay slot. Calls below are decoded direct
calls; reference totals are curated relocation rows, not a count of pointers.
The pre-existing detailed CFG, signature, width and referent evidence is in
the linked dossiers; bodies and signatures are not being re-invented here.

| OPEN VA / extent | Function | Before | Evidence and retained source hypothesis |
| --- | --- | --- | --- |
| `80014268 / 174` | `opening_scene0_run` | 100% | `void(void)`; sole dispatcher call at `800157d0`; 11 calls, 23 references, two internal jumps; entity IDs 11/12, signed yaw ±40, seventeen 28-byte camera points, volume 100 and signed fade state; 32-byte frame and stack-pop return. [Dossier](open_semantic_opening_scene0.tsv). |
| `800143dc / 180` | `opening_scene1_draw_fade` | 100% | `void(u8)`; both caller sites mask shade to eight bits; two 40-byte SDK `POLY_FT4` packets; eight calls, eleven references, linear frame construction and presentation; 32-byte frame and stack-pop return. [Dossier](open_semantic_opening_scene1.tsv). |
| `8001455c / ac` | `opening_scene1_run` | 100% | `void(void)` from dispatcher `80015834`; eight calls, nine references; shade steps ±4, fade bound 129, sequence stop 600 and input/time exit 1000; 32-byte frame. [Dossier](open_semantic_opening_scene1.tsv). |
| `80014608 / 1fc` | `opening_entity_transition` | 99.921260% | `void(s16, const VECTOR *)`; all four calls in scene 3/ending establish modes 0–3 and stack VECTOR arguments; four 40-byte entities starting at index 24; 48-frame loop; two calls, three internal jumps, three address pairs; genuine snapshot stores at stack 16/24/20 with load delays. Only ten frame instructions differ: retail 56 bytes, probe 48. Preserve that residue without invented padding. [Dossier](open_semantic_opening_entity_transition.tsv). |
| `80014804 / 330` | `opening_scene3_run` | 99.931370% | `void(void)` from dispatcher `80015870`; 27 calls and 47 references; two texture/CLUT pairs, three-point camera path, signed overlay clipping, VECTOR transition position and signed fade counters. Fourteen frame immediates differ: retail 112 bytes, probe 96. [Dossier](open_semantic_opening_scene3.tsv). |
| `80014b34 / 2f4` | `opening_ending_scene_run` | 100% | `void(void)` from dispatcher `80015888`; 20 calls, 45 references and two internal jumps; nine-point camera path, transition modes 3 then 2, signed yaw/brightness/blend, authentic DRAWENV fields; 72-byte frame and complete epilogue. [Dossier](open_semantic_opening_ending_scene.tsv). |
| `80014e28 / 798` | `opening_ending_scroll_run` | 97.129630% | `void(void)` from dispatcher `80015890` with nop slot; 40 calls, eight internal jumps and 31 address pairs; nine texture/CLUT pairs and panels, six CVECTOR colors, signed short states, 264-byte frame. The infinite loop jumps back at `80015588`; the unreachable 48-byte epilogue still owns `jr` at `800155b8` and its stack-pop slot. Preserve lighting-join/instruction-order/temp residue. [Dossier](open_semantic_ending_scroll.tsv). |

## Ownership evidence and limits

The functions cover `80014268..800155c0` without a gap. More importantly,
`opening_run` directly dispatches the five scenes, scene 1 exclusively calls
its fade helper, and scene 3/ending share every call to the entity transition.
They use the same resource, camera, entity, render and audio interfaces and
the same probe profile. This supports a related reconstruction module, not
proof of the original source filename or its exact boundaries. The neighboring
camera interpolation module and fade helper remain separate.

The four serialized camera paths form a gap-free 896-byte run:

| Retail interval | Existing objects | Bytes |
| --- | --- | --- |
| `800354f4..80035874` | scene 0, scene 3, ending, ending-scroll camera paths | 896 |
| `80035874..800358d0` | scene-0 SoundRef, scene-3 rectangles, scroll panels | 92 |
| `80037284..800372c0` | scene-3 UV/color, scroll backgrounds/colors/UV | 60 |

There are seventeen DATA claims, totaling 1048 bytes, and a separate 32-byte
RODATA claim at `80012000` for the scroll's local MATRIX initializer. Seven
global and ten local data bindings are retained. Existing identity-inventory
owners still describe the logical scene subfamilies; source claims and the
manifest identify the consolidated compilation owner. No identity is renamed.

The initialized-data runs are **not** contiguous with each other. Consolidation
must not insert the intervening retail bytes as synthetic padding or force a
new section/profile solely to pass placement. A small-data interpretation is
a hypothesis requiring independent section/compiler/linker evidence. The
current single `.data` model should continue to reject inconsistent bases.

Two xref caveats were discovered in the ownership audit. Both inventory rows
remain `candidate`, though the current conservative rules display their
decoded address construction as `validated`:

- SDK `sin_1`, at `8002f104/8002f108`, constructs `800357e8`, currently labeled
  `opening_ending_camera_path+0xc4`. Its preceding unsigned range test admits
  arguments 2048–3071 and its delay slot shifts the argument left by one. The
  actual halfword reads are therefore `800367e8..80036fe6`, the same table
  accessed by the other three quadrants—not camera data. This is a biased
  table base needing a proper referent/addend model in a separate SDK campaign.
- Startup at `8001aa7c/8001aa80` loads `$gp = 80037284`, then tail-jumps to main.
  Equality with the UV array's address does not prove a UV read or its ownership.
  GP/section-base modeling is unresolved. Do not infer private linkage from
  a convenient current xref count.

Neither row is silently promoted, deleted or redirected in this campaign.
Their raw constants remain evidence for subsequent reachability work.

## Verification plan

Rebuild the merged object and callers; compare every function score by
image/name across all three images, not by its former TU key. Audit data bytes,
extents and bindings per named object and all ordered code referents. Adapt
the transition/scroll controls to function spans in a multi-function TU without
weakening their exact residue/retail assertions. Add controls for the complete
seven-function run, seventeen owners and the two real data intervals.

Run focused matches, all repository tests, lint, diff checks and the full
default build. Strict data/placement failures remain failures; no data-only
score is a banking criterion. Record each final function verdict and the
changed unit-level denominators after verification.

## Final verdicts

The kept change consolidates six C files and four narrow headers into one
scene module/header, retaining the scene-zero renderer interface separately.
All seven function bodies and seventeen data definitions are verbatim copies
of the baseline, merely ordered by their retail claims. No vendor source,
inventory address, DATA extent, field type, compiler profile or linkage changes.

| Function | Final strict objdiff | Verdict |
| --- | --- | --- |
| `opening_scene0_run` | 100% | Exact preserved. |
| `opening_scene1_draw_fade` | 100% | Exact preserved. |
| `opening_scene1_run` | 100% | Exact preserved. |
| `opening_entity_transition` | 99.921260% | Same ten frame-immediate differences; not exact. |
| `opening_scene3_run` | 99.931370% | Same fourteen frame-immediate differences; not exact. |
| `opening_ending_scene_run` | 100% | Exact preserved. |
| `opening_ending_scroll_run` | 97.129630% | Same instruction/temp/lighting-join residue; not exact. |

The raw pre/post comparison resolves each relocation to the same existing
owner plus offset, without blanking relocated words: all seven compiled
bodies and all seven delinked bodies retain identical instructions and ordered
relocation kinds/targets after that rebasing. The source/target ELF row counts
are respectively 33/33, 14/14, 10/10, 11/11, 66/66, 68/68 and 111/110. The
scroll's extra compiled jump is pre-existing, not introduced by consolidation.
This per-owner audit is explicitly **not** a whole-section linking success.

All 484 report rows retain their size and strict score, compared by image and
function identity rather than TU-relative symbol offset. All 354 exact game
functions remain exact. Outside the merged six units, 110 compiled objects
and all 111 delinked objects are byte-identical. The dispatcher's remaining
compiled-object change is solely `.debug_line`: entry line 46 becomes 43 after
the include cleanup; every allocated byte, symbol and relocation is identical.

The seventeen compiled and target initializers each equal their own retail
range, with seven global and ten local bindings preserved. The target `.data`
extent is 1048 bytes; the compiled section is 1056 bytes. Every claim in the
first 988-byte run implies section base `800354f4`, while every claim in the
last 60-byte run implies `80036ea8`. The placement gate correctly refuses both
as one section; no linker runs for this unit in the roundtrip gate.

Repository TUs: 117 → 112 (OPEN 41 → 36). Strict source data: 15/63 → 13/60,
because four data-owning units become one, absorbing two formerly passing
data units into the non-exact merged section. Target relink: 109/117 → 106/112;
three passing and three conflicting units become one conflicting unit. The
remaining six conflicts are GAME PAD and OPEN scenes, map-cell windows,
entity-render data, audio and PAD. These changing TU denominators are not a
claim of additional retail-byte closure or a regression in a function.

Focused raw-retail/ownership controls, all 485 local repository tests, Ruff,
`git diff --check` and `nix flake check -L` pass. The flake's separate sandbox
skips 54 controls needing local retail/oracle artifacts. The full default
`kf build` remains red on strict data, the six placement conflicts and the
unchanged reachability backlog (665 config-only ranges). No new exactness is
claimed or banked, and complete linked-image equality remains unproven.
