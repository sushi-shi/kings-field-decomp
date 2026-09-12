# Data ownership inventory cleanup

The ownership metric introduced after `8a098da3` separates unresolved owners
from raw address-derived names. Its first inventory pass starts from 2,898
identities, 2,704 raw `DAT_` names and 2,098 unresolved ownership candidates.
Counts include both overlays and are not counts of original C objects.

## Existing game RODATA owners

Remove 601 address-only identities whose entire extents lie inside existing,
image-qualified source `RODATA` claims. The byte census in `data.tsv` remains
intact for these entries. Their units already own the tables, and the safe
delinker represents their references as section-relative offsets. Source
claims, instructions and relocation targets are unchanged. Existing semantic
identities inside those ranges are outside this address-only cleanup.

## LIBSPU dispatch tables

| Image | Table | Bytes | Consumer | Bound instruction | Base address pair |
| --- | --- | --- | --- | --- | --- |
| GAME | `80013320` | `84c` | `_spu_ioctl` at `8003f478` | `8003f4a0` | `8003f4b4/8003f4b8` |
| OPEN | `800127ec` | `84c` | `_spu_ioctl` at `8001f298` | `8001f2c0` | `8001f2d4/8001f2d8` |

Both consumers bound the selector at `0x212`, multiply it by four, load the
indexed pointer and jump through it. Each complete table has 531 entries,
targeting 54 internal blocks of the corresponding `0xa28`-byte function.
Every target's offset from its owning function agrees across the overlays.
This proves the table relationship beyond merely pointing into SDK code.
Provider attribution is supported by the archive/member and cross-overlay
evidence in `config/evidence/overlay_lineage.tsv` (`spu-ioctl-dispatch`) and
`functions_vendored.tsv`; exact historical SDK revision remains unresolved.

Replace each table's 531 word identities and structural rows with one complete
`spu_ioctl_dispatch_table` identity owned by `libspu_spu`. Keep linkage unknown;
the semantic name describes the proved role without claiming an original
symbol. Name the two existing base-address references without changing their
numeric targets, statuses or instruction forms. All pointer relocation rows
remain in place.

The supplied Release 2.5 SDK has a different dispatcher/table. These inventory
changes do not supply retail table bytes to the executable build and do not
add a candidate data contribution or reconstructed SDK body.

Generated before/after dossiers are under `build/data-ownership-cleanup/`:
removed RODATA identities with their owners, SDK bounds and complete relative
target sequences, previous relocation rows and target-module hashes. The raw
checks use hash-validated images and image-qualified semantic queries.

## Result and remaining work

The curated inventory now contains 1,237 identities: 1,041 raw `DAT_` names
and 1,036 unresolved ownership entries. Removing already-owned RODATA labels
does not change the ownership metric. Proving the two complete SDK tables
resolves 1,062 previously unowned word entries.

All 101 delinked source-unit targets are byte-identical across this inventory
change. A fresh rebuild and strict comparison of the 18 affected RODATA units
covers 164 functions, including 159 exact functions; none of the 162 functions
with recorded baselines regresses. No function is newly banked by this cleanup.

Remaining unresolved entries comprise 806 load-image entries and 230 BSS
entries across the three programs. They still require complete-object extents
and consumer/owner evidence. The repeated LIBSND control-change dispatch
families are a next investigation target; a pointer into a known library
function alone is insufficient to consolidate or exclude an entry.
