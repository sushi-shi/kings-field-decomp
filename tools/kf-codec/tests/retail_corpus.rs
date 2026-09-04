//! Opt-in checks against locally extracted SLPS-00017 resources.
//!
//! Run with `KF_RETAIL_DIR` pointing at the configured retail disc directory:
//! `cargo test --test retail_corpus -- --ignored`.

use std::fs;
use std::path::{Path, PathBuf};

use kf_codec::asset_archive::{Archive, AssetHeader, ASSET_HEADER_SIZE};
use kf_codec::game_data::{CommonData, MixA, MixB};
use kf_codec::tmd::{prepare_primitive_indices, PrepareReport};

#[derive(Default)]
struct Census {
    tmds: usize,
    objects: usize,
    primitives: usize,
    recognized: usize,
}

impl Census {
    fn add_tmd(&mut self, bytes: &[u8], source: &Path) {
        let mut owned = bytes.to_vec();
        let PrepareReport {
            object_count,
            primitive_count,
            recognized_primitive_count,
            ..
        } = prepare_primitive_indices(&mut owned)
            .unwrap_or_else(|error| panic!("{}: {error}", source.display()));
        self.tmds += 1;
        self.objects += usize::from(object_count);
        self.primitives += primitive_count;
        self.recognized += recognized_primitive_count;
    }

    fn add_archive(&mut self, archive: Archive<'_>, source: &Path) {
        let mut assets = archive.assets();
        while let Some(asset) = assets.next() {
            let asset = asset.unwrap_or_else(|error| panic!("{}: {error}", source.display()));
            let tmd = asset.tmd_data().unwrap_or_else(|| {
                panic!(
                    "{}: asset {} TMD offset is outside the asset",
                    source.display(),
                    asset.index
                )
            });
            self.add_tmd(tmd, source);
        }
        assert!(
            assets.remainder().is_empty(),
            "{}: nested archive has {} trailing bytes",
            source.display(),
            assets.remainder().len()
        );
    }

    fn add_asset(&mut self, bytes: &[u8], source: &Path) {
        let header = AssetHeader::parse(bytes).unwrap_or_else(|| {
            panic!(
                "{}: asset header shorter than {ASSET_HEADER_SIZE}",
                source.display()
            )
        });
        let byte_size = usize::try_from(header.byte_size).unwrap();
        let asset = bytes.get(..byte_size).unwrap_or_else(|| {
            panic!(
                "{}: asset declares {byte_size} bytes, {} available",
                source.display(),
                bytes.len()
            )
        });
        let tmd_offset = usize::try_from(header.tmd_data_offset).unwrap();
        let tmd = asset.get(tmd_offset..).unwrap_or_else(|| {
            panic!(
                "{}: TMD offset {tmd_offset} is outside {byte_size}-byte asset",
                source.display()
            )
        });
        self.add_tmd(tmd, source);
    }
}

fn kf_root() -> PathBuf {
    let configured = PathBuf::from(
        std::env::var_os("KF_RETAIL_DIR")
            .expect("set KF_RETAIL_DIR to the extracted retail disc directory"),
    );
    let nested = configured.join("KF");
    if nested.is_dir() {
        nested
    } else {
        configured
    }
}

#[test]
#[ignore = "requires proprietary SLPS-00017 files via KF_RETAIL_DIR"]
fn all_game_resource_containers_and_tmds_parse() {
    let root = kf_root();
    let com_path = root.join("COM/COM.DAT");
    let com_bytes = fs::read(&com_path).unwrap();
    let com = CommonData::parse(&com_bytes).unwrap();
    assert_eq!(
        [
            com.common_asset.len(),
            com.render_cell_windows.len(),
            com.weapon_records.len(),
            com.armor_records.len(),
            com.magic_records.len(),
            com.map_object_definitions.len(),
            com.player_level_growth.len(),
        ],
        [420, 3264, 704, 756, 480, 1128, 600]
    );
    assert!(com.remainder.is_empty());

    let mut census = Census::default();
    census.add_asset(com.common_asset, &com_path);
    for floor in 1..=5 {
        let floor_dir = root.join(format!("B{floor}"));
        let mixa_path = floor_dir.join("MIXA.DAT");
        let mixa_bytes = fs::read(&mixa_path).unwrap();
        let mixa = MixA::parse(&mixa_bytes).unwrap();
        assert!(mixa.map_grids.remainder.is_empty());
        assert!(mixa.remainder.is_empty());

        let mixb_path = floor_dir.join("MIXB.DAT");
        let mixb_bytes = fs::read(&mixb_path).unwrap();
        let mixb = MixB::parse(&mixb_bytes).unwrap();
        assert!(mixb.remainder.is_empty());
        assert_eq!(mixb.embedded_variant_assets.is_some(), floor != 5);
        census.add_tmd(mixb.primary_tmd, &mixb_path);
        census.add_tmd(mixb.secondary_tmd, &mixb_path);
        census.add_archive(mixb.assets_from_id_10, &mixb_path);
        census.add_archive(mixb.assets_from_id_30, &mixb_path);
        if let Some(archive) = mixb.embedded_variant_assets {
            census.add_archive(archive, &mixb_path);
        }
    }

    for variant in 1..=3 {
        let path = root.join(format!("B5/CHR{variant}.MIM"));
        let bytes = fs::read(&path).unwrap();
        let archive = Archive::parse(&bytes).unwrap();
        census.add_archive(archive, &path);
    }

    for directory in ["ITEM1", "ITEM2", "ITEM3", "ITEM4"] {
        for entry in fs::read_dir(root.join(directory)).unwrap() {
            let path = entry.unwrap().path();
            if path.extension().is_some_and(|extension| extension == "TMD") {
                let bytes = fs::read(&path).unwrap();
                census.add_tmd(&bytes, &path);
            }
        }
    }

    for weapon in 0..16 {
        let path = root.join(format!("WEPON/WEP{weapon:02}.MIM"));
        let bytes = fs::read(&path).unwrap();
        census.add_asset(&bytes, &path);
    }

    assert_eq!(census.tmds, 246);
    assert_eq!(census.objects, 1_911);
    assert_eq!(census.primitives, 117_119);
    assert_eq!(census.recognized, census.primitives);
}
