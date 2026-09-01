# Recovered object placement and partial link order

The retail PS-X EXEs contain no object names, section table, symbol table, or
relocation table. Object boundaries are therefore not intrinsically visible.
This inventory records the narrower fact we can prove: a complete Psy-Q
Release 2.5 object `.text` contribution matches a specific retail address after
masking only linker-owned relocation bits.

Sorting those placements by address recovers part of the final linked order for
each independently linked program. It does not recover the complete linker
command line, imply that unmatched gaps are padding, or establish archive
storage order. Object data and BSS contributions may also be placed separately
from the `.text` spans listed here.

The current evidence anchors 44 unique relocation-masked `.text` spans and eight
tiny complete objects: seven placements in `PSX.EXE`, 23 in `GAME.EXE`, and 22
in `OPEN.EXE`. Three additional `LIBGTE` object spans per overlay are supported
by exact cross-overlay function order and archive XDEF order while their exact
SDK revision remains unresolved. `contiguous` means that the preceding listed
span ends exactly at the next span. A numeric gap contains code not assigned by
the listed inventory; its ownership remains open.

The source rows are
`config/evidence/psyq_release_25_text_sections.tsv` and
`config/evidence/psyq_release_25_complete_objects.tsv`; the version-skewed GTE
rows come from `config/evidence/overlay_lineage.tsv`.

## Proven contiguous chains

The strongest recovered local sequences are:

```text
PSX.EXE:
  SNMAIN -> A36 -> C113 -> C57 -> C66 -> C67 -> C114

GAME.EXE and OPEN.EXE:
  EVENT -> 96VEC
  S_SR -> S_ITC -> S_STM -> S_STSA -> S_R|S_W -> S_WP
  TMD -> PRIM -> EXT -> OTAG -> SPRINTF

GAME.EXE only at current confidence:
  SCSMVOL -> SESKON
  REG -> MTX -> SMP -> [8-byte alignment] -> CMB -> [12-byte alignment] -> GEO

OPEN.EXE only at current confidence:
  COR -> REG

GAME.EXE and OPEN.EXE, version-skewed GTE lineage:
  MTX -> SMP -> [8-byte alignment] -> CMB -> [12-byte alignment] -> GEO
```

`S_R|S_W` means the bytes prove Sony/Psy-Q ownership but cannot distinguish the
two identical archive members. The order above is retail address order. The
controlled PSYLINK tests show that direct input-object order affects addresses,
while merely changing archive member storage order does not establish linked
order.

## `PSX.EXE`

| start | end | size | gap from previous | object candidate | evidence |
|---:|---:|---:|---:|---|---|
| `0x800100f8` | `0x800101c4` | `0xcc` | — | `LIBSN.LIB/SNMAIN.OBJ` | `strong_unique` |
| `0x800101c4` | `0x800101d4` | `0x10` | contiguous | `LIBAPI.LIB/A36.OBJ` | `complete-byte-match` |
| `0x800101d4` | `0x800101e4` | `0x10` | contiguous | `LIBAPI.LIB/C113.OBJ` | `complete-byte-match` |
| `0x800101e4` | `0x800101f4` | `0x10` | contiguous | `LIBAPI.LIB/C57.OBJ` | `complete-byte-match` |
| `0x800101f4` | `0x80010204` | `0x10` | contiguous | `LIBAPI.LIB/C66.OBJ` | `complete-byte-match` |
| `0x80010204` | `0x80010214` | `0x10` | contiguous | `LIBAPI.LIB/C67.OBJ` | `complete-byte-match` |
| `0x80010214` | `0x80010224` | `0x10` | contiguous | `LIBAPI.LIB/C114.OBJ` | `complete-byte-match` |

This seven-object sequence has no unresolved byte gap between its admitted
placements.

## `GAME.EXE`

| start | end | size | gap from previous | object candidate | evidence |
|---:|---:|---:|---:|---|---|
| `0x8003ac6c` | `0x8003b4b8` | `0x84c` | — | `LIBCD.LIB/SYS.OBJ` | `strong_unique` |
| `0x8003c570` | `0x8003c6b0` | `0x140` | `0x10b8` | `LIBCD.LIB/EVENT.OBJ` | `strong_unique` |
| `0x8003c6b0` | `0x8003c810` | `0x160` | contiguous | `LIBCD.LIB/96VEC.OBJ` | `strong_unique` |
| `0x8003d2cc` | `0x8003e258` | `0xf8c` | `0xabc` | `LIBCD.LIB/CDROM.OBJ` | `strong_unique` |
| `0x80041564` | `0x80042358` | `0xdf4` | `0x330c` | `LIBSPU.LIB/S_M_INT.OBJ` | `strong_unique` |
| `0x800464d4` | `0x80046580` | `0xac` | `0x417c` | `LIBSPU.LIB/S_SR.OBJ` | `short_unique` |
| `0x80046580` | `0x8004666c` | `0xec` | contiguous | `LIBSPU.LIB/S_ITC.OBJ` | `strong_unique` |
| `0x8004666c` | `0x800466c0` | `0x54` | contiguous | `LIBSPU.LIB/S_STM.OBJ` | `short_unique` |
| `0x800466c0` | `0x8004672c` | `0x6c` | contiguous | `LIBSPU.LIB/S_STSA.OBJ` | `short_unique` |
| `0x8004672c` | `0x8004677c` | `0x50` | contiguous | `LIBSPU.LIB/S_R.OBJ|LIBSPU.LIB/S_W.OBJ` | `archive_ambiguous` |
| `0x8004677c` | `0x80046804` | `0x88` | contiguous | `LIBSPU.LIB/S_WP.OBJ` | `short_unique` |
| `0x80046834` | `0x8004688c` | `0x58` | `0x30` | `LIBSND.LIB/SCSMVOL.OBJ` | `short_unique` |
| `0x8004688c` | `0x800468d8` | `0x4c` | contiguous | `LIBSND.LIB/SESKON.OBJ` | `short_unique` |
| `0x80049fd0` | `0x8004a0b8` | `0xe8` | `0x36f8` | `LIBSND.LIB/ADSR.OBJ` | `strong_unique` |
| `0x8004b9b0` | `0x8004c22c` | `0x87c` | `0x18f8` | `LIBGTE.LIB/COR.OBJ` | `strong_unique` |
| `0x8004c9ac` | `0x8004cc4c` | `0x2a0` | `0x780` | `LIBGTE.LIB/REG.OBJ` | `strong_unique` |
| `0x8004cc4c` | `0x8004da4c` | `0xe00` | contiguous | `LIBGTE.LIB/MTX.OBJ` | `sdk-lineage-supported` |
| `0x8004da4c` | `0x8004dfd4` | `0x588` | contiguous | `LIBGTE.LIB/SMP.OBJ` | `sdk-lineage-supported` |
| `0x8004dfdc` | `0x8004e940` | `0x964` | `0x8` | `LIBGTE.LIB/CMB.OBJ` | `sdk-lineage-supported` |
| `0x8004e94c` | `0x8004fa00` | `0x10b4` | `0xc` | `LIBGTE.LIB/GEO.OBJ` | `strong_unique` |
| `0x800502dc` | `0x800502ec` | `0x10` | `0x8dc` | `LIBAPI.LIB/C51.OBJ` | `complete-byte-match` |
| `0x800529a0` | `0x800540dc` | `0x173c` | `0x26b4` | `LIBGPU.LIB/TMD.OBJ` | `strong_unique` |
| `0x800540dc` | `0x800547a0` | `0x6c4` | contiguous | `LIBGPU.LIB/PRIM.OBJ` | `strong_unique` |
| `0x800547a0` | `0x800549ac` | `0x20c` | contiguous | `LIBGPU.LIB/EXT.OBJ` | `strong_unique` |
| `0x800549ac` | `0x80054d20` | `0x374` | contiguous | `LIBGPU.LIB/OTAG.OBJ` | `strong_unique` |
| `0x80054d20` | `0x800555b0` | `0x890` | contiguous | `LIBGPU.LIB/SPRINTF.OBJ` | `strong_unique` |

## `OPEN.EXE`

| start | end | size | gap from previous | object candidate | evidence |
|---:|---:|---:|---:|---|---|
| `0x8001aa8c` | `0x8001b2d8` | `0x84c` | — | `LIBCD.LIB/SYS.OBJ` | `strong_unique` |
| `0x8001c390` | `0x8001c4d0` | `0x140` | `0x10b8` | `LIBCD.LIB/EVENT.OBJ` | `strong_unique` |
| `0x8001c4d0` | `0x8001c630` | `0x160` | contiguous | `LIBCD.LIB/96VEC.OBJ` | `strong_unique` |
| `0x8001d0ec` | `0x8001e078` | `0xf8c` | `0xabc` | `LIBCD.LIB/CDROM.OBJ` | `strong_unique` |
| `0x80021384` | `0x80022178` | `0xdf4` | `0x330c` | `LIBSPU.LIB/S_M_INT.OBJ` | `strong_unique` |
| `0x800262f4` | `0x800263a0` | `0xac` | `0x417c` | `LIBSPU.LIB/S_SR.OBJ` | `short_unique` |
| `0x800263a0` | `0x8002648c` | `0xec` | contiguous | `LIBSPU.LIB/S_ITC.OBJ` | `strong_unique` |
| `0x8002648c` | `0x800264e0` | `0x54` | contiguous | `LIBSPU.LIB/S_STM.OBJ` | `short_unique` |
| `0x800264e0` | `0x8002654c` | `0x6c` | contiguous | `LIBSPU.LIB/S_STSA.OBJ` | `short_unique` |
| `0x8002654c` | `0x8002659c` | `0x50` | contiguous | `LIBSPU.LIB/S_R.OBJ|LIBSPU.LIB/S_W.OBJ` | `archive_ambiguous` |
| `0x8002659c` | `0x80026624` | `0x88` | contiguous | `LIBSPU.LIB/S_WP.OBJ` | `short_unique` |
| `0x80026654` | `0x800266ac` | `0x58` | `0x30` | `LIBSND.LIB/SCSMVOL.OBJ` | `short_unique` |
| `0x80029da4` | `0x80029e8c` | `0xe8` | `0x36f8` | `LIBSND.LIB/ADSR.OBJ` | `strong_unique` |
| `0x8002b784` | `0x8002c000` | `0x87c` | `0x18f8` | `LIBGTE.LIB/COR.OBJ` | `strong_unique` |
| `0x8002c000` | `0x8002c2a0` | `0x2a0` | contiguous | `LIBGTE.LIB/REG.OBJ` | `strong_unique` |
| `0x8002ca20` | `0x8002d820` | `0xe00` | `0x780` | `LIBGTE.LIB/MTX.OBJ` | `sdk-lineage-supported` |
| `0x8002d820` | `0x8002dda8` | `0x588` | contiguous | `LIBGTE.LIB/SMP.OBJ` | `sdk-lineage-supported` |
| `0x8002ddb0` | `0x8002e714` | `0x964` | `0x8` | `LIBGTE.LIB/CMB.OBJ` | `sdk-lineage-supported` |
| `0x8002e720` | `0x8002f7d4` | `0x10b4` | `0xc` | `LIBGTE.LIB/GEO.OBJ` | `strong_unique` |
| `0x80030050` | `0x80030060` | `0x10` | `0x87c` | `LIBAPI.LIB/C51.OBJ` | `complete-byte-match` |
| `0x80032684` | `0x80033dc0` | `0x173c` | `0x2624` | `LIBGPU.LIB/TMD.OBJ` | `strong_unique` |
| `0x80033dc0` | `0x80034484` | `0x6c4` | contiguous | `LIBGPU.LIB/PRIM.OBJ` | `strong_unique` |
| `0x80034484` | `0x80034690` | `0x20c` | contiguous | `LIBGPU.LIB/EXT.OBJ` | `strong_unique` |
| `0x80034690` | `0x80034a04` | `0x374` | contiguous | `LIBGPU.LIB/OTAG.OBJ` | `strong_unique` |
| `0x80034a04` | `0x80035294` | `0x890` | contiguous | `LIBGPU.LIB/SPRINTF.OBJ` | `strong_unique` |

## Boundaries inferred from individual FIDs

Function IDs retain the source module and member offset, so several compatible
FID hits can imply another object base with `retail_va - member_offset`. Those
clusters are useful candidates for extending this order, but an individual FID
does not prove the original object's end, omitted functions, or alignment.
Consequently, FID-derived object placements are not mixed into the exact tables
above until their offsets agree and the complete proposed span is reviewed.
