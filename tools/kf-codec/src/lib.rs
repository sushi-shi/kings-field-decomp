//! Allocation-free codecs for King's Field resource containers.
//!
//! The grammars here come from retail `GAME.EXE` resource-consumer
//! disassembly and the retail disc corpus. They intentionally do not call into
//! the matching C reconstruction: the Rust readers provide an independent
//! implementation for differential tests.
//!
//! All readers borrow their input and all writers use caller-owned buffers.
//! Multi-byte fields are decoded with explicit byte order, so the
//! result does not depend on host alignment or byte order.

#![no_std]
#![forbid(unsafe_code)]

pub mod animation;
pub mod asset_archive;
pub mod audio;
pub mod audio_vab_state;
pub mod chunked;
pub mod common;
pub mod game_data;
pub mod map_resources;
pub mod math;
pub mod placements;
pub mod records;
pub mod registry;
pub mod save;
pub mod stat;
pub mod tim;
pub mod tmd;
pub mod world_persist;
pub mod world_state;

#[derive(Debug)]
pub(crate) enum Sink<'a> {
    Count(usize),
    Write { bytes: &'a mut [u8], at: usize },
}

impl Sink<'_> {
    pub(crate) fn extend(&mut self, value: &[u8]) -> bool {
        match self {
            Self::Count(len) => match len.checked_add(value.len()) {
                Some(next) => {
                    *len = next;
                    true
                }
                None => false,
            },
            Self::Write { bytes, at } => {
                let Some(end) = at.checked_add(value.len()) else {
                    return false;
                };
                let Some(destination) = bytes.get_mut(*at..end) else {
                    return false;
                };
                destination.copy_from_slice(value);
                *at = end;
                true
            }
        }
    }

    pub(crate) fn len(&self) -> usize {
        match self {
            Self::Count(len) => *len,
            Self::Write { at, .. } => *at,
        }
    }
}
