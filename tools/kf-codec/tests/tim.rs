use kf_codec::tim::{Images, TimError};

#[test]
fn reads_clut_before_pixels_and_stops_at_terminator() {
    let mut data = Vec::new();
    for word in [
        0x10u32, 8, 16, 0x00010002, 0x00010002, 0x11223344, 16, 0x00040003, 0x00020001, 0x55667788,
        0,
    ] {
        data.extend_from_slice(&word.to_le_bytes());
    }
    let mut images = Images::new(&data);
    let image = images.next().unwrap().unwrap();
    assert_eq!(image.clut.unwrap().pixels, [0x44, 0x33, 0x22, 0x11]);
    assert_eq!(image.image.rectangle.x, 3);
    assert_eq!(image.image.rectangle.height, 2);
    assert_eq!(image.image.pixels, [0x88, 0x77, 0x66, 0x55]);
    let descriptor = image.psx_descriptor(0x80060000);
    assert_eq!(descriptor.mode, 8);
    assert_eq!(descriptor.clut_rectangle, 0x8006000c);
    assert_eq!(descriptor.clut_data, 0x80060014);
    assert_eq!(descriptor.image_rectangle, 0x8006001c);
    assert_eq!(descriptor.image_data, 0x80060024);
    let words: Vec<_> = descriptor
        .to_le_bytes()
        .chunks_exact(4)
        .map(|word| u32::from_le_bytes(word.try_into().unwrap()))
        .collect();
    assert_eq!(words, [8, 0x8006000c, 0x80060014, 0x8006001c, 0x80060024]);
    assert_eq!(images.position(), 40);
    assert!(images.next().is_none());
    assert_eq!(images.remainder(), [0, 0, 0, 0]);
}

#[test]
fn no_clut_descriptor_clears_both_pointers_and_keeps_mode_bits() {
    let bytes: Vec<_> = [0x10u32, 0x80000002, 15, 0, 0]
        .into_iter()
        .flat_map(u32::to_le_bytes)
        .collect();
    let mut images = Images::new(&bytes);
    let image = images.next().unwrap().unwrap();
    let descriptor = image.psx_descriptor(0xfffffff0);
    assert_eq!(descriptor.mode, 0x80000002);
    assert_eq!((descriptor.clut_rectangle, descriptor.clut_data), (0, 0));
    assert_eq!(
        (descriptor.image_rectangle, descriptor.image_data),
        (0xfffffffc, 4)
    );
    assert_eq!(image.encoded_len, 20);
    assert_eq!(images.position(), 20);
}

#[test]
fn descriptor_offsets_are_relative_to_the_complete_stream() {
    let bytes: Vec<_> = [0x10u32, 2, 12, 0, 0]
        .into_iter()
        .cycle()
        .take(10)
        .flat_map(u32::to_le_bytes)
        .collect();
    let second = Images::new(&bytes).nth(1).unwrap().unwrap();
    let descriptor = second.psx_descriptor(0x80060000);
    assert_eq!(second.offset, 20);
    assert_eq!(descriptor.image_rectangle, 0x80060020);
    assert_eq!(descriptor.image_data, 0x80060028);
}

#[test]
fn rectangle_cannot_read_past_its_declared_block() {
    let mut data = Vec::new();
    for word in [0x10u32, 2, 12, 0, 0x00010001] {
        data.extend_from_slice(&word.to_le_bytes());
    }
    assert!(matches!(
        Images::new(&data).next().unwrap(),
        Err(TimError::Truncated { .. })
    ));
}
