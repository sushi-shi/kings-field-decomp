//! Immutable animation data for the port, without the retail pool/cache addresses.
use crate::animation::{Animation, MorphObject};
use crate::mesh::{MeshError, MeshVector, Model};
use core::mem::MaybeUninit;

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct AnimationInfo {
    pub model_offset: u32,
    pub model_size: u32,
    pub vertices: u32,
    pub clips: u32,
    pub keyframes: u32,
    pub morphs: u32,
    pub deltas: u32,
}
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct AnimationClip {
    pub first_frame: u32,
    pub frame_count: u32,
}
#[repr(u16)]
#[derive(Clone, Copy, Default)]
pub enum BlendDirection {
    #[default]
    Forward = 0,
    Reverse = 1,
}
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct AnimationFrame {
    pub duration: u16,
    pub direction: BlendDirection,
    pub first_morph: u32,
    pub morph_count: u32,
    pub rest_morph: u32,
}
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct AnimationMorph {
    pub first_vertex: u32,
    pub vertex_count: u32,
    pub first_delta: u32,
}
pub struct AnimationModel<'a> {
    animation: Animation<'a>,
    pub info: AnimationInfo,
}
fn charge_morph(info: &mut AnimationInfo, object: MorphObject<'_>) -> Result<(), MeshError> {
    if object.base_vertex > info.vertices
        || object.vertex_count > info.vertices - object.base_vertex
        || object.vertex_count > 1_000_000 - info.deltas
    {
        return Err(MeshError::InvalidIndex);
    }
    info.deltas += object.vertex_count;
    Ok(())
}
impl<'a> AnimationModel<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, MeshError> {
        let animation = Animation::parse(bytes).map_err(|_| MeshError::Truncated)?;
        let header = animation.header();
        let model_bytes = animation
            .bytes()
            .get(header.tmd_data_offset as usize..)
            .ok_or(MeshError::Truncated)?;
        let model = Model::parse(model_bytes)?;
        let vertex_count = model.object(0)?.vertices.len() / 8;
        // Clip IDs are bytes; 255 remains the semantic "no clip" sentinel.
        if vertex_count == 0 || vertex_count > u16::MAX as usize || animation.clip_count() > 255 {
            return Err(MeshError::Unsupported);
        }
        let mut info = AnimationInfo {
            model_offset: header.tmd_data_offset,
            model_size: model_bytes.len() as u32,
            vertices: vertex_count as u32,
            clips: animation.clip_count() as u32,
            ..AnimationInfo::default()
        };
        for clip in animation.clips() {
            let clip = clip.map_err(|_| MeshError::Truncated)?;
            if clip.keyframe_count == 0 || u32::from(clip.keyframe_count) > 4096 - info.keyframes {
                return Err(MeshError::Unsupported);
            }
            info.keyframes += u32::from(clip.keyframe_count);
            let mut duration = 0u32;
            for frame in clip.keyframes() {
                let frame = frame.map_err(|_| MeshError::Truncated)?;
                duration += u32::from(frame.duration);
                if frame.duration == 0
                    || duration > u16::MAX as u32
                    || frame.reverse > 1
                    || u32::from(frame.morph_count) + 1 > 65536 - info.morphs
                {
                    return Err(MeshError::Unsupported);
                }
                info.morphs += u32::from(frame.morph_count) + 1;
                for index in frame
                    .morph_indices()
                    .chain(core::iter::once(frame.rest_index))
                {
                    let object = animation
                        .morph_object(index)
                        .map_err(|_| MeshError::Truncated)?;
                    charge_morph(&mut info, object)?;
                }
            }
        }
        Ok(Self { animation, info })
    }
    // All metadata and work budgets are checked by parse before any output is touched.
    pub fn decode(
        &self,
        clips: &mut [MaybeUninit<AnimationClip>],
        frames: &mut [MaybeUninit<AnimationFrame>],
        morphs: &mut [MaybeUninit<AnimationMorph>],
        deltas: &mut [MaybeUninit<MeshVector>],
    ) -> Result<(), MeshError> {
        if clips.len() < self.info.clips as usize
            || frames.len() < self.info.keyframes as usize
            || morphs.len() < self.info.morphs as usize
            || deltas.len() < self.info.deltas as usize
        {
            return Err(MeshError::Truncated);
        }
        let mut frame_at = 0;
        let mut morph_at = 0;
        let mut delta_at = 0;
        for (clip_at, clip) in self.animation.clips().enumerate() {
            let clip = clip.expect("preflighted animation clip");
            clips[clip_at].write(AnimationClip {
                first_frame: frame_at as u32,
                frame_count: u32::from(clip.keyframe_count),
            });
            for frame in clip.keyframes() {
                let frame = frame.expect("preflighted animation frame");
                frames[frame_at].write(AnimationFrame {
                    duration: frame.duration,
                    direction: if frame.reverse == 0 {
                        BlendDirection::Forward
                    } else {
                        BlendDirection::Reverse
                    },
                    first_morph: morph_at as u32,
                    morph_count: u32::from(frame.morph_count),
                    rest_morph: morph_at as u32 + u32::from(frame.morph_count),
                });
                frame_at += 1;
                for index in frame
                    .morph_indices()
                    .chain(core::iter::once(frame.rest_index))
                {
                    let object = self
                        .animation
                        .morph_object(index)
                        .expect("preflighted morph");
                    morphs[morph_at].write(AnimationMorph {
                        first_vertex: object.base_vertex,
                        vertex_count: object.vertex_count,
                        first_delta: delta_at as u32,
                    });
                    morph_at += 1;
                    for vertex in object.deltas() {
                        deltas[delta_at].write(MeshVector {
                            x: vertex.vx,
                            y: vertex.vy,
                            z: vertex.vz,
                        });
                        delta_at += 1;
                    }
                }
            }
        }
        Ok(())
    }
}
