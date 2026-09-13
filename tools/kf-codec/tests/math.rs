use kf_codec::animation::Vertex;
use kf_codec::math::{SVector, Vec3s};

#[test]
fn shared_vectors_preserve_signed_components_and_sdk_padding() {
    let vector = Vec3s::from_le_bytes([0, 0x80, 0xff, 0x7f, 0xff, 0xff]);
    assert_eq!(
        vector,
        Vec3s {
            x: i16::MIN,
            y: i16::MAX,
            z: -1
        }
    );
    assert_eq!(vector.to_le_bytes(), [0, 0x80, 0xff, 0x7f, 0xff, 0xff]);

    let bytes = [0, 0x80, 0xff, 0x7f, 0xff, 0xff, 0xcd, 0xab];
    let sdk = SVector::from_le_bytes(bytes);
    assert_eq!(
        sdk,
        SVector {
            vx: i16::MIN,
            vy: i16::MAX,
            vz: -1,
            pad: -21555
        }
    );
    let animation_vertex: Vertex = sdk;
    assert_eq!(animation_vertex.to_le_bytes(), bytes);
}
