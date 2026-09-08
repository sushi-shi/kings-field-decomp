# Executable links and complete-file comparisons

Run `kf link` inside `nix develop` after `kf init`. Use `kf link --image game`
to select one program. The command refreshes source and target objects, links
each selected image, and writes:

- `build/link/psx/PSX.EXE`, `build/link/game/GAME.EXE`, `build/link/open/OPEN.EXE`;
- a `linked.elf`, linker script, map and per-attempt diagnostics in each directory;
- `build/link/comparison.json` and per-image `comparison.json` reports, including
  file hashes, unmasked differing-byte counts, source symbol address/size deltas,
  library verification and all retail-backed inputs.

A successful exit means that the links succeeded and the candidate files were
compared. Check `comparison.file_equal` for byte equality. This command does not
bank function or data progress, replace strict `kf build` gates, or run the game.
Existing executables are removed before retrying a link, so a failed attempt
cannot leave an earlier candidate looking current.

The campaign links each image independently from its compiled game objects and
the supplied Psy-Q libraries, then compares the generated PS-X EXE with verified
retail. Matching a function in objdiff is not yet proof of executable equality.

## Link Match Plan

Preserve the current C, compiler profiles, claims and banked function ledger.
Adapt the original LNK v2 library records to the ELF format of the current probe
objects, checking whole payloads, allocations, alignments, symbols and MIPS
relocations. Validate fixups independently against original PSYLINK 1.17.
Link the three programs separately, retaining linker maps, missing definitions,
section placements, provider provenance and complete-file comparison reports.
Unsupported input or unresolved symbols must fail the link. Existing strict
data and ownership gates retain their meaning.

The supplied archive is Psy-Q Release 2.5; several retail library revisions are
known to differ. Its linked bytes are an independent comparison input, not a
promise that the archive version or final link order is historically exact.
The overlay `NONE2.OBJ` startup is identified in retail but absent from that
archive. Its existing delinked vendor objects supply the two startup entries
with explicit retail provenance. They do not count as reconstructed game code.

## Library adapter

`scripts/kf/sdk_link.py` consumes the pinned `psyk list --code` representation
of complete library members. It preserves all six native section classes,
exported and common symbols, code/data payloads, explicit reservations and
the four observed MIPS patch types. SDK reservations inside initialized
sections remain listed separately from supplied payload bytes; flat output
fills those holes with zero. Native XBSS reservations use the independently
observed four-byte rounding; their final ordering remains a linker comparison
question.

The native relocation control proves a detail that is easy to miss in the
parser's display: `($4-[symbol])` is evaluated by PSYLINK as `symbol - 4`.
The adapter preserves that operand order, signed-low carry, jump targets and
full-word pointer addends. This is a format observation, not a compiler or
game-source change.

After linking, every selected SDK member is checked again at its actual output
address. Its original payload and explicit patch expressions must reproduce
every linked byte. This catches mistakes in ELF implicit addends and HI/LO
pairing independently of the format adapter's emitted relocation rows.

## Initial storage and API bindings

The first real links exposed 63 undefined symbols in GAME and 14 in OPEN.
Four GAME symbols are curated API spellings: `InitCARD2`, `StartCARD2`,
`StopCARD2` and `erase` correspond to the supplied archive's `InitCARD`,
`StartCARD`, `StopCARD` and `delete`. The link accepts these aliases only after
the complete retail function body agrees with the archive export and has no
patches. Source and inventory names remain unchanged.

The other unresolved names are globals already described by the inventories.
The link supplies 33 GAME and 14 OPEN BSS reservations, and **349 initialized
GAME bytes across 26 inventory objects** from verified retail. Four string
census extents omit their terminating NUL; its actual byte is checked and
included explicitly in the link report. BSS is not populated from retail.
Overlapping source owners, ambiguous extents and raw pointer words needing
relocation ownership fail instead of creating duplicate definitions.

These generated inputs are provisional link storage, not independently
reconstructed data. Their evidence and unresolved extent/linkage boundary stay
visible. The PS-X header retains retail loader/region metadata while regenerating
the entry and load size. Those template bytes and the retail startup are also
declared in each report. The executables have not been tested by running them.

## First verified executable comparison

All three links succeed and reproduce their output hashes on a second run.
These are complete-file differences at original file offsets, including the
header and padding; moved code, changed relocation values and reordered SDK
objects all contribute. They are not objdiff function percentages.

| Image | Linked / retail file size | Differing file bytes |
| --- | ---: | ---: |
| PSX.EXE | 4,096 / 4,096 | 431 |
| GAME.EXE | 288,768 / 288,768 | 243,454 |
| OPEN.EXE | 155,648 / 155,648 | 131,449 |

The first source-placement control is PSX `main`: retail `80010028`, linked
`80010030`. GNU ld honors the source object's sixteen-byte `.text` alignment,
inserting eight bytes before it. The archive lookup order also differs from the
recovered retail object order. Both effects are visible in the map and symbol
report; neither is hidden by lowering alignment or rewriting defined symbols.

All 725 local tests, Ruff, whitespace checks and `nix flake check -L` pass.
The full `kf build` still fails its existing data/ownership/placement checks:
13/61 source data units match, four SDK data contributions pass, and target
roundtrip is 110/116. No comparison artifact failure is reported. Creating a
linked comparison candidate does not close those reconstruction requirements.
