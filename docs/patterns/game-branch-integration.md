# GAME hypothesis campaign integration

Integration of `codex/game-362-exact` at `ce690006` into master, starting
from `5491cad3`, on 2026-09-08. The branch contains 33 commits beyond its
common ancestor. Its earlier exact reconstructions already have corresponding
source corrections on master; this integration preserves the newer shared
types, SDK subobject accesses and exact notification helper. Historical
experiment sections from both lines of work are retained. Their scores and
test counts describe their stated snapshots, not the current whole tree.

## Source plan and verdicts

Refresh retail hashes and six image-qualified evidence views for the four
remaining source changes. Recompile master and integrated forms independently,
resolve complete instruction words and ordered references against the curated
owners, and compare every sibling. Preserve current typed glyph and coordinate
views. Do not import the old branch's baseline ledger or regress newer exact
source to an older representation.

| GAME function | VA / retail bytes | Before strict % | Integrated strict % |
| --- | --- | ---: | ---: |
| `menu_status_panel` | `8002430c` / 1692 | 81.900710 | 99.962170 |
| `menu_draw_item_name_frame` | `800292f8` / 1976 | 95.376520 | 99.570850 |
| `map_world_state_persist` | `80035b5c` / 696 | 96.005745 | 97.528730 |
| `player_update` | `80018880` / 6684 | 96.903650 | 96.939560 |

The status panel's ordinary reflected-quad helper reproduces 407 of 423
retail words. All twenty calls and 105 ordered address pairs agree. Its
sixteen differences are confined to frame allocation, saved-register offsets,
restoration and release: source 48 bytes versus retail 112.

The item-name frame keeps the current `string.glyphs` object, addresses the
complete `MenuGlyphRow`, and restores XY corner assignment order. Exactly
479 of 494 retail words agree. The remaining twelve frame words and three
reordered loads reproduce the previously documented residue. All fifteen
sibling bodies are unchanged, including fourteen exact functions.

The world-state writer's sentinel for-loop restores the first 356 retail
bytes. Its seven ordered address pairs and zero calls agree with retail;
all five exact siblings remain unchanged. The body still has 700 bytes
versus retail 696. The complete five-floor persistence oracle passes.

The promoted item-domain decode changes exactly one candidate word at
`+0xf8`: `andi a0,a0,255` becomes the retail call-delay NOP. All other
1657 candidate words and every ordered call/address target are unchanged.
The body remains 6632 bytes and retains its separate frame, CFG and
instruction-order residues. The callee's byte-domain interface is preserved.

The old renderer's separate effect view and per-pass loop-variable declarations
are superseded by master's shared typed effect owner and stronger current
reconstruction. Old scalar coordinate casts are superseded by the shared
position subobject. No such older representation is restored. The correction
that an unused source object cannot be inferred from a frame gap is retained.

## Verification

Fresh canonical compilations reproduce the full-build objects, including their
section contents and relocations. Their program sections also reproduce the
independent integration candidates; disposable source paths account for the
expected debug metadata differences. Each delinked target independently
reproduces its complete retail function bytes. No partial result is banked.

Ruff and all 720 repository tests pass (113.065 seconds), as does
`git diff --check`. Full `kf build` retains GAME **339/362 exact**, OPEN
**106/108**, and PSX **1/1**, with zero artifact failures. GAME aggregate
similarity is 99.623%; this is not closure. Data matches remain GAME 12/42,
OPEN 3/20 and PSX 0/1; target relinks remain 75/77, 34/38 and 1/1. Existing
data ownership and section-placement checks still make the build nonzero.
Concurrent native executable-toolchain changes are outside this integration.

Generated evidence, baseline/candidate/canonical objects, resolved words,
strict scores and verification logs are under `build/branch-integration/`.
The generated per-function trial report covers all 362 game functions. It
counts 462 saved JSON entries, 315 for the 23 functions still partial.
Entries include baselines, repeats and compilation failures; manual edits,
trace controls and independent verification compiles are not counted. A zero
therefore means no entry in the scanned reports, not that a function was
never attempted. Generated artifacts remain uncommitted.
