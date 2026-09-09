# Data-owner recovery in GAME and OPEN

## Campaign plan and starting evidence

The isolated campaign starts at `48132a4e`. A fresh `kf init`, full build and
`kf verify data --image game --image open --detail --coverage` reproduce
GAME **8/44** and OPEN **3/20**, with zero missing artifacts. Preserve every
banked function, the strict byte/addend checks, and the complete data-owner
denominator. A passing object does not establish whole-image equality.

The first campaign step is an assembler-format correction, with no C function
edits. Compare the pinned ASPSX 1.07 and GNU analysis objects independently of
retail claims. Synthetic controls must cover section alignment, explicit
interior alignment, complete section bytes, local reservations, and the original
PSYLINK 1.17's linked symbol addresses. Only an independently demonstrated
format difference can change the analysis-object contract. Source widths,
literal bytes, reservation extents, symbol offsets and relocations stay strict.

Initial native observations:

- ASPSX emits raw alignment tag 8 for `.rdata`, `.text`, `.data`, `.sdata`,
  `.sbss`, and `.bss`. The existing original-linker calibration establishes
  that this tag means four-byte section alignment. GNU's default sixteen-byte
  `.data`/`.bss` alignment is a different format's convention.
- ASPSX `.align 0` through `.align 4` changes the **relative** offset of a
  following byte to 1, 2, 4, 8, and 16; all section tags remain 8. Original
  linker controls must verify the distinction before changing generated ELF.
- `.comm` emits individual XBSS records; `.lcomm` emits local offsets into a
  reservation section. Treating both as one ordered `.bss` block invents an
  allocation relationship. This needs a separate allocation contract, not an
  exception to the current section placement rule.
- OPEN's native `memory.c` object reserves 80, 8 and 8 bytes for the arena and
  two heap globals. Retail identities describe 80, 4 and 4 bytes, with the
  latter two adjacent. This is a real source/allocation question even after
  the ELF alignment difference is corrected.

Subsequent source campaigns will separately review literal-section boundaries,
allocation classes and owning aggregates, initialized-data grouping and packed
selectors. Each edited function requires its matcher snapshot before edits;
claim-only corrections must record the literal/relocation/adjacent-owner
evidence. Residual switch-table addends must be traced to their destination
instructions and kept failing until reconstructed honestly.

Before handoff, rebuild affected units, run the full data and function checks,
Ruff, repository tests, diff checks and flake checks for tooling changes. Record
the result for every original owner, including any unresolved failures.

## Native section declarations: verified result

Five fresh ASPSX objects with `.align 0` through `.align 4` were independently
linked by PSYLINK at `0x80010004`. Complete CPE payloads, including interior
zeros and explicit trailing zeros, agree with the expected bytes. Even the
`.align 4` case has a four-byte section base and its second symbol at
`0x80010014`. All six standard section declarations retain LNK tag 8.

The C-to-analysis-ELF path now represents this native constraint in
`sh_addralign`; direct GNU assembly retains GNU's own contract. The conversion
accepts only ASPSX 1.07 and its standard allocated sections. It does not consume
retail claims or change payload bytes, offsets, extents, symbols, relocations or
COMMON rounding. Every object records the conversion and original GNU values.
`test_aspsx_sections.py` repeats the original-tool control.

All **112** rebuilt objects differ from baseline only in these section-header
fields. Every complete function report remains unchanged. Source data improves
to GAME **24/44**, OPEN **8/20**; the strict build still rejects the real
remaining ownership, allocation and addend differences.

## Literal boundary plan and evidence

Before changing the following claims, all ten source units were freshly
compiled with their existing profiles and assembled by ASPSX. Each native
`.rdata` extent ends at the final literal's NUL; the additional claimed bytes
are zeros before the next four-byte-aligned contribution. Every retained
target/source byte and ordered relocation row already agrees. The two mixed
table/literal contributions retain all jump-table words and relocation rows.
Native literal bytes and extents agree, and no curated reference, including
candidates, targets any proposed removed tail.

| Image/unit | RODATA address | Old claim | Native extent | Last literal | References into contribution |
| --- | --- | ---: | ---: | --- | ---: |
| GAME resources | `80012178` | 60 | 57 | `MIXB.DAT` | 5 |
| GAME item | `800122a0` | 40 | 37 | `COM\\STAT.DAT` | 2 |
| GAME menu_map_viewer | `800122e4` | 12 | 9 | `MAP\\M00.` | 1 |
| GAME menu_runtime | `80012350` | 12 | 10 | `TIM\\M000.` | 1 |
| GAME save_system | `8001235c` | 376 | 374 | `TIM\\M000.` | 10 |
| OPEN opening_controller | `80012020` | 24 | 21 | `B0\\MIX3.` | 2 |
| OPEN resources | `8001205c` | 180 | 177 | `B0\\ENDG.` | 15 |
| OPEN render_init | `80012110` | 40 | 37 | `primitive over fllow!!!\n` | 2 |
| OPEN render | `80012138` | 136 | 135 | `POINT OVER !!!!!!\n` | 2 |
| OPEN audio | `8001227c` | 52 | 50 | `VAB body open failed\n` | 2 |

The source change is limited to these RODATA claims. The literals, function
bodies, tables, identities and census bytes remain as evidenced. The gaps are
linker placement padding, not bytes to append to the compiler's string object.
The rebuild must confirm identical source objects and function reports, exact
retained data/relocations, and the expected target-only extent changes.

## Initialized small-data ownership plan

GAME `actor` defines a 250-byte action table, a 12-byte phase-sound table and
one three-byte loop sound. The first two occupy one contiguous retail region;
`boss_death_loop_sound` is at `80057b80` among the late initialized globals.
A fresh GCC 2.5.7 `-G8` compile places only the three-byte datum in `.sdata`.
ASPSX 1.07 assembles its entire 6256-byte `.text` identically with either `-G0`
or `-G8`, with the same ordered relocation operations and no GP-relative patch.
The function-level verdict is unchanged for every actor function. The source
bodies, types and initializers are not edited.

Represent this reviewed hypothesis explicitly in `data_sections.tsv`, keyed by
image/address and checked against the curated identity. Extend target writing,
placement and strict comparison to `.sdata`; require its bytes, relocations,
allocation flags and one independently linkable section base. Use the `-G8`
profile only for this unit. This is a productive layout probe, not proof of the
original compiler flags. No section is inferred from the comparison object.

## Result and remaining ownership work

The final strict result is GAME **27/44** and OPEN **10/20**, up from **8/44**
and **3/20** with the same owners and function bodies. The 26 additional passes
come from native section alignment (21), literal extents (4), and explicit
actor small-data ownership (1). All 112 source objects were audited: 111 differ
only in the calibrated alignment fields, and actor has identical `.text`
bytes and ordered code relocations. Every complete function report is unchanged.
Fresh native actor objects agree with both complete initialized sections,
including the zero spans represented by LNK record 8.

The remaining 27 failures are real unresolved contracts under the current
source/probe, rather than permission to weaken the data gate:

- COMMON globals are independent native XBSS requests. The analysis adapter
  currently materializes them as fixed BSS offsets. Native PSYLINK controls in
  [common-allocation-fidelity.md](common-allocation-fidelity.md) prove that global
  allocation order is not source order, and fixed local reservations behave
  differently. Enabling ELF COMMON alone cannot prove retail placement; the
  independent relinker deliberately rejects that unsupported contract.
- GCC 2.5.7 rounds tentative four-byte globals to eight-byte requests. The
  memory units' 80-byte arena plus two adjacent four-byte retail globals cannot
  be closed by pretending that 80+8+8 equals 88. Camera and formatting-buffer
  extents likewise remain unresolved; extra opaque padding is not evidence.
- Several modules claim initialized globals in distant regions. Actor has a
  demonstrated small-data split, but OPEN also has late 16-byte initializers,
  so globally enabling `-G8` does not explain the remaining layout. TU and
  section ownership need independent recovery.
- GAME's adjacent three-byte sound selectors currently compile as individually
  aligned globals. The prior array hypothesis changes address reuse in a
  banked function; converting them to an array solely for packing regresses
  proved code. Their authentic source/compiler model remains open.
- GAME render, map_object and map_scripts retain switch-table addend mismatches.
  These are actual destination differences, not relocations to mask.
- The two pad units are SDK ownership cases. Their private small-BSS/global
  reservations require the correct provider revision, not reconstructed game
  bodies or inclusion as new game progress.

The full build continues to fail its strict gates, including these data failures
and pre-existing executable-coverage/target-placement limits. No new function
is banked. The following ledger records every original GAME/OPEN data owner;
`match` requires every compared section and placement to pass.

| Image | Unit | Final verdict |
| --- | --- | --- |
| GAME | `display_play_transition` | match |
| GAME | `game` | `.bss` size |
| GAME | `equipment` | `.bss` placement |
| GAME | `player_death` | `.bss` layout |
| GAME | `player_core` | `.bss` layout |
| GAME | `player_use_item` | match |
| GAME | `player_update` | `.data` placement, `.bss` size |
| GAME | `collision` | match |
| GAME | `memory` | `.bss` size |
| GAME | `cd_file` | `.bss` layout |
| GAME | `resources` | `.data` placement, `.bss` layout |
| GAME | `render` | `.rodata` addend, `.bss` size |
| GAME | `render_enqueue` | match |
| GAME | `render_map_cells` | match |
| GAME | `entity_render` | match |
| GAME | `geometry_render` | match |
| GAME | `notify_queue` | match |
| GAME | `render_frame` | `.data` placement |
| GAME | `item` | match |
| GAME | `menu` | match |
| GAME | `menu_map_viewer` | match |
| GAME | `menu_panels` | match |
| GAME | `menu_select` | match |
| GAME | `menu_item_model_preview` | match |
| GAME | `menu_runtime` | `.data` placement |
| GAME | `save_system` | `.data` placement, `.bss` size |
| GAME | `actor` | match |
| GAME | `actor_behavior` | match |
| GAME | `actor_pool` | match |
| GAME | `map_object_pool` | `.bss` size |
| GAME | `map_object` | `.data` size, `.rodata` addend |
| GAME | `audio` | `.bss` placement |
| GAME | `lighting` | match |
| GAME | `map_scripts` | `.rodata` addend |
| GAME | `map_events` | match |
| GAME | `map_load` | match |
| GAME | `player_warp` | match |
| GAME | `effect_pool` | match |
| GAME | `effect_map_collision` | match |
| GAME | `effect_update` | match |
| GAME | `effect_dispatch` | match |
| GAME | `magic` | match |
| GAME | `debug_text` | match |
| GAME | `pad` | `.bss` size |
| OPEN | `opening_helpers` | `.bss` size |
| OPEN | `camera_path` | `.bss` size |
| OPEN | `opening_scenes` | `.data` placement |
| OPEN | `opening_fade` | match |
| OPEN | `opening_controller` | match |
| OPEN | `matrix_rotation` | match |
| OPEN | `memory` | `.bss` size |
| OPEN | `resources` | `.bss` layout |
| OPEN | `render_init` | `.bss` size |
| OPEN | `render` | match |
| OPEN | `render_tmd` | match |
| OPEN | `render_map` | match |
| OPEN | `render_sprite` | match |
| OPEN | `render_map_cells` | match |
| OPEN | `entity_render` | `.data` placement |
| OPEN | `opening_entity_pool` | match |
| OPEN | `audio` | `.bss` placement |
| OPEN | `audio_play_voice` | match |
| OPEN | `format` | `.bss` size |
| OPEN | `pad` | `.bss` size |

## Reproduction and validation

Work in `nix develop`, initialize the hash-identical retail inputs, then run:

```sh
kf build
kf verify data --image game --image open --detail --coverage
python -m unittest discover -s tests -p test_aspsx_sections.py
python -m unittest discover -s tests -p test_small_data.py
ruff check scripts tests
python -m unittest discover -s tests
nix flake check -L
```

The native actor control uses `scripts.kf.executable.compile_unit` with the
unchanged `game.actor` unit and the `probe-gcc257-o2-g0` and
`probe-gcc257-o2-g8` profiles. Run each returned `assembler_command` through
`dos_run`, then inspect the complete objects with `psyk list --code`.
Assembling the G8 source once more with ASPSX `-G0` gives the same sections.
Count record-8 zero spans when measuring initialized `.data`; they are not
missing object bytes. The native G8 payload hashes are:

| Section | Extent | SHA-256 |
| --- | ---: | --- |
| `.text` | 6256 | `904d4f6f88e8586e3777075127809ee7b2d5a39f7963707cbc9a561e25d8f966` |
| `.data` | 264 | `69eaa8a35bc2462d7657f1c43899681388d8406f09bd65cb41a7cdaaa56ea890` |
| `.sdata` | 3 | `9d92402433de9dad0764b3f33ee79a99dbd170efba7ecced2e05cb5d8a032427` |

Validation completed: Ruff and diff checks passed; 745 repository tests passed
with 9 environmental skips; all flake checks passed. The native alignment and
independent small-data linker controls passed in the development environment.
The full build was rerun and still correctly rejects unresolved strict gates;
the detailed data check has zero missing artifacts. No claimed function result
regressed and no source body was changed.
