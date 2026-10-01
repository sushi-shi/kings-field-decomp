//! SLUS-00255 MO/TMD and RTIM conversion. Output is the same deterministic KFA1
//! presentation pack as scripts/kf3_assets.py; no retail bytes are built in.
use super::disc::{span, u16_at, u32_at};
use std::collections::BTreeMap;
type Result<T> = std::result::Result<T, &'static str>;

struct Archive<'a> {
    data: &'a [u8],
    offsets: Vec<usize>,
}
impl<'a> Archive<'a> {
    fn new(data: &'a [u8]) -> Result<Self> {
        let count = u16_at(data, 0)? as usize;
        if !(1..=1022).contains(&count) || data.len() % 2048 != 0 {
            return Err("Invalid KFIII archive header");
        }
        let offsets = (0..=count)
            .map(|i| u16_at(data, 2 + i * 2).map(|v| v as usize * 2048))
            .collect::<Result<Vec<_>>>()?;
        if offsets[0] != 2048
            || *offsets.last().unwrap() != data.len()
            || offsets.windows(2).any(|w| w[0] > w[1])
        {
            return Err("Invalid KFIII archive offsets");
        }
        Ok(Self { data, offsets })
    }
    fn get(&self, slot: usize) -> Result<&'a [u8]> {
        let &start = self.offsets.get(slot).ok_or("Missing archive slot")?;
        let end = *self
            .offsets
            .iter()
            .skip(slot + 1)
            .find(|&&v| v > start)
            .ok_or("Empty archive slot")?;
        span(self.data, start, end - start)
    }
}
#[derive(Clone)]
struct Face {
    vertices: Vec<usize>,
    normals: Vec<usize>,
    colors: Vec<[u8; 3]>,
    uv: Vec<[u8; 2]>,
    material: Option<(u16, u16)>,
    unlit: bool,
}
struct Object {
    vertices: Vec<[i16; 3]>,
    normals: Vec<[i16; 3]>,
    faces: Vec<Face>,
}
fn vectors(data: &[u8], base: usize, offset: usize, count: usize) -> Result<Vec<[i16; 3]>> {
    let at = base.checked_add(offset).ok_or("Mesh offset overflow")?;
    let bytes = span(data, at, count * 8)?;
    (0..count)
        .map(|i| {
            Ok([
                u16_at(bytes, i * 8)? as i16,
                u16_at(bytes, i * 8 + 2)? as i16,
                u16_at(bytes, i * 8 + 4)? as i16,
            ])
        })
        .collect()
}
fn model(data: &[u8]) -> Result<Vec<Object>> {
    let size = u32_at(data, 0)? as usize;
    let animations = u32_at(data, 4)?;
    let tmd = u32_at(data, 8)? as usize;
    if size < 12 || animations > 256 || tmd < 12 || tmd >= size {
        return Err("Invalid KFIII model header");
    }
    let data = span(data, 0, size)?;
    if u32_at(data, tmd)? != 0x41 || u32_at(data, tmd + 4)? != 0 {
        return Err("Expected a relative TMD model");
    }
    let count = u32_at(data, tmd + 8)? as usize;
    if !(1..=64).contains(&count) {
        return Err("Invalid TMD object count");
    }
    let base = tmd + 12;
    let table = span(data, base, count * 28)?;
    let (mut objects, mut triangles, mut total_vertices) = (Vec::new(), 0, 0);
    for i in 0..count {
        let row = &table[i * 28..][..28];
        let nv = u32_at(row, 4)? as usize;
        let nn = u32_at(row, 12)? as usize;
        let np = u32_at(row, 20)? as usize;
        total_vertices += nv.min(16385) + nn.min(16385);
        if !(1..=16384).contains(&nv)
            || nn > 16384
            || np > 4096
            || total_vertices > 65536
            || u32_at(row, 24)? != 0
        {
            return Err("Character exceeds mesh limits");
        }
        let vertices = vectors(data, base, u32_at(row, 0)? as usize, nv)?;
        let normals = vectors(data, base, u32_at(row, 8)? as usize, nn)?;
        if normals
            .iter()
            .flatten()
            .any(|&n| !(-4096..=4096).contains(&n))
        {
            return Err("Invalid character normal");
        }
        let mut cursor = base
            .checked_add(u32_at(row, 16)? as usize)
            .ok_or("Primitive offset overflow")?;
        let mut faces = Vec::new();
        for _ in 0..np {
            let header = span(data, cursor, 4)?;
            let (words, flags, mode) = (header[1] as usize, header[2], header[3]);
            let body = span(data, cursor + 4, words * 4)?;
            cursor += 4 + words * 4;
            if mode & 0xe0 != 0x20 || flags & !7 != 0 {
                return Err("Unsupported TMD primitive");
            }
            let corners = if mode & 8 != 0 { 4 } else { 3 };
            let textured = mode & 4 != 0;
            let smooth = mode & 16 != 0;
            let unlit = flags & 1 != 0;
            triangles += corners - 2;
            if triangles > 4096 {
                return Err("Character triangle limit exceeded");
            }
            let mut indices = if textured { corners * 4 } else { 4 };
            let mut colors = vec![[128; 3]; corners];
            if !textured || unlit {
                let color_at = if textured { corners * 4 } else { 0 };
                let ncolors = if flags & 4 != 0 { corners } else { 1 };
                for (j, color) in colors.iter_mut().enumerate() {
                    color.copy_from_slice(span(
                        body,
                        color_at + if ncolors == 1 { 0 } else { j * 4 },
                        3,
                    )?);
                }
                indices = color_at + ncolors * 4;
            }
            let mut vi = Vec::new();
            let mut ni = Vec::new();
            if !unlit && !smooth {
                ni.push(u16_at(body, indices)? as usize);
                indices += 2;
            }
            for _ in 0..corners {
                if !unlit && smooth {
                    ni.push(u16_at(body, indices)? as usize);
                    indices += 2;
                }
                vi.push(u16_at(body, indices)? as usize);
                indices += 2;
            }
            if vi.iter().any(|&v| v >= nv) || ni.iter().any(|&n| n >= nn) {
                return Err("Invalid TMD vertex or normal reference");
            }
            let mut uv = Vec::new();
            let material = if textured {
                for j in 0..corners {
                    let b = span(body, j * 4, 2)?;
                    uv.push([b[0], b[1]]);
                }
                Some((u16_at(body, 6)?, u16_at(body, 2)?))
            } else {
                None
            };
            faces.push(Face {
                vertices: vi,
                normals: ni,
                colors,
                uv,
                material,
                unlit,
            });
        }
        objects.push(Object {
            vertices,
            normals,
            faces,
        });
    }
    if triangles == 0 {
        return Err("Empty character mesh");
    }
    Ok(objects)
}
#[derive(Clone)]
struct Memory {
    words: Vec<u16>,
    present: Vec<bool>,
}
impl Memory {
    fn new() -> Self {
        Self {
            words: vec![0; 1024 * 512],
            present: vec![false; 1024 * 512],
        }
    }
    fn word(&self, at: usize) -> Result<u16> {
        if !self.present.get(at).copied().unwrap_or(false) {
            return Err("Character texture references unloaded memory");
        }
        Ok(self.words[at])
    }
    fn pixel(&self, page: u16, clut: u16, u: usize, v: usize) -> Result<[u8; 4]> {
        let mode = (page >> 7) & 3;
        if mode > 2 {
            return Err("Unsupported texture mode");
        }
        let divisor = 4usize >> mode;
        let x = usize::from(page & 15) * 64 + u / divisor;
        if x >= 1024 {
            return Err("Texture page exceeds image memory");
        }
        let mut word = self.word((usize::from((page >> 4) & 1) * 256 + v) * 1024 + x)?;
        if mode != 2 {
            let bits = 4 << mode;
            let index = (word >> ((u % divisor) * bits)) & ((1 << bits) - 1);
            word = self.word(
                usize::from(clut >> 6) * 1024 + usize::from(clut & 63) * 16 + usize::from(index),
            )?;
        }
        let mut rgba = [0, 0, 0, if word == 0 { 0 } else { 255 }];
        for (channel, shift) in [0, 5, 10].into_iter().enumerate() {
            let c = ((word >> shift) & 31) as u8;
            rgba[channel] = (c << 3) | (c >> 2);
        }
        Ok(rgba)
    }
}
fn rtim(data: &[u8], mut memory: Option<&mut Memory>, prefix: bool) -> Result<usize> {
    let mut at = 0;
    while at < data.len() {
        let header = span(data, at, 16)?;
        if header.iter().all(|&v| v == 0) || header.iter().all(|&v| v == 255) {
            return Ok(at);
        }
        if prefix && at != 0 && header[..8] != header[8..] {
            return Ok(at);
        }
        for _ in 0..2 {
            let header = span(data, at, 16)?;
            if header[..8] != header[8..] {
                return Err("RTIM rectangle header copies disagree");
            }
            let (x, y, w, h) = (
                u16_at(header, 0)? as usize,
                u16_at(header, 2)? as usize,
                u16_at(header, 4)? as usize,
                u16_at(header, 6)? as usize,
            );
            if w == 0 || h == 0 || x + w > 1024 || y + h > 512 {
                return Err("RTIM rectangle exceeds texture memory");
            }
            let words = span(data, at + 16, w * h * 2)?;
            if let Some(memory) = memory.as_deref_mut() {
                for row in 0..h {
                    for col in 0..w {
                        let to = (y + row) * 1024 + x + col;
                        memory.words[to] = u16_at(words, (row * w + col) * 2)?;
                        memory.present[to] = true;
                    }
                }
            }
            at += 16 + words.len();
        }
    }
    Ok(at)
}
fn scaled(value: i32, height: i32) -> Result<i16> {
    // Match Python's nearest-even rounding using integer arithmetic.
    let value = i64::from(value) * 2000;
    let height = i64::from(height);
    let (q, r) = (value.div_euclid(height), value.rem_euclid(height));
    let rounded = q + i64::from(r * 2 > height || (r * 2 == height && q & 1 != 0));
    if !(-8192..=8192).contains(&rounded) {
        return Err("Character position exceeds presentation bounds");
    }
    Ok(rounded as i16)
}
fn mesh(objects: &[Object], memory: &Memory, slot: u16) -> Result<Vec<u8>> {
    let mut materials: BTreeMap<(u16, u16), [usize; 5]> = BTreeMap::new();
    let (mut top, mut bottom) = (i16::MAX, i16::MIN);
    for object in objects {
        for v in &object.vertices {
            top = top.min(v[1]);
            bottom = bottom.max(v[1]);
        }
        for face in &object.faces {
            if let Some(key) = face.material {
                let b = materials.entry(key).or_insert([256, 256, 0, 0, 0]);
                for &[u, v] in &face.uv {
                    b[0] = b[0].min(u as usize);
                    b[1] = b[1].min(v as usize);
                    b[2] = b[2].max(u as usize);
                    b[3] = b[3].max(v as usize);
                }
            }
        }
    }
    if top >= bottom {
        return Err("Character has no height");
    }
    let mut atlas = vec![255u8; 1024];
    let mut height = 1;
    for (&(page, clut), bounds) in &mut materials {
        let &[x, y, right, lower, _] = &*bounds;
        let rows = lower - y + 1;
        if height + rows > 4096 {
            return Err("Character atlas exceeds limits");
        }
        bounds[4] = height;
        for v in y..=lower {
            for u in x..=right {
                atlas.extend_from_slice(&memory.pixel(page, clut, u, v)?);
            }
            atlas.resize(atlas.len() + (256 - (right - x + 1)) * 4, 0);
        }
        height += rows;
    }
    let mut triangles = Vec::new();
    for object in objects {
        for face in &object.faces {
            let indices: &[usize] = if face.vertices.len() == 4 {
                &[0, 1, 2, 1, 3, 2]
            } else {
                &[0, 1, 2]
            };
            for &corner in indices {
                let [x, y, z] = object.vertices[face.vertices[corner]];
                for c in [i32::from(x), i32::from(y) - i32::from(bottom), i32::from(z)] {
                    triangles.extend_from_slice(
                        &scaled(c, i32::from(bottom) - i32::from(top))?.to_le_bytes(),
                    );
                }
                let normal = if face.unlit {
                    [0; 3]
                } else {
                    object.normals[face.normals[if face.normals.len() == 1 { 0 } else { corner }]]
                };
                for n in normal {
                    triangles.extend_from_slice(&n.to_le_bytes());
                }
                let (u, v) = if let Some(key) = face.material {
                    let b = materials[&key];
                    (
                        face.uv[corner][0] as usize - b[0],
                        face.uv[corner][1] as usize - b[1] + b[4],
                    )
                } else {
                    (0, 0)
                };
                triangles.extend_from_slice(&(u as u16).to_le_bytes());
                triangles.extend_from_slice(&(v as u16).to_le_bytes());
                triangles.extend_from_slice(&face.colors[corner]);
                triangles.push(u8::from(face.unlit));
            }
        }
    }
    let mut output = Vec::new();
    output.extend_from_slice(&slot.to_le_bytes());
    output.extend_from_slice(&(height as u16).to_le_bytes());
    output.extend_from_slice(&((triangles.len() / 60) as u32).to_le_bytes());
    output.extend_from_slice(&triangles);
    output.extend_from_slice(&atlas);
    Ok(output)
}
pub fn convert(mo: &[u8], rtim_bytes: &[u8], fdat: &[u8], mof: &[u8]) -> Result<Vec<u8>> {
    let (mo, textures, fdat, mof) = (
        Archive::new(mo)?,
        Archive::new(rtim_bytes)?,
        Archive::new(fdat)?,
        Archive::new(mof)?,
    );
    let mut common = Memory::new();
    rtim(fdat.get(84)?, Some(&mut common), false)?;
    let mut areas = [None; 43];
    for area in 0..28u8 {
        let db = fdat.get(area as usize * 3 + 1)?;
        if u32_at(db, 0)? != 12992 {
            return Err("Unexpected SLUS-00255 entity definitions");
        }
        span(db, 4, 32 * 120)?;
        for index in 0..32 {
            if let Some(first) = areas.get_mut(db[4 + index * 120] as usize) {
                if first.is_none() {
                    *first = Some(area);
                }
            }
        }
    }
    let mut output = b"KFA1\x01\0\0\0".to_vec();
    let mut count = 0u16;
    for slot in 0..43 {
        let at = *mo.offsets.get(slot).ok_or("Missing NPC model slot")?;
        if slot != 0 && mo.offsets[slot - 1] == at {
            continue;
        }
        // Slot 35 is a dummy. Slot 18 needs an unresolved runtime palette;
        // reject failures in other characters instead of silently dropping them.
        if slot == 35 || slot == 18 {
            continue;
        }
        let end = (slot + 1..mo.offsets.len())
            .find(|&i| mo.offsets[i] != at)
            .ok_or("Empty NPC model slot")?;
        let area = areas[slot..end.min(43)]
            .iter()
            .flatten()
            .min()
            .copied()
            .ok_or("NPC has no map reference")?;
        let data = textures.get(area as usize)?;
        let mut start = 0;
        if area < 12 {
            start = rtim(data, None, false)?;
            if span(data, start, 64)? != [255; 64] {
                return Err("Missing map texture separator");
            }
            start += 64;
        }
        let mut memory = common.clone();
        rtim(&data[start..], Some(&mut memory), true)?;
        let entry = mesh(&model(mo.get(slot)?)?, &memory, slot as u16)?;
        if output.len() + entry.len() > super::KF_AVATAR_MAX_BYTES as usize {
            return Err("Character pack exceeds limits");
        }
        output.extend_from_slice(&entry);
        count += 1;
    }
    if count != 38 {
        return Err("Unexpected KFIII character roster");
    }
    // Orladin is a map object: area 17, local model 1 (MOF 545), rather
    // than an MO NPC. Append its ID without renumbering existing selections.
    let mut memory = common;
    rtim(textures.get(17)?, Some(&mut memory), true)?;
    let entry = mesh(&model(mof.get(545)?)?, &memory, 43)?;
    if output.len() + entry.len() > super::KF_AVATAR_MAX_BYTES as usize {
        return Err("Character pack exceeds limits");
    }
    output.extend_from_slice(&entry);
    count += 1;
    output[6..8].copy_from_slice(&count.to_le_bytes());
    Ok(output)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn model_bounds_indices_and_conversion() {
        let mut b = Vec::new();
        for v in [100u32, 0, 12, 0x41, 0, 1, 28, 3, 52, 1, 60, 1, 0] {
            b.extend_from_slice(&v.to_le_bytes());
        }
        for v in [0i16, 0, 0, 0, 100, 0, 0, 0, 0, 100, 0, 0, 0, 0, 4096, 0] {
            b.extend_from_slice(&v.to_le_bytes());
        }
        b.extend([4, 3, 0, 0x20, 128, 64, 32, 0]);
        for v in [0u16, 0, 1, 2] {
            b.extend_from_slice(&v.to_le_bytes());
        }
        let objects = model(&b).unwrap();
        let converted = mesh(&objects, &Memory::new(), 41).unwrap();
        assert_eq!(converted.len(), 8 + 60 + 1024);
        assert_eq!(&converted[..8], &[41, 0, 1, 0, 1, 0, 0, 0]);
        for end in 0..b.len() {
            assert!(model(&b[..end]).is_err(), "length {end}");
        }
        for at in [24, 28, 32, 36, 40, 44, 48] {
            let mut bad = b.clone();
            bad[at..at + 4].copy_from_slice(&u32::MAX.to_le_bytes());
            assert!(model(&bad).is_err(), "field {at}");
        }
        let mut bad = b.clone();
        bad[98] = 3;
        assert!(model(&bad).is_err());
        let mut bad = b.clone();
        bad[81] = 127;
        assert!(model(&bad).is_err());
        let mut bad = b.clone();
        bad[86] = 8;
        assert!(model(&bad).is_err());
    }
    #[test]
    fn aliases_texture_extents_and_rounding() {
        let mut bytes = vec![0; 6144];
        for (i, v) in [4u16, 1, 1, 2, 3, 3].iter().enumerate() {
            bytes[i * 2..i * 2 + 2].copy_from_slice(&v.to_le_bytes());
        }
        bytes[2048] = 17;
        bytes[4096] = 29;
        let archive = Archive::new(&bytes).unwrap();
        assert_eq!(archive.get(0).unwrap()[0], 17);
        assert_eq!(archive.get(1).unwrap()[0], 17);
        assert_eq!(archive.get(2).unwrap()[0], 29);
        assert!(archive.get(3).is_err());
        assert!(archive.get(usize::MAX).is_err());
        bytes[4] = 0;
        assert!(Archive::new(&bytes).is_err());
        for (v, h, result) in [
            (1, 4000, 0),
            (3, 4000, 2),
            (-1, 4000, 0),
            (-3, 4000, -2),
            (1, 100, 20),
        ] {
            assert_eq!(scaled(v, h).unwrap(), result);
        }
        assert!(scaled(i32::MAX, 1).is_err());
        let mut rect = Vec::new();
        for v in [0u16, 0, 1, 1, 0, 0, 1, 1, 31] {
            rect.extend_from_slice(&v.to_le_bytes());
        }
        let mut pair = rect.clone();
        pair.extend_from_slice(&rect);
        let mut memory = Memory::new();
        assert_eq!(rtim(&pair, Some(&mut memory), false).unwrap(), 36);
        assert_eq!(memory.pixel(256, 0, 0, 0).unwrap(), [255, 0, 0, 255]);
        assert!(memory.pixel(256, 0, 1, 0).is_err());
        for end in 1..pair.len() {
            assert!(rtim(&pair[..end], None, false).is_err());
        }
    }
}
