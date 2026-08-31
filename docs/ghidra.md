# Ghidra and PSX extensions

The default shell supplies Ghidra 12.0.4 with
[`lab313ru/ghidra_psx_ldr`](https://github.com/lab313ru/ghidra_psx_ldr)
tag `2026.07.08`, pinned at commit
`85d9efaf5693418979152c2298f776734824035b`. Upstream's build matrix explicitly
tests this tag with Ghidra 12.0.4.

The extension adds:

- PS-X EXE loading and overlay support;
- PSX R3000/GTE language support;
- Psy-Q OBJ and LIB filesystem/loaders;
- bundled upstream Psy-Q signature databases and data types;
- GTE and PSX symbol-import scripts.

`flake.nix` builds the extension with Ghidra's `buildExtension.gradle`, then
composes it with Nixpkgs' `ghidra.withExtensions`. The resulting GUI and
headless launchers set Nixpkgs' `NIX_GHIDRAHOME` extension root. The shell
exports that same root for PyGhidra while `GHIDRA_INSTALL_DIR` continues to
point at the base installation. This makes the extension active by default
without copying Ghidra or writing into the user's settings directory.

The upstream extension also tracks an old XML-form `mips32le.sla`. Merely
packaging that file lets the Java classes load but makes a real PS-X import
fail with `Missing SLA format header`. The Nix build deletes it and invokes
Ghidra 12.0.4's pinned Sleigh compiler on the `.slaspec` sources. The flake
check imports a synthetic PS-X EXE, so both loader discovery and the compiled
language are exercised.

To add another extension, define another immutable source input and extension
derivation, then add its output to the list passed to `ghidra.withExtensions`.
This keeps plugin versions coupled to the Ghidra version and makes
`nix flake check` catch API or build incompatibilities.

The bundled signature sets currently start at Psy-Q 2.60. They are useful for
later-library comparison but must not be treated as signatures for the
Release 2.5 libraries matched in King's Field; our hash-pinned 2.5 libraries
remain the primary evidence for those bodies.

`kf-vendored-seed` uses the 2.60 JSON corpus as a secondary byte-signature
lane. It requires a unique executable occurrence and a meaningful signature
label aligned to an admitted function start. Resulting rows retain
`psyq260-signature*` confidence and never override a Release 2.5 object claim.
These are plugin wildcard signatures, not a Ghidra FID database; the TSV keeps
the evidence type explicit so a future generated FID corpus can be compared
without conflating the two mechanisms.
