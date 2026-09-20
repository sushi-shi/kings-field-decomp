use crate::{INVALID, OK, OUTPUT_FULL};
use core::{mem::align_of, ptr, slice};
use kf_codec::audio::{ChannelMessage, SeqEventKind, Sequence, VabBank};

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
    tones: [Tone; 16],
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
    programs: [Program; 128],
    samples: [SampleRange; 256],
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
    if bank.header.master_volume > 127 || bank.header.pan > 127 {
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
    for slot in 0..128 {
        let program = bank.program_slot(slot).unwrap();
        if program.tone_count == 0 {
            continue;
        }
        if program.tone_count > 16 || program.master_volume > 127 || program.pan > 127 {
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
                || tone.volume > 127
                || tone.pan > 127
                || tone.center_shift > 127
                || tone.minimum_note > tone.maximum_note
                || tone.maximum_note > 127
                || tone.mode & !4 != 0
            {
                return INVALID;
            }
            target.tones[ordinal] = Tone {
                priority: tone.priority,
                reverb: u8::from(tone.mode & 4 != 0),
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
                    attack_shift: ((tone.adsr1 >> 10) & 31) as u8,
                    attack_step: ((tone.adsr1 >> 8) & 3) as u8,
                    decay_shift: ((tone.adsr1 >> 4) & 15) as u8,
                    sustain_shift: ((tone.adsr2 >> 8) & 31) as u8,
                    sustain_step: ((tone.adsr2 >> 6) & 3) as u8,
                    release_shift: (tone.adsr2 & 31) as u8,
                    sustain_level: (((u32::from(tone.adsr1) & 15) + 1) * 2048).min(32767) as u16,
                    attack_exponential: (tone.adsr1 >> 15) as u8,
                    sustain_exponential: (tone.adsr2 >> 15) as u8,
                    sustain_decreasing: ((tone.adsr2 >> 14) & 1) as u8,
                    release_exponential: ((tone.adsr2 >> 5) & 1) as u8,
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
        || size % 16 != 0
        || size / 16 > u32::MAX as usize / 28
        || !output(info)
    {
        return INVALID;
    }
    let mut frames = 0;
    let mut loop_begin = 0;
    for block in slice::from_raw_parts(data, size).chunks_exact(16) {
        if block[0] >> 4 > 4 || block[1] & !7 != 0 {
            return INVALID;
        }
        if block[1] & 4 != 0 {
            loop_begin = frames;
        }
        frames += 28;
        if block[1] & 1 != 0 {
            info.write(SampleInfo {
                frames,
                loop_begin,
                loop_end: if block[1] & 2 != 0 { frames } else { 0 },
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
    if !input(data, size) || size != 16 || !output(predictor) || !output(pcm) {
        return INVALID;
    }
    if capacity < 28 {
        return OUTPUT_FULL;
    }
    let block = slice::from_raw_parts(data, 16);
    const FILTERS: [(i32, i32); 5] = [(0, 0), (60, 0), (115, -52), (98, -55), (122, -60)];
    let filter = usize::from(block[0] >> 4);
    if filter >= FILTERS.len() || block[1] & !7 != 0 {
        return INVALID;
    }
    let shift = match block[0] & 15 {
        0..=12 => block[0] & 15,
        _ => 9,
    };
    let state = &mut *predictor;
    if !(-32768..=32767).contains(&state.previous) || !(-32768..=32767).contains(&state.older) {
        return INVALID;
    }
    let (positive, negative) = FILTERS[filter];
    for n in 0..28 {
        let nibble = ((block[2 + n / 2] >> ((n & 1) * 4)) & 15) as i32;
        let signed = if nibble < 8 { nibble } else { nibble - 16 };
        let value = ((signed * 4096) >> shift)
            + ((state.previous * positive + state.older * negative + 32) >> 6);
        let value = value.clamp(-32768, 32767);
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
    if sequence.header.version != 1 || sequence.header.resolution == 0 || sequence.header.tempo == 0
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
            kind: 0,
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
                        result.kind = u32::from(
                            channel.message == ChannelMessage::NoteOn && result.velocity != 0,
                        );
                        result.note = channel.data1;
                    }
                    ChannelMessage::ControlChange if channel.data1 == 7 => {
                        result.kind = 2;
                        result.value = u32::from(channel.data2.unwrap_or(0));
                    }
                    ChannelMessage::ProgramChange => {
                        result.kind = 3;
                        result.value = u32::from(channel.data1);
                    }
                    ChannelMessage::PitchBend => {
                        result.kind = 4;
                        result.value =
                            u32::from(channel.data1) | (u32::from(channel.data2.unwrap_or(0)) << 7);
                    }
                    _ => return INVALID,
                }
            }
            SeqEventKind::Meta {
                meta_type: 0x51,
                data,
            } if data.len() == 3 => {
                result.kind = 5;
                result.value =
                    (u32::from(data[0]) << 16) | (u32::from(data[1]) << 8) | u32::from(data[2]);
                if result.value == 0 {
                    return INVALID;
                }
            }
            SeqEventKind::Meta {
                meta_type: 0x2f,
                data,
            } if data.is_empty() => {
                result.kind = 6;
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
                meta_type: 0x2f,
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
