# TMD differential oracle

`scripts.kf.tmd_oracle` compares three implementations on identical input:
retail `tmd_prepare_primitive_indices` at `0x8001c2b0`, the freshly compiled C
candidate from `build/objdiff/game/base/8001b7b0_render.o`, and independent
allocation-free Rust. `--no-rebuild` explicitly reuses a previously built C
object; a supplied alternate candidate object is used as-is.

```sh
nix develop --command python -m scripts.kf.tmd_oracle
```

This is not a game or overlay launch. The runner maps the verified GAME load
image into a zeroed 2 MiB PSX RAM model, supplies one TMD payload and the
`current_asset` pointer, enters the function directly, and stops at a sentinel
return address or an instruction limit. The shared `parser_machine` also
guards writes and code fetches and audits executed load-delay dependencies.
No BIOS, CD, GPU, audio, callbacks, or game loop execute.

The candidate's function bytes and compiler-emitted `.rodata` switch table are
read from the ELF object and relocated at runtime. Both the older `tmd_state`
identity and the complete `graphics_context` owner are accepted whether they
are undefined references or definitions in an owned `.bss` section, but both
must resolve to the retail `current_asset` field at `0x80090fc8`. Unsupported
or unresolved relocation forms fail closed.

By default the oracle discovers resource bytes only below the locally
configured retail disc directory. It covers the two raw TMD chunks and all
asset entries in `KF/B1..B5/MIXB.DAT`, all `KF/B5/CHR*.MIM` archives, the
common-data asset, all weapon assets, and the 67 item TMDs. The fixed
SLPS-00017 census is 246 payloads, 1,911 objects, and 117,119 packets, with all
eight masked polygon modes represented. Generated zero-object, zero-primitive,
unknown-mode/wrapping and all-mode/ABE cases run before the shipped corpus. No retail payload is
copied into the repository.

The result is semantic evidence, not matching closure. The whole mutated asset
is compared, and generated cases are also checked against a small format model,
but only strict objdiff `100%` establishes byte identity. A passing run cannot
prove untouched parser paths outside this corpus, compiler attribution, or the
correctness of unrelated resource loaders.
