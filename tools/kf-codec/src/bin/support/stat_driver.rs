use kf_codec::stat::{self, ItemDatabaseDestinations, ItemFileSearch, StatDestinations};

struct Directory {
    records: Vec<u8>,
    paths: Vec<u8>,
}

impl ItemFileSearch for Directory {
    fn search_file(&mut self, index: u8, path: &[u8; 40], record: &mut [u8; 20]) -> bool {
        self.paths.extend_from_slice(path);
        let at = usize::from(index) * 21;
        if self.records[at] == 0 {
            return false;
        }
        record.copy_from_slice(&self.records[at + 1..at + 21]);
        true
    }
}

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 9 || blocks[8].len() != 80 * 21 {
        return Err(
            "item-database expects STAT, six banks, file table, and 80 directory responses".into(),
        );
    }
    let mut directory = Directory {
        records: blocks.pop().unwrap(),
        paths: Vec::new(),
    };
    let source = blocks.remove(0);
    let [menu_assets, window_layouts, item_names, magic_names, buy_prices, sell_prices, file_table] =
        &mut blocks[..]
    else {
        unreachable!()
    };
    stat::load_item_database(
        &source,
        ItemDatabaseDestinations {
            stat: StatDestinations {
                menu_assets,
                window_layouts,
                item_names,
                magic_names,
                buy_prices,
                sell_prices,
            },
            file_table,
        },
        &mut directory,
    )
    .map_err(|error| format!("{error:?}"))?;
    blocks.push(directory.paths);
    Ok(blocks)
}
