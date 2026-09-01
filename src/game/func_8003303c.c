#include <kf/semantic_types.h>

extern struct KfVec4i audio_listener_position;
extern struct KfVec4s audio_listener_rotation;

void func_8003303c(
    const struct KfVec4i *position_or_null,
    const struct KfVec4s *rotation_or_null)
{
    if (position_or_null != 0) {
        audio_listener_position = *position_or_null;
    }
    if (rotation_or_null != 0) {
        audio_listener_rotation = *rotation_or_null;
    }
}
