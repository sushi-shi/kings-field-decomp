# GAME audio initialization fill value

## Function Match Plan

GAME `800328e0 audio_initialize(void)` is 164 retail bytes in `game.audio`,
strict 95.121950% before this trial. Hash-identical retail, all six semantic
views, the sole call at `80014758`, neighboring function boundaries, shared
audio types, SDK declarations, provider evidence and source history were
inspected. The caller supplies no arguments and does not consume a result.
This is game-owned sequence/reverb/workspace policy; the eight sound-library
callees retain their independent LIBSND archive/header attribution. The ninth
call is the game memory allocator. No library body is reconstructed here.

Retail has three blocks, one conditional branch, one return, nine calls and
three validated address pairs, with no strings or unresolved indirect flow.
The 24-byte frame saves ra at +16 and restores sp in the return delay slot.
Keep sequence table capacity 2, one track, tick mode 1, volume 127, studio-C
reverb 4, depth 16 and allocation size `0x3000`. The ordered data targets are
`audio_sequence_table` (`80059738`), `audio_state+8` (`80095870`) and
`audio_state+16` (`80095878`). The final loop writes signed-halfword -1 to
voice IDs 9 through 0; its pointer decrement executes in the branch slot.
`audio_play_voice` consumes this inactive sentinel before key-off/replacement.

The baseline has the same extents, calls, referents and known CFG successors.
Its only listing difference swaps setup of the fill value and loop index:
retail `+78: li a0,-1; +7c: li v0,9`, candidate the reverse.

OPEN `80019ba4 audio_initialize` is an independent exact 184-byte control.
Its retail reset-loop setup and body are identical to GAME. Its existing C
names a signed-halfword `inactive_voice_id`, sets it before the index, and
uses it for every voice-slot store. That source was present in `e583b2b`.
OPEN's volume, reverb, buffer size and two post-loop state stores differ and
must not be transferred to GAME.

Test that same consumed fill-value lifetime in GAME, without changing the
ten-iteration loop, shared structure layout or any other statement. This is
a narrowly supported cross-overlay source hypothesis, not historical proof
of a local variable. Do not add dead values, volatile, assembly, barriers or
extra calls. Independently rebuild both the original and trial full unit;
audit complete linked words, ordered calls and numeric address targets for
all fifteen functions, preserving the fourteen exact siblings. Require strict
objdiff 100%, full build and repository verification before keeping/banking.

## Linked-byte verdict

The original and trial were independently compiled as complete units. The
original differs only at +`0x78` and +`0x7c`; the trial recovers all 41 retail
words, all nine ordered call targets and all three numeric address targets.
The fourteen siblings retain their complete original words and references,
each raw-exact against its own retail extent. All six functions of the
separately compiled canonical OPEN audio unit remain raw-exact as well.

Retail body SHA-256:

- GAME: `ee3d7f5a4dc02830d621c082e24564f2a59646bb0e856427c67ea28f3334fd57`
- OPEN: `04cb761a249c1d0e60e0b31f02edf1e848903585d6882bfe41f33f5bc8f4f585`

Keep only the GAME fill-value local, initialized before the loop index and
consumed by the actual ten halfword stores. No ownership, SDK interface,
constant, control-flow or compiler-profile change is required. This supports
the shared source shape without attributing a historical optimizer mechanism.

## Verification

The canonical source was freshly compiled and independently audited as well
as the temporary trial. Strict objdiff is now 100% for all fifteen GAME audio
functions. Comparing all 484 report rows with the pre-trial snapshot changes
only GAME `800328e0`, from 95.121950% to 100%; there are no artifact failures.
Eligible exact counts are GAME 313/362 and OPEN 98/108 (PSX remains 1/1).

Ruff, all 680 repository tests and `git diff --check` pass. The full `kf build`
was run and remains non-green for the existing data-section placement/extent,
target-relink and known-reference ownership gaps. Data-owning matches remain
PSX 0/1, GAME 9/42 and OPEN 2/19; this function result is not whole-image
closure. Unrelated concurrent map-object source/header changes were neither
edited nor staged by this campaign.

Source and evidence are committed in `ed926a0`. The first targeted banking
attempt correctly refused the concurrent unstaged map-object inputs. After
their independent commit `c503559`, ordinary
`kf bank --function game:0x800328e0` accepted exactly this one strict-100% row;
no dirty override or unrelated baseline refresh was used.
