//! Bounded, independent decoding of locally derived KFIII presentation meshes.
use crate::ffi::bindings::{KfAvatarVertex, KF_AVATAR_MAX_BYTES, KF_AVATAR_SLOTS};
pub mod disc;
mod source;

pub struct Mesh {
    pub slot: u16,
    pub height: u16,
    pub vertices: Vec<KfAvatarVertex>,
    pub rgba: Vec<u8>,
}
pub struct Pack(pub Vec<Mesh>);
struct Reader<'a>(&'a [u8]);
impl<'a> Reader<'a> {
    fn take(&mut self, n: usize) -> Option<&'a [u8]> {
        let result = self.0.get(..n)?;
        self.0 = &self.0[n..];
        Some(result)
    }
    fn u16(&mut self) -> Option<u16> {
        Some(u16::from_le_bytes(self.take(2)?.try_into().ok()?))
    }
    fn u32(&mut self) -> Option<u32> {
        Some(u32::from_le_bytes(self.take(4)?.try_into().ok()?))
    }
}
pub fn decode(bytes: &[u8]) -> Option<Pack> {
    if bytes.len() > KF_AVATAR_MAX_BYTES as usize {
        return None;
    }
    let mut r = Reader(bytes);
    if r.take(4)? != b"KFA1" || r.u16()? != 1 {
        return None;
    }
    let count = r.u16()?;
    if count == 0 || count > KF_AVATAR_SLOTS as u16 {
        return None;
    }
    let mut meshes = Vec::new();
    let mut seen = [false; KF_AVATAR_SLOTS as usize];
    for _ in 0..count {
        let slot = r.u16()?;
        let height = r.u16()?;
        let triangles = r.u32()? as usize;
        if slot as usize >= seen.len()
            || seen[slot as usize]
            || !(1..=4096).contains(&height)
            || !(1..=4096).contains(&triangles)
        {
            return None;
        }
        seen[slot as usize] = true;
        let mut source = Reader(r.take(triangles * 60)?);
        let mut vertices = Vec::with_capacity(triangles * 3);
        for _ in 0..triangles * 3 {
            let (x, y, z) = (
                source.u16()? as i16,
                source.u16()? as i16,
                source.u16()? as i16,
            );
            let (nx, ny, nz) = (
                source.u16()? as i16,
                source.u16()? as i16,
                source.u16()? as i16,
            );
            let (u, v) = (source.u16()?, source.u16()?);
            let color = source.take(4)?;
            if [x, y, z].iter().any(|&p| !(-8192..=8192).contains(&p))
                || [nx, ny, nz].iter().any(|&n| !(-4096..=4096).contains(&n))
                || u >= 256
                || v >= height
                || color[3] > 1
            {
                return None;
            }
            vertices.push(KfAvatarVertex {
                x,
                y,
                z,
                nx,
                ny,
                nz,
                u,
                v,
                r: color[0],
                g: color[1],
                b: color[2],
                unlit: color[3],
            });
        }
        let rgba = r.take(256 * height as usize * 4)?.to_vec();
        meshes.push(Mesh {
            slot,
            height,
            vertices,
            rgba,
        });
    }
    if !r.0.is_empty() {
        return None;
    }
    Some(Pack(meshes))
}

#[cfg(test)]
mod tests {
    use super::*;
    fn pack() -> Vec<u8> {
        let mut b = b"KFA1\x01\0\x01\0\x29\0\x01\0\x01\0\0\0".to_vec();
        b.extend([0; 60]);
        b.extend([255; 1024]);
        b
    }
    #[test]
    fn strict_extents_and_fields() {
        let b = pack();
        let p = decode(&b).unwrap();
        assert_eq!(p.0[0].slot, 41);
        assert_eq!(p.0[0].vertices.len(), 3);
        for end in 0..b.len() {
            assert!(decode(&b[..end]).is_none());
        }
        for (at, value) in [
            (4, 2),
            (6, 0),
            (8, KF_AVATAR_SLOTS as u8),
            (10, 0),
            (12, 0),
            (17, 127),
            (23, 127),
            (29, 1),
            (30, 1),
            (35, 2),
        ] {
            let mut bad = b.clone();
            bad[at] = value;
            assert!(decode(&bad).is_none(), "offset {at}");
        }
        let mut extra = b.clone();
        extra.push(0);
        assert!(decode(&extra).is_none());
        let mut duplicate = b.clone();
        duplicate[6] = 2;
        duplicate.extend_from_slice(&b[8..]);
        assert!(decode(&duplicate).is_none());
    }
}
