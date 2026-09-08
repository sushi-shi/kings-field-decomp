# GAME shop-panel control flow

## Function Match Plan

Start at `968ed27` on master with hash-verified Japanese retail and the existing
`probe-gcc257-o2-g0` profile. The shared list-confirmation widget is exact;
the [item-use flow audit](game-item-use-flow.md) supplies a related source
hypothesis, not permission to transplant its distinct frame loop.

| GAME function | Extent | Starting strict objdiff | Retail reference snapshot |
| --- | --- | --- | --- |
| `80021538 item_menu_buy` | 1476 bytes | 97.200540% | 21 proven calls, 12 validated internal jumps, 11 validated HI16/LO16 pairs |
| `80021afc item_menu_sell` | 1280 bytes | 96.771870% | 20 proven calls, 11 validated internal jumps, 5 validated HI16/LO16 pairs |

Both are claims in `game.item`, whose four other functions are controls:
`80020b4c item_load_floor_placements` 98.888885%,
`80020cfc item_load_database` 99.746666%,
`800212d8 item_menu_root` 100%, and `80021ffc item_pickup_confirm` 100%.
Do not change their source, module data owners or existing RODATA claim.
The six semantic views and baseline comparison are in `build/shop-*.log`.
Source history `14298ac`, the source-shapes note and the subsequent
[price-domain audit](game-shop-price-domains.md) were reviewed before edits.
The original history's assertion of correct CFG is not established by a score.

The sole direct callers are the root's `800213f4`/`80021404` calls, passing
its retained shop ID through a0 in each delay slot and ignoring v0. Preserve
both `void(s32 shop_id)` signatures. The preceding root's return owns
`80021534`; the buy/sell return slots are `80021af8`/`80021ff8`. Both panels
have an 1856-byte frame, list at +24, eighty ten-halfword labels at +64,
two eighty-byte arrays at +1664/+1744 and saves starting at +1824. The shared
KfMenuList, list initializer, model loader/releaser, detail renderer and
confirmation widget constrain the unchanged interfaces and field widths.

These shop/inventory policies have no vendored roster entries, no local
strings, no indirect control transfers and no GTE/library wrapper shape.
PadRead remains the separately attributed LIBETC PAD.OBJ boundary. All item
names, prices, stock and player fields keep their existing owners and curated
identities. Buy lists IDs 42..79 then 0..41 only when shop availability is
nonzero and player quantity is below 99. Prices are u16 rows with two shop
columns, gold is u32 and stock/cursor fields are bytes. Buy decrements shop
availability only for Gold Cross 52. Sell scans IDs below 52, excludes one
equipped copy using all seven byte equipment IDs, then credits the sell price.

Restore one evidenced source fact per focused comparison (both related
functions may share the edit):

1. Clear confirmation before the exit guard. The stores at `800217c8` and
   `80021d4c` are unconditional branch delay slots, not exit-only updates.
2. Set selected index to `entry_count - 1` before either upward-wrap viewport
   arm, as the `80021898`/`80021e1c` delay-slot stores require.
3. Use cancellation-first result handling after the confirmation call to
   reproduce its pending-sentinel store and fallthrough accepted-item load.
4. Restore scroll-first inner up/down arms and confirmation-before-input
   initialization where their raw branch/prologue order supports it.

Preserve the existing empty-list guard and the single loop-head present call
(`8002176c`/`80021cf0`); these already agree with retail. Both model-load
failure paths bypass ordinary release/transactions and reach the shared
restore tail (`80021ad4`/`80021fd4`). Normal exit first waits for input release,
then releases the model; only a non-cancelled selection changes stock/gold.
Check each path, its raw constants, referents and delay slots rather than
counting return instructions alone.

Rebuild after every focused edit, compare from the first divergence and keep
all four neighboring controls unchanged. Full build, raw-word audit, all-image
scores, existing tests, Ruff and whitespace checks precede handoff. Bank only
verified 100% functions from clean campaign inputs; preserve unrelated dirt.

## Results

Both panels retain all five evidenced corrections. The reset and long-list
wrap changes correct actual behavior; cancellation-first handling recovers the
sentinel assignment in the post-call branch delay slot, and the initialization
and scroll-arm order reproduce the observed instruction order. Neither change
needs a fake local, forced register, inline assembly or compiler-profile change.

| GAME function | Final strict objdiff | Complete words | Remaining differing words | Verdict |
| --- | --- | --- | --- | --- |
| `80021538 item_menu_buy` | 99.769646% | 369 / 369 | 17 | Non-exact; not banked |
| `80021afc item_menu_sell` | 99.609375% | 320 / 320 | 25 | Non-exact; not banked |

Fresh compilation followed by explicit numeric relocation resolution reproduces
all other retail words. The delinked target itself was independently resolved
and checked against the hash-verified EXE. Retail body SHA256 values, including
the return delay slots, are:

- Buy: `7c5d06bf6a8c8ab48f8f53a09ef5d5169cd2db4be94e4ec14f77dd6ffdbb57eb`.
- Sell: `c1bb1a7d57b4e8921b10dbf8e325cd25079d5ef6fe2d8d49da6125c8f058604c`.

The final buy comparison has 69 transfers: 21 calls, 12 internal jumps,
35 conditional branches and one return. Sell has 68: 20 calls, 11 internal
jumps, 36 conditional branches and one return. Every transfer retains its
site, opcode and numeric destination; the frame size, memory offsets,
immediates and instruction order also agree. Both model-load failure entries
still bypass model release and transactions, unlike normal completion.

All remaining differing bits are GPR operand fields. This diagnosis is not a
masked exactness criterion and is not proof of the historical allocator:

| Value | Buy retail / candidate | Sell retail / candidate |
| --- | --- | --- |
| Completed/pending selection | s1 / s2 | s1 / s3 |
| Loop's index-array base | s2 / s1 | s2 / s1 |
| Current input | s3 / s3 | s3 / s2 |

Buy's differing word offsets are `030 230 254 274 280 284 28c 2dc 420 45c
4d4 4f8 524 52c 530 57c 594` (hex, relative to the claim). Three are owned
delay slots: `800217ac` and `80021814` set selection, while `80021a68`
shifts it for the price row.

Sell's differing word offsets are `02c 030 1f0 214 234 240 244 24c 274 284
288 29c 2a0 2b0 34c 35c 3e0 400 410 424 440 464 490 494 4b4`. Seven are
delay slots: `80021d30`, `80021d80`, `80021d98`, `80021dac`, `80021e58`,
`80021f0c` and `80021f90`. They carry the same selection/input operations but
different registers. The delay-slot words therefore do **not** all match.

The ordered buy HI16/LO16 targets remain:
`800652a8 80059108 800652a8 80058dc0 800652a8 800594b8 800a07ac
800594b8 800a07ac 800652a8 800652a8`.
Sell retains `800a07e4 80058dc0 800652a8 800a07ac 800595f8`.
Both ordered call-target lists agree exactly. There is no evidence here for
missing relocation curation, a different call set or an unmodeled exit edge.
The remaining register symptoms are unattributed; no declaration permutation
or source distortion was attempted to remove them.

All 484 all-image score rows were compared with the pre-campaign snapshot;
only these two percentages changed. Root and pickup remain strict 100% and
reproduce all 152/174 raw words. Both loader scores are unchanged. Eligible
exact counts remain 399/471 overall, GAME 300/362, OPEN 98/108 and PSX 1/1.
The [shop literal ledger](game-shop-literal-ledger.md) is refreshed for the
corrected predicates and unconditional resets; its 153 occurrences are unchanged.

Ruff, whitespace checks and all 657 existing tests pass (86.080 seconds).
No tests or tooling were added. The full build still fails on the existing
data/ownership/placement checks: source data 8/61, SDK/config data 4/4 and
target relink 110/116 (PSX 1/1, GAME 75/77, OPEN 34/38). Six conflicting
section bases and incomplete known-reference coverage remain; artifact
failures remain zero. The shop unit's existing 40/37-byte RODATA extent and
BSS placement failures are unchanged. No bank entries change, and unrelated
worktree edits are excluded from this campaign.


## Strict closure follow-up

The [GCC trace campaign](game-ui-trace-matches.md#shop-results) closes both shop
panels at strict 100%. The buy source shares the navigation model-load join;
the sell source consumes the confirmation return through its existing numeric
selection channel. Both retain the flow corrections above, and complete
relocation-resolved comparison verifies all 689 retail words. The historical
17/25-word residues recorded here are resolved by those retained changes.
