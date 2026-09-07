# OPEN entity-pool retained-literal ledger

Companion to [the model-domain evidence](open-model-domain.md). All 13 retained
occurrences in `src/open/opening_entity_pool.c` are listed in source order.
Comments, strings, identifier digits, named enum/macro definitions and address
claims are excluded. Initialization alone does not identify the three opaque
control fields; their zeros remain documented without invented role names.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `opening_entity_pool_reset` | 13 | `1` | `u16 remaining = KF_OPENING_ENTITY_CAPACITY - 1;` | Post-decrement loop starts at capacity minus one to visit all 32 slots. |
| `opening_entity_pool_reset` | 18 | `0` | `} while (remaining-- != 0);` | Zero terminates the post-decrement slot countdown. |
| `opening_entity_pool_reset` | 20 | `0` | `opening_entity_state.unknown_control_50e = 0;` | Observed zero initialization; no validated read establishes the control field's role. |
| `opening_entity_pool_reset` | 21 | `0` | `opening_entity_state.unknown_control_50c = 0;` | Observed zero initialization; no validated read establishes the control field's role. |
| `opening_entity_pool_reset` | 22 | `0` | `opening_entity_state.unknown_control_50a = 0;` | Observed zero initialization; no validated read establishes the control field's role. |
| `opening_entity_find_by_object_id` | 39 | `0` | `return 0;` | Null pointer when no active entity matches the model ID. |
| `opening_entity_pool_load_placements` | 46 | `0` | `u16 exhausted = 0;` | False initial value of the placement-exhausted flag. |
| `opening_entity_pool_load_placements` | 49 | `1` | `u16 remaining = KF_OPENING_ENTITY_CAPACITY - 1;` | Post-decrement loop starts at capacity minus one to visit all 32 slots. |
| `opening_entity_pool_load_placements` | 52 | `1` | `if (exhausted == 1) {` | True value of the placement-exhausted flag. |
| `opening_entity_pool_load_placements` | 62 | `0` | `entity->rotation.z = 0;` | Zero roll; placements provide yaw only. |
| `opening_entity_pool_load_placements` | 63 | `0` | `entity->rotation.x = 0;` | Zero pitch; placements provide yaw only. |
| `opening_entity_pool_load_placements` | 82 | `1` | `exhausted = 1;` | True after the placement terminator; later slots remain inactive. |
| `opening_entity_pool_load_placements` | 87 | `0` | `} while (remaining-- != 0);` | Zero terminates the post-decrement slot countdown. |
