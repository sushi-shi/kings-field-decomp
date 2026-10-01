//! Pull-based ISO9660 reader. Callers supply only requested slices of a local
//! ISO or MODE2/2352 data track; neither a whole-disc allocation nor I/O lives here.
use super::source;
use std::collections::{BTreeMap, BTreeSet, VecDeque};

const SECTOR: usize = 2048;
const LIMIT: usize = 32 * 1024 * 1024;
const ARCHIVES: [&str; 4] = [
    "CD/COM/MO.T",
    "CD/COM/RTIM.T",
    "CD/COM/FDAT.T",
    "CD/COM/MOF.T",
];
type Result<T> = std::result::Result<T, &'static str>;

pub(super) fn span(data: &[u8], offset: usize, size: usize) -> Result<&[u8]> {
    data.get(offset..offset.checked_add(size).ok_or("Invalid extent")?)
        .ok_or("Truncated character resource")
}
pub(super) fn u16_at(data: &[u8], offset: usize) -> Result<u16> {
    Ok(u16::from_le_bytes(
        span(data, offset, 2)?.try_into().unwrap(),
    ))
}
pub(super) fn u32_at(data: &[u8], offset: usize) -> Result<u32> {
    Ok(u32::from_le_bytes(
        span(data, offset, 4)?.try_into().unwrap(),
    ))
}
fn both32(data: &[u8], offset: usize) -> Result<u32> {
    let value = u32_at(data, offset)?;
    if value != u32::from_be_bytes(span(data, offset + 4, 4)?.try_into().unwrap()) {
        return Err("ISO byte-order copies disagree");
    }
    Ok(value)
}
#[derive(Clone)]
struct Extent {
    path: String,
    sector: u32,
    size: usize,
}
#[derive(PartialEq)]
enum Stage {
    ProbeIso,
    ProbeBin,
    Directory,
    Archive,
    Complete,
    Failed,
}
pub struct Import {
    size: u32,
    stride: usize,
    payload: usize,
    volume_sectors: u32,
    stage: Stage,
    current: Extent,
    directories: VecDeque<Extent>,
    visited: BTreeSet<u32>,
    files: BTreeMap<String, Extent>,
    archives: Vec<Vec<u8>>,
    total_bytes: usize,
    output: Vec<u8>,
    pub error: &'static str,
}
impl Import {
    pub fn new(size: u32) -> Result<Self> {
        if !(17 * 2352..=800 * 1024 * 1024).contains(&size) {
            return Err("Expected a KFIII ISO or BIN data track (up to 800 MiB)");
        }
        Ok(Self {
            size,
            stride: SECTOR,
            payload: 0,
            volume_sectors: 0,
            stage: Stage::ProbeIso,
            current: Extent {
                path: String::new(),
                sector: 16,
                size: SECTOR,
            },
            directories: VecDeque::new(),
            visited: BTreeSet::new(),
            files: BTreeMap::new(),
            archives: Vec::new(),
            total_bytes: 0,
            output: Vec::new(),
            error: "",
        })
    }
    pub fn request(&self) -> Result<Option<(u32, u32)>> {
        match self.stage {
            Stage::Complete => Ok(None),
            Stage::Failed => Err(self.error),
            _ => {
                let extent = &self.current;
                if extent.size == 0 || extent.size > LIMIT {
                    return Err("Invalid disc read size");
                }
                let sectors = extent.size.div_ceil(SECTOR);
                if self.volume_sectors != 0
                    && u64::from(extent.sector) + sectors as u64 > u64::from(self.volume_sectors)
                {
                    return Err("Disc extent exceeds its volume");
                }
                let offset = u64::from(extent.sector) * self.stride as u64 + self.payload as u64;
                let length = (sectors - 1) * self.stride + (extent.size - (sectors - 1) * SECTOR);
                if offset + length as u64 > u64::from(self.size) {
                    return Err("Disc extent exceeds its data track");
                }
                Ok(Some((offset as u32, length as u32)))
            }
        }
    }
    pub fn supply(&mut self, bytes: &[u8]) -> Result<()> {
        let result = self
            .consume(bytes)
            .and_then(|()| self.request().map(|_| ()));
        if let Err(error) = result {
            self.stage = Stage::Failed;
            self.error = error;
        }
        result
    }
    fn consume(&mut self, bytes: &[u8]) -> Result<()> {
        let (_, length) = self.request()?.ok_or("Character import already finished")?;
        if bytes.len() != length as usize {
            return Err("Incomplete disc read");
        }
        let mut data = Vec::with_capacity(self.current.size);
        for sector in bytes.chunks(self.stride) {
            data.extend_from_slice(&sector[..sector.len().min(SECTOR)]);
        }
        if self.stage == Stage::ProbeIso && data.get(..7) != Some(b"\x01CD001\x01") {
            self.stage = Stage::ProbeBin;
            self.stride = 2352;
            self.payload = 24;
            return Ok(());
        }
        match self.stage {
            Stage::ProbeIso | Stage::ProbeBin => self.volume(&data),
            Stage::Directory => {
                self.directory(&data)?;
                self.advance()
            }
            Stage::Archive => {
                self.archives.push(data);
                self.advance()
            }
            _ => Err("Invalid character import state"),
        }
    }
    fn volume(&mut self, data: &[u8]) -> Result<()> {
        if span(data, 0, 7)? != b"\x01CD001\x01"
            || span(data, 40, 32)? != b"SLUS-00255                      "
        {
            return Err("Select US King's Field II (SLUS-00255), the western KFIII release");
        }
        self.volume_sectors = both32(data, 80)?;
        if self.volume_sectors <= 16
            || u64::from(self.volume_sectors) * self.stride as u64 > u64::from(self.size)
            || u16_at(data, 128)? != 2048
            || span(data, 130, 2)? != [8, 0]
        {
            return Err("Invalid ISO volume size or block size");
        }
        let root = span(data, 156, 34)?;
        if root[0] < 34 || root[1] != 0 || root[25] != 2 || root[26] != 0 || root[27] != 0 {
            return Err("Invalid ISO root directory");
        }
        self.queue_directory(Extent {
            path: String::new(),
            sector: both32(root, 2)?,
            size: both32(root, 10)? as usize,
        })?;
        self.advance()
    }
    fn queue_directory(&mut self, extent: Extent) -> Result<()> {
        if extent.size == 0
            || extent.size > 1024 * 1024
            || extent.path.matches('/').count() > 8
            || self.visited.len() >= 128
            || !self.visited.insert(extent.sector)
        {
            return Err("Cyclic or oversized ISO directory");
        }
        self.directories.push_back(extent);
        Ok(())
    }
    fn directory(&mut self, data: &[u8]) -> Result<()> {
        let mut at = 0;
        while at < data.len() {
            let length = data[at] as usize;
            if length == 0 {
                at = (at / SECTOR + 1) * SECTOR;
                continue;
            }
            if length < 34 || at % SECTOR + length > SECTOR {
                return Err("Invalid ISO directory record");
            }
            let row = span(data, at, length)?;
            at += length;
            if row[1] != 0 || row[25] & !3 != 0 || row[26] != 0 || row[27] != 0 {
                return Err("Unsupported ISO file extent");
            }
            let name = span(row, 33, row[32] as usize)?;
            if name == [0] || name == [1] {
                continue;
            }
            let name = std::str::from_utf8(name).map_err(|_| "Invalid ISO file name")?;
            let name = name.strip_suffix(";1").unwrap_or(name);
            if name.is_empty()
                || name == "."
                || name == ".."
                || !name
                    .bytes()
                    .all(|c| c.is_ascii_uppercase() || c.is_ascii_digit() || c == b'_' || c == b'.')
            {
                return Err("Invalid ISO file name");
            }
            let path = if self.current.path.is_empty() {
                name.to_owned()
            } else {
                format!("{}/{name}", self.current.path)
            };
            let extent = Extent {
                path: path.clone(),
                sector: both32(row, 2)?,
                size: both32(row, 10)? as usize,
            };
            if row[25] & 2 != 0 {
                // Only these branches contain the character archives.
                if path == "CD" || path == "CD/COM" {
                    self.queue_directory(extent)?;
                }
            } else if ARCHIVES.contains(&path.as_str()) {
                self.total_bytes = self
                    .total_bytes
                    .checked_add(extent.size)
                    .ok_or("Archive size overflow")?;
                if extent.size == 0
                    || self.total_bytes > LIMIT
                    || self.files.insert(path, extent).is_some()
                {
                    return Err("Duplicate or oversized character archive");
                }
            }
        }
        Ok(())
    }
    fn advance(&mut self) -> Result<()> {
        if let Some(next) = self.directories.pop_front() {
            self.current = next;
            self.stage = Stage::Directory;
        } else if self.archives.len() < ARCHIVES.len() {
            self.current = self
                .files
                .get(ARCHIVES[self.archives.len()])
                .ok_or("KFIII character archive is missing")?
                .clone();
            self.stage = Stage::Archive;
        } else {
            self.output = source::convert(
                &self.archives[0],
                &self.archives[1],
                &self.archives[2],
                &self.archives[3],
            )?;
            self.archives.clear();
            self.stage = Stage::Complete;
        }
        Ok(())
    }
    pub fn result(&self) -> Option<&[u8]> {
        (self.stage == Stage::Complete).then_some(self.output.as_slice())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn both(data: &mut [u8], at: usize, value: u32) {
        data[at..at + 4].copy_from_slice(&value.to_le_bytes());
        data[at + 4..at + 8].copy_from_slice(&value.to_be_bytes());
    }
    fn volume() -> Vec<u8> {
        let mut v = vec![0; SECTOR];
        v[..7].copy_from_slice(b"\x01CD001\x01");
        v[40..72].copy_from_slice(b"SLUS-00255                      ");
        both(&mut v, 80, 32);
        v[128..132].copy_from_slice(&[0, 8, 8, 0]);
        v[156] = 34;
        v[181] = 2;
        both(&mut v, 158, 20);
        both(&mut v, 166, 2048);
        v
    }
    #[test]
    fn sliced_iso_and_bin_reads_and_failures() {
        let mut iso = Import::new(32 * 2048).unwrap();
        assert_eq!(iso.request().unwrap(), Some((16 * 2048, 2048)));
        iso.supply(&volume()).unwrap();
        assert_eq!(iso.request().unwrap(), Some((20 * 2048, 2048)));
        assert!(iso.supply(&[0; 2047]).is_err());
        assert!(iso.request().is_err());
        assert!(iso.result().is_none());
        let mut bin = Import::new(32 * 2352).unwrap();
        bin.supply(&[0; 2048]).unwrap();
        assert_eq!(bin.request().unwrap(), Some((16 * 2352 + 24, 2048)));
        bin.supply(&volume()).unwrap();
        assert_eq!(bin.request().unwrap(), Some((20 * 2352 + 24, 2048)));
        for at in [40, 80, 84, 128, 130, 157, 181, 158, 166] {
            let mut invalid = volume();
            invalid[at] ^= 1;
            let mut iso = Import::new(32 * 2048).unwrap();
            assert!(iso.supply(&invalid).is_err(), "offset {at}");
        }
    }
    #[test]
    fn cyclic_and_malformed_directories_are_terminal() {
        let mut row = vec![0; SECTOR];
        row[0] = 36;
        row[25] = 2;
        row[32] = 2;
        row[33..35].copy_from_slice(b"CD");
        both(&mut row, 2, 20);
        both(&mut row, 10, 2048);
        for mode in 0..4 {
            let mut importer = Import::new(32 * 2048).unwrap();
            importer.supply(&volume()).unwrap();
            let mut bad = row.clone();
            if mode == 1 {
                bad[0] = 33;
            }
            if mode == 2 {
                bad[32] = 255;
            }
            if mode == 3 {
                bad[33] = b'/';
            }
            assert!(importer.supply(&bad).is_err());
            assert!(importer.request().is_err());
            assert!(importer.result().is_none());
        }
        assert!(Import::new(801 * 1024 * 1024).is_err());
        let mut bad = volume();
        both(&mut bad, 80, 0);
        assert!(Import::new(32 * 2048).unwrap().supply(&bad).is_err());
    }
}
