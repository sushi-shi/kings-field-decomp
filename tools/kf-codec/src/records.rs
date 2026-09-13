//! Fixed-size GAME resource-table transformations.
//!
//! These functions model the destination writes made by the retail loaders.
//! Callers provide the complete, pre-initialized destination buffer: only the
//! bytes written by `GAME.EXE` are changed and any tail remains untouched.

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

/// Copies the sixteen 44-byte records, then negates the little-endian angle
/// at offset `0x26` in each destination record with retail u16 wrapping.
pub fn load_weapon_records(source: &[u8], destination: &mut [u8]) -> Result<(), TransformError> {
    copy_prefix(source, destination, WEAPON_RECORDS_SIZE)?;
    for index in 0..WEAPON_RECORD_COUNT {
        let at = index * WEAPON_RECORD_SIZE + 0x26;
        let angle = read_u16(destination, at);
        write_u16(destination, at, angle.wrapping_neg());
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

/// Signed three-component vector, corresponding to the game's KfVec3s.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct Vec3s {
    pub x: i16,
    pub y: i16,
    pub z: i16,
}

/// Decoded SDK SVECTOR fields; the stored pad halfword is preserved.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct SVector {
    pub vx: i16,
    pub vy: i16,
    pub vz: i16,
    pub pad: i16,
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
            render_translation: Vec3s {
                x: read_u16(bytes, 0x1c) as i16,
                y: read_u16(bytes, 0x1e) as i16,
                z: read_u16(bytes, 0x20) as i16,
            },
            unknown_22: read_u16(bytes, 0x22),
            render_rotation: SVector {
                vx: read_u16(bytes, 0x24) as i16,
                vy: read_u16(bytes, 0x26) as i16,
                vz: read_u16(bytes, 0x28) as i16,
                pad: read_u16(bytes, 0x2a) as i16,
            },
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
        write_u16(bytes, 0x1c, self.render_translation.x as u16);
        write_u16(bytes, 0x1e, self.render_translation.y as u16);
        write_u16(bytes, 0x20, self.render_translation.z as u16);
        write_u16(bytes, 0x22, self.unknown_22);
        write_u16(bytes, 0x24, self.render_rotation.vx as u16);
        write_u16(bytes, 0x26, self.render_rotation.vy as u16);
        write_u16(bytes, 0x28, self.render_rotation.vz as u16);
        write_u16(bytes, 0x2a, self.render_rotation.pad as u16);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ArmorRecord {
    pub unknown_00: u16,
    pub defense_components_02: [u16; 3],
    pub status_resistance: u16,
    pub defense_components_0a: [u16; 2],
    pub hp_regeneration_interval: u16,
    pub hp_drain_interval: u16,
    pub unknown_12: [u8; 10],
}

impl ArmorRecord {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..ARMOR_RECORD_SIZE)?;
        Some(Self {
            unknown_00: read_u16(bytes, 0),
            defense_components_02: read_u16_array::<3>(bytes, 2),
            status_resistance: read_u16(bytes, 8),
            defense_components_0a: read_u16_array::<2>(bytes, 0x0a),
            hp_regeneration_interval: read_u16(bytes, 0x0e),
            hp_drain_interval: read_u16(bytes, 0x10),
            unknown_12: copy_array(bytes, 0x12),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..ARMOR_RECORD_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.unknown_00);
        write_u16_array(bytes, 2, &self.defense_components_02);
        write_u16(bytes, 8, self.status_resistance);
        write_u16_array(bytes, 0x0a, &self.defense_components_0a);
        write_u16(bytes, 0x0e, self.hp_regeneration_interval);
        write_u16(bytes, 0x10, self.hp_drain_interval);
        bytes[0x12..0x1c].copy_from_slice(&self.unknown_12);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MagicRecord {
    pub learned: u8,
    pub charge_rate: u8,
    pub sound_refs: [u8; 6],
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
            sound_refs: copy_array(bytes, 2),
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
        bytes[2..8].copy_from_slice(&self.sound_refs);
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
    pub power: u16,
    pub magic: u16,
    pub experience: u32,
}

impl PlayerLevelGrowth {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..PLAYER_LEVEL_GROWTH_SIZE)?;
        Some(Self {
            maximum_hp: read_u16(bytes, 0),
            maximum_mp: read_u16(bytes, 2),
            power: read_u16(bytes, 4),
            magic: read_u16(bytes, 6),
            experience: read_u32(bytes, 8),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..PLAYER_LEVEL_GROWTH_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.maximum_hp);
        write_u16(bytes, 2, self.maximum_mp);
        write_u16(bytes, 4, self.power);
        write_u16(bytes, 6, self.magic);
        write_u32(bytes, 8, self.experience);
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

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ActorDefinition {
    /// No semantic claim is made for bytes not yet supported by shared C
    /// fields. Keeping the full record makes decode/encode lossless.
    pub bytes: [u8; ACTOR_DEFINITION_SIZE],
}

impl ActorDefinition {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        Some(Self {
            bytes: copy_array(bytes.get(..ACTOR_DEFINITION_SIZE)?, 0),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..ACTOR_DEFINITION_SIZE) else {
            return false;
        };
        bytes.copy_from_slice(&self.bytes);
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
