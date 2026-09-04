//! Host-only deterministic context for the floor-state restoration oracle.

use kf_codec::world_state::{self, *};

struct Context {
    floor_heights: Vec<u8>,
    random: u32,
    found_actor: i32,
    trace: Vec<u8>,
}

impl RestoreContext for Context {
    fn rand(&mut self) -> i32 {
        self.random = self.random.wrapping_mul(1_103_515_245).wrapping_add(12_345);
        let value = ((self.random >> 16) & 0x7fff) as i32;
        self.trace.push(1);
        self.trace.extend_from_slice(&value.to_le_bytes());
        value
    }

    fn floor_height(&mut self, cell_z: u16, cell_x: u16) -> u8 {
        self.floor_heights[usize::from(cell_z) * 100 + usize::from(cell_x)]
    }

    fn apply_copy_region(&mut self, region: u8) {
        self.trace.extend_from_slice(&[2, region]);
    }

    fn find_actor_at_tile(&mut self, tile_x: u8, tile_z: u8) -> i32 {
        self.trace.extend_from_slice(&[3, tile_x, tile_z]);
        self.trace
            .extend_from_slice(&self.found_actor.to_le_bytes());
        self.found_actor
    }

    fn clear_object_link(&mut self, link_id: u8) {
        self.trace.extend_from_slice(&[4, link_id]);
    }

    fn trigger_object_link(&mut self, link_id: u8) {
        self.trace.extend_from_slice(&[5, link_id]);
    }

    fn begin_actor_death_by_definition(&mut self, definition_id: u16) {
        self.trace.push(6);
        self.trace.extend_from_slice(&definition_id.to_le_bytes());
    }
}

pub fn execute(mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 8
        || blocks[1].len() != 11
        || blocks[6].len() != 10_000
        || blocks[7].len() != 8
    {
        return Err("world-restore expects record, 11-byte conditions, event/head/actor/object buffers, 10000 heights, and seed/found-actor".into());
    }
    let source = blocks.remove(0);
    let conditions = blocks.remove(0);
    let floor_heights = blocks.remove(4);
    let params = blocks.remove(4);
    let [event_pool, actor_state_head, actor_pool, object_pool] = &mut blocks[..] else {
        unreachable!()
    };
    let mut context = Context {
        floor_heights,
        random: u32::from_le_bytes(params[..4].try_into().unwrap()),
        found_actor: i32::from_le_bytes(params[4..].try_into().unwrap()),
        trace: Vec::new(),
    };
    let conditions = RestoreConditions {
        current_floor: conditions[0],
        highest_floor: conditions[1],
        world_byte_1: conditions[2],
        world_byte_2: conditions[3],
        flag_8009eafc: conditions[4],
        flag_8009f844: conditions[5],
        flag_8009f845: conditions[6],
        flag_8009f846: conditions[7],
        inventory_0a: conditions[8],
        inventory_0b: conditions[9],
        boss_defeat_complete: conditions[10],
    };
    world_state::restore_floor_state(
        &source,
        WorldStateDestinations {
            event_pool,
            actor_state_head,
            actor_pool,
            object_pool,
        },
        conditions,
        &mut context,
    )
    .map_err(|error| format!("{error:?}"))?;
    blocks.push(context.trace);
    Ok(blocks)
}
