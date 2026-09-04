//! Memory-card save-file views and transformations used by retail `GAME.EXE`.

pub const SAVE_HEADER_SIZE: usize = 0x280;
pub const PLAYSTATION_HEADER_SIZE: usize = 0x200;
pub const SAVE_DIRECTORY_SIZE: usize = 0x80;
pub const SAVE_PAYLOAD_SIZE: usize = 0x2580;
pub const PLAYER_STATE_SIZE: usize = 0xe0;
pub const WORLD_STATE_OFFSET: usize = 0x30c;
pub const WORLD_STATE_SIZE: usize = 8_500;
pub const AUXILIARY_STATE_OFFSET: usize = 0x2440;
pub const AUXILIARY_STATE_SIZE: usize = 240;
pub const MAGIC_FLAGS_OFFSET: usize = 0x2530;
pub const MAGIC_FLAG_COUNT: usize = 24;
pub const PAYLOAD_OPAQUE_TAIL_OFFSET: usize = 0x2548;
pub const PAYLOAD_OPAQUE_TAIL_SIZE: usize = 56;
pub const MAGIC_RECORD_SIZE: usize = 20;
pub const MAGIC_RECORDS_SIZE: usize = MAGIC_FLAG_COUNT * MAGIC_RECORD_SIZE;
pub const SLOT_COUNT: usize = 4;
pub const SLOT_SUMMARY_SIZE: usize = 24;
pub const SLOT_IDS_OFFSET: usize = 0x200;
pub const SLOT_SUMMARIES_OFFSET: usize = 0x220;
pub const ICON_SOURCE_SIZE: usize = 0x800;
pub const READ_ATTEMPTS: usize = 5;

const PRESERVED_PLAYER_RANGES: [(usize, usize); 2] = [(0x6c, 4), (0x74, 4)];
const SAVE_TITLE: [u8; 52] = [
    0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x83, 0x81, 0x83,
    0x81, 0x40, 0x81, 0x40, 0x82, 0x6a, 0x82, 0x68, 0x82, 0x6d, 0x82, 0x66, 0x81, 0x66, 0x82, 0x72,
    0x81, 0x40, 0x82, 0x65, 0x82, 0x68, 0x82, 0x64, 0x82, 0x6b, 0x82, 0x63, 0x81, 0x40, 0x81, 0x40,
    0x81, 0x84, 0x81, 0x84,
];

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SaveError {
    BufferTooSmall {
        buffer: SaveBuffer,
        needed: usize,
        available: usize,
    },
    InvalidSlotId {
        directory_index: usize,
        slot_id: u8,
    },
    NoWritableDirectoryEntry,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SaveBuffer {
    Header,
    Payload,
    Catalog,
    PlayerState,
    WorldState,
    AuxiliaryState,
    MagicRecords,
}

#[derive(Clone, Copy, Debug)]
pub struct SaveHeader<'a> {
    bytes: &'a [u8],
}

impl<'a> SaveHeader<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, SaveError> {
        require(bytes, SAVE_HEADER_SIZE, SaveBuffer::Header)?;
        Ok(Self { bytes })
    }

    pub fn playstation_header(&self) -> &'a [u8] {
        &self.bytes[..PLAYSTATION_HEADER_SIZE]
    }

    pub fn directory(&self) -> &'a [u8] {
        &self.bytes[PLAYSTATION_HEADER_SIZE..SAVE_HEADER_SIZE]
    }

    pub fn slot_id(&self, index: usize) -> Option<u8> {
        (index < SLOT_COUNT).then(|| self.bytes[SLOT_IDS_OFFSET + index])
    }

    pub fn summary(&self, index: usize) -> Option<&'a [u8]> {
        if index >= SLOT_COUNT {
            return None;
        }
        let at = SLOT_SUMMARIES_OFFSET + index * SLOT_SUMMARY_SIZE;
        Some(&self.bytes[at..at + SLOT_SUMMARY_SIZE])
    }
}

#[derive(Clone, Copy, Debug)]
pub struct SavePayload<'a> {
    bytes: &'a [u8],
}

impl<'a> SavePayload<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, SaveError> {
        require(bytes, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
        Ok(Self { bytes })
    }

    pub fn player_state(&self) -> &'a [u8] {
        &self.bytes[..PLAYER_STATE_SIZE]
    }

    pub fn unresolved_0e0(&self) -> &'a [u8] {
        &self.bytes[PLAYER_STATE_SIZE..WORLD_STATE_OFFSET]
    }

    pub fn world_state(&self) -> &'a [u8] {
        &self.bytes[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE]
    }

    pub fn auxiliary_state(&self) -> &'a [u8] {
        &self.bytes[AUXILIARY_STATE_OFFSET..AUXILIARY_STATE_OFFSET + AUXILIARY_STATE_SIZE]
    }

    pub fn magic_flags(&self) -> &'a [u8] {
        &self.bytes[MAGIC_FLAGS_OFFSET..MAGIC_FLAGS_OFFSET + MAGIC_FLAG_COUNT]
    }

    pub fn opaque_tail(&self) -> &'a [u8] {
        &self.bytes[PAYLOAD_OPAQUE_TAIL_OFFSET..SAVE_PAYLOAD_SIZE]
    }
}

pub struct SaveRuntimeDestinations<'a> {
    pub player_state: &'a mut [u8],
    pub world_state: &'a mut [u8],
    pub auxiliary_state: &'a mut [u8],
    pub magic_records: &'a mut [u8],
}

pub struct SaveRuntimeSources<'a> {
    pub player_state: &'a [u8],
    pub world_state: &'a [u8],
    pub auxiliary_state: &'a [u8],
    pub magic_records: &'a [u8],
}

/// Restores the proved runtime ranges. The live player pointers/word at 0x6c
/// and 0x74 survive the 224-byte load, matching the retail save reader.
pub fn restore_payload(
    payload: &[u8],
    destinations: SaveRuntimeDestinations<'_>,
) -> Result<(), SaveError> {
    require(payload, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
    validate_runtime_destinations(&destinations)?;
    let mut preserved = [[0; 4]; 2];
    for (index, (at, size)) in PRESERVED_PLAYER_RANGES.iter().copied().enumerate() {
        debug_assert_eq!(size, 4);
        preserved[index].copy_from_slice(&destinations.player_state[at..at + size]);
    }
    destinations.player_state[..PLAYER_STATE_SIZE].copy_from_slice(&payload[..PLAYER_STATE_SIZE]);
    for (index, (at, size)) in PRESERVED_PLAYER_RANGES.iter().copied().enumerate() {
        destinations.player_state[at..at + size].copy_from_slice(&preserved[index]);
    }
    destinations.world_state[..WORLD_STATE_SIZE]
        .copy_from_slice(&payload[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE]);
    destinations.auxiliary_state[..AUXILIARY_STATE_SIZE].copy_from_slice(
        &payload[AUXILIARY_STATE_OFFSET..AUXILIARY_STATE_OFFSET + AUXILIARY_STATE_SIZE],
    );
    for index in 0..MAGIC_FLAG_COUNT {
        destinations.magic_records[index * MAGIC_RECORD_SIZE] = payload[MAGIC_FLAGS_OFFSET + index];
    }
    Ok(())
}

/// Serializes exactly the ranges written by `save_file_write_slot`; unresolved
/// payload bytes from 0xe0..0x30c and 0x2548..0x2580 remain caller-initialized.
pub fn serialize_payload(
    sources: SaveRuntimeSources<'_>,
    payload: &mut [u8],
) -> Result<(), SaveError> {
    require(payload, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
    validate_runtime_sources(&sources)?;
    payload[..PLAYER_STATE_SIZE].copy_from_slice(&sources.player_state[..PLAYER_STATE_SIZE]);
    payload[WORLD_STATE_OFFSET..WORLD_STATE_OFFSET + WORLD_STATE_SIZE]
        .copy_from_slice(&sources.world_state[..WORLD_STATE_SIZE]);
    payload[AUXILIARY_STATE_OFFSET..AUXILIARY_STATE_OFFSET + AUXILIARY_STATE_SIZE]
        .copy_from_slice(&sources.auxiliary_state[..AUXILIARY_STATE_SIZE]);
    for index in 0..MAGIC_FLAG_COUNT {
        payload[MAGIC_FLAGS_OFFSET + index] = sources.magic_records[index * MAGIC_RECORD_SIZE];
    }
    Ok(())
}

/// Pure `save_system_read_catalog` transformation. Retail clears only the
/// first 0x24 output bytes before optionally distributing active summaries.
pub fn apply_catalog(
    header_status: i32,
    header: &[u8],
    summaries: &mut [u8],
) -> Result<i32, SaveError> {
    require(summaries, SLOT_SUMMARY_SIZE * 3, SaveBuffer::Catalog)?;
    summaries[..0x24].fill(0);
    if header_status != 1 {
        return Ok(header_status);
    }
    let header = SaveHeader::parse(header)?;
    for directory_index in 0..SLOT_COUNT {
        let slot_id = header.slot_id(directory_index).unwrap();
        if slot_id == 0 || slot_id == 4 {
            continue;
        }
        if !(1..=3).contains(&slot_id) {
            return Err(SaveError::InvalidSlotId {
                directory_index,
                slot_id,
            });
        }
        let output = usize::from(slot_id - 1) * SLOT_SUMMARY_SIZE;
        summaries[output..output + SLOT_SUMMARY_SIZE]
            .copy_from_slice(header.summary(directory_index).unwrap());
    }
    Ok(header_status)
}

pub trait SaveReadIo {
    fn clear_events(&mut self);
    fn open_read(&mut self) -> bool;
    fn seek(&mut self, offset: usize);
    /// Returns the provider read count; negative values are failures.
    fn read(&mut self, destination: &mut [u8]) -> i32;
    fn close(&mut self);
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum SaveFile {
    Main,
    Temporary,
}

/// BIOS-shaped services for the slot writer. Implementations may record
/// requests without accessing a real memory card or filesystem.
pub trait SaveWriteIo: IconSource {
    fn clear_events(&mut self);
    fn open(&mut self, file: SaveFile, flags: u32) -> i32;
    fn seek(&mut self, file: i32, offset: usize);
    fn write(&mut self, file: i32, bytes: &[u8]) -> i32;
    fn close(&mut self, file: i32);
    fn erase_temporary(&mut self);
}

/// Serialize and rotate a save directory entry with retail's two-stage write.
/// The payload is committed before changing the in-memory header. Header-write
/// failure does not roll back that change. A missing previous entry writes the
/// retired marker at header byte 0x1ff, as the retail signed -1 index does.
pub fn write_slot<C: SaveWriteIo>(
    slot_id: i16,
    header: &mut [u8],
    payload: &mut [u8],
    sources: SaveRuntimeSources<'_>,
    context: &mut C,
) -> Result<i32, SaveError> {
    require(header, SAVE_HEADER_SIZE, SaveBuffer::Header)?;
    require(payload, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
    validate_runtime_sources(&sources)?;
    context.clear_events();
    let file = context.open(SaveFile::Main, 1);
    context.close(file);
    if file == -1 {
        context.clear_events();
        let file = context.open(SaveFile::Main, 0x50200);
        context.close(file);
        if file == -1 {
            let file = context.open(SaveFile::Temporary, 0x200);
            context.close(file);
            context.erase_temporary();
            return Ok(if file == -1 { 3 } else { 5 });
        }
        initialize_buffers(header, payload, context)?;
    }
    let ids = &header[SLOT_IDS_OFFSET..SLOT_IDS_OFFSET + SLOT_COUNT];
    let entry = ids
        .iter()
        .position(|&id| id == 4)
        .or_else(|| ids.iter().position(|&id| id == 0))
        .ok_or(SaveError::NoWritableDirectoryEntry)?;
    let previous = ids.iter().position(|&id| i16::from(id) == slot_id);
    let player = sources.player_state;
    serialize_payload(sources, payload)?;
    context.clear_events();
    let file = context.open(SaveFile::Main, 2);
    if file == -1 {
        return Ok(14);
    }
    let written = write_retried(
        context,
        file,
        SAVE_HEADER_SIZE + SAVE_PAYLOAD_SIZE * entry,
        &payload[..SAVE_PAYLOAD_SIZE],
    );
    context.close(file);
    if !written {
        return Ok(14);
    }
    header[SLOT_IDS_OFFSET + entry] = slot_id as u8;
    header[previous.map_or(SLOT_IDS_OFFSET - 1, |index| SLOT_IDS_OFFSET + index)] = 4;
    let halfword = |at| u32::from(u16::from_le_bytes([player[at], player[at + 1]]));
    let summary = [
        u32::from_le_bytes(player[..4].try_into().unwrap()),
        u32::from(player[0x0a]),
        halfword(0x12),
        halfword(0x10),
        halfword(0x16),
        halfword(0x14),
    ];
    let at = SLOT_SUMMARIES_OFFSET + entry * SLOT_SUMMARY_SIZE;
    for (destination, value) in header[at..at + SLOT_SUMMARY_SIZE]
        .chunks_exact_mut(4)
        .zip(summary)
    {
        destination.copy_from_slice(&value.to_le_bytes());
    }
    context.clear_events();
    let file = context.open(SaveFile::Main, 2);
    if file == -1 {
        return Ok(14);
    }
    let written = write_retried(context, file, 0, &header[..SAVE_HEADER_SIZE]);
    context.close(file);
    Ok(if written { 1 } else { 14 })
}

fn write_retried<C: SaveWriteIo>(context: &mut C, file: i32, offset: usize, bytes: &[u8]) -> bool {
    for _ in 0..READ_ATTEMPTS {
        context.clear_events();
        context.seek(file, offset);
        context.clear_events();
        if context.write(file, bytes) == bytes.len() as i32 {
            return true;
        }
    }
    false
}

/// `save_file_read_header`: clear the full output before opening, then retry a
/// 0x280-byte read at offset zero at most five times. Returns 1, 7, or 13.
pub fn read_header<C: SaveReadIo>(
    destination: &mut [u8],
    context: &mut C,
) -> Result<i32, SaveError> {
    require(destination, SAVE_HEADER_SIZE, SaveBuffer::Header)?;
    destination[..SAVE_HEADER_SIZE].fill(0);
    context.clear_events();
    if !context.open_read() {
        return Ok(7);
    }
    let success = read_retried(context, 0, &mut destination[..SAVE_HEADER_SIZE]);
    context.close();
    Ok(if success { 1 } else { 13 })
}

/// `save_file_read_slot`, including cached-directory selection, five header
/// attempts, summary-only validation (mismatch is status 12), five payload
/// attempts, and the post-read runtime restoration.
pub fn read_slot<C: SaveReadIo>(
    slot_id: i16,
    cached_header: &[u8],
    payload_buffer: &mut [u8],
    destinations: SaveRuntimeDestinations<'_>,
    context: &mut C,
) -> Result<i32, SaveError> {
    let cached = SaveHeader::parse(cached_header)?;
    require(payload_buffer, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
    validate_runtime_destinations(&destinations)?;
    let mut entry = None;
    for index in 0..SLOT_COUNT {
        if i16::from(cached.slot_id(index).unwrap()) == slot_id {
            entry = Some(index);
            break;
        }
    }
    let Some(entry) = entry else { return Ok(7) };
    context.clear_events();
    if !context.open_read() {
        return Ok(7);
    }
    let mut fresh_header = [0; SAVE_HEADER_SIZE];
    if !read_retried(context, 0, &mut fresh_header) {
        context.close();
        return Ok(13);
    }
    if cached_header[SLOT_SUMMARIES_OFFSET..SAVE_HEADER_SIZE]
        != fresh_header[SLOT_SUMMARIES_OFFSET..SAVE_HEADER_SIZE]
    {
        context.close();
        return Ok(12);
    }
    let offset = SAVE_HEADER_SIZE + SAVE_PAYLOAD_SIZE * entry;
    if !read_retried(context, offset, &mut payload_buffer[..SAVE_PAYLOAD_SIZE]) {
        context.close();
        return Ok(13);
    }
    context.close();
    restore_payload(payload_buffer, destinations)?;
    Ok(1)
}

fn read_retried<C: SaveReadIo>(context: &mut C, offset: usize, destination: &mut [u8]) -> bool {
    for _ in 0..READ_ATTEMPTS {
        context.clear_events();
        context.seek(offset);
        context.clear_events();
        if context.read(destination) == destination.len() as i32 {
            return true;
        }
    }
    false
}

pub trait IconSource {
    /// Models the three `cd_file_load_into` calls into the same stack image.
    fn load_icon(&mut self, path: &[u8], image: &mut [u8; ICON_SOURCE_SIZE]);
}

/// Builds the 0x280-byte header and clears the 0x2580-byte payload. ICO1
/// contributes its CLUT and first frame; ICO2/ICO3 contribute one frame each.
pub fn initialize_buffers<C: IconSource>(
    header: &mut [u8],
    payload: &mut [u8],
    context: &mut C,
) -> Result<(), SaveError> {
    require(header, SAVE_HEADER_SIZE, SaveBuffer::Header)?;
    require(payload, SAVE_PAYLOAD_SIZE, SaveBuffer::Payload)?;
    header[..SAVE_HEADER_SIZE].fill(0);
    header[0] = b'S';
    header[1] = b'C';
    header[2] = 0x13;
    header[3] = 5;
    header[4..4 + SAVE_TITLE.len()].copy_from_slice(&SAVE_TITLE);
    let mut image = [0; ICON_SOURCE_SIZE];
    context.load_icon(b"TIM\\ICO1.TIM", &mut image);
    header[0x60..0x80].copy_from_slice(&image[0x14..0x34]);
    header[0x80..0x100].copy_from_slice(&image[0x40..0xc0]);
    context.load_icon(b"TIM\\ICO2.TIM", &mut image);
    header[0x100..0x180].copy_from_slice(&image[0x40..0xc0]);
    context.load_icon(b"TIM\\ICO3.TIM", &mut image);
    header[0x180..0x200].copy_from_slice(&image[0x40..0xc0]);
    payload[..SAVE_PAYLOAD_SIZE].fill(0);
    Ok(())
}

fn validate_runtime_destinations(
    destinations: &SaveRuntimeDestinations<'_>,
) -> Result<(), SaveError> {
    require(
        destinations.player_state,
        PLAYER_STATE_SIZE,
        SaveBuffer::PlayerState,
    )?;
    require(
        destinations.world_state,
        WORLD_STATE_SIZE,
        SaveBuffer::WorldState,
    )?;
    require(
        destinations.auxiliary_state,
        AUXILIARY_STATE_SIZE,
        SaveBuffer::AuxiliaryState,
    )?;
    require(
        destinations.magic_records,
        MAGIC_RECORDS_SIZE,
        SaveBuffer::MagicRecords,
    )
}

fn validate_runtime_sources(sources: &SaveRuntimeSources<'_>) -> Result<(), SaveError> {
    require(
        sources.player_state,
        PLAYER_STATE_SIZE,
        SaveBuffer::PlayerState,
    )?;
    require(
        sources.world_state,
        WORLD_STATE_SIZE,
        SaveBuffer::WorldState,
    )?;
    require(
        sources.auxiliary_state,
        AUXILIARY_STATE_SIZE,
        SaveBuffer::AuxiliaryState,
    )?;
    require(
        sources.magic_records,
        MAGIC_RECORDS_SIZE,
        SaveBuffer::MagicRecords,
    )
}

fn require(bytes: &[u8], needed: usize, buffer: SaveBuffer) -> Result<(), SaveError> {
    if bytes.len() < needed {
        Err(SaveError::BufferTooSmall {
            buffer,
            needed,
            available: bytes.len(),
        })
    } else {
        Ok(())
    }
}
