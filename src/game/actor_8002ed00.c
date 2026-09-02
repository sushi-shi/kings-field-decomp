#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorState actor_state;

extern s16 angle_approach(s16 current, s16 target, s32 step);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern s32 actor_move_xz_with_collision(const struct KfVecXZs *delta, s32 stop_on_collision);

/* Turns the current actor toward its movement yaw and steps along it; a negative DIRECTION walks backwards. */
ADDRESS(0x8002ed00, 0xd4)
void actor_move_along_heading(s32 direction, s32 stop_on_collision)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    struct KfVecXZs delta;
    u32 rate;

    if (actor->collision_state == 1) {
        rate = definition->turn_rate;
        actor->rotation.y = angle_approach(actor->rotation.y, actor->movement_yaw, (rate + rate + rate) >> 1);
    } else {
        actor->rotation.y = angle_approach(actor->rotation.y, actor->movement_yaw, definition->turn_rate);
    }
    angle_to_forward_xz(actor->rotation.y, &delta);
    vector2s_scale_shift11(definition->move_speed, (s16 *)&delta);
    if (direction < 0) {
        delta.x = -delta.x;
        delta.z = -delta.z;
    }
    actor_move_xz_with_collision(&delta, stop_on_collision);
}
