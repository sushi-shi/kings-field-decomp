# Character IDs and exchange roles

## Function Match Plan

Use a byte-backed `KfCharacterId` in map-event definitions and live records,
and carry the same domain through the dialogue API. Name the three character
selectors whose item-exchange outcomes establish useful roles. These are role
names, not recovered proper names. Keep character 12 as an explicit unresolved
encoding; its link-clear operation does not yet identify the linked object.

All addresses select GAME.EXE. Review current source, history, cached complete
retail/CFG/caller/callee/string dossiers and the existing
[dialogue evidence](game-map-dialogue-state.md). The resource consumer and
interaction policy are game code around separately attributed SDK operations.

| Function / extent | Evidence and intended change |
| --- | --- |
| `map_event_pool_load`, `0x800338b8 / 0x22c` | The 56-byte-frame loader copies definition +1 to live +1 using `lbu`/`sb` at `0x80033940..48`. Type both fields so the existing assignment preserves the domain. Its caller is the map-resource loader; the prior [animation/turn review](map-event-animation-turn-domains.md) records the complete pool-load evidence. |
| `map_event_interact`, `0x80034a80 / 0x2d4` | A byte load at `0x80034a94` feeds the character dispatch. ID 3 consumes Gold Cross and gives Key of the Dead; ID 7 consumes Dragon King Grass Fruit and gives Harp; ID 8 consumes Mirror of Truth and enables Healing. Name those cases, retaining all seven proven calls, 28 validated data pairs and five internal jumps. The frame is 32 bytes; no strings are directly referenced. |
| `map_event_show_person_image`, `0x80017fa4 / 0xb0` | Loads +1 twice for the two decimal digits of `PRSN/PERcc.TIM`; explicitly encode each byte enum operand before division/remainder. Preserve its 24-byte frame, two render calls, image-display call and two validated path pairs. Its caller is `player_use_item`. |
| `talk_show_dialogue_page`, `0x8002c9d4 / 0xa4` | All four proven callers in the interaction routine load character +1 into a2. The formatter uses signed-word division by ten, then repeats the digits in directory and filename. Use `KF_ENUM_PARAM(KfCharacterId, s32)` and explicit s32 encoding, retaining the C parameter width, 24-byte frame, seven path pairs and sole image-display call. |
| `map_interaction_dispatch`, `0x80034de4 / 0x904` | The shop call loads character +1 at `0x80035088` and invokes the variadic menu dispatcher at `0x8003508c`. Explicitly encode to u8 so default argument promotion still supplies int to its existing optional-argument reader. The [map-image review](map-image-groups.md) records the surrounding dispatcher evidence. |

## Boundaries and unresolved values

The authored resource byte remains the character-ID storage boundary. Definition
and live fields have the same byte domain; their loader assignment needs no
integer conversion. Dialogue digits are representation arithmetic, so encode at
those expressions. The shop's stock-bank index shares the character's numeric
encoding; its variadic boundary must receive the promoted integer expected by
the [optional-argument reader](menu-optional-argument.md).

The enum's named entries are not a whitelist. Other authored character IDs
continue to pass through the typed fields and filename formatter. The retained
case 12 is decoded explicitly into this domain, without adding a range check.
Its link ID 55 remains documented as unresolved. Proper names and the remaining
resource identities need further evidence.

Preserve the existing inventory quantities, dialogue stages/pages, branch order,
signedness, return paths and ordered referents. Builds, compiler checks, tests,
post-edit matches and banking remain deferred until the naming pass finishes.

## Source review result

The three exchange selectors now have role names. Definition-to-live assignment
and all four dialogue calls carry the character domain directly. Portrait and
dialogue digit construction encode at the arithmetic expressions; the one shop
call encodes to u8 before default argument promotion. Character 12 and link 55
remain explicit unresolved values with their documented branch roles.

Reversing the intended substitutions recovers all three edited C files exactly.
The header review preserves byte storage in both records and s32 in the retail
C dialogue signature. The two structure rows and one function identity reflect
those source types without changing evidence confidence. Current token/expression
accounting covers all 111 C files and 6,089 retained occurrences.

All five listed functions remain unverified after this type edit, including the
loader affected through its shared field types. Source accounting does not prove
binary matching. The naming goal remains open.
