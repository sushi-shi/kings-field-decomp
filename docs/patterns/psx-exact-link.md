# PSX source and original-tool executable link

## Match plan and evidence boundary

Use the existing exact C source and the original library archives. Feed the
compiler's assembly directly to ASPSX, its object directly to PSYLINK, and
PSYLINK's CPE directly to CPE2X. Retain remaining file differences. No injected
padding, copied retail header, ELF section adjustment or library object-format
conversion is part of executable generation.

PSX `main` is 208 bytes at `80010028`, with fifteen text relocation sites.
Its C unit also owns two 20-byte path arrays and an 8-byte two-pointer table.
The seven SDK text members and SDK stack-size datum are library inputs, not
reconstructed game functions. The C source and SDK/API types are unchanged.

## Tools and ordinary commands

The compiler is the pinned GCC 2.5.7 PSX rebuild, using the existing `-O2 -G0
-mcpu=r2000` profile. The preserved ASPSX binary used by upstream's assembler
[test suite](https://github.com/mkst/maspsx/tree/746b895f02929ecd148af7b1f4ff05b69f973878/aspsx)
identifies itself as 1.07 and runs under DOSBox-X. Its SHA-256 is:

```text
83cfea6712cc444780614b111db6d5cf6046bb25d5070c734a8ae967641050c5
```

The flake pins the upstream assembler archive by SHA-256 and extracts only
this binary. It differs from both previously tested SDK-media ASPSX copies,
which stopped with a software-key/network-manager error. The exact protection
mechanism in those copies was not established. The working binary's historical
use by FromSoftware remains unproved; no modification is applied to it here.

The compiler writes Unix line endings. ASPSX's DOS text reader requires CRLF,
so file line endings are changed; all directives, instructions, labels and
literal contents pass through unchanged. ASPSX produces LNK v2 directly.
The PSX command file is ordinary linker input:

```text
        org $80010000
        include "U0000.OBJ"
        inclib "LIBSN.LIB"
        inclib "LIBAPI.LIB"
        regs pc=__SN_ENTRY_POINT
```

The original tools then run:

```text
aspsx -G0 -o U0000.OBJ U0000.S
psylink /c @LINK.LNK,PSX.CPE,PSX.SYM,PSX.MAP
cpe2x PSX.CPE
```

No library-member extraction order is supplied. Native PSYLINK selects and
places the original members through its own archive processing. No post-link
operation modifies its CPE or CPE2X's EXE.

## Result

The native map is:

| Range | Contents | Bytes |
| --- | --- | ---: |
| `80010000–80010028` | Compiled C path arrays | 40 |
| `80010028–800100f8` | Compiled C `main` | 208 |
| `800100f8–80010224` | Original SDK text | 300 |
| `80010224–8001022c` | Compiled C pointer table | 8 |
| `8001022c–80010230` | SDK stack-size word | 4 |
| `80010230–80010234` | SDK uninitialized `.sbss` | 4 |

All 560 initialized bytes agree with retail at their actual addresses.
The source's `.align 2` directives become native LNK alignment tag 8, which
PSYLINK's independent controls establish as four-byte alignment. No GNU ELF
minimum or section-renaming workaround affects this path.

The unchanged CPE2X 1.3 output is 4096 bytes and has the correct load origin,
load size and entry (`80010100`). It differs in 59 header bytes and seven
padding bytes. These are not repaired by copying retail metadata or inserting
the CPE-shaped tail. Header contents outside defined loader fields appear to
contain converter runtime residue; exact historical behavior remains open.
The [tail/BSS evidence](psyq-cpe-tail-and-bss.md) explains why the tail is not
an initialized source global.

## Verification

`tests/test_executable.py` compiles two independent C source units and links
native objects with the unchanged SDK archives, with retail access disabled.
It verifies original CPE records against the unchanged EXE, and separately
checks that an undefined source symbol fails without synthesized storage,
retail fallback or a stale executable. CPE inspection rejects truncated or
unknown records, and file comparison includes the whole header and tail.

The repository suite passes 720 tests, the native executable controls pass,
and `ruff check scripts tests` and `nix flake check -L` pass. The PSX function
remains strict objdiff 100%; no game function is newly reconstructed here.
Full `kf build` still fails data/ownership/target-placement gates. In
particular, removing the ELF section adapter restores the GNU analysis path's
16-byte `.data` alignment mismatch for PSX. Native ASPSX/PSYLINK placement
is independently correct as shown above; the analysis gate is not bypassed.

Run `kf link --image psx` in `nix develop` after `kf init`. The commands,
logs, native objects and complete-file report are under `build/link/psx/`.
The artifact has not been executed as a game.
