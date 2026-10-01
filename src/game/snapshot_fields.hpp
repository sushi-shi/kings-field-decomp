// Copy explicit wire fields; runtime enum widths and containers are independent.
template<class T, class U>
static void snapshot_assign(T &destination, const U &source)
{
    if constexpr (requires { std::size(destination); std::size(source); }) {
        static_assert(std::size(T{}) == std::size(U{}));
        for (std::size_t i = 0; i < std::size(destination); ++i)
            snapshot_assign(destination[i], source[i]);
    } else {
        destination = static_cast<T>(source);
    }
}
template<bool Restore, class T, class U>
static void snapshot_field(T &runtime, U &record)
{
    if constexpr (Restore) snapshot_assign(runtime, record);
    else snapshot_assign(record, runtime);
}
template<bool Restore, class T, class U>
static void snapshot_union(T &runtime, U &record)
{
    static_assert(sizeof(T) == sizeof(U));
    if constexpr (Restore) runtime = std::bit_cast<T>(record);
    else record = std::bit_cast<U>(runtime);
}

template<bool Restore>
static void snapshot_player(PlayerContext &player, KfNetWorldPlayer &record)
{
    auto &p = player.state;
    snapshot_field<Restore>(p.experience, record.experience);
    snapshot_field<Restore>(p.next_level_experience, record.next_level_experience);
    snapshot_field<Restore>(p.progress_state.level, record.progress_state_level);
    snapshot_field<Restore>(p.progress_state.unknown_01, record.progress_state_unknown_01);
    snapshot_field<Restore>(p.progress_state.current_floor, record.progress_state_current_floor);
    snapshot_field<Restore>(p.progress_state.highest_floor, record.progress_state_highest_floor);
    snapshot_field<Restore>(p.map_variant, record.map_variant);
    snapshot_field<Restore>(p.allow_near_actor_spawn, record.allow_near_actor_spawn);
    snapshot_field<Restore>(p.weapon_charge_delay, record.weapon_charge_delay);
    snapshot_field<Restore>(p.unknown_0f, record.unknown_0f);
    snapshot_field<Restore>(p.vitals.maximum_hp, record.vitals_maximum_hp);
    snapshot_field<Restore>(p.vitals.current_hp, record.vitals_current_hp);
    snapshot_field<Restore>(p.vitals.maximum_mp, record.vitals_maximum_mp);
    snapshot_field<Restore>(p.vitals.current_mp, record.vitals_current_mp);
    snapshot_field<Restore>(p.attack_charge_state.current, record.attack_charge_state_current);
    snapshot_field<Restore>(p.attack_charge_state.committed, record.attack_charge_state_committed);
    snapshot_field<Restore>(p.magic_charge, record.magic_charge);
    snapshot_field<Restore>(p.physical_power_training, record.physical_power_training);
    snapshot_field<Restore>(p.magic_training, record.magic_training);
    snapshot_field<Restore>(p.base_physical_power, record.base_physical_power);
    snapshot_field<Restore>(p.base_magic, record.base_magic);
    snapshot_field<Restore>(p.physical_power, record.physical_power);
    snapshot_field<Restore>(p.magic, record.magic);
    snapshot_field<Restore>(p.status_effect_flags, record.status_effect_flags);
    snapshot_field<Restore>(p.gold, record.gold);
    snapshot_field<Restore>(p.cutting_attack, record.cutting_attack);
    snapshot_field<Restore>(p.striking_attack, record.striking_attack);
    snapshot_field<Restore>(p.piercing_attack, record.piercing_attack);
    snapshot_field<Restore>(p.holy_attack, record.holy_attack);
    snapshot_field<Restore>(p.fire_attack, record.fire_attack);
    snapshot_field<Restore>(p.unknown_3a, record.unknown_3a);
    snapshot_field<Restore>(p.cutting_defense, record.cutting_defense);
    snapshot_field<Restore>(p.striking_defense, record.striking_defense);
    snapshot_field<Restore>(p.piercing_defense, record.piercing_defense);
    snapshot_field<Restore>(p.poison_resistance, record.poison_resistance);
    snapshot_field<Restore>(p.magic_defense, record.magic_defense);
    snapshot_field<Restore>(p.fire_defense, record.fire_defense);
    snapshot_field<Restore>(p.curse_timer, record.curse_timer);
    snapshot_field<Restore>(p.darkness_timer, record.darkness_timer);
    snapshot_field<Restore>(p.poison_timer, record.poison_timer);
    snapshot_field<Restore>(p.slowed_timer, record.slowed_timer);
    snapshot_field<Restore>(p.fire_defense_timer, record.fire_defense_timer);
    snapshot_field<Restore>(p.illusion_staff_timer, record.illusion_staff_timer);
    snapshot_field<Restore>(p.unknown_54, record.unknown_54);
    snapshot_field<Restore>(p.equipment_effect_ticks, record.equipment_effect_ticks);
    snapshot_field<Restore>(p.selected_magic_id, record.selected_magic_id);
    snapshot_field<Restore>(p.unknown_5d, record.unknown_5d);
    snapshot_field<Restore>(p.equipped_weapon_id, record.equipped_weapon_id);
    snapshot_field<Restore>(p.unknown_65, record.unknown_65);
    snapshot_field<Restore>(p.weapon_attack_phase, record.weapon_attack_phase);
    snapshot_field<Restore>(p.unknown_72, record.unknown_72);
    snapshot_field<Restore>(p.weapon_magic_shots_remaining, record.weapon_magic_shots_remaining);
    snapshot_field<Restore>(p.weapon_magic_delay, record.weapon_magic_delay);
    snapshot_field<Restore>(p.weapon_attack_fully_charged, record.weapon_attack_fully_charged);
    snapshot_field<Restore>(p.unknown_7b, record.unknown_7b);
    snapshot_field<Restore>(p.equipped_head_armor_id, record.equipped_head_armor_id);
    snapshot_field<Restore>(p.equipped_body_armor_id, record.equipped_body_armor_id);
    snapshot_field<Restore>(p.equipped_shield_id, record.equipped_shield_id);
    snapshot_field<Restore>(p.equipped_arm_armor_id, record.equipped_arm_armor_id);
    snapshot_field<Restore>(p.equipped_leg_armor_id, record.equipped_leg_armor_id);
    snapshot_field<Restore>(p.equipped_accessory_id, record.equipped_accessory_id);
    snapshot_field<Restore>(p.audio_effects_enabled, record.audio_effects_enabled);
    snapshot_field<Restore>(p.audio_music_enabled, record.audio_music_enabled);
    snapshot_field<Restore>(p.hud_gauges_enabled, record.hud_gauges_enabled);
    snapshot_field<Restore>(p.compass_enabled, record.compass_enabled);
    snapshot_field<Restore>(p.view_rotation_offset.vx, record.view_rotation_offset_vx);
    snapshot_field<Restore>(p.view_rotation_offset.vy, record.view_rotation_offset_vy);
    snapshot_field<Restore>(p.view_rotation_offset.vz, record.view_rotation_offset_vz);
    snapshot_field<Restore>(p.update_state, record.update_state);
    snapshot_field<Restore>(p.unknown_a3, record.unknown_a3);
    snapshot_field<Restore>(p.camera_position.vx, record.camera_position_vx);
    snapshot_field<Restore>(p.camera_position.vy, record.camera_position_vy);
    snapshot_field<Restore>(p.camera_position.vz, record.camera_position_vz);
    snapshot_field<Restore>(p.foot_height, record.foot_height);
    snapshot_field<Restore>(p.camera_rotation.vx, record.camera_rotation_vx);
    snapshot_field<Restore>(p.camera_rotation.vy, record.camera_rotation_vy);
    snapshot_field<Restore>(p.camera_rotation.vz, record.camera_rotation_vz);
    snapshot_field<Restore>(p.motion_state.strafe_velocity, record.motion_state_strafe_velocity);
    snapshot_field<Restore>(p.motion_state.forward_velocity, record.motion_state_forward_velocity);
    snapshot_field<Restore>(p.motion_state.movement_speed, record.motion_state_movement_speed);
    snapshot_field<Restore>(p.motion_state.yaw_step, record.motion_state_yaw_step);
    snapshot_field<Restore>(p.motion_state.pitch_step, record.motion_state_pitch_step);
    snapshot_field<Restore>(p.motion_state.map_cell.x, record.motion_state_map_cell_x);
    snapshot_field<Restore>(p.motion_state.map_cell.z, record.motion_state_map_cell_z);
    snapshot_field<Restore>(p.previous_map_cell.x, record.previous_map_cell_x);
    snapshot_field<Restore>(p.previous_map_cell.z, record.previous_map_cell_z);
    snapshot_field<Restore>(p.unknown_ce, record.unknown_ce);
    snapshot_field<Restore>(p.view_bob_offset, record.view_bob_offset);
    snapshot_field<Restore>(p.view_bob_phase, record.view_bob_phase);
    snapshot_field<Restore>(p.death_camera_pitch_step, record.death_camera_pitch_step);
    snapshot_field<Restore>(p.death_visual_blend, record.death_visual_blend);
    snapshot_field<Restore>(p.vertical_velocity, record.vertical_velocity);
    snapshot_field<Restore>(p.vertical_state, record.vertical_state);
    snapshot_field<Restore>(p.unknown_df, record.unknown_df);
    snapshot_field<Restore>(player.party_slot, record.party_slot);
    snapshot_field<Restore>(player.previous_input, record.previous_input);
    snapshot_field<Restore>(player.cast_pose_ticks, record.cast_pose_ticks);
    snapshot_field<Restore>(player.movement_velocity_limit, record.movement_velocity_limit);
    snapshot_field<Restore>(player.turn_step_limit, record.turn_step_limit);
    snapshot_field<Restore>(player.item_stock, record.item_stock);
    snapshot_field<Restore>(player.learned_magic, record.learned_magic);
    snapshot_field<Restore>(player.random.state, record.random_state);
}

template<bool Restore>
static void snapshot_actor(KfActor &value, KfNetWorldActor &record)
{
    snapshot_field<Restore>(value.generation, record.generation);
    snapshot_field<Restore>(value.random.state, record.random_state);
    snapshot_field<Restore>(value.target_player_slot, record.target_player_slot);
    snapshot_field<Restore>(value.target_player_generation, record.target_player_generation);
    snapshot_field<Restore>(value.transform_step, record.transform_step);
    snapshot_field<Restore>(value.slot_state, record.slot_state);
    snapshot_field<Restore>(value.definition_id, record.definition_id);
    snapshot_field<Restore>(value.culling_mode, record.culling_mode);
    snapshot_field<Restore>(value.heading_quadrant, record.heading_quadrant);
    snapshot_field<Restore>(value.tile_z, record.tile_z);
    snapshot_field<Restore>(value.tile_x, record.tile_x);
    snapshot_field<Restore>(value.lifecycle, record.lifecycle);
    snapshot_field<Restore>(value.spawn_chance, record.spawn_chance);
    snapshot_field<Restore>(value.action, record.action);
    snapshot_field<Restore>(value.death_drop_object_id, record.death_drop_object_id);
    snapshot_field<Restore>(value.animation_clip, record.animation_clip);
    snapshot_field<Restore>(value.vertical_state, record.vertical_state);
    snapshot_field<Restore>(value.unknown_0c, record.unknown_0c);
    snapshot_field<Restore>(value.local_z, record.local_z);
    snapshot_field<Restore>(value.local_x, record.local_x);
    snapshot_field<Restore>(value.animation_phase, record.animation_phase);
    snapshot_field<Restore>(value.health, record.health);
    snapshot_field<Restore>(value.cell_x, record.cell_x);
    snapshot_field<Restore>(value.cell_z, record.cell_z);
    snapshot_field<Restore>(value.unknown_1a, record.unknown_1a);
    snapshot_field<Restore>(value.position.vx, record.position_vx);
    snapshot_field<Restore>(value.position.vy, record.position_vy);
    snapshot_field<Restore>(value.position.vz, record.position_vz);
    snapshot_field<Restore>(value.rotation.vector.vx, record.rotation_vector_vx);
    snapshot_field<Restore>(value.rotation.vector.vy, record.rotation_vector_vy);
    snapshot_field<Restore>(value.rotation.vector.vz, record.rotation_vector_vz);
    snapshot_field<Restore>(value.action_progress, record.action_progress);
    snapshot_field<Restore>(value.collision_state, record.collision_state);
    snapshot_field<Restore>(value.movement_yaw, record.movement_yaw);
    snapshot_field<Restore>(value.animation_step, record.animation_step);
    snapshot_field<Restore>(value.vertical_velocity, record.vertical_velocity);
    snapshot_field<Restore>(value.movement_x, record.movement_x);
    snapshot_field<Restore>(value.movement_z, record.movement_z);
    snapshot_field<Restore>(value.movement_y, record.movement_y);
    snapshot_field<Restore>(value.unknown_46, record.unknown_46);
}

template<bool Restore>
static void snapshot_effect(KfEffectRecord &value, KfNetWorldEffect &record)
{
    snapshot_field<Restore>(value.owner_player_slot, record.owner_player_slot);
    snapshot_field<Restore>(value.owner_player_generation, record.owner_player_generation);
    snapshot_field<Restore>(value.generation, record.generation);
    snapshot_field<Restore>(value.age, record.age);
    snapshot_field<Restore>(value.random.state, record.random_state);
    snapshot_field<Restore>(value.target_player_slot, record.target_player_slot);
    snapshot_field<Restore>(value.target_player_generation, record.target_player_generation);
    snapshot_field<Restore>(value.type, record.slot_type);
    snapshot_field<Restore>(value.kind, record.kind);
    snapshot_union<Restore>(value.base_render_id, record.base_render_id);
    snapshot_union<Restore>(value.render_id, record.render_id);
    snapshot_field<Restore>(value.animation_clip, record.animation_clip);
    snapshot_field<Restore>(value.sound_played, record.sound_played);
    snapshot_field<Restore>(value.id, record.id);
    snapshot_field<Restore>(value.phase, record.phase);
    snapshot_union<Restore>(value.visual, record.visual);
    snapshot_field<Restore>(value.unknown_0a, record.unknown_0a);
    snapshot_field<Restore>(value.position.vx, record.position_vx);
    snapshot_field<Restore>(value.position.vy, record.position_vy);
    snapshot_field<Restore>(value.position.vz, record.position_vz);
    snapshot_field<Restore>(value.rotation.vector.vx, record.rotation_vector_vx);
    snapshot_field<Restore>(value.rotation.vector.vy, record.rotation_vector_vy);
    snapshot_field<Restore>(value.rotation.vector.vz, record.rotation_vector_vz);
    snapshot_field<Restore>(value.scale_x, record.scale_x);
    snapshot_field<Restore>(value.scale_y, record.scale_y);
    snapshot_field<Restore>(value.scale_z, record.scale_z);
    snapshot_field<Restore>(value.unknown_2a, record.unknown_2a);
    snapshot_field<Restore>(value.direction.vector.vx, record.direction_vector_vx);
    snapshot_field<Restore>(value.direction.vector.vy, record.direction_vector_vy);
    snapshot_field<Restore>(value.direction.vector.vz, record.direction_vector_vz);
    snapshot_union<Restore>(value.control, record.control);
    snapshot_union<Restore>(value.propagation, record.propagation);
}

template<bool Restore>
static void snapshot_object(KfMapObject &value, KfNetWorldObject &record)
{
    snapshot_field<Restore>(value.generation, record.generation);
    snapshot_field<Restore>(value.object_id, record.object_id);
    snapshot_field<Restore>(value.unknown_01, record.unknown_01);
    snapshot_field<Restore>(value.cell_x, record.cell_x);
    snapshot_field<Restore>(value.cell_z, record.cell_z);
    snapshot_field<Restore>(value.unknown_06, record.unknown_06);
    snapshot_field<Restore>(value.position.vx, record.position_vx);
    snapshot_field<Restore>(value.position.vy, record.position_vy);
    snapshot_field<Restore>(value.position.vz, record.position_vz);
    snapshot_field<Restore>(value.rotation.vector.vx, record.rotation_vector_vx);
    snapshot_field<Restore>(value.rotation.vector.vy, record.rotation_vector_vy);
    snapshot_field<Restore>(value.rotation.vector.vz, record.rotation_vector_vz);
    snapshot_field<Restore>(value.link.words, record.link_words);
    snapshot_field<Restore>(value.action, record.action);
    snapshot_field<Restore>(value.unknown_29, record.unknown_29);
    snapshot_field<Restore>(value.action_timer, record.action_timer);
}

template<bool Restore>
static void snapshot_event(KfMapEvent &value, KfNetWorldEvent &record)
{
    snapshot_field<Restore>(value.state, record.state);
    snapshot_field<Restore>(value.character_id, record.character_id);
    snapshot_field<Restore>(value.model_index, record.model_index);
    snapshot_field<Restore>(value.dialogue_pages.last_page, record.dialogue_pages_last_page);
    snapshot_field<Restore>(value.dialogue.stage_limit, record.dialogue_stage_limit);
    snapshot_field<Restore>(value.dialogue.stage, record.dialogue_stage);
    snapshot_field<Restore>(value.dialogue.page, record.dialogue_page);
    snapshot_field<Restore>(value.dialogue.page_delay, record.dialogue_page_delay);
    snapshot_field<Restore>(value.unknown_0c, record.unknown_0c);
    snapshot_field<Restore>(value.unknown_0d, record.unknown_0d);
    snapshot_field<Restore>(value.behavior, record.behavior);
    snapshot_field<Restore>(value.animation_clip, record.animation_clip);
    snapshot_field<Restore>(value.collision_turn_pending, record.collision_turn_pending);
    snapshot_field<Restore>(value.unknown_11, record.unknown_11);
    snapshot_field<Restore>(value.animation_phase, record.animation_phase);
    snapshot_field<Restore>(value.home_x, record.home_x);
    snapshot_field<Restore>(value.home_z, record.home_z);
    snapshot_field<Restore>(value.cell_x, record.cell_x);
    snapshot_field<Restore>(value.cell_z, record.cell_z);
    snapshot_field<Restore>(value.radius, record.radius);
    snapshot_field<Restore>(value.unknown_22, record.unknown_22);
    snapshot_field<Restore>(value.reference_position.vx, record.reference_position_vx);
    snapshot_field<Restore>(value.reference_position.vy, record.reference_position_vy);
    snapshot_field<Restore>(value.reference_position.vz, record.reference_position_vz);
    snapshot_field<Restore>(value.rotation.vx, record.rotation_vx);
    snapshot_field<Restore>(value.rotation.vy, record.rotation_vy);
    snapshot_field<Restore>(value.rotation.vz, record.rotation_vz);
    snapshot_field<Restore>(value.rotation_target, record.rotation_target);
    snapshot_field<Restore>(value.unknown_42, record.unknown_42);
}
