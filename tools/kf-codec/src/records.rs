//! Fixed-size GAME resource-table transformations.
//!
//! These functions model the destination writes made by the retail loaders.
//! Callers provide the complete, pre-initialized destination buffer: only the
//! bytes written by `GAME.EXE` are changed and any tail remains untouched.

use crate::audio::SoundRef;
use crate::math::{SVector, Vec3s};

pub const RENDER_CELL_WINDOWS_SIZE: usize = 3_264;
pub const WEAPON_RECORD_SIZE: usize = 44;
pub const WEAPON_RECORD_COUNT: usize = 16;
pub const WEAPON_RECORDS_SIZE: usize = WEAPON_RECORD_SIZE * WEAPON_RECORD_COUNT;
pub const ARMOR_RECORD_SIZE: usize = 28;
pub const ARMOR_RECORDS_SIZE: usize = 1_176;
pub const MAGIC_RECORD_SIZE: usize = 20;
pub const MAGIC_RECORDS_SIZE: usize = 480;
pub const MAP_OBJECT_DEFINITION_SIZE: usize = 8;
pub const MAP_OBJECT_DEFINITIONS_SIZE: usize = 1_280;
pub const PLAYER_LEVEL_GROWTH_SIZE: usize = 12;
pub const PLAYER_LEVEL_GROWTH_TABLE_SIZE: usize = 480;
pub const ACTOR_DEFINITION_SIZE: usize = 152;
pub const ACTOR_DEFINITIONS_SIZE: usize = 1_824;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum TransformError {
    SourceTooSmall { needed: usize, available: usize },
    DestinationTooSmall { needed: usize, available: usize },
}

fn copy_prefix(source: &[u8], destination: &mut [u8], length: usize) -> Result<(), TransformError> {
    let Some(source) = source.get(..length) else {
        return Err(TransformError::SourceTooSmall {
            needed: length,
            available: source.len(),
        });
    };
    let available = destination.len();
    let Some(destination) = destination.get_mut(..length) else {
        return Err(TransformError::DestinationTooSmall {
            needed: length,
            available,
        });
    };
    destination.copy_from_slice(source);
    Ok(())
}

pub fn load_render_cell_windows(
    source: &[u8],
    destination: &mut [u8],
) -> Result<(), TransformError> {
    copy_prefix(source, destination, RENDER_CELL_WINDOWS_SIZE)
}

/// Copies the sixteen 44-byte records, then mirrors the SDK rotation's Y
/// component with retail halfword wrapping (including `i16::MIN`).
pub fn load_weapon_records(source: &[u8], destination: &mut [u8]) -> Result<(), TransformError> {
    copy_prefix(source, destination, WEAPON_RECORDS_SIZE)?;
    for bytes in destination[..WEAPON_RECORDS_SIZE].chunks_exact_mut(WEAPON_RECORD_SIZE) {
        let mut record = WeaponRecord::decode(bytes).expect("preflighted weapon record");
        record.render_rotation.vy = record.render_rotation.vy.wrapping_neg();
        record.encode(bytes);
    }
    Ok(())
}

/// Retail copies 1,176 bytes although COM.DAT's nominal armor chunk is only
/// 756 bytes. `source` must therefore be the safe whole-file-tail view exposed
/// by `CommonData::armor_records_source()`.
pub fn load_armor_records(source: &[u8], destination: &mut [u8]) -> Result<(), TransformError> {
    copy_prefix(source, destination, ARMOR_RECORDS_SIZE)
}

pub fn load_magic_records(source: &[u8], destination: &mut [u8]) -> Result<(), TransformError> {
    copy_prefix(source, destination, MAGIC_RECORDS_SIZE)
}

/// Retail copies 1,280 bytes although COM.DAT's nominal definitions chunk is
/// only 1,128 bytes. Pass `CommonData::map_object_definitions_source()`.
pub fn load_map_object_definitions(
    source: &[u8],
    destination: &mut [u8],
) -> Result<(), TransformError> {
    copy_prefix(source, destination, MAP_OBJECT_DEFINITIONS_SIZE)
}

pub fn load_player_level_growth(
    source: &[u8],
    destination: &mut [u8],
) -> Result<(), TransformError> {
    copy_prefix(source, destination, PLAYER_LEVEL_GROWTH_TABLE_SIZE)
}

pub fn load_actor_definitions(source: &[u8], destination: &mut [u8]) -> Result<(), TransformError> {
    copy_prefix(source, destination, ACTOR_DEFINITIONS_SIZE)
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct WeaponRecord {
    pub unknown_00: u8,
    pub charge_rate: u8,
    pub attack_components: [u16; 5],
    pub hp_regen_interval: u16,
    pub mp_regen_interval: u16,
    pub projection_distance: u16,
    pub attack_z_offset: u16,
    pub unknown_14: [u8; 8],
    pub render_translation: Vec3s,
    pub unknown_22: u16,
    pub render_rotation: SVector,
}

impl WeaponRecord {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..WEAPON_RECORD_SIZE)?;
        Some(Self {
            unknown_00: bytes[0],
            charge_rate: bytes[1],
            attack_components: read_u16_array::<5>(bytes, 2),
            hp_regen_interval: read_u16(bytes, 0x0c),
            mp_regen_interval: read_u16(bytes, 0x0e),
            projection_distance: read_u16(bytes, 0x10),
            attack_z_offset: read_u16(bytes, 0x12),
            unknown_14: copy_array(bytes, 0x14),
            render_translation: Vec3s::from_le_bytes(copy_array(bytes, 0x1c)),
            unknown_22: read_u16(bytes, 0x22),
            render_rotation: SVector::from_le_bytes(copy_array(bytes, 0x24)),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..WEAPON_RECORD_SIZE) else {
            return false;
        };
        bytes[0] = self.unknown_00;
        bytes[1] = self.charge_rate;
        write_u16_array(bytes, 2, &self.attack_components);
        write_u16(bytes, 0x0c, self.hp_regen_interval);
        write_u16(bytes, 0x0e, self.mp_regen_interval);
        write_u16(bytes, 0x10, self.projection_distance);
        write_u16(bytes, 0x12, self.attack_z_offset);
        bytes[0x14..0x1c].copy_from_slice(&self.unknown_14);
        bytes[0x1c..0x22].copy_from_slice(&self.render_translation.to_le_bytes());
        write_u16(bytes, 0x22, self.unknown_22);
        bytes[0x24..0x2c].copy_from_slice(&self.render_rotation.to_le_bytes());
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ArmorRecord {
    pub unknown_00: u16,
    pub cutting_defense: u16,
    pub striking_defense: u16,
    pub piercing_defense: u16,
    pub poison_resistance: u16,
    pub magic_defense: u16,
    pub fire_defense: u16,
    pub hp_regen_interval: u16,
    pub hp_drain_interval: u16,
    pub unknown_12: [u8; 10],
}

impl ArmorRecord {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..ARMOR_RECORD_SIZE)?;
        Some(Self {
            unknown_00: read_u16(bytes, 0),
            cutting_defense: read_u16(bytes, 2),
            striking_defense: read_u16(bytes, 4),
            piercing_defense: read_u16(bytes, 6),
            poison_resistance: read_u16(bytes, 8),
            magic_defense: read_u16(bytes, 0x0a),
            fire_defense: read_u16(bytes, 0x0c),
            hp_regen_interval: read_u16(bytes, 0x0e),
            hp_drain_interval: read_u16(bytes, 0x10),
            unknown_12: copy_array(bytes, 0x12),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..ARMOR_RECORD_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.unknown_00);
        write_u16(bytes, 2, self.cutting_defense);
        write_u16(bytes, 4, self.striking_defense);
        write_u16(bytes, 6, self.piercing_defense);
        write_u16(bytes, 8, self.poison_resistance);
        write_u16(bytes, 0x0a, self.magic_defense);
        write_u16(bytes, 0x0c, self.fire_defense);
        write_u16(bytes, 0x0e, self.hp_regen_interval);
        write_u16(bytes, 0x10, self.hp_drain_interval);
        bytes[0x12..0x1c].copy_from_slice(&self.unknown_12);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MagicRecord {
    pub learned: u8,
    pub charge_rate: u8,
    pub sounds: [SoundRef; 2],
    pub damage_components: [u16; 4],
    pub mp_cost: u16,
    pub unknown_12: [u8; 2],
}

impl MagicRecord {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MAGIC_RECORD_SIZE)?;
        Some(Self {
            learned: bytes[0],
            charge_rate: bytes[1],
            sounds: [
                SoundRef::from_bytes(copy_array(bytes, 2)),
                SoundRef::from_bytes(copy_array(bytes, 5)),
            ],
            damage_components: read_u16_array::<4>(bytes, 8),
            mp_cost: read_u16(bytes, 0x10),
            unknown_12: copy_array(bytes, 0x12),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MAGIC_RECORD_SIZE) else {
            return false;
        };
        bytes[0] = self.learned;
        bytes[1] = self.charge_rate;
        bytes[2..5].copy_from_slice(&self.sounds[0].to_bytes());
        bytes[5..8].copy_from_slice(&self.sounds[1].to_bytes());
        write_u16_array(bytes, 8, &self.damage_components);
        write_u16(bytes, 0x10, self.mp_cost);
        bytes[0x12..0x14].copy_from_slice(&self.unknown_12);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MapObjectDefinition {
    pub behavior_type: u8,
    pub unknown_01: u8,
    pub collision_radius: u16,
    pub interaction_radius: u16,
    pub unknown_06: [u8; 2],
}

impl MapObjectDefinition {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MAP_OBJECT_DEFINITION_SIZE)?;
        Some(Self {
            behavior_type: bytes[0],
            unknown_01: bytes[1],
            collision_radius: read_u16(bytes, 2),
            interaction_radius: read_u16(bytes, 4),
            unknown_06: copy_array(bytes, 6),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MAP_OBJECT_DEFINITION_SIZE) else {
            return false;
        };
        bytes[0] = self.behavior_type;
        bytes[1] = self.unknown_01;
        write_u16(bytes, 2, self.collision_radius);
        write_u16(bytes, 4, self.interaction_radius);
        bytes[6..8].copy_from_slice(&self.unknown_06);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct PlayerLevelGrowth {
    pub maximum_hp: u16,
    pub maximum_mp: u16,
    pub physical_power_step: u16,
    pub magic_step: u16,
    pub experience_threshold: u32,
}

impl PlayerLevelGrowth {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..PLAYER_LEVEL_GROWTH_SIZE)?;
        Some(Self {
            maximum_hp: read_u16(bytes, 0),
            maximum_mp: read_u16(bytes, 2),
            physical_power_step: read_u16(bytes, 4),
            magic_step: read_u16(bytes, 6),
            experience_threshold: read_u32(bytes, 8),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..PLAYER_LEVEL_GROWTH_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.maximum_hp);
        write_u16(bytes, 2, self.maximum_mp);
        write_u16(bytes, 4, self.physical_power_step);
        write_u16(bytes, 6, self.magic_step);
        write_u32(bytes, 8, self.experience_threshold);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct RenderCellWindow {
    pub width: u16,
    pub height: u16,
    pub origin_x: u16,
    pub origin_z: u16,
    pub cells: [u8; 196],
}

impl RenderCellWindow {
    pub const SIZE: usize = 204;

    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..Self::SIZE)?;
        Some(Self {
            width: read_u16(bytes, 0),
            height: read_u16(bytes, 2),
            origin_x: read_u16(bytes, 4),
            origin_z: read_u16(bytes, 6),
            cells: copy_array(bytes, 8),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..Self::SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.width);
        write_u16(bytes, 2, self.height);
        write_u16(bytes, 4, self.origin_x);
        write_u16(bytes, 6, self.origin_z);
        bytes[8..204].copy_from_slice(&self.cells);
        true
    }
}

/// Packed actor action parameters; effect codes retain flags and unset values.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ActorActionParameters {
    pub effect_codes: [u8; 3],
    pub effect_chances: [u8; 3],
    pub drop_object: u8,
    pub drop_chance: u8,
}

impl ActorActionParameters {
    pub const BYTE_SIZE: usize = 8;

    pub const fn from_bytes(bytes: [u8; Self::BYTE_SIZE]) -> Self {
        Self {
            effect_codes: [bytes[0], bytes[1], bytes[2]],
            effect_chances: [bytes[3], bytes[4], bytes[5]],
            drop_object: bytes[6],
            drop_chance: bytes[7],
        }
    }

    pub const fn to_bytes(self) -> [u8; Self::BYTE_SIZE] {
        [
            self.effect_codes[0],
            self.effect_codes[1],
            self.effect_codes[2],
            self.effect_chances[0],
            self.effect_chances[1],
            self.effect_chances[2],
            self.drop_object,
            self.drop_chance,
        ]
    }
}

/// Decoded `KfActorDefinition`; unknown storage and all table slots survive
/// round trips. Resource enum/flag bytes retain their full on-disc domain.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ActorDefinition {
    pub pursuit_distance_scale: u8,
    pub model_and_texture: u8,
    pub melee_attack_chance: u8,
    pub status_effect: u8,
    pub status_effect_chance: u8,
    pub action_parameters: ActorActionParameters,
    pub move_speed: u8,
    pub action_animations: [u8; 16],
    pub turn_rate: u8,
    pub sounds: [SoundRef; 3],
    pub attachment_offsets: [Vec3s; 2],
    pub special_attack_chance: i16,
    pub special_attack_range: i16,
    pub unknown_38: [u8; 2],
    pub action_animation_steps: [u16; 16],
    pub action_animation_phases: [u16; 16],
    pub collision_radius: u16,
    pub collision_height: u16,
    pub awareness_distance: u16,
    pub initial_health: u16,
    pub effect_owner_id: u16,
    pub experience_reward: u16,
    pub attack_components: [u16; 3],
    pub defenses: [u16; 5],
    pub gold_drop_limit: u16,
}

impl ActorDefinition {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..ACTOR_DEFINITION_SIZE)?;
        Some(Self {
            pursuit_distance_scale: bytes[0],
            model_and_texture: bytes[1],
            melee_attack_chance: bytes[2],
            status_effect: bytes[3],
            status_effect_chance: bytes[4],
            action_parameters: ActorActionParameters::from_bytes(copy_array(bytes, 5)),
            move_speed: bytes[0x0d],
            action_animations: copy_array(bytes, 0x0e),
            turn_rate: bytes[0x1e],
            sounds: core::array::from_fn(|index| {
                SoundRef::from_bytes(copy_array(bytes, 0x1f + index * SoundRef::BYTE_SIZE))
            }),
            attachment_offsets: core::array::from_fn(|index| {
                Vec3s::from_le_bytes(copy_array(bytes, 0x28 + index * Vec3s::BYTE_SIZE))
            }),
            special_attack_chance: read_u16(bytes, 0x34) as i16,
            special_attack_range: read_u16(bytes, 0x36) as i16,
            unknown_38: copy_array(bytes, 0x38),
            action_animation_steps: read_u16_array(bytes, 0x3a),
            action_animation_phases: read_u16_array(bytes, 0x5a),
            collision_radius: read_u16(bytes, 0x7a),
            collision_height: read_u16(bytes, 0x7c),
            awareness_distance: read_u16(bytes, 0x7e),
            initial_health: read_u16(bytes, 0x80),
            effect_owner_id: read_u16(bytes, 0x82),
            experience_reward: read_u16(bytes, 0x84),
            attack_components: read_u16_array(bytes, 0x86),
            defenses: read_u16_array(bytes, 0x8c),
            gold_drop_limit: read_u16(bytes, 0x96),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..ACTOR_DEFINITION_SIZE) else {
            return false;
        };
        bytes[0] = self.pursuit_distance_scale;
        bytes[1] = self.model_and_texture;
        bytes[2] = self.melee_attack_chance;
        bytes[3] = self.status_effect;
        bytes[4] = self.status_effect_chance;
        bytes[5..0x0d].copy_from_slice(&self.action_parameters.to_bytes());
        bytes[0x0d] = self.move_speed;
        bytes[0x0e..0x1e].copy_from_slice(&self.action_animations);
        bytes[0x1e] = self.turn_rate;
        for (index, sound) in self.sounds.iter().enumerate() {
            let at = 0x1f + index * SoundRef::BYTE_SIZE;
            bytes[at..at + SoundRef::BYTE_SIZE].copy_from_slice(&sound.to_bytes());
        }
        for (index, offset) in self.attachment_offsets.iter().enumerate() {
            let at = 0x28 + index * Vec3s::BYTE_SIZE;
            bytes[at..at + Vec3s::BYTE_SIZE].copy_from_slice(&offset.to_le_bytes());
        }
        write_u16(bytes, 0x34, self.special_attack_chance as u16);
        write_u16(bytes, 0x36, self.special_attack_range as u16);
        bytes[0x38..0x3a].copy_from_slice(&self.unknown_38);
        write_u16_array(bytes, 0x3a, &self.action_animation_steps);
        write_u16_array(bytes, 0x5a, &self.action_animation_phases);
        write_u16(bytes, 0x7a, self.collision_radius);
        write_u16(bytes, 0x7c, self.collision_height);
        write_u16(bytes, 0x7e, self.awareness_distance);
        write_u16(bytes, 0x80, self.initial_health);
        write_u16(bytes, 0x82, self.effect_owner_id);
        write_u16(bytes, 0x84, self.experience_reward);
        write_u16_array(bytes, 0x86, &self.attack_components);
        write_u16_array(bytes, 0x8c, &self.defenses);
        write_u16(bytes, 0x96, self.gold_drop_limit);
        true
    }
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}

fn write_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn write_u32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn read_u16_array<const N: usize>(bytes: &[u8], at: usize) -> [u16; N] {
    let mut output = [0; N];
    let mut index = 0;
    while index < N {
        output[index] = read_u16(bytes, at + index * 2);
        index += 1;
    }
    output
}

fn write_u16_array<const N: usize>(bytes: &mut [u8], at: usize, values: &[u16; N]) {
    let mut index = 0;
    while index < N {
        write_u16(bytes, at + index * 2, values[index]);
        index += 1;
    }
}

fn copy_array<const N: usize>(bytes: &[u8], at: usize) -> [u8; N] {
    let mut output = [0; N];
    output.copy_from_slice(&bytes[at..at + N]);
    output
}
