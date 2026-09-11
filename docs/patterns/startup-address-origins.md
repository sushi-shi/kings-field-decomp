# Startup addresses: ownership and source origin

The startup literals describe two different things: writes into program-owned
storage and the initial heap policy. Their numerical values are decoded facts;
their original macro names, definitions, generation mechanism, and maintainer
are not recovered. A matching literal is not proof of a recovered source
reference. This review supersedes earlier claims that retail retained evidence
of "no relocation" or that every startup destination was a proved BSS start.

## Scope and evidence

The initial audit found seven explicit cached-RAM address literals after
excluding comments and `ADDRESS`/`DATA`/`RODATA` annotations: two in GAME
startup, two in OPEN startup, and three in the shared allocator. The four
startup address literals have since been replaced by section boundaries. The related
heap lengths, repeated-store counts, and arena budgets are included below.
This is a source-literal census, not a census of every numeric instruction or
address introduced by SDK macros.

Both images were hash-validated through `kf init`. The review read each
startup/helper and allocator function's image-qualified disassembly/CFG,
incoming/outgoing references, strings and match state; the store destinations'
owners and consumers; adjacent data; source history; and original SDK objects
and headers. Raw pointer-word searches found none of the six non-base RAM
addresses below in either loaded payload. Nearby instruction-pair searches
were reviewed against the actual functions rather than promoted automatically.
Generated dossiers and native compiler controls are under the ignored
`build/startup-address-audit/` directory.

## Program-specific addresses and sizes

| Image / value | Actual use and owner | References and original-source verdict |
| --- | --- | --- |
| GAME `80058060` | First word of the 32-byte `MATRIX player_death_saved_color_matrix`, currently owned by `game.player_death`. Startup repeatedly writes zero to that word. | Literal at `800142a0/a4`; the helper is called at `800142b0`. Independent symbolic references at `80015198/19c`, `8001866c/670`, and `800186a4/6a8` use the matrix in death initialization/fading. The coincidence identifies the affected object, but does not prove startup originally named that matrix. |
| OPEN `800377a0` | First word of the 24-byte SDK `CdlFILE cd_search_file`, private to the current `open.resources` owner. | Literal at `8001376c/770`, helper call at `8001377c`. Twelve validated owner/interior references in `cd_file_load_allocated` and `cd_file_load_into`; the object is filled through `CdSearchFile` and its sector/size fields are consumed by the loaders. Startup's original reference mechanism remains unresolved. |
| GAME `800a0980` | Initial address handed to BIOS `InitHeap`; no curated object lives at that address. | Literal at `800142b8/bc`, call at `800142c4`. The last currently admitted BSS object ends at `800a087a`; that census is incomplete and cannot establish the actual section end. Do not invent a heap-start global from this argument. |
| OPEN `80080100` | Initial address handed to BIOS `InitHeap`; no curated object lives at that address. | Literal at `8001378c/790`, call at `80013798`. The last currently admitted BSS object ends at `8007595a`. The much larger gap rules out simply treating the last admitted datum as the heap's defining boundary. |
| GAME `157680` | Initial heap byte count, passed in the `InitHeap` call delay slot. | `800142c0/c8`; exactly `801f8000 - 800a0980`. A folded subtraction from a shared heap endpoint is a valid source hypothesis. |
| OPEN `177f00` | Initial heap byte count, likewise passed in the call delay slot. | `80013794/79c`; exactly `801f8000 - 80080100`. It follows the same policy as GAME. |
| GAME `67fe8`, OPEN `70218` | Counts passed to the repeated-word-store helper. | GAME `800142a8/ac`, OPEN `80013774/778`. Both equal `(801f8000 - startup_address) / sizeof(int)`. This measures words through the shared stack boundary, not just an independently established BSS extent. Both helpers repeatedly store through an unchanged pointer. |

The repeated stores do not clear a range: GAME `8001427c/280` and OPEN
`80013748/74c` branch back with `sw a2,0(a0)` in the delay slot; no instruction
advances `a0`. The only known callers are their respective `main` functions.
Replacing either helper with `memset`, or treating its count as a recovered
section size, would change the decoded behavior.

The word-count calculation independently reaches the same endpoint in both
images, without assuming that the initial heap address is the BSS end:

```text
GAME: 80058060 + 4 * 67fe8 = 801f8000
OPEN: 800377a0 + 4 * 70218 = 801f8000
```

That endpoint is cached RAM end minus the 32 KiB stack reservation. A normal
advancing fill with these counts would cover storage and the initial heap up
to the reserved stack. The actual retail function does not advance its pointer;
the reconstruction preserves that behavior rather than replacing it with
`memset`. The arithmetic relationship is established; the original definition
names and why the helper has this behavior remain unknown.

The store destinations are near the transition from private startup/library
storage to larger game allocations. They belong to different semantic object
families in the two programs. A shared startup boundary definition is therefore
a plausible explanation; direct references to those particular matrix/file
objects are not established. The preceding SDK/PAD storage also means neither
address is proved to be the start of all uninitialized storage.

## Shared memory policy and its maintainers

| Definition | Meaning, users and evidence | Ownership / source mechanism |
| --- | --- | --- |
| `80000000` | Cached RAM segment base. `memory_malloc_checked` adds it modulo 32 bits and accepts only offsets `0..1fffff`; GAME `8001aac8/acc`, OPEN `80015dec/df0`. | Hardware/SDK definition: preserved `R3000.H` defines `K0BASE` and `PHYS_TO_K0`; `LIBGS.H` defines `PSBANK`. This is not an address of a game object. The original choice of macro versus literal is unknown. |
| `200000` | Two MiB RAM extent, giving the allocator's inclusive offset limit `1fffff`. | Platform constraint used by game allocation policy. Changing TU order does not change it. |
| `801f8000` | Exclusive endpoint of both initial heaps and later `memory_reset_system_heap` calls. GAME `8001abd8/1abe4`, OPEN `80015efc/15f08`; each subtracts `memory_arena.system_heap_start`. | Equals cached RAM end `80200000` minus the SDK default 32 KiB stack reservation. The game embeds the endpoint; it does not read `_stacksize` in these functions. Original shared definition and responsibility for keeping it in sync remain unknown. |
| `801effff` | Inclusive initial arena limit, set by allocation mode zero. GAME `8001ab58/60`, OPEN `80015e7c/84`. `memory_capture_system_heap_start` adds one, making the following heap start `801f0000`. | Game memory-policy boundary: RAM end minus 64 KiB, minus one. It leaves a 32 KiB interval before the heap endpoint. This is not a movable datum. |
| `100000` | Mode zero's one-MiB `malloc` request in both overlays. | Game allocation budget. The code subsequently sets the arena limit independently; the limit must not be inferred as `returned_pointer + request_size`. |
| GAME `fefff`, OPEN `112fff` | Inclusive last-byte offsets used when rebasing the arena onto its current cursor. | Overlay-specific budgets of `ff000` and `113000` bytes. These are offsets added at runtime, not absolute addresses or linker placements. |

`memory_arena` owns the changing state: start/end, current allocation cursor,
LIFO entries, captured system-heap start and computed size. The mode and reset
callers in GAME initialization/resources and OPEN controller/resources maintain
that state. `InitHeap` receives an address and byte count; it does not discover
the program's static-storage end for its caller.

The initial boundary is consumed by each overlay's `main` when it calls
`InitHeap`. Later `memory_malloc_checked` calls `malloc`, and
`memory_set_allocation_mode(KF_MEMORY_CREATE_ARENA)` obtains the initial
one-MiB arena through that wrapper. `memory_allocate` either advances the
arena cursor or calls the wrapper in heap mode. These consumers need the
allocator's state, not repeated references to the original section boundary.
The later `memory_reset_system_heap` uses a boundary derived from the arena,
not the original startup heap address.

A linker-supplied end-of-static-storage symbol would let startup adjust the
initial heap whenever code or global storage changes. The available byte
count must change with it. A stale fixed start instead lets the allocator
overwrite newly placed globals. This explains why such a symbol would be
useful; it does not prove that the two retail starts equal `.bss` boundaries.

There is concrete SDK ownership evidence for the stack and ordinary startup:

- `LIBSN.LIB/SNDEF.OBJ` defines `_stacksize` as initialized `00008000`.
  Object SHA-256: `5aa187ff42906dbc2ee0213a3e4cccac010fa0ef1ffee5a349a28ca60856dae9`.
- `LIBSN.LIB/SNMAIN.OBJ` uses real patch expressions `sectstart(.sbss)` and
  `sectend(.bss)` to clear storage and derive a heap start. Its heap-size
  calculation references `_stacksize`. Object SHA-256:
  `f8a1f1b8b0aa9b25c0884bc524648c4b9ede5801e61df7649a9596aa3b674897`.
- Retail PSX uses this ordinary startup family. Its loader explicitly clears
  the overlay EXEC stack fields; GAME and OPEN retain the established stack.
- GAME/OPEN use the small `NONE2.OBJ` startup family: it establishes GP and
  tail-calls `main`; their game code supplies the heap initialization.

Thus SDK/linker ownership of ordinary startup boundaries is demonstrable.
It does not identify the missing GAME/OPEN project definition or a particular
original developer. The preserved SDK samples also contain centrally named
fixed addresses for externally loaded TIM/TMD/audio assets; fixed addresses
were available as a source mechanism, not evidence that these game values
were arbitrary or maintained the same way.

For example, `demo/GRAPHICS/TMDVIEW/TMDVIEW5/TUTO1.C` defines `TEX_ADDR`
as `80010000` and `MODEL_ADDR` as `80040000`. Its `MAKEFILE.MAK` loads the TIM
and TMD at those same addresses with `pqbload`, and links the program at
`80080000`. The sample's C definitions and load recipe jointly maintain that
memory map. The source and makefile SHA-256 values are respectively
`61d6864cd22706e75ee730ad72f028dbc2425dc54a579819445ccd6e0cc8be3b` and
`481861d1242440eaa786eb34e1353954c820e8e498c35e2b6d52c3c883fc2ca2`.
This is concrete period practice, not proof of the game's missing build recipe.

## What the compiler can and cannot establish

### Comparisons with other PS1 projects

The local reference checkouts show more than one startup convention. These
are other games' decompilation models and preserved runtime sources, not
recovered King's Field source.

- Silent Hill, checkout `92cce2d9b7140a68ff75cdf39b263cf87357b55d`:
  [`snmain.s`](https://github.com/shdecompilations/silent-hill-decomp/blob/92cce2d9b7140a68ff75cdf39b263cf87357b55d/src/main/libsn/snmain.s)
  clears BSS, derives the heap from the BSS end and `_stacksize`, calls
  `InitHeap`, then calls `main`. The game's
  [`main.c`](https://github.com/shdecompilations/silent-hill-decomp/blob/92cce2d9b7140a68ff75cdf39b263cf87357b55d/src/main/main.c)
  supplies `_ramsize` and `_stacksize`, then initializes services and loads
  overlays. This checkout explicitly marks its BSS-end override as a hack:
  `main_BSS_END` is bound to `g_bodyProg` because the reconstructed section
  extent disagrees with the startup value. That override is not evidence for
  King's Field's boundaries. SH1 also has a separate fixed-budget file-system
  arena, initialized by `Fs_QueueInitialize` through `Fs_InitializeMem`.
- Digimon World, checkout `0388847013ae1769fac660d96554946f094e6733`:
  [`main.c`](https://github.com/jype0/dw_decomp/blob/0388847013ae1769fac660d96554946f094e6733/src/main/main.c)
  calls `initializeHeap` first. That helper aligns `_end`, subtracts the stack
  reservation from `_stack_addr`, and passes the resulting region to
  `InitHeap3`. The preserved Metrowerks
  [`_psstart.c`](https://github.com/jype0/dw_decomp/blob/0388847013ae1769fac660d96554946f094e6733/src/main/_psstart.c)
  documents a runtime revision that moved C heap initialization into `main`,
  retaining startup initialization for C++ static constructors. Its 1996
  provenance is later than King's Field. In this reconstruction's
  [`main.ld`](https://github.com/jype0/dw_decomp/blob/0388847013ae1769fac660d96554946f094e6733/config/main.ld),
  `_end` names a stored word containing `main_BSS_END`; it is not an array
  label whose own address is the heap boundary.
- Rage Racer, checkout `c5273505acbe282aa92f88cff52065fbc5007aed`:
  [`_start.s`](https://github.com/khasinski/rage-racer-decomp/blob/c5273505acbe282aa92f88cff52065fbc5007aed/src/main/PAL/main/boot/_start.s)
  uses `main_BSS_END` for clearing and heap setup before entering `MainLoop`.
  Its address pairs are `lui/addiu`, as in the supplied Psy-Q startup family.

These comparisons establish that heap setup may live in startup or in game
initialization, and that section-derived boundaries and fixed arenas can
coexist. Neither using an original allocator nor naming an address repairs a
stale caller-supplied memory region. They do not establish the origin of
King's Field's `lui/ori` startup pairs.

### Pinned compiler controls

With the pinned GCC 2.5.7/ASPSX 1.07 path, these two control bodies produce
identical native text and call patches:

```c
InitHeap((void *)0x800a0980, 0x157680);
```

```c
#define HEAP_START 0x800a0980
#define HEAP_END   0x801f8000
InitHeap((void *)HEAP_START, HEAP_END - HEAP_START);
```

The definitions could have been handwritten or generated before preprocessing;
the object cannot distinguish those origins. Their names above are explanatory
control names, not recovered original symbols.

An ordinary `extern char heap_start[]` reference instead produces `lui/addiu`
and native HI16/LO16 patches. The constant produces retail's `lui/ori`.
GAME's startup store makes the distinction especially visible: the literal
uses high half `8005` and `ori 8060`, while the actual matrix references use
carry-adjusted high half `8006` and `addiu 8060`. A plain `&matrix` replacement
does not explain both forms under this probe.

This narrows the source hypotheses; it does not prove the historical compiler
or eliminate a generated memory-map constant. The retail EXEs retain neither
symbols nor original relocation tables. Missing rows in our curated relocation
inventory are not proof that the original objects had no relocations.

The two startup structure references were missing from `relocs.tsv`. They are
now recorded at GAME `800142a0/a4` targeting
`player_death_saved_color_matrix`, and OPEN `8001376c/770` targeting
`cd_search_file`. The decoded destinations and writes into those objects are
established. These rows retain `lui+ori` and candidate status because the
delinker's ELF HI16/LO16 contract cannot represent their unsigned-low semantics;
that tooling limitation does not erase the object references. The original
source spelling and relocation mechanism remain separate questions.

The two heap arguments are also recorded as relocation candidates at GAME
`800142b8/bc` and OPEN `8001378c/790`, labelled `initial_heap_start` within
each image. This names their decoded role, not a new allocated object or a
validated `.bss` boundary. Their region remains RAM until section ownership
is established. No absolute linker definition was added.

Isolated full-main trials replacing only the heap-address macro with an
`extern unsigned char initial_heap_start[]` reference both change retail's
`ori` into `addiu` and introduce HI16/LO16 patches. The trials therefore do
not preserve strict exactness, independently of the missing target patches.
The trials and first instruction divergences are in
`build/startup-address-audit/heap-symbol-trials/`. The current source keeps the
symbolic model described below; those earlier literal matches do not justify
restoring fixed startup addresses.

## Current source and native boundary declarations

Both `main.c` files now use `BSS_START` for the repeated store and `BSS_END`
for the initial heap. They derive the repeat count and heap size from the
shared RAM/stack policy in `include/kf/overlay.h`. No per-image startup address
or length is supplied to the build. These are reconstruction names for section
boundaries, not a claim to have recovered the original header spelling.

`config/link/overlay_bounds.asm` exports labels in two empty sections.
PSYLINK places them immediately before and after `.bss`, after `.sbss`.
The native object has zero storage bytes, no instructions, and no patch
records. The linked symbols were checked against the native maps in both
images and equal the actual `.bss` start and exclusive end. COMMON allocations
remain inside those boundaries; no global is forced to an individual address.

The pinned ASPSX 1.07 compiler assembler has no named-section directive, and
the Release 2.5 macro assembler requires a software key. Boundary declarations
therefore use a separately hash-pinned native ASMPSX 2.34 from the preserved
[Frogger SDK tree](https://github.com/HighwayFrogs/frogger-psx/blob/fa2d5185b19ae89aaceeb47b2828369e06566edf/sdk/bin/SDK4.0/DOS/ASMPSX.EXE).
This affects only these zero-byte declarations; all C output
still uses the same ASPSX 1.07. Tool hashes are recorded in `build.json`.
This assembler choice is build support, not historical attribution to King's
Field. The original meaning of the retail section boundaries remains a source
model to refine with further ownership evidence.

The non-advancing helper remains retail-exact. The main functions no longer
claim strict byte equality: real symbol references and size arithmetic replace
the folded constants. Matching should expose that remaining source/toolchain
question rather than dictate hardcoded addresses in C.

## Source history and final verdicts

The current literal spelling entered reconstruction in GAME commit `b5ebf9f1`
and OPEN commit `81aa366c`. Commit `f88ba981` gave the values macro/enum names;
it did not recover their historical definitions. Those repository changes
identify reconstruction decisions, not the original game's maintainer.

| Function family, in both GAME and OPEN | Verdict |
| --- | --- |
| `main` | Symbolic section boundaries and derived RAM/stack sizes retained; calls and order preserved; original boundary spelling and constant-folding mechanism remain unresolved. |
| `repeat_store_word` | Non-advancing store preserved; count equals words from startup address to stack bottom in both images; original intent remains unresolved. |
| `memory_malloc_checked` | RAM-base/extent check understood; SDK provides an authentic base-address definition. |
| `memory_set_allocation_mode` | Initial/rebased arena boundaries and budgets understood; original names not recovered. |
| `memory_capture_system_heap_start` | Inclusive arena end plus one; directly derived at runtime. |
| `memory_reset_system_heap` | Shared exclusive endpoint minus captured start; stack reservation corroborated by the SDK. |

The initial review added four missing structure/heap-reference candidates to
the relocation inventory. The follow-up retained symbolic startup source and
derived sizes instead of the literal spellings. Next useful evidence would be an original project memory-map
header/generator/link command, an original object carrying these references,
or another build of the same startup source whose values can be compared with
independently recovered section boundaries. Guessing a name, replacing an
address with the object currently there, or forcing linker placement does not
close those missing source facts.
