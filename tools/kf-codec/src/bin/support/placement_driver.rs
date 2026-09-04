//! Deterministic service context for the host-only placement oracle.

use kf_codec::placements::{self, *};

struct Context {
    grids: Vec<u8>,
    trace: Vec<u8>,
    random: u32,
    effects: i32,
}

impl Context {
    fn height(&self, z: u8, x: u8) -> u8 {
        self.grids[10_000 + usize::from(z) * 100 + usize::from(x)]
    }

    fn occupancy(&mut self, x: u16, z: u16, amount: i32) {
        self.trace.push(3);
        self.trace.extend_from_slice(&x.to_le_bytes());
        self.trace.extend_from_slice(&z.to_le_bytes());
        self.trace.extend_from_slice(&amount.to_le_bytes());
        for row in (i32::from(z) - 2).max(0)..=(i32::from(z) + 2).min(99) {
            for column in (i32::from(x) - 2).max(0)..=(i32::from(x) + 2).min(99) {
                let byte = &mut self.grids[30_000 + row as usize * 100 + column as usize];
                *byte = (*byte & 0xe0) | ((i32::from(*byte) + amount) as u8 & 31);
            }
        }
    }
}

impl FloorItemContext for Context {
    fn floor_height(&mut self, z: u8, x: u8) -> u8 {
        self.height(z, x)
    }

    fn rand(&mut self) -> i32 {
        self.random = self.random.wrapping_mul(1_103_515_245).wrapping_add(12_345);
        let value = ((self.random >> 16) & 0x7fff) as i32;
        self.trace.push(1);
        self.trace.extend_from_slice(&value.to_le_bytes());
        value
    }
}

impl ActorContext for Context {
    fn floor_height_at_position(&mut self, position: Vec3i) -> i32 {
        let cell = ((position.z / 2000) * 100 + position.x / 2000) as u16 as usize;
        let mut height = -i32::from(self.grids[10_000 + cell]) * 100;
        if self.grids[40_000 + cell] == 6 {
            let raised = match self.grids[20_000 + cell] {
                1 => position.x % 2000 > 1000,
                2 => position.z % 2000 < 1000,
                3 => position.x % 2000 < 1000,
                4 => position.z % 2000 > 1000,
                _ => false,
            };
            if raised {
                height += 300;
            }
        }
        self.trace.push(2);
        for value in [position.x, position.y, position.z, height] {
            self.trace.extend_from_slice(&value.to_le_bytes());
        }
        height
    }
}

impl MapEventContext for Context {
    fn floor_height(&mut self, z: u8, x: u8) -> u8 {
        self.height(z, x)
    }
    fn adjust_occupancy(&mut self, x: u16, z: u16, amount: i32) {
        self.occupancy(x, z, amount);
    }
}

impl MapObjectContext for Context {
    fn floor_height(&mut self, z: u8, x: u8) -> u8 {
        self.height(z, x)
    }
    fn adjust_occupancy(&mut self, x: u16, z: u16, amount: i32) {
        self.occupancy(x, z, amount);
    }
    fn construct_effect(&mut self, request: EffectRequest) -> i32 {
        let index = (self.effects * 7 + 3) % 48;
        self.effects += 1;
        self.trace.push(4);
        self.trace.extend_from_slice(&[
            request.argument,
            request.kind,
            request.effect_id,
            u8::from(request.rotation.is_some()),
        ]);
        for value in [request.position.x, request.position.y, request.position.z] {
            self.trace.extend_from_slice(&value.to_le_bytes());
        }
        let rotation = request.rotation.unwrap_or_default();
        for value in [rotation.x, rotation.y, rotation.z] {
            self.trace.extend_from_slice(&value.to_le_bytes());
        }
        self.trace.extend_from_slice(&index.to_le_bytes());
        index
    }
    fn mark_collision_edge(&mut self, object: ObjectSnapshot, value: u8, yaw: u16) {
        self.trace.push(5);
        self.trace
            .extend_from_slice(&object.pool_index.to_le_bytes());
        self.trace.push(value);
        self.trace.extend_from_slice(&yaw.to_le_bytes());
        let x = i32::from(object.cell_x as u8);
        let z = i32::from(object.cell_z as u8);
        let mut points = Vec::new();
        match object.behavior_type {
            2 | 3 => {
                points.push((x, z));
                points.push(match yaw & 0xfff {
                    0 => (x, (z + 1) & 255),
                    0x400 => ((x + 1) & 255, z),
                    0x800 => (x, (z - 1) & 255),
                    0xc00 => ((x - 1) & 255, z),
                    _ => (x, z),
                });
            }
            0 => match yaw & 0xfff {
                0 => points.extend([(x + 1, z - 1), (x + 1, z)]),
                0x400 => points.extend([(x, z + 1), (x + 1, z + 1)]),
                0x800 => points.extend([(x - 1, z + 1), (x - 1, z)]),
                0xc00 => points.extend([(x, z - 1), (x - 1, z - 1)]),
                _ => {}
            },
            _ => {}
        }
        for (x, z) in points {
            let index = 40_000 + (z * 100 + x) as usize;
            self.grids[index] = value;
        }
    }
}

pub fn execute(operation: &str, mut blocks: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>, String> {
    if blocks.len() != 4 || blocks[2].len() != 50_000 {
        return Err("placements expects source, initial pool, five grids, definitions".into());
    }
    let definitions = blocks.pop().unwrap();
    let grids = blocks.pop().unwrap();
    let mut destination = blocks.pop().unwrap();
    let source = blocks.pop().unwrap();
    let mut context = Context {
        grids,
        trace: Vec::new(),
        random: 0x1234_5678,
        effects: 0,
    };
    let report = match operation {
        "placements-items" => {
            placements::load_floor_item_placements(&source, &mut destination, &mut context)
        }
        "placements-actors" => {
            placements::load_actor_placements(&source, &mut destination, &mut context)
        }
        "placements-events" => {
            placements::load_map_event_definitions(&source, &mut destination, &mut context)
        }
        "placements-objects" => placements::load_map_object_placements(
            &source,
            &definitions,
            &mut destination,
            &mut context,
        ),
        _ => return Err(format!("unknown placement operation {operation}")),
    }
    .map_err(|error| format!("{error:?}"))?;
    Ok(vec![
        destination,
        context.grids,
        context.trace,
        report.active_records.to_le_bytes().to_vec(),
    ])
}
