#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerProgressState player_progress_state;
extern u8 player_map_variant;
extern void audio_play_map_sequence(u8 sequence_id);

ADDRESS(0x8001b49c, 0xbc)
void audio_play_current_map_sequence(void)
{
    s32 sequence_id = 0;

    switch (player_progress_state.current_floor) {
    case 1:
        if (player_progress_state.level >= 15) {
            sequence_id = 1;
        }
        break;
    case 2:
        if (player_progress_state.level >= 25) {
            sequence_id = 1;
        }
        break;
    case 5:
        if (player_map_variant == 3) {
            sequence_id = 1;
        }
        break;
    }
    audio_play_map_sequence(sequence_id);
}
