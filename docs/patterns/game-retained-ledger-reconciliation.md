# Grouped retained-literal ledger reconciliation

Snapshot at `fcd555e844fb24f0b952e029c9ea3985c2ebdfd7`. The three source modules were compared to their
existing ledger expressions and expanded token multiplicities. Each source
numeric/character token is identified by function, complete source-line
expression and literal spelling. Repeated uses count separately; source line
numbers provide navigation and are not treated as identity.

| Source | Existing ledger inputs | Current occurrences | Missing / extra |
| --- | --- | ---: | --- |
| `src/game/player_death.c` | death/combat remainder, progression, database/startup | 197 | 0 / 0 |
| `src/game/player_use_item.c` | item-use | 79 | 0 / 0 |
| `src/game/notify_queue.c` | notification | 116 | 0 / 0 |

There were no conflicting reasons. The current death-module count is the union
of three focused inputs; comparing its former remainder ledger alone to the
whole file was insufficient. Grouped token rows in item-use and notification
were also outside the earlier single-occurrence validator's accepted format.
This was a coverage-verification limitation, not evidence of undocumented code.

The ledgers now list each current occurrence separately. The death ledger
covers its full module; the focused startup/progression ledgers remain useful
evidence references. Notification offset-check reasons now describe their
constant member-address base without incorrectly calling it unevaluated.
No source, constant value, ABI, identity inventory, test or build tool changes.

The normalized ledgers establish complete retained-value accounting. They do
not establish that no further naming is useful. In particular, the item-use
module still has meaningful interaction reach/radius, mirror range and
illusion-staff timer values whose existing reasons justify a focused naming
and consumer-propagation pass. Model IDs still require mesh/resource evidence
before receiving semantic identities. The remaining source `unknown_` fields
are unchanged by this documentation audit.

Verification: the complete coverage audit passed for all 111 source-file counts
and the 26 verified ledgers (2,404 occurrences), including links and anchors.
Ruff, all 680 repository tests (84.533 seconds), and `git diff --check` passed.
The full `kf build` was run and still reports the existing source-data and
target-relink mismatches; this documentation-only change does not resolve them.
