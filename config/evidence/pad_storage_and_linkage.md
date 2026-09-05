# PAD data storage and exported identifier

Reviewed 2026-09-05 from `2cd4eb2`, in the isolated data-matching worktree.
This is vendor-source verification, not new game reconstruction progress.
Function bodies, signatures, addresses, extents, literal claims and relocation
sites are unchanged. Only the owned data declarations and their curated
storage/linkage model are corrected.

## Reproducible provider evidence

In the pinned `nix develop` environment:

```sh
sha256sum "$PSYQ_LIB/LIBETC.LIB" "$PSYQ_INCLUDE/LIBETC.H"
psyk list -r -d "$PSYQ_LIB/LIBETC.LIB"
```

The input SHA-256 values are:

- LIBETC.LIB: `571cfbbc00c34e3f1f0fd19eb54ba88ae1a15d1ba3bcb0d30e9d0fcfe42bc7be`
- LIBETC.H: `be7929aa6219cd5347639381aa62b59ea5f605ebef08390c7777d60e7c11fc34`

`LIBETC.H` declares `extern int PadIdentifier;`. The PAD member's records
independently establish the distinction between its local state and external
identifier:

| Record | Section | Offset / allocation |
| --- | --- | --- |
| Local symbol `pad_buf` | `.sbss` (50e9) | offset 0 |
| Local symbol `pad_status` | `.sbss` (50e9) | offset 8 |
| Uninitialized data | `.sbss` | 16 bytes |
| XBSS export `PadIdentifier` | `.bss` (50ea) | 8-byte allocation |

Both section declarations have alignment eight. The code's pad-word fixups use
`.sbss` base plus 0/8; the identifier fixups name its external symbol. The
archive's eight-byte allocation is not an eight-byte C integer: the supplied
header says `int`, and the retail instructions access one 32-bit word. Keep the
four-byte DATA claims; allocation extent and alignment require a separate model.

The supplied member is revision-skewed: its marker names `pad.c` v3.5 dated
1994-10-15, whereas both retail programs retain the v1.17 marker dated
1994-10-14 and extra identifier-check/diagnostic code. Thus section/linkage
attribution is **supported** by archive, header and retail topology, not claimed
as a byte-identical original PAD.OBJ. Its exported header contract contradicts
the previous private `s32` definition even though all six known retail identifier
references stay within PAD.OBJ. No-reference-outside is not static-linkage proof.

## Retail storage evidence

After `kf init`, inspect each image independently:

```sh
kf sema --image game disasm PadInit --blocks
kf sema --image game xref PadIdentifier
kf sema --image open disasm PadInit --blocks
kf sema --image open xref pad_buf
kf sema --image open dump 0x80037560 --size 0x2a0 --hex --no-disasm
```

OPEN's CPE conversion residue starts at `0x800375d8`, file offset `0x25dd8`:
`43 50 45 01 08 00 03 90 00 00 00 00`. The identical prefix occurs in GAME at
`0x80057e68`. OPEN's remaining loaded page is zero-filled through its header
load end `0x80037800`; the two private PAD words lie inside that tail.

| Image | `pad_buf` | `pad_status` | `PadIdentifier` |
| --- | --- | --- | --- |
| GAME.EXE | 0x80058020 | 0x80058028 | 0x8006bd88 |
| OPEN.EXE | 0x80037760 | 0x80037768 | 0x80049528 |

In both PadInit copies, the same offsets from the function entry perform:

- +0xc/+0x10: LUI/SW stores incoming `$a0` into `PadIdentifier`;
- +0x14/+0x18: LUI/SW stores zero into `pad_status`;
- +0x1c: `li v0,-1`, then +0x20/+0x24 LUI/SW initializes `pad_buf`;
- +0x28: first conditional branch, with the `0x20000001` argument's LUI in
  its delay slot; only afterwards can either call receive the buffer address.

PadRead later loads and complements `pad_buf`; pad_status has only the init
store. There are four reviewed pad_buf references, one pad_status reference and
six PadIdentifier references per image. The BIOS service behind PAD_init2 is
still an unresolved indirect boundary, not a newly proved buffer-capacity path.
The archive's `.sbss` records, paired retail spacing, write-before-use behavior
and CPE tail together support uninitialized storage, not explicit loaded zeros.

OPEN's two `data.tsv` rows therefore remain present at the same addresses and
four-byte extents, but change from `defined` to `bss`. Their identity rows use
`storage=bss`, and source drops `= 0`. Both programs define `int PadIdentifier`
with external linkage and obtain its authentic declaration through `LIBETC.H`.
The private pad words and diagnostic functions remain static. No payload range
is deleted, renamed to invented lore or enlarged to imitate allocation padding.

## Per-function snapshot and kept verdict

The pre-edit pass read `addr`, `disasm --blocks`, `xref`, `xref --callees`,
`strings` and `match` for each row below in each image. Existing complete
function dossiers remain in `game_vendor_pad.tsv` and `open_vendor_pad.tsv`;
their earlier private-identifier/load-zero statements are superseded here.
The contiguous bands are bounded by the LIBETC INTR tail before PadInit and
LIBAPI CloseEvent after the final diagnostic. No TU boundary is moved.

| Function | GAME.EXE VA | OPEN.EXE VA | Body | Preserved semantics | Before / after objdiff |
| --- | --- | --- | --- | --- | --- |
| PadInit | 0x800500b8 | 0x8002fe8c | 0x74 | Signed incoming identifier; three word stores; PAD_init2 or diagnostic; ResetCallback; selected result retained in s0; SP restored in return slot | 100% / 100% each |
| PadRead | 0x8005012c | 0x8002ff00 | 0x44 | Identifier load before frame; PAD_dr or diagnostic; 32-bit NOR result; RA load separated by NOR; SP restored in return slot | 100% / 100% each |
| PadStop | 0x80050170 | 0x8002ff44 | 0x3c | Identifier load; StopPAD2 or diagnostic; shared void epilogue with load-delay NOP | 100% / 100% each |
| pad_init_bad_identifier | 0x800501ac | 0x8002ff80 | 0x30 | K&R no-parameter form; identifier as printf argument; retained incidental v0; unchanged format and NOP call slot | 100% / 100% each |
| pad_read_bad_identifier | 0x800501dc | 0x8002ffb0 | 0x30 | Void printf diagnostic; signed `%d`; unchanged format, call slot, load separator and return slot | 100% / 100% each |
| pad_stop_bad_identifier | 0x8005020c | 0x8002ffe0 | 0x30 | Void printf diagnostic; signed `%d`; unchanged format, call slot, load separator and return slot | 100% / 100% each |

The raw `kf try` listing reports named-symbol versus `.text`-relative calls
to the private diagnostics as differences. These are not changed instructions
or new call destinations: rebuilt native objdiff resolves each to the same
function, and all twelve exact verdicts above use its strict 100% score.
There is no source steering or new unattributed instruction-selection residue.

## Unresolved allocation boundary

The pinned GCC 2.5.7 `-O2 -G0` source probe emits `.comm PadIdentifier,8`
and `.lcomm pad_buf,8` / `.lcomm pad_status,8` for these four-byte C words.
That independently preserves external versus local allocation intent before
the maspsx adapter turns both forms into one `.bss` section. The adapter's
`sdata_limit=0` classification and BSS emission are the relevant next controls;
enabling a small-data code-generation option without checking its GP-relative
instruction effects would not be a justified correction.

The current target writer and probe adapter still flatten the PAD allocation
classes into `.bss`. They cannot place the private word pair and the distant
exported identifier at one section base. Changing the source declarations does
not justify overriding symbol addresses, adding a large gap, moving the owner
or declaring the roundtrip successful. Both units stay rejected by the default
placement gate. Explicit `.sbss` versus external-BSS allocation support, checked
against native assembler records, is the next required modeling step.

Likewise, source objects still contain assembler-rounded allocation extents.
Their data sections are not exact merely because the twelve function bodies
are. Vendor verification remains excluded from the game denominator and bank.

Verification: all twelve PAD function scores and all other 472 report rows
remain unchanged. Only the two PAD source objects and module targets change;
all per-function targets remain identical. The full build retains 354 exact
game functions and fails the known data/placement/reachability gaps. All 459
local tests, Ruff and `nix flake check -L` pass. The flake suite has 49 expected
local-retail/oracle skips; the new SDK header and archive controls run there.
