//! Bounded, immutable TMD decoding for the portable renderer. Unlike retail
//! preparation, indices remain indices and no pointers are written into assets.
use crate::tmd::{F3, F4, FT3, FT4, G3, G4, GT3, GT4, HEADER_SIZE, OBJECT_SIZE};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum MeshError {
    Truncated,
    Unsupported,
    Overflow,
    InvalidIndex,
}

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct MeshInfo {
    pub objects: u32,
    pub vertices: u32,
    pub normals: u32,
    pub faces: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct MeshObject {
    pub first_vertex: u32,
    pub vertex_count: u32,
    pub first_normal: u32,
    pub normal_count: u32,
    pub first_face: u32,
    pub face_count: u32,
    pub scale: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct MeshVector {
    pub x: i16,
    pub y: i16,
    pub z: i16,
}

pub const TEXTURED: u32 = 1;
pub const GOURAUD: u32 = 2;
pub const SEMITRANSPARENT: u32 = 4;
pub const DOUBLE_SIDED: u32 = 8;

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct MeshFace {
    pub vertices: [u32; 4],
    pub normals: [u32; 4],
    pub texture_page: u16,
    pub palette: u16,
    pub uv: [[u8; 2]; 4],
    pub color: [u8; 3],
    pub corner_count: u8,
    pub flags: u32,
}

pub struct Model<'a> {
    bytes: &'a [u8],
    pub info: MeshInfo,
}
pub struct Object<'a> {
    pub vertices: &'a [u8],
    pub normals: &'a [u8],
    packets: &'a [u8],
    pub face_count: u32,
    pub scale: i32,
}
pub struct Faces<'a> {
    bytes: &'a [u8],
    remaining: u32,
    stopped: bool,
}

fn span(bytes: &[u8], at: usize, size: usize) -> Result<&[u8], MeshError> {
    bytes
        .get(at..at.checked_add(size).ok_or(MeshError::Overflow)?)
        .ok_or(MeshError::Truncated)
}
fn u16_at(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}
fn u32_at(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}
pub fn vector(bytes: &[u8]) -> MeshVector {
    MeshVector {
        x: u16_at(bytes, 0) as i16,
        y: u16_at(bytes, 2) as i16,
        z: u16_at(bytes, 4) as i16,
    }
}
fn vectors(bytes: &[u8], relative: u32, count: u32) -> Result<&[u8], MeshError> {
    let at = HEADER_SIZE
        .checked_add(relative as usize)
        .ok_or(MeshError::Overflow)?;
    span(
        bytes,
        at,
        (count as usize).checked_mul(8).ok_or(MeshError::Overflow)?,
    )
}
impl<'a> Model<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, MeshError> {
        let header = span(bytes, 0, HEADER_SIZE)?;
        if u32_at(header, 0) != 0x41 || u32_at(header, 4) != 0 {
            return Err(MeshError::Unsupported);
        }
        let count = u32_at(header, 8);
        if count > 4096 {
            return Err(MeshError::Unsupported);
        }
        span(
            bytes,
            HEADER_SIZE,
            (count as usize)
                .checked_mul(OBJECT_SIZE)
                .ok_or(MeshError::Overflow)?,
        )?;
        let mut model = Self {
            bytes,
            info: MeshInfo {
                objects: count,
                ..MeshInfo::default()
            },
        };
        for index in 0..count {
            let object = model.object(index)?;
            let nv = (object.vertices.len() / 8) as u32;
            let nn = (object.normals.len() / 8) as u32;
            if nv > 1_000_000 - model.info.vertices
                || nn > 1_000_000 - model.info.normals
                || object.face_count > 1_000_000 - model.info.faces
            {
                return Err(MeshError::Unsupported);
            }
            for face in object.faces() {
                let face = face?;
                for corner in 0..face.corner_count as usize {
                    if face.vertices[corner] >= nv || face.normals[corner] >= nn {
                        return Err(MeshError::InvalidIndex);
                    }
                }
            }
            model.info.vertices = model
                .info
                .vertices
                .checked_add(nv)
                .ok_or(MeshError::Overflow)?;
            model.info.normals = model
                .info
                .normals
                .checked_add(nn)
                .ok_or(MeshError::Overflow)?;
            model.info.faces = model
                .info
                .faces
                .checked_add(object.face_count)
                .ok_or(MeshError::Overflow)?;
        }
        Ok(model)
    }
    pub fn object(&self, index: u32) -> Result<Object<'a>, MeshError> {
        if index >= self.info.objects {
            return Err(MeshError::InvalidIndex);
        }
        let at = HEADER_SIZE + index as usize * OBJECT_SIZE;
        let row = span(self.bytes, at, OBJECT_SIZE)?;
        let vertices = vectors(self.bytes, u32_at(row, 0), u32_at(row, 4))?;
        let normals = vectors(self.bytes, u32_at(row, 8), u32_at(row, 12))?;
        let packet_at = HEADER_SIZE
            .checked_add(u32_at(row, 16) as usize)
            .ok_or(MeshError::Overflow)?;
        let packets = self.bytes.get(packet_at..).ok_or(MeshError::Truncated)?;
        let scale = u32_at(row, 24) as i32;
        if !(-30..=30).contains(&scale) {
            return Err(MeshError::Unsupported);
        }
        Ok(Object {
            vertices,
            normals,
            packets,
            face_count: u32_at(row, 20),
            scale,
        })
    }
}
pub fn validate_archive(bytes: &[u8]) -> Result<(), MeshError> {
    let archive = crate::asset_archive::Archive::parse(bytes).map_err(|_| MeshError::Truncated)?;
    if archive.count() > 1024 {
        return Err(MeshError::Unsupported);
    }
    let mut faces = 0;
    for asset in archive.assets() {
        let asset = asset.map_err(|_| MeshError::Truncated)?;
        let model = Model::parse(asset.tmd_data().ok_or(MeshError::Truncated)?)?;
        faces += model.info.faces;
        if faces > 1_000_000 {
            return Err(MeshError::Unsupported);
        }
    }
    Ok(())
}
impl<'a> Object<'a> {
    pub fn faces(&self) -> Faces<'a> {
        Faces {
            bytes: self.packets,
            remaining: self.face_count,
            stopped: false,
        }
    }
}
impl Iterator for Faces<'_> {
    type Item = Result<MeshFace, MeshError>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.stopped || self.remaining == 0 {
            return None;
        }
        let result = decode_face(self.bytes);
        match result {
            Ok((face, size)) => {
                self.bytes = &self.bytes[size..];
                self.remaining -= 1;
                Some(Ok(face))
            }
            Err(error) => {
                self.stopped = true;
                Some(Err(error))
            }
        }
    }
}
fn decode_face(bytes: &[u8]) -> Result<(MeshFace, usize), MeshError> {
    let header = span(bytes, 0, 4)?;
    let body_size = header[1] as usize * 4;
    let body = span(bytes, 4, body_size)?;
    // No-light/gradation packets have different layouts; do not silently treat
    // them as the lit eight-mode family used by the shipped floor models.
    if header[2] & !2 != 0 {
        return Err(MeshError::Unsupported);
    }
    let mode = header[3] & !2;
    let (count, textured, gouraud, minimum) = match mode {
        F3 => (3, false, false, 12),
        F4 => (4, false, false, 16),
        G3 => (3, false, true, 16),
        G4 => (4, false, true, 20),
        FT3 => (3, true, false, 20),
        FT4 => (4, true, false, 28),
        GT3 => (3, true, true, 24),
        GT4 => (4, true, true, 32),
        _ => return Err(MeshError::Unsupported),
    };
    if body_size != minimum {
        return Err(MeshError::Unsupported);
    }
    let mut face = MeshFace {
        corner_count: count as u8,
        color: [128; 3],
        ..MeshFace::default()
    };
    if textured {
        face.flags |= TEXTURED;
        face.palette = u16_at(body, 2);
        face.texture_page = u16_at(body, 6);
        for i in 0..count {
            face.uv[i] = [body[i * 4], body[i * 4 + 1]];
        }
    } else {
        face.color.copy_from_slice(&body[..3]);
    }
    if gouraud {
        face.flags |= GOURAUD;
    }
    if header[3] & 2 != 0 {
        face.flags |= SEMITRANSPARENT;
    }
    if header[2] & 2 != 0 {
        face.flags |= DOUBLE_SIDED;
    }
    let index_at = if textured { count * 4 } else { 4 };
    for i in 0..count {
        let normal_at = index_at + if gouraud { i * 4 } else { 0 };
        let vertex_at = if gouraud {
            normal_at + 2
        } else {
            index_at + 2 + i * 2
        };
        face.normals[i] = u16_at(body, normal_at) as u32;
        face.vertices[i] = u16_at(body, vertex_at) as u32;
    }
    Ok((face, 4 + body_size))
}
