# Original CCPSX driver defaults: bounded OPEN control

This provenance check starts from master `4cd8fae`. It observes a supplied
driver; it does not establish King's Field's historical compiler, flags or
optimizer mechanisms. Canonical source and `probe-gcc257-o2-g0` stay unchanged.

## Original driver observation

The pinned Release 2.5 collection's CCPSX documentation describes `-v` command
echoing and `-S` assembly output. Copy its original CCPSX, original 2.6.0
frontend and PSYQ.INI verbatim to an isolated directory. Their SHA-256 values:

```text
CCPSX.EXE 50a85bebf4bb2e1672ce5fb33d1943e35657fc76ad093e03310e048adc12065b
CC1PSX.EXE e65635ec539f2b9c6db6f2b350c1fd300a26fdd0a264b3f2defefa9c2d0d6dd3
PSYQ.INI bda1c75a8b1668f1b24b78eb4311704bd78c9cffd776376ae151ff0587ad15a1
```

Use `nix develop`'s DOSBox Staging with `--noprimaryconf --nolocalconf
--noautoexec`, mount only that directory as C, and set COMPILER_PATH and
PSYQ_PATH to `c:/`. Compile the preprocessed control
`int ccpsx_driver_probe(int value) { return value + 1; }` with
`ccpsx -v -S -O2 probe.i -o probe.s`. No game, protected assembler or linker
is run, and no executable is modified. CCPSX echoes:

```text
cc1psx.exe -quiet -O2 probe.i -o probe.s
```

The frontend successfully emits the leaf body and identifies GNU C 2.6.0
`[AL 1.1, MM 40]`, defaults `-mgas -msoft-float`, G value 8, CPU 3000 and
ISA 1. No additional optimization or GP option is supplied by the driver.
CCPSX 1.02 rejects the `-G0` driver spelling, including with PSYQ.INI present;
this does not prove another spelling or direct frontend use was unavailable.

## OPEN comparison plan and verdict

The observed G8 default motivates one isolated comparison, not a profile
sweep. Recompile unchanged `open.render_init`, `open.render_unlit` and
`open.render_map` under the pinned 2.5.7 profile, changing only small_data
from zero to eight. Retain the compiler, CPU flag, optimization and assembler.
The refreshed per-function contracts are in the [display](open-display-init-pointers.md)
and [emitter](open-emitter-source-controls.md) notes. Map's owned four-byte
CVECTOR supplies a small-datum control alongside the large external runtime;
the three exact initialization functions supply unchanged-code controls.
Compare every resolved instruction and ordered call/data target, checking
explicitly for newly introduced GPREL16 references rather than ignoring them.

All six functions have identical resolved code and ordered targets under
G0 and G8; no text GPREL16 discriminator appears. The 44/468/132-byte exact
controls stay raw exact. Display remains 484 bytes/117 unequal aligned words,
unlit 680/52 and map 952/5. This original-driver default does not explain
these OPEN differences. It supplies no evidence for a canonical profile
change, an invented GP referent, or a new exact match.
