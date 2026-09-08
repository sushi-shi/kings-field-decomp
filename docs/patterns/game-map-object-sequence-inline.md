# GAME effect-slot sequence helper boundary

## Function Match Plan

GAME `80031834`, 404 retail bytes, `map_object_spawn_effect` is
94.504950% at b9f0b5f. Refresh all six semantic views, both argument paths
in `actor_update_current_action`, adjacent allocator/action/debris bodies,
source history and prior twelve spawn controls before editing. The caller
supplies byte kind/ID, a word-coordinate position and negative half collision
height. Five proven calls, two validated global address pairs, three internal
jumps, ten branches including division guards and one return retain their
ordered targets and delay slots. There are no strings or indirect transfers.
Group starts 170/180, capacity ten, halfword sequence accesses, signed division
by 2000 and action thresholds 43/48/65 remain fixed. This is game policy;
`rand` is independently identified as Sony LIBAPI C47 by its archive signature.

The exact debris constructor repeats acquisition followed by halfword
sequence publication. Test the existing body against two static inline helper
boundaries: acquire/publish from a selected first index and sequence pointer,
and selection plus acquire/publish from byte kind. Preserve initialization
order and all sibling functions. This is a source-boundary hypothesis, not
proof of an original helper. The first raw mismatch is saved ra at +32 rather
than +36 in the 40-byte frame, at function offset +4.

## Verdict

All three JSON states compile to 400 bytes and strict 94.504950%; none is
exact. The two helpers produce identical linked instructions. Relative to
baseline they only replace the pointer-copy/store pair at +64/+68:
`move s0,v0; sh a0,34(s0)` becomes `sh a0,34(v0); move s0,v0`.
Retail instead retains sequence in s0 and object in s1 and publishes via
`sh v1,34(s1)` at +68. The original first mismatch remains +4.
All five call targets and both global address pairs are unchanged. Independent
fresh compiles and relocation resolution confirm all seven siblings unchanged,
including the six exact controls. No source, identity or bank change is kept.

Generated results: `build/hypotheses/20260908-204437-game-map_object-map_object_spawn_effect`.
The JSON manifest, independent compile script and raw verifier are
`build/spawn-inline-hypotheses.json`, `build/spawn-inline-inspect.py` and
`build/verify-spawn-inline.py`. These generated artifacts are not committed.

Ruff, all 713 repository tests (101.525 seconds) and `git diff --check`
pass. Full `kf build` retains GAME 337/362 exact and 99.182% aggregate
similarity, with zero artifact failures. Data matches remain GAME 11/41,
OPEN 3/19 and PSX 0/1; target relink remains 75/77, 34/38 and 1/1.
The full build exits nonzero on the existing incomplete closure checks.

## Post-sequence initialization boundaries

Function Match Plan at `62f2ebe`: refresh hashes and all six GAME views for
`80031834`/404 bytes, inspect its complete raw body, both actor argument paths,
allocator/action/debris siblings, shared 44-byte object and position layouts,
source history and earlier acquisition/publication trials. The current
candidate remains 400 bytes / 94.504950%. Preserve the five direct calls, two
global sequence-address pairs, three internal jumps, signed division guards,
40-byte frame and final velocity clear on every object-ID path. The custom
constructor remains game code; rand retains its LIBAPI signature attribution.

The effect and debris constructors share coordinate-to-cell and rotation
initialization. Test three static inline boundaries after sequence publication:
coordinates plus cells; that region plus rotation; and all initialization
from object ID through action-band selection. Helpers receive the existing
typed object/position pointers and word Y offset; the full initializer also
receives the existing object-ID type. Acquisition, sequence postincrement and
the common final velocity clear remain in the caller. No input width, field,
operation, literal, store order, owner or SDK boundary changes.

All four JSON states, including canonical, compile to exactly the same
400-byte body, five ordered calls and two global address pairs at 94.504950%.
Independent disposable builds compare all 100 linked words, not just scores.
The first retail difference remains +4: saved ra at sp+32 rather than +36,
followed by the missing s4 save and sequence/object register reuse. None of
these initialization scopes explains the 404-byte retail body, so reject all
three and retain canonical source unchanged. Nothing is banked.

Every one of the seven sibling functions reproduces its complete raw retail
body and ordered references in all four independent builds. This includes
the now-exact object-pool updater. All manifest baselines match the current
source. Results are in
`build/hypotheses/20260908-215303-game-map_object-map_object_spawn_effect`;
the manifest, six-view dossier and independently compiled objects use the
`spawn-init-inline` prefix under `build/`.

Full `kf build` retains GAME 337/362 exact (99.428% aggregate), with
zero artifact failures. Data matches remain GAME 11/41, OPEN 3/19 and
PSX 0/1; target relink remains 75/77, 34/38 and 1/1. The build exits
nonzero on the existing incomplete closure checks. Production source,
configuration and tests are unchanged; the preceding 713-test and Ruff
results still cover them. `git diff --check` passes for this record.
