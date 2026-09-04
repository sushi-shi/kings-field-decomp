//! Host transport for testing the allocation-free codec library.
//!
//! Requests and responses use a little-endian block count followed by
//! `(u32 length, bytes)` for each block. Only this host executable uses std.

use std::io::{self, Read, Write};

#[path = "support/animation_driver.rs"]
mod animation_driver;
#[path = "support/audio_driver.rs"]
mod audio_driver;
#[path = "support/audio_vab_state_driver.rs"]
mod audio_vab_state_driver;
#[path = "support/map_resource_driver.rs"]
mod map_resource_driver;
#[path = "support/placement_driver.rs"]
mod placement_driver;
#[path = "support/save_driver.rs"]
mod save_driver;
#[path = "support/save_write_driver.rs"]
mod save_write_driver;
#[path = "support/stat_driver.rs"]
mod stat_driver;
#[path = "support/world_persist_driver.rs"]
mod world_persist_driver;
#[path = "support/world_state_driver.rs"]
mod world_state_driver;

fn decode_blocks(bytes: &[u8]) -> Result<Vec<Vec<u8>>, String> {
    let mut at = 0usize;
    let mut word = || -> Result<usize, String> {
        let end = at.checked_add(4).ok_or("frame offset overflow")?;
        let value = bytes.get(at..end).ok_or("truncated frame count")?;
        at = end;
        Ok(u32::from_le_bytes(value.try_into().unwrap()) as usize)
    };
    let count = word()?;
    if count > 128 {
        return Err("too many frame blocks".into());
    }
    let mut result = Vec::with_capacity(count);
    for _ in 0..count {
        let header_end = at.checked_add(4).ok_or("frame offset overflow")?;
        let header = bytes.get(at..header_end).ok_or("truncated block length")?;
        let size = u32::from_le_bytes(header.try_into().unwrap()) as usize;
        let end = header_end.checked_add(size).ok_or("block size overflow")?;
        result.push(
            bytes
                .get(header_end..end)
                .ok_or("truncated block")?
                .to_vec(),
        );
        at = end;
    }
    if at != bytes.len() {
        return Err("trailing bytes after request".into());
    }
    Ok(result)
}

fn encode_blocks(blocks: &[Vec<u8>]) -> io::Result<()> {
    let mut out = io::stdout().lock();
    out.write_all(&(blocks.len() as u32).to_le_bytes())?;
    for block in blocks {
        let length = u32::try_from(block.len()).map_err(io::Error::other)?;
        out.write_all(&length.to_le_bytes())?;
        out.write_all(block)?;
    }
    Ok(())
}

fn execute(operation: &str, mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    match operation {
        "world-persist" => world_persist_driver::execute(blocks),
        "save-write-slot" => save_write_driver::execute(blocks),
        "audio-vab" | "audio-seq" => audio_driver::execute(operation, blocks),
        "audio-vab-state" => audio_vab_state_driver::execute(blocks),
        "map-resources" => map_resource_driver::execute(blocks),
        "world-restore" => world_state_driver::execute(blocks),
        "common-load" => {
            if blocks.len() != 9 || blocks[8].len() != 4 {
                return Err("common-load expects COM, six destinations, registry, base u32".into());
            }
            let base = u32::from_le_bytes(blocks.pop().unwrap()[..].try_into().unwrap());
            let mut data = blocks.remove(0);
            let [render_windows, weapons, armor, magic, object_definitions, growth, registry] =
                &mut blocks[..]
            else {
                unreachable!()
            };
            let result = kf_codec::common::load(
                &mut data,
                base,
                kf_codec::common::Destinations {
                    render_windows,
                    weapons,
                    armor,
                    magic,
                    object_definitions,
                    growth,
                    registry,
                },
            )
            .map_err(|error| format!("{error:?}"))?;
            blocks.push(data);
            blocks.push(result.selected_tmd.to_le_bytes().to_vec());
            blocks.push(result.arena_cursor.to_le_bytes().to_vec());
            Ok(blocks)
        }
        "animation-bind" => animation_driver::execute(blocks),
        "animation-instance" => animation_driver::execute_instance(blocks),
        "animation-static" => animation_driver::execute_static(blocks),
        "save-restore" | "save-serialize" | "save-catalog" | "save-read-header"
        | "save-read-slot" | "save-initialize" => save_driver::execute(operation, blocks),
        "register-archive" | "register-asset" | "register-tmd" => {
            if blocks.len() != 3 || blocks[2].len() != 10 {
                return Err(
                    "register expects payload, initial table, and <u16 slot,u32 base,u32 selected>"
                        .into(),
                );
            }
            let params = blocks.pop().unwrap();
            let mut table = blocks.pop().unwrap();
            let mut payload = blocks.pop().unwrap();
            let slot = u16::from_le_bytes(params[..2].try_into().unwrap());
            let base = u32::from_le_bytes(params[2..6].try_into().unwrap());
            let mut selected = u32::from_le_bytes(params[6..10].try_into().unwrap());
            let register = match operation {
                "register-archive" => kf_codec::registry::register_archive,
                "register-asset" => kf_codec::registry::register_asset,
                "register-tmd" => kf_codec::registry::register_tmd,
                _ => unreachable!(),
            };
            register(&mut payload, slot, base, &mut table, &mut selected)
                .map_err(|error| format!("{error:?}"))?;
            Ok(vec![payload, table, selected.to_le_bytes().to_vec()])
        }
        "item-database" => stat_driver::execute(blocks),
        "placements-items" | "placements-actors" | "placements-events" | "placements-objects" => {
            placement_driver::execute(operation, blocks)
        }
        "records-render" | "records-weapons" | "records-armor" | "records-magic"
        | "records-objects" | "records-growth" | "records-actors" => {
            if blocks.len() != 2 {
                return Err("records expects source and initial destination".into());
            }
            let mut destination = blocks.pop().unwrap();
            let source = &blocks[0];
            use kf_codec::records;
            let loader = match operation {
                "records-render" => records::load_render_cell_windows,
                "records-weapons" => records::load_weapon_records,
                "records-armor" => records::load_armor_records,
                "records-magic" => records::load_magic_records,
                "records-objects" => records::load_map_object_definitions,
                "records-growth" => records::load_player_level_growth,
                "records-actors" => records::load_actor_definitions,
                _ => unreachable!(),
            };
            loader(source, &mut destination).map_err(|error| format!("{error:?}"))?;
            Ok(vec![destination])
        }
        "stat" => {
            if blocks.len() != 7 {
                return Err("stat expects source and six initial destinations".into());
            }
            let source = blocks.remove(0);
            let [menu_assets, window_layouts, item_names, magic_names, buy_prices, sell_prices] =
                &mut blocks[..]
            else {
                unreachable!()
            };
            kf_codec::stat::load_stat_database(
                &source,
                kf_codec::stat::StatDestinations {
                    menu_assets,
                    window_layouts,
                    item_names,
                    magic_names,
                    buy_prices,
                    sell_prices,
                },
            )
            .map_err(|error| format!("{error:?}"))?;
            Ok(blocks)
        }
        "tmd" => {
            if blocks.len() != 2 || blocks[1].len() != 4 {
                return Err("tmd expects payload and a u32 TMD offset".into());
            }
            let offset = u32::from_le_bytes(blocks[1][..].try_into().unwrap()) as usize;
            let mut payload = blocks.remove(0);
            let tmd = payload
                .get_mut(offset..)
                .ok_or("TMD offset outside payload")?;
            kf_codec::tmd::prepare_primitive_indices(tmd).map_err(|error| error.to_string())?;
            Ok(vec![payload])
        }
        "chunks" => {
            if blocks.len() != 1 {
                return Err("chunks expects one stream".into());
            }
            kf_codec::chunked::chunks(&blocks[0])
                .map(|chunk| {
                    chunk
                        .map(|chunk| chunk.payload.to_vec())
                        .map_err(|e| e.to_string())
                })
                .collect()
        }
        "tim" | "tim-state" => {
            let base = if operation == "tim-state" {
                if blocks.len() != 2 || blocks[1].len() != 4 {
                    return Err("tim-state expects stream and PSX base u32".into());
                }
                u32::from_le_bytes(blocks.pop().unwrap().try_into().unwrap())
            } else {
                if blocks.len() != 1 {
                    return Err("tim expects one stream".into());
                }
                0
            };
            let mut output = Vec::new();
            let mut descriptors = Vec::new();
            for image in kf_codec::tim::Images::new(&blocks[0]) {
                let image = image.map_err(|error| error.to_string())?;
                let descriptor = image.psx_descriptor(base).to_le_bytes();
                for block in image.clut.into_iter().chain([image.image]) {
                    descriptors.extend_from_slice(&descriptor);
                    output.extend_from_slice(&block.rectangle.to_le_bytes());
                    output.extend_from_slice(&(block.pixels.len() as u32).to_le_bytes());
                    output.extend_from_slice(block.pixels);
                }
            }
            if operation == "tim-state" {
                Ok(vec![output, descriptors])
            } else {
                Ok(vec![output])
            }
        }
        _ => Err(format!("unknown codec operation {operation:?}")),
    }
}

fn main() {
    let result = (|| -> Result<(), String> {
        let operation = std::env::args().nth(1).ok_or("missing codec operation")?;
        let mut request = Vec::new();
        io::stdin()
            .take(64 * 1024 * 1024 + 1)
            .read_to_end(&mut request)
            .map_err(|error| error.to_string())?;
        if request.len() > 64 * 1024 * 1024 {
            return Err("request exceeds 64 MiB".into());
        }
        let response = execute(&operation, decode_blocks(&request)?)?;
        encode_blocks(&response).map_err(|error| error.to_string())
    })();
    if let Err(error) = result {
        eprintln!("kf-codec-oracle: {error}");
        std::process::exit(1);
    }
}
