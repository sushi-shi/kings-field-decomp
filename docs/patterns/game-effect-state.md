# GAME effect-state ownership

## Function Match Plan and pre-edit evidence

Target: `GAME.EXE:0x8003781c effect_pool_set_current`, 52 bytes,
`src/game/effect_pool.c` / `game.effect_pool`, initially **71.076920%**
strict objdiff. GAME starts at 271/362 exact, the effect-pool unit at 2/4.
Hash-identical retail was initialized in this matching turn. The six required
semantic views were read with `--image game`, including the CFG and unfiltered
references. Source history and the earlier shared-base residue note were read.

- One linear leaf, no frame, calls, strings, conditional branches or indirect
  transfers. `$a0` is stored as the current effect pointer. `lbu +1` reads
  the record's kind; shifts/add multiply it by 20 to index a magic row.
  The selected row pointer is published. `jr ra` owns a final `nop`.
- Two validated HI16/LO16 pairs at `8003781c/20` and `80037840/44` target
  `8009db84` (current effect) and `8009db80` (current magic). The key retail
  instruction at `8003782c` is `addiu a1,a1,-3364`: it derives `8009ce60`
  (magic rows) from the current-effect slot address. This fills the kind
  load-delay slot. No third address pair exists for the table base.
- The sole proven caller, `effect_pool_sweep` at `8003a79c`, walks 48
  sixty-byte records, skips type `0xff`, passes the record in the call delay
  slot and immediately invokes `effect_update_dispatch`. No result is used.
  Its complete retail body was inspected. The preceding exact typed spawn
  constructor and following collision helper's boundary/current-effect use
  were inspected independently; neither body is a target of this campaign.
- Vendor negative control: absent from vendored/FID inventories, no SDK
  phase/effect API, no GTE/libc wrapper shape. Custom record fields, magic
  selection and dispatcher state identify game policy. No library body is
  added to progress.

The complete owner is independently supported by startup: the proven `memset`
call at `80014718` receives destination `8009ce60`, zero and **`0xd28`** in
its delay slot. It stops at `8009db88`, exactly where the next separately
cleared map-runtime owner begins. The exact magic loader copies 120 words
(480 bytes); effect sweep/reset walks 48 records of 60 bytes. Thus:

| Offset | Size | Shared field | Previous global |
| --- | --- | --- | --- |
| `0` | `0x1e0` | 24 `KfMagicRecord` rows | `magic_records` |
| `0x1e0` | `0xb40` | 48 `KfEffectRecord` rows | `effect_pool_records` |
| `0xd20` | `4` | current magic pointer | `current_effect_magic_record` |
| `0xd24` | `4` | current effect pointer | `current_effect` |

There are no gaps or unexplained padding fields in this interval. All four
former global xref views and all 55 curated address-pair rows were read,
including candidates. Consumers now access `effect_state` members directly;
the previous-global column above records historical identities, not live aliases.
There is one shared type, one BSS definition and one curated identity.
The existing claims are configuration-only; this campaign adds the actual
source owner in `game.effect_pool`.

First hypothesis: define `KfEffectState` and `effect_state`, with compile-time
size/offset checks. Preserve every member operation and the selector's two
assignments. Update only the referent symbol in the 55 relocation rows, keeping
all sites, decoded target VAs, statuses and evidence unchanged. Adapt existing
oracle lookups to the same owner offsets; do not create overlapping globals.
Require all 13 raw words, four ordered relocation entries and strict 100%
for closure; rebuild all consumers and preserve every banked exact function.
Contiguity alone is not proof of the original TU or a compiler mechanism.

## Retained reconstruction and strict result

The source definition is `DATA(0x8009ce60, 0xd28) KfEffectState effect_state`.
Compile-time typedef checks enforce its 3,368-byte size, four-byte alignment
and offsets `0x1e0`, `0xd20`, `0xd24`. The emitted BSS symbol itself is exactly
`0xd28` bytes. Header aliases preserve existing consumer operations while
referring to members of the same owner, not independent globals. Startup now
spells its already-existing whole-object clear with `sizeof(KfEffectState)`.

The initial trial immediately reproduced the selector's retail instructions;
its comparison still used the old target symbols. Regenerating the canonical
targets with the curated owner resolves that referent difference. The body
itself stays the same two assignments; no new arithmetic, constants, casts,
forced registers or assembly were added to the function.

Strict canonical objdiff is **100%**, up from 71.076920%. All **13 encoded
words** agree, including the `-3364` adjustment and final return delay slot.
All four ordered relocation entries agree: HI16/LO16 `effect_state+0xd24`
at function-relative `0/4`, then HI16/LO16 `effect_state+0xd20` at `0x24/0x28`.
All **55** curated incoming pairs retain every field except the target symbol;
the **46 reviewed / 9 candidate** split remains unchanged. Candidate entries
were not promoted to make a consumer exact.

The effect-pool unit moves **2/4 to 3/4** exact; GAME **271/362 to 272/362**.
The general constructor remains 52.130020%, with its existing non-exact
switch/body reconstruction unchanged. All other **483** reported function
rows, including SDK controls, retain their scores and sizes. All existing
exact consumers, including the pool search, reset, sweep, magic loader and
startup loop, remain exact. OPEN stays 97/108; PSX stays 1/1. No vendored body
or denominator was changed.

## Data closure and verification

The full build still fails its strict unfinished data/ownership/relink gates:
source data **11/59**, target relink **108/114** (six conflicting section
bases), and incomplete known-reference ownership. Both config-backed SDK
data contributions pass. GAME's source-owned range count increases **73 to
74**, while its config-only ranges fall **380 to 376** (uncompared **379 to
375**). The new BSS comparison exposes a section-extent residue: retail
`0xd28`, emitted `.bss` `0xd30`, although the actual object symbol is exactly
`0xd28`. The source does not add padding to disguise that difference; the
effect-pool unit's existing `.rodata` size difference also remains.

The affected unit and all consumers were rebuilt, canonical matches and the
full `kf build` rerun, raw words/relocations and all-image score snapshots
compared. Existing oracle lookups now derive the magic array at owner+0,
effect pool at +`0x1e0` and current effect at +`0xd24`. Their byte extents and
declared operations do not change. The inventory control initially caught
new field-row ordering and stale census totals; both were corrected, without
loosening the assertions. Only this strict-exact selector is selected for
banking. Generated objects, reports and trial logs remain ignored in `build/`.

Final checks: `ruff check scripts tests`, `git diff --check`, all **551**
repository tests (51.961 seconds), and `nix flake check -L` pass. The isolated
flake suite passes with its normal 77 unavailable-retail/tool skips. No new
test campaign or weakened gate was introduced.
