pub const MENU_ASSETS_SIZE: usize = 912;
pub const MENU_GLYPH_STRING_SIZE: usize = 24;
pub const MENU_WINDOW_LAYOUT_SIZE: usize = 264;
pub const MENU_WINDOW_LAYOUT_COUNT: usize = 9;
pub const MENU_WINDOW_LAYOUTS_SIZE: usize = MENU_WINDOW_LAYOUT_SIZE * MENU_WINDOW_LAYOUT_COUNT;
pub const MENU_GLYPH_ROW_SIZE: usize = 20;
pub const ITEM_NAME_COUNT: usize = 80;
pub const ITEM_NAMES_SIZE: usize = MENU_GLYPH_ROW_SIZE * ITEM_NAME_COUNT;
pub const MAGIC_NAME_COUNT: usize = 9;
pub const MAGIC_NAMES_SIZE: usize = MENU_GLYPH_ROW_SIZE * MAGIC_NAME_COUNT;
pub const PRICE_ENTRY_COUNT: usize = 80;
pub const PRICE_ENTRY_SIZE: usize = 4;
pub const PRICE_TABLE_SIZE: usize = PRICE_ENTRY_COUNT * PRICE_ENTRY_SIZE;
pub const STAT_DATA_SIZE: usize = MENU_ASSETS_SIZE
    + MENU_WINDOW_LAYOUTS_SIZE
    + ITEM_NAMES_SIZE
    + MAGIC_NAMES_SIZE
    + PRICE_TABLE_SIZE * 2;

pub const CD_FILE_RECORD_SIZE: usize = 20;
pub const ITEM_MODEL_COUNT: usize = 80;
pub const ITEM_FILE_TABLE_SIZE: usize = CD_FILE_RECORD_SIZE * ITEM_MODEL_COUNT;
pub const ITEM_MODEL_PATH_SIZE: usize = 40;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum StatError {
    SourceTooSmall {
        needed: usize,
        available: usize,
    },
    DestinationTooSmall {
        section: StatSection,
        needed: usize,
        available: usize,
    },
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum StatSection {
    MenuAssets,
    WindowLayouts,
    ItemNames,
    MagicNames,
    BuyPrices,
    SellPrices,
}

pub struct StatDestinations<'a> {
    pub menu_assets: &'a mut [u8],
    pub window_layouts: &'a mut [u8],
    pub item_names: &'a mut [u8],
    pub magic_names: &'a mut [u8],
    pub buy_prices: &'a mut [u8],
    pub sell_prices: &'a mut [u8],
}

pub trait ItemFileSearch {

    fn search_file(
        &mut self,
        index: u8,
        path: &[u8; ITEM_MODEL_PATH_SIZE],
        record: &mut [u8; CD_FILE_RECORD_SIZE],
    ) -> bool;
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ItemFileTableError {
    DestinationTooSmall { needed: usize, available: usize },
}

#[derive(Debug, Eq, PartialEq)]
pub enum ItemDatabaseError {
    Stat(StatError),
    FileTable(ItemFileTableError),
}

pub struct ItemDatabaseDestinations<'a> {
    pub stat: StatDestinations<'a>,
    pub file_table: &'a mut [u8],
}

pub fn load_stat_database(
    source: &[u8],
    destinations: StatDestinations<'_>,
) -> Result<(), StatError> {
    if source.len() < STAT_DATA_SIZE {
        return Err(StatError::SourceTooSmall {
            needed: STAT_DATA_SIZE,
            available: source.len(),
        });
    }
    validate_destination(
        destinations.menu_assets.len(),
        MENU_ASSETS_SIZE,
        StatSection::MenuAssets,
    )?;
    validate_destination(
        destinations.window_layouts.len(),
        MENU_WINDOW_LAYOUTS_SIZE,
        StatSection::WindowLayouts,
    )?;
    validate_destination(
        destinations.item_names.len(),
        ITEM_NAMES_SIZE,
        StatSection::ItemNames,
    )?;
    validate_destination(
        destinations.magic_names.len(),
        MAGIC_NAMES_SIZE,
        StatSection::MagicNames,
    )?;
    validate_destination(
        destinations.buy_prices.len(),
        PRICE_TABLE_SIZE,
        StatSection::BuyPrices,
    )?;
    validate_destination(
        destinations.sell_prices.len(),
        PRICE_TABLE_SIZE,
        StatSection::SellPrices,
    )?;

    let mut at = 0;
    copy_section(source, &mut at, destinations.menu_assets, MENU_ASSETS_SIZE);
    copy_section(
        source,
        &mut at,
        destinations.window_layouts,
        MENU_WINDOW_LAYOUTS_SIZE,
    );
    copy_section(source, &mut at, destinations.item_names, ITEM_NAMES_SIZE);
    copy_section(source, &mut at, destinations.magic_names, MAGIC_NAMES_SIZE);
    copy_section(source, &mut at, destinations.buy_prices, PRICE_TABLE_SIZE);
    copy_section(source, &mut at, destinations.sell_prices, PRICE_TABLE_SIZE);
    debug_assert_eq!(at, STAT_DATA_SIZE);
    Ok(())
}

pub fn resolve_item_model_files<C: ItemFileSearch>(
    destination: &mut [u8],
    context: &mut C,
) -> Result<(), ItemFileTableError> {
    if destination.len() < ITEM_FILE_TABLE_SIZE {
        return Err(ItemFileTableError::DestinationTooSmall {
            needed: ITEM_FILE_TABLE_SIZE,
            available: destination.len(),
        });
    }
    let mut path = [0; ITEM_MODEL_PATH_SIZE];
    path[..20].copy_from_slice(b"\\KF\\ITEM0\\I000.TMD;1");
    for index in 0..ITEM_MODEL_COUNT {
        let number = index + 1;
        path[8] = b'1' + (index / 30) as u8;
        path[11] = b'0' + (number / 100) as u8;
        path[12] = b'0' + ((number % 100) / 10) as u8;
        path[13] = b'0' + (number % 10) as u8;
        let record =
            &mut destination[index * CD_FILE_RECORD_SIZE..(index + 1) * CD_FILE_RECORD_SIZE];
        let mut candidate = [0; CD_FILE_RECORD_SIZE];
        candidate.copy_from_slice(record);
        if context.search_file(index as u8, &path, &mut candidate) {
            let size = read_u32(&candidate, 4);
            if size & 0x7ff != 0 {
                write_u32(&mut candidate, 4, ((size >> 11) + 1) << 11);
            }
            record.copy_from_slice(&candidate);
        }
    }
    Ok(())
}

pub fn load_item_database<C: ItemFileSearch>(
    source: &[u8],
    destinations: ItemDatabaseDestinations<'_>,
    context: &mut C,
) -> Result<(), ItemDatabaseError> {
    if destinations.file_table.len() < ITEM_FILE_TABLE_SIZE {
        return Err(ItemDatabaseError::FileTable(
            ItemFileTableError::DestinationTooSmall {
                needed: ITEM_FILE_TABLE_SIZE,
                available: destinations.file_table.len(),
            },
        ));
    }
    load_stat_database(source, destinations.stat).map_err(ItemDatabaseError::Stat)?;
    resolve_item_model_files(destinations.file_table, context).map_err(ItemDatabaseError::FileTable)
}

fn validate_destination(
    available: usize,
    needed: usize,
    section: StatSection,
) -> Result<(), StatError> {
    if available < needed {
        Err(StatError::DestinationTooSmall {
            section,
            needed,
            available,
        })
    } else {
        Ok(())
    }
}

fn copy_section(source: &[u8], at: &mut usize, destination: &mut [u8], size: usize) {
    destination[..size].copy_from_slice(&source[*at..*at + size]);
    *at += size;
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MenuPoint {
    pub x: i16,
    pub y: i16,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MenuGlyphString {
    pub position: MenuPoint,
    pub glyphs: MenuGlyphRow,
}

impl MenuGlyphString {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MENU_GLYPH_STRING_SIZE)?;
        Some(Self {
            position: MenuPoint {
                x: read_u16(bytes, 0) as i16,
                y: read_u16(bytes, 2) as i16,
            },
            glyphs: MenuGlyphRow::decode(&bytes[4..])?,
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MENU_GLYPH_STRING_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.position.x as u16);
        write_u16(bytes, 2, self.position.y as u16);
        self.glyphs.encode(&mut bytes[4..])
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MenuGlyphRow {
    pub codes: [i16; 10],
}

impl MenuGlyphRow {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..MENU_GLYPH_ROW_SIZE)?;
        Some(Self {
            codes: read_i16_array(bytes, 0),
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..MENU_GLYPH_ROW_SIZE) else {
            return false;
        };
        write_i16_array(bytes, 0, &self.codes);
        true
    }
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct PriceEntry {

    pub by_shop: [u16; 2],
}

impl PriceEntry {
    pub fn decode(bytes: &[u8]) -> Option<Self> {
        bytes.get(..PRICE_ENTRY_SIZE)?;
        Some(Self {
            by_shop: [read_u16(bytes, 0), read_u16(bytes, 2)],
        })
    }

    pub fn encode(&self, bytes: &mut [u8]) -> bool {
        let Some(bytes) = bytes.get_mut(..PRICE_ENTRY_SIZE) else {
            return false;
        };
        write_u16(bytes, 0, self.by_shop[0]);
        write_u16(bytes, 2, self.by_shop[1]);
        true
    }
}

fn read_u16(bytes: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([bytes[at], bytes[at + 1]])
}

fn write_u16(bytes: &mut [u8], at: usize, value: u16) {
    bytes[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn read_u32(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]])
}

fn write_u32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn read_i16_array(bytes: &[u8], at: usize) -> [i16; 10] {
    let mut output = [0; 10];
    let mut index = 0;
    while index < output.len() {
        output[index] = i16::from_le_bytes([bytes[at + index * 2], bytes[at + index * 2 + 1]]);
        index += 1;
    }
    output
}

fn write_i16_array(bytes: &mut [u8], at: usize, values: &[i16; 10]) {
    let mut index = 0;
    while index < values.len() {
        bytes[at + index * 2..at + index * 2 + 2].copy_from_slice(&values[index].to_le_bytes());
        index += 1;
    }
}
