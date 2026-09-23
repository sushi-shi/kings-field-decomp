# Gameplay startup after the resource cleanup

The reported September 2026 local cleanup added `resource_chunk_data<T>` to the common-resource
loader. Its check required a complete native table inside each nominal chunk.
Entering GAME consequently exited with `Truncated or unaligned resource record`.
This was an explicit `host_fail` / exit 1, not a segmentation fault.

Two original COM copies extend into the next chunk. The existing
Rust `CommonData::parse` in `codecs/src/game_data.rs` already models their sources
as bounded file tails:

| Table | Payload offset | Nominal payload | Original copy |
| --- | ---: | ---: | ---: |
| Armor | `0x1134` | 756 bytes | 1,176 bytes |
| Map-object definitions | `0x1610` | 1,128 bytes | 1,280 bytes |

Armor includes the following four-byte magic header and 416 magic bytes. Object
definitions include the following growth header and 148 growth bytes. Preserve
these copied bytes and the original next-chunk cursor. Only these two source
views use `resource_stream_tail`; other records retain their chunk bounds. The
tail view validates the chunk header and the table check still enforces file
bounds and alignment.

The same local cleanup also replaced module snapshots with direct initialization
and assignment of `ObjectType`. That cannot compile for C-array globals. The
snapshot now owns its value in a struct and copies array elements recursively,
including nested arrays, while scalar and struct values use ordinary assignment.

Native `host_fail` now releases mouse capture, pauses audio and displays the
message until dismissed, then performs shutdown and exits 1. Stderr remains
available and is flushed before opening the dialog. Browser status reporting is
unchanged. Resource diagnostics distinguish size and alignment errors and name
the two COM tables with required/available byte counts.

## Verification

Run the seven synthetic regression tests without retail files:

```sh
nix develop --command python3 -m unittest discover -s tests -v
```

They compile the actual `common_resources_load` function and equipment loader
with stubbed file/graphics/audio services and patterned COM chunks. They verify
both complete original copy spans, following magic/growth data, truncated file
rejection, nominal chunk rejection, alignment, invalid chunk headers, and repeated
scalar/array/pointer resets. AddressSanitizer and UndefinedBehaviorSanitizer are
enabled. In restricted environments where LeakSanitizer cannot inspect processes,
run with `ASAN_OPTIONS=detect_leaks=0`; this retains address and UB checking.

Linux and WebAssembly builds passed. A virtual-display run with the original
Japanese resources reached gameplay and opened the Japanese inventory menu.
A separate client using the real `host_fail` verified the visible dialog, stderr
message, and exit status 1 after dismissal. Captures and logs are local ignored
`build/resource-fix-*` artifacts. These checks do not establish full playthrough
or English/browser runtime coverage.
