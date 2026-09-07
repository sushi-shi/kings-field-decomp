# Actor pool retained-literal ledger

Complete accounting for the **six retained occurrences** in `src/game/actor_pool.c`
after the [culling/stream-state review](actor-culling-domain.md). Five former
literal uses now belong to the separate culling and placement-stream enums.
Claims and named definitions are excluded; each duplicate token has a row.

## `src/game/actor_pool.c`

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `actor_pool_update` | 16 | `1` | `u16 count = KF_ACTOR_CAPACITY - 1;` | Convert the named pool capacity into the initial zero-based countdown. The post-decrement loop visits all 128 slots. |
| `actor_pool_update` | 27 | `0` | `} while (count-- != 0);` | Exhaustion of the output-slot countdown; preserve the test before decrement. |
| `actor_pool_update` | 28 | `0` | `actor_bind_current(0);` | Null actor pointer clears the current actor binding after the complete pool sweep. |
| `actor_pool_load_placements` | 41 | `1` | `u16 count = KF_ACTOR_CAPACITY - 1;` | Convert the named pool capacity into the initial zero-based countdown. The post-decrement loop visits all 128 slots. |
| `actor_pool_load_placements` | 82 | `0` | `} while (actor++, count-- != 0);` | Advance every output actor, including the terminator and later freed slots, and exhaust the countdown without advancing the placement pointer on those paths. |
| `actor_definitions_load` | 94 | `0` | `} while (--count != 0);` | Stop after the named definition array has been copied word by word; pre-decrement the remaining word count. |
