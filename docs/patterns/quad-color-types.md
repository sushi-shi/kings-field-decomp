# Typed colors for simple quads and fades

## Function Match Plan

Replace raw RGB component indices in the simple quad builders and fade callers
with the supplied SDK `CVECTOR` fields. Propagate `const CVECTOR *` through
GAME/OPEN FT4, OPEN F4 and OPEN G4 declarations, definitions and all callers.
Replace the two private four-byte fade wrappers and the opening overlay color
array with `CVECTOR`. Retain color values, read-only borrowing, copy lifetimes,
byte arithmetic, component-write order, descriptor representations and owners.

The flake's `LIBGTE.H` declares CVECTOR as four `u_char` members, `r, g, b, cd`.
The retail helpers load unsigned bytes at offsets zero, one and two and write
the corresponding SDK packet RGB fields. They never read the fourth byte.
All callers supply four-byte objects; several ending-scene callers already
use CVECTOR and previously passed the address of its first member.

## Evidence and resulting source

| Image / function | VA / extent | Evidence and change |
| --- | --- | --- |
| GAME `sprite_add_ft4` | 0x80014314 / 0x1c0 | Loads the color pointer as an O32 stack argument; lbu at 0x80014474/80/8c reads RGB. It now reads `color->r/g/b`, retaining packet initialization, texture/position inputs and OT insertion. |
| OPEN `sprite_add_ft4` | 0x8001399c / 0x1c0 | Same RGB consumption, with three callers and OPEN's separate primitive/OT owner. Shared typed color argument; selector words, rectangle halfwords and texture bytes retain their prior contracts. |
| OPEN `sprite_add_f4` | 0x80013b5c / 0x114 | Reads one RGB input and initializes the flat-shaded packet separately. Its ending-scene caller now passes the complete background CVECTOR. |
| OPEN `sprite_add_g4` | 0x80013804 / 0x198 | Four color pointers supply independent RGB triplets, including the fourth pointer on the stack. The caller supplies the top color twice and bottom color twice. All four arguments now preserve those complete CVECTOR objects. |
| GAME `display_play_transition` | 0x800144d4 / 0x1a0 | Copies the four-byte base color, advances its first byte and mirrors it into bytes two and one. Uses a CVECTOR local with the same red/blue/green write order and passes its address. |
| OPEN `opening_fade_in` | 0x800155c0 / 0xfc | Same unsigned-byte fade arithmetic and RGB mirroring; uses a CVECTOR local and typed FT4 call. The frame loop and synchronization stay intact. |
| OPEN `opening_scene3_run` | 0x80014804 / 0x330 | Its FT4 call at 0x80014a2c references the four-byte RGB-200 overlay color at 0x8003728c. The global is now CVECTOR, with the same initializer and data claim. |
| OPEN `opening_ending_scroll_run` | 0x80014e28 / 0x798 | Calls G4 at 0x800153ec, F4 at 0x80015404 and FT4 at 0x8001553c. Its existing top, bottom, background and panel colors are now passed directly instead of borrowing each red byte as an array. |

Fresh image-qualified retail disassembly was reviewed for the six helper/fade
bodies, together with the existing caller dossiers, curated xrefs, strings,
data and source history. Direct calls are proven edges; data pairs are validated
referents. Game-specific packet composition and fading remain distinct from
the separately identified Sony GPU callees. No relocation or ownership changes.

GAME 0x80057b24 and OPEN 0x800372d0 remain four initialized zero bytes.
OPEN 0x8003728c remains `{200, 200, 200, 0}`. The `cd` values are retained,
but these helpers do not consume them as opacity or replace packet commands.
The fade still performs the original byte addition and comparison; neither
saturation behavior nor the authored frame/step values is rewritten.

Named fields replace 35 numeric component accesses, and three explicit color
array extents disappear with the typed objects: 38 fewer inline literals.
The [fade/quad ledger](fade-quad-literal-ledger.md) covers all 97 remaining
occurrences in the two fades and two FT4 modules. The existing OPEN render and
scene ledgers are reconciled for the G4 fields and overlay declaration.
Curated signatures, data types and the existing overlay inventory expectation
agree with source.

Compilation, tests and post-edit matching are deferred until the naming pass
finishes, as requested. This records source and retail evidence, not a verified
new binary result; no function is banked by this change.
