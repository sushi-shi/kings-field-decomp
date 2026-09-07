# Ending scroll enable state and update cadence

## Function Match Plan

OPEN `opening_ending_scroll_run`, 0x80014e28 / 0x798, has two remaining
untyped control locals: the scrolling enable latch and the four-tick update
counter. Give each its own signed-halfword enum domain, keeping all stores,
predicates and the decrement/wrap sequence. Their scope is this function;
no field, call signature or external data identity changes.

The complete image-qualified retail dossier, CFG, caller/callee references,
data owners and source history were reviewed for the
[rectangle change](quad-rectangle-types.md). The stored object report records
97.952675%; subsequent source type edits remain unbuilt. This scene combines
game-owned resource, camera, entity and GPU composition policy; Sony providers
are separately attributed, and no vendored body is reconstructed.

Retail tests the latch before swapping the orange disk for the starfield at
0x8001531c..40, stores one to enable scrolling, and uses a positive comparison
at 0x80015474..84 before walking the panel rows. The only produced states are
zero and one. Name them waiting-for-disk and active; retain the existing signed
halfword source width and `> waiting` predicate rather than rewriting it.

The counter's zero test at 0x8001540c..14 controls starfield roll and brightness.
The zero/two tests at 0x8001549c..a8 control panel movement and the stop-triggered
sequence transition. At 0x80015560..7c the counter decrements, narrows to signed
halfword, compares with -1, then wraps to three. The numeric cycle is
`0 -> 3 -> 2 -> 1 -> 0`: starfield plus panels, hold after starfield, panels,
hold after panels. Name those four ticks and the transient wrap value.

Before scrolling becomes active the counter stays at zero: starfield roll
still executes every frame, while brightness waits for its entity to become
active. After panel movement reaches its stopping Y, the cadence continues;
the sequence state owns the music transition. Neither hold tick suspends
rendering, camera, background interpolation or other scene updates.

Use explicit enum encoding/decoding around the counter's subtract-one
arithmetic so the modern scoped-enum view keeps the domain. Retain the exact
operation order and signed-halfword narrowing. Compilation, tests, matching
and banking remain deferred until the naming pass finishes, as requested.


## Source result

`KfEndingScrollState` now types every producer and consumer of `scroll_state`.
`KfEndingScrollTick` types `scroll_tick`, both update predicates, the initial
value and the wrap assignment. Its two hold ticks retain separate names because
they occupy different positions in the countdown. Encoding, subtracting one
and decoding retains the old signed-halfword decrement before the wrap test;
there is no table, additional call or new control-flow branch.

Ten prior control-value occurrences are replaced; one unit-decrement literal
remains with an individual ledger reason. The scene has 570 retained numeric
occurrences, all reconciled against its current source expressions. No data,
external signature, test fixture or build configuration changes. The change
has source/retail review and literal accounting only; modern compilation and
retail object matching remain outstanding under the deferred-build policy.
