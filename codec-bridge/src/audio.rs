use crate::{INVALID, OK, OUTPUT_FULL};
use core::{mem::align_of, ptr, slice};
use kf_codec::audio::midi;
use kf_codec::audio::{ChannelMessage, SeqEventKind, Sequence, VabBank};
use kf_codec::audio::{VAB_OFFSET_ENTRIES, VAB_PROGRAM_SLOTS, VAB_TONES_PER_PROGRAM};

const SUPPORTED_SEQ_VERSION: u32 = 1;
const TONE_REVERB_FLAG: u8 = 4;
const ADSR_ATTACK_SHIFT_OFFSET: u32 = 10;
const ADSR_ATTACK_STEP_OFFSET: u32 = 8;
const ADSR_DECAY_SHIFT_OFFSET: u32 = 4;
const ADSR_SUSTAIN_SHIFT_OFFSET: u32 = 8;
const ADSR_SUSTAIN_STEP_OFFSET: u32 = 6;
const ADSR_RATE_SHIFT_MASK: u16 = 31;
const ADSR_STEP_MASK: u16 = 3;
const ADSR_DECAY_SHIFT_MASK: u16 = 15;
const ADSR_SUSTAIN_LEVEL_MASK: u32 = 15;
const ADSR_SUSTAIN_LEVEL_STEP: u32 = 2048;
const ENVELOPE_LEVEL_MAX: u32 = 32767;
const ADSR_EXPONENTIAL_OFFSET: u32 = 15;
const ADSR_SUSTAIN_DIRECTION_OFFSET: u32 = 14;
const ADSR_RELEASE_EXPONENTIAL_OFFSET: u32 = 5;
const ADPCM_BLOCK_BYTES: usize = 16;
const ADPCM_BLOCK_FRAMES: usize = 28;
const ADPCM_HEADER_BYTES: usize = 2;
const ADPCM_FILTER_OFFSET: u32 = 4;
const ADPCM_FILTER_LAST: u8 = 4;
const ADPCM_FLAGS_MASK: u8 = 7;
const ADPCM_LOOP_START: u8 = 4;
const ADPCM_END: u8 = 1;
const ADPCM_REPEAT: u8 = 2;
const ADPCM_SHIFT_MASK: u8 = 15;
const ADPCM_MAX_SHIFT: u8 = 12;
const ADPCM_RESERVED_SHIFT: u8 = 9;
const ADPCM_NIBBLE_BITS: usize = 4;
const ADPCM_NIBBLE_MASK: u8 = 15;
const ADPCM_NIBBLE_SIGN_BIT: i32 = 8;
const ADPCM_NIBBLE_MODULUS: i32 = 16;
const ADPCM_SAMPLE_SCALE: i32 = 4096;
const ADPCM_PREDICTOR_ROUNDING: i32 = 32;
const ADPCM_PREDICTOR_FRACTION_BITS: u32 = 6;

// Wire values match KfMusicEventKind in include/kf/audio/codec.h.
#[repr(u32)]
enum MusicEventKind {
    NoteOff = 0,
    NoteOn = 1,
    Volume = 2,
    Program = 3,
    PitchBend = 4,
    Tempo = 5,
    End = 6,
}

#[repr(C)]
struct Envelope {
    attack_shift: u8,
    attack_step: u8,
    decay_shift: u8,
    sustain_shift: u8,
    sustain_step: u8,
    release_shift: u8,
    sustain_level: u16,
    attack_exponential: u8,
    sustain_exponential: u8,
    sustain_decreasing: u8,
    release_exponential: u8,
}
#[repr(C)]
struct Tone {
    priority: u8,
    reverb: u8,
    volume: u8,
    pan: u8,
    center_note: u8,
    center_shift: u8,
    minimum_note: u8,
    maximum_note: u8,
    vibrato_width: u8,
    vibrato_time: u8,
    portamento_width: u8,
    portamento_time: u8,
    bend_down: u8,
    bend_up: u8,
    sample_index: u16,
    envelope: Envelope,
}
#[repr(C)]
struct Program {
    tone_count: u8,
    volume: u8,
    pan: u8,
    priority: u8,
    tones: [Tone; VAB_TONES_PER_PROGRAM],
}
#[repr(C)]
struct SampleRange {
    offset: u32,
    size: u32,
}
#[repr(C)]
pub struct BankData {
    volume: u8,
    pan: u8,
    sample_count: u16,
    programs: [Program; VAB_PROGRAM_SLOTS],
    samples: [SampleRange; VAB_OFFSET_ENTRIES],
}
#[repr(C)]
pub struct SampleInfo {
    frames: u32,
    loop_begin: u32,
    loop_end: u32,
}
#[repr(C)]
pub struct MusicEvent {
    delta: u32,
    kind: u32,
    value: u32,
    channel: u8,
    note: u8,
    velocity: u8,
    reserved: u8,
}
#[repr(C)]
pub struct MusicInfo {
    resolution: u32,
    tempo: u32,
    event_count: u32,
}

fn input(data: *const u8, size: usize) -> bool {
    !data.is_null() && size <= isize::MAX as usize && size <= u32::MAX as usize
}
fn output<T>(data: *mut T) -> bool {
    !data.is_null() && data as usize % align_of::<T>() == 0
}

#[no_mangle]
pub unsafe extern "C" fn kf_audio_bank_decode(
    header: *const u8,
    header_size: usize,
    body: *const u8,
    body_size: usize,
    destination: *mut BankData,
) -> i32 {
    if !input(header, header_size) || !input(body, body_size) || !output(destination) {
        return INVALID;
    }
    let bank = match VabBank::parse(
        slice::from_raw_parts(header, header_size),
        slice::from_raw_parts(body, body_size),
    ) {
        Ok(bank) => bank,
        Err(_) => return INVALID,
    };
    if bank.header.master_volume > midi::DATA_MASK || bank.header.pan > midi::DATA_MASK {
        return INVALID;
    }
    // Initialize the caller's allocation directly: this bank is too large for
    // a temporary value on the browser's C/Rust stack.
    ptr::write_bytes(destination, 0, 1);
    let result = &mut *destination;
    result.volume = bank.header.master_volume;
    result.pan = bank.header.pan;
    result.sample_count = bank.header.sample_count;
    let mut packed_program = 0;
    for slot in 0..VAB_PROGRAM_SLOTS {
        let program = bank.program_slot(slot).unwrap();
        if program.tone_count == 0 {
            continue;
        }
        if usize::from(program.tone_count) > VAB_TONES_PER_PROGRAM
            || program.master_volume > midi::DATA_MASK
            || program.pan > midi::DATA_MASK
        {
            return INVALID;
        }
        let target = &mut result.programs[slot];
        target.tone_count = program.tone_count;
        target.volume = program.master_volume;
        target.pan = program.pan;
        target.priority = program.priority;
        // Program numbers are sparse; occupied slots own consecutive tone blocks.
        for ordinal in 0..usize::from(program.tone_count) {
            let tone = match bank.tone(packed_program, ordinal) {
                Some(tone) => tone,
                None => return INVALID,
            };
            if tone.program != slot as i16
                || tone.sample < 1
                || tone.sample as u16 > bank.header.sample_count
                || tone.volume > midi::DATA_MASK
                || tone.pan > midi::DATA_MASK
                || tone.center_shift > midi::DATA_MASK
                || tone.minimum_note > tone.maximum_note
                || tone.maximum_note > midi::DATA_MASK
                || tone.mode & !TONE_REVERB_FLAG != 0
            {
                return INVALID;
            }
            target.tones[ordinal] = Tone {
                priority: tone.priority,
                reverb: u8::from(tone.mode & TONE_REVERB_FLAG != 0),
                volume: tone.volume,
                pan: tone.pan,
                // This is a pitch reference, not a MIDI note-on. Retail banks
                // use values above 127 (B1 program 19 has center note 150).
                center_note: tone.center_note,
                center_shift: tone.center_shift,
                minimum_note: tone.minimum_note,
                maximum_note: tone.maximum_note,
                vibrato_width: tone.vibrato_width,
                vibrato_time: tone.vibrato_time,
                portamento_width: tone.portamento_width,
                portamento_time: tone.portamento_time,
                bend_down: tone.pitch_bend_minimum,
                bend_up: tone.pitch_bend_maximum,
                sample_index: tone.sample as u16 - 1,
                envelope: Envelope {
                    attack_shift: ((tone.adsr1 >> ADSR_ATTACK_SHIFT_OFFSET) & ADSR_RATE_SHIFT_MASK)
                        as u8,
                    attack_step: ((tone.adsr1 >> ADSR_ATTACK_STEP_OFFSET) & ADSR_STEP_MASK) as u8,
                    decay_shift: ((tone.adsr1 >> ADSR_DECAY_SHIFT_OFFSET) & ADSR_DECAY_SHIFT_MASK)
                        as u8,
                    sustain_shift: ((tone.adsr2 >> ADSR_SUSTAIN_SHIFT_OFFSET)
                        & ADSR_RATE_SHIFT_MASK) as u8,
                    sustain_step: ((tone.adsr2 >> ADSR_SUSTAIN_STEP_OFFSET) & ADSR_STEP_MASK) as u8,
                    release_shift: (tone.adsr2 & ADSR_RATE_SHIFT_MASK) as u8,
                    sustain_level: (((u32::from(tone.adsr1) & ADSR_SUSTAIN_LEVEL_MASK) + 1)
                        * ADSR_SUSTAIN_LEVEL_STEP)
                        .min(ENVELOPE_LEVEL_MAX) as u16,
                    attack_exponential: (tone.adsr1 >> ADSR_EXPONENTIAL_OFFSET) as u8,
                    sustain_exponential: (tone.adsr2 >> ADSR_EXPONENTIAL_OFFSET) as u8,
                    sustain_decreasing: ((tone.adsr2 >> ADSR_SUSTAIN_DIRECTION_OFFSET) & 1) as u8,
                    release_exponential: ((tone.adsr2 >> ADSR_RELEASE_EXPONENTIAL_OFFSET) & 1)
                        as u8,
                },
            };
        }
        packed_program += 1;
    }
    if packed_program != usize::from(bank.header.program_count) {
        return INVALID;
    }
    for sample in bank.samples() {
        result.samples[usize::from(sample.index)] = SampleRange {
            offset: sample.offset as u32,
            size: sample.data.len() as u32,
        };
    }
    OK
}

#[repr(C)]
pub struct Predictor {
    previous: i32,
    older: i32,
}

#[no_mangle]
pub unsafe extern "C" fn kf_audio_sample_info(
    data: *const u8,
    size: usize,
    info: *mut SampleInfo,
) -> i32 {
    if !input(data, size)
        || size == 0
        || size % ADPCM_BLOCK_BYTES != 0
        || size / ADPCM_BLOCK_BYTES > u32::MAX as usize / ADPCM_BLOCK_FRAMES
        || !output(info)
    {
        return INVALID;
    }
    let mut frames = 0;
    let mut loop_begin = 0;
    for block in slice::from_raw_parts(data, size).chunks_exact(ADPCM_BLOCK_BYTES) {
        if block[0] >> ADPCM_FILTER_OFFSET > ADPCM_FILTER_LAST || block[1] & !ADPCM_FLAGS_MASK != 0
        {
            return INVALID;
        }
        if block[1] & ADPCM_LOOP_START != 0 {
            loop_begin = frames;
        }
        frames += ADPCM_BLOCK_FRAMES as u32;
        if block[1] & ADPCM_END != 0 {
            info.write(SampleInfo {
                frames,
                loop_begin,
                loop_end: if block[1] & ADPCM_REPEAT != 0 {
                    frames
                } else {
                    0
                },
            });
            return OK;
        }
    }
    info.write(SampleInfo {
        frames,
        loop_begin: 0,
        loop_end: 0,
    });
    OK
}

#[no_mangle]
pub unsafe extern "C" fn kf_audio_decode_block(
    data: *const u8,
    size: usize,
    predictor: *mut Predictor,
    pcm: *mut i16,
    capacity: usize,
) -> i32 {
    if !input(data, size) || size != ADPCM_BLOCK_BYTES || !output(predictor) || !output(pcm) {
        return INVALID;
    }
    if capacity < ADPCM_BLOCK_FRAMES {
        return OUTPUT_FULL;
    }
    let block = slice::from_raw_parts(data, ADPCM_BLOCK_BYTES);
    const FILTERS: [(i32, i32); 5] = [(0, 0), (60, 0), (115, -52), (98, -55), (122, -60)];
    let filter = usize::from(block[0] >> ADPCM_FILTER_OFFSET);
    if filter >= FILTERS.len() || block[1] & !ADPCM_FLAGS_MASK != 0 {
        return INVALID;
    }
    let shift = match block[0] & ADPCM_SHIFT_MASK {
        0..=ADPCM_MAX_SHIFT => block[0] & ADPCM_SHIFT_MASK,
        _ => ADPCM_RESERVED_SHIFT,
    };
    let state = &mut *predictor;
    if !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.previous)
        || !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.older)
    {
        return INVALID;
    }
    let (positive, negative) = FILTERS[filter];
    for n in 0..ADPCM_BLOCK_FRAMES {
        let nibble = ((block[ADPCM_HEADER_BYTES + n / 2] >> ((n & 1) * ADPCM_NIBBLE_BITS))
            & ADPCM_NIBBLE_MASK) as i32;
        let signed = if nibble < ADPCM_NIBBLE_SIGN_BIT {
            nibble
        } else {
            nibble - ADPCM_NIBBLE_MODULUS
        };
        let value = ((signed * ADPCM_SAMPLE_SCALE) >> shift)
            + ((state.previous * positive + state.older * negative + ADPCM_PREDICTOR_ROUNDING)
                >> ADPCM_PREDICTOR_FRACTION_BITS);
        let value = value.clamp(i32::from(i16::MIN), i32::from(i16::MAX));
        pcm.add(n).write(value as i16);
        state.older = state.previous;
        state.previous = value;
    }
    OK
}

#[no_mangle]
pub unsafe extern "C" fn kf_music_decode(
    data: *const u8,
    size: usize,
    events: *mut MusicEvent,
    capacity: usize,
    info: *mut MusicInfo,
) -> i32 {
    if !input(data, size)
        || !output(info)
        || (!events.is_null() && !output(events))
        || (events.is_null() && capacity != 0)
        || capacity > isize::MAX as usize / core::mem::size_of::<MusicEvent>()
    {
        return INVALID;
    }
    let sequence = match Sequence::parse(slice::from_raw_parts(data, size)) {
        Ok(sequence) => sequence,
        Err(_) => return INVALID,
    };
    if sequence.header.version != SUPPORTED_SEQ_VERSION
        || sequence.header.resolution == 0
        || sequence.header.tempo == 0
    {
        return INVALID;
    }
    let mut count = 0;
    let mut duration = 0u64;
    for parsed in sequence.events() {
        let event = match parsed {
            Ok(event) => event,
            Err(_) => return INVALID,
        };
        let mut result = MusicEvent {
            delta: event.delta,
            kind: MusicEventKind::NoteOff as u32,
            value: 0,
            channel: 0,
            note: 0,
            velocity: 0,
            reserved: 0,
        };
        duration += u64::from(event.delta);
        match event.kind {
            SeqEventKind::Channel(channel) => {
                result.channel = channel.channel;
                match channel.message {
                    ChannelMessage::NoteOff | ChannelMessage::NoteOn => {
                        result.velocity = channel.data2.unwrap_or(0);
                        result.kind =
                            if channel.message == ChannelMessage::NoteOn && result.velocity != 0 {
                                MusicEventKind::NoteOn as u32
                            } else {
                                MusicEventKind::NoteOff as u32
                            };
                        result.note = channel.data1;
                    }
                    ChannelMessage::ControlChange if channel.data1 == midi::CHANNEL_VOLUME => {
                        result.kind = MusicEventKind::Volume as u32;
                        result.value = u32::from(channel.data2.unwrap_or(0));
                    }
                    ChannelMessage::ProgramChange => {
                        result.kind = MusicEventKind::Program as u32;
                        result.value = u32::from(channel.data1);
                    }
                    ChannelMessage::PitchBend => {
                        result.kind = MusicEventKind::PitchBend as u32;
                        result.value = u32::from(channel.data1)
                            | (u32::from(channel.data2.unwrap_or(0)) << midi::DATA_BITS);
                    }
                    _ => return INVALID,
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_TEMPO,
                data,
            } if data.len() == 3 => {
                result.kind = MusicEventKind::Tempo as u32;
                result.value =
                    (u32::from(data[0]) << 16) | (u32::from(data[1]) << 8) | u32::from(data[2]);
                if result.value == 0 {
                    return INVALID;
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_END,
                data,
            } if data.is_empty() => {
                result.kind = MusicEventKind::End as u32;
            }
            _ => return INVALID,
        }
        if !events.is_null() {
            if count == capacity {
                return OUTPUT_FULL;
            }
            events.add(count).write(result);
        }
        count += 1;
        if matches!(
            event.kind,
            SeqEventKind::Meta {
                meta_type: midi::META_END,
                ..
            }
        ) {
            if duration == 0 {
                return INVALID;
            }
            info.write(MusicInfo {
                resolution: u32::from(sequence.header.resolution),
                tempo: sequence.header.tempo,
                event_count: count as u32,
            });
            return OK;
        }
    }
    INVALID
}
