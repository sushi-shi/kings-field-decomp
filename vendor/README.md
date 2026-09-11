# SDK interfaces and unused reference reconstructions

The reference C files in `src/` are **not used to build PSX.EXE, GAME.EXE or OPEN.EXE**.
The executable build uses the original Psy-Q SDK objects and libraries.

The headers in `include/` **are used by game code**. They provide guarded
SDK includes, missing API declarations and case-sensitive include shims.
Their public include names remain `psyq/*.h` and `sys/*.h`; the compiler
searches `vendor/include/` before the separately supplied SDK headers.

They preserve reference reconstructions of 13 retail Sony LIBETC routines:
the GAME interrupt tail and GAME/OPEN controller front ends. The supplied
SDK revision differs from retail in these routines, so the files remain
available for byte comparison and source inspection.

Their units retain `scope = "vendored"` in `config/units.toml` for those
comparison checks. They are excluded from game reconstruction progress and
banking. These files do not contain GAME/OPEN `main`, `InitHeap`, or the BIOS
allocator implementation.

See [provider evidence](../docs/vendored-functions.md) and the
[startup audit](../docs/patterns/startup-sdk-provenance.md). Original project
licensing does not grant rights to Sony/Psy-Q material.
