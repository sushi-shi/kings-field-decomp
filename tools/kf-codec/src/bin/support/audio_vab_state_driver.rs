//! Host transport for the allocation-free VAB runtime-state transformation.

use kf_codec::audio_vab_state::{load_vab_success, SonyVabRegions, VabLoadInputs, VabServiceCall};

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
