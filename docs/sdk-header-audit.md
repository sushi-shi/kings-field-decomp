# Classic SDK header audit

Classic uses GCC 2.5.7 with the original headers from the hash-pinned Psy-Q
Release 2.5 `PSXLIB/INCLUDE` directory. The compiler, SDK and library revisions
remain working probes, not a proven attribution of the retail toolchain.

The wrappers include those original headers. They do not replace SDK library
implementations. The C++ declaration adapters in `sdk.h` and `libc.h` are
excluded from classic by generation; classic keeps the SDK's unprototyped C
declarations for `AddPrim`, `DrawOTag`, `SetSemiTrans`, `memcpy`, `memset`,
`malloc` and `free`.

## Removed duplication

`psyq/kernel.h` now selects `LANGUAGE_C`, includes the SDK's `ASM.H` for
`NREGS`, then includes `KERNEL.H`. The SDK supplies `struct EXEC` and its
other kernel layouts; the project no longer copies `struct EXEC`.
`tests/test_sdk_headers.py` checks the preprocessor's definition owner,
the 60-byte structure, and generated field loads with GCC 2.5.7.

## Remaining compatibility interfaces

The complete pinned header directory was checked for the supplemental APIs.
Occurrences inside comments and macros are not declarations.

| Wrapper | Original headers used | Why supplemental declarations remain |
| --- | --- | --- |
| `psyq/sdk.h` | `SYS/TYPES.H`, `LIBGTE.H`, `LIBGPU.H`, `LIBETC.H` | `ReadSZ2` is undeclared. `LIBGPU.H` declares `SetPolyGT`, whereas the linked interface used by the game is `SetPolyGT3`. |
| `psyq/cd.h` | `LIBCD.H` | No `CdReadSync` declaration. |
| `psyq/audio.h` | `LIBSND.H` | No declarations for the internal `Snd_play` and `Snd_stop` entry points. |
| `psyq/pad.h` | `LIBETC.H` | This header supplies button masks and `PadIdentifier`, but no `PadInit`, `PadRead` or `PadStop` declarations. |
| `psyq/kernel.h` | `ASM.H`, `KERNEL.H`, `SYS/FILE.H` | Kernel layouts and file constants are present; the BIOS, event, heap, card and executable-loading function declarations used here are absent. |
| `psyq/libc.h` | `MEMORY.H`, `MALLOC.H`, `RAND.H` | No declarations for `rand`, `printf`, `exit`, `strcpy` or `strcat`. `STDIO.H` is a placeholder; `STRINGS.H` declares other string functions. |

Retaining these declarations preserves known return types, argument conversions
and callback signatures instead of relying on implicit `int` calls. A C++
requirement alone is not a reason to add a replacement declaration to classic.

The guarded wrappers prevent repeated definitions in SDK headers without guards.
`sys/types.h` and `sys/fcntl.h` forward case-sensitive host includes to the SDK's
uppercase filenames.

Classic's `stdarg.h` now comes from GCC 2.5.7, independently of the SDK,
whose header directory supplies none. Nix extracts the original `gstdarg.h`
and `va-mips.h` from the pinned GNU source archive, installing `gstdarg.h` as
`stdarg.h` just as GCC's makefile does. Neither file is copied into the export.
Classic supplies the GCC/MIPS driver definitions required to select those
headers. Its build records both header hashes. The matching header remains
on master; source's C++ view uses compiler builtins.
