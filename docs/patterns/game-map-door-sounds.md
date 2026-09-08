# GAME map-object door-sound selection

## Function Match Plan

At `f954711`, GAME `80031cc8 map_object_pool_update`, 3096 bytes in
`game.map_object`, is strict 98.966410%. All six semantic views, current
source, main-loop caller, adjoining clear-link/audio-initializer functions,
audio callee, shared types, prior family evidence and source history were
reviewed. The pinned unit profile remains unchanged.

The no-argument updater scans 190 typed 44-byte records. Preserve byte
actions and IDs, halfword timers and angles, word positions, all action
branches, 35 direct calls, 23 numeric address materializations and their
ordered referents. Its 96-byte frame, 64 conditional branches, return and
owned delay slots remain unchanged. The navigator's action-table jump is
unresolved; its 99 candidate rows are not promoted by this experiment.
The source retains the existing 396-byte RODATA claim. Known CFG edges
agree, without implying complete indirect reachability.

This is game policy. The separately attributed SDK rand/rsin/rcos routines
remain external providers. There are no strings or new library bodies.
The two door-sound paths select sound 1 for object ID 119 and sound 7
otherwise, then play at the object's position with volume 127. Retail
loads and compares the byte ID before materializing the fallback address;
the candidate materializes it first. All addresses and calls already agree.

Test four ordinary source forms independently at opening and closing:
pointer if/else, direct calls in each branch, conditional call argument,
and default pointer with conditional override. This gives sixteen JSON
states. Preserve all computations and ownership; do not introduce a value
carrier or attribute the instruction order to a historical compiler.
Require canonical strict comparison, independently resolved words and
references, unchanged siblings, full build and repository checks before
banking and committing to master.

## Result

Only direct calls at both sites reach **100%**. Changing either site alone
reaches 99.483210%; all combinations without direct calls remain at
98.966410%. The compiler merges each branch pair into the single retail
audio call. The retained source removes the now-unused sound pointer.
Neither the call count nor runtime behavior changes.

Fresh `kf try` gives seven identical listings in the eight-function unit;
canonical `kf match` confirms seven strict-exact functions. The remaining
`map_object_spawn_effect` stays 94.504950%. Independent numeric relocation
resolution checks all 1222 source words, 52 calls and 32 address
materializations: the seven exact functions equal retail, and every sibling
is identical to a fresh pre-change compile. The updater's 774 words equal
retail completely. Exactly eight words change, at offsets 0x104–0x110 and
0x198–0x1a4. The RODATA bytes are unchanged. Moving sound 7 by four bytes
changes three words and fails the raw control.

Full `kf build` reports GAME **334/362**, OPEN **106/108**, PSX **1/1**,
with all thirteen vendored source controls exact. It exits with existing
data ownership/placement and target-relink failures: GAME data 10/41,
OPEN 3/19, PSX 0/1; target relinks 75/77, 34/38 and 1/1. Artifact failures
remain zero. No comparison rule, compiler option or tooling is changed.

Ruff, whitespace checks and all 713 repository tests pass (144.948 seconds,
nine optional skips). Banking selects the newly exact updater and six
already-exact siblings under the current unit input hash. The partial
spawn function is not banked.

Generated results remain under
`build/hypotheses/20260908-162609-game-map_object-map_object_pool_update`;
the raw audit is `build/pool-raw-verification.json`. They are not committed.
