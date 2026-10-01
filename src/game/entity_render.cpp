#include <kf/lib/null.h>
#include <kf/game/graphics.h>
#include <kf/game/party_runtime.h>

#include <kf/platform/prelude.h>
#include <kf/game/asset.h>
#include <kf/game/render.h>
#include <kf/game/state.h>
#include <kf/lib/geometry_types.h>
#include <kf/lib/math.h>

#include <array>

enum { EFFECT_MODEL_DEPTH_BIAS = 100 };

std::array<KfSpriteQuad, KF_FLOOR_ITEM_SPRITE_COUNT> floor_item_sprites = {
    KfSpriteQuad{0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    KfSpriteQuad{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    KfSpriteQuad{0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    KfSpriteQuad{0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    KfSpriteQuad{0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    KfSpriteQuad{0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    KfSpriteQuad{0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

std::array<KfSpriteQuad, KF_EFFECT_BILLBOARD_SPRITE_COUNT> effect_billboard_sprites = {
    KfSpriteQuad{0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},
    KfSpriteQuad{0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},
    KfSpriteQuad{0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},
    KfSpriteQuad{0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},
    KfSpriteQuad{0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},
    KfSpriteQuad{0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    KfSpriteQuad{0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    KfSpriteQuad{0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    KfSpriteQuad{0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    KfSpriteQuad{0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    KfSpriteQuad{0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    KfSpriteQuad{0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    KfSpriteQuad{0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    KfSpriteQuad{0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    KfSpriteQuad{0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
};

void render_effect(KfEffectRecord *effect, const MATRIX *lights, const PartyEntityPose &pose,
    const PartyEffectAppearance &appearance)
{
    SVECTOR relative_position;
    VECTOR scale;
    MATRIX model;
    u16 asset;
    KfTmdObject *object;

    if (appearance.render_id.model == KF_EFFECT_MODEL_NONE) {
        return;
    }
    relative_position = VECTOR{
        pose.position.vx - game_graphics_runtime.render_state.view_position.vx,
        pose.position.vy - game_graphics_runtime.render_state.view_position.vy,
        pose.position.vz - game_graphics_runtime.render_state.view_position.vz}.narrowed();
    kf::render_place_model(model, game_graphics_runtime.render_state.view_matrix, relative_position);
    const KfEulerAngles angles {pose.rotation.vx, pose.rotation.vy, pose.rotation.vz};
    matrix_set_rotation_yxz(&angles, &model);
    scale = {(s16)appearance.scale_x, (s16)appearance.scale_y, (s16)appearance.scale_z};
    kf::matrix_scale_axes(model, scale);
    if (appearance.animation_clip == KF_ANIMATION_CLIP_NONE) {
        if (kf_enum_encode<u8>(appearance.render_id.billboard) >= KF_EFFECT_BILLBOARD_SPRITE_COUNT)
            kf::host_fail("Effect references an invalid billboard.");
        kf::matrix_multiply_rotation(game_graphics_runtime.render_state.pitch_matrix, model, model);
        render_enqueue_sprite(&effect_billboard_sprites[kf_enum_encode<u8>(appearance.render_id.billboard)], 0, KF_SPRITE_DEPTH_CUE_NORMAL, lights, &model, game_graphics_runtime.render_state.projection);
    } else {
        kf::matrix_multiply_rotation(game_graphics_runtime.render_state.view_matrix, model, model);
        asset = kf_enum_encode<u8>(appearance.render_id.model) + KF_ASSET_EFFECT_FIRST;
        asset_registry_select(asset);
        object = tmd_get_object(tmd_context(), 0);
        if (render_bind_instance_vertices(
                &effect->animation_cache, asset, appearance.animation_clip, appearance.animation_phase,
                object->vertex_count) == false) {
            tmd_select_object_vertices(tmd_context(), 0);
            tmd_project_vertices(tmd_get_object(tmd_context(), 0)->vertex_count, &model, game_graphics_runtime.render_state.projection);
        } else {
            tmd_project_vertices(object->vertex_count, &model, game_graphics_runtime.render_state.projection);
        }
        render_enqueue_tmd(0, EFFECT_MODEL_DEPTH_BIAS, lights);
    }
}

void entity_render_reset_module_state(void)
{
    kf::restore_initial_value<floor_item_sprites>();
    kf::restore_initial_value<effect_billboard_sprites>();
}
