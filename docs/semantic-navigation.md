# Semantic navigation

`kf sema` is the read-only investigation interface for the three linked King's
Field programs. It combines the manually curated function/data/relocation TSVs
with instruction bytes from the configured retail executables. It does not
modify the TSVs, a Ghidra project, reconstruction sources, or the score ledger.

Every query requires an image:

```sh
kf sema --image psx  map
kf sema --image game addr 0x80014268
kf sema --image open disasm 0x80014268 --blocks
```

This qualification is mandatory because `GAME.EXE` and `OPEN.EXE` are separate
linked overlays. For example, both currently have a function at `0x80014268`,
but the GAME extent is `0x24` bytes and the OPEN extent is `0x174` bytes. A
bare virtual address is therefore not a repository-wide identity.

## Evidence model

The PS-X EXEs contain linked bytes, not linker relocation records. The
navigator consequently keeps three visible evidence tiers:

- `proven`: decoded direct control flow in the retail instruction bytes;
- `validated`: a curated relocation candidate that passes the same conservative
  byte, owner, pair, target, and overlap checks used by the safe delinker;
- `candidate`: useful inventory evidence that does not pass that conservative
  proof boundary, including raw pointer candidates and ambiguous pairs.

Rejected rows and candidates that disagree with the retail bytes are hidden.
Other candidates are shown with their tier by default. Use `--confirmed-only`
on address, disassembly, CFG, dump, xref, and string queries to keep only
`proven` and `validated` references. This filter never promotes a candidate.

The delinker and navigator share the pure validation rules in
`scripts/kf/relocations.py`; their interpretation of a safe relocation cannot
silently drift apart.

## Commands

### Address and RVA

```sh
kf sema --image game addr func_80014fa8
kf sema --image game addr 0x80014fac --json
kf sema --image game rva 0x2fa8
```

The dossier reports the containing binding and offset, load/file position,
function body versus padding, evidence provenance, vendored attribution,
reconstruction unit and match score when present, and incoming/outgoing
reference counts.

### Disassembly, blocks, CFG, and bytes

```sh
kf sema --image game disasm 0x80014268
kf sema --image game disasm 0x80014268 --blocks
kf sema --image game disasm 0x80014268 --lite --json
kf sema --image game cfg 0x80014268
kf sema --image game cfg 0x80014268 --format dot
kf sema --image game cfg 0x80014268 --format json
kf sema --image game dump 0x80014268 --hex
```

GNU `mipsel-linux-gnu-objdump` supplies display text. A small in-tree MIPS-I
decoder independently owns control semantics. Basic blocks include the
instruction immediately after a branch, jump, call, or return as its delay
slot. CFG fallthrough begins at `PC+8`; loop heads, unreachable blocks, common
tails, and unusual branches into delay slots are annotated. GPR loads are also
marked because their value is unavailable to the immediately following
instruction on the R3000A.

Direct J/JAL and conditional branch targets are decoded. JALR/JR targets are
reported as unresolved indirect calls/jumps. Switch-table and general indirect
target recovery are deliberately deferred; the tool does not invent edges from
unproven pointer tables.

### Xrefs and call trees

```sh
kf sema --image game xref 0x80014268
kf sema --image game xref 0x80014268 --callees
kf sema --image game xref 0x80014268 --tree --depth 4
kf sema --image game xref 0x80014268 --callees --tree --raw
```

The default direction is incoming. `--callees` selects outgoing references.
Trees list visible candidate leaves but recurse only through proven/validated
control-flow references, so a candidate cannot create a fictional call graph.
`--raw` exposes every merged evidence origin and rejection reason.

### Strings

```sh
kf sema --image game strings --find "DEBUG STOP"
kf sema --image game strings 0x80012dd4
kf sema --image game strings func_800507b8
kf sema --image game strings --ranked
```

Strings come from curated `data.tsv` extents and are decoded from the selected
retail image as ASCII or Shift-JIS. A function target lists the strings it
references. Ranking orders functions by proven, validated, then candidate
string evidence.

### Map and match state

```sh
kf sema --image game map
kf sema --image game map at 0x80014268
kf sema --image game map range 0x80014000 0x80015000
kf sema --image game map gaps
kf sema --image game map units
kf sema --image game map find libgpu
kf sema --image game match --all
kf sema --image game match game.func_80014fa8
kf sema --image game match --worst 10
```

`map gaps` distinguishes true inventory holes from admitted `unclassified`
data rows. The current inventories cover all three load images, but the
unclassified rows remain explicit places where code, tables, constants,
padding, or writable data may still be missing. `match` reads existing objdiff
reports; it does not build. Exact means `100%` unless `--loose` is explicitly
requested.

## Machine-readable and batch use

All data-oriented commands accept `--json`; CFG additionally accepts DOT. JSON
documents use the stable top-level envelope:

```json
{
  "schema": "kf-sema-v1",
  "image": "GAME.EXE",
  "view": "addr",
  "result": {}
}
```

Pass `-` as the semantic command to read one query per line from stdin. Blank
lines and lines beginning with `#` are ignored:

```sh
printf '%s\n' \
  'addr 0x80014268' \
  'xref 0x80014268 --callees --confirmed-only' \
  'strings --find "DEBUG STOP"' |
  kf sema --image game -
```

## Scope and curation

The navigator reflects the current WIP inventories; it is not a claim that all
function boundaries, data types, or relocation rows are correct. Use it to find
and compare evidence, then curate `config/retail/functions.tsv`, `data.tsv`,
`relocs.tsv`, or the separate label overlays. Regenerate or audit inventories
with the existing census tools before admitting discoveries. Ghidra remains
useful for interactive analysis and type work, but `kf sema` has no Ghidra
database dependency and is deterministic from committed configuration plus the
hash-verified retail bytes.
