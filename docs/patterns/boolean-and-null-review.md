# Pointer zeros, Booleans and the modern check

This pass brings KF2's lane-E2 review (KF2
`docs/boolean-and-null-review.md`) to KF1. KF1 had already spelled its pointer
zeros as `NULL` and typed its proven flags. That earlier campaign is recorded
in [boolean-modeling.md](boolean-modeling.md). This pass adds the modern C++
view of the pointer census and a verdict for every Boolean-candidate domain
in the literal census. Every executable is byte-identical. Every assembler
output also matches, apart from debug records and blank lines. The
[enum-domain plan](enum-domain-plan.md) owns the non-Boolean literals.

## Pointer zeros

```sh
nix develop -c python -m scripts.kf.pointer_zeros            # retail C and modern C++ views
nix develop -c python -m scripts.kf.pointer_zeros --mode retail
```

The census parses all 97 C variants twice: in the retail C view and in the
modern C++ view. It now also reaches branches selected only by `__cplusplus`
or `KF_MODERN_TYPES`. It reports a literal zero converted to a pointer.
Zeros expanded from `NULL` are not reported, and neither is a zero that an
SDK macro supplies itself. It reports **0 sites in both views**.

`kf/lib/null.h` now defines `NULL` as `nullptr` in the C++ view. The retail
C view keeps Psy-Q Release 2.5's integer `0`, so the compiler input is
unchanged. Clang's `-Wzero-as-null-pointer-constant` drops from 134 project
diagnostics to 0. All 134 were `NULL` spellings checked as integer zero. KF1
needs no `offsetof` header: its source has no null-object layout checks, and
its offset assertions live in test fixtures that use `__builtin_offsetof`.

`kf literals` reports 129 pointer-zero sink rows:

| Rows | Verdict |
| ---: | --- |
| 127 | Already spelled `NULL` |
| 2 | Allocator arena bounds (`MEMORY_INITIAL_ARENA_LAST_ADDRESS`, `MEMORY_SYSTEM_HEAP_END_ADDRESS`): fixed addresses with an owning literal review, not null pointers |

Under KF2's tool the census also reported 30 literal pointer zeros:

- 17 `memset` fill bytes and sizes;
- 13 `SetSemiTrans` switches.

Psy-Q declares these functions without prototypes. The KF1 census keeps their
positional parameters, typed by the argument, so all 30 are now integer
arguments.

## Booleans

```sh
nix develop -c kf bools --output build/bools.json
```

`kf bools` stopped with incomplete coverage on nine `vendor/include` wrappers
that contain only preprocessor directives. KF2's coverage rule now treats
directive-only wrappers as reviewed. The audit covers all 97 variants and 79
project headers. It reports 3,508 integral slots: 22 already Boolean, 556
already enum, 1,289 non-Boolean, 386 external, 19 vendored, 1,049 unknown,
and 187 earlier-reviewed single-value, numeric-use and unknown-writer cases.
It reports no unreviewed candidate.

The literal census finds three typed-Boolean comparisons. Each tested a
predicate's result against zero, and each is now a truth test. GCC folds both
spellings to the same `EQ 0`, so the bytes do not change:

- `!angle_within_tolerance(...)` in `actor_update_current_action`;
- `!angle_mod_delta_le_half_turn(...)` in `player_move_horizontal`;
- `if (player_warp_trigger_update())` in the GAME main loop.

`kf literals` reports 341 Boolean-candidate domains before the change and 339
after; the two converted comparisons left no literal behind.
[boolean-domain-review.tsv](boolean-domain-review.tsv) gives each domain a
verdict:

| Verdict | Domains | Literal sites | Meaning |
| --- | ---: | ---: | --- |
| converted | 3 | 3 | Typed-Boolean comparison rewritten as a truth test |
| typed | 11 | 0 | Already a `KfBool*` slot spelled `KF_FALSE`/`KF_TRUE` |
| named | 110 | 0 | Existing constants already spell every value (for example `KF_MAP_SCRIPT_SET`/`UNSET`, `KF_PLAYER_STATUS_NONE`, SDK macros) |
| reject | 217 | 665 | Not a Boolean slot (reasons below) |

The rejections follow the rules in [boolean-modeling.md](boolean-modeling.md):

- 122 quantities. Counters, deltas, sizes, timers and indices whose slot is
  written by arithmetic or increments.
- 20 SDK, BIOS or libc arguments and results. `PadRead(1)`, the
  `SetSemiTrans`/`SetDispMask` switches, `exit(1)`, `TestEvent() == 1`,
  `InitCARD2(1)`. These stay the SDK's integers.
- 25 other quantities tested or reset at zero: angles, offsets, health, gold,
  velocities, damage components, counts, drop-sequence counters and queue
  indices.
- 11 pad input words. A zero test means no button pressed.
- 8 write-only resets of unidentified words.
- 10 masked-bit, sign, parity or period tests of wider words. The flag
  constants are already named.
- 4 NUL string terminators and 2 sector remainders.
- 15 other cases. Each has its own reason in the ledger, among them:
  - `(sound->tone & 0x80) == 1` is never true; retail keeps it as written.
  - `render_entities::visible` holds the `KfCellVisibility` byte and the
    square-culling predicate in one retail register.
  - `player_move_horizontal` returns 1 on every exit, and no caller reads the
    result.

The candidates that `boolean-modeling.md` already rejected stay rejected:
`cd_file_load_*::loaded` also holds a sector count, and the option, padding
and effect-sound slots have meaningful enums.

## Modern check

`kf check-types` passes 97/97 source/image variants before and after; KF1's
SDK prototypes and valueless returns were already settled by the
[cast-union](cast-union-debt.md) and
[enum-field](enum-field-review.md) campaigns. The check now also runs the
literal census and reports 0 literals in enum-typed sinks.

## Cleanliness ratchet

`kf verify board` floors were lowered to the live counts only: `void* views`
248 to 246 and `.c-local aggregates` 7 to 6. No floor was raised.
