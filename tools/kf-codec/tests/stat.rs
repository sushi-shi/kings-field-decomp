use kf_codec::stat::*;

#[test]
fn stat_extent_and_splits_are_exact() {
    assert_eq!(STAT_DATA_SIZE, 5_708);
    let source: Vec<u8> = (0..STAT_DATA_SIZE)
        .map(|index| (index as u8).wrapping_mul(29))
        .collect();
    let mut menu = vec![0xcc; MENU_ASSETS_SIZE + 3];
    let mut windows = vec![0xcc; MENU_WINDOW_LAYOUTS_SIZE + 3];
    let mut items = vec![0xcc; ITEM_NAMES_SIZE + 3];
    let mut magic = vec![0xcc; MAGIC_NAMES_SIZE + 3];
    let mut buy = vec![0xcc; PRICE_TABLE_SIZE + 3];
    let mut sell = vec![0xcc; PRICE_TABLE_SIZE + 3];

    load_stat_database(
        &source,
        StatDestinations {
            menu_assets: &mut menu,
            window_layouts: &mut windows,
            item_names: &mut items,
            magic_names: &mut magic,
            buy_prices: &mut buy,
            sell_prices: &mut sell,
        },
    )
    .unwrap();

    let mut at = 0;
    for (actual, size) in [
        (&menu, MENU_ASSETS_SIZE),
        (&windows, MENU_WINDOW_LAYOUTS_SIZE),
        (&items, ITEM_NAMES_SIZE),
        (&magic, MAGIC_NAMES_SIZE),
        (&buy, PRICE_TABLE_SIZE),
        (&sell, PRICE_TABLE_SIZE),
    ] {
        assert_eq!(&actual[..size], &source[at..at + size]);
        assert!(actual[size..].iter().all(|&byte| byte == 0xcc));
        at += size;
    }
    assert_eq!(at, source.len());
}

#[test]
fn preflight_failure_preserves_all_destinations() {
    let source = [0; STAT_DATA_SIZE];
    let mut menu = [1; MENU_ASSETS_SIZE];
    let mut windows = [2; MENU_WINDOW_LAYOUTS_SIZE];
    let mut items = [3; ITEM_NAMES_SIZE - 1];
    let mut magic = [4; MAGIC_NAMES_SIZE];
    let mut buy = [5; PRICE_TABLE_SIZE];
    let mut sell = [6; PRICE_TABLE_SIZE];
    assert_eq!(
        load_stat_database(
            &source,
            StatDestinations {
                menu_assets: &mut menu,
                window_layouts: &mut windows,
                item_names: &mut items,
                magic_names: &mut magic,
                buy_prices: &mut buy,
                sell_prices: &mut sell,
            }
        ),
        Err(StatError::DestinationTooSmall {
            section: StatSection::ItemNames,
            needed: ITEM_NAMES_SIZE,
            available: ITEM_NAMES_SIZE - 1,
        })
    );
    assert!(menu.iter().all(|&byte| byte == 1));
    assert!(windows.iter().all(|&byte| byte == 2));
    assert!(items.iter().all(|&byte| byte == 3));
    assert!(magic.iter().all(|&byte| byte == 4));
    assert!(buy.iter().all(|&byte| byte == 5));
    assert!(sell.iter().all(|&byte| byte == 6));
}

#[test]
fn known_rows_round_trip() {
    let row = [
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
        0x10, 0x21, 0x32, 0x43, 0x54,
    ];
    let mut encoded = [0; MENU_GLYPH_ROW_SIZE];
    MenuGlyphRow::decode(&row).unwrap().encode(&mut encoded);
    assert_eq!(row, encoded);

    let string = [0x7d; MENU_GLYPH_STRING_SIZE];
    let mut encoded = [0; MENU_GLYPH_STRING_SIZE];
    MenuGlyphString::decode(&string)
        .unwrap()
        .encode(&mut encoded);
    assert_eq!(string, encoded);

    let price = [0xde, 0xad, 0xbe, 0xef];
    let mut encoded = [0; PRICE_ENTRY_SIZE];
    PriceEntry::decode(&price).unwrap().encode(&mut encoded);
    assert_eq!(price, encoded);
}

#[derive(Default)]
struct SearchTrace {
    paths: Vec<[u8; ITEM_MODEL_PATH_SIZE]>,
}

impl ItemFileSearch for SearchTrace {
    fn search_file(
        &mut self,
        index: u8,
        path: &[u8; ITEM_MODEL_PATH_SIZE],
        record: &mut [u8; CD_FILE_RECORD_SIZE],
    ) -> bool {
        self.paths.push(*path);
        record[0] = index;
        if index == 1 {
            record[4..8].copy_from_slice(&4096u32.to_le_bytes());
            true
        } else if index % 3 == 0 {
            record[4..8].copy_from_slice(&(u32::from(index) * 2048 + 1).to_le_bytes());
            true
        } else {
            false
        }
    }
}

#[test]
fn item_file_names_and_sector_rounding_match_retail() {
    let mut destination = vec![0x91; ITEM_FILE_TABLE_SIZE + 5];
    let mut trace = SearchTrace::default();
    resolve_item_model_files(&mut destination, &mut trace).unwrap();
    assert_eq!(trace.paths.len(), 80);
    assert_eq!(&trace.paths[0][..21], b"\\KF\\ITEM1\\I001.TMD;1\0");
    assert_eq!(&trace.paths[29][..21], b"\\KF\\ITEM1\\I030.TMD;1\0");
    assert_eq!(&trace.paths[30][..21], b"\\KF\\ITEM2\\I031.TMD;1\0");
    assert_eq!(&trace.paths[79][..21], b"\\KF\\ITEM3\\I080.TMD;1\0");
    assert!(trace
        .paths
        .iter()
        .all(|path| path[21..].iter().all(|&byte| byte == 0)));
    for index in 0..ITEM_MODEL_COUNT {
        let at = index * CD_FILE_RECORD_SIZE;
        if index == 1 {
            assert_eq!(destination[at], 1);
            assert_eq!(
                u32::from_le_bytes(destination[at + 4..at + 8].try_into().unwrap()),
                4096
            );
        } else if index % 3 == 0 {
            assert_eq!(destination[at], index as u8);
            assert_eq!(
                u32::from_le_bytes(destination[at + 4..at + 8].try_into().unwrap()),
                (index as u32 + 1) * 2048
            );
        } else {
            assert!(destination[at..at + CD_FILE_RECORD_SIZE]
                .iter()
                .all(|&byte| byte == 0x91));
        }
    }
    assert_eq!(&destination[ITEM_FILE_TABLE_SIZE..], &[0x91; 5]);
}

#[test]
fn file_table_capacity_is_preflighted() {
    let mut destination = vec![0x55; ITEM_FILE_TABLE_SIZE - 1];
    let mut trace = SearchTrace::default();
    assert_eq!(
        resolve_item_model_files(&mut destination, &mut trace),
        Err(ItemFileTableError::DestinationTooSmall {
            needed: ITEM_FILE_TABLE_SIZE,
            available: ITEM_FILE_TABLE_SIZE - 1,
        })
    );
    assert!(trace.paths.is_empty());
    assert!(destination.iter().all(|&byte| byte == 0x55));
}

#[test]
#[ignore = "requires KF_RETAIL_DIR pointing at the initialized retail disc root"]
fn retail_stat_dat_has_the_proven_extent() {
    let root = std::env::var("KF_RETAIL_DIR").expect("set KF_RETAIL_DIR");
    let root = std::path::Path::new(&root);
    let root = if root.join("KF").is_dir() {
        root.join("KF")
    } else {
        root.to_path_buf()
    };
    let bytes = std::fs::read(root.join("COM/STAT.DAT")).unwrap();
    assert_eq!(bytes.len(), STAT_DATA_SIZE);
}
