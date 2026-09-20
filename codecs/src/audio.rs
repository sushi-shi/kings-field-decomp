#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct SoundRef {
    pub program: u8,
    pub tone: u8,
    pub note: u8,
}

impl SoundRef {
    pub const BYTE_SIZE: usize = 3;

    pub const fn from_bytes(bytes: [u8; Self::BYTE_SIZE]) -> Self {
        Self {
            program: bytes[0],
            tone: bytes[1],
            note: bytes[2],
        }
    }

    pub const fn to_bytes(self) -> [u8; Self::BYTE_SIZE] {
        [self.program, self.tone, self.note]
    }
}

use core::fmt;

pub const VAB_MAGIC: [u8; 4] = *b"pBAV";
pub const VAB_HEADER_SIZE: usize = 32;
pub const VAB_PROGRAM_SLOTS: usize = 128;
pub const VAB_PROGRAM_SIZE: usize = 16;
pub const VAB_TONES_PER_PROGRAM: usize = 16;
pub const VAB_TONE_SIZE: usize = 32;
pub const VAB_OFFSET_ENTRIES: usize = 256;
pub const VAB_OFFSET_TABLE_SIZE: usize = VAB_OFFSET_ENTRIES * 2;
pub const VAB_SAMPLE_UNIT: usize = 8;


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
pub const SEQ_HEADER_SIZE: usize = 15;
pub const MIDI_VLQ_MAX: u32 = 0x0fff_ffff;

pub const SEQ_SCORE_RECORD_SIZE: usize = 176;

pub const GAME_AUDIO_STATE_SIZE: usize = 0x90;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VabError {
    TruncatedHeader {
        need: usize,
        have: usize,
    },
    InvalidMagic([u8; 4]),
    InvalidCount {
        field: &'static str,
        value: u16,
        maximum: u16,
    },
    HeaderLength {
        expected: usize,
        actual: usize,
    },
    FileSize {
        declared: u32,
        actual: usize,
    },
    NonzeroFirstSampleOffset(u16),
    SampleLengthOverflow {
        sample: u16,
        units: u16,
    },
    BodyLength {
        expected: usize,
        actual: usize,
    },
    OutputFull {
        need: usize,
        have: usize,
    },
}

impl fmt::Display for VabError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "VAB {self:?}")
    }
}

impl core::error::Error for VabError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabHeader {
    pub form: [u8; 4],
    pub version: u32,
    pub id: i32,
    pub file_size: u32,
    pub reserved0: u16,
    pub program_count: u16,
    pub tone_count: u16,
    pub sample_count: u16,
    pub master_volume: u8,
    pub pan: u8,
    pub attribute1: u8,
    pub attribute2: u8,
    pub reserved1: u32,
}

impl VabHeader {
    pub fn parse(bytes: &[u8]) -> Result<Self, VabError> {
        if bytes.len() < VAB_HEADER_SIZE {
            return Err(VabError::TruncatedHeader {
                need: VAB_HEADER_SIZE,
                have: bytes.len(),
            });
        }
        let form: [u8; 4] = bytes[..4].try_into().unwrap();
        if form != VAB_MAGIC {
            return Err(VabError::InvalidMagic(form));
        }
        Ok(Self {
            form,
            version: le_u32(bytes, 4),
            id: le_u32(bytes, 8) as i32,
            file_size: le_u32(bytes, 12),
            reserved0: le_u16(bytes, 16),
            program_count: le_u16(bytes, 18),
            tone_count: le_u16(bytes, 20),
            sample_count: le_u16(bytes, 22),
            master_volume: bytes[24],
            pan: bytes[25],
            attribute1: bytes[26],
            attribute2: bytes[27],
            reserved1: le_u32(bytes, 28),
        })
    }

    pub fn to_le_bytes(self) -> [u8; VAB_HEADER_SIZE] {
        let mut output = [0; VAB_HEADER_SIZE];
        output[..4].copy_from_slice(&self.form);
        output[4..8].copy_from_slice(&self.version.to_le_bytes());
        output[8..12].copy_from_slice(&self.id.to_le_bytes());
        output[12..16].copy_from_slice(&self.file_size.to_le_bytes());
        output[16..18].copy_from_slice(&self.reserved0.to_le_bytes());
        output[18..20].copy_from_slice(&self.program_count.to_le_bytes());
        output[20..22].copy_from_slice(&self.tone_count.to_le_bytes());
        output[22..24].copy_from_slice(&self.sample_count.to_le_bytes());
        output[24] = self.master_volume;
        output[25] = self.pan;
        output[26] = self.attribute1;
        output[27] = self.attribute2;
        output[28..32].copy_from_slice(&self.reserved1.to_le_bytes());
        output
    }

    pub fn encoded_header_len(self) -> Result<usize, VabError> {
        validate_counts(self)?;
        Ok(VAB_HEADER_SIZE
            + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE
            + usize::from(self.program_count) * VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE
            + VAB_OFFSET_TABLE_SIZE)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabProgram {
    pub tone_count: u8,
    pub master_volume: u8,
    pub priority: u8,
    pub mode: u8,
    pub pan: u8,
    pub reserved0: i8,
    pub attribute: i16,
    pub reserved1: u32,
    pub reserved2: u32,
}

impl VabProgram {
    fn parse(bytes: &[u8]) -> Self {
        Self {
            tone_count: bytes[0],
            master_volume: bytes[1],
            priority: bytes[2],
            mode: bytes[3],
            pan: bytes[4],
            reserved0: bytes[5] as i8,
            attribute: le_u16(bytes, 6) as i16,
            reserved1: le_u32(bytes, 8),
            reserved2: le_u32(bytes, 12),
        }
    }

    pub fn to_le_bytes(self) -> [u8; VAB_PROGRAM_SIZE] {
        let mut output = [0; VAB_PROGRAM_SIZE];
        output[0] = self.tone_count;
        output[1] = self.master_volume;
        output[2] = self.priority;
        output[3] = self.mode;
        output[4] = self.pan;
        output[5] = self.reserved0 as u8;
        output[6..8].copy_from_slice(&self.attribute.to_le_bytes());
        output[8..12].copy_from_slice(&self.reserved1.to_le_bytes());
        output[12..16].copy_from_slice(&self.reserved2.to_le_bytes());
        output
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VabTone {
    pub priority: u8,
    pub mode: u8,
    pub volume: u8,
    pub pan: u8,
    pub center_note: u8,
    pub center_shift: u8,
    pub minimum_note: u8,
    pub maximum_note: u8,
    pub vibrato_width: u8,
    pub vibrato_time: u8,
    pub portamento_width: u8,
    pub portamento_time: u8,
    pub pitch_bend_minimum: u8,
    pub pitch_bend_maximum: u8,
    pub reserved1: u8,
    pub reserved2: u8,
    pub adsr1: u16,
    pub adsr2: u16,
    pub program: i16,
    pub sample: i16,
    pub reserved: [i16; 4],
}

impl VabTone {
    fn parse(bytes: &[u8]) -> Self {
        Self {
            priority: bytes[0],
            mode: bytes[1],
            volume: bytes[2],
            pan: bytes[3],
            center_note: bytes[4],
            center_shift: bytes[5],
            minimum_note: bytes[6],
            maximum_note: bytes[7],
            vibrato_width: bytes[8],
            vibrato_time: bytes[9],
            portamento_width: bytes[10],
            portamento_time: bytes[11],
            pitch_bend_minimum: bytes[12],
            pitch_bend_maximum: bytes[13],
            reserved1: bytes[14],
            reserved2: bytes[15],
            adsr1: le_u16(bytes, 16),
            adsr2: le_u16(bytes, 18),
            program: le_u16(bytes, 20) as i16,
            sample: le_u16(bytes, 22) as i16,
            reserved: [
                le_u16(bytes, 24) as i16,
                le_u16(bytes, 26) as i16,
                le_u16(bytes, 28) as i16,
                le_u16(bytes, 30) as i16,
            ],
        }
    }

    pub fn to_le_bytes(self) -> [u8; VAB_TONE_SIZE] {
        let mut output = [0; VAB_TONE_SIZE];
        output[..16].copy_from_slice(&[
            self.priority,
            self.mode,
            self.volume,
            self.pan,
            self.center_note,
            self.center_shift,
            self.minimum_note,
            self.maximum_note,
            self.vibrato_width,
            self.vibrato_time,
            self.portamento_width,
            self.portamento_time,
            self.pitch_bend_minimum,
            self.pitch_bend_maximum,
            self.reserved1,
            self.reserved2,
        ]);
        for (at, value) in [
            self.adsr1,
            self.adsr2,
            self.program as u16,
            self.sample as u16,
            self.reserved[0] as u16,
            self.reserved[1] as u16,
            self.reserved[2] as u16,
            self.reserved[3] as u16,
        ]
        .into_iter()
        .enumerate()
        {
            let offset = 16 + at * 2;
            output[offset..offset + 2].copy_from_slice(&value.to_le_bytes());
        }
        output
    }
}

#[derive(Debug, Clone, Copy)]
pub struct VabBank<'a> {
    pub header: VabHeader,
    header_bytes: &'a [u8],
    body_bytes: &'a [u8],
    tone_table_offset: usize,
    offset_table_offset: usize,
}

impl<'a> VabBank<'a> {
    pub fn parse(header_bytes: &'a [u8], body_bytes: &'a [u8]) -> Result<Self, VabError> {
        let header = VabHeader::parse(header_bytes)?;
        let expected_header = header.encoded_header_len()?;
        if header_bytes.len() != expected_header {
            return Err(VabError::HeaderLength {
                expected: expected_header,
                actual: header_bytes.len(),
            });
        }
        let actual_file_size =
            header_bytes
                .len()
                .checked_add(body_bytes.len())
                .ok_or(VabError::FileSize {
                    declared: header.file_size,
                    actual: usize::MAX,
                })?;
        if usize::try_from(header.file_size).ok() != Some(actual_file_size) {
            return Err(VabError::FileSize {
                declared: header.file_size,
                actual: actual_file_size,
            });
        }
        let tone_table_offset = VAB_HEADER_SIZE + VAB_PROGRAM_SLOTS * VAB_PROGRAM_SIZE;
        let offset_table_offset = expected_header - VAB_OFFSET_TABLE_SIZE;
        let first = le_u16(header_bytes, offset_table_offset);
        if first != 0 {
            return Err(VabError::NonzeroFirstSampleOffset(first));
        }
        let mut body_len = 0usize;
        for sample in 0..header.sample_count {
            let units = le_u16(
                header_bytes,
                offset_table_offset + (usize::from(sample) + 1) * 2,
            );
            let bytes = usize::from(units)
                .checked_mul(VAB_SAMPLE_UNIT)
                .ok_or(VabError::SampleLengthOverflow { sample, units })?;
            body_len = body_len
                .checked_add(bytes)
                .ok_or(VabError::SampleLengthOverflow { sample, units })?;
        }
        if body_len != body_bytes.len() {
            return Err(VabError::BodyLength {
                expected: body_len,
                actual: body_bytes.len(),
            });
        }
        Ok(Self {
            header,
            header_bytes,
            body_bytes,
            tone_table_offset,
            offset_table_offset,
        })
    }

    pub fn encoded_header(&self) -> &'a [u8] {
        self.header_bytes
    }

    pub fn encoded_body(&self) -> &'a [u8] {
        self.body_bytes
    }

    pub fn program(&self, index: usize) -> Option<VabProgram> {
        if index >= usize::from(self.header.program_count) {
            return None;
        }
        self.program_slot(index)
    }

    pub fn program_slot(&self, index: usize) -> Option<VabProgram> {
        if index >= VAB_PROGRAM_SLOTS {
            return None;
        }
        let at = VAB_HEADER_SIZE + index * VAB_PROGRAM_SIZE;
        Some(VabProgram::parse(
            &self.header_bytes[at..at + VAB_PROGRAM_SIZE],
        ))
    }

    pub fn programs(&self) -> VabPrograms<'a> {
        VabPrograms {
            bytes: self.header_bytes,
            index: 0,
            count: usize::from(self.header.program_count),
        }
    }

    pub fn program_slots(&self) -> VabPrograms<'a> {
        VabPrograms {
            bytes: self.header_bytes,
            index: 0,
            count: VAB_PROGRAM_SLOTS,
        }
    }

    pub fn tone(&self, program: usize, tone: usize) -> Option<VabTone> {
        if program >= usize::from(self.header.program_count) || tone >= VAB_TONES_PER_PROGRAM {
            return None;
        }
        let at = self.tone_table_offset + (program * VAB_TONES_PER_PROGRAM + tone) * VAB_TONE_SIZE;
        Some(VabTone::parse(&self.header_bytes[at..at + VAB_TONE_SIZE]))
    }

    pub fn tones(&self, program: usize) -> Option<VabTones<'a>> {
        let count = usize::from(self.program(program)?.tone_count).min(VAB_TONES_PER_PROGRAM);
        Some(VabTones {
            bytes: self.header_bytes,
            at: self.tone_table_offset + program * VAB_TONES_PER_PROGRAM * VAB_TONE_SIZE,
            index: 0,
            count,
        })
    }

    pub fn sample_size_units(&self, sample: usize) -> Option<u16> {
        if sample >= usize::from(self.header.sample_count) {
            return None;
        }
        Some(le_u16(
            self.header_bytes,
            self.offset_table_offset + (sample + 1) * 2,
        ))
    }

    pub fn samples(&self) -> VabSamples<'a> {
        VabSamples {
            bank: *self,
            index: 0,
            body_offset: 0,
        }
    }

    pub fn encode_header_into(&self, output: &mut [u8]) -> Result<usize, VabError> {
        copy_output(self.header_bytes, output)
    }

    pub fn encode_body_into(&self, output: &mut [u8]) -> Result<usize, VabError> {
        copy_output(self.body_bytes, output)
    }
}

fn validate_counts(header: VabHeader) -> Result<(), VabError> {
    if usize::from(header.program_count) > VAB_PROGRAM_SLOTS {
        return Err(VabError::InvalidCount {
            field: "programs",
            value: header.program_count,
            maximum: VAB_PROGRAM_SLOTS as u16,
        });
    }
    let maximum_tones = header
        .program_count
        .saturating_mul(VAB_TONES_PER_PROGRAM as u16);
    if header.tone_count > maximum_tones {
        return Err(VabError::InvalidCount {
            field: "tones",
            value: header.tone_count,
            maximum: maximum_tones,
        });
    }
    if usize::from(header.sample_count) >= VAB_OFFSET_ENTRIES {
        return Err(VabError::InvalidCount {
            field: "samples",
            value: header.sample_count,
            maximum: (VAB_OFFSET_ENTRIES - 1) as u16,
        });
    }
    Ok(())
}

fn copy_output(input: &[u8], output: &mut [u8]) -> Result<usize, VabError> {
    if output.len() < input.len() {
        return Err(VabError::OutputFull {
            need: input.len(),
            have: output.len(),
        });
    }
    output[..input.len()].copy_from_slice(input);
    Ok(input.len())
}

pub struct VabPrograms<'a> {
    bytes: &'a [u8],
    index: usize,
    count: usize,
}

impl Iterator for VabPrograms<'_> {
    type Item = VabProgram;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.count {
            return None;
        }
        let at = VAB_HEADER_SIZE + self.index * VAB_PROGRAM_SIZE;
        self.index += 1;
        Some(VabProgram::parse(&self.bytes[at..at + VAB_PROGRAM_SIZE]))
    }
}

pub struct VabTones<'a> {
    bytes: &'a [u8],
    at: usize,
    index: usize,
    count: usize,
}

impl Iterator for VabTones<'_> {
    type Item = VabTone;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.count {
            return None;
        }
        let at = self.at + self.index * VAB_TONE_SIZE;
        self.index += 1;
        Some(VabTone::parse(&self.bytes[at..at + VAB_TONE_SIZE]))
    }
}

#[derive(Debug, Clone, Copy)]
pub struct VabSample<'a> {
    pub index: u16,
    pub offset: usize,
    pub size_units: u16,
    pub data: &'a [u8],
}

pub struct VabSamples<'a> {
    bank: VabBank<'a>,
    index: u16,
    body_offset: usize,
}

impl<'a> Iterator for VabSamples<'a> {
    type Item = VabSample<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index == self.bank.header.sample_count {
            return None;
        }
        let size_units = self.bank.sample_size_units(usize::from(self.index))?;
        let size = usize::from(size_units) * VAB_SAMPLE_UNIT;
        let offset = self.body_offset;
        self.body_offset += size;
        let sample = VabSample {
            index: self.index,
            offset,
            size_units,
            data: &self.bank.body_bytes[offset..offset + size],
        };
        self.index += 1;
        Some(sample)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SeqError {
    Truncated {
        at: usize,
        need: usize,
        available: usize,
    },
    InvalidMagic([u8; 4]),
    VariableLengthTooLong {
        at: usize,
    },
    VariableLengthOutOfRange(u32),
    MissingRunningStatus {
        at: usize,
        byte: u8,
    },
    InvalidDataByte {
        at: usize,
        byte: u8,
    },
    UnsupportedStatus {
        at: usize,
        status: u8,
    },
    OutputFull {
        need: usize,
        have: usize,
    },
}

impl fmt::Display for SeqError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "SEQ {self:?}")
    }
}

impl core::error::Error for SeqError {}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SeqHeader {
    pub form: [u8; 4],
    pub version: u32,
    pub resolution: u16,

    pub tempo: u32,
    pub time_signature_numerator: u8,
    pub time_signature_denominator_shift: u8,
}

impl SeqHeader {
    pub fn parse(bytes: &[u8]) -> Result<Self, SeqError> {
        let header = seq_span(bytes, 0, SEQ_HEADER_SIZE)?;
        let form: [u8; 4] = header[..4].try_into().unwrap();
        if form != SEQ_MAGIC {
            return Err(SeqError::InvalidMagic(form));
        }
        Ok(Self {
            form,
            version: u32::from_be_bytes(header[4..8].try_into().unwrap()),
            resolution: u16::from_be_bytes(header[8..10].try_into().unwrap()),
            tempo: u32::from_be_bytes([0, header[10], header[11], header[12]]),
            time_signature_numerator: header[13],
            time_signature_denominator_shift: header[14],
        })
    }

    pub fn to_be_bytes(self) -> Result<[u8; SEQ_HEADER_SIZE], SeqError> {
        if self.tempo > 0x00ff_ffff {
            return Err(SeqError::VariableLengthOutOfRange(self.tempo));
        }
        let mut output = [0; SEQ_HEADER_SIZE];
        output[..4].copy_from_slice(&self.form);
        output[4..8].copy_from_slice(&self.version.to_be_bytes());
        output[8..10].copy_from_slice(&self.resolution.to_be_bytes());
        let tempo = self.tempo.to_be_bytes();
        output[10..13].copy_from_slice(&tempo[1..]);
        output[13] = self.time_signature_numerator;
        output[14] = self.time_signature_denominator_shift;
        Ok(output)
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Sequence<'a> {
    pub header: SeqHeader,
    bytes: &'a [u8],
}

impl<'a> Sequence<'a> {
    pub fn parse(bytes: &'a [u8]) -> Result<Self, SeqError> {
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

    pub fn encoded(&self) -> &'a [u8] {
        self.bytes
    }

    pub fn encode_into(&self, output: &mut [u8]) -> Result<usize, SeqError> {
        if output.len() < self.bytes.len() {
            return Err(SeqError::OutputFull {
                need: self.bytes.len(),
                have: output.len(),
            });
        }
        output[..self.bytes.len()].copy_from_slice(self.bytes);
        Ok(self.bytes.len())
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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SeqCallback {
    KeyOn {
        sequence: i16,
        vab_id: i16,
        program: u8,
        note: u8,
        volume: i16,
        pan: u8,
    },
    KeyOff {
        sequence: i16,
        vab_id: i16,
        program: u8,
        note: u8,
    },
    PitchBend {
        sequence: i16,
        vab_id: i16,
        program: u8,
        value: u8,
    },
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SeqRuntimeError {
    Decode(SeqError),
    UnsupportedVersion(u32),
    ZeroTempo,
    ZeroResolution,
    ZeroVblank,
    EmptySequence,
    AddressOutOfRange(usize),
    InvalidChannel(u8),
    ArithmeticTrap,
    InvalidSequenceSlot(i16),
    SequenceSlotsUnavailable(u32),
    SequenceSlotOccupied(i16),
    MissingFollowingEvent,
    UnsupportedChannelMessage(ChannelMessage),
    UnsupportedControl(u8),
    UnsupportedMeta(u8),
    UnsupportedSystem(u8),
}

impl fmt::Display for SeqRuntimeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "SEQ runtime {self:?}")
    }
}

impl core::error::Error for SeqRuntimeError {}

impl From<SeqError> for SeqRuntimeError {
    fn from(error: SeqError) -> Self {
        Self::Decode(error)
    }
}

pub fn initialize_sequence_score(
    sequence: &Sequence<'_>,
    sequence_address: u32,
    vab_id: i16,
    vblank_minus: u32,
    score: &mut [u8; SEQ_SCORE_RECORD_SIZE],
) -> Result<(), SeqRuntimeError> {
    if sequence.header.version != 1 {
        return Err(SeqRuntimeError::UnsupportedVersion(sequence.header.version));
    }
    if sequence.header.tempo == 0 {
        return Err(SeqRuntimeError::ZeroTempo);
    }
    if sequence.header.resolution == 0 {
        return Err(SeqRuntimeError::ZeroResolution);
    }
    if vblank_minus == 0 {
        return Err(SeqRuntimeError::ZeroVblank);
    }
    let first = sequence
        .events()
        .next()
        .ok_or(SeqRuntimeError::EmptySequence)??;

    put_score_u16(score, 108, 0);
    for offset in [63, 64, 85, 65, 66, 86, 62] {
        score[offset] = 0;
    }
    put_score_u32(score, 152, 0);
    put_score_u32(score, 156, 0);
    put_score_u32(score, 160, 0);
    put_score_u16(score, 148, 0);
    put_score_u16(score, 106, 0);
    score[87] = 0;
    put_score_u32(score, 164, 0);
    score[83] = 0;
    score[84] = 0;
    score[60] = 0;
    put_score_u16(score, 110, vab_id as u16);
    for channel in 0..16 {
        score[88 + channel] = channel as u8;
        score[67 + channel] = 64;
        put_score_u16(score, 112 + channel * 2, 127);
    }
    put_score_u16(score, 144, 1);
    put_score_u16(score, 42, 127);
    put_score_u16(score, 44, 127);
    put_score_u32(score, 48, sequence_address);

    put_score_u16(score, 108, sequence.header.resolution);
    let tempo = sequence.header.tempo;
    let quotient = 60_000_000 / tempo;
    let remainder = 60_000_000 % tempo;
    let beats_per_minute = quotient.wrapping_add(u32::from(remainder > tempo / 2));
    put_score_u32(score, 160, beats_per_minute);
    put_score_u32(score, 168, beats_per_minute);

    let cursor = event_data_address(sequence_address, first.offset, first.delta_len)?;
    put_score_u32(score, 48, cursor);
    put_score_u32(score, 52, cursor);
    put_score_u32(score, 56, cursor);
    let scaled_delta = first.delta.wrapping_mul(10);
    put_score_u32(score, 152, scaled_delta);
    put_score_u32(score, 156, scaled_delta);
    put_score_u32(score, 164, scaled_delta);

    update_tick_interval(
        score,
        sequence.header.resolution,
        beats_per_minute,
        vblank_minus,
    )?;
    Ok(())
}

pub fn apply_successful_game_sequence_open(
    audio_state: &mut [u8; GAME_AUDIO_STATE_SIZE],
    open_flags: &mut u32,
    allocated_sequence_id: i16,
) -> Result<(), SeqRuntimeError> {
    if !(0..32).contains(&allocated_sequence_id) {
        return Err(SeqRuntimeError::InvalidSequenceSlot(allocated_sequence_id));
    }
    if (*open_flags as i32) < 0 {
        return Err(SeqRuntimeError::SequenceSlotsUnavailable(*open_flags));
    }
    let bit = 1u32 << allocated_sequence_id;
    if *open_flags & bit != 0 {
        return Err(SeqRuntimeError::SequenceSlotOccupied(allocated_sequence_id));
    }
    *open_flags |= bit;
    audio_state[12..14].copy_from_slice(&allocated_sequence_id.to_le_bytes());
    audio_state[16..20].copy_from_slice(&1u32.to_le_bytes());
    Ok(())
}

pub fn apply_sequence_event(
    current: &SeqEvent<'_>,
    following: Option<&SeqEvent<'_>>,
    sequence_address: u32,
    sequence_id: i16,
    score: &mut [u8; SEQ_SCORE_RECORD_SIZE],
) -> Result<Option<SeqCallback>, SeqRuntimeError> {
    match current.kind {
        SeqEventKind::Channel(channel) => {
            if !channel.used_running_status {
                score[61] = channel.status & 0xf0;
                score[62] = channel.channel;
            }
            let active_channel = score[62];
            if active_channel >= 16 {
                return Err(SeqRuntimeError::InvalidChannel(active_channel));
            }
            let callback = match channel.message {
                ChannelMessage::NoteOn => {
                    let velocity = channel.data2.unwrap_or(0);
                    let channel = usize::from(active_channel);
                    let program = score[88 + channel];
                    let vab_id = score_i16(score, 110);
                    if velocity == 0 {
                        Some(SeqCallback::KeyOff {
                            sequence: sequence_id,
                            vab_id,
                            program,
                            note: channel_event_note(current),
                        })
                    } else {
                        let channel_volume = i32::from(score_i16(score, 112 + channel * 2));
                        let volume = (i32::from(velocity) * channel_volume / 127) as i16;
                        Some(SeqCallback::KeyOn {
                            sequence: sequence_id,
                            vab_id,
                            program,
                            note: channel_event_note(current),
                            volume,
                            pan: score[67 + channel],
                        })
                    }
                }
                ChannelMessage::ControlChange => {
                    let controller = channel.data1;
                    if controller != 7 {
                        return Err(SeqRuntimeError::UnsupportedControl(controller));
                    }
                    let value = channel
                        .data2
                        .ok_or(SeqRuntimeError::UnsupportedControl(controller))?;
                    put_score_u16(
                        score,
                        112 + usize::from(active_channel) * 2,
                        u16::from(value),
                    );
                    None
                }
                ChannelMessage::ProgramChange => {
                    score[88 + usize::from(active_channel)] = channel.data1;
                    None
                }
                ChannelMessage::PitchBend => {
                    let value = channel
                        .data2
                        .ok_or(SeqRuntimeError::UnsupportedChannelMessage(
                            ChannelMessage::PitchBend,
                        ))?;
                    Some(SeqCallback::PitchBend {
                        sequence: sequence_id,
                        vab_id: score_i16(score, 110),
                        program: score[88 + usize::from(active_channel)],
                        value,
                    })
                }
                message => return Err(SeqRuntimeError::UnsupportedChannelMessage(message)),
            };
            advance_score_delta(following, sequence_address, score)?;
            Ok(callback)
        }
        SeqEventKind::Meta {
            meta_type: 0x2f, ..
        } => {
            score[61] = 0xff;
            score[62] = 0x0f;
            let loops = score_u16(score, 106).wrapping_add(1);
            put_score_u16(score, 106, loops);
            if score_i16(score, 104) != 0 {
                return Err(SeqRuntimeError::UnsupportedMeta(0x2f));
            }
            put_score_u32(score, 156, 0);
            score[83] = 0;
            put_score_u32(score, 164, 0);
            put_score_u32(score, 48, score_u32(score, 52));
            Ok(None)
        }
        SeqEventKind::Meta { meta_type, .. } => Err(SeqRuntimeError::UnsupportedMeta(meta_type)),
        SeqEventKind::SystemExclusive { status, .. } | SeqEventKind::System { status, .. } => {
            Err(SeqRuntimeError::UnsupportedSystem(status))
        }
    }
}

fn channel_event_note(event: &SeqEvent<'_>) -> u8 {
    match event.kind {
        SeqEventKind::Channel(channel) => channel.data1,
        _ => unreachable!(),
    }
}

fn advance_score_delta(
    following: Option<&SeqEvent<'_>>,
    sequence_address: u32,
    score: &mut [u8; SEQ_SCORE_RECORD_SIZE],
) -> Result<(), SeqRuntimeError> {
    let following = following.ok_or(SeqRuntimeError::MissingFollowingEvent)?;
    let cursor = event_data_address(sequence_address, following.offset, following.delta_len)?;
    put_score_u32(score, 48, cursor);
    let scaled = following.delta.wrapping_mul(10);
    put_score_u32(score, 156, score_u32(score, 156).wrapping_add(scaled));
    put_score_u32(score, 164, scaled);
    Ok(())
}

fn event_data_address(
    sequence_address: u32,
    offset: usize,
    delta_len: usize,
) -> Result<u32, SeqRuntimeError> {
    let relative = offset
        .checked_add(delta_len)
        .ok_or(SeqRuntimeError::AddressOutOfRange(usize::MAX))?;
    let relative =
        u32::try_from(relative).map_err(|_| SeqRuntimeError::AddressOutOfRange(relative))?;
    Ok(sequence_address.wrapping_add(relative))
}

fn update_tick_interval(
    score: &mut [u8; SEQ_SCORE_RECORD_SIZE],
    resolution: u16,
    beats_per_minute: u32,
    vblank_minus: u32,
) -> Result<(), SeqRuntimeError> {
    let rate = (i32::from(resolution as i16) as u32).wrapping_mul(beats_per_minute);
    let vblank_60 = vblank_minus.wrapping_mul(60);
    let rate_10 = rate.wrapping_mul(10);
    if rate_10 < vblank_60 {
        if rate == 0 {
            return Err(SeqRuntimeError::ArithmeticTrap);
        }
        let interval = vblank_minus.wrapping_mul(600) / rate;
        put_score_u16(score, 144, interval as u16);
        put_score_u16(score, 146, interval as u16);
    } else {
        if vblank_60 == 0 {
            return Err(SeqRuntimeError::ArithmeticTrap);
        }
        put_score_u16(score, 144, u16::MAX);
        let mut interval = rate_10 / vblank_60;
        let remainder = rate_10 % vblank_60;
        if vblank_minus.wrapping_mul(30) < remainder {
            interval = interval.wrapping_add(1);
        }
        put_score_u16(score, 146, interval as u16);
    }
    put_score_u16(score, 148, score_u16(score, 146));
    Ok(())
}

fn score_u16(score: &[u8; SEQ_SCORE_RECORD_SIZE], at: usize) -> u16 {
    u16::from_le_bytes([score[at], score[at + 1]])
}

fn score_i16(score: &[u8; SEQ_SCORE_RECORD_SIZE], at: usize) -> i16 {
    score_u16(score, at) as i16
}

fn score_u32(score: &[u8; SEQ_SCORE_RECORD_SIZE], at: usize) -> u32 {
    u32::from_le_bytes([score[at], score[at + 1], score[at + 2], score[at + 3]])
}

fn put_score_u16(score: &mut [u8; SEQ_SCORE_RECORD_SIZE], at: usize, value: u16) {
    score[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn put_score_u32(score: &mut [u8; SEQ_SCORE_RECORD_SIZE], at: usize, value: u32) {
    score[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

pub struct SeqEvents<'a> {
    bytes: &'a [u8],
    at: usize,
    running_status: Option<u8>,
    stopped: bool,
}

impl<'a> SeqEvents<'a> {
    pub const fn position(&self) -> usize {
        self.at
    }

    pub fn remainder(&self) -> &'a [u8] {
        &self.bytes[self.at..]
    }

    fn parse_next(&mut self) -> Result<Option<SeqEvent<'a>>, SeqError> {
        if self.at == self.bytes.len() {
            return Ok(None);
        }
        let start = self.at;
        let (delta, delta_len) = read_variable_length_at(self.bytes, &mut self.at)?;
        let status_at = self.at;
        let first = take(self.bytes, &mut self.at)?;
        let (status, first_data, used_running_status) = if first < midi::STATUS_BIT {
            let status = self.running_status.ok_or(SeqError::MissingRunningStatus {
                at: status_at,
                byte: first,
            })?;
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
                let meta_type = take(self.bytes, &mut self.at)?;
                let (length, _) = read_variable_length_at(self.bytes, &mut self.at)?;
                let size = usize::try_from(length)
                    .map_err(|_| SeqError::VariableLengthOutOfRange(length))?;
                let data = seq_span(self.bytes, self.at, size)?;
                self.at += size;
                SeqEventKind::Meta { meta_type, data }
            }
            midi::SYSEX_START | midi::SYSEX_END => {
                self.running_status = None;
                let (length, _) = read_variable_length_at(self.bytes, &mut self.at)?;
                let size = usize::try_from(length)
                    .map_err(|_| SeqError::VariableLengthOutOfRange(length))?;
                let data = seq_span(self.bytes, self.at, size)?;
                self.at += size;
                SeqEventKind::SystemExclusive { status, data }
            }
            midi::TIME_CODE | midi::SONG_POSITION | midi::SONG_SELECT | midi::TUNE_REQUEST | midi::TIMING_CLOCK | midi::START | midi::CONTINUE | midi::STOP | midi::ACTIVE_SENSING => {
                if status < midi::TIMING_CLOCK {
                    self.running_status = None;
                }
                let size = match status {
                    midi::TIME_CODE | midi::SONG_SELECT => 1,
                    midi::SONG_POSITION => 2,
                    _ => 0,
                };
                let data = seq_span(self.bytes, self.at, size)?;
                for (index, byte) in data.iter().copied().enumerate() {
                    if byte >= midi::STATUS_BIT {
                        return Err(SeqError::InvalidDataByte {
                            at: self.at + index,
                            byte,
                        });
                    }
                }
                self.at += size;
                SeqEventKind::System { status, data }
            }
            _ => {
                return Err(SeqError::UnsupportedStatus {
                    at: status_at,
                    status,
                })
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
    type Item = Result<SeqEvent<'a>, SeqError>;

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

pub fn read_variable_length(bytes: &[u8]) -> Result<(u32, usize), SeqError> {
    let mut at = 0;
    let (value, size) = read_variable_length_at(bytes, &mut at)?;
    Ok((value, size))
}

fn read_variable_length_at(bytes: &[u8], at: &mut usize) -> Result<(u32, usize), SeqError> {
    let start = *at;
    let mut value = 0u32;
    for index in 0..midi::VLQ_BYTES_MAX {
        let byte = take(bytes, at)?;
        value = (value << midi::DATA_BITS) | u32::from(byte & midi::DATA_MASK);
        if byte & midi::VLQ_CONTINUATION == 0 {
            return Ok((value, index + 1));
        }
    }
    Err(SeqError::VariableLengthTooLong { at: start })
}

pub fn encode_variable_length(value: u32, output: &mut [u8]) -> Result<usize, SeqError> {
    if value > MIDI_VLQ_MAX {
        return Err(SeqError::VariableLengthOutOfRange(value));
    }
    let size = if value >= 1 << (3 * midi::DATA_BITS) {
        4
    } else if value >= 1 << (2 * midi::DATA_BITS) {
        3
    } else if value >= 1 << midi::DATA_BITS {
        2
    } else {
        1
    };
    if output.len() < size {
        return Err(SeqError::OutputFull {
            need: size,
            have: output.len(),
        });
    }
    for (index, destination) in output[..size].iter_mut().enumerate() {
        let shift = (size - index - 1) * midi::DATA_BITS;
        *destination = ((value >> shift) & u32::from(midi::DATA_MASK)) as u8;
        if index + 1 != size {
            *destination |= midi::VLQ_CONTINUATION;
        }
    }
    Ok(size)
}

fn take(bytes: &[u8], at: &mut usize) -> Result<u8, SeqError> {
    let value = *bytes.get(*at).ok_or(SeqError::Truncated {
        at: *at,
        need: 1,
        available: 0,
    })?;
    *at += 1;
    Ok(value)
}

fn take_data(bytes: &[u8], at: &mut usize) -> Result<u8, SeqError> {
    let offset = *at;
    let value = take(bytes, at)?;
    if value >= midi::STATUS_BIT {
        return Err(SeqError::InvalidDataByte {
            at: offset,
            byte: value,
        });
    }
    Ok(value)
}

fn seq_span(bytes: &[u8], at: usize, size: usize) -> Result<&[u8], SeqError> {
    at.checked_add(size)
        .and_then(|end| bytes.get(at..end))
        .ok_or(SeqError::Truncated {
            at,
            need: size,
            available: bytes.len().saturating_sub(at),
        })
}

fn le_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn le_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}
