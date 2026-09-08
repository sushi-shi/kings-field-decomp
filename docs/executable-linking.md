# Executable links and complete-file comparisons

PSX's normal 4096-byte executable differs in seven padding bytes. See the
[PSX layout record](patterns/psx-exact-link.md) for the compiler-directive
alignment bridge, recovered SDK order and unresolved container behavior:
the header is a retail template and historical packing remains unproved.
The tables below retain the earlier investigation results.

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

`--diagnostic-cpe-padding` explicitly enables an inferred CPE prefix in the
final padding. It writes separate executables and reports under
`build/link/diagnostic-cpe-padding/`, labels console output as diagnostic and
cannot replace normal results. Its byte equality does not prove the original
developers' build process. Normal links use zero padding.

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
declared in each report. The optional diagnostic models the CPE v1 prefix
observed after initialized data in all three retail images at the actual
linked load end. Reports explicitly mark the mechanism as inferred;
neither tested native converter reproduces it. The executables have not been
tested by running them.

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

## PSX difference investigation

The first PSX candidate's 431 differing bytes are explained by object order,
source-section alignment and the padded load tail. A fresh `kf try --unit
psx.main` rebuild and strict `kf match --unit psx.main` retain the exact
208-byte `main`. No game C or library instruction change is needed in the
following diagnostic links.

The initial production archive search extracted `A36, C113, C114, C66, C67, SNMAIN,
SNDEF, C57`. The separately established
[retail text order](object-link-order.md) is `SNMAIN, A36, C113, C57, C66,
C67, C114`; `SNDEF` contributes only the four-byte `_stacksize` datum.
The order experiment passes those eight complete converted objects directly,
after the compiled source object, instead of searching the archives. It does
not place individual SDK functions or replace their relocations with constants.

Each subsequent experiment changes one section-alignment field on a **scratch
copy** of the source ELF with GNU objcopy. All source payloads, symbols and
relocation rows are unchanged. The normal generated linker script still honors
each input's stated alignment and links whole sections.

| Cumulative diagnostic change | Differing file bytes | First differing load VA |
| --- | ---: | --- |
| Production candidate | 431 | `80010028` |
| Recovered SDK object order | 398 | `80010028` |
| Source `.text` alignment 16 → 4 | 22 | `80010050` |
| Source `.data` alignment 16 → 4 | 9 | `8001010c` |
| Source empty `.bss` alignment 16 → 4 | 7 | `80010230` |

The `.text` constraint initially moves `main` from `80010028` to `80010030`.
With only SDK order corrected, the entry moves from retail `80010100` to
`80010108`; in the production candidate it is `80010158`. After fixing text
placement diagnostically, `.data` still places `overlay_path_table` at
`80010230` instead of `80010224`, moving `_stacksize` and `.sbss` by twelve
bytes. Finally, the source object's **empty** `.bss` still rounds its start/end
from `80010234` to `80010240`. SNMAIN refers to that section end twice, leaving
two changed low-immediate bytes at `8001010c` and `8001015c` until the scratch
`.bss` constraint is changed too.

Every diagnostic link independently verifies all eight selected SDK members:
304 original payload bytes and 18 native patch expressions. In the last
experiment, the header, both literal paths, the complete 508-byte text range,
the two-pointer table and `_stacksize` agree with retail at their actual file
offsets. Its only differences are seven nonzero retail bytes in this tail:

```text
PSX.EXE file 0x0a30 / VA 0x80010230:
retail: 43 50 45 01 08 00 03 90 00 00 00 00
linked: 00 00 00 00 00 00 00 00 00 00 00 00
```

This is the same CPE-shaped prefix already observed after initialized data in
GAME and OPEN. It overlaps SNMAIN's independently proved four-byte `.sbss`
reservation; startup clears that word. It must not become a source initializer.
See [CPE tail evidence](patterns/psyq-cpe-tail-and-bss.md) for the original
converter control and its limit.

The scratch comparison SHA-256 is
`ea798637bdebb11e9fe569e0656aa809c8681063808baf6a61d9ab7410417651`.
Reports, maps and experimental inputs are under `build/link/psx-investigation/`.
To reproduce the layout experiment after `kf link --image psx`, copy
`build/objdiff/psx/base/80010028_main.o`, apply cumulative
`mipsel-linux-gnu-objcopy --set-section-alignment .text=4` / `.data=4` /
`.bss=4` options, and pass that copy plus the complete SDK objects in the order
above to `scripts.kf.executable.script`. Link with the same GNU ld flags as
`kf link`, then use `serialize`, `compare` and `sdk_link.verify_linked` to check
the complete file and original SDK expressions.

These diagnostic results isolated the causes without establishing original
ASPSX metadata or reproducing the retail converter. At that checkpoint the
production link stayed at 431 differences and no scratch object was banked.
The subsequent [PSX layout work](patterns/psx-exact-link.md) implements a compiler
directive contract. Its inferred container model is confined to an optional
diagnostic; normal links retain the seven unresolved tail bytes.

The rebuilt source and repeated diagnostic links reproduce all counts and
hashes above. Ruff and all 725 repository tests pass. The required full build
retains the existing data/ownership/placement failures: 14/61 source data units,
4/4 SDK data contributions and 110/116 target relinks pass, with no artifact
failures. PSX `main` remains 100%; this investigation banks no new function.
