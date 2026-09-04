//! Host transport for the allocation-free VAB runtime-state transformation.

use kf_codec::audio_vab_state::{
    load_vab_runtime, load_vab_success, SonyVabRegions, VabLoadInputs, VabRuntimeCall,
    VabRuntimeInputs, VabServiceCall,
};

fn encode_calls(calls: &[VabServiceCall]) -> Vec<u8> {
    let mut output = Vec::with_capacity(calls.len() * 20);
    for call in calls {
        let (tag, args) = match *call {
            VabServiceCall::GetInTransfer => (1, [0, 0, 0, 0]),
            VabServiceCall::SetInTransfer(value) => (2, [value as u32, 0, 0, 0]),
            VabServiceCall::Malloc(size) => (3, [size, 0, 0, 0]),
            VabServiceCall::SetTransferMode(mode) => (4, [mode as u32, 0, 0, 0]),
            VabServiceCall::SetTransferStartAddress(address) => (5, [address, 0, 0, 0]),
            VabServiceCall::Read { source, size } => (6, [source, size, 0, 0]),
            VabServiceCall::IsTransferCompleted(mode) => (7, [mode as u32, 0, 0, 0]),
        };
        output.extend_from_slice(&u32::to_le_bytes(tag));
        for arg in args {
            output.extend_from_slice(&arg.to_le_bytes());
        }
    }
    output
}

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 16 || blocks[15].len() != 16 {
        return Err(
            "audio-vab-state expects VH, VB, GAME audio state, 12 Sony regions, and four u32 inputs"
                .into(),
        );
    }
    let params = blocks.pop().unwrap();
    let inputs = VabLoadInputs {
        vh_address: u32::from_le_bytes(params[0..4].try_into().unwrap()),
        vb_address: u32::from_le_bytes(params[4..8].try_into().unwrap()),
        spu_allocation: u32::from_le_bytes(params[8..12].try_into().unwrap()),
        in_transfer: i32::from_le_bytes(params[12..16].try_into().unwrap()),
    };

    let [vh, vb, audio, maximum_programs, open_bank_count, bank_status, vh_end_pointers, header_pointers, program_pointers, tone_pointers, spu_start_addresses, body_sizes, current_header_pointer, current_program_pointer, current_tone_pointer] =
        &mut blocks[..]
    else {
        unreachable!()
    };
    let result = load_vab_success(
        vh,
        vb,
        audio,
        SonyVabRegions {
            maximum_programs,
            open_bank_count,
            bank_status,
            vh_end_pointers,
            header_pointers,
            program_pointers,
            tone_pointers,
            spu_start_addresses,
            body_sizes,
            current_header_pointer,
            current_program_pointer,
            current_tone_pointer,
        },
        inputs,
    )
    .map_err(|error| format!("{error:?}"))?;

    let trace = encode_calls(&result.calls);
    let mut report = Vec::with_capacity(28);
    report.extend_from_slice(&(result.report.bank_id as i32).to_le_bytes());
    report.extend_from_slice(&u32::from(result.report.maximum_programs).to_le_bytes());
    report.extend_from_slice(&u32::from(result.report.dense_program_count).to_le_bytes());
    report.extend_from_slice(&result.report.tone_table_address.to_le_bytes());
    report.extend_from_slice(&result.report.length_table_address.to_le_bytes());
    report.extend_from_slice(&result.report.vh_end_address.to_le_bytes());
    report.extend_from_slice(&result.report.body_size.to_le_bytes());

    // VB is an immutable transfer source and is intentionally absent from the
    // response; every mutable input region remains in request order.
    blocks.remove(1);
    blocks.push(trace);
    blocks.push(report);
    Ok(blocks)
}

pub fn execute_runtime(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 18
        || blocks[15].len() != 16
        || blocks[16].len() != 1
        || blocks[17].len() != 12
    {
        return Err(
            "audio-vab-runtime expects the 16 state inputs, prefix byte, and three control words"
                .into(),
        );
    }
    let control = blocks.pop().unwrap();
    let mut prefix = blocks.pop().unwrap();
    let params = blocks.pop().unwrap();
    let word = |bytes: &[u8], at: usize| u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap());
    let inputs = VabRuntimeInputs {
        load: VabLoadInputs {
            vh_address: word(&params, 0),
            vb_address: word(&params, 4),
            spu_allocation: word(&params, 8),
            in_transfer: word(&params, 12) as i32,
        },
        read_result: if word(&control, 0) != 0 {
            Some(word(&control, 4))
        } else {
            None
        },
        incoming_bank_id: word(&control, 8) as i16,
    };
    let [vh, vb, audio, maximum_programs, open_bank_count, bank_status, vh_end_pointers, header_pointers, program_pointers, tone_pointers, spu_start_addresses, body_sizes, current_header_pointer, current_program_pointer, current_tone_pointer] =
        &mut blocks[..]
    else {
        unreachable!()
    };
    let mut trace = Vec::new();
    load_vab_runtime(
        vh,
        vb,
        audio
            .as_mut_slice()
            .try_into()
            .map_err(|_| "invalid GAME audio state size")?,
        SonyVabRegions {
            maximum_programs,
            open_bank_count,
            bank_status,
            vh_end_pointers,
            header_pointers,
            program_pointers,
            tone_pointers,
            spu_start_addresses,
            body_sizes,
            current_header_pointer,
            current_program_pointer,
            current_tone_pointer,
        },
        &mut prefix[0],
        inputs,
        |call| {
            let (tag, args) = match call {
                VabRuntimeCall::Spu(call) => {
                    trace.extend(encode_calls(&[call]));
                    return;
                }
                VabRuntimeCall::VSync => (8u32, [0, 0, 0, 0]),
                VabRuntimeCall::SequenceVolume { id, volume } => (
                    9,
                    [
                        id as i32 as u32,
                        volume as i32 as u32,
                        volume as i32 as u32,
                        0,
                    ],
                ),
                VabRuntimeCall::SequenceStop(id) => (10, [id as i32 as u32, 0, 0, 0]),
                VabRuntimeCall::SequenceClose(id) => (11, [id as i32 as u32, 0, 0, 0]),
                VabRuntimeCall::HeaderError => (12, [0, 0, 0, 0]),
                VabRuntimeCall::BodyError => (13, [0, 0, 0, 0]),
            };
            for word in [tag, args[0], args[1], args[2], args[3]] {
                trace.extend_from_slice(&word.to_le_bytes());
            }
        },
    )
    .map_err(|error| format!("{error:?}"))?;
    blocks.remove(1);
    blocks.push(trace);
    blocks.push(prefix);
    Ok(blocks)
}
