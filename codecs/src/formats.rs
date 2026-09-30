//! Fixed on-disc records. Endian fields have byte alignment on every target.

use crate::bytes::{BeU16, BeU24, BeU32, LeI16, LeU16, LeU32};
use bytemuck::{Pod, Zeroable};

macro_rules! record {
    ($name:ident, $size:literal, { $($field:ident: $ty:ty),* $(,)? }) => {
        #[repr(C)]
        #[derive(Clone, Copy, Debug, Pod, Zeroable)]
        pub(crate) struct $name { $(pub $field: $ty),* }

        const _: () = assert!(size_of::<$name>() == $size && align_of::<$name>() == 1);
    };
}

macro_rules! bitfield {
    ($name:ident, { $($field:ident: $start:literal..$end:literal),* $(,)? }) => {
        #[repr(transparent)]
        #[derive(Clone, Copy, Debug, Pod, Zeroable)]
        pub(crate) struct $name(LeU16);

        impl $name { $(
            #[expect(clippy::as_conversions, reason = "the field width fits in u8")]
            pub(crate) fn $field(self) -> u8 {
                const { assert!($start < $end && $end <= 16 && $end - $start <= 8) };
                ((self.0.get() >> $start) & (u16::MAX >> (16 - ($end - $start)))) as u8
            }
        )* }
    };
}

bitfield!(Adsr1, {
    sustain_level: 0..4,
    decay_shift: 4..8,
    attack_step: 8..10,
    attack_shift: 10..15,
    attack_exponential: 15..16,
});

bitfield!(Adsr2, {
    release_shift: 0..5,
    release_exponential: 5..6,
    sustain_step: 6..8,
    sustain_shift: 8..13,
    sustain_decreasing: 14..15,
    sustain_exponential: 15..16,
});

record!(VabHeader, 32, {
    magic: [u8; 4],
    _version: [u8; 4],
    _id: [u8; 4],
    file_size: LeU32,
    _reserved0: [u8; 2],
    program_count: LeU16,
    tone_count: LeU16,
    sample_count: LeU16,
    master_volume: u8,
    pan: u8,
    _attributes: [u8; 2],
    _reserved1: [u8; 4],
});

record!(VabProgram, 16, {
    tone_count: u8,
    master_volume: u8,
    priority: u8,
    _mode: u8,
    pan: u8,
    _reserved: [u8; 11],
});

record!(VabTone, 32, {
    priority: u8,
    mode: u8,
    volume: u8,
    pan: u8,
    center_note: u8,
    center_shift: u8,
    minimum_note: u8,
    maximum_note: u8,
    vibrato_width: u8,
    vibrato_time: u8,
    portamento_width: u8,
    portamento_time: u8,
    pitch_bend_minimum: u8,
    pitch_bend_maximum: u8,
    _reserved0: [u8; 2],
    adsr1: Adsr1,
    adsr2: Adsr2,
    program: LeI16,
    sample: LeI16,
    _reserved1: [u8; 8],
});

record!(SeqHeader, 15, {
    magic: [u8; 4],
    version: BeU32,
    resolution: BeU16,
    tempo: BeU24,
    _time_signature: [u8; 2],
});

record!(TimHeader, 8, {
    magic: LeU32,
    mode: LeU32,
});

record!(TimRectangle, 8, {
    x: LeI16,
    y: LeI16,
    width: LeI16,
    height: LeI16,
});

record!(TimBlock, 12, {
    encoded_bytes: LeU32,
    rectangle: TimRectangle,
});

record!(AdpcmHeader, 2, {
    predictor_shift: u8,
    flags: u8,
});

record!(AssetHeader, 20, {
    encoded_bytes: LeU32,
    clip_count: LeU32,
    tmd_offset: LeU32,
    morph_table_offset: LeU32,
    clip_table_offset: LeU32,
});

record!(AnimationClip, 4, {
    keyframe_count: LeU16,
    _reserved: [u8; 2],
});

record!(AnimationKeyframe, 8, {
    reverse: LeU16,
    duration: LeU16,
    rest_morph: LeU16,
    morph_count: LeU16,
});

record!(AnimationMorph, 12, {
    _reserved: [u8; 4],
    base_vertex: LeU32,
    delta_count: LeU32,
});

record!(AnimationDelta, 8, {
    x: LeI16,
    y: LeI16,
    z: LeI16,
    _reserved: [u8; 2],
});

record!(ActorPlacement, 16, {
    slot_state: u8,
    definition_flags: u8,
    heading_quadrant: u8,
    tile_z: u8,
    tile_x: u8,
    spawn_chance: u8,
    death_drop_object_id: u8,
    _reserved0: [u8; 3],
    local_z: LeI16,
    local_x: LeI16,
    _reserved1: [u8; 2],
});

record!(ObjectPlacement, 20, {
    object_id: u8,
    _reserved: u8,
    tile_z: u8,
    tile_x: u8,
    yaw: LeU16,
    local_z: LeI16,
    local_x: LeI16,
    local_y: LeI16,
    link: [LeU32; 2],
});

record!(EventPlacement, 24, {
    state: u8,
    character_id: u8,
    model_index: u8,
    cell_z: u8,
    cell_x: u8,
    dialogue_pages: [u8; 5],
    dialogue_stage_limit: u8,
    unknown_0b: u8,
    unknown_0c: u8,
    behavior: u8,
    position_z_offset: LeI16,
    position_x_offset: LeI16,
    initial_rotation: LeU16,
    radius: LeU16,
    _reserved: [u8; 2],
});
