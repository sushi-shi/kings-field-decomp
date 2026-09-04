//! Host-only deterministic services for save codec differential tests.

use kf_codec::save::{self, *};

const TRACE_CLEAR: u8 = 1;
const TRACE_OPEN: u8 = 2;
const TRACE_SEEK: u8 = 3;
const TRACE_READ: u8 = 4;
const TRACE_CLOSE: u8 = 5;
const TRACE_ICON: u8 = 6;

struct Io {
    file: Vec<u8>,
    open: bool,
    results: Vec<i32>,
    next_result: usize,
    offset: usize,
    trace: Vec<u8>,
}

impl Io {
    fn new(file: Vec<u8>, scenario: &[u8]) -> Result<Self, String> {
        if scenario.len() < 5 {
            return Err("save I/O scenario needs open byte and result count".into());
        }
        let count = u32::from_le_bytes(scenario[1..5].try_into().unwrap()) as usize;
        if scenario.len() != 5 + count * 4 {
            return Err("save I/O scenario result list is truncated or has a tail".into());
        }
        let results = scenario[5..]
            .chunks_exact(4)
            .map(|word| i32::from_le_bytes(word.try_into().unwrap()))
            .collect();
        Ok(Self {
            file,
            open: scenario[0] != 0,
            results,
            next_result: 0,
            offset: 0,
            trace: Vec::new(),
        })
    }
}

impl SaveReadIo for Io {
    fn clear_events(&mut self) {
        self.trace.push(TRACE_CLEAR);
    }

    fn open_read(&mut self) -> bool {
        self.trace
            .extend_from_slice(&[TRACE_OPEN, u8::from(self.open)]);
        self.open
    }

    fn seek(&mut self, offset: usize) {
        self.offset = offset;
        self.trace.push(TRACE_SEEK);
        self.trace.extend_from_slice(&(offset as u32).to_le_bytes());
    }

    fn read(&mut self, destination: &mut [u8]) -> i32 {
        let returned = self
            .results
            .get(self.next_result)
            .copied()
            .unwrap_or(destination.len() as i32);
        self.next_result += 1;
        if returned > 0 {
            let count = usize::try_from(returned)
                .unwrap()
                .min(destination.len())
                .min(self.file.len().saturating_sub(self.offset));
            destination[..count].copy_from_slice(&self.file[self.offset..self.offset + count]);
        }
        self.trace.push(TRACE_READ);
        self.trace
            .extend_from_slice(&(destination.len() as u32).to_le_bytes());
        self.trace.extend_from_slice(&returned.to_le_bytes());
        returned
    }

    fn close(&mut self) {
        self.trace.push(TRACE_CLOSE);
    }
}

struct Icons {
    images: [Vec<u8>; 3],
    next: usize,
    trace: Vec<u8>,
}

impl IconSource for Icons {
    fn load_icon(&mut self, path: &[u8], image: &mut [u8; ICON_SOURCE_SIZE]) {
        self.trace.push(TRACE_ICON);
        self.trace.push(path.len() as u8);
        self.trace.extend_from_slice(path);
        let source = &self.images[self.next];
        self.next += 1;
        let count = source.len().min(ICON_SOURCE_SIZE);
        image[..count].copy_from_slice(&source[..count]);
    }
}

pub fn execute(operation: &str, mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    match operation {
        "save-restore" => {
            if blocks.len() != 5 {
                return Err("save-restore expects payload, player, world, auxiliary, magic".into());
            }
            let payload = blocks.remove(0);
            let [player, world, auxiliary, magic] = &mut blocks[..] else {
                unreachable!()
            };
            save::restore_payload(
                &payload,
                SaveRuntimeDestinations {
                    player_state: player,
                    world_state: world,
                    auxiliary_state: auxiliary,
                    magic_records: magic,
                },
            )
            .map_err(|error| format!("{error:?}"))?;
            Ok(blocks)
        }
        "save-serialize" => {
            if blocks.len() != 5 {
                return Err(
                    "save-serialize expects payload, player, world, auxiliary, magic".into(),
                );
            }
            let mut payload = blocks.remove(0);
            let [player, world, auxiliary, magic] = &blocks[..] else {
                unreachable!()
            };
            save::serialize_payload(
                SaveRuntimeSources {
                    player_state: player,
                    world_state: world,
                    auxiliary_state: auxiliary,
                    magic_records: magic,
                },
                &mut payload,
            )
            .map_err(|error| format!("{error:?}"))?;
            Ok(vec![payload])
        }
        "save-catalog" => {
            if blocks.len() != 3 || blocks[0].len() != 4 {
                return Err("save-catalog expects status, header, summaries".into());
            }
            let status = i32::from_le_bytes(blocks.remove(0).try_into().unwrap());
            let header = blocks.remove(0);
            let mut summaries = blocks.remove(0);
            let status = save::apply_catalog(status, &header, &mut summaries)
                .map_err(|error| format!("{error:?}"))?;
            Ok(vec![status.to_le_bytes().to_vec(), summaries])
        }
        "save-read-header" => {
            if blocks.len() != 3 {
                return Err("save-read-header expects initial header, file, scenario".into());
            }
            let mut header = blocks.remove(0);
            let file = blocks.remove(0);
            let scenario = blocks.remove(0);
            let mut io = Io::new(file, &scenario)?;
            let status =
                save::read_header(&mut header, &mut io).map_err(|error| format!("{error:?}"))?;
            Ok(vec![status.to_le_bytes().to_vec(), header, io.trace])
        }
        "save-read-slot" => {
            if blocks.len() != 9 || blocks[0].len() != 2 {
                return Err("save-read-slot expects slot, cached header, payload, player, world, auxiliary, magic, file, scenario".into());
            }
            let slot = i16::from_le_bytes(blocks.remove(0).try_into().unwrap());
            let header = blocks.remove(0);
            let mut payload = blocks.remove(0);
            let file = blocks.remove(4);
            let scenario = blocks.remove(4);
            let [player, world, auxiliary, magic] = &mut blocks[..] else {
                unreachable!()
            };
            let mut io = Io::new(file, &scenario)?;
            let status = save::read_slot(
                slot,
                &header,
                &mut payload,
                SaveRuntimeDestinations {
                    player_state: player,
                    world_state: world,
                    auxiliary_state: auxiliary,
                    magic_records: magic,
                },
                &mut io,
            )
            .map_err(|error| format!("{error:?}"))?;
            let mut output = vec![status.to_le_bytes().to_vec(), payload];
            output.append(&mut blocks);
            output.push(io.trace);
            Ok(output)
        }
        "save-initialize" => {
            if blocks.len() != 5 {
                return Err("save-initialize expects header, payload, and three icon files".into());
            }
            let mut header = blocks.remove(0);
            let mut payload = blocks.remove(0);
            let images: [Vec<u8>; 3] = blocks.try_into().unwrap();
            let mut icons = Icons {
                images,
                next: 0,
                trace: Vec::new(),
            };
            save::initialize_buffers(&mut header, &mut payload, &mut icons)
                .map_err(|error| format!("{error:?}"))?;
            Ok(vec![header, payload, icons.trace])
        }
        _ => Err(format!("unknown save operation {operation}")),
    }
}
