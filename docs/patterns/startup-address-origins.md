# Startup addresses: ownership and source origin

GAME and OPEN `main` received their `.bss` start and initial heap start as
compile-time integers. The lui/ori pairs and the folded word and byte counts
cannot come from a symbol reference under the pinned compiler, so retail
carried no relocation at those sites. Both store addresses equal the retail
`.bss` start, so they were taken from that program's own link layout. How
the heap starts were chosen remains unresolved. King's Field II differs:
its GAME `main` references a linker-resolved symbol and computes the heap size
at run time.

## Scope and evidence

The initial audit found seven explicit cached-RAM address literals after
excluding comments and `ADDRESS`/`DATA`/`RODATA` annotations: two in GAME
startup, two in OPEN startup, and three in the shared allocator. The four
startup addresses are now numeric startup-layout constants. The related
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
| GAME `80058060` | First word of the retail `.bss`: the 32-byte `MATRIX player_death_saved_color_matrix`, owned by `game.player_death`. Startup repeatedly writes zero to that word. | Literal at `800142a0/a4`; helper call at `800142b0`. The last `.sbss` object is LIBGPU `OTAG.OBJ`'s 8-byte `buf.2` at `80058058`, referenced only from OTAG code at `80054a10`/`80054a28`. Death code reaches the matrix through ordinary lui/addiu references at `80015198/19c`, `8001866c/670` and `800186a4/6a8`. |
| OPEN `800377a0` | First word of the retail `.bss`: the 24-byte SDK `CdlFILE cd_search_file`, owned by `open.resources`. | Literal at `8001376c/770`; helper call at `8001377c`. The last `.sbss` object is the same OTAG `buf.2` at `80037798`. The CD loaders use ordinary symbolic references. |
| GAME `800a0980` | Initial address handed to BIOS `InitHeap`; no curated object lives there. | Literal at `800142b8/bc`; call at `800142c4`. The highest referenced static object, LIBSND `VMANAGER` common `800a0878`, ends at `800a087a`. With PSYLINK's 8-byte common stride, static storage ends at `800a0880`, 256 bytes below the heap start. |
| OPEN `80080100` | Initial address handed to BIOS `InitHeap`; no curated object lives there. | Literal at `8001378c/790`; call at `80013798`. The same VMANAGER common at `80075958` is the highest referenced static object, so referenced storage ends at `80075960`: 42,912 bytes below the heap start. |
| GAME `157680` | Initial heap byte count, passed in the `InitHeap` call delay slot. | `800142c0/c8`; exactly `801f8000 - 800a0980`, folded by the compiler. |
| OPEN `177f00` | Initial heap byte count, likewise passed in the call delay slot. | `80013794/79c`; exactly `801f8000 - 80080100`, folded by the compiler. |
| GAME `67fe8`, OPEN `70218` | Counts passed to the repeated-word-store helper. | GAME `800142a8/ac`, OPEN `80013774/778`. Both equal `(801f8000 - store address) / sizeof(int)`, folded by the compiler. Both helpers repeatedly store through an unchanged pointer. |

The repeated stores do not clear a range: GAME `8001427c/280` and OPEN
`80013748/74c` branch back with `sw a2,0(a0)` in the delay slot; no instruction
advances `a0`. The only known callers are their respective `main` functions.
Replacing either helper with `memset`, or treating its count as a recovered
section size, would change the decoded behavior. A normal advancing fill with
these counts would clear static storage and the heap up to the stack
reservation; why the retail helper does not advance remains unknown.


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

A linker-resolved end-of-static-storage symbol, King's Field II's form, lets
startup follow code and storage changes, with the byte count computed at run
time. KF1's compiled numbers instead had to be kept in step with each link;
a stale heap start would let the allocator overwrite newly placed globals.

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

## Mechanism verdict

Under the pinned GCC 2.5.7, a `CONST_INT` address is emitted as `li` and
assembled by ASPSX 1.07 as `lui`/`ori` with no patch. A symbol is emitted as
`la` and assembled as `lui`/`addiu` with LNK patch types 82/84, the HI16/LO16
pair that PSYLINK fills. The linker only fills immediates; it cannot turn an
`addiu` into an `ori` or fold a subtraction. The SDK's own `2MBYTE.OBJ`
section expressions, `sectstart(.sbss)` and `sectend(.bss)`, use the same
lui/addiu patch pairs.

Controlled probes compiled `-O2 -G0` through `cc1psx-257` and ASPSX 1.07:

| Spelling | Object | Agreement with retail |
| --- | --- | --- |
| `#define BSS_START 0x80058060u`, `#define HEAP_START 0x800a0980u`; counts written as `(801f8000 - X) / sizeof(int)` and `801f8000 - X` | `lui 8005/ori 8060`, `lui 6/ori 7fe8`, `lui 800a/ori 0980`, `lui 15/ori 7680`; only the two `jal` patches | Identical to GAME `main`'s store and heap sequence |
| `extern u8 BSS_START[], BSS_END[]` with the same arithmetic | lui/addiu with type 82/84 patches, then run-time `subu` and `srl`; saves `$s0` | Not retail; this is King's Field II's form |
| `&player_death_saved_color_matrix` for the store, numeric heap start | lui/addiu patch pair and run-time count | Not retail; the object reference does not explain the store |
| `asm("BSS_START = ...")`, `.set` or `equ` definitions in a C unit | ASPSX 1.07 rejects all three | No local absolute-symbol route exists |

The verdict is therefore **proven** at the instruction level: the original C
saw integers for both addresses, and the counts were folded from them.
Function-wide, the numeric source compiles to strict 100% for both `main`s.

Both store addresses are the exact retail `.bss` start, immediately after the
same final OTAG `.sbss` word, in two differently laid-out programs. A
hand-chosen number would not land there twice, so the values were copied
or generated from each program's own link map. This boundary identity is
**validated**: the result rests on curated references rather than original
symbols. The PS-X headers also retain fragments of each program's own
alphabetical map listing: GAME has `...94C  TransposeMatrix` (retail
`8004e94c`), OPEN has `8001C4D8  _96_v...` (retail `_96_vec`). They confirm
that each build produced a map; they do not show how the constants were made.

The heap start remains a **candidate** origin with three admissible forms:
the true `.bss` end with unreferenced trailing commons (256 bytes in GAME and
42,912 bytes in OPEN); the end plus a 256-byte margin, which fits GAME exactly
but would require OPEN's static storage to end at `80080000`; or a
hand-chosen or stale value. PSYLINK allocates commons in a name-dependent
order, not object order: the native link interleaves SDK and game commons.
It ends with LIBSPU `_spu_EVdma` and `_spu_rev_offsetaddr`, then LIBSND
`_svm_okof1` and `_svm_okof2`. Retail ends with the same owner sequence: two
S_INI words, then two VMANAGER halfwords. An unreferenced game common could
sort after them, and retail bytes cannot show one.

King's Field II GAME `main` (`80013634`) loads `801f8000` with lui/ori, takes
`801da018` through lui/addiu, and computes the size with `subu` in the
`InitHeap` delay slot. That is the symbol form above, and `801da018` is that
program's referenced `.bss` end. KF2 therefore used a linker-resolved symbol,
while KF1 compiled numbers.

## Source and build model

`include/kf/{game,open}/startup_layout.h` define `GAME_BSS_START`,
`GAME_HEAP_START`, `OPEN_BSS_START` and `OPEN_HEAP_START` as the retail
numbers. The names describe the decoded roles; the original spellings and
header are not recovered. `main` derives the counts from the shared
`OVERLAY_STACK_BOTTOM`, `801f8000`, which is cached RAM end minus the SDK
default 32 KiB stack. The former zero-byte `.bss_start`/`.bss_end` sections,
their ASMPSX 2.34 dependency and the `BSS_START`/`BSS_END` linker labels are
removed: no King's Field object used them.

A numeric layout must be kept in step with the link. The original
maintainer had to do that. The reconstruction's native layout does not yet
match retail; for example, GAME's native `.bss` extends past `800a0980`. The
builder therefore checks the first link's map. If `.bss` starts elsewhere or
static storage reaches the heap start, it writes a refreshed header under the
link directory, with the native `.bss` start and a heap start no lower than
the 8-byte-aligned static-storage end. It then recompiles that header's
users and relinks. `build.json` records the source, linked and refreshed
values. The objdiff, `kf try` and `kf analyze` compilations use the checked-in
retail numbers. A retail-faithful layout needs no refresh.

The four former lui/ori relocation candidates in `relocs.tsv` are rejected as
`folded-integer-constant`. The decoded objects at the store addresses remain
established by their independent symbolic references.

## Final verdicts

| Function family, in both GAME and OPEN | Verdict |
| --- | --- |
| `main` | Exact. Numeric `.bss` start and heap start, with folded counts; KF1-specific mechanism proven; `.bss` start identity validated; heap-start derivation remains a candidate. |
| `repeat_store_word` | Exact. Non-advancing store preserved; count equals words from the `.bss` start to the stack bottom; why it does not advance remains unresolved. |
| `memory_malloc_checked` | RAM-base/extent check understood; SDK provides an authentic base-address definition. |
| `memory_set_allocation_mode` | Initial/rebased arena boundaries and budgets understood; original names not recovered. |
| `memory_capture_system_heap_start` | Inclusive arena end plus one; directly derived at runtime. |
| `memory_reset_system_heap` | Shared exclusive endpoint minus captured start; stack reservation corroborated by the SDK. |

Next useful evidence would be an original memory-map header or generator, a
map from the retail link, or proof of unreferenced trailing commons. Any of
these could settle the heap starts.
