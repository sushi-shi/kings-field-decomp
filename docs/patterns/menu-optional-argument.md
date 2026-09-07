# Menu dispatch optional-byte boundary

## Function Match Plan

GAME `menu_enter_mode`, 0x80036e38 / 0xc8, is a variadic mode dispatcher.
The reviewed six-view retail dossier has a 24-byte frame, four incoming
argument-home stores, eight proven call sites, four validated internal jumps
and no strings. Its stored object is 100%; no current-source rebuild is claimed.
The adjacent unreferenced return stub remains semantically unresolved and is
not renamed or reconstructed by this change.

Retail first saves `a0..a3`, then reloads the mode as a word. Pickup and shop
branches read the low byte of the first optional argument at 0x80036ebc and
0x80036ed0. Root mode calls `menu_root` without reading that argument. Calls,
cleanup and full-word result transport remain in their original order.
This is game-owned menu/heap orchestration; the GPU synchronization provider
is separately attributed to Psy-Q.

The current callers establish the modern variadic type: the shop caller passes
`event->character_id` (`u8`), two container callers dereference `u8 *item_id`,
and the direct pickup caller explicitly encodes its object ID as `u8`.
All four become `int` under default argument promotion. `s32` is the project's
`long` alias and must not be substituted for that promoted type merely because
both occupy one target word. The root caller passes no optional argument.

Introduce branch-local `u8 item_id` and `u8 shop_id` names. Under modern types,
use the compiler's variadic builtins to read `int`, narrow to the consumed byte
and close the argument list before calling the selected menu. Under the pinned
retail compiler, preserve the home-slot access but express its stride as
`sizeof(mode)`, the existing full-word mode width. The project already uses
this modern/retail split in notification argument handling.

The source contract still allows root mode to omit the argument, still narrows
pickup/shop inputs to their low byte, and still transports mode-specific results
through the existing `u32` channel. It does not add a default-mode result,
validate item IDs, reinterpret shop output as pickup success, or change callers.
Compilation, tests and post-edit matching remain deferred until naming finishes,
as requested; no banking follows from this change.

## Source result

Both optional inputs now have branch-local names and byte storage. Modern
branches retrieve the callers' promoted `int` and narrow it explicitly; the
root branch never opens an argument list. Retail branches retain the original
argument-home address with its offset derived from the mode type. Pickup-result
encoding and the shop's zero return retain their separate meanings.

The two hard-coded byte offsets disappear. The module's two remaining literals
are the blocking `DrawSync` selector and shop-result zero, each accounted for in
the [small-module ledger](small-module-literal-ledger.md). No callers, signatures,
tests, data, unresolved-stub identities or toolchain configuration change.
Source and ABI review are complete for this edit; compilation and matching
remain outstanding under the deferred-build policy.
