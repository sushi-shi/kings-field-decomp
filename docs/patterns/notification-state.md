# GAME notification payload and control ownership

`notify_enqueue` (`GAME.EXE:0x8001fa44`, 0xa0 bytes) and
`notify_effect_update` (`GAME.EXE:0x8001fafc`, 0x2cc bytes) share a numeric
payload table and the queue's control fields. The reviewed function snapshots
are in `config/evidence/game_semantic_notification_state.tsv`.

The linked instructions establish these relationships:

| Retail site | Address calculation | Meaning |
| --- | --- | --- |
| `0x8001fab0` | queue head at `0x80095087` minus 17 | payload array at `0x80095076` |
| `0x8001fbe8` | phase at `0x80095088` minus 18, indexed by twice the tail | same eight `u16` payloads |
| `0x8001fd34` | phase minus two | dequeue cursor at `0x80095086` |
| `0x8001fdb0` | dequeue cursor plus two | reset the phase |

`KfNotificationState` models `0x80095076..0x8009508c`: eight payloads, byte
tail/head/phase/hold fields, and a `u16` X angle. The queue module owns this
BSS definition and the separate eight-byte `notification_message_ids` array.
The control fields now form the six-byte `KfNotificationControl` subobject at
+0x10; the complete owner and every retail field address are unchanged. See
[the dequeue follow-up](game-notification-dequeue.md) for its instruction
evidence and the remaining initial-load difference. Consumers use `notify.h`.
Seventeen existing relocation rows retain their
addresses, instruction forms and evidence classifications; their target name
now identifies the aggregate. No relocation is added at a computed offset.

The message-ID array is independently materialized in every consumer. Including
it in the aggregate made the current probe derive its initialization address
from the phase pointer, whereas retail materializes it separately. Keeping that
array distinct preserves `render_initialize` exactly. This is a supported
working boundary, not proof of the complete original graphics object.

The variadic enqueue signature is supported by the four argument-register
spills, one-argument call sites, and the numeric-message caller at `0x80035554`
which supplies an additional integer in `$a1`. Its payload is read with `lhu`
from the first trailing argument's home slot. The identity inventory now agrees
with the existing variadic source/header declaration.

At the original ownership checkpoint, enqueue, the digit helper, buffer flip,
frame renderer and renderer initializer remained 100%. The map interaction
consumer remained 83.012130%. The effect state machine was 97.452515%, compared with the committed
98.296090% source that used cross-object byte arithmetic. The first instruction
divergence is the delay slot after the phase-three angle test: the probe forms
the tail pointer earlier. It also reuses the phase base for a tail load, chooses
different dequeue registers, and separately materializes the final phase store.
The call set, field widths, constants, and semantic branches agree; this is an
unattributed code-generation residue, and no exact claim was made for it.
The subsequent dequeue follow-up uses the current `probe-gcc257-o2-g0`
and improves the effect to 99.385475% without changing any consumer instructions.

The broader `KfGraphicsContext` experiment remains deferred. A full migration
across `0x80090ebc..0x800957e0` preserved strict data checks but lost three exact
functions: `render_initialize`, `display_present_frame`, and
`menu_present_frame`. Removing unsupported local pointer arithmetic from the
notification state does not close that larger ownership investigation. Future
work must reconcile its address-sharing witnesses with all three consumers.
