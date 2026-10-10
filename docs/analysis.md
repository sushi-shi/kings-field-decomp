# Native analysis

Run these commands from the checkout in `nix develop`. The pinned shell includes
Clang's sanitizers, Static Analyzer, clang-tidy, cppcheck, Valgrind and Ruff.
Reports and instrumented builds stay under `build/analysis/`.

## Static checks

```sh
python3 scripts/analyze.py static
```

This builds the current source tree and runs all three analyzers using CMake's
compilation database. Sources are selected by the current CMake build.
Choose one tool with `--tool clang`, `--tool tidy`, or
`--tool cppcheck`; limit source paths with `--file src/audio/`. Use `--jobs` to
bound parallel work. `--output` selects a separate report directory.

Clang emits text and plist reports; clang-tidy emits text and suggested fixes
without applying them; cppcheck emits XML. `summary.json` records tool exits.
Warnings require review: successful tool exits do not mean there are no findings.
The runner configures a dedicated build; do not point `--build-dir` at a release
or unrelated build whose compiler flags you want to preserve.

## Runtime sanitizers

```sh
python3 scripts/analyze.py build address
ASAN_OPTIONS=detect_stack_use_after_return=1:strict_string_checks=1:check_initialization_order=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
build/analysis/address/kings-field --data /path/to/verified/resources --language ja
```

`address` combines ASan, UBSan, floating-point division checks and standard-library
assertions. `thread` builds ThreadSanitizer separately; ASan and TSan cannot be
combined. `valgrind` builds an uninstrumented debug executable with assertions.

Run the existing regression suite with:

```sh
UBSAN_OPTIONS=halt_on_error=1 python3 -m unittest discover -s tests -v
ruff check scripts tests
```

For a repeatable disc-backed run, supply an already extracted resource directory
(the build observer is a developer tool, not the normal game launcher):

```sh
xvfb-run -a python3 tests/runtime_scenarios.py \
  --build-dir build/analysis/address --output build/analysis/game-address \
  --data /path/to/resources --language ja --movement --resources
```

This loads cutscene resources and performs six gameplay/save entries across five
floors. It uses isolated saves and a 320x240 window. It does not verify a complete
playthrough or natural ending. `--switch-language` adds language round trips;
`--baseline` compares the six saved files with an earlier run. Use separate Xvfb
server numbers when starting simultaneous runs.

Run the same observer against `build/analysis/thread` with
`TSAN_OPTIONS=halt_on_error=1`. SDL and graphics drivers may not be instrumented,
so inspect dependency stacks before attributing a race to game code. A software
Mesa run with `LIBGL_ALWAYS_SOFTWARE=1 LP_NUM_THREADS=0` can isolate driver-worker
noise; it reduces graphics-thread coverage and is not a substitute for a normal
run. Do not hide findings with blanket suppressions.

## Valgrind and leak checks

```sh
python3 scripts/analyze.py build valgrind
xvfb-run -a python3 tests/runtime_scenarios.py \
  --build-dir build/analysis/valgrind --output build/analysis/game-valgrind \
  --runner 'valgrind --track-origins=yes --leak-check=full --show-leak-kinds=definite,indirect --errors-for-leak-kinds=definite,indirect --error-exitcode=1' \
  --timeout 1800 --data /path/to/resources --language ja --movement --resources
```

Valgrind is much slower than the game. Check both its error count and leak summary;
still-reachable allocations are reported separately from lost allocations.

ASan normally also enables LeakSanitizer on Linux. If process inspection is
restricted, LeakSanitizer may fail even for `kings-field --help`. Run in an
environment that permits its process inspection, or explicitly set
`ASAN_OPTIONS=detect_leaks=0` and use Valgrind for leaks. This disables only leak
checking. The runner preserves your sanitizer environment options.

## Isolated codecs and fuzzing

```sh
python3 scripts/analyze.py codecs memory
python3 scripts/analyze.py codecs valgrind
python3 scripts/analyze.py codecs fuzz --seconds 60
```

The codec suites can also run with `address` or `thread`. MemorySanitizer with
origin tracking checks for uninitialized reads in these isolated tests; a full
game MSan run would require instrumented dependencies. Passing single-threaded
codec tests under TSan does not establish that the game is race-free.

The libFuzzer harness exercises image, animation, placement and audio parsers
under ASan/UBSan. An optional `--corpus /path/to/local/seeds` starts with existing
inputs. Corpus mutations, failures and logs stay in the selected output/corpus
directories. Codec rejection diagnostics are expected and can produce large logs.
Keep retail seeds and derived assets local. A short fuzz run is a smoke check,
not a proof that a parser accepts or rejects every input correctly.
