use super::bindings::*;
use super::{input_valid, output_valid, status};
use crate::audio::midi;
use crate::audio::{
    AdpcmFlags, AdpcmHeader, ChannelMessage, SeqEventKind, Sequence, ToneFlags, VabBank,
};
use crate::audio::{VAB_PROGRAM_SLOTS, VAB_TONES_PER_PROGRAM};
use crate::bytes::read_u24_be;
use crate::cast::{AsU64, AsUsize};
use core::{ptr, slice};

const SUPPORTED_SEQ_VERSION: u32 = 1;
const ADSR_SUSTAIN_LEVEL_STEP: u16 = 2048;
const ENVELOPE_LEVEL_MAX: u16 = 32767;
const ADPCM_BLOCK_BYTES: usize = crate::cast::u32_to_usize(KF_AUDIO_ADPCM_BLOCK_BYTES);
const ADPCM_BLOCK_FRAMES: usize = crate::cast::u32_to_usize(KF_AUDIO_ADPCM_BLOCK_FRAMES);
const ADPCM_HEADER_BYTES: usize = 2;
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
    status(audio_bank_decode(
        header,
        header_size,
        body,
        body_size,
        destination,
    ))
}

unsafe fn audio_bank_decode(
    header: *const u8,
    header_size: usize,
    body: *const u8,
    body_size: usize,
    destination: *mut KfAudioBankData,
) -> crate::Result<()> {
    if !input(header, header_size) || !input(body, body_size) || !output_valid(destination, 1) {
        crate::bail!("invalid audio bank pointer or size");
    }
    let bank = VabBank::parse(
        slice::from_raw_parts(header, header_size),
        slice::from_raw_parts(body, body_size),
    )?;
    if bank.header.master_volume > midi::DATA_MASK || bank.header.pan > midi::DATA_MASK {
        crate::bail!("invalid VAB volume or pan");
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
        let program = bank.program_slot(slot.as_usize())?;
        if program.tone_count == 0 {
            continue;
        }
        if u16::from(program.tone_count) > VAB_TONES_PER_PROGRAM
            || program.master_volume > midi::DATA_MASK
            || program.pan > midi::DATA_MASK
        {
            crate::bail!("invalid VAB program");
        }
        let target = &mut result.programs[slot.as_usize()];
        target.tone_count = program.tone_count;
        target.volume = program.master_volume;
        target.pan = program.pan;
        target.priority = program.priority;
        // Program numbers are sparse; occupied slots own consecutive tone blocks.
        for ordinal in 0..program.tone_count.as_usize() {
            let tone = bank.tone(packed_program, ordinal)?;
            let sample = u16::try_from(tone.sample.get())?;
            let flags = ToneFlags::from_bits(tone.mode)
                .ok_or(crate::Error::invalid("unknown VAB tone flags"))?;
            if i32::from(tone.program.get()) != i32::from(slot)
                || sample < 1
                || sample > bank.header.sample_count.get()
                || tone.volume > midi::DATA_MASK
                || tone.pan > midi::DATA_MASK
                || tone.center_shift > midi::DATA_MASK
                || tone.minimum_note > tone.maximum_note
                || tone.maximum_note > midi::DATA_MASK
            {
                crate::bail!("invalid VAB tone");
            }
            target.tones[ordinal] = KfAudioTone {
                priority: tone.priority,
                reverb: u8::from(flags.contains(ToneFlags::REVERB)),
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
                sample_index: sample - 1,
                envelope: KfAudioEnvelope {
                    attack_shift: tone.adsr1.attack_shift(),
                    attack_step: tone.adsr1.attack_step(),
                    decay_shift: tone.adsr1.decay_shift(),
                    sustain_shift: tone.adsr2.sustain_shift(),
                    sustain_step: tone.adsr2.sustain_step(),
                    release_shift: tone.adsr2.release_shift(),
                    sustain_level: ((u16::from(tone.adsr1.sustain_level()) + 1)
                        * ADSR_SUSTAIN_LEVEL_STEP)
                        .min(ENVELOPE_LEVEL_MAX),
                    attack_exponential: tone.adsr1.attack_exponential(),
                    sustain_exponential: tone.adsr2.sustain_exponential(),
                    sustain_decreasing: tone.adsr2.sustain_decreasing(),
                    release_exponential: tone.adsr2.release_exponential(),
                },
            };
        }
        packed_program += 1;
    }
    if packed_program != bank.header.program_count.get().as_usize() {
        crate::bail!("VAB program count mismatch");
    }
    for sample in bank.samples() {
        let sample = sample?;
        result.samples[sample.index.as_usize()] = KfAudioSampleRange {
            offset: sample.offset,
            size: sample.size,
        };
    }
    Ok(())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_audio_sample_info(
    data: *const u8,
    size: usize,
    info: *mut KfAudioSampleInfo,
) -> KfCodecResult {
    status(audio_sample_info(data, size, info))
}

unsafe fn audio_sample_info(
    data: *const u8,
    size: usize,
    info: *mut KfAudioSampleInfo,
) -> crate::Result<()> {
    if !input(data, size)
        || size == 0
        || size % ADPCM_BLOCK_BYTES != 0
        || size / ADPCM_BLOCK_BYTES > u32::MAX.as_usize() / ADPCM_BLOCK_FRAMES
        || !output_valid(info, 1)
    {
        crate::bail!("invalid ADPCM pointer or size");
    }
    let mut frames = 0;
    let mut loop_begin = 0;
    for block in slice::from_raw_parts(data, size).chunks_exact(ADPCM_BLOCK_BYTES) {
        let header = AdpcmHeader::parse(block)?;
        if header.flags.contains(AdpcmFlags::LOOP_START) {
            loop_begin = frames;
        }
        frames += KF_AUDIO_ADPCM_BLOCK_FRAMES;
        if header.flags.contains(AdpcmFlags::END) {
            info.write(KfAudioSampleInfo {
                frames,
                loop_begin,
                loop_end: if header.flags.contains(AdpcmFlags::REPEAT) {
                    frames
                } else {
                    0
                },
            });
            return Ok(());
        }
    }
    info.write(KfAudioSampleInfo {
        frames,
        loop_begin: 0,
        loop_end: 0,
    });
    Ok(())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_audio_decode_block(
    data: *const u8,
    size: usize,
    predictor: *mut KfAudioPredictor,
    pcm: *mut i16,
    capacity: usize,
) -> KfCodecResult {
    status(audio_decode_block(data, size, predictor, pcm, capacity))
}

unsafe fn audio_decode_block(
    data: *const u8,
    size: usize,
    predictor: *mut KfAudioPredictor,
    pcm: *mut i16,
    capacity: usize,
) -> crate::Result<()> {
    if !input(data, size)
        || size != ADPCM_BLOCK_BYTES
        || !output_valid(predictor, 1)
        || !output_valid(pcm, 1)
    {
        crate::bail!("invalid ADPCM block pointer or size");
    }
    if capacity < ADPCM_BLOCK_FRAMES {
        return Err(crate::Error::output_full());
    }
    let block = slice::from_raw_parts(data, ADPCM_BLOCK_BYTES);
    let header = AdpcmHeader::parse(block)?;
    let state = &mut *predictor;
    if !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.previous)
        || !(i32::from(i16::MIN)..=i32::from(i16::MAX)).contains(&state.older)
    {
        crate::bail!("invalid ADPCM predictor");
    }
    let (positive, negative) = header.coefficients;
    for n in 0..ADPCM_BLOCK_FRAMES {
        let nibble = i32::from(
            (block[ADPCM_HEADER_BYTES + n / 2] >> ((n & 1) * ADPCM_NIBBLE_BITS))
                & ADPCM_NIBBLE_MASK,
        );
        let signed = if nibble < ADPCM_NIBBLE_SIGN_BIT {
            nibble
        } else {
            nibble - ADPCM_NIBBLE_MODULUS
        };
        let value = ((signed * ADPCM_SAMPLE_SCALE) >> header.shift)
            + ((state.previous * positive + state.older * negative + ADPCM_PREDICTOR_ROUNDING)
                >> ADPCM_PREDICTOR_FRACTION_BITS);
        let value = value.clamp(i32::from(i16::MIN), i32::from(i16::MAX));
        pcm.add(n).write(i16::try_from(value)?);
        state.older = state.previous;
        state.previous = value;
    }
    Ok(())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kf_music_decode(
    data: *const u8,
    size: usize,
    events: *mut KfMusicEvent,
    capacity: usize,
    info: *mut KfMusicInfo,
) -> KfCodecResult {
    status(music_decode(data, size, events, capacity, info))
}

unsafe fn music_decode(
    data: *const u8,
    size: usize,
    events: *mut KfMusicEvent,
    capacity: usize,
    info: *mut KfMusicInfo,
) -> crate::Result<()> {
    if !input(data, size)
        || !output_valid(info, 1)
        || (!events.is_null() && !output_valid(events, capacity))
        || (events.is_null() && capacity != 0)
    {
        crate::bail!("invalid music pointer or size");
    }
    let sequence = Sequence::parse(slice::from_raw_parts(data, size))?;
    if sequence.header.version.get() != SUPPORTED_SEQ_VERSION
        || sequence.header.resolution.get() == 0
        || sequence.header.tempo.get() == 0
    {
        crate::bail!("unsupported SEQ header");
    }
    let mut count = 0u32;
    let mut duration = 0u64;
    for parsed in sequence.events() {
        let event = parsed?;
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
                    _ => crate::bail!("unsupported SEQ channel message"),
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_TEMPO,
                data,
            } if data.len() == 3 => {
                result.kind = KF_MUSIC_TEMPO;
                result.value = read_u24_be(data, 0)?;
                if result.value == 0 {
                    crate::bail!("zero SEQ tempo");
                }
            }
            SeqEventKind::Meta {
                meta_type: midi::META_END,
                data,
            } if data.is_empty() => {
                result.kind = KF_MUSIC_END;
            }
            _ => crate::bail!("unsupported SEQ event"),
        }
        if !events.is_null() {
            if count.as_usize() == capacity {
                return Err(crate::Error::output_full());
            }
            events.add(count.as_usize()).write(result);
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
                crate::bail!("zero SEQ duration");
            }
            info.write(KfMusicInfo {
                resolution: u32::from(sequence.header.resolution.get()),
                tempo: sequence.header.tempo.get(),
                event_count: count,
            });
            return Ok(());
        }
    }
    Err(crate::Error::invalid("SEQ is missing an end event"))
}
