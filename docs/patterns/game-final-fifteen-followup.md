# Remaining game source follow-up

## Function Match Plan

The user authorized extending the Sony-helper campaign to game control flow
and object ownership. Start from `1eea5f81`, based on master `2b1baa89`, with
456/471 strict-exact functions. The fifteen unfinished functions remain the
objective; their previous [SDK controls](sony-final-fifteen.md) and
[earlier-header control](sony-sdk-july-1994.md) are negative evidence.

GAME `effect_map_collision` (`80037850`, 1900 bytes, 99.873690%) was refreshed
first. Its one differing delay-slot word has already received the ordinary
selector, result, explicit-zero and helper-boundary controls. No new source
fact supports repeating those trials or removing the defined zero result.

The first new source control is GAME `map_object_spawn_effect` (`80031834`,
404 retail bytes versus 400 probe bytes, 94.504950%). Fresh retail validation,
six semantic views, the actor caller, all callees, both adjacent functions,
source history and shared map-object declarations were inspected. It remains
game pool-selection and initialization policy, with five ordered calls and
two validated sequence referents. The seven exact unit siblings are controls.

The caller provides byte kind and object ID, a complete VECTOR pointer and a
signed Y offset. The acquisition call returns a live object from one of two
ten-object groups. Both sequence counters and the destination sequence field
are unsigned halfwords in disjoint parts of `KfMapObjectState`. Retail reloads
the selected counter after acquisition, copies the object pointer during that
load delay, increments and stores the counter, then stores its previous value
at object+34. The probe instead reuses one saved register successively for
the sequence and object, with an additional load-delay nop.

Earlier trials retain the postincrement result or separately name the old and
new counters. Test the remaining consumed expression boundary: the object's
sequence is the predecessor of the newly incremented unsigned-halfword
counter. Compare `++*sequence - 1` directly with a named `u16 next_sequence`
consumed by that subtraction. This preserves both 16-bit stores, including
65535 -> 0 wraparound; verify the entire halfword domain. Do not retain an
extra reload, subtraction, mask, changed store order or altered alias model
merely to change register allocation. Compile complete alternate units under
the unchanged pinned profile, resolve every word and ordered reference, and
preserve all exact siblings. Only strict 100% plus the full required checks
can be banked.

## Sequence result

Both forms emit the same 404-byte candidate at 93.663370%. They retain all
five calls, both ordered referents and seven exact siblings, but add an actual
`addiu v1,v1,-1` before the destination halfword store. Neither recovers the
object lifetime or the retail saved-register set. Exhaustive halfword-domain
evaluation confirms the values, including wraparound, but the extra operation
rejects the hypothesis. Neither source is retained.

## Path-owner Function Match Plan

The next related family comprises three fixed-width filename builders. Their
byte stores, preserved literal templates and shared string consumers identify
complete path extents and actual mutable digit fields:

| GAME function | Retail / current bytes | Strict % | Object and mutable offsets |
| --- | ---: | ---: | --- |
| `map_show_screen_image`, `80034d54` | 144 / 148 | 88.888885 | Static 16-byte path; floor +5, group +8, number +9/+10 |
| `item_load_database`, `80020cfc` | 1500 / 1500 | 99.746666 | Local 40-byte path; bank +8, number +11/+12/+13 |
| `talk_show_dialogue_page`, `8002c9d4` | 164 / 164 | 98.780490 | Global 20-byte path; directory +6/+7, floor +10, stage +11, repeated character +12/+13, page +14 |

Refresh all six views, callers, callees by decoded image-qualified address,
neighbors, source history and current raw controls. These are game formatters;
their SDK string/CD callees retain separate library ownership. The map and
dialogue functions each have one call, with five and seven ordered address
pairs respectively. The item loader has five calls and ten ordered pairs,
including the existing initializer's 19-byte tail clear. Keep every initialized
byte, signed division, store order, source load, call and exact sibling.

Test a complete union owner with the unchanged character-array text member
and a character-field view at the proved offsets. The API still receives the
text member. Preserve the existing directory cursor where one is present;
do not simultaneously alter divmod expression or cursor lifetime. Use static
size/offset checks. For the global dialogue path, give its temporary shared
declaration the same complete type as its definition; no incompatible per-file
view or cast over a differently declared object is permitted. Keep templates
as the original complete string literals and introduce no runtime copy or
additional object. Compare complete allocated objects and every sibling,
then inspect the first raw difference before deciding whether any source fact
is retained.

## Path-owner results

The map and dialogue field views produce identical complete allocated objects
to their fresh baselines: 148 bytes at 88.888885% and 164 bytes at 98.780490%.
All 14 and 23 exact siblings respectively remain exact; the map unit's other
unfinished sibling is also unchanged. The unchanged map reference order still
differs from retail; no referent or relocation correction follows from this
control. Dialogue retains its seven ordered referents and single call.

The item union changes the initializer from a 21-byte literal plus a 19-byte
`memset` to a complete 40-byte template copy. It removes the clear call, grows
the allocated literal section from 37 to 53 bytes and emits 1580 bytes at
90.586670%. Its five exact siblings remain exact, but the target function's
changed initialization and call set reject the owner shape. The existing
literal ownership bounds are preserved; they are not enlarged to admit the
candidate. No path-owner source is retained.

## Rendering compilation-context Function Match Plan

Test the contiguous nine-function GAME band `8001e5ec..8001f798` in one
disposable compilation unit. Confirmed dispatcher calls, shared graphics
state, matrix consumers and visibility-grid use provide family evidence beyond
adjacency. This is a compilation-context control, not proof of an original TU
boundary. All five existing units use `probe-gcc257-o2-g0` with no defines.
The immediately preceding screen-sprite unit uses a different profile and is
excluded so the control changes no compiler settings.

| Function | Address | Baseline verdict |
| --- | --- | --- |
| `render_map_cell` | `8001e5ec` | 99.878380%, frame/stack-coordinate residue |
| `render_map_cells` | `8001e83c` | Exact control |
| `render_actor` | `8001e9a4` | Exact control |
| `render_map_object` | `8001ebb8` | Exact control |
| `menu_render_item_model` | `8001ed38` | Exact control |
| `render_floor_item` | `8001ed90` | Exact control |
| `render_actor_sprite` | `8001eedc` | Exact control |
| `render_map_event` | `8001f0c4` | Exact control |
| `render_entities` | `8001f218` | 99.517044%, saved-register role residue |

Refresh each function's six views, history, adjacent functions and constraining
call sites before compiling. Keep every function body, shared declaration and
profile unchanged. Arrange functions by ascending claim and all three owned
data definitions by ascending data claim. Preserve each initializer and check
its object bytes separately. Compare every function against a fresh baseline,
its delinked target and raw retail words, including ordered calls and data
referents. Any change must be explained from its first divergence before a
source grouping could be retained.

## Rendering compilation-context results

All nine functions reproduce their fresh individual-unit baselines word for
word after relocation. Every ordered call and data referent is unchanged, and
all seven exact controls still equal complete retail bodies at strict 100%.
The three owned data objects also equal their individual-unit targets and raw
retail bytes: `floor_item_sprites` (84 bytes), `effect_billboard_sprites`
(264 bytes) and `render_fixed_cell_window` (204 bytes).

`render_map_cell` remains 592 bytes at 99.878380%, with the same 88-byte frame
against retail's 120. `render_entities` remains 1408 bytes at 99.517044%, with
the same thirty unequal GPR words and no changed non-register instruction bit.
Compilation context does not explain either residue under this control. No
unit grouping, source body or manifest change is retained.

## Campaign disposition

Six alternate complete units exercise seven target-function controls across
six unfinished functions: two sequence expressions, three filename owners and
one nine-function rendering compilation. None closes a function. The other
nine unfinished functions retain their preceding evidence and verdicts in
[the fifteen-function dossier](sony-final-fifteen.md); these new controls do
not claim to retest their source hypotheses. Production sources, headers,
profiles, inventories and the exact bank remain unchanged. The total is still
**456/471 strict-exact functions**, with all fifteen targets unfinished.

The evidence for another source correction must go beyond an equivalent
expression with a different score. In particular, neither the cell's unused
stack region nor the orbit function's larger retail frame proves an unused
matrix owner; previous controls already rejected that shortcut. Matching
operand values alone likewise does not establish a different collision return
or filename initialization. The observed residual instructions remain
unattributed; no historical compiler mechanism is inferred.

Generated six-view dossiers, alternate sources, exact-sibling controls, raw
resolved words, ordered referents and data comparisons are under
`build/broader-source/`, including `spawn-counter/`, `path-owners/` and
`render-context/`. They are not committed.

## Verification

A fresh `kf build --reconfigure` rebuilds affected objects and retains PSX 1/1,
GAME 348/362 and OPEN 107/108 exact functions. It exits nonzero for the existing
incomplete reference ownership, data comparisons and section-placement
failures: PSX 1/1, GAME 29/41 and OPEN 12/20 data-owning units match, with zero
artifact failures; target relink remains PSX 1/1, GAME 57/64 and OPEN 34/38.

All 777 Python tests and 9,223 subtests pass. Ruff, Rust tests and whitespace
checks pass. No function is newly banked, and no source/configuration change
from these rejected or byte-identical controls enters the commit.
