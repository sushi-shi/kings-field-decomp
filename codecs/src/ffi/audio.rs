use super::bindings::*;
use super::{input_valid, output_valid, OK};
use crate::audio::midi;
use crate::audio::{ChannelMessage, SeqEventKind, Sequence, VabBank};
use crate::audio::{VAB_PROGRAM_SLOTS, VAB_TONES_PER_PROGRAM};
use crate::bytes::read_u24_be;
use crate::cast::{AsU64, AsUsize};
use core::{ptr, slice};

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
const ADPCM_BLOCK_BYTES: usize = crate::cast::u32_to_usize(KF_AUDIO_ADPCM_BLOCK_BYTES);
const ADPCM_BLOCK_FRAMES: usize = crate::cast::u32_to_usize(KF_AUDIO_ADPCM_BLOCK_FRAMES);
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

fn input(data: *const u8, size: usize) -> bool {
    input_valid(data, size) && size <= u32::MAX.as_usize()
}
#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_audio_bank_decode(
    header: *const u8,
    header_size: usize,
    body: *const u8,
    body_size: usize,
    destination: *mut KfAudioBankData,
) -> KfCodecResult {
    if !input(header, header_size) || !input(body, body_size) || !output_valid(destination, 1) {
        return crate::Error::invalid("invalid audio bank pointer or size").into();
    }
    let bank = match VabBank::parse(
        slice::from_raw_parts(header, header_size),
        slice::from_raw_parts(body, body_size),
    ) {
        Ok(bank) => bank,
        Err(error) => return error.into(),
    };
    if bank.header.master_volume > midi::DATA_MASK || bank.header.pan > midi::DATA_MASK {
        return crate::Error::invalid("invalid VAB volume or pan").into();
    }
    // Initialize the caller's allocation directly: this bank is too large for
    // a temporary value on the browser's C/Rust stack.
    ptr::write_bytes(destination, 0, 1);
    let result = &mut *destination;
    result.volume = bank.header.master_volume;
    result.pan = bank.header.pan;
    result.sample_count = bank.header.sample_count.get();
    let mut packed_program = 0;
    for slot in 0..VAB_PROGRAM_SLOTS {
        let program = match bank.program_slot(slot) {
            Ok(program) => program,
            Err(error) => return error.into(),
        };
        if program.tone_count == 0 {
            continue;
        }
        if program.tone_count.as_usize() > VAB_TONES_PER_PROGRAM
            || program.master_volume > midi::DATA_MASK
            || program.pan > midi::DATA_MASK
        {
            return crate::Error::invalid("invalid VAB program").into();
        }
        let target = &mut result.programs[slot];
        target.tone_count = program.tone_count;
        target.volume = program.master_volume;
        target.pan = program.pan;
        target.priority = program.priority;
        // Program numbers are sparse; occupied slots own consecutive tone blocks.
        for ordinal in 0..program.tone_count.as_usize() {
            let tone = match bank.tone(packed_program, ordinal) {
                Ok(tone) => tone,
                Err(error) => return error.into(),
            };
            if tone.program.get() != slot as i16
                || tone.sample.get() < 1
                || tone.sample.get() as u16 > bank.header.sample_count.get()
                || tone.volume > midi::DATA_MASK
                || tone.pan > midi::DATA_MASK
                || tone.center_shift > midi::DATA_MASK
                || tone.minimum_note > tone.maximum_note
                || tone.maximum_note > midi::DATA_MASK
                || tone.mode & !TONE_REVERB_FLAG != 0
            {
                return crate::Error::invalid("invalid VAB tone").into();
            }
            target.tones[ordinal] = KfAudioTone {
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
                sample_index: tone.sample.get() as u16 - 1,
                envelope: KfAudioEnvelope {
                    attack_shift: ((tone.adsr1.get() >> ADSR_ATTACK_SHIFT_OFFSET)
                        & ADSR_RATE_SHIFT_MASK) as u8,
                    attack_step: ((tone.adsr1.get() >> ADSR_ATTACK_STEP_OFFSET) & ADSR_STEP_MASK)
                        as u8,
                    decay_shift: ((tone.adsr1.get() >> ADSR_DECAY_SHIFT_OFFSET)
                        & ADSR_DECAY_SHIFT_MASK) as u8,
                    sustain_shift: ((tone.adsr2.get() >> ADSR_SUSTAIN_SHIFT_OFFSET)
                        & ADSR_RATE_SHIFT_MASK) as u8,
                    sustain_step: ((tone.adsr2.get() >> ADSR_SUSTAIN_STEP_OFFSET) & ADSR_STEP_MASK)
                        as u8,
                    release_shift: (tone.adsr2.get() & ADSR_RATE_SHIFT_MASK) as u8,
                    sustain_level: (((u32::from(tone.adsr1.get()) & ADSR_SUSTAIN_LEVEL_MASK) + 1)
                        * ADSR_SUSTAIN_LEVEL_STEP)
                        .min(ENVELOPE_LEVEL_MAX) as u16,
                    attack_exponential: (tone.adsr1.get() >> ADSR_EXPONENTIAL_OFFSET) as u8,
                    sustain_exponential: (tone.adsr2.get() >> ADSR_EXPONENTIAL_OFFSET) as u8,
                    sustain_decreasing: ((tone.adsr2.get() >> ADSR_SUSTAIN_DIRECTION_OFFSET) & 1)
                        as u8,
                    release_exponential: ((tone.adsr2.get() >> ADSR_RELEASE_EXPONENTIAL_OFFSET) & 1)
                        as u8,
                },
            };
        }
        packed_program += 1;
    }
    if packed_program != bank.header.program_count.get().as_usize() {
        return crate::Error::invalid("VAB program count mismatch").into();
    }
    for sample in bank.samples() {
        let sample = match sample {
            Ok(sample) => sample,
            Err(error) => return error.into(),
        };
        result.samples[sample.index.as_usize()] = KfAudioSampleRange {
            offset: sample.offset as u32,
            size: sample.data.len() as u32,
        };
    }
    OK
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_audio_sample_info(
    data: *const u8,
    size: usize,
    info: *mut KfAudioSampleInfo,
) -> KfCodecResult {
    if !input(data, size)
        || size == 0
        || size % ADPCM_BLOCK_BYTES != 0
        || size / ADPCM_BLOCK_BYTES > u32::MAX.as_usize() / ADPCM_BLOCK_FRAMES
        || !output_valid(info, 1)
    {
        return crate::Error::invalid("invalid ADPCM pointer or size").into();
    }
    let mut frames = 0;
    let mut loop_begin = 0;
    for block in slice::from_raw_parts(data, size).chunks_exact(ADPCM_BLOCK_BYTES) {
        if block[0] >> ADPCM_FILTER_OFFSET > ADPCM_FILTER_LAST || block[1] & !ADPCM_FLAGS_MASK != 0
        {
            return crate::Error::invalid("invalid ADPCM filter or flags").into();
        }
        if block[1] & ADPCM_LOOP_START != 0 {
            loop_begin = frames;
        }
        frames += ADPCM_BLOCK_FRAMES as u32;
        if block[1] & ADPCM_END != 0 {
            info.write(KfAudioSampleInfo {
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
    info.write(KfAudioSampleInfo {
        frames,
        loop_begin: 0,
        loop_end: 0,
    });
    OK
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_audio_decode_block(
    data: *const u8,
    size: usize,
    predictor: *mut KfAudioPredictor,
    pcm: *mut i16,
    capacity: usize,
) -> KfCodecResult {
    if !input(data, size)
        || size != ADPCM_BLOCK_BYTES
        || !output_valid(predictor, 1)
        || !output_valid(pcm, 1)
    {
        return crate::Error::invalid("invalid ADPCM block pointer or size").into();
    }
    if capacity < ADPCM_BLOCK_FRAMES {
        return crate::Error::output_full().into();
    }
    let block = slice::from_raw_parts(data, ADPCM_BLOCK_BYTES);
    const FILTERS: [(i32, i32); 5] = [(0, 0), (60, 0), (115, -52), (98, -55), (122, -60)];
    let filter = (block[0] >> ADPCM_FILTER_OFFSET).as_usize();
    if filter >= FILTERS.len() || block[1] & !ADPCM_FLAGS_MASK != 0 {
        return crate::Error::invalid("invalid ADPCM filter or flags").into();
    }
    let shift = match block[0] & ADPCM_SHIFT_MASK {
        0..=ADPCM_MAX_SHIFT => block[0] & ADPCM_SHIFT_MASK,
        _ => ADPCM_RESERVED_SHIFT,
    };
    let state = &mut *predictor;
    if !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.previous)
        || !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.older)
    {
        return crate::Error::invalid("invalid ADPCM predictor").into();
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

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_music_decode(
    data: *const u8,
    size: usize,
    events: *mut KfMusicEvent,
    capacity: usize,
    info: *mut KfMusicInfo,
) -> KfCodecResult {
    if !input(data, size)
        || !output_valid(info, 1)
        || (!events.is_null() && !output_valid(events, capacity))
        || (events.is_null() && capacity != 0)
    {
        return crate::Error::invalid("invalid music pointer or size").into();
    }
    let sequence = match Sequence::parse(slice::from_raw_parts(data, size)) {
        Ok(sequence) => sequence,
        Err(error) => return error.into(),
    };
    if sequence.header.version.get() != SUPPORTED_SEQ_VERSION
        || sequence.header.resolution.get() == 0
        || sequence.header.tempo.get() == 0
    {
        return crate::Error::invalid("unsupported SEQ header").into();
    }
    let mut count = 0;
    let mut duration = 0u64;
    for parsed in sequence.events() {
        let event = match parsed {
            Ok(event) => event,
            Err(error) => return error.into(),
        };
        let mut result = KfMusicEvent {
            delta: event.delta,
            kind: KF_MUSIC_NOTE_OFF,
            value: 0,
            channel: 0,
            note: 0,
            velocity: 0,
            reserved: 0,
        };
        duration += event.delta.as_u64();
        match event.kind {
            SeqEventKind::Channel(channel) => {
                result.channel = channel.channel;
                match channel.message {
                    ChannelMessage::NoteOff | ChannelMessage::NoteOn => {
                        result.velocity = channel.data2.unwrap_or(0);
                        result.kind =
                            if channel.message == ChannelMessage::NoteOn && result.velocity != 0 {
                                KF_MUSIC_NOTE_ON
                            } else {
                                KF_MUSIC_NOTE_OFF
                            };
                        result.note = channel.data1;
                    }
                    ChannelMessage::ControlChange if channel.data1 == midi::CHANNEL_VOLUME => {
                        result.kind = KF_MUSIC_VOLUME;
                        result.value = u32::from(channel.data2.unwrap_or(0));
                    }
                    ChannelMessage::ProgramChange => {
                        result.kind = KF_MUSIC_PROGRAM;
                        result.value = u32::from(channel.data1);
                    }
                    ChannelMessage::PitchBend => {
                        result.kind = KF_MUSIC_PITCH_BEND;
                        result.value = u32::from(channel.data1)
                            | (u32::from(channel.data2.unwrap_or(0)) << midi::DATA_BITS);
                    }
                    _ => return crate::Error::invalid("unsupported SEQ channel message").into(),
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_TEMPO,
                data,
            } if data.len() == 3 => {
                result.kind = KF_MUSIC_TEMPO;
                result.value = match read_u24_be(data, 0) {
                    Ok(tempo) => tempo,
                    Err(error) => return error.into(),
                };
                if result.value == 0 {
                    return crate::Error::invalid("zero SEQ tempo").into();
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_END,
                data,
            } if data.is_empty() => {
                result.kind = KF_MUSIC_END;
            }
            _ => return crate::Error::invalid("unsupported SEQ event").into(),
        }
        if !events.is_null() {
            if count == capacity {
                return crate::Error::output_full().into();
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
                return crate::Error::invalid("zero SEQ duration").into();
            }
            info.write(KfMusicInfo {
                resolution: u32::from(sequence.header.resolution.get()),
                tempo: sequence.header.tempo.get(),
                event_count: count as u32,
            });
            return OK;
        }
    }
    crate::Error::invalid("SEQ is missing an end event").into()
}
