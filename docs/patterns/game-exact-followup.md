# Matching follow-up on the readability backport

## Function Match Plan

Start from PR #29, `2473d3c7`, in an isolated worktree. The fresh native
build succeeds for all three executables. Analysis retains 458/471 exact
game functions and all thirteen exact vendored controls; the existing data
ownership/relink gates are separate from function matching.

The effect collision and orbit routines were considered first. Their prior
campaigns already reject the supported selector, width and owner changes.
In particular, an unexplained 64-byte frame interval in GAME
`effect_projectile_update_2d` does not justify an unused local object. Do not
repeat those diagnostic padding controls or bank their artificial matches.

Continue instead from #10's typed item-name row in GAME
`menu_draw_item_detail`, `80027b7c`, 732 bytes, strict 99.672134%, using the
unchanged `probe-gcc257-o2-g8` menu-unit profile. Refreshed
six-view evidence, the complete retail CFG, all six caller windows, adjacent
preview/quad functions, shared menu types and source history retain the
signed-word item/shop/mode interface, 255 guard, ten halfword copies,
seventeen calls, twenty ordered data referents and 160-byte frame. This is
game menu policy, not a vendored routine; matrix and rendering providers
remain separate calls.

Only the nine register operands of the name-copy region disagree. Retail
uses source/count/destination `a2/a0/a1`; the current indexed loop uses
`a1/a2/a0`. The neighboring exact preview repeats the same complete row-copy
operation. Test a typed inline copy boundary receiving the existing
`MenuGlyphString` and `MenuGlyphRow`, now composed with #10's early complete
table/row selection in the consolidated menu translation unit. Earlier
helper controls used older source states; they are negative evidence, not a
reason to assume this composition will succeed.

Preserve every value, access width, copy direction, table/row identity,
primitive-cursor publication, draw call and surrounding operation. Reject
extra calls, layout changes, unsafe interior-pointer walks, artificial
carriers and any regression in the 34 exact sibling functions. Compare
complete resolved words and ordered physical referents, not just the fuzzy
percentage. Keep only a supported improvement; exact closure requires 100%.

The row-copy helper is negative: 732 bytes, 99.672134%, the same nine raw
differences, all seventeen calls and twenty referents unchanged, and all
34 siblings exact. Remove the helper. A second composition tests the actual
ascending halfword source cursor while retaining the complete early table
owner and indexed destination. Retail explicitly advances that cursor by
two after each load; keep the ten-copy bound and legal array extent.

The explicit source cursor recovers three words: strict 99.781420%, still
732 bytes, with six differing words. It preserves all calls, referents and
34 exact siblings. Only the count/destination register exchange remains;
the source cursor now agrees throughout. Compose the typed copy helper with
this supported traversal once, checking whether the complete operation
boundary also accounts for those remaining two values.

## Object-spawn Function Match Plan

GAME `map_object_spawn_effect`, `80031834`, starts at 97.128716%, 408 probe
bytes versus 404 retail bytes. Fresh six-view evidence, the entire retail
body, both incoming argument paths in the actor action routine, the pool
acquisition/action callees and adjacent debris constructor retain byte
drop-source and object identifiers, a VECTOR pointer and signed-word Y
offset. The game-specific pool/action policy is not vendored; `rand` is an
external SDK provider. Five calls and both sequence-counter references
already agree, and all seven siblings are exact.

With the early drop-range predicate retained by an earlier campaign, the
probe keeps the acquired object separate from the sequence pointer, but
computes the Boolean before the first action branch and carries it in `s2`.
Retail instead evaluates that third range at the second branch, keeps the
masked object ID in `a1` for the comparisons and has no extra argument move.
Test the cohesive final action-initialization operation as a typed inline
helper: reset action,
select one of the three ID-band actions, then reset vertical velocity.
Move the range comparison with its consumer; leave acquisition, sequence
publication, positions, cell coordinates, random yaw and their ordering
outside this boundary. This is narrower than the previously rejected
whole-object initializer and acquire/publish helpers.

Do not force register assignments or retain a helper merely because its
fuzzy score rises. Check the five calls, two sequence referents, load/store
widths, checked divisions, three action thresholds, return joins and all
seven exact siblings against complete retail words.

The action helper is rejected: 94.504950%, 400 bytes, 54 unequal aligned
words (plus the four-byte extent deficit), versus 36 unequal aligned words
and a four-byte excess at baseline. It reproduces the older sequence/object
coalescing residue. Five calls, both counter referents and all seven exact
siblings survive, but the size and load-delay mismatch do not describe
retail. Restore the production source; do not bank this routine.

## Retained result

The source-cursor helper composition is identical to the local cursor loop.
Remove that helper and retain only the explicit source traversal in
`menu_draw_item_detail`: no new helper, type, layout, constant or interface.
The final increment forms the legal one-past pointer to the ten-code row;
it is not dereferenced. The item guard, item domain and destination remain
unchanged.

Three instructions are corrected at `+e0`, `+110` and `+114`. The six
remaining differences at `+ec`, `+f4`, `+118`, `+11c`, `+120` and `+128`
exchange the count and destination registers only. The first remaining
divergence is `80027c68`: retail initializes the count in `a0`, while the
probe uses `a1`. This is an unattributed residue, not historical compiler
attribution or an exact closure.

| Function | Final verdict |
| --- | --- |
| GAME `menu_draw_item_detail` | Retain the source cursor: 99.672134% to 99.781420%, nine to six unequal words, 732 bytes throughout. Not exact; not banked. |
| GAME `map_object_spawn_effect` | Reject the action helper; unchanged at 97.128716%, 408 probe bytes versus 404 retail bytes. Not banked. |
| GAME `effect_projectile_update_2d` | Refreshed evidence and prior controls do not identify the unused frame interval. Unchanged at 99.934210%; no padding trial or source change. |
| GAME `effect_map_collision` | Prior one-word selector/return controls remain negative; no new supported source hypothesis or source change. Unchanged at 99.873690%. |

The source-cursor change is a partial improvement. The number of exact game
functions does not increase: 458/471, with thirteen still unfinished.
Generated dossiers, complete raw listings and sibling audits remain under
`build/exact-followup/`; none is committed.

## Verification

The final canonical `kf match --unit game.menu_runtime` recompiles the unit.
`kf build` successfully links all three native executables. The subsequent
full analysis retains the pre-existing data ownership, reference-coverage
and section-placement failures; those gates still exit nonzero and are not
claimed closed by this function-only change.

Before/after snapshots cover all 97 allocated comparison objects and all
484 function rows, including vendored controls. Only `game.menu_runtime`
changes: six instruction words in the intended function, at `+e0`, `+ec`,
`+110`, `+114`, `+118` and `+120`. Three now equal retail; the other three
remain register-only differences. All symbol contracts, ordered relocation
records and non-code allocated sections are unchanged. All 483 other match
scores are unchanged, and all 34 exact menu siblings remain raw-exact.

All 880 existing Python tests pass with no skips (309.170 seconds); the
existing codec-oracle binary was built before running them. No tests were
added or changed. The menu modern-type check, Ruff and whitespace checks
pass. Existing Rust
tooling tests pass, with five pre-existing proprietary-corpus tests ignored
by the default invocation. An independent clean-context review reproduces
the strict scores, raw sibling matches and source/evidence hashes, checks
the legal cursor extent, and confirms the rejected object-spawn source and
compiled unit equal their baselines. It reports no actionable findings.
