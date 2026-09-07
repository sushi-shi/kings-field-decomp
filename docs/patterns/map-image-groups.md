# Map image groups and retained script literals

## Function Match Plan

GAME `map_show_screen_image` (0x80034d54 / 0x90) formats
`KAN\Bf\Kgnn.TIM`: floor `f`, image group `g`, and decimal image index `nn`.
Its sole caller is `map_interaction_dispatch` (0x80034de4 / 0x904).
Give the group its own local signed-word enum and propagate it through the
caller local, both assignments and the formatter argument. Explicitly encode
the enum only when constructing its ASCII filename digit.

The helper's complete six-view dossier shows a 24-byte frame, signed division
of the second argument by ten, four path-byte stores, a floor-byte load and one
proven call to `screen_show_image_until_input`. Its five data references retain
their validated status; no strings are directly referenced through call operands.
At 0x80034d90..98 the first argument gains ASCII zero and is stored as a byte.
That does not distinguish a unique original narrow/word argument spelling.
Keep the current source's signed-word representation, and correct the stale
curated parameter names/types while retaining candidate signature confidence.

The dispatcher tests the named signboard and inscription-panel object IDs
(130/131) at 0x800355c8..e4, supplies group zero/one in the branch delay slots,
loads the linked image index as an unsigned byte and calls the formatter at
0x800355ec. These names describe which object family requests each image set,
not a guessed image title. The unsupported-object return and notification gate
remain intact. This edit changes no switch topology, jump-table evidence,
call sequence, data identity or object predicate.

In the same dispatcher, the shop-event branch requests map sequence index two
between the talk-pose update and interaction, then restores the current map
sequence after the menu. Name it `MAP_SHOP_SEQUENCE_INDEX`. Its observed role
is sufficient for that name even though the original track title is unknown.
The audio interface still receives its existing signed-word index.

Current source, history, curated xrefs, complete cached retail/CFG dossiers,
the adjacent dialogue routine and both called services were reviewed. These
are game-specific interaction, resource-path and audio policies around separately
attributed SDK providers; no vendored body is reconstructed. Existing match
reports describe stored objects, not the later naming edits. Builds, tests and
post-edit matching remain deferred until the naming pass finishes, as requested.

## Ledger reconciliation

The old map-script ledger predates the dispatch-flow corrections. Its stale
rows still describe two removed animation resets, the old asset-index expression,
the old search variable, an earlier countdown form and manual gold-byte assembly.
Review the current paths before removing those rows: animation availability now
comes from the map-event asset family, the search advances its current index,
the chest scan decrements before testing, and the typed link union supplies gold.
The reconciled ledger describes the current expressions and named image selectors;
matching history and historical scores are not rewritten as fresh verification.


## Source review result

Three raw selector occurrences now use names. The group domain reaches both
producer branches and the formatter argument; only filename encoding converts
it back to an integer. The curated signature retains candidate confidence.
The source diff changes these names/types without changing numeric values or
control flow. The map-script ledger matches all 232 remaining occurrences;
together with the renderer setup review, current accounting covers all 111 C
files and 6,102 occurrences.

The stored helper and dispatcher match percentages are 88.888885 and 95.585785,
respectively. These describe existing objects and are not post-edit verification.
Both functions remain unverified after this naming edit; no build, compiler
check, test, new match or bank operation was run.
