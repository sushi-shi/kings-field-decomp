# OPEN input action domain

The function match plan is to propagate the existing continue/advance/skip
domain through the shared BSS word, poller and controller locals. Preserve the
global's u32 representation and the locals' s32 representation using the
existing enum compatibility layer. Scene consumers already compare against the
named constants and acquire the domain through the shared declaration.

Retail evidence in OPEN.EXE:

- `opening_poll_input`, `0x80013c70`, extent `0x3c`: calls Sony `PadRead(1)`;
  zero input leaves the previous action latched. Otherwise bit `0x100` selects
  two, and its absence selects one. The joined `sw` at `0x80013c98` writes
  `opening_input_action` at `0x80043178`. The policy is game code, as recorded
  in `config/evidence/open_semantic_opening_helpers.tsv`.
- `opening_run`, `0x800156bc`, extent `0x214`: retains one in s2 and two in s1,
  compares them with full-word action loads, and resets the action before
  scenes. Advance selects the next scene; skip selects the final-image reload.
  Initialization, ending dispatch, asset strings and common cleanup stay intact.
- `opening_scene0_run`, `0x80014268`, extent `0x174`, and
  `opening_scene1_run`, `0x8001455c`, extent `0xac`, inspect the shared word
  with zero/nonzero predicates. `opening_scene3_run`, `0x80014804`, extent
  `0x330`, also uses those predicates through its polling and transition paths.
  None needs an integer conversion.

The poller disassembly and existing controller/scene dossiers were reviewed;
curated references agree on one full-word BSS owner. The existing baseline is
100% for these five functions. This edit changes declarations and local types,
with no changes to expressions, control flow, calls or enum values. The data
identity and existing inventory fixture now name `KfOpeningInputAction`.

Compilation and match verification are deferred until the naming pass is
finished, at the user's request. Baseline scores are not verification of this
edit; no new exact result is claimed or banked.
