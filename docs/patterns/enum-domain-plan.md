# Literal census and enum-domain plan

This pass brings King's Field II's literal census (KF2 `docs/enum-domain-plan.md`)
to KF1 and records a verdict for every domain it proposes. KF1 had already
named most of its codes in the earlier campaigns
([strict-enum-audit.md](strict-enum-audit.md),
[enum-equality-review.md](enum-equality-review.md),
[enum-reuse-review.md](enum-reuse-review.md) and the per-file literal ledgers
indexed in [source-literal-coverage.md](source-literal-coverage.md)). The
census therefore finds a short remainder, not a new campaign. The curated rows
are in [enum-domain-plan.tsv](enum-domain-plan.tsv); Boolean domains are in
[boolean-and-null-review.md](boolean-and-null-review.md).

Every change keeps all three executables byte-identical. Every assembler
output also matches, apart from debug records and blank lines. All 462 exact
functions stay exact, and GAME/OPEN `main` keep their banked scores.

## Census

`kf literals` (`scripts/kf/literals.py`) parses all 97 manifest C variants
with the retail Clang flags. It records every written integer or character
literal, enum constant and object-like macro constant at its spelling site,
with the sink that consumes it: `Type.field`, `fn::local`, `fn:argN`,
`fn:return`, a switch subject, an array index, a mask, a shift, an arithmetic
operand, a declaration size or a pointer zero. Domains are unions of sinks
connected by value flow: copies, initializers, call arguments, returns,
equality between two slots, slots indexing the same array, and case labels
echoed into another slot for two or more values. Slots written by arithmetic,
and hubs, keep their literals but do not join domains. A hub is a slot with
more than 12 flow partners, or a local with more than 4. A domain is a lead;
the TSV records the decision.

```sh
nix develop -c kf literals --output build/literals.json   # full JSON
nix develop -c kf literals --domains                       # one row per domain
nix develop -c kf literals --member KfMapObject.action     # domains of one slot
nix develop -c kf literals --value 7                       # domains using a value
```

Each domain lists its KF2 counterparts. KF2 is read from `$KF2_REPO` or a
sibling `kings-field-2-decomp` checkout, using KF2's `KF_ENUM_BEGIN` enums and
the fields and parameters they type.

The KF1 port differs from KF2's tool in three ways. Each change has a control
in `tests/test_literals.py`:

- A read through `KF_ENUM_ENCODE` is a raw integer view (`slot#raw`) and does
  not carry the enum domain. KF1 spells 522 explicit encode boundaries. Under
  the KF2 rule, `KF_ENUM_ENCODE(s32, direction) < 0` was a literal in an
  enum-typed slot, and `kf check-types` failed on two such boundaries.
- A masked or shifted read (`x&0x3`, `x>>4`) is a packed integer sub-slot. It
  no longer inherits the stored enum's type.
- A K&R declaration keeps positional parameters, typed by the argument.
  Psy-Q `MEMORY.H` declares `memset()`. Without this rule, its fill bytes and
  sizes, and `SetSemiTrans`'s switch, were reported as 30 pointer zeros.

`kf check-types` now also runs the census. It fails on any written literal
whose sink is an enum-typed field, parameter, return, promoted local or switch
subject. It passes all 97/97 variants and reports 0 such literals.

## Numbers

The before column uses the ported tool at `85f89d30`. The raw KF2 rules
reported 2 typed-enum literals and 30 pointer-zero rows.

| Measure | Before | After |
| --- | ---: | ---: |
| Written numeric tokens in `src/` and `include/` | 8,086 | 8,023 |
| ... parsed / retail claims / `sizeof` / macro bodies / inactive | 6,763 / 1,300 / 3 / 11 / 8 | 6,700 / 1,300 / 3 / 11 / 8 |
| Literal locations | 6,792 | 6,729 |
| Literal sink rows | 6,877 | 6,814 |
| Enum-constant and macro-constant sink rows | 7,004 | 7,078 |
| Literal `case` labels | 13 | 9 |
| Literal returns | 18 | 7 |
| Literals in enum-typed sinks (check-types) | 0 | 0 |
| Literals compared with typed Booleans | 3 | 0 |
| Pointer-zero sink rows | 0 | 0 |
| Scoped enums (`KF_ENUM_BEGIN`) | 106 | 106 |
| Integral slots / flow edges | 3,662 / 2,403 | 3,662 / 2,403 |
| Candidate domains / multi-member / hubs | 1,134 / 146 / 111 | 1,132 / 146 / 111 |

The nine remaining `case` labels are `format_vsprintf`'s directive characters
(`'%'`, `'0'`, `'d'`, `'x'`, `'s'`, their capitals, and `'\n'`). The seven
remaining literal returns have no enum consumer: the zero of
`combat_calculate_damage_component` and `player_calculate_damage_component`,
`player_move_horizontal`'s constant 1 (twice), `save_workspace_allocate`'s
-1/0, and `memory_card_show_status_message`'s -1. No caller reads the
results of the last three functions.

After the change, the census domains break down by verdict. The literal-site
count is in parentheses:

| Verdict | Domains (literal sites) | Treatment |
| --- | --- | --- |
| `data-table` | 85 (1,162) | Glyph rows, palettes, matrices and resource tables. Initializers stay numeric. |
| `array-index` | 88 (774) | Array positions: `MATRIX.m`, glyph rows, double-buffer initialisation, magic sound/damage slots (positional per [game-effect-dispatch-literal-ledger.md](game-effect-dispatch-literal-ledger.md)) |
| `quantity` | 289 (645) | Arithmetic-written slots: heights, speeds, timers, counters |
| `boolean-candidate` | 339 (665) | Every domain has a row in [boolean-domain-review.tsv](boolean-domain-review.tsv) |
| `enum-candidate` / `flags-candidate` / `review` | 57 / 28 / 42 (47 / 4 / 71) | Every domain with a literal has a TSV row; 85 have no literal left |
| `singleton` | 60 (11) | One value, no flow partner |
| `typed-enum` | 144 (4) | The four sites are integer sinks in mixed channels; TSV rows "result channels" and "array positions" |

Coverage rule: every `enum-candidate`, `flags-candidate` and `review` domain
in `kf literals` has a member slot named in the TSV `members` column, or has
no literal site left. The same holds for the two `typed-enum` domains that
still contain literals. A check over the final census finds 44 of 44 such
domains covered.

## Applied domains

**Query-miss sentinels.** Each distance helper returns -1 when the point is out
of reach: `player_distance_to_point`, `actor_distance_to_point` (and its
wrapper `actor_player_distance`), `map_object_distance_to_point`,
`map_event_distance_to_point` and `player_distance_to_point_in_cone`. Each pool
search returns -1 when no slot matches: `actor_pool_find_overlap`,
`actor_pool_find_at_tile`, `map_object_pool_find_near_point`,
`map_object_pool_find_interaction_from` and `map_event_pool_find_overlap`.
These now return `KF_DISTANCE_NONE` (`lib/math.h`) and `KF_ACTOR_INDEX_NONE`
(`game/actor.h`), or `KF_MAP_OBJECT_INDEX_NONE` and `KF_MAP_EVENT_INDEX_NONE`
(`lib/map.h`). Every consumer compares against the same name. KF2 uses the
same names for its distance and actor-overlap sentinels.

The census joined these with `KF_COLLISION_NONE` through
`collision_query_world::hit`, and the player helper used that spelling before.
That join is a linked boundary, not one domain. `collision_query_world` copies
each helper's result into `hit` and compares it with that helper's sentinel.
Only then does it OR in the `KF_COLLISION_*` class bit and return a packed
collision result. `map_object_probe_door_closing` returns that packed result
unchanged, so its two callers now compare with `KF_COLLISION_NONE`. The pass
replaces 42 literal sites, two of them in the door probe's callers.

**Floor-local actor definitions (`game/actor.h`).** An actor definition slot
indexes the loaded floor's `KfActorDefinitionTable`. A slot therefore names a
monster only on its floor. Every literal use is guarded by
`current_floor == KF_FLOOR_4/5` or sits in a floor-5 script or load case:

- Floor 5's definition 7 is the boss: `KF_FLOOR5_BOSS_DEFINITION`. Its
  encounter rewrites five attack animations; its damage is gated until the
  encounter starts; it plays its own sound policy and death sequence.
- On floor 4, definition 5 transforms into 6 after death:
  `KF_FLOOR4_TRANSFORM_SOURCE_DEFINITION`/`RESULT_DEFINITION`. This replaces
  `player_warp.c`'s local `ACTOR_TRANSFORM_RESULT_DEFINITION`.
- The boss death sequence, and a floor-5 load after its defeat, end every
  actor of definitions 0, 2, 3 and 4: `KF_FLOOR5_BOSS_DEATH_DEFINITION_0/2/3/4`.
  These are WIP names: the role is known, the monsters are not.

The actor-spawner effect (kind 9) spawns definitions 2, 4 and 0. Its effect
code comes from actor definition data, so its floor is not proven. Those three
literals stay numeric. The floor names replace 22 literal sites.

**Smaller names.**

- The three yaw switches and the boss-emitter rotation switch list 0 beside
  the named quarter-turn angles. That case is now `KF_ANGLE_NO_TURN`.
  Assignments of angle 0 stay numeric.
- `memory_card_show_status_message` sends `SAVE_STATUS_OK` to
  `SAVE_MESSAGE_NONE` (-1). The loader does not treat -1 as its 0xff skip
  value. The old inline comment moved to the constant.
- Untargeted effects (collision-target bits 0) return
  `KF_COLLISION_UNTARGETED` (1). This value is neither `NONE` nor a classed hit.
  The name records only where it occurs, as the earlier ledger noted.
- `menu_format_number` writes `value % 10` as the glyph code, so digit glyphs
  equal their values. Zero padding fills with `MENU_NUMBER_ZERO`.
- The player's weapon-magic effects pass `KF_PLAYER_DAMAGE_MULTIPLIER_ONE` to
  `effect_pool_construct`, as `magic.c` already does for spells. The
  constructor byte (`KfEffectRecord.id`) reaches only `multiplier_tenths` or a
  child effect. Its WIP owner names and `WARP_SHIMMER_OWNER_ID` stay as
  decided in [shared-constant-review.md](shared-constant-review.md) and
  `docs/naming-review-gameplay.tsv`.

## Equal values kept separate

| Value | Names | Verdict |
| --- | --- | --- |
| -1 | `KF_DISTANCE_NONE`, `KF_ACTOR_INDEX_NONE`, `KF_MAP_OBJECT_INDEX_NONE`, `KF_MAP_EVENT_INDEX_NONE`, `KF_COLLISION_NONE` | linked: `collision_query_world` tests each finder result with its own sentinel before it builds the packed result |
| -1 | `SAVE_MESSAGE_NONE`, `KF_MENU_RESULT_CANCELLED`, `MENU_TEXT_END`, `KF_SAVE_OVERLAY_NONE` | retain: no flow between message IDs, menu results, glyph terminators and overlay rows |
| 0 | `KF_ANGLE_NO_TURN`, `MENU_NUMBER_ZERO`, `KF_FLOOR5_BOSS_DEATH_DEFINITION_0` | retain: angle, glyph code and definition slot |
| 1 | `KF_COLLISION_UNTARGETED`, `KF_MAP_ORIENT_UNROTATED` | retain: packed collision result versus orientation byte |
| 7 | `KF_FLOOR5_BOSS_DEFINITION`; dialogue page 7 | retain: definition slot versus authored page number |
| 10 | `KF_PLAYER_DAMAGE_MULTIPLIER_ONE`, `WARP_SHIMMER_OWNER_ID`, `MENU_NUMBER_BLANK` | linked (first two: both reach the constructor's byte, but the warp role is unproven, see above); retain the glyph code |

The 0x58 and `object->action = 5` examples behind KF2's plan have no
counterpart here. KF1's map-object action field is already the scoped
`KfMapObjectOperation`, and none of its switches has a literal case.

## Literals that stay numbers

The retained rows in the TSV group the remaining enum-candidate and review
domains by reason:

- SDK, libc and format arguments: fill bytes, screen coordinates, back colour,
  sequence volume, formatter characters, save-header magic.
- Quantities: HP/MP deltas, zero heights and radii, colour levels, depth
  biases, blends, scales, countdowns.
- Authored floor-script data: dialogue page numbers, the floor-1 tile and world
  points, the spawner's definition slots.
- Array positions and glyph text: the first level-growth row, pool starts, the
  highlighted first row, kana label codes.
- Packed low bytes under an explicit encode.
- Result channels. `menu_root` returns an item ID or a negative `KfMenuResult`
  in one word, `save_workspace_allocate`'s status has no reader, and effect ID
  0 is a zero damage multiplier.

Floor-local map-event slots (`map_runtime_state.events[1..3]`) and
`world_state.floors[4]` stay positional. Their union member (`script.floor5`)
or the guarding character case already names them.

## Apply checklist

For each domain:

1. Confirm membership from the census edges and the retail
   loads, stores and compares.
2. Choose storage: `KF_ENUM_STORAGE`, `KF_ENUM_PARAM` or `KF_ENUM_PROMOTED`
   for scoped enums, or a named constant where the slot keeps integer storage.
3. Replace every member literal.
4. Rebuild, then compare the EXE hashes and the non-debug assembler lines.
5. Run `kf check-types`, `kf analyze`, `kf verify board`, Ruff, pytest and
   `git diff --check`.
