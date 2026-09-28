# Cleanup runtime audit

This audit compares the original working port (`55287721`), the local cast and
language cleanup, and the current `port` refactors (`632843cf`) with the resource
fix. These are source-port comparisons, not retail MIPS equivalence proofs.

## Findings

- **Common table bounds:** the local checked-record conversion rejected two
  original COM copies that cross chunk boundaries. The fix retains the copied
  bytes and checks the file boundary. See [resource loading](resource-loader-regression.md).
- **Array snapshots:** direct assignment of C-array globals prevented the local
  cleanup from compiling. Recursive typed copying supports arrays and nested
  arrays without changing pointer values.
- **Lighting copy:** the local cleanup replaced an 18-byte color-matrix copy
  with whole-`MATRIX` assignment. `matrix_interpolate` only initializes the nine
  rotation elements, leaving translation indeterminate. A test compiled with
  `-ftrivial-auto-var-init=pattern` reproduced translation corruption. Lighting
  currently consumes only the rotation elements; no visible rendering failure
  was established. Interpolating directly into the destination preserves its
  translation and supports aliased endpoints. Current upstream still used the
  narrower copy; the PR's direct interpolation and test preserve that behavior.
- **Hidden fatal errors:** required-resource loaders and arena allocation had
  bare `exit(1)` paths in addition to `host_fail`. All in-game fatal exits now
  use the host error handler. Required-file errors include the resource name;
  detailed I/O reasons still go to stderr. Optional-file failures still return
  to their callers. Startup CLI/import failures remain console diagnostics.

## Checks and scope

| Area | Check | Result |
| --- | --- | --- |
| Resource copies | Actual common loader on patterned COM data, short files, bad alignment and headers | Original copy spans preserved; invalid inputs rejected |
| Native state | Scalar, array, nested-array and pointer snapshot round trips | Passed |
| Animation guards | 169 Japanese model assets, 214 clips, 813 keyframes and 2,664 morph references | No shipped record rejected by the local `sizeof` bounds checks |
| Persistence representation | 65,536 patterned eight-byte map links through the local serializer/deserializer | Every byte preserved |
| Floor initialization | Floors 1–5, then floor 1 again; 16 normal updates per entry | Six save files identical across all three versions |
| Player operations | 96 updates per entry with fixed forward, turn, strafe, attack, backward and look input | Six more save files identical across all three versions |
| Save/load | Write, read catalog, deliberately change gold, read saved slot | Gold restored on all 12 scenario entries per version |
| GAME/OPEN lifecycle | Real teardown, opening startup/skip and re-entry between scenario entries | Completed; initial arena/path checks passed |
| Lighting | Four blend values, translation sentinels, aliased source/destination | Passed after fix |
| Error display | Actual SDL dialog kept open until dismissal; stderr retained; exit status checked | Passed |

The current-port runtime checks ran with address/undefined-behavior sanitizers.
Leak scanning was disabled because this execution environment blocks the
process inspection it requires. Linux and WebAssembly builds passed. The port's
Rust manifests disable tests; `cargo test` only builds those packages and is not
counted as test coverage here.

These scenarios force floor entry with the correct floor-five variant and skip
the opening through its input path. They do not exercise natural stairs/warps,
every actor/combat branch, the full opening/ending, audio fidelity, all shop and
dialogue choices, browser runtime, or English resources. Save equivalence is not
pixel, audio or complete internal-state equivalence. No additional divergence
was found in the exercised paths; that is not a claim that every cleanup is
bug-free.

## Repeating the checks

```sh
nix develop --command env ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -v
nix develop --command cmake --preset sanitize
nix develop --command xvfb-run -a -s '-screen 0 1600x1200x24' \
  python3 tests/runtime_scenarios.py --data /path/to/extracted/disc
nix develop --command xvfb-run -a -s '-screen 0 1600x1200x24' \
  python3 tests/runtime_scenarios.py --data /path/to/extracted/disc --movement
```

The runtime driver rebuilds the selected target and compiles three small
observer wrappers around the real game functions. It uses a separate virtual
display and save directory. It does not rewrite production sources or replace
the normal executable. The normal loop, loaders, rendering, actor/effect updates,
save code and transition cleanup execute; only the selected inputs, entry floor,
scenario length and opening skip are controlled.

Use `--source-root /path/to/reference --build linux` to run the same fixture
against the original port, then `--baseline /path/to/reference/build/cleanup-audit`
to compare all six save files byte for byte. For the movement scenario, add
`--movement` and use its `build/cleanup-audit-movement` directory. Retail assets,
generated observer objects, save files and logs stay under ignored/local paths.
