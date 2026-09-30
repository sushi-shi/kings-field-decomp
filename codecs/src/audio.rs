use crate::bytes;
use crate::cast::AsUsize;
use crate::formats::{SeqHeader, VabHeader, VabProgram, VabTone};

pub const VAB_MAGIC: [u8; 4] = *b"pBAV";
const VAB_HEADER_SIZE: usize = size_of::<VabHeader>();
pub const VAB_PROGRAM_SLOTS: u16 = 128;
const VAB_PROGRAM_SIZE: usize = size_of::<VabProgram>();
pub const VAB_TONES_PER_PROGRAM: u16 = 16;
const VAB_TONE_SIZE: usize = size_of::<VabTone>();
pub const VAB_OFFSET_ENTRIES: usize = 256;
pub const VAB_OFFSET_TABLE_SIZE: usize = VAB_OFFSET_ENTRIES * size_of::<u16>();
pub const VAB_SAMPLE_UNIT: u32 = 8;

// MIDI status and variable-length encodings used by the SEQ parser/bridge.
pub mod midi {
    pub const STATUS_BIT: u8 = 0x80;
    pub const CHANNEL_STATUS_FIRST: u8 = 0x80;
    pub const CHANNEL_STATUS_LAST: u8 = 0xef;
    pub const MESSAGE_SHIFT: u32 = 4;
    pub const CHANNEL_MASK: u8 = 15;
    pub const NOTE_OFF: u8 = 8;
    pub const NOTE_ON: u8 = 9;
    pub const POLYPHONIC_PRESSURE: u8 = 10;
    pub const CONTROL_CHANGE: u8 = 11;
    pub const PROGRAM_CHANGE: u8 = 12;
    pub const CHANNEL_PRESSURE: u8 = 13;
    pub const PITCH_BEND: u8 = 14;
    pub const META: u8 = 0xff;
    pub const SYSEX_START: u8 = 0xf0;
    pub const SYSEX_END: u8 = 0xf7;
    pub const TIME_CODE: u8 = 0xf1;
    pub const SONG_POSITION: u8 = 0xf2;
    pub const SONG_SELECT: u8 = 0xf3;
    pub const TUNE_REQUEST: u8 = 0xf6;
    pub const TIMING_CLOCK: u8 = 0xf8;
    pub const START: u8 = 0xfa;
    pub const CONTINUE: u8 = 0xfb;
    pub const STOP: u8 = 0xfc;
    pub const ACTIVE_SENSING: u8 = 0xfe;
    pub const DATA_BITS: usize = 7;
    pub const DATA_MASK: u8 = 127;
    pub const VLQ_BYTES_MAX: usize = 4;
    pub const VLQ_CONTINUATION: u8 = 0x80;
    pub const CHANNEL_VOLUME: u8 = 7;
    pub const META_TEMPO: u8 = 0x51;
    pub const META_END: u8 = 0x2f;
}

pub const SEQ_MAGIC: [u8; 4] = *b"pQES";
const SEQ_HEADER_SIZE: usize = size_of::<SeqHeader>();

impl VabHeader {
    pub fn parse(bytes: &[u8]) -> crate::Result<Self> {
        let record: Self = bytes::read(bytes, 0)?;
        if record.magic != VAB_MAGIC {
            return Err(crate::Error::invalid_magic("VAB", record.magic));
        }
        Ok(record)
    }

    pub fn encoded_header_len(self) -> crate::Result<usize> {
        validate_counts(self)?;
        Ok(VAB_HEADER_SIZE
            + VAB_PROGRAM_SLOTS.as_usize() * VAB_PROGRAM_SIZE
            + self.program_count.get().as_usize()
                * VAB_TONES_PER_PROGRAM.as_usize()
                * VAB_TONE_SIZE
            + VAB_OFFSET_TABLE_SIZE)
    }
}

#[derive(Debug, Clone, Copy)]
pub struct VabBank<'a> {
    pub header: VabHeader,
    header_bytes: &'a [u8],
    tone_table_offset: usize,
    offset_table_offset: usize,
}

impl<'a> VabBank<'a> {
    pub fn parse(header_bytes: &'a [u8], body_bytes: &'a [u8]) -> crate::Result<Self> {
        let header = VabHeader::parse(header_bytes)?;
        let expected_header = header.encoded_header_len()?;
        if header_bytes.len() != expected_header {
            return Err(crate::Error::size_mismatch(
                "VAB header",
                expected_header,
                header_bytes.len(),
            ));
        }
        let actual_file_size =
            header_bytes
                .len()
                .checked_add(body_bytes.len())
                .ok_or(crate::Error::size_mismatch(
                    "VAB file",
                    header.file_size.get().as_usize(),
                    usize::MAX,
                ))?;
        if header.file_size.get().as_usize() != actual_file_size {
            return Err(crate::Error::size_mismatch(
                "VAB file",
                header.file_size.get().as_usize(),
                actual_file_size,
            ));
        }
        let tone_table_offset = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS.as_usize() * VAB_PROGRAM_SIZE;
        let offset_table_offset = expected_header - VAB_OFFSET_TABLE_SIZE;
        let first = bytes::read_u16_le(header_bytes, offset_table_offset)?;
        if first != 0 {
            return Err(crate::Error::invalid_value(
                "VAB first sample offset (expected zero)",
                i64::from(first),
            ));
        }
        let mut body_len = 0u32;
        for sample in 0..header.sample_count.get() {
            let units = bytes::read_u16_le(
                header_bytes,
                offset_table_offset + (sample.as_usize() + 1) * 2,
            )?;
            let bytes = u32::from(units)
                .checked_mul(VAB_SAMPLE_UNIT)
                .ok_or(crate::Error::invalid("VAB sample length overflow"))?;
            body_len = body_len
                .checked_add(bytes)
                .ok_or(crate::Error::invalid("VAB sample length overflow"))?;
        }
        if body_len.as_usize() != body_bytes.len() {
            return Err(crate::Error::size_mismatch(
                "VAB body",
                body_len.as_usize(),
                body_bytes.len(),
            ));
        }
        Ok(Self {
            header,
            header_bytes,
            tone_table_offset,
            offset_table_offset,
        })
    }

    pub fn program_slot(&self, index: usize) -> crate::Result<VabProgram> {
        if index >= VAB_PROGRAM_SLOTS.as_usize() {
            return Err(crate::Error::invalid_index(
                index,
                VAB_PROGRAM_SLOTS.as_usize(),
            ));
        }
        let at = VAB_HEADER_SIZE + index * VAB_PROGRAM_SIZE;
        bytes::read(self.header_bytes, at)
    }

    pub fn tone(&self, program: usize, tone: usize) -> crate::Result<VabTone> {
        if program >= self.header.program_count.get().as_usize() {
            return Err(crate::Error::invalid_index(
                program,
                self.header.program_count.get().as_usize(),
            ));
        }
        if tone >= VAB_TONES_PER_PROGRAM.as_usize() {
            return Err(crate::Error::invalid_index(
                tone,
                VAB_TONES_PER_PROGRAM.as_usize(),
            ));
        }
        let at = self.tone_table_offset
            + (program * VAB_TONES_PER_PROGRAM.as_usize() + tone) * VAB_TONE_SIZE;
        bytes::read(self.header_bytes, at)
    }

    pub fn sample_size_units(&self, sample: usize) -> crate::Result<u16> {
        if sample >= self.header.sample_count.get().as_usize() {
            return Err(crate::Error::invalid_index(
                sample,
                self.header.sample_count.get().as_usize(),
            ));
        }
        bytes::read_u16_le(
            self.header_bytes,
            self.offset_table_offset + (sample + 1) * 2,
        )
    }

    pub fn samples(&self) -> VabSamples<'a> {
        VabSamples {
            bank: *self,
            index: 0,
            body_offset: 0,
        }
    }
}

fn validate_counts(header: VabHeader) -> crate::Result<()> {
    if header.program_count.get() > VAB_PROGRAM_SLOTS {
        return Err(crate::Error::invalid_count(
            "VAB programs",
            header.program_count.get(),
            VAB_PROGRAM_SLOTS,
        ));
    }
    let maximum_tones = header
        .program_count
        .get()
        .saturating_mul(VAB_TONES_PER_PROGRAM);
    if header.tone_count.get() > maximum_tones {
        return Err(crate::Error::invalid_count(
            "VAB tones",
            header.tone_count.get(),
            maximum_tones,
        ));
    }
    if header.sample_count.get().as_usize() >= VAB_OFFSET_ENTRIES {
        return Err(crate::Error::invalid_count(
            "VAB samples",
            header.sample_count.get(),
            u16::try_from(VAB_OFFSET_ENTRIES - 1)?,
        ));
    }
    Ok(())
}

#[derive(Debug, Clone, Copy)]
pub struct VabSample {
    pub index: u16,
    pub offset: u32,
    pub size: u32,
}

pub(crate) struct VabSamples<'a> {
    bank: VabBank<'a>,
    index: u16,
    body_offset: u32,
}

impl<'a> Iterator for VabSamples<'a> {
    type Item = crate::Result<VabSample>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.bank.header.sample_count.get() {
            return None;
        }
        let size_units = match self.bank.sample_size_units(self.index.as_usize()) {
            Ok(units) => units,
            Err(error) => {
                self.index = self.bank.header.sample_count.get();
                return Some(Err(error));
            }
        };
        let size = u32::from(size_units) * VAB_SAMPLE_UNIT;
        let offset = self.body_offset;
        self.body_offset += size;
        let sample = VabSample {
            index: self.index,
            offset,
            size,
        };
        self.index += 1;
        Some(Ok(sample))
    }
}

impl SeqHeader {
    pub fn parse(bytes: &[u8]) -> crate::Result<Self> {
        let record: Self = bytes::read(bytes, 0)?;
        if record.magic != SEQ_MAGIC {
            return Err(crate::Error::invalid_magic("SEQ", record.magic));
        }
        Ok(record)
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Sequence<'a> {
    pub header: SeqHeader,
    bytes: &'a [u8],
}

impl<'a> Sequence<'a> {
    pub fn parse(bytes: &'a [u8]) -> crate::Result<Self> {
        Ok(Self {
            header: SeqHeader::parse(bytes)?,
            bytes,
        })
    }

    pub fn events(&self) -> SeqEvents<'a> {
        SeqEvents {
            bytes: self.bytes,
            at: SEQ_HEADER_SIZE,
            running_status: None,
            stopped: false,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ChannelMessage {
    NoteOff,
    NoteOn,
    PolyphonicKeyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ChannelEvent {
    pub status: u8,
    pub channel: u8,
    pub message: ChannelMessage,
    pub data1: u8,
    pub data2: Option<u8>,
    pub used_running_status: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SeqEventKind<'a> {
    Channel(ChannelEvent),
    Meta { meta_type: u8, data: &'a [u8] },
    SystemExclusive { status: u8, data: &'a [u8] },
    System { status: u8, data: &'a [u8] },
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SeqEvent<'a> {
    pub offset: usize,
    pub delta: u32,
    pub delta_len: usize,
    pub kind: SeqEventKind<'a>,
    pub encoded: &'a [u8],
}

pub(crate) struct SeqEvents<'a> {
    bytes: &'a [u8],
    at: usize,
    running_status: Option<u8>,
    stopped: bool,
}

impl<'a> SeqEvents<'a> {
    fn parse_next(&mut self) -> crate::Result<Option<SeqEvent<'a>>> {
        if self.at == self.bytes.len() {
            return Ok(None);
        }
        let start = self.at;
        let (delta, delta_len) = read_variable_length_at(self.bytes, &mut self.at)?;
        let status_at = self.at;
        let first = bytes::take(self.bytes, &mut self.at)?;
        let (status, first_data, used_running_status) = if first < midi::STATUS_BIT {
            let status = self.running_status.ok_or(crate::Error::invalid_byte(
                "missing SEQ running status",
                status_at,
                first,
            ))?;
            (status, Some(first), true)
        } else {
            (first, None, false)
        };
        let kind = match status {
            midi::CHANNEL_STATUS_FIRST..=midi::CHANNEL_STATUS_LAST => {
                self.running_status = Some(status);
                let message = match status >> midi::MESSAGE_SHIFT {
                    midi::NOTE_OFF => ChannelMessage::NoteOff,
                    midi::NOTE_ON => ChannelMessage::NoteOn,
                    midi::POLYPHONIC_PRESSURE => ChannelMessage::PolyphonicKeyPressure,
                    midi::CONTROL_CHANGE => ChannelMessage::ControlChange,
                    midi::PROGRAM_CHANGE => ChannelMessage::ProgramChange,
                    midi::CHANNEL_PRESSURE => ChannelMessage::ChannelPressure,
                    midi::PITCH_BEND => ChannelMessage::PitchBend,
                    _ => unreachable!(),
                };
                let count = if matches!(
                    message,
                    ChannelMessage::ProgramChange | ChannelMessage::ChannelPressure
                ) {
                    1
                } else {
                    2
                };
                let data1 = match first_data {
                    Some(value) => value,
                    None => take_data(self.bytes, &mut self.at)?,
                };
                let data2 = if count == 2 {
                    Some(take_data(self.bytes, &mut self.at)?)
                } else {
                    None
                };
                SeqEventKind::Channel(ChannelEvent {
                    status,
                    channel: status & midi::CHANNEL_MASK,
                    message,
                    data1,
                    data2,
                    used_running_status,
                })
            }
            midi::META => {
                self.running_status = None;
                let meta_type = bytes::take(self.bytes, &mut self.at)?;
                let (length, _) = read_variable_length_at(self.bytes, &mut self.at)?;
                let size = length.as_usize();
                let data = bytes::span(self.bytes, self.at, size)?;
                self.at += size;
                SeqEventKind::Meta { meta_type, data }
            }
            midi::SYSEX_START | midi::SYSEX_END => {
                self.running_status = None;
                let (length, _) = read_variable_length_at(self.bytes, &mut self.at)?;
                let size = length.as_usize();
                let data = bytes::span(self.bytes, self.at, size)?;
                self.at += size;
                SeqEventKind::SystemExclusive { status, data }
            }
            midi::TIME_CODE
            | midi::SONG_POSITION
            | midi::SONG_SELECT
            | midi::TUNE_REQUEST
            | midi::TIMING_CLOCK
            | midi::START
            | midi::CONTINUE
            | midi::STOP
            | midi::ACTIVE_SENSING => {
                if status < midi::TIMING_CLOCK {
                    self.running_status = None;
                }
                let size = match status {
                    midi::TIME_CODE | midi::SONG_SELECT => 1,
                    midi::SONG_POSITION => 2,
                    _ => 0,
                };
                let data = bytes::span(self.bytes, self.at, size)?;
                for (index, byte) in data.iter().copied().enumerate() {
                    if byte >= midi::STATUS_BIT {
                        return Err(crate::Error::invalid_byte(
                            "invalid SEQ data byte",
                            self.at + index,
                            byte,
                        ));
                    }
                }
                self.at += size;
                SeqEventKind::System { status, data }
            }
            _ => {
                return Err(crate::Error::invalid_byte(
                    "unsupported SEQ status",
                    status_at,
                    status,
                ))
            }
        };
        Ok(Some(SeqEvent {
            offset: start,
            delta,
            delta_len,
            kind,
            encoded: &self.bytes[start..self.at],
        }))
    }
}

impl<'a> Iterator for SeqEvents<'a> {
    type Item = crate::Result<SeqEvent<'a>>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.stopped {
            return None;
        }
        match self.parse_next() {
            Ok(Some(event)) => Some(Ok(event)),
            Ok(None) => {
                self.stopped = true;
                None
            }
            Err(error) => {
                self.stopped = true;
                Some(Err(error))
            }
        }
    }
}

fn read_variable_length_at(bytes: &[u8], at: &mut usize) -> crate::Result<(u32, usize)> {
    let start = *at;
    let mut value = 0u32;
    for index in 0..midi::VLQ_BYTES_MAX {
        let byte = bytes::take(bytes, at)?;
        value = (value << midi::DATA_BITS) | u32::from(byte & midi::DATA_MASK);
        if byte & midi::VLQ_CONTINUATION == 0 {
            return Ok((value, index + 1));
        }
    }
    Err(crate::Error::invalid_at(
        "SEQ variable-length value exceeds four bytes",
        start,
    ))
}

#[track_caller]
fn take_data(bytes: &[u8], at: &mut usize) -> crate::Result<u8> {
    let offset = *at;
    let value = bytes::take(bytes, at)?;
    if value >= midi::STATUS_BIT {
        return Err(crate::Error::invalid_byte(
            "invalid SEQ data byte",
            offset,
            value,
        ));
    }
    Ok(value)
}
