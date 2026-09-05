# OPEN general TMD polygon emission

OPEN `render_enqueue_tmd` at `8001764c` is complete reconstructed C at
**98.802410% strict objdiff** under `probe-gcc257-o2-g0`. Retail is 3320 bytes
(`0xcf8`); the probe emits 3324 bytes (`0xcfc`), including the return delay
slot. The function is not exact and is not banked. Its exact caller,
`opening_entity_render` at `80018ecc`, remains 100% after the identity rename.
Per-function snapshots and verdicts are in `open_semantic_tmd_enqueue.tsv`.

## Function Match Plan and evidence

Before the first edit, the campaign inspected all six OPEN semantic queries,
the complete disassembly/CFG, both calls at `80019050` and `800190d0`, the
preceding projection epilogue, following unlit renderer, allocator behavior,
source history, GAME's behavioral homolog, switch bytes and color xrefs, and
the pinned LIBGTE/LIBGPU archive symbols and headers. The published plan was
to reconstruct each mode, retain signed depth rules and SDK interfaces,
recover the local switch and complete color owner, and compare referents,
calls, CFG and widths before attributing remaining code-generation symptoms.

The sole caller supplies an unsigned halfword object ID and a signed halfword
depth bias. Case 25 projects with the perspective-right variant and passes
zero; ordinary cases project normally and pass the selected signed bias.
The renderer does not project vertices itself. Cases 26/27 use the separate
unlit emitter and are not silently folded into this function.

This is game-owned prepared-TMD traversal and allocation/depth policy. The
called NormalClip/NormalColor functions belong to LIBGTE/SMP, and the packet
constructors, SetSemiTrans and AddPrim belong to LIBGPU/PRIM. Pinned
`psyk list --recursive` identifies those providers and their XDEFs; the
existing vendored ledger retains the known GTE revision skew. No SDK bodies
are reconstructed here. GAME `render_enqueue_tmd` is a behavioral homolog,
not a byte-identical body: it has different graphics ownership and inline
allocation/error handling. A common original file is not established.

## Complete dispatch and packet behavior

The 29-word switch at `800121c0` covers modes `20` through `3c`. It is the
unit's 116-byte RODATA claim, replacing 29 independent pointer identities.
The actual `jr v0` value chain uses the unsigned `(mode - 32) <= 28` bound,
four-byte table indexing, a word load, its load delay, and the jump delay slot.
There are twelve case targets and one default packet-advance target:

| Mode | GPU packet | Lighting |
| --- | --- | --- |
| `20`, `22` | F3 | Face normal, average triangle p2; `22` semi-transparent |
| `24` | FT3 | Face normal, neutral color, average triangle p2 |
| `28`, `2a` | F4 | Face normal, average quad p2; `2a` semi-transparent |
| `2c` | FT4 | Face normal, neutral color, average quad p2 |
| `30` | G3 | NormalColorDpq3 with vertex-0 p2 |
| `32` | G3 | Semi-transparent NormalColorCol3, without depth cue |
| `34` | GT3 | NormalColorDpq3, neutral color and vertex-0 p2 |
| `38` | G4 | NormalColorDpq3 plus the fourth normal, vertex-0 p2 |
| `3a` | G4 | Four separate NormalColorDpq calls, vertex-0 p2; semi-transparent |
| `3c` | GT4 | NormalColorDpq3 plus fourth normal, neutral color and vertex-0 p2 |

Every case rejects `NormalClip <= 0` before allocating. The allocator is
called with the actual SDK packet size, including 52 bytes for GT4, and its
result is used without a NULL check. OPEN's allocator itself loops forever
printing an error on overflow; GAME's different inline return policy is not
copied into this function.

Prepared vertex and normal indices are unsigned halfword **byte offsets**.
Projected XY is copied as one word, and each UV pair as one halfword. CLUT and
texture page come from the packet, not the active sprite material. Untextured
colors come from the packet's RGB/mode prefix. Textured lighting uses one
initialized CVECTOR at `800372f0`, bytes `80 80 80 00`. The nine encoded color
references remain owner+3 (`800372f3`); the former one-byte identity is removed.
The following four bytes at `800372f4` remain separately unclassified.

Triangle depth is `((sz0 + sz1 + sz2) / 3) >> 2`; quad depth is
`(sz0 + sz1 + sz2 + sz3) >> 4`. Both use signed halfword inputs, add the signed
bias, accept depths at least five, and index with `depth & 0x3fff`. There is
no upper-depth guard. The signed division checks and both trap instructions
are retained. All modes, including defaults and rejected faces, consume the
four-byte header and advance by `(header >> 6) & 0x3fc`. The primitive count
is a word-sized postdecrement loop, with the final decrement in a delay slot.

## Shared packed views without replacing SDK APIs

`gpu_packets.h` pairs each of the eight authentic SDK polygon types with a
typed packed view in a same-size union. The packed view exposes `long` XY,
halfword UV pairs, and CVECTOR color lanes. Constructors receive the real
`POLY_*` member; lighting receives actual CVECTOR fields. These views model
the observed word/halfword accesses, not a claim that the original source used
these union declarations. SDK padding fields retain their real positions.

The shared `KfTmdPrimitive` union describes the mode-selected body and its
common color/texture prefixes. Three-vertex modes never access the fourth UV
pair. Existing TMD object and primitive layouts are unchanged.
`tests/fixtures/open_tmd_enqueue_layout.c` checks every paired SDK field,
packet size/alignment, packed widths and TMD prefix offset. A wrong FT3 X1
offset is rejected. No cast-hiding macros or incompatible per-file structs
are introduced; the source-quality ratchet stays below its existing limits.

Release 2.5 LIBGPU.H declares `SetPolyGT(POLY_GT3 *)`, whereas PRIM.OBJ and
retail export `SetPolyGT3`. The shared SDK wrapper now supplies that missing
void-returning declaration; a typed function-pointer fixture checks it.
The retail helper writes only the packet length and code and returns with
the code store in its delay slot. No replacement SDK implementation is added.

## Focused comparisons and remaining work

The first complete source scored 81.412050%. It placed one depth check after
the switch, causing the probe to merge triangle division and shading tails
that retail keeps separate. A mode-local vertex-scope experiment scored
72.243370% and did not recover that topology; it was rejected. Shared vertex
scratch and per-mode depth checks/insertions recover all 57 ordered call
sites and reach 97.272285%, with one emitted AddPrim tail as in retail.

Advancing the next packet from the current typed body removes a redundant
parallel cursor lifetime, reaching 98.802410%. It preserves the same header
and body extents while eliminating repeated stack reloads in the loop. These
are source-level CFG and cursor corrections, not flag or permutation searches.

The first remaining raw difference is the 88-byte compiled frame versus
96 bytes in retail. Both save the same ten registers and preserve the incoming
bias in a saved register. The compiled count/header occupy sp+32/+40 instead
of retail +48/+40. Retail materializes the current-asset slot `80069b60`, adds
32 for projected vertices, then loads the ordering-table pointer at -276 from
that base. The source retains the supported separate owners and introduces
two legitimate absolute HI16/LO16 pairs. This is the same unresolved wider
graphics ownership seen in the unlit and map renderers; no fake aggregate or
out-of-bounds aliases are introduced to force it.

The remaining instruction differences include normal/projected-base register
roles, commutative address operands, some packet/normal argument copies, and
the triangle result register at the depth join. All 57 direct calls, eleven
internal jumps, eleven retail address pairs, 29 table rows and two caller
relocations are reviewed. Tests compare all twelve static call-path sets,
including clip/depth rejection and division traps, between retail and the
actual compiled object. Ordered data targets agree after explicitly accounting
for the two separate global bases. Those controls establish path/referent
agreement, not identical machine CFGs or a historical compiler mechanism.

This completes the unstarted census: OPEN has **91 exact / 108 started /
108 eligible**, leaving 17 partial C reconstructions and zero unstarted
functions. The count of exact game functions across the three images stays
353. The renderer and its switch are not banked as exact.

The full three-image rebuild, OPEN strict check, Ruff, and all 376 repository
tests pass with no local skips. All 117 reconstructed object hashes are
unchanged by the SetPolyGT3 declaration correction. The new union types also
leave existing consumers' code unchanged; only the new renderer and the exact
caller's symbolic callee rename changed objects in the initial OPEN rebuild.
All 63 data-owning units and 13 vendor-source controls remain exact. The
source-quality ratchet is unchanged, with 877 pointer casts below its existing
882 limit. Only the exact caller's baseline is refreshed, not the new partial
function. The four pre-existing GAME historical-best deficits documented in
`open-format-display-sdk.md` remain outside this campaign.

Staged `nix flake check -L` also passes. Its isolated run passes the 376-test
suite with 36 controls skipped because local retail, generated objects or
tools are unavailable; the workspace run above exercises all of them.
