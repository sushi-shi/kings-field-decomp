//! King's Field model-animation tables and vertex blending.
//!
//! This is an allocation-free model of `render_bind_animated_instance`
//! @`0x800205d4` in retail `GAME.EXE`. Offsets in the asset's object, clip,
//! and keyframe tables are relative to the start of the containing asset.
//!
//! Retail does not initialize its keyframe-cache index before scanning a clip:
//! the value is inherited in callee-saved register `$s5`. [`select_frame`]
//! therefore requires that initial value instead of silently inventing zero.

use core::fmt;

use crate::asset_archive::{AssetHeader, ASSET_HEADER_SIZE};

pub const CLIP_HEADER_SIZE: usize = 4;
pub const KEYFRAME_HEADER_SIZE: usize = 8;
pub const MORPH_OBJECT_HEADER_SIZE: usize = 12;
pub const TMD_HEADER_SIZE: usize = 12;
pub const TMD_OBJECT_SIZE: usize = 28;
pub const VERTEX_SIZE: usize = Vertex::BYTE_SIZE;
pub const FULL_WEIGHT: u16 = 0x1000;
pub const POOL_RECORD_SIZE: usize = 20;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AnimationError {
    TruncatedAssetHeader {
        available: usize,
    },
    InvalidAssetSize {
        declared: u32,
    },
    TruncatedAsset {
        declared: u32,
        available: usize,
    },
    NegativeClipCount {
        count: i32,
    },
    OffsetOverflow {
        field: &'static str,
        offset: u32,
    },
    RangeOverflow {
        field: &'static str,
        at: usize,
        count: usize,
        stride: usize,
    },
    OutOfBounds {
        field: &'static str,
        at: usize,
        need: usize,
        available: usize,
    },
    ClipOutOfRange {
        index: u16,
        count: usize,
    },
    KeyframeOutOfRange {
        index: u16,
        count: u16,
    },
    MorphDeltaOutOfRange {
        index: u32,
        count: u32,
    },
    EmptyClip {
        index: u16,
    },
    ZeroDuration {
        clip: u16,
        keyframe: u16,
    },
    ZeroVertexCount,
    VertexRangeOutOfBounds {
        object: u16,
        base: u32,
        count: u32,
        vertices: u16,
    },
    CacheTooSmall {
        need: usize,
        have: usize,
    },
    OutputTooSmall {
        need: usize,
        have: usize,
    },
    CallerVertexCountMismatch {
        caller: u16,
        asset: u16,
    },
    BacklinkMismatch {
        record: u32,
        expected: u32,
    },
    MissingSuccessfulAllocation,
    LifecycleEventsTooSmall {
        need: usize,
        have: usize,
    },
}

impl fmt::Display for AnimationError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            Self::TruncatedAssetHeader { available } => {
                write!(f, "animation asset header has {available} bytes")
            }
            Self::InvalidAssetSize { declared } => {
                write!(f, "animation asset declares invalid size {declared}")
            }
            Self::TruncatedAsset {
                declared,
                available,
            } => write!(
                f,
                "animation asset declares {declared} bytes, {available} available"
            ),
            Self::NegativeClipCount { count } => {
                write!(f, "animation asset declares negative clip count {count}")
            }
            Self::OffsetOverflow { field, offset } => {
                write!(f, "{field} offset {offset:#x} is not representable")
            }
            Self::RangeOverflow {
                field,
                at,
                count,
                stride,
            } => write!(
                f,
                "{field} range at {at:#x} for {count} entries of {stride} bytes overflows"
            ),
            Self::OutOfBounds {
                field,
                at,
                need,
                available,
            } => write!(
                f,
                "{field} at {at:#x} needs {need} bytes, {available} available"
            ),
            Self::ClipOutOfRange { index, count } => {
                write!(f, "clip {index} is outside {count} clips")
            }
            Self::KeyframeOutOfRange { index, count } => {
                write!(f, "keyframe {index} is outside {count} keyframes")
            }
            Self::MorphDeltaOutOfRange { index, count } => {
                write!(f, "morph delta {index} is outside {count} deltas")
            }
            Self::EmptyClip { index } => write!(f, "clip {index} has no keyframes"),
            Self::ZeroDuration { clip, keyframe } => {
                write!(f, "clip {clip} keyframe {keyframe} has zero duration")
            }
            Self::ZeroVertexCount => {
                f.write_str("retail's do-while vertex copy is undefined for zero vertices")
            }
            Self::VertexRangeOutOfBounds {
                object,
                base,
                count,
                vertices,
            } => write!(
                f,
                "morph object {object} vertex range {base}..{} exceeds {vertices} vertices",
                base.saturating_add(count)
            ),
            Self::CacheTooSmall { need, have } => {
                write!(f, "animation cache holds {have} vertices, need {need}")
            }
            Self::OutputTooSmall { need, have } => {
                write!(f, "animation output holds {have} vertices, need {need}")
            }
            Self::CallerVertexCountMismatch { caller, asset } => write!(
                f,
                "caller supplies {caller} animation vertices, asset object zero has {asset}"
            ),
            Self::BacklinkMismatch { record, expected } => write!(
                f,
                "pool record owner_slot {record:#x} differs from caller anchor {expected:#x}"
            ),
            Self::MissingSuccessfulAllocation => {
                f.write_str("animation allocation outcomes contain no successful allocation")
            }
            Self::LifecycleEventsTooSmall { need, have } => {
                write!(
                    f,
                    "animation lifecycle output holds {have} events, need {need}"
                )
            }
        }
    }
}

impl core::error::Error for AnimationError {}

/// Byte-exact view of the 20-byte per-instance cache record.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PoolRecord {
    pub state: i16,
    pub asset_index: u16,
    pub clip_index: u16,
    pub keyframe_index: u16,
    /// Encoded PSX address of `KfMorphObject`, not a host pointer.
    pub rest_morph: u32,
    /// Encoded PSX address of the cached `SVECTOR` array.
    pub cached_vertices: u32,
    /// Encoded PSX address of the owner's `KfPoolRecord *` field.
    pub owner_slot: u32,
}

impl PoolRecord {
    pub const fn from_le_bytes(bytes: [u8; POOL_RECORD_SIZE]) -> Self {
        Self {
            state: i16::from_le_bytes([bytes[0], bytes[1]]),
            asset_index: u16::from_le_bytes([bytes[2], bytes[3]]),
            clip_index: u16::from_le_bytes([bytes[4], bytes[5]]),
            keyframe_index: u16::from_le_bytes([bytes[6], bytes[7]]),
            rest_morph: u32::from_le_bytes([bytes[8], bytes[9], bytes[10], bytes[11]]),
            cached_vertices: u32::from_le_bytes([bytes[12], bytes[13], bytes[14], bytes[15]]),
            owner_slot: u32::from_le_bytes([bytes[16], bytes[17], bytes[18], bytes[19]]),
        }
    }

    pub const fn to_le_bytes(self) -> [u8; POOL_RECORD_SIZE] {
        let state = self.state.to_le_bytes();
        let asset_index = self.asset_index.to_le_bytes();
        let clip_index = self.clip_index.to_le_bytes();
        let keyframe_index = self.keyframe_index.to_le_bytes();
        let rest_morph = self.rest_morph.to_le_bytes();
        let allocation = self.cached_vertices.to_le_bytes();
        let owner_slot = self.owner_slot.to_le_bytes();
        [
            state[0],
            state[1],
            asset_index[0],
            asset_index[1],
            clip_index[0],
            clip_index[1],
            keyframe_index[0],
            keyframe_index[1],
            rest_morph[0],
            rest_morph[1],
            rest_morph[2],
            rest_morph[3],
            allocation[0],
            allocation[1],
            allocation[2],
            allocation[3],
            owner_slot[0],
            owner_slot[1],
            owner_slot[2],
            owner_slot[3],
        ]
    }
}

/// Caller-owned state needed before the animation table walk begins.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct InstanceRequest {
    pub asset_index: u16,
    pub caller_vertex_count: u16,
    pub record_present: bool,
    pub pool_record_available: bool,
    pub record_address: u32,
    pub owner_slot_address: u32,
}

/// Ordered externally visible pool/allocator operations performed by retail.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LifecycleEvent {
    AllocateRecord { available: bool },
    ReleaseRecord { allocation: u32, owner_slot: u32 },
    AllocateVertices { byte_count: u32, result: u32 },
    ReleaseAll,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InstanceOutcome {
    Static,
    PoolUnavailable,
    Ready { reinitialized: bool },
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct InstanceReport {
    pub outcome: InstanceOutcome,
    pub allocation_attempts: usize,
    pub event_count: usize,
}

/// Model the record ownership and allocation prefix of the retail binder.
///
/// `allocation_results` contains the deterministic return values for successive
/// `memory_malloc_checked` calls. Zero means failure and causes a
/// [`LifecycleEvent::ReleaseAll`] before the next attempt. Because retail
/// retries forever, the bounded API requires a nonzero result whenever a new
/// allocation is needed. All validation is performed before state is changed.
pub fn prepare_instance(
    animation: &Animation<'_>,
    anchor: &mut u32,
    record: &mut PoolRecord,
    request: InstanceRequest,
    allocation_results: &[u32],
    events: &mut [LifecycleEvent],
) -> Result<InstanceReport, AnimationError> {
    let release = request.record_present;

    if !animation.is_animated() {
        if release && record.owner_slot != request.owner_slot_address {
            return Err(AnimationError::BacklinkMismatch {
                record: record.owner_slot,
                expected: request.owner_slot_address,
            });
        }
        let needed = usize::from(release);
        require_event_capacity(events, needed)?;
        if release {
            events[0] = LifecycleEvent::ReleaseRecord {
                allocation: record.cached_vertices,
                owner_slot: record.owner_slot,
            };
            release_record(anchor, record);
        }
        return Ok(InstanceReport {
            outcome: InstanceOutcome::Static,
            allocation_attempts: 0,
            event_count: needed,
        });
    }

    let asset_vertex_count = animation.base_vertex_count()?;
    if request.caller_vertex_count != asset_vertex_count {
        return Err(AnimationError::CallerVertexCountMismatch {
            caller: request.caller_vertex_count,
            asset: asset_vertex_count,
        });
    }

    let different_asset = request.record_present && record.asset_index != request.asset_index;
    if different_asset && record.owner_slot != request.owner_slot_address {
        return Err(AnimationError::BacklinkMismatch {
            record: record.owner_slot,
            expected: request.owner_slot_address,
        });
    }
    let needs_record = !request.record_present;
    if needs_record && !request.pool_record_available {
        require_event_capacity(events, 1)?;
        events[0] = LifecycleEvent::AllocateRecord { available: false };
        return Ok(InstanceReport {
            outcome: InstanceOutcome::PoolUnavailable,
            allocation_attempts: 0,
            event_count: 1,
        });
    }
    let reinitialized = needs_record || different_asset;
    if !reinitialized {
        return Ok(InstanceReport {
            outcome: InstanceOutcome::Ready {
                reinitialized: false,
            },
            allocation_attempts: 0,
            event_count: 0,
        });
    }

    let Some(success_index) = allocation_results.iter().position(|&result| result != 0) else {
        return Err(AnimationError::MissingSuccessfulAllocation);
    };
    let allocation_attempts = success_index + 1;
    let prefix_events = usize::from(needs_record || different_asset);
    let needed_events = prefix_events + allocation_attempts + success_index;
    require_event_capacity(events, needed_events)?;

    let mut event = 0usize;
    if needs_record {
        events[event] = LifecycleEvent::AllocateRecord { available: true };
        event += 1;
        // `pool_allocate` initializes only this field before returning a free
        // record; all other bytes retain their caller-provided contents.
        record.clip_index = 0xff;
    } else {
        events[event] = LifecycleEvent::ReleaseRecord {
            allocation: record.cached_vertices,
            owner_slot: record.owner_slot,
        };
        event += 1;
        release_record(anchor, record);
        record.clip_index = 0xff;
    }

    record.asset_index = request.asset_index;
    record.owner_slot = request.owner_slot_address;
    let byte_count = u32::from(request.caller_vertex_count) << 3;
    for &result in &allocation_results[..allocation_attempts] {
        events[event] = LifecycleEvent::AllocateVertices { byte_count, result };
        event += 1;
        record.cached_vertices = result;
        if result == 0 {
            events[event] = LifecycleEvent::ReleaseAll;
            event += 1;
        }
    }
    *anchor = request.record_address;

    Ok(InstanceReport {
        outcome: InstanceOutcome::Ready {
            reinitialized: true,
        },
        allocation_attempts,
        event_count: event,
    })
}

fn release_record(anchor: &mut u32, record: &mut PoolRecord) {
    record.state = 0;
    *anchor = 0;
    record.cached_vertices = 0;
}

fn require_event_capacity(events: &[LifecycleEvent], need: usize) -> Result<(), AnimationError> {
    if events.len() < need {
        Err(AnimationError::LifecycleEventsTooSmall {
            need,
            have: events.len(),
        })
    } else {
        Ok(())
    }
}

/// Animation vertices and deltas are the SDK's padded signed short vector.
pub use crate::math::SVector as Vertex;

#[derive(Debug, Clone, Copy)]
pub struct Animation<'a> {
    bytes: &'a [u8],
    header: AssetHeader,
    clip_count: usize,
}

impl<'a> Animation<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, AnimationError> {
        let Some(header) = AssetHeader::parse(bytes) else {
            return Err(AnimationError::TruncatedAssetHeader {
                available: bytes.len(),
            });
        };
        if header.byte_size < ASSET_HEADER_SIZE as u32 {
            return Err(AnimationError::InvalidAssetSize {
                declared: header.byte_size,
            });
        }
        let byte_size =
            usize::try_from(header.byte_size).map_err(|_| AnimationError::InvalidAssetSize {
                declared: header.byte_size,
            })?;
        let Some(bytes) = bytes.get(..byte_size) else {
            return Err(AnimationError::TruncatedAsset {
                declared: header.byte_size,
                available: bytes.len(),
            });
        };
        let clip_count = usize::try_from(header.animation_clip_count).map_err(|_| {
            AnimationError::NegativeClipCount {
                count: header.animation_clip_count,
            }
        })?;
        // Static assets do not have a clip table, and retail never reads the
        // field when animation_clip_count is zero.
        if clip_count != 0 {
            let clip_table = offset(header.clip_table_offset, "clip table")?;
            checked_range(bytes, clip_table, clip_count, 4, "clip table")?;
        }
        Ok(Self {
            bytes,
            header,
            clip_count,
        })
    }

    pub const fn bytes(&self) -> &'a [u8] {
        self.bytes
    }

    pub const fn header(&self) -> AssetHeader {
        self.header
    }

    pub const fn clip_count(&self) -> usize {
        self.clip_count
    }

    pub const fn is_animated(&self) -> bool {
        self.clip_count != 0
    }

    pub const fn clips(&self) -> Clips<'a> {
        Clips {
            animation: *self,
            index: 0,
        }
    }

    pub fn clip(&self, index: u16) -> Result<Clip<'a>, AnimationError> {
        let index_usize = usize::from(index);
        if index_usize >= self.clip_count {
            return Err(AnimationError::ClipOutOfRange {
                index,
                count: self.clip_count,
            });
        }
        let table = offset(self.header.clip_table_offset, "clip table")?;
        let entry = table
            .checked_add(index_usize * 4)
            .ok_or(AnimationError::RangeOverflow {
                field: "clip table",
                at: table,
                count: self.clip_count,
                stride: 4,
            })?;
        let clip_at = offset(read_u32(self.bytes, entry), "clip")?;
        let header = slice(self.bytes, clip_at, CLIP_HEADER_SIZE, "clip header")?;
        let keyframe_count = read_u16(header, 0);
        checked_range(
            self.bytes,
            clip_at + CLIP_HEADER_SIZE,
            usize::from(keyframe_count),
            4,
            "keyframe table",
        )?;
        Ok(Clip {
            animation: *self,
            index,
            offset: clip_at,
            keyframe_count,
            unknown_02: read_u16(header, 2),
        })
    }

    pub fn morph_object(&self, index: u16) -> Result<MorphObject<'a>, AnimationError> {
        let table = offset(self.header.object_table_offset, "morph-object table")?;
        let entry =
            table
                .checked_add(usize::from(index) * 4)
                .ok_or(AnimationError::RangeOverflow {
                    field: "morph-object table",
                    at: table,
                    count: usize::from(index) + 1,
                    stride: 4,
                })?;
        let entry_bytes = slice(self.bytes, entry, 4, "morph-object table entry")?;
        let object_at = offset(read_u32(entry_bytes, 0), "morph object")?;
        let header = slice(
            self.bytes,
            object_at,
            MORPH_OBJECT_HEADER_SIZE,
            "morph-object header",
        )?;
        let vertex_count = read_u32(header, 8);
        let count = usize::try_from(vertex_count).map_err(|_| AnimationError::RangeOverflow {
            field: "morph deltas",
            at: object_at + MORPH_OBJECT_HEADER_SIZE,
            count: usize::MAX,
            stride: VERTEX_SIZE,
        })?;
        let deltas_at = object_at + MORPH_OBJECT_HEADER_SIZE;
        let deltas = checked_range(self.bytes, deltas_at, count, VERTEX_SIZE, "morph deltas")?;
        Ok(MorphObject {
            index,
            offset: object_at,
            unknown_00: read_u32(header, 0),
            base_vertex: read_u32(header, 4),
            vertex_count,
            deltas,
        })
    }

    pub fn base_vertex_count(&self) -> Result<u16, AnimationError> {
        let (_, count, _) = self.base_vertex_bytes()?;
        Ok(count)
    }

    pub fn base_vertex_offset(&self) -> Result<usize, AnimationError> {
        let (_, _, offset) = self.base_vertex_bytes()?;
        Ok(offset)
    }

    pub fn base_vertices(&self) -> Result<BaseVertices<'a>, AnimationError> {
        let (bytes, count, _) = self.base_vertex_bytes()?;
        Ok(BaseVertices {
            bytes,
            count,
            index: 0,
        })
    }

    pub fn validate(&self) -> Result<ValidationReport, AnimationError> {
        let vertices = self.base_vertex_count()?;
        let mut clips = 0usize;
        let mut keyframes = 0usize;
        let mut morph_references = 0usize;
        for clip in self.clips() {
            let clip = clip?;
            clips += 1;
            for keyframe in clip.keyframes() {
                let keyframe = keyframe?;
                keyframes += 1;
                validate_object_range(self.morph_object(keyframe.rest_index)?, vertices)?;
                for index in keyframe.morph_indices() {
                    validate_object_range(self.morph_object(index)?, vertices)?;
                    morph_references += 1;
                }
            }
        }
        Ok(ValidationReport {
            clip_count: clips,
            keyframe_count: keyframes,
            morph_reference_count: morph_references,
            vertex_count: vertices,
        })
    }

    pub fn select_frame(
        &self,
        clip_index: u16,
        phase: u16,
        initial_kf_index: u16,
    ) -> Result<FrameSelection, AnimationError> {
        let clip = self.clip(clip_index)?;
        select_frame(clip, phase, initial_kf_index)
    }

    pub fn bind_frame(
        &self,
        clip_index: u16,
        phase: u16,
        initial_kf_index: u16,
        previous: Option<CacheKey>,
        cache: &mut [Vertex],
        output: &mut [Vertex],
    ) -> Result<BindReport, AnimationError> {
        let selection = self.select_frame(clip_index, phase, initial_kf_index)?;
        self.bind_selected_frame(selection, previous, cache, output)
    }

    pub fn bind_selected_frame(
        &self,
        selection: FrameSelection,
        previous: Option<CacheKey>,
        cache: &mut [Vertex],
        output: &mut [Vertex],
    ) -> Result<BindReport, AnimationError> {
        let vertex_count = usize::from(self.base_vertex_count()?);
        if vertex_count == 0 {
            return Err(AnimationError::ZeroVertexCount);
        }
        if cache.len() < vertex_count {
            return Err(AnimationError::CacheTooSmall {
                need: vertex_count,
                have: cache.len(),
            });
        }
        if output.len() < vertex_count {
            return Err(AnimationError::OutputTooSmall {
                need: vertex_count,
                have: output.len(),
            });
        }

        let clip = self.clip(selection.clip_index)?;
        let selected = clip.keyframe(selection.keyframe_ordinal)?;
        let cache_hit = previous.is_some_and(|key| {
            key.clip_index == selection.clip_index && key.keyframe_index == selection.keyframe_index
        });
        let rest_index;

        if cache_hit {
            rest_index = previous.expect("cache hit requires a key").rest_index;
        } else {
            for (destination, source) in cache[..vertex_count].iter_mut().zip(self.base_vertices()?)
            {
                *destination = source;
            }
            for index in selected.morph_indices() {
                apply_object(
                    &mut cache[..vertex_count],
                    self.morph_object(index)?,
                    FULL_WEIGHT,
                )?;
            }
            rest_index = selected.rest_index;
        }

        output[..vertex_count].copy_from_slice(&cache[..vertex_count]);
        apply_object(
            &mut output[..vertex_count],
            self.morph_object(rest_index)?,
            selection.fraction,
        )?;

        let cache_key = CacheKey {
            clip_index: selection.clip_index,
            keyframe_index: selection.keyframe_index,
            rest_index,
        };
        Ok(BindReport {
            selection,
            cache_key,
            cache_rebuilt: !cache_hit,
            vertex_count,
        })
    }

    fn base_vertex_bytes(&self) -> Result<(&'a [u8], u16, usize), AnimationError> {
        let tmd_at = offset(self.header.tmd_data_offset, "TMD")?;
        let object_at =
            tmd_at
                .checked_add(TMD_HEADER_SIZE)
                .ok_or(AnimationError::RangeOverflow {
                    field: "TMD object zero",
                    at: tmd_at,
                    count: 1,
                    stride: TMD_HEADER_SIZE + TMD_OBJECT_SIZE,
                })?;
        let object = slice(self.bytes, object_at, TMD_OBJECT_SIZE, "TMD object zero")?;
        let relative = offset(read_u32(object, 0), "TMD vertex")?;
        let count = read_u32(object, 4) as u16;
        let vertex_at = object_at
            .checked_add(relative)
            .ok_or(AnimationError::RangeOverflow {
                field: "TMD vertices",
                at: object_at,
                count: usize::from(count),
                stride: VERTEX_SIZE,
            })?;
        let bytes = checked_range(
            self.bytes,
            vertex_at,
            usize::from(count),
            VERTEX_SIZE,
            "TMD vertices",
        )?;
        Ok((bytes, count, vertex_at))
    }
}

pub struct Clips<'a> {
    animation: Animation<'a>,
    index: usize,
}

impl<'a> Iterator for Clips<'a> {
    type Item = Result<Clip<'a>, AnimationError>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.animation.clip_count {
            return None;
        }
        let index = match u16::try_from(self.index) {
            Ok(index) => index,
            Err(_) => {
                self.index = self.animation.clip_count;
                return Some(Err(AnimationError::ClipOutOfRange {
                    index: u16::MAX,
                    count: self.animation.clip_count,
                }));
            }
        };
        self.index += 1;
        Some(self.animation.clip(index))
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Clip<'a> {
    animation: Animation<'a>,
    pub index: u16,
    pub offset: usize,
    pub keyframe_count: u16,
    pub unknown_02: u16,
}

impl<'a> Clip<'a> {
    pub const fn keyframes(&self) -> Keyframes<'a> {
        Keyframes {
            clip: *self,
            index: 0,
        }
    }

    pub fn keyframe(&self, index: u16) -> Result<Keyframe<'a>, AnimationError> {
        if index >= self.keyframe_count {
            return Err(AnimationError::KeyframeOutOfRange {
                index,
                count: self.keyframe_count,
            });
        }
        let table = self.offset + CLIP_HEADER_SIZE;
        let entry = table + usize::from(index) * 4;
        let keyframe_at = offset(read_u32(self.animation.bytes, entry), "keyframe")?;
        let header = slice(
            self.animation.bytes,
            keyframe_at,
            KEYFRAME_HEADER_SIZE,
            "keyframe header",
        )?;
        let morph_count = read_u16(header, 6);
        let indices = checked_range(
            self.animation.bytes,
            keyframe_at + KEYFRAME_HEADER_SIZE,
            usize::from(morph_count),
            2,
            "morph-index table",
        )?;
        Ok(Keyframe {
            animation: self.animation,
            clip_index: self.index,
            index,
            offset: keyframe_at,
            reverse: read_u16(header, 0),
            duration: read_u16(header, 2),
            rest_index: read_u16(header, 4),
            morph_count,
            morph_indices: indices,
        })
    }
}

pub struct Keyframes<'a> {
    clip: Clip<'a>,
    index: u16,
}

impl<'a> Iterator for Keyframes<'a> {
    type Item = Result<Keyframe<'a>, AnimationError>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.clip.keyframe_count {
            return None;
        }
        let index = self.index;
        self.index = self.index.wrapping_add(1);
        Some(self.clip.keyframe(index))
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Keyframe<'a> {
    animation: Animation<'a>,
    pub clip_index: u16,
    pub index: u16,
    pub offset: usize,
    pub reverse: u16,
    pub duration: u16,
    pub rest_index: u16,
    pub morph_count: u16,
    morph_indices: &'a [u8],
}

impl Keyframe<'_> {
    pub const fn morph_indices(&self) -> MorphIndices<'_> {
        MorphIndices {
            bytes: self.morph_indices,
            index: 0,
        }
    }

    pub fn rest_morph(&self) -> Result<MorphObject<'_>, AnimationError> {
        self.animation.morph_object(self.rest_index)
    }
}

pub struct MorphIndices<'a> {
    bytes: &'a [u8],
    index: usize,
}

impl Iterator for MorphIndices<'_> {
    type Item = u16;

    fn next(&mut self) -> Option<Self::Item> {
        let at = self.index.checked_mul(2)?;
        let bytes = self.bytes.get(at..at + 2)?;
        self.index += 1;
        Some(read_u16(bytes, 0))
    }
}

#[derive(Debug, Clone, Copy)]
pub struct MorphObject<'a> {
    pub index: u16,
    pub offset: usize,
    pub unknown_00: u32,
    pub base_vertex: u32,
    pub vertex_count: u32,
    deltas: &'a [u8],
}

impl MorphObject<'_> {
    pub const fn deltas(&self) -> MorphDeltas<'_> {
        MorphDeltas {
            bytes: self.deltas,
            index: 0,
        }
    }

    pub fn delta(&self, index: u32) -> Result<Vertex, AnimationError> {
        if index >= self.vertex_count {
            return Err(AnimationError::MorphDeltaOutOfRange {
                index,
                count: self.vertex_count,
            });
        }
        let at = usize::try_from(index)
            .ok()
            .and_then(|index| index.checked_mul(VERTEX_SIZE))
            .ok_or(AnimationError::RangeOverflow {
                field: "morph delta",
                at: 0,
                count: usize::MAX,
                stride: VERTEX_SIZE,
            })?;
        Ok(read_vertex(&self.deltas[at..at + VERTEX_SIZE]))
    }
}

pub struct MorphDeltas<'a> {
    bytes: &'a [u8],
    index: usize,
}

impl Iterator for MorphDeltas<'_> {
    type Item = Vertex;

    fn next(&mut self) -> Option<Self::Item> {
        let at = self.index.checked_mul(VERTEX_SIZE)?;
        let bytes = self.bytes.get(at..at + VERTEX_SIZE)?;
        self.index += 1;
        Some(read_vertex(bytes))
    }
}

pub struct BaseVertices<'a> {
    bytes: &'a [u8],
    count: u16,
    index: u16,
}

impl Iterator for BaseVertices<'_> {
    type Item = Vertex;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.count {
            return None;
        }
        let at = usize::from(self.index) * VERTEX_SIZE;
        self.index = self.index.wrapping_add(1);
        Some(read_vertex(&self.bytes[at..at + VERTEX_SIZE]))
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct FrameSelection {
    pub clip_index: u16,
    pub keyframe_ordinal: u16,
    pub keyframe_index: u16,
    pub fraction: u16,
    pub rest_index: u16,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CacheKey {
    pub clip_index: u16,
    pub keyframe_index: u16,
    pub rest_index: u16,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct BindReport {
    pub selection: FrameSelection,
    pub cache_key: CacheKey,
    pub cache_rebuilt: bool,
    pub vertex_count: usize,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ValidationReport {
    pub clip_count: usize,
    pub keyframe_count: usize,
    pub morph_reference_count: usize,
    pub vertex_count: u16,
}

pub fn select_frame(
    clip: Clip<'_>,
    phase: u16,
    initial_kf_index: u16,
) -> Result<FrameSelection, AnimationError> {
    if clip.keyframe_count == 0 {
        return Err(AnimationError::EmptyClip { index: clip.index });
    }

    let mut accumulated = 0u32;
    let mut previous = 0u32;
    let mut cache_index = initial_kf_index;
    let mut last = None;
    for ordinal in 0..clip.keyframe_count {
        let keyframe = clip.keyframe(ordinal)?;
        if keyframe.duration == 0 {
            return Err(AnimationError::ZeroDuration {
                clip: clip.index,
                keyframe: ordinal,
            });
        }
        last = Some(keyframe);
        accumulated = accumulated.wrapping_add(u32::from(keyframe.duration));
        if phase < accumulated as u16 {
            let elapsed = phase.wrapping_sub(previous as u16);
            let forward = ((u32::from(elapsed) << 12) / u32::from(keyframe.duration)) as u16;
            let fraction = if keyframe.reverse == 0 {
                forward
            } else {
                FULL_WEIGHT.wrapping_sub(forward)
            };
            return Ok(FrameSelection {
                clip_index: clip.index,
                keyframe_ordinal: ordinal,
                keyframe_index: cache_index,
                fraction,
                rest_index: keyframe.rest_index,
            });
        }
        previous = accumulated;
        cache_index = cache_index.wrapping_add(1);
    }

    let keyframe = last.expect("non-empty clip has a last keyframe");
    Ok(FrameSelection {
        clip_index: clip.index,
        keyframe_ordinal: keyframe.index,
        keyframe_index: cache_index.wrapping_sub(1),
        fraction: FULL_WEIGHT,
        rest_index: keyframe.rest_index,
    })
}

fn apply_object(
    vertices: &mut [Vertex],
    object: MorphObject<'_>,
    weight: u16,
) -> Result<(), AnimationError> {
    let base = usize::try_from(object.base_vertex).map_err(|_| {
        AnimationError::VertexRangeOutOfBounds {
            object: object.index,
            base: object.base_vertex,
            count: object.vertex_count,
            vertices: u16::try_from(vertices.len()).unwrap_or(u16::MAX),
        }
    })?;
    let count = usize::try_from(object.vertex_count).map_err(|_| {
        AnimationError::VertexRangeOutOfBounds {
            object: object.index,
            base: object.base_vertex,
            count: object.vertex_count,
            vertices: u16::try_from(vertices.len()).unwrap_or(u16::MAX),
        }
    })?;
    let Some(end) = base.checked_add(count) else {
        return Err(AnimationError::VertexRangeOutOfBounds {
            object: object.index,
            base: object.base_vertex,
            count: object.vertex_count,
            vertices: u16::try_from(vertices.len()).unwrap_or(u16::MAX),
        });
    };
    let Some(destination) = vertices.get_mut(base..end) else {
        return Err(AnimationError::VertexRangeOutOfBounds {
            object: object.index,
            base: object.base_vertex,
            count: object.vertex_count,
            vertices: u16::try_from(vertices.len()).unwrap_or(u16::MAX),
        });
    };
    for (vertex, delta) in destination.iter_mut().zip(object.deltas()) {
        vertex.vx = vertex.vx.wrapping_add(scale_delta(delta.vx, weight));
        vertex.vy = vertex.vy.wrapping_add(scale_delta(delta.vy, weight));
        vertex.vz = vertex.vz.wrapping_add(scale_delta(delta.vz, weight));
    }
    Ok(())
}

fn validate_object_range(object: MorphObject<'_>, vertices: u16) -> Result<(), AnimationError> {
    if object.base_vertex.checked_add(object.vertex_count) > Some(u32::from(vertices)) {
        return Err(AnimationError::VertexRangeOutOfBounds {
            object: object.index,
            base: object.base_vertex,
            count: object.vertex_count,
            vertices,
        });
    }
    Ok(())
}

fn scale_delta(delta: i16, weight: u16) -> i16 {
    let scaled = (i64::from(delta) * i64::from(weight)) >> 12;
    scaled.clamp(i64::from(i16::MIN), i64::from(i16::MAX)) as i16
}

fn offset(value: u32, field: &'static str) -> Result<usize, AnimationError> {
    usize::try_from(value).map_err(|_| AnimationError::OffsetOverflow {
        field,
        offset: value,
    })
}

fn checked_range<'a>(
    bytes: &'a [u8],
    at: usize,
    count: usize,
    stride: usize,
    field: &'static str,
) -> Result<&'a [u8], AnimationError> {
    let need = count
        .checked_mul(stride)
        .ok_or(AnimationError::RangeOverflow {
            field,
            at,
            count,
            stride,
        })?;
    slice(bytes, at, need, field)
}

fn slice<'a>(
    bytes: &'a [u8],
    at: usize,
    need: usize,
    field: &'static str,
) -> Result<&'a [u8], AnimationError> {
    let available = bytes.len().saturating_sub(at);
    let end = at.checked_add(need).ok_or(AnimationError::RangeOverflow {
        field,
        at,
        count: need,
        stride: 1,
    })?;
    bytes.get(at..end).ok_or(AnimationError::OutOfBounds {
        field,
        at,
        need,
        available,
    })
}

fn read_vertex(bytes: &[u8]) -> Vertex {
    Vertex::from_le_bytes([
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
    ])
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}
