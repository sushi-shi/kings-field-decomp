#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * On-screen notification ring.  Player stat routines (level, training,
 * experience, item use) enqueue a message id here; the notification effect
 * state machine (notify_effect_update) consumes it a frame later.  Message id 0x13
 * carries a u16 payload stored in a parallel table.
 *
 * WIP: the ring, its payload table, and the head cursor are unresolved
 * address-only globals, so the payload table is reached as a byte view offset
 * from the head cursor (the retail shared-symbol addend the original struct
 * produced).  The trailing varargs slot carries the id 0x13 payload.
 */

ADDRESS(0x8001fa44, 0xa0)
void notify_enqueue(int id, ...)
{
    u8 *head;

    if (id == 0xff) {
        return;
    }
    head = &DAT_80095087;
    if (DAT_8009506e[*head] == 0xff) {
        DAT_8009506e[*head] = id;
        if (id == 0x13) {
            u16 *payload = (u16 *)(head - 17);
            payload[*head] = *(u16 *)(&id + 1);
        }
        *head = (*head + 1) & 7;
    }
}
