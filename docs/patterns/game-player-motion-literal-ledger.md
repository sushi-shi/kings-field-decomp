# Player motion retained literals

The complete current rows for `player_sync_position_to_map`,
`player_update_view_bob` and `player_update_vertical_motion` now live in the
[complete player-core ledger](game-player-core-literal-ledger.md).
Those functions retain seven numeric occurrences; view bob has none.

The [state and units review](game-player-vertical-state.md) records the
retail evidence. Counter resets, the near-spawn Boolean and occupancy delta
remain arithmetic or Boolean values. The height cutoff -6999 and map attribute
82 still lack a proved environmental identity. Consolidation preserves their
reasons while keeping a single current ledger for the whole module.
