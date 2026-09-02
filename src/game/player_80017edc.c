#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
/* Image paths whose decimal digits are patched before display. */
extern char enemy_info_image_path_template[13];
extern char person_image_path_template[14];
extern void func_8001fde4(s32 first, s32 second);
extern void screen_show_image_until_input(const char *path);

ADDRESS(0x80017edc, 0xc8)
void actor_show_info_image(const KfActor *actor)
{
    func_8001fde4(0, 0);
    func_8001fde4(0, 0);
    enemy_info_image_path_template[3] = '0' + player_state.progress_state.current_floor;
    enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;
    enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;
    screen_show_image_until_input(enemy_info_image_path_template);
}

ADDRESS(0x80017fa4, 0xb0)
void map_event_show_person_image(const KfMapEvent *event)
{
    func_8001fde4(0, 0);
    func_8001fde4(0, 0);
    person_image_path_template[8] = '0' + event->kind / 10;
    person_image_path_template[9] = '0' + event->kind % 10;
    screen_show_image_until_input(person_image_path_template);
}
