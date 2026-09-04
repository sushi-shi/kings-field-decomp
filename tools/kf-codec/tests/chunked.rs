use kf_codec::chunked::{self, ChunkError, Cursor};

#[test]
fn reads_the_retail_length_prefix_shape() {
    let bytes = [3, 0, 0, 0, 0xaa, 0xbb, 0xcc, 2, 0, 0, 0, 0x11, 0x22];
    let chunks: Vec<_> = chunked::chunks(&bytes)
        .map(|result| result.unwrap())
        .collect();

    assert_eq!(chunks.len(), 2);
    assert_eq!(chunks[0].header_offset, 0);
    assert_eq!(chunks[0].payload, [0xaa, 0xbb, 0xcc]);
    assert_eq!(chunks[1].header_offset, 7);
    assert_eq!(chunks[1].payload, [0x11, 0x22]);
}

#[test]
fn cursor_leaves_padding_for_the_enclosing_schema() {
    let bytes = [1, 0, 0, 0, 0xaa, 0xff, 0xff];
    let mut cursor = Cursor::new(&bytes);

    assert_eq!(cursor.next_chunk().unwrap().unwrap().payload, [0xaa]);
    assert_eq!(cursor.position(), 5);
    assert_eq!(cursor.remainder(), [0xff, 0xff]);
}

#[test]
fn truncated_payload_reports_header_and_lengths() {
    let mut cursor = Cursor::new(&[4, 0, 0, 0, 1, 2]);
    assert_eq!(
        cursor.next_chunk(),
        Err(ChunkError::TruncatedPayload {
            at: 0,
            declared: 4,
            available: 2,
        })
    );
}

#[test]
fn roundtrip_is_byte_exact() {
    let chunks: [&[u8]; 3] = [b"VAB", b"", b"TMD payload"];
    let mut encoded = vec![0; chunked::encoded_len(&chunks).unwrap()];
    let written = chunked::encode_into(&chunks, &mut encoded).unwrap();
    assert_eq!(written, encoded.len());

    let decoded: Vec<&[u8]> = chunked::chunks(&encoded)
        .map(|chunk| chunk.unwrap().payload)
        .collect();
    assert_eq!(decoded, chunks);
}

#[test]
fn writer_reports_the_required_size() {
    let chunks: [&[u8]; 1] = [b"abc"];
    let mut output = [0u8; 6];
    assert_eq!(
        chunked::encode_into(&chunks, &mut output),
        Err(ChunkError::OutputFull { need: 7, have: 6 })
    );
}
