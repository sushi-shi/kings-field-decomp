# Compiler attribution probe: does GCC 2.4.1 (or any candidate) beat cc1psx-257?

Dedicated attribution pass over the `game-to-100` residue corpus. Question:
the `game` campaign is capped by residues where the rebuilt `cc1psx-257`
(Decompals old-gcc 0.17, GCC 2.5.7) **over-optimizes relative to retail** — it
drops redundant `andi`s retail kept, changes a small branch structure, fills
load-delay slots retail left, and makes different frame/register choices. The
linked Psy-Q libraries are Release-2.5-era, so the hypothesis was that retail
used a *less aggressive*, Release-2.5-era compiler — prime suspect the
**GCC 2.4.1 CC1PSX** from the same 1994-12-29 floppy collection. This note
records whether 2.4.1 runs here, and whether it or any other available
compiler/profile reproduces retail's codegen better than 2.5.7 `-O2`.

**Verdict: no.** GCC 2.4.1 now runs in this environment, but on the residue
batch it emits code **byte-identical to GCC 2.5.7 `-O2`** on most functions and
is **strictly worse where its optimizer diverges**. It makes the identical
"over-optimizations" (same dropped `andi`, same frame size, same register
allocation, same scheduling). The union of retail-exact functions reachable by
any 2.4.1 profile is a strict subset of those reachable by 2.5.7 profiles:
2.4.1 closes **zero** residues that 2.5.7 cannot. The real residue lever is the
2.5.7 **scheduling model** (`-mcpu` on/off, `-fno-schedule-insns`), which is
function-specific and matches retail for different functions under different
settings — an unattributed scheduling residue, not a compiler-version gap.
Keep `probe-gcc257-o2-g0`.

## Running GCC 2.4.1 CC1PSX in this environment (it works)

`compilers/gcc-2.4.1/CC1PSX` in the Psy-Q candidate store is a **bare i386
DJGPP COFF** image (`file`: *Intel i386 COFF executable*, magic `4c 01 03 00`),
with no MS-DOS/`go32` stub, so plain DOSBox cannot load it. The two GCC 2.6.0
CC1PSX binaries in the same store are *MS-DOS executable, COFF*: a full **go32
v2 extender** (strings: `go32 version %d.%d Copyright (C) 1994 DJ Delorie`,
`StubInfoMagic!!`) prepended to the appended COFF. The go32 stub finds its
payload COFF at the end of its own DOS image; the MZ header's page fields give
that size exactly:

- `gcc-2.6.0-release-2.5/CC1PSX.EXE` MZ header ⇒ DOS image size `0x1334a`,
  and the COFF magic sits at file offset `0x1334a`. So bytes `[0, 0x1334a)` are
  `[go32 v2 stub + StubInfo]` and the COFF follows.

A runnable 2.4.1 is therefore built by transplanting that stub onto the bare
2.4.1 COFF (a "frankenbinary"):

```
stub = CC1PSX(2.6.0-release-2.5)[0 : 0x1334a]   # go32 v2 + StubInfo
CC24GO.EXE = stub + CC1PSX(2.4.1)               # whole bare COFF appended
```

The go32 v2 extender loads the older 2.4.1 COFF without complaint and the
compiler self-identifies correctly:

```
 # GNU C 2.4.1 [AL 1.1, MM 40] Sony Playstation compiled by GNU C
 # Cc1 arguments (-G value = 0, Cpu = default, ISA = 1): -quiet -O2 -G0 -o
```

It accepts `-mcpu=r2000` / `-mcpu=r3000` (banner then reports `Cpu = r2000` /
`r3000`), so its instruction scheduler can be driven the same way as the 2.5.7
rebuild. It runs under headless `dosbox-x` with the same invocation the
`psylink_order_smoke` test uses (`-silent -fastlaunch`, `SDL_*=dummy`, mount a
work dir, `-c "CC24GO.EXE -quiet -O2 -G0 IN.I -o OUT.S"`). Preprocessing is done
once on the Linux side with the 2.5.7 `cpp` (cpp is not the variable under
test); only `cc1` runs in DOS.

Reproducibility (hashes; nothing binary is committed):

| file | sha256 |
| --- | --- |
| `gcc-2.4.1/CC1PSX` (bare COFF) | `d164b281…afee9548` |
| `gcc-2.6.0-release-2.5/CC1PSX.EXE` (stub donor) | `e65635ec…d0d6dd3` |
| `CC24GO.EXE` (frankenbinary) | `016efe5b…8ec724fe` |

`ASPSX.EXE` still refuses to run without its hardware/software key
("Software Data Key … not found"), so the assembler model stays maspsx and the
genuine Psy-Q `ccpsx` link path remains unavailable — but the 2.4.1 **compiler**
is no longer tooling-blocked.

## Probe harness

`cpp(2.5.7) → cc1(candidate) → maspsx → mipsel objdump`, diffed per claimed
function against the carved module target `build/delink/game/modules/<obj>`
using the exact normalization of `kf try` (`objdump -dr`, symbol-relative
branch targets, trailing `nop` trimmed, ordered relocations kept). Only the
`cc1` binary and its flags vary; maspsx flags (`--expand-div`, ASPSX 1.07) and
data-claim `.type/.size` annotation are the unit's committed profile for every
candidate. Native compilers run by absolute path; DOS compilers run under
dosbox-x. Similarity is `difflib` ratio; "EXACT" means identical instruction +
relocation stream.

## Residue batch (18 functions, 7 units, all named residue classes)

| unit / function | residue class |
| --- | --- |
| `entity_model_render` `render_actor` `0x8001e9a4` | redundant `andi`, frame size, regalloc, arg scheduling |
| `entity_model_render` `render_map_object` `0x8001ebb8` | branch-structure / arg scheduling |
| `entity_render` `render_floor_item` | redundant `andi 0xf0; andi 0xff` |
| `entity_render` `render_actor_sprite` | control (exact under R3000 sched) |
| `sprite_add_ft4` `func_80014314` | callee-saved register permutation (POLY_FT4) |
| `pad` (7 functions) | load-vs-frame-alloc scheduling |
| `map_events` (4 functions) | mixed scheduling / regalloc |
| `render_sprite` `func_8001e480` | load placement |
| `render_map_cells` | control (exact under R3000 sched) |

### Per-profile EXACT counts (identical instruction+reloc stream vs retail)

All `-G0`, maspsx `--expand-div` ASPSX 1.07. `(base)` is the committed
`probe-gcc257-o2-g0`.

| unit (n) | 257 O2 r2000 (base) | 257 O2 plain | 257 O1 | 257 O2 −fno-sched | 260 O2 | **241 O2 r2000** | **241 O2 plain** | **241 O1** |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| entity_model_render (2) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| entity_render (2) | **1** | 0 | 0 | 0 | 0 | **1** | 0 | 0 |
| sprite_add_ft4 (1) | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| pad (7) | 3 | **7** | 7 | 7 | 0 | 3 | **7** | 7 |
| map_events (4) | **2** | 0 | 0 | 0 | 0 | 1 | 0 | 0 |
| render_sprite (1) | 0 | 0 | 0 | **1** | 0 | 0 | 0 | 0 |
| render_map_cells (1) | **1** | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| **total (18)** | **7** | 7 | 7 | 8 | 0 | **5** | 7 | 7 |

Reading the table:

- **2.4.1 never beats 2.5.7 at matched flags.** `241 O2 r2000` = 5/18 vs
  `257 O2 r2000` = 7/18: 2.4.1 is *worse*, losing `map_events func_8003596c`
  and `render_map_cells` — the two functions where its own combiner/CSE output
  diverges from 2.5.7's, and it diverges *away* from retail. `241 O2 plain`
  = 7/18 = `257 O2 plain`, and `241 O1` = `257 O1`.
- On 5 of the 6 non-`pad` residue functions, `241 O2` output is **byte-identical**
  to `257 O2` (the only difference in the objdump diff is the label header). The
  two compilers share the MIPS backend for this code; 2.4.1 is not a "less
  aggressive" alternative.
- **260 is disqualified everywhere** (0/18), consistent with the recorded
  epilogue evidence (it cannot put `addiu $sp` in the `jr $ra` delay slot;
  373/373 retail framed functions do).
- No 2.4.1 profile makes any function exact that no 2.5.7 profile makes exact.
  The union of 2.4.1's exacts (`pad` 7, `render_actor_sprite`, `map_events
  func_80035708`) ⊂ the union of 2.5.7's exacts.

## Per-site evidence (the named residues)

**Redundant `andi` (the headline hypothesis) — 2.4.1 drops it too.**
`render_floor_item`, retail:

```
lbu  v0,2(s1)
nop
andi v0,v0,0xf0
andi v0,v0,0xff      # redundant after 0xf0; retail keeps it
beqz v0,...
```

Both `cc1psx-257` and `CC24GO` (2.4.1) emit only `andi v0,v0,0xf0; beqz` — the
redundant `andi 0xff` is dropped identically. In `render_actor`, retail keeps
`andi v0,s1,0xff; srl v0,v0,0x4`; both compilers fold it to a single
`srl v0,s3,0x4`. 2.4.1 performs the same redundant-mask elimination as 2.5.7;
the hypothesis that an earlier compiler would preserve retail's extra `andi`
is **false** for 2.4.1.

**Frame size — neither reproduces retail.** `render_actor` retail allocates
`addiu sp,sp,-168` and saves at `160..144`; both 2.5.7 and 2.4.1 allocate
`-160` (8 bytes smaller) with the same saves shifted down. This 8-byte
"phantom" frame slack (also seen on `player_death_update`) is unreproduced by
either compiler at any flag.

**Branch structure / argument scheduling — same residue in 2.4.1.**
`render_map_object`: retail forms an argument address (`addiu a0,sp,16`) *before*
the preceding `subu`; both compilers fill the load-delay slot with `nop` and
schedule the `addiu` later. `render_map_object` is the one batch function where
2.4.1-O2 and 2.5.7-O2 genuinely differ from each other, but both differ from
retail by the same amount (91%); 2.4.1 is not closer.

**Scheduling is the real lever, and it is function-specific.** `pad` is the
clean case. Retail `pad_stop_bad_identifier`:

```
lui  a1,%hi(DAT_8006bd88)
lw   a1,0(a1)          # independent global load first
addiu sp,sp,-24        # then the frame
sw   ra,16(sp)
```

The committed `-mcpu=r2000` (== `-mcpu=r3000`; both select the R3000A pipeline
model) **hoists the frame allocation to the top**, moving `addiu sp`/`sw ra`
ahead of the load and breaking the match — `pad` scores 3/7. Dropping `-mcpu`
(plain `-O2`) keeps the load before the frame and makes **all 7 `pad` functions
exact**, under both 2.5.7 and 2.4.1. Conversely `map_events`, `render_map_cells`
and `render_actor_sprite` are exact **only** under the R3000 schedule
(`-mcpu=r2000`), and `render_sprite func_8001e480` is exact **only** under
`-fno-schedule-insns`. No single global flag captures all of them; retail's
scheduler sits at a point the rebuilt 2.5.7 scheduler reaches for some
functions and not others, and 2.4.1 shares the *same* scheduler (its default,
no-`-mcpu` schedule equals 2.5.7's).

## Conclusion and recommendation

1. **Do not switch the probe to 2.4.1 (or 2.6.0).** 2.4.1 reproduces retail no
   better than 2.5.7 and is worse where it differs; 2.6.0 is disqualified by
   the epilogue. `probe-gcc257-o2-g0` stays the default. The Release-2.5-era
   library evidence does **not** imply a Release-2.5-era *C compiler* less
   aggressive than 2.5.7 — 2.4.1, the actual Release-2.5 GCC, optimizes these
   functions identically.
2. **The ~95-residue ceiling is a scheduling/regalloc residue, not a
   compiler-version residue.** It is unattributed: the genuine Psy-Q `ccpsx`
   scheduler (still key-blocked at the assembler/link stage) or a closer 2.5.x
   scheduler rebuild is the only remaining lever that could move it. The
   compiler frontend is no longer the tooling gap — 2.4.1 through 2.8.x all run.
3. **Actionable per-unit discriminator for the orchestrator (not applied
   here).** `pad` reaches 7/7 exact under plain `-O2` (no `-mcpu`) versus 3/7
   under the committed `-mcpu=r2000` profile, because its TU matches retail's
   *unscheduled* frame/load order. This is a legitimate recorded discriminator
   (exact and explained), the mirror image of the units that need `-mcpu=r2000`.
   It confirms `-mcpu` is a per-TU choice, not a global truth, and is worth a
   campaign-level review of which banked/residue units flip with it. Profile
   changes are a campaign decision and were deliberately not made in this pass.

Method note (why DOSBox/go32): the candidate compilers are 1994 DOS/DJGPP
binaries and the PS-X EXEs carry no compiler strings, so version attribution
can only be tested by controlled recompilation. go32 v2 under DOSBox is simply
how a DJGPP COFF `cc1` is executed on this host; the run's only evidential
content is the assembly it emits, compared byte-for-byte against the carved
retail objects.

## Per-TU profile assignments (campaign sweep)

The discriminator from the conclusion above was applied as a campaign. Every
`GAME.EXE` unit carrying a non-exact function (48 units, 93 non-exact
functions) was rebuilt under each candidate 2.5.7 scheduling profile and its
per-function EXACT set was compared against the banked default
(`probe-gcc257-o2-g0`, `-mcpu=r2000`):

| profile | `cc1` flags | model |
| --- | --- | --- |
| `probe-gcc257-o2-g0` (default) | `-mcpu=r2000` | R2000 schedule, attributed |
| `probe-gcc257-o2-plain` | *(none)* | generic 2.5.7 schedule (no `-mcpu`) |
| `probe-gcc257-o2-nosched` | `-mcpu=r2000 -fno-schedule-insns` | R2000, cc1 scheduler off |
| `probe-gcc257-o2-r3000` | `-mcpu=r3000` | R3000 schedule (swept, unused) |
| `probe-gcc257-o2-plain-nosched` | `-fno-schedule-insns` | generic, scheduler off (swept, unused) |

Selection rule (config-only; source untouched): a unit is reassigned only if a
profile's EXACT set is a **strict superset** of the default's — it must add at
least one byte-exact function and **drop none** (zero within-unit regression).
Ties keep the most-attributed profile (`-mcpu=r2000` preferred, then `plain`).
A profile that merely raises a fuzzy % without reaching a new EXACT, or that
trades one exact for another, is rejected.

The original sweep reported three flipped units. Later provider audits proved
that `game.audio_sequence_track` reconstructed five `LIBSND.LIB` routines from
`SSPLAY.OBJ` and `STOP.OBJ`, while `game.audio_sequence` reconstructed the
`SSOPEN.OBJ` public openers. Both units are now excluded rather than treated as
game compiler evidence. The later `game.audio_sequence_tick` and
`game.audio_sequence_envelope` reconstructions were likewise removed after
`SSCALL`, `DECRE`, `REPLAY`, `CRES`, `PAUSE`, and `TEMPO` archive evidence
identified all seven bodies as version-skewed LIBSND code. The corrected
game-only result is one unit and
**GAME exact 265 → 266 (+1)**, with no regressions in any image:

| unit | old profile | new profile | exacts | functions flipped |
| --- | --- | --- | --- | --- |
| `game.render_sprite` | `probe-gcc257-o2-g0` | `probe-gcc257-o2-nosched` | 0/1 → 1/1 (+1) | `func_8001e480` (`sprite_add_ft4` body) |

`render_sprite` reaches exact under both `nosched` and `plain-nosched`;
`nosched` is kept because it retains the attributed `-mcpu=r2000` and adds only
the documented scheduler-off lever. The former LIBSND units supply no evidence
about the compiler used for game-owned translation units.

The remaining surveyed game units did not move under any scheduling model (kept
`probe-gcc257-o2-g0`). This is itself attribution evidence: the residue ceiling
is not a global `-mcpu` on/off switch. The scheduling model is a genuine per-TU
discriminator for a small set of units, but most residues are a deeper
register-allocation / instruction-selection difference that no available 2.5.7
scheduling profile closes. `probe-gcc257-o2-g0` remains the correct default.
