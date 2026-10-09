# Development tools

The [tool retention review](tool-accounting.md) traces command, manual-audit,
oracle and export consumers. No unused-script deletion list is established.

Inside `nix develop`, `objdiff` automatically opens `build/objdiff`, including
from subdirectories. This single project groups units under `psx/`, `game/`,
and `open/`. Run `kf analyze` first to generate it; an explicit `objdiff -p PATH`
still opens another project.

The development shell also provides the pinned `pcsx-redux` build used for
runtime checks. The Nix AppImage wrapper supplies its OpenGL, PulseAudio, and
ALSA runtime dependencies. Disc images remain local and are not part of the
flake.

Use `kf-run-retail` to launch the hash-identical disc. After `kf build`,
`kf-run-candidate` makes a temporary disc from the newly linked executables and
launches it. Both use the retail resources configured by `kf init`. If the raw
disc is not the sole `.cue`/`.bin` beside that resource directory, point to it
explicitly:

```sh
export KF_RETAIL_DISC="/path/to/King's Field (Japan).cue"
kf-run-retail
kf build
kf-run-candidate
```

`KF_RETAIL_DISC` may name a cue, raw Mode 2/2352 binary, or a directory
containing one disc. The candidate defaults to `build/link`; an explicit
`KF_CANDIDATE_DIR` can select another source-to-EXE output tree.
`KF_CANDIDATE_IMAGES` can select a comma-separated subset of the three
executables for controlled hybrid tests. The retail BIN must match SLPS-00017 SHA-256
`ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9`.

Neovim/CoC and other clangd clients discover the generated `compile_commands.json`
at the repository root. `nix develop`, `kf configure`, and `kf analyze` refresh it
with the pinned SDK includes, MIPS layout, and modern C++20 type checking.
The retail build still compiles the C sources with its pinned C compiler;
modern scoped enums and Boolean types preserve their declared storage widths.
Use `kf clangd --image open`
or `kf clangd --image game` to select the context for sources shared by both
images; the choice persists under `build/clangd/`. Start Neovim inside the Nix
shell so it uses the pinned clangd. `kf clangd --mode retail` selects the C89
editor view; `--mode modern` restores scoped-enum checks. The mode also persists.

Run `kf check-types` to check every source/image variant, including both
versions of shared sources. Use `--unit game.actor` or `--image game` to focus
the check. Conversions between typed pointers and `void *` must be explicit
in source, in both directions. Modern C++ checking rejects implicit restoration
of a typed pointer; the retail editor enables `-Werror=implicit-void-ptr-cast`.
A read-only target-C AST check additionally rejects implicit erasure to `void *`,
which Clang otherwise accepts silently. It also runs the `kf literals` census
and fails on any written literal stored into, compared with or passed as a
scoped-enum field, parameter, return, promoted local or switch subject; a
`KF_ENUM_ENCODE` boundary is an integer view and stays allowed. The checker
does not rewrite source or suppress diagnostics. Logs are saved under
`build/clangd/checks/`.

Run `kf literals --domains` to list every written integer literal's sink and the
value-flow domains that join them; see the
[enum-domain plan](patterns/enum-domain-plan.md).

Run `kf bools --output build/boolean-audit/all.json` to audit integral fields,
locals, globals, arguments, pointer outputs, arrays, and return values through
Python libclang. `--list` displays candidates and review cases; `--image` and
`--unit` select a partial census. The report retains unknown writers, numeric
uses, and existing enum/Boolean domains. Review proposals against retail before
changing types; see [Boolean modeling](patterns/boolean-modeling.md).

Run `python -m scripts.kf.pointer_zeros` inside `nix develop` to find written
zero literals converted to pointers using [pylibclang](https://pypi.org/project/pylibclang/).
It prints `file:line:column` locations for assignments, initializers (including
aggregates), casts, arguments, returns, and comparisons, excluding existing
`NULL` expansions and ordinary integer zeros. It scans every manifest C variant
and its included project headers, deduplicating written locations. Inactive
preprocessor branches and implicit zero-fill are outside the scan.
Use `--image game`, `--unit game.actor`, or `--path src/game/` to narrow the
search; `--json` emits locations and image/unit/function/type contexts for agents.
`--check` exits 1 when sites remain; parse errors exit 2. The script never edits
C sources.
