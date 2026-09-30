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
