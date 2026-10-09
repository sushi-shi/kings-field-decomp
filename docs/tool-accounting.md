# Development tool retention review

Original reviewed input: `5deb5b1a` (95 files). The assignment metric adds one
module to the command closure; the current accounting covers **96 Python files**
under `scripts/`, including package initializers and generated-project build
templates. It does not claim every source body has been reviewed or every
tool is used frequently.

## Evidence and decision

Imports were traced from the Nix command wrappers and `kf` dispatch, including
relative imports and package initialization. The semantic navigator's module
names are literal dispatch entries; the codec aggregate constructs its ten
oracle module names dynamically. Those paths were inspected explicitly.
Ninja generation, test imports, export allowlists/build templates and documented
manual commands were checked separately.

| Primary accounting route | Files | Concrete consumer |
| --- | ---: | --- |
| Static closure of Nix commands and `kf` | 74 | `flake.nix` wrappers, `scripts/kf/cli.py` dispatch and their imports |
| Additional manual audits/oracles and shared helpers | 16 | Documented `python -m` commands, codec aggregate dispatch and oracle imports |
| Additional toolchain/export builders | 5 | Nix SDK construction and the two standalone build templates |
| Semantic module runner | 1 | `python -m scripts.kf.sema` package entry point |
| Unaccounted files | 0 | No deletion candidate established |

The counts assign each file to its first accounting route, avoiding double
counting shared code. A static import establishes a dependency; it does not
prove that every branch executes in a particular command. Test imports show
consumers, not complete behavior coverage. These distinctions matter when
deciding whether a tool is disposable.

**Verdict:** retain the current tools. No approved deletion list or unused
script has been established. In particular, preserve literal, Boolean, cast,
type, ownership/relocation and strict-match checks. The assignment cleanup adds `expression_assignments.py` to the static command
closure through the cleanliness board. The tests added by this cleanup were
removed at the user's request; the metric itself is retained.
The remaining retention decisions still apply.

## Commands and shared infrastructure

The Nix wrappers provide `kf`, retail validation/seeding, function
audit/admission, vendored identification, FID census, delinking, object
compilation/comparison, and retail/candidate launch commands. Their Python
entry points and tool dependencies are defined in [flake.nix](../flake.nix).

`kf` dispatch imports initialization, manifest/build graph, progress/banking,
export generation, semantic navigation, lineage, inventory, verification,
native linking, editor/type checks, casts/enums/literals/parameters, resource
census, Boolean audit, trials and hypothesis controls. Supporting modules
are reached through those imports. The actual handlers are in
[cli.py](../scripts/kf/cli.py); command usage remains in the
[build guide](build-system.md) and [development guide](development-tools.md).

[graph.py](../scripts/kf/graph.py) emits Ninja commands through its edge
handlers. Its `_script_inputs()` includes every Python file under `scripts/kf`
and `scripts/psxbuild` as dependency inputs. Merely appearing in that list is
**not** proof a module is executed; the command/import trace supplies the
stronger accounting route. Deleting a file based on that list alone would
misread dependency tracking as runtime use.

The [linker](../scripts/psxbuild/link.py) imports SDK compilation support and
[sdk_compat.py](../scripts/psxbuild/sdk_compat.py) through relative imports.
The interrupt-library correction therefore belongs to the current native
build as well as the exports; it is not an export-only leftover.

## Manual tools outside the command closure

| Tool or helper | Entry point / consumer | Retention evidence |
| --- | --- | --- |
| `pointer_zeros.py` | `python -m scripts.kf.pointer_zeros` | [Development commands](development-tools.md), `test_pointer_zeros.py` |
| `warnings.py` | `python -m scripts.kf.warnings` | [Warning audit](patterns/compiler-warning-triage.md); its reports distinguish inherited behavior and data preconditions |
| `sdk_usage.py` | `python -m scripts.kf.sdk_usage` | [SDK audit](sdk-object-audit.md), `test_sdk_usage.py` |
| `codec_oracle.py` | `python -m scripts.kf.codec_oracle` | [Parser census](game-resource-parser-coverage.md), `test_codec_oracle.py`; dispatches all ten families |
| Ten `*_oracle.py` modules | Aggregate dispatch and individual commands | [Codec commands and provider scope](../tools/README.md); family tests import each oracle |
| `codec_candidate.py` | Oracle calls to `rebuild_units` | Rebuilds the selected native MIPS C candidates with their manifest profiles |
| `rust_codec.py` | Oracle and test imports | Builds the Rust host driver and provides the comparison transport |

The ten families are TMD, resources, map resources, animation, audio,
VAB state, save reads, save writes, world-state load and world persistence.
The aggregate's `import_module(f"scripts.kf.{name}_oracle")` means a literal
module-name search alone misses those consumers. Their bounded retail/C/Rust
checks are not interchangeable with a native build or whole-game acceptance.

## Export and toolchain consumers

The five additional files have explicit build roles:

- [create-toolchain.py](../scripts/create-toolchain.py) is passed as
  `sdkBuilder` by the main flake and both export flakes; the shared
  [toolchain definition](../nix/psx-toolchain.nix) invokes it.
- [Classic build template](../scripts/kf/clean_project/build.py) is copied
  to the export root as `build.py` and invoked by its flake.
- [C++ build template](../scripts/kf/clean_cpp_project/build.py) is selected
  by the modern generator and invoked by its standalone flake.
- [clang.py](../scripts/psxbuild/clang.py) compiles the C++ project and
  invokes [elf_to_lnk.py](../scripts/psxbuild/elf_to_lnk.py) through a relative
  import. Their conversion/build controls are in `test_cpp_source.py`.

[clean.py](../scripts/kf/clean.py) copies the `scripts/psxbuild` support,
toolchain builder and classic templates; [clean_cpp.py](../scripts/kf/clean_cpp.py)
replaces the modern templates. These modules are needed by generated projects
even when they are absent from the master's main command closure.

Finally, [sema/__main__.py](../scripts/kf/sema/__main__.py) is the package
runner. `kf sema` calls the same public implementation directly; having that
CLI path does not make the module runner disposable.
