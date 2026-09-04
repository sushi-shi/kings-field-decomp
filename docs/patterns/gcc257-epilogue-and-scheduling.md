# GCC 2.5.7 probe: epilogue, `div` expansion, and scheduling evidence

## Retail evidence

A census over every admitted `GAME.EXE` function (`scripts` scratch analysis,
2026-09-01) classified each `jr $ra` by its neighbours:

| Form | Game functions | Vendored functions |
| --- | ---: | ---: |
| `jr $ra` with `addiu $sp,$sp,+N` in the delay slot | 373 | 178 |
| of which `lw $ra; nop; jr $ra; addiu $sp` | 103 | 60 |
| `addiu $sp; jr $ra; nop` | 0 | 0 |
| `jr $ra; nop` (no frame) | 81 | 151 |

Retail never restores `$sp` before the return jump. The 2.6.0 probe emitted
`lw $31; addu $sp; j $31` in reorder mode and neither maspsx (any modelled
ASPSX version) nor GNU `as` moves the `addu` into the delay slot when the
jump reads a register loaded two instructions earlier, so every framed
function stopped at `lw ra; addiu sp; jr ra; nop`.

Retail signed division carries the checked macro expansion
(`bnez; nop; break 7; li $at,-1; bne; lui $at,0x8000; bne; nop; break 6;
mflo`), for example `fixed6_ratio_step` at `0x80015128`.

Retail `jal` sites never contain a relocated `la`/`lw` half or a load in the
delay slot (`lui a0; addiu a0,lo; jal; nop` at `0x80014564`,
`lui s0; lhu s0; jal rand; nop` at `0x80016414`), which is consistent with
GNU-style reorder rules and with GCC's own delay-slot filling.

## Probe evidence

- Both historical DOS `CC1PSX` 2.6.0 binaries (Release 2.5 and the separate
  2.60 disk), run under headless DOSBox, emit assembly byte-identical to the
  native Decompals 2.6.0 rebuild for the probe wrapper
  `audio_play_spatial_default_range`. The compiler binary is not the variable.
- `ASPSX.EXE` refuses to run without its software key, so the assembler
  model stays maspsx.
- GCC 2.5.7 (Decompals old-gcc 0.17, `gcc-2.5.7-psx`) prints its own framed
  epilogue as `.set noreorder; lw $31; nop; j $31; addu $sp; .set reorder`,
  and hoists an argument load above `subu $sp` inside an explicit
  `.set noreorder` block. Both are retail forms
  (`memory_card_clear_events` at `0x8002b36c` starts with `lui/lw a0` before
  `addiu sp`).
- maspsx `--expand-div` reproduces the checked `div` expansion with
  `break 7`/`break 6` for the ASPSX 1.07 model.

Corpus: the 49 units enrolled before the profile change, compared as
normalized instruction plus relocation listings against the carved targets
(trailing alignment `nop` ignored):

| Compiler / flags (all with `--expand-div`) | Exact |
| --- | ---: |
| 2.6.0 `-O2`, committed profile before this change | 20 |
| 2.6.0 `-O2` plus an external `addu $sp` delay-slot swap | 29 |
| 2.6.0 `-O2 -fno-schedule-insns2` plus the swap | 32 |
| 2.6.3 `-O2` / `-O2 -fno-schedule-insns2` plus the swap | 29 / 32 |
| **2.5.7 `-O2`, no swap needed** | **34** |
| 2.5.7 `-O1`, `-O3`, `-O2 -fno-peephole` | 31, 34, 34 |
| 2.7.0, 2.7.1, 2.7.2, 2.7.2.3 `-O2` | 26 |
| 2.8.0, 2.8.1 `-O2` | 14 |
| 2.91.66 `-O2` | 12 |

The original opposite discriminator no longer holds: the current
`-mcpu=r2000` GCC 2.5.7 probe also matches
`weapon_records_load_and_mirror_angles` (`0x800150a8`) exactly, including the
`move v1,a1` schedule before the loop load. `menu_release_item_model`
(`0x8002af0c`) remains a separate scheduling-profile discriminator. The retail
compiler is still only narrowed to a 2.5/2.6-era MIPS GCC; the exact build
remains unproven.

## GCC 2.5.8 control

GCC 2.5.8 (built from the GNU archive tarball with the same PSX patch set as
the Decompals 2.5.7 rebuild, static i386) differs from 2.5.7 only in
`combine.c`, `cse.c`, `jump.c`, and nine lines of `mips.h`; `sched.c`,
`reorg.c`, `mips.c`, and `mips.md` are byte-identical. Substituted for
`cc1psx-257` under `kf try` across all 37 modules it reproduced exactly the
same 53 exact and 18 non-exact functions, so the 2.5.x point release is not
the missing discriminator and the remaining residues are not explained by
the combiner either.

## Consequences

- `probe-gcc257-o2-g0` (`cc1psx-257`, `-O2 -G0`, maspsx ASPSX 1.07 with
  `--expand-div`) is the default probe for new units.
- `probe-gcc260-o2-g0` stays available and keeps the one banked unit it
  alone matches. Per-unit profile choice is a recorded discriminator, not a
  steering device: a unit moves only when the change is exact and explained.
- Non-exact residues under 2.5.7 are single-instruction placements
  (`move` before a load, `sw ra` kept out of a branch slot) or register
  assignment. They remain unattributed until the optimizer sources are read.
