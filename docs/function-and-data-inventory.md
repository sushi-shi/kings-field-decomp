# Function, global, and static identities

The structural census says where linked code and data exist. The identity
layer records the current source-level interpretation without changing delink
symbols or pretending that WIP names are original symbols:

- `config/retail/function_identities.tsv` has exactly one row for each
  carveable non-vendored function: 1 PSX, 497 GAME, and 246 OPEN rows;
- `config/retail/data_identities.tsv` tracks initialized objects and referenced
  BSS objects that may become C globals, file-local statics, or
  function-local statics; and
- address-derived `func_` and `DAT_` names are stable unresolved identities.

The current first pass contains 3,846 data identities, including 969
non-overlapping BSS extents. Of those BSS rows, 11 have semantic review from
the initial matching campaign. Six initialized-data identities describe the
private GAME/OPEN `LIBGTE/MTX` matrix stacks, offsets, and error-path scratch
words; the remaining direct-access extents deliberately retain address-only names.

Functions owned by an object family use `owner_action`, with the same parts in
the `owner` and `action` columns. Signatures use semicolon-separated C
declarations so argument names and widths can be refined independently. Data
rows carry an exact admitted extent, load/BSS storage, tentative linkage scope,
datatype, and owner.

The confidence values are `address-only`, `candidate`, `supported`, and
`proven`. `proven` is reserved for a recovered source symbol/signature or an
equally direct witness; a plausible semantic reconstruction normally remains
`supported`.

## Review loop

```sh
kf inventory check
kf inventory propose
kf inventory ghidra --image game --image open
kf inventory propose-ghidra

kf sema --image game addr vector2s_scale_shift11
kf sema --image game xref pool_records --confirmed-only
kf sema --image game disasm vector2s_scale_shift11 --blocks
```

`kf inventory propose` writes two ignored review files:

- `build/function-inventory/evidence.tsv` combines MIPS live-in signature
  hints, callers, callees, indirect calls, strings, data references, and basic
  instruction-shape counts for all 744 functions;
- `build/function-inventory/data-evidence.tsv` lists confirmed/candidate users,
  read/write/address counts, and tentative scope/owner hints for every tracked
  datum;
- `build/function-inventory/relocs-bss.tsv` proposes decoded MIPS HI/LO pairs
  from non-vendored code to admitted BSS extents.

A datum with one user receives a `function-static` hint and that function as
an owner hint; the adjacent confidence column distinguishes confirmed from
candidate xrefs. This is prioritization evidence only: another xref may still
be missing, and local linkage cannot be reconstructed from one use alone.
Multiple users produce `shared`, not `global`: without reliable TU ownership,
xrefs cannot distinguish external linkage from a file-local static used by
several functions.

`kf inventory ghidra` keeps persistent, ignored projects under
`build/ghidra-inventory`. Its JSON contains the inferred prototype, structured
parameter storage, decompiled C, and non-flow memory references for each
non-vendored function. Ghidra results are proposals because analysis can merge
functions, infer the wrong width, or manufacture a misleading default name.
They must be checked against retail instructions, delay slots, callers,
callees, relocation referents, and the pinned headers before admission.
`kf inventory propose-ghidra` merges those inferred prototypes into an ignored
review TSV while preserving supported/proven signatures; it also never edits
the curated identity file. It also writes `ghidra-data-xrefs.tsv`, grouping
Ghidra's memory references by target and marking each as admitted data/text,
scratchpad, hardware, or unowned RAM. Unowned RAM and zero-admitted-site groups
are queues for missing global/static extents and relocation review.
Its companion `data-identities.ghidra.tsv` proposes non-overlapping BSS extents
only for RAM addresses directly read or written by an instruction. Mere scalar
constants, pointer-only parameters, heap initialization addresses, scratchpad,
and hardware references are not auto-admitted.

## Curation rules

- Never key an identity by VA alone; `image` plus `va` is the key.
- Preserve an address-derived name until evidence supports a semantic one.
- Record semantic argument names only after checking call sites and the body.
- Do not create an interior global when the address is a field or element of a
  larger owned object; refine the owner's datatype and extent instead.
- `scope=static` means file-local linkage and `scope=function-static` means a
  function-local static. Neither may be promoted solely from xref count.
- Keep generated reports under `build/`; only reviewed edits belong in the two
  identity TSVs.

The inventories are intentionally iterative. Missing functions, relocations,
data extents, and xrefs can invalidate earlier candidates without invalidating
the workflow.

The current scan has admitted 3,177 such pairs as candidate xrefs to 590 BSS
owners. Safe delinking still withholds an out-of-load pair until its relocation
row is individually reviewed; candidate rows also cannot suppress a stronger
reviewed pair at the same instruction.
