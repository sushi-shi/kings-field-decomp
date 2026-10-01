//! Explicit world wire schema. Typed C records are storage, never the encoding.
use super::{Error, Result};
use crate::ffi::bindings::*;

struct Reader<'a> {
    bytes: &'a [u8],
    at: usize,
}
impl<'a> Reader<'a> {
    fn new(bytes: &'a [u8]) -> Result<Self> {
        if bytes.len() > KF_NET_TRANSFER_LIMIT as usize {
            return Err(Error::Invalid);
        }
        Ok(Self { bytes, at: 0 })
    }
    fn take<const N: usize>(&mut self) -> Result<[u8; N]> {
        let end = self.at.checked_add(N).ok_or(Error::Invalid)?;
        let value = self
            .bytes
            .get(self.at..end)
            .ok_or(Error::Invalid)?
            .try_into()
            .map_err(|_| Error::Invalid)?;
        self.at = end;
        Ok(value)
    }
    fn value<T: Wire + Default>(&mut self) -> Result<T> {
        let mut value = T::default();
        value.read(self)?;
        Ok(value)
    }
}
struct Writer {
    bytes: Vec<u8>,
}
impl Writer {
    fn put(&mut self, bytes: &[u8]) -> Result<()> {
        if bytes.len() > KF_NET_TRANSFER_LIMIT as usize - self.bytes.len() {
            return Err(Error::Full);
        }
        self.bytes.extend_from_slice(bytes);
        Ok(())
    }
}
trait Wire {
    fn read(&mut self, reader: &mut Reader<'_>) -> Result<()>;
    fn write(&self, writer: &mut Writer) -> Result<()>;
}
macro_rules! integer {
    ($($t:ty),*) => { $(impl Wire for $t {
        fn read(&mut self, reader: &mut Reader<'_>) -> Result<()> { *self = Self::from_le_bytes(reader.take()?); Ok(()) }
        fn write(&self, writer: &mut Writer) -> Result<()> { writer.put(&self.to_le_bytes()) }
    })* };
}
integer!(u8, u16, u32, u64, i16, i32);
impl<T: Wire, const N: usize> Wire for [T; N] {
    fn read(&mut self, reader: &mut Reader<'_>) -> Result<()> {
        for value in self {
            value.read(reader)?;
        }
        Ok(())
    }
    fn write(&self, writer: &mut Writer) -> Result<()> {
        for value in self {
            value.write(writer)?;
        }
        Ok(())
    }
}
// Each list fixes wire order explicitly and shares it between encode/decode.
macro_rules! record {
    ($t:ty, $($field:ident),* $(,)?) => { impl Wire for $t {
        fn read(&mut self, reader: &mut Reader<'_>) -> Result<()> { $(self.$field.read(reader)?;)* Ok(()) }
        fn write(&self, writer: &mut Writer) -> Result<()> { $(self.$field.write(writer)?;)* Ok(()) }
    } };
}

record!(
    KfNetWorldSound,
    sequence,
    x,
    y,
    z,
    program,
    tone,
    note,
    volume,
    max_distance,
    attenuation_distance,
);
record!(
    KfNetWorldStory,
    kind,
    initiator,
    effect,
    tick,
    object,
    generation,
    camera_x,
    camera_y,
    camera_z,
    pitch,
    yaw,
    roll,
    page,
    ready_mask
);
record!(
    KfNetWorldPlayer,
    experience,
    next_level_experience,
    progress_state_level,
    progress_state_unknown_01,
    progress_state_current_floor,
    progress_state_highest_floor,
    map_variant,
    allow_near_actor_spawn,
    weapon_charge_delay,
    unknown_0f,
    vitals_maximum_hp,
    vitals_current_hp,
    vitals_maximum_mp,
    vitals_current_mp,
    attack_charge_state_current,
    attack_charge_state_committed,
    magic_charge,
    physical_power_training,
    magic_training,
    base_physical_power,
    base_magic,
    physical_power,
    magic,
    status_effect_flags,
    gold,
    cutting_attack,
    striking_attack,
    piercing_attack,
    holy_attack,
    fire_attack,
    unknown_3a,
    cutting_defense,
    striking_defense,
    piercing_defense,
    poison_resistance,
    magic_defense,
    fire_defense,
    curse_timer,
    darkness_timer,
    poison_timer,
    slowed_timer,
    fire_defense_timer,
    illusion_staff_timer,
    unknown_54,
    equipment_effect_ticks,
    selected_magic_id,
    unknown_5d,
    equipped_weapon_id,
    unknown_65,
    weapon_attack_phase,
    unknown_72,
    weapon_magic_shots_remaining,
    weapon_magic_delay,
    weapon_attack_fully_charged,
    unknown_7b,
    equipped_head_armor_id,
    equipped_body_armor_id,
    equipped_shield_id,
    equipped_arm_armor_id,
    equipped_leg_armor_id,
    equipped_accessory_id,
    audio_effects_enabled,
    audio_music_enabled,
    hud_gauges_enabled,
    compass_enabled,
    view_rotation_offset_vx,
    view_rotation_offset_vy,
    view_rotation_offset_vz,
    update_state,
    unknown_a3,
    camera_position_vx,
    camera_position_vy,
    camera_position_vz,
    foot_height,
    camera_rotation_vx,
    camera_rotation_vy,
    camera_rotation_vz,
    motion_state_strafe_velocity,
    motion_state_forward_velocity,
    motion_state_movement_speed,
    motion_state_yaw_step,
    motion_state_pitch_step,
    motion_state_map_cell_x,
    motion_state_map_cell_z,
    previous_map_cell_x,
    previous_map_cell_z,
    unknown_ce,
    view_bob_offset,
    view_bob_phase,
    death_camera_pitch_step,
    death_visual_blend,
    vertical_velocity,
    vertical_state,
    unknown_df,
    party_slot,
    previous_input,
    cast_pose_ticks,
    movement_velocity_limit,
    turn_step_limit,
    item_stock,
    learned_magic,
    random_state,
);

record!(
    KfNetWorldActor,
    generation,
    random_state,
    target_player_slot,
    target_player_generation,
    transform_step,
    slot_state,
    definition_id,
    culling_mode,
    heading_quadrant,
    tile_z,
    tile_x,
    lifecycle,
    spawn_chance,
    action,
    death_drop_object_id,
    animation_clip,
    vertical_state,
    unknown_0c,
    local_z,
    local_x,
    animation_phase,
    health,
    cell_x,
    cell_z,
    unknown_1a,
    position_vx,
    position_vy,
    position_vz,
    rotation_vector_vx,
    rotation_vector_vy,
    rotation_vector_vz,
    action_progress,
    collision_state,
    movement_yaw,
    animation_step,
    vertical_velocity,
    movement_x,
    movement_z,
    movement_y,
    unknown_46,
);

record!(
    KfNetWorldEffect,
    owner_player_slot,
    owner_player_generation,
    generation,
    age,
    random_state,
    target_player_slot,
    target_player_generation,
    slot_type,
    kind,
    base_render_id,
    render_id,
    animation_clip,
    sound_played,
    id,
    phase,
    visual,
    unknown_0a,
    position_vx,
    position_vy,
    position_vz,
    rotation_vector_vx,
    rotation_vector_vy,
    rotation_vector_vz,
    scale_x,
    scale_y,
    scale_z,
    unknown_2a,
    direction_vector_vx,
    direction_vector_vy,
    direction_vector_vz,
    control,
    propagation,
);

record!(
    KfNetWorldObject,
    generation,
    object_id,
    unknown_01,
    cell_x,
    cell_z,
    unknown_06,
    position_vx,
    position_vy,
    position_vz,
    rotation_vector_vx,
    rotation_vector_vy,
    rotation_vector_vz,
    link_words,
    action,
    unknown_29,
    action_timer,
);

record!(
    KfNetWorldEvent,
    state,
    character_id,
    model_index,
    dialogue_pages_last_page,
    dialogue_stage_limit,
    dialogue_stage,
    dialogue_page,
    dialogue_page_delay,
    unknown_0c,
    unknown_0d,
    behavior,
    animation_clip,
    collision_turn_pending,
    unknown_11,
    animation_phase,
    home_x,
    home_z,
    cell_x,
    cell_z,
    radius,
    unknown_22,
    reference_position_vx,
    reference_position_vy,
    reference_position_vz,
    rotation_vx,
    rotation_vy,
    rotation_vz,
    rotation_target,
    unknown_42,
);

fn require(valid: bool) -> Result<()> {
    if valid {
        Ok(())
    } else {
        Err(Error::Invalid)
    }
}
impl Wire for KfNetWorldHeader {
    fn read(&mut self, r: &mut Reader<'_>) -> Result<()> {
        require(r.value::<u32>()? == 0x3153464b && r.value::<u16>()? == 11)?;
        self.full.read(r)?;
        self.epoch.read(r)?;
        self.tick.read(r)?;
        self.floor.read(r)?;
        self.variant.read(r)?;
        header_valid(self)
    }
    fn write(&self, w: &mut Writer) -> Result<()> {
        header_valid(self)?;
        0x3153464bu32.write(w)?;
        11u16.write(w)?;
        self.full.write(w)?;
        self.epoch.write(w)?;
        self.tick.write(w)?;
        self.floor.write(w)?;
        self.variant.write(w)
    }
}
fn header_valid(h: &KfNetWorldHeader) -> Result<()> {
    let variant_valid = match h.floor {
        1..=4 => h.variant == 0,
        5 => (1..=3).contains(&h.variant),
        _ => false,
    };
    // Header inspection precedes loading floor assets in the native client.
    require(h.full <= 1 && h.epoch != 0 && variant_valid)
}
impl Wire for KfNetWorldMember {
    fn read(&mut self, r: &mut Reader<'_>) -> Result<()> {
        self.presence.read(r)?;
        self.character_id.read(r)?;
        self.generation.read(r)?;
        self.acknowledged_input.read(r)?;
        self.quest_rewards.read(r)?;
        self.connected.read(r)?;
        self.avatar.read(r)?;
        self.loot_claims.read(r)?;
        require(self.presence <= 4 && self.connected <= 1)?;
        if self.presence >= 2 {
            self.player.read(r)?;
        }
        Ok(())
    }
    fn write(&self, w: &mut Writer) -> Result<()> {
        self.presence.write(w)?;
        self.character_id.write(w)?;
        self.generation.write(w)?;
        self.acknowledged_input.write(w)?;
        self.quest_rewards.write(w)?;
        self.connected.write(w)?;
        self.avatar.write(w)?;
        self.loot_claims.write(w)?;
        if self.presence >= 2 {
            self.player.write(w)?;
        }
        Ok(())
    }
}
fn read_grid(r: &mut Reader<'_>, cells: &mut [u8]) -> Result<()> {
    let mut at = 0;
    while at < cells.len() {
        let count = r.value::<u16>()? as usize;
        let value = r.value::<u8>()?;
        require(count != 0 && count <= cells.len() - at)?;
        cells[at..at + count].fill(value);
        at += count;
    }
    Ok(())
}
fn write_grid(w: &mut Writer, cells: &[u8]) -> Result<()> {
    let mut at = 0;
    while at < cells.len() {
        let value = cells[at];
        let count = cells[at..]
            .iter()
            .take(u16::MAX as usize)
            .take_while(|&&v| v == value)
            .count();
        (count as u16).write(w)?;
        value.write(w)?;
        at += count;
    }
    Ok(())
}
impl Wire for KfNetWorld {
    fn read(&mut self, r: &mut Reader<'_>) -> Result<()> {
        self.header.read(r)?;
        self.quest_rewards.read(r)?;
        self.members.read(r)?;
        self.actors.read(r)?;
        self.action_animations.read(r)?;
        self.effects.read(r)?;
        self.objects.read(r)?;
        self.events.read(r)?;
        self.gold_drop_sequence.read(r)?;
        self.definition_drop_sequence.read(r)?;
        self.placement_drop_sequence.read(r)?;
        self.dialogue_advance_gate.read(r)?;
        self.ambient_script_countdown.read(r)?;
        for floor in &mut self.floors {
            floor.script.read(r)?;
            if self.header.full != 0 {
                floor.records.read(r)?;
            }
        }
        self.sound_sequence.read(r)?;
        self.sounds.read(r)?;
        for grid in [
            &mut self.collision_flags,
            &mut self.cell_orientation,
            &mut self.floor_height,
            &mut self.collision,
            &mut self.cell_attribute,
        ] {
            read_grid(r, grid)?;
        }
        self.random_state.read(r)?;
        self.story.read(r)?;
        Ok(())
    }
    fn write(&self, w: &mut Writer) -> Result<()> {
        self.header.write(w)?;
        self.quest_rewards.write(w)?;
        self.members.write(w)?;
        self.actors.write(w)?;
        self.action_animations.write(w)?;
        self.effects.write(w)?;
        self.objects.write(w)?;
        self.events.write(w)?;
        self.gold_drop_sequence.write(w)?;
        self.definition_drop_sequence.write(w)?;
        self.placement_drop_sequence.write(w)?;
        self.dialogue_advance_gate.write(w)?;
        self.ambient_script_countdown.write(w)?;
        for floor in &self.floors {
            floor.script.write(w)?;
            if self.header.full != 0 {
                floor.records.write(w)?;
            }
        }
        self.sound_sequence.write(w)?;
        self.sounds.write(w)?;
        for grid in [
            &self.collision_flags,
            &self.cell_orientation,
            &self.floor_height,
            &self.collision,
            &self.cell_attribute,
        ] {
            write_grid(w, grid)?;
        }
        self.random_state.write(w)?;
        self.story.write(w)?;
        Ok(())
    }
}
fn optional_index(id: u8, count: u8) -> bool {
    id == 255 || id < count
}
fn position(x: i32, z: i32) -> bool {
    (0..200000).contains(&x) && (0..200000).contains(&z)
}
fn coordinate(value: i32) -> bool {
    // Leave room for falls, cinematic offsets and a projectile's final step,
    // while keeping native coordinate differences/additions away from overflow.
    (-1_000_000..=1_000_000).contains(&value)
}
fn clip_valid(limits: &KfNetWorldLimits, asset: usize, clip: u8) -> bool {
    limits
        .asset_clips
        .get(asset)
        .is_some_and(|&count| count == 0 || (count > 0 && i32::from(clip) < count))
}
fn actor_action_clip(action: u8) -> Option<usize> {
    match action {
        0 => Some(0),
        1..=3 | 32 | 33 => Some(1),
        4 => Some(2),
        5 => Some(3),
        6 => Some(4),
        16..=22 => Some(usize::from(action - 11)),
        _ => None,
    }
}
fn effect_billboard_frames(kind: u8) -> Option<u8> {
    match kind {
        4 => Some(2), // Lightning Bolt, including the canonicalized alternate.
        5 => Some(5), // Fire Ball impact.
        7 | 10..=12 => Some(1),
        19 | 32 => Some(3), // Ground trail / Lightning impact.
        _ => None,
    }
}
fn link_valid(operation: u8, bytes: &[u8; 8]) -> bool {
    match operation {
        0 | 1 => optional_index(bytes[1], 190),
        10 => optional_index(bytes[1], 5),
        81..=83 => bytes[1] < 48,
        8 => bytes[1..5].iter().all(|&id| optional_index(id, 80)),
        9 => bytes[..4].iter().all(|&id| optional_index(id, 80)),
        _ => true,
    }
}
fn saved_floor_valid(bytes: &[u8; 1690], limits: &KfNetWorldLimits) -> Result<()> {
    let mut r = Reader::new(bytes)?;
    match r.value::<u8>()? {
        0 => return Ok(()),
        1 => (),
        _ => return Err(Error::Invalid),
    }
    for _ in 0..8 {
        let e = r.take::<7>()?;
        require(
            matches!(e[0], 0 | 1 | 3 | 255) && e[1] <= 5 && e[2] <= 5 && (e[0] == 255 || e[2] != 0),
        )?;
    }
    let actors = r.value::<u8>()?;
    require(actors <= 128)?;
    for _ in 0..actors {
        let a = r.take::<2>()?;
        require(a[0] < 128 && matches!(a[1], 0 | 3))?;
    }
    let ids = r.take::<190>()?;
    for (i, &id) in ids.iter().enumerate() {
        require(
            optional_index(id, 160)
                && (id == 255 || i < 160 || if i < 170 { id == 39 } else { id < 80 }),
        )?;
    }
    let objects = r.value::<u8>()?;
    require(objects <= 160)?;
    for _ in 0..objects {
        let index = r.value::<u8>()? as usize;
        require(index < 160)?;
        let id = ids[index] as usize;
        let operation = *limits.object_operations.get(id).ok_or(Error::Invalid)?;
        require(link_valid(operation, &r.take()?))?;
    }
    for group in 0..3 {
        for _ in 0..10 {
            require(r.value::<u8>()? < 100 && r.value::<u8>()? < 100)?;
            r.value::<u8>()?;
            if group == 0 {
                r.value::<u8>()?;
            }
        }
    }
    Ok(())
}
fn validate_party(world: &KfNetWorld) -> Result<()> {
    header_valid(&world.header)?;
    for (slot, sound) in world.sounds.iter().enumerate() {
        let age = (world.sound_sequence % KF_WORLD_SOUNDS + KF_WORLD_SOUNDS - slot as u32)
            % KF_WORLD_SOUNDS;
        let expected = world.sound_sequence.saturating_sub(age);
        require(sound.sequence == expected)?;
        if expected == 0 {
            continue;
        }
        require(
            sound.program <= 127
                && sound.tone <= 15
                && sound.note <= 127
                && (sound.program != 0 || sound.tone != 0 || sound.note != 0)
                && (1..=127).contains(&sound.volume)
                && (1..=60000).contains(&sound.max_distance)
                && (sound.max_distance..=60000).contains(&sound.attenuation_distance)
                && [sound.x, sound.y, sound.z]
                    .iter()
                    .all(|v| (-1000000..=1000000).contains(v)),
        )?;
    }
    let s = &world.story;
    require(s.kind <= 4)?;
    if s.kind == 4 {
        require(
            matches!(world.header.floor, 1 | 5)
                && world.floors[4].script[3] == 1
                && s.initiator == 0
                && s.effect == 255
                && s.page == 0
                && s.tick == 0
                && s.ready_mask < 16
                && s.ready_mask & 1 == 1,
        )?;
    } else if s.kind == 3 {
        require(world.header.floor == 5 && s.page <= 2 && s.ready_mask < 16 && s.effect == 255)?;
        require(if s.page == 0 {
            s.tick < 2 && s.ready_mask == 0
        } else {
            s.tick == 0
        })?;
    } else {
        require(s.page == 0 && s.ready_mask == 0)?;
    }
    if s.kind != 0 && s.effect != 255 {
        require(s.kind == 2 && (361..373).contains(&s.tick) && s.effect < 48)?;
        let effect = &world.effects[s.effect as usize];
        require(
            effect.slot_type == 0 && effect.kind == 18 && effect.phase == (s.tick - 360) as u8,
        )?;
    }
    if s.kind != 0 {
        require(
            s.initiator < 4
                && world.members[s.initiator as usize].presence >= 2
                && position(s.camera_x, s.camera_z)
                && (-1_000_000..=1_000_000).contains(&s.camera_y),
        )?;
        if s.kind == 1 {
            require(world.header.floor == 2 && s.tick < 50)?;
        } else if s.kind == 2 {
            require(
                world.header.floor == 5
                    && s.tick <= 620
                    && (180..190).contains(&s.object)
                    && s.generation != 0,
            )?;
            let object = &world.objects[s.object as usize];
            require(
                object.generation == s.generation
                    && object.object_id
                        == if s.tick <= 100 {
                            255
                        } else if s.tick <= 360 {
                            10
                        } else {
                            11
                        },
            )?;
        }
    }
    require(world.quest_rewards & !KF_WORLD_QUEST_REWARD_MASK == 0)?;
    for (slot, member) in world.members.iter().enumerate() {
        if member.character_id != [0; 32] {
            require(
                world.members[..slot]
                    .iter()
                    .all(|prior| prior.character_id != member.character_id),
            )?;
        }
        require(member.quest_rewards & !world.quest_rewards == 0)?;
        require(member.presence <= 4 && member.connected <= 1)?;
        require(u32::from(member.avatar) < KF_AVATAR_SLOTS)?;
        require(member.loot_claims.iter().flatten().all(|&mask| mask < 16))?;
        if member.presence < 2 {
            continue;
        }
        let p = &member.player;
        require(
            p.party_slot as usize == slot
                && u32::from(p.cast_pose_ticks) <= KF_WORLD_CAST_POSE_TICKS
                && p.progress_state_level != 0
                && i32::from(p.progress_state_current_floor) == world.header.floor
                && p.map_variant == world.header.variant
                && (1..=5).contains(&p.progress_state_highest_floor)
                && p.vitals_maximum_hp != 0
                && p.vitals_maximum_mp != 0
                && p.vitals_current_hp <= p.vitals_maximum_hp
                && p.vitals_current_mp <= p.vitals_maximum_mp
                && optional_index(p.equipped_weapon_id, 16)
                && optional_index(p.selected_magic_id, 9)
                && optional_index(p.equipped_accessory_id, 80)
                && position(p.camera_position_vx, p.camera_position_vz)
                && coordinate(p.camera_position_vy)
                && coordinate(p.foot_height)
                && p.motion_state_map_cell_x < 100
                && p.motion_state_map_cell_z < 100
                && p.learned_magic.iter().all(|&v| v <= 1),
        )?;
        for id in [
            p.equipped_head_armor_id,
            p.equipped_body_armor_id,
            p.equipped_shield_id,
            p.equipped_arm_armor_id,
            p.equipped_leg_armor_id,
        ] {
            require(id == 255 || (13..55).contains(&id))?;
        }
    }
    Ok(())
}
fn validate(world: &KfNetWorld, limits: &KfNetWorldLimits) -> Result<()> {
    validate_party(world)?;
    for ((&attribute, &orientation), &collision) in world
        .cell_attribute
        .iter()
        .zip(&world.cell_orientation)
        .zip(&world.collision)
    {
        // Rendered cells index the four rotation matrices after subtracting one.
        // Attributes outside 1..=100 have no mesh and never index those matrices.
        require(!(1..=100).contains(&attribute) || (1..=4).contains(&orientation))?;
        require(collision <= 6)?;
    }
    for a in &world.actors {
        if a.slot_state == 255 {
            continue;
        }
        require(
            a.slot_state <= 3
                && a.definition_id < 12
                && a.lifecycle <= 3
                && a.cell_x < 100
                && a.cell_z < 100
                && position(a.position_vx, a.position_vz)
                && coordinate(a.position_vy)
                && optional_index(a.target_player_slot, 4)
                && a.culling_mode <= 1
                && a.heading_quadrant <= 3
                && a.vertical_state <= 4
                && a.collision_state <= 2
                && optional_index(a.death_drop_object_id, 160)
                && matches!(a.action, 0..=6 | 16..=22 | 32 | 33 | 127 | 255),
        )?;
        if a.slot_state != 0 {
            // Dynamic actors have no home tile; placed actors can respawn there.
            require(
                a.tile_x < 100
                    && a.tile_z < 100
                    && position(
                        i32::from(a.tile_x) * 2000 + i32::from(a.local_x),
                        i32::from(a.tile_z) * 2000 + i32::from(a.local_z),
                    ),
            )?;
        }
        let definition = a.definition_id as usize;
        require(clip_valid(
            limits,
            limits.actor_assets[definition] as usize,
            a.animation_clip,
        ))?;
        if a.lifecycle == 1 && a.action_progress == 0 {
            if let Some(action_clip) = actor_action_clip(a.action) {
                // The next update installs this clip, replacing the current one.
                require(clip_valid(
                    limits,
                    limits.actor_assets[definition] as usize,
                    world.action_animations[definition][action_clip],
                ))?;
            }
        }
    }
    for (definition, clips) in world.action_animations.iter().enumerate() {
        let asset = limits.actor_assets[definition] as usize;
        let absent = limits.asset_clips.get(asset).is_none_or(|&count| count < 0);
        for &clip in clips {
            // Definitions can spawn later, even if no current actor uses them.
            require(
                clip == 255
                    || if absent {
                        clip == 0
                    } else {
                        clip_valid(limits, asset, clip)
                    },
            )?;
        }
    }
    for e in &world.effects {
        if e.slot_type == 255 {
            continue;
        }
        require(
            matches!(e.kind, 0..=24 | 32..=34 | 36 | 41 | 42 | 44 | 48 | 52)
                && optional_index(e.owner_player_slot, 4)
                && optional_index(e.target_player_slot, 4),
        )?;
        if e.kind == 19 {
            require(e.control < 48)?;
        }
        if let Some(frames) = effect_billboard_frames(e.kind) {
            // Validate the whole sequence, not just the currently displayed sprite.
            require(
                e.animation_clip == 255
                    && e.base_render_id <= 22 - frames
                    && (e.base_render_id..e.base_render_id + frames).contains(&e.render_id),
            )?;
        }
        if e.kind == 52 {
            require(
                e.rotation_vector_vx >= 0
                    && e.rotation_vector_vy >= 0
                    && i32::from(e.rotation_vector_vx) + i32::from(e.rotation_vector_vy) <= 5,
            )?;
            // These union fields are sweep/hold counters, not world coordinates.
            require((0..=32767).contains(&e.position_vx) && e.position_vy >= -1)?;
        } else {
            require(
                coordinate(e.position_vx) && coordinate(e.position_vy) && coordinate(e.position_vz),
            )?;
        }
        if e.render_id != 255 {
            if e.animation_clip == 255 {
                require(e.render_id < 22 && optional_index(e.base_render_id, 22))?;
            } else {
                require(clip_valid(
                    limits,
                    e.render_id as usize + 30,
                    e.animation_clip,
                ))?;
            }
        }
    }
    for o in &world.objects {
        if o.object_id == 255 {
            continue;
        }
        require(o.generation != 0 && o.object_id < 160 && o.cell_x < 100 && o.cell_z < 100)?;
        require(position(o.position_vx, o.position_vz) && coordinate(o.position_vy))?;
        let operation = limits.object_operations[o.object_id as usize];
        if matches!(operation, 0 | 2 | 3) {
            // Door collision updates touch neighboring cells on both axes.
            require((1..99).contains(&o.cell_x) && (1..99).contains(&o.cell_z))?;
        }
        let mut link = [0; 8];
        link[..4].copy_from_slice(&o.link_words[0].to_le_bytes());
        link[4..].copy_from_slice(&o.link_words[1].to_le_bytes());
        require(link_valid(o.action, &link) && link_valid(operation, &link))?;
    }
    for e in &world.events {
        require(matches!(e.state, 0 | 1 | 3 | 255))?;
        if e.state != 1 {
            continue;
        }
        require(
            e.cell_x < 100
                && e.cell_z < 100
                && position(e.reference_position_vx, e.reference_position_vz)
                && coordinate(e.reference_position_vy)
                && position(e.home_x, e.home_z)
                && (1..=5).contains(&e.dialogue_stage)
                && e.dialogue_stage_limit <= 5
                && e.behavior <= 2
                && clip_valid(limits, e.model_index as usize + 10, e.animation_clip),
        )?;
        if e.behavior == 0 {
            require((1..=2).contains(&e.character_id))?;
        }
    }
    if world.header.full != 0 {
        for floor in &world.floors {
            saved_floor_valid(&floor.records, limits)?;
        }
    }
    Ok(())
}
pub(crate) fn info(bytes: &[u8]) -> Result<KfNetWorldHeader> {
    let mut r = Reader::new(bytes)?;
    let mut header = KfNetWorldHeader {
        epoch: 0,
        tick: 0,
        floor: 0,
        full: 0,
        variant: 0,
    };
    header.read(&mut r)?;
    Ok(header)
}
pub(crate) fn encode(world: &KfNetWorld, limits: &KfNetWorldLimits) -> Result<Vec<u8>> {
    validate(world, limits)?;
    let mut w = Writer { bytes: Vec::new() };
    world.write(&mut w)?;
    Ok(w.bytes)
}
pub(crate) fn decode(
    bytes: &[u8],
    limits: &KfNetWorldLimits,
    world: &mut KfNetWorld,
) -> Result<()> {
    let mut r = Reader::new(bytes)?;
    world.read(&mut r)?;
    require(r.at == bytes.len())?;
    validate(world, limits)
}

pub(crate) fn summary(bytes: &[u8], scratch: &mut KfNetWorld) -> Result<KfNetWorldSummary> {
    let mut r = Reader::new(bytes)?;
    scratch.read(&mut r)?;
    require(r.at == bytes.len() && scratch.header.full == 1)?;
    validate_party(scratch)?;
    let host = &scratch.members[0];
    require(host.presence >= 2 && host.character_id != [0; 32])?;
    let p = &host.player;
    Ok(KfNetWorldSummary {
        experience: p.experience.try_into().map_err(|_| Error::Invalid)?,
        hp: p.vitals_current_hp,
        maximum_hp: p.vitals_maximum_hp,
        mp: p.vitals_current_mp,
        maximum_mp: p.vitals_maximum_mp,
        floor: scratch.header.floor as u8,
        level: p.progress_state_level,
        owner: host.character_id,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ffi::world::new_world;

    fn fixture() -> (Box<KfNetWorld>, KfNetWorldLimits) {
        let mut world = new_world();
        world.header = KfNetWorldHeader {
            epoch: 7,
            tick: 0x12345678,
            floor: 1,
            full: 1,
            variant: 0,
        };
        for a in &mut world.actors {
            a.slot_state = 255;
        }
        for e in &mut world.effects {
            e.slot_type = 255;
        }
        for o in &mut world.objects {
            o.generation = 1;
            o.object_id = 255;
        }
        for e in &mut world.events {
            e.state = 255;
        }
        let limits = KfNetWorldLimits {
            asset_clips: [-1; 48],
            actor_assets: [0; 12],
            object_operations: [255; 160],
        };
        (world, limits)
    }
    fn unchecked_bytes(world: &KfNetWorld) -> Vec<u8> {
        let mut w = Writer { bytes: Vec::new() };
        world.write(&mut w).unwrap();
        w.bytes
    }
    #[test]
    fn appended_avatar_survives_full_and_fast_snapshots() {
        let (mut world, limits) = fixture();
        let mut restored = new_world();
        for full in [0, 1] {
            world.header.full = full;
            world.members[0].avatar = 43;
            decode(&encode(&world, &limits).unwrap(), &limits, &mut restored).unwrap();
            assert_eq!(restored.members[0].avatar, 43);
            world.members[0].avatar = KF_AVATAR_SLOTS as u8;
            assert!(encode(&world, &limits).is_err());
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
        }
    }
    #[test]
    fn world_wire_roundtrip_and_run_bounds() {
        let (mut world, limits) = fixture();
        world.floor_height[9999] = 42;
        world.actors[0].random_state = 0x98765432;
        world.effects[0].age = u32::MAX;
        world.random_state = 0xabcdef01;
        world.gold_drop_sequence = 65535; // monotonic drop age, not a pool index
        world.members[0].character_id = [0xa5; 32];
        let bytes = encode(&world, &limits).unwrap();
        assert_eq!(
            &bytes[..20],
            &[b'K', b'F', b'S', b'1', 11, 0, 1, 7, 0, 0, 0, 0x78, 0x56, 0x34, 0x12, 1, 0, 0, 0, 0]
        );
        let mut restored = new_world();
        decode(&bytes, &limits, &mut restored).unwrap();
        assert_eq!(restored.floor_height[9999], 42);
        assert_eq!(restored.actors[0].random_state, 0x98765432);
        assert_eq!(restored.effects[0].age, u32::MAX);
        assert_eq!(restored.random_state, 0xabcdef01);
        let mut old_version = bytes.clone();
        old_version[4] = 9;
        assert!(decode(&old_version, &limits, &mut restored).is_err());
        assert_eq!(restored.members[0].character_id, [0xa5; 32]);
        world.members[1].character_id = world.members[0].character_id;
        assert!(encode(&world, &limits).is_err());
        world.members[1].character_id = [0; 32];
        assert_eq!(encode(&restored, &limits).unwrap(), bytes);
        for length in [0, 19, 20, bytes.len() - 1] {
            assert!(decode(&bytes[..length], &limits, &mut restored).is_err());
        }
        let mut invalid = bytes.clone();
        let last_run = invalid.len() - 35 - 3; // trailing story record follows the grids
        invalid[last_run..last_run + 2].copy_from_slice(&0u16.to_le_bytes());
        assert!(decode(&invalid, &limits, &mut restored).is_err());
        invalid[last_run..last_run + 2].copy_from_slice(&10001u16.to_le_bytes());
        assert!(decode(&invalid, &limits, &mut restored).is_err());
        invalid = bytes.clone();
        invalid.push(0);
        assert!(decode(&invalid, &limits, &mut restored).is_err());
        world.header.full = 0;
        let fast = encode(&world, &limits).unwrap();
        assert_eq!(bytes.len() - fast.len(), 5 * 1690);
        decode(&fast, &limits, &mut restored).unwrap();
    }
    #[test]
    fn grids_reject_invalid_render_rotations_and_collision_kinds() {
        let (mut world, limits) = fixture();
        let mut restored = new_world();
        for attribute in [1, 68, 100] {
            world.cell_attribute[123] = attribute;
            for rotation in 1..=4 {
                world.cell_orientation[123] = rotation;
                for collision in 0..=6 {
                    world.collision[123] = collision;
                    decode(&encode(&world, &limits).unwrap(), &limits, &mut restored).unwrap();
                }
            }
            for rotation in [0, 5, 255] {
                world.cell_orientation[123] = rotation;
                assert!(
                    decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
                    "invalid rotation {rotation} reached a rendered cell"
                );
                assert!(encode(&world, &limits).is_err());
            }
        }
        world.cell_attribute[123] = 255; // No mesh; its unused rotation is not indexed.
        for attribute in [0, 101, 255] {
            world.cell_attribute[123] = attribute;
            decode(&encode(&world, &limits).unwrap(), &limits, &mut restored).unwrap();
        }
        for collision in [7, 128, 255] {
            world.collision[123] = collision;
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
            assert!(encode(&world, &limits).is_err());
        }
    }
    #[test]
    fn sound_history_is_bounded_and_validated() {
        let (mut world, limits) = fixture();
        let empty = world.sounds[0];
        for latest in [1, 31, 32, 37, u32::MAX] {
            world.sound_sequence = latest;
            world.sounds.fill(empty);
            for age in 0..latest.min(KF_WORLD_SOUNDS) {
                let sequence = latest - age;
                world.sounds[(sequence % KF_WORLD_SOUNDS) as usize] = KfNetWorldSound {
                    sequence,
                    x: 100000,
                    y: -1700,
                    z: 100000,
                    program: 1,
                    tone: 2,
                    note: 60,
                    volume: 127,
                    max_distance: 16000,
                    attenuation_distance: 28000,
                };
            }
            let mut restored = new_world();
            let bytes = encode(&world, &limits).unwrap();
            decode(&bytes, &limits, &mut restored).unwrap();
            assert_eq!(restored.sound_sequence, latest);
            assert_eq!(encode(&restored, &limits).unwrap(), bytes);
            let slot = (latest % KF_WORLD_SOUNDS) as usize;
            let valid = world.sounds[slot];
            for bad in [
                KfNetWorldSound {
                    sequence: 0,
                    ..valid
                },
                KfNetWorldSound {
                    sequence: latest - 1,
                    ..valid
                },
                KfNetWorldSound {
                    x: i32::MAX,
                    ..valid
                },
                KfNetWorldSound {
                    y: i32::MIN,
                    ..valid
                },
                KfNetWorldSound { tone: 16, ..valid },
                KfNetWorldSound {
                    program: 128,
                    ..valid
                },
                KfNetWorldSound { note: 128, ..valid },
                KfNetWorldSound {
                    volume: 128,
                    ..valid
                },
                KfNetWorldSound {
                    max_distance: 60001,
                    ..valid
                },
                KfNetWorldSound {
                    attenuation_distance: 0,
                    ..valid
                },
                KfNetWorldSound {
                    attenuation_distance: 100,
                    ..valid
                },
            ] {
                world.sounds[slot] = bad;
                assert!(encode(&world, &limits).is_err());
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
            }
            world.sounds[slot] = valid;
        }
    }
    #[test]
    fn world_header_rejects_unavailable_floor_variants() {
        let (world, limits) = fixture();
        let mut bytes = encode(&world, &limits).unwrap();
        let mut restored = new_world();
        for floor in 1i32..=5 {
            for variant in 0u8..=4 {
                bytes[15..19].copy_from_slice(&floor.to_le_bytes());
                bytes[19] = variant;
                let available = if floor == 5 {
                    (1..=3).contains(&variant)
                } else {
                    variant == 0
                };
                assert_eq!(
                    info(&bytes).is_ok(),
                    available,
                    "header floor {floor} variant {variant}"
                );
                assert_eq!(decode(&bytes, &limits, &mut restored).is_ok(), available);
            }
        }
    }
    #[test]
    fn cast_pose_bounds() {
        let (mut world, limits) = fixture();
        let member = &mut world.members[0];
        member.presence = 2;
        let p = &mut member.player;
        p.progress_state_level = 1;
        p.progress_state_current_floor = 1;
        p.progress_state_highest_floor = 1;
        p.vitals_maximum_hp = 100;
        p.vitals_maximum_mp = 100;
        p.equipped_head_armor_id = 255;
        p.equipped_body_armor_id = 255;
        p.equipped_shield_id = 255;
        p.equipped_arm_armor_id = 255;
        p.equipped_leg_armor_id = 255;
        let mut restored = new_world();
        for full in [0, 1] {
            world.header.full = full;
            for ticks in [0, 1, 6, 12, 13, 255] {
                world.members[0].player.cast_pose_ticks = ticks;
                let bytes = unchecked_bytes(&world);
                assert_eq!(decode(&bytes, &limits, &mut restored).is_ok(), ticks <= 12);
                if ticks <= 12 {
                    assert_eq!(restored.members[0].player.cast_pose_ticks, ticks);
                }
            }
        }
    }
    #[test]
    fn story_rejects_invalid_phases_and_references() {
        let (mut world, limits) = fixture();
        world.header.floor = 5;
        world.header.variant = 1;
        let member = &mut world.members[0];
        member.presence = 2;
        let p = &mut member.player;
        p.progress_state_level = 1;
        p.progress_state_current_floor = 5;
        p.map_variant = 1;
        p.progress_state_highest_floor = 5;
        p.vitals_maximum_hp = 100;
        p.vitals_maximum_mp = 100;
        p.equipped_head_armor_id = 255;
        p.equipped_body_armor_id = 255;
        p.equipped_shield_id = 255;
        p.equipped_arm_armor_id = 255;
        p.equipped_leg_armor_id = 255;
        world.story = KfNetWorldStory {
            kind: 2,
            initiator: 0,
            effect: 255,
            tick: 0,
            object: 180,
            generation: 1,
            camera_x: 1000,
            camera_y: -1700,
            camera_z: 1000,
            pitch: 0,
            yaw: 0,
            roll: 0,
            page: 0,
            ready_mask: 0,
        };
        let bytes = encode(&world, &limits).unwrap();
        let mut restored = new_world();
        decode(&bytes, &limits, &mut restored).unwrap();
        // Explicit wire offsets, independent of the padded C ABI layout.
        for (offset, value) in [
            (0, 5),
            (1, 4),
            (2, 48),
            (3, 255),
            (4, 255),
            (5, 179),
            (6, 1),
            (7, 0),
            (14, 127),
            (18, 127),
            (22, 127),
        ] {
            let mut invalid = bytes.clone();
            invalid[bytes.len() - 31 + offset] = value;
            assert!(
                decode(&invalid, &limits, &mut restored).is_err(),
                "story offset {offset}"
            );
        }
        world.story.tick = 361;
        world.objects[180].object_id = 11;
        assert!(encode(&world, &limits).is_ok());
        world.story.effect = 0;
        assert!(encode(&world, &limits).is_err()); // freed effect cannot be advanced
        world.story.effect = 255;
        world.objects[180].object_id = 10;
        assert!(encode(&world, &limits).is_err()); // wrong transformation phase
        world.objects[180].object_id = 11;
        world.story.generation = 2;
        assert!(encode(&world, &limits).is_err()); // reused object slot
        world.story.kind = 3;
        world.story.tick = 0;
        world.story.page = 1;
        world.story.ready_mask = 1;
        let bytes = encode(&world, &limits).unwrap();
        decode(&bytes, &limits, &mut restored).unwrap();
        for (offset, value) in [(2, 0), (3, 1), (29, 3), (30, 16)] {
            let mut invalid = bytes.clone();
            invalid[bytes.len() - 31 + offset] = value;
            assert!(decode(&invalid, &limits, &mut restored).is_err());
        }
        world.story.page = 0;
        assert!(encode(&world, &limits).is_err()); // interlude cannot carry acknowledgements
        world.story.ready_mask = 0;
        world.story.tick = 1;
        assert!(encode(&world, &limits).is_ok());
        world.story.tick = 2;
        assert!(encode(&world, &limits).is_err());
        world.story.kind = 4;
        world.story.tick = 0;
        world.story.ready_mask = 1;
        assert!(encode(&world, &limits).is_err()); // undefeated campaign cannot end
        world.floors[4].script[3] = 1;
        let bytes = encode(&world, &limits).unwrap();
        decode(&bytes, &limits, &mut restored).unwrap();
        for (offset, value) in [(1, 1), (2, 0), (3, 1), (29, 1), (30, 0), (30, 16)] {
            let mut invalid = bytes.clone();
            invalid[bytes.len() - 31 + offset] = value;
            assert!(decode(&invalid, &limits, &mut restored).is_err());
        }
        world.header.floor = 2;
        world.header.variant = 0;
        world.members[0].player.map_variant = 0;
        world.members[0].player.progress_state_current_floor = 2;
        assert!(encode(&world, &limits).is_err());
        world.header.floor = 1;
        world.members[0].player.progress_state_current_floor = 1;
        assert!(encode(&world, &limits).is_ok()); // the other original exit
    }
    #[test]
    fn world_rejects_unsafe_combat_transitions() {
        let (mut world, mut limits) = fixture();
        limits.asset_clips.fill(1);
        let mut restored = new_world();
        world.actors[0].slot_state = 1;
        world.actors[0].lifecycle = 1;
        world.actors[0].death_drop_object_id = 99;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
        world.actors[0].death_drop_object_id = 160;
        assert!(
            decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
            "Invalid death drop escaped validation"
        );
        world.actors[0].death_drop_object_id = 255;
        world.actors[0].action = 16; // Jump will install this action's clip next tick.
        world.action_animations[0][5] = 255;
        assert!(
            decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
            "Pending action selected an unavailable animation"
        );
        world.action_animations[0][5] = 0;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
        world.actors[0].slot_state = 255;
        world.effects[0].slot_type = 0;
        world.effects[0].kind = 5;
        world.effects[0].animation_clip = 255;
        // Current sprite is in range, but base + impact phase would not be.
        world.effects[0].base_render_id = 21;
        world.effects[0].render_id = 21;
        assert!(
            decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
            "Fire Ball could advance outside the sprite table"
        );
        world.effects[0].base_render_id = 0;
        world.effects[0].render_id = 0;
        world.effects[0].animation_clip = 0;
        assert!(
            decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
            "Billboard frame updates could select unrelated model assets"
        );
        world.effects[0].animation_clip = 255;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
        for (kind, base, frames) in [
            (4, 17, 2),
            (5, 0, 5),
            (7, 5, 1),
            (10, 9, 1),
            (11, 8, 1),
            (12, 10, 1),
            (19, 14, 3),
            (32, 19, 3),
        ] {
            world.effects[0].kind = kind;
            world.effects[0].base_render_id = base;
            for frame in 0..frames {
                world.effects[0].render_id = base + frame;
                decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
            }
            world.effects[0].base_render_id = 255;
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
        }
    }
    #[test]
    fn world_rejects_unbounded_entity_coordinates() {
        let (mut world, mut limits) = fixture();
        limits.asset_clips.fill(0);
        let mut restored = new_world();
        world.actors[0].slot_state = 1;
        world.objects[0].object_id = 0;
        world.events[0].state = 1;
        world.events[0].dialogue_stage = 1;
        world.events[0].character_id = 1;
        world.effects[0].slot_type = 0;
        world.effects[0].kind = 5;
        world.members[0].presence = 2;
        world.effects[0].animation_clip = 255;
        let player = &mut world.members[0].player;
        player.progress_state_level = 1;
        player.progress_state_current_floor = 1;
        player.progress_state_highest_floor = 1;
        player.vitals_maximum_hp = 1;
        player.vitals_maximum_mp = 1;
        player.equipped_head_armor_id = 255;
        player.equipped_body_armor_id = 255;
        player.equipped_shield_id = 255;
        player.equipped_arm_armor_id = 255;
        player.equipped_leg_armor_id = 255;
        for full in [0, 1] {
            world.header.full = full;
            decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
            for bad in [i32::MIN, -1, 200000, i32::MAX] {
                world.actors[0].position_vx = bad;
                assert!(
                    decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
                    "actor x {bad} escaped validation"
                );
                world.actors[0].position_vx = 0;
                world.objects[0].position_vz = bad;
                assert!(
                    decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
                    "object z {bad} escaped validation"
                );
                world.objects[0].position_vz = 0;
                world.events[0].reference_position_vx = bad;
                assert!(
                    decode(&unchecked_bytes(&world), &limits, &mut restored).is_err(),
                    "NPC x {bad} escaped validation"
                );
                world.events[0].reference_position_vx = 0;
            }
            for bad in [i32::MIN, i32::MAX] {
                world.members[0].player.camera_position_vy = bad;
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
                world.members[0].player.camera_position_vy = 0;
                world.members[0].player.foot_height = bad;
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
                world.members[0].player.foot_height = 0;
                world.actors[0].position_vy = bad;
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
                world.actors[0].position_vy = 0;
                world.effects[0].position_vz = bad;
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
                world.effects[0].position_vz = 0;
                world.events[0].home_x = bad;
                assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
                world.events[0].home_x = 0;
            }
            world.actors[0].tile_x = 100;
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
            world.actors[0].tile_x = 0;
            world.actors[0].local_z = -1;
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
            world.actors[0].local_z = 0;
            // Dynamic actors deliberately have no placement/home tile.
            world.actors[0].slot_state = 0;
            world.actors[0].tile_x = 255;
            world.actors[0].tile_z = 255;
            // A projectile can cross the boundary during its final update.
            world.effects[0].position_vz = 200001;
            decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
            world.actors[0].slot_state = 1;
            world.actors[0].tile_x = 0;
            world.actors[0].tile_z = 0;
            world.effects[0].position_vz = 0;
            world.effects[0].kind = 52;
            world.effects[0].position_vy = i32::MIN;
            assert!(decode(&unchecked_bytes(&world), &limits, &mut restored).is_err());
            world.effects[0].position_vy = -1; // Finished hold counter survives reverse animation.
            decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
            world.effects[0].position_vy = 0;
            world.effects[0].kind = 5;
        }
    }
    #[test]
    fn world_rejects_remote_indices_and_persisted_links() {
        let (mut world, mut limits) = fixture();
        let mut restored = new_world();
        let rejected = |w: &KfNetWorld, l: &KfNetWorldLimits, out: &mut KfNetWorld| {
            assert!(decode(&unchecked_bytes(w), l, out).is_err());
        };
        world.members[0].presence = 5;
        rejected(&world, &limits, &mut restored);
        world.members[0].presence = 0;
        world.quest_rewards = 32;
        rejected(&world, &limits, &mut restored);
        world.quest_rewards = 1;
        world.members[0].quest_rewards = 2;
        rejected(&world, &limits, &mut restored);
        world.members[0].quest_rewards = 1;
        world.members[0].loot_claims[4][189] = 16;
        rejected(&world, &limits, &mut restored);
        world.members[0].loot_claims[4][189] = 15;
        world.actors[0].slot_state = 0;
        world.actors[0].definition_id = 12;
        rejected(&world, &limits, &mut restored);
        world.actors[0].definition_id = 0;
        limits.asset_clips[0] = 2;
        world.actors[0].animation_clip = 2;
        rejected(&world, &limits, &mut restored);
        world.actors[0].animation_clip = 1;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
        world.action_animations[0][5] = 2;
        rejected(&world, &limits, &mut restored);
        world.action_animations[0][5] = 0;
        world.objects[0].object_id = 0;
        world.objects[0].action = 81;
        world.objects[0].link_words[0] = 48 << 8;
        rejected(&world, &limits, &mut restored);
        world.objects[0].link_words[0] = 47 << 8;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
        world.effects[0].slot_type = 0;
        world.effects[0].animation_clip = 255;
        world.effects[0].render_id = 22;
        rejected(&world, &limits, &mut restored);
        world.effects[0].slot_type = 255;
        let floor = &mut world.floors[0].records;
        floor[0] = 1;
        for i in 0..8 {
            floor[1 + i * 7] = 255;
        }
        floor[58..248].fill(255); // no actors, then 190 object IDs
        floor[58] = 0;
        floor[248] = 1; // one link record
        floor[249] = 0;
        floor[251] = 48; // effect index in link byte 1
        limits.object_operations[0] = 81;
        rejected(&world, &limits, &mut restored);
        world.floors[0].records[251] = 47;
        decode(&unchecked_bytes(&world), &limits, &mut restored).unwrap();
    }
}
