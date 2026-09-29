#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct Vec3s {
    pub x: i16,
    pub y: i16,
    pub z: i16,
}

impl Vec3s {
    pub const BYTE_SIZE: usize = 6;

    pub const fn from_le_bytes(bytes: [u8; Self::BYTE_SIZE]) -> Self {
        Self {
            x: i16::from_le_bytes([bytes[0], bytes[1]]),
            y: i16::from_le_bytes([bytes[2], bytes[3]]),
            z: i16::from_le_bytes([bytes[4], bytes[5]]),
        }
    }

    pub const fn to_le_bytes(self) -> [u8; Self::BYTE_SIZE] {
        let x = self.x.to_le_bytes();
        let y = self.y.to_le_bytes();
        let z = self.z.to_le_bytes();
        [x[0], x[1], y[0], y[1], z[0], z[1]]
    }
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct Vec3i {
    pub x: i32,
    pub y: i32,
    pub z: i32,
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct EulerAngles {
    pub x: i16,
    pub y: i16,
    pub z: i16,
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct SVector {
    pub vx: i16,
    pub vy: i16,
    pub vz: i16,
    pub pad: i16,
}

impl SVector {
    pub const BYTE_SIZE: usize = 8;

    pub const fn from_le_bytes(bytes: [u8; Self::BYTE_SIZE]) -> Self {
        Self {
            vx: i16::from_le_bytes([bytes[0], bytes[1]]),
            vy: i16::from_le_bytes([bytes[2], bytes[3]]),
            vz: i16::from_le_bytes([bytes[4], bytes[5]]),
            pad: i16::from_le_bytes([bytes[6], bytes[7]]),
        }
    }

    pub const fn to_le_bytes(self) -> [u8; Self::BYTE_SIZE] {
        let x = self.vx.to_le_bytes();
        let y = self.vy.to_le_bytes();
        let z = self.vz.to_le_bytes();
        let pad = self.pad.to_le_bytes();
        [x[0], x[1], y[0], y[1], z[0], z[1], pad[0], pad[1]]
    }
}
