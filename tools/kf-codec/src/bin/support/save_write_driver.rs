//! Host-only write sink. No operation reaches the host filesystem.

use kf_codec::save::{
    self, IconSource, SaveFile, SaveRuntimeSources, SaveWriteIo, ICON_SOURCE_SIZE,
};

struct Writer {
    file: Vec<u8>,
    opens: std::vec::IntoIter<i32>,
    results: std::vec::IntoIter<i32>,
    icons: std::vec::IntoIter<Vec<u8>>,
    offset: usize,
    trace: Vec<u8>,
}

impl IconSource for Writer {
    fn load_icon(&mut self, path: &[u8], image: &mut [u8; ICON_SOURCE_SIZE]) {
        self.trace.extend_from_slice(&[6, path.len() as u8]);
        self.trace.extend_from_slice(path);
        let source = self.icons.next().expect("three icon inputs");
        assert!(source.len() <= image.len());
        image[..source.len()].copy_from_slice(&source);
    }
}

impl SaveWriteIo for Writer {
    fn clear_events(&mut self) {
        self.trace.push(1);
    }

    fn open(&mut self, file: SaveFile, flags: u32) -> i32 {
        let result = self.opens.next().expect("explicit open result");
        self.trace
            .extend_from_slice(&[2, u8::from(file == SaveFile::Temporary)]);
        self.trace.extend_from_slice(&flags.to_le_bytes());
        self.trace.extend_from_slice(&result.to_le_bytes());
        result
    }

    fn seek(&mut self, file: i32, offset: usize) {
        self.offset = offset;
        self.trace.push(3);
        self.trace.extend_from_slice(&file.to_le_bytes());
        self.trace.extend_from_slice(&(offset as u32).to_le_bytes());
    }

    fn write(&mut self, file: i32, bytes: &[u8]) -> i32 {
        let result = self.results.next().unwrap_or(bytes.len() as i32);
        self.trace.push(4);
        self.trace.extend_from_slice(&file.to_le_bytes());
        self.trace
            .extend_from_slice(&(bytes.len() as u32).to_le_bytes());
        self.trace.extend_from_slice(&result.to_le_bytes());
        self.trace.extend_from_slice(bytes);
        if result > 0 {
            let count = (result as usize).min(bytes.len());
            self.file[self.offset..self.offset + count].copy_from_slice(&bytes[..count]);
            self.offset += count;
        }
        result
    }

    fn close(&mut self, file: i32) {
        self.trace.push(5);
        self.trace.extend_from_slice(&file.to_le_bytes());
    }

    fn erase_temporary(&mut self) {
        self.trace.push(7);
    }
}

fn words(bytes: Vec<u8>) -> Result<std::vec::IntoIter<i32>, String> {
    if bytes.len() % 4 != 0 {
        return Err("save write results need whole i32 words".into());
    }
    Ok(bytes
        .chunks_exact(4)
        .map(|word| i32::from_le_bytes(word.try_into().unwrap()))
        .collect::<Vec<_>>()
        .into_iter())
}

pub fn execute(blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    let [slot, mut header, mut payload, player, world, auxiliary, magic, file, opens, results,
        icon1, icon2, icon3]: [Vec<u8>; 13] = blocks.try_into()
        .map_err(|_| "save-write-slot needs slot, header, payload, four runtime ranges, file, open/write results, three icons")?;
    let slot = i16::from_le_bytes(slot.try_into().map_err(|_| "slot must be i16")?);
    let mut writer = Writer {
        file,
        opens: words(opens)?,
        results: words(results)?,
        icons: vec![icon1, icon2, icon3].into_iter(),
        offset: 0,
        trace: Vec::new(),
    };
    let status = save::write_slot(
        slot,
        &mut header,
        &mut payload,
        SaveRuntimeSources {
            player_state: &player,
            world_state: &world,
            auxiliary_state: &auxiliary,
            magic_records: &magic,
        },
        &mut writer,
    )
    .map_err(|error| format!("{error:?}"))?;
    if writer.opens.next().is_some() || writer.results.next().is_some() {
        return Err("save writer did not consume its result scenario".into());
    }
    Ok(vec![
        status.to_le_bytes().to_vec(),
        header,
        payload,
        writer.trace,
        writer.file,
    ])
}
