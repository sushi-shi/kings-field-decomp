# Unknown-field format provenance

The 79 remaining `unknown_` declarations belong to 33 reviewed owners. None is
currently identified as an unnamed field *inside* an established complete SDK
record. Several owners embed standard SDK types or point to standard files;
those boundaries are documented below. The surrounding game layouts are
supported by their consumers, but their unknown fields remain unresolved.
This audit does not certify every possible historical tool or SDK revision.

`Custom` below means the observed game grammar/runtime layout does not match
the cited standard owner. It does not attribute a particular original author,
converter, or historical C declaration. `Unresolved aggregate` also preserves
uncertainty about subdivision and original ownership of the contiguous region.
All 79 field spellings and extents remain unchanged by this format audit.

## Pinned SDK evidence

Paths below are relative to `$PSYQ_SDK`, exported by `nix develop` as the
repository's complete Psy-Q Release 2.5 media tree. Headers are under
`isa board/PSXLIB/INCLUDE/`; line numbers count the original files, including
CRLF lines. The media archive is pinned in [flake.nix](../../flake.nix) to
`sha256-SaLzzryjqEIclPHeQ7nSDnN1CwQrCDHdKxcy0o+Ud4M=`.

| Reference | Exact pinned source | What the source establishes |
| --- | --- | --- |
| S1 | `LIBGTE.H:99–120`, `216–224`, `336` | MATRIX has a nine-short rotation and three-long translation; VECTOR and SVECTOR explicitly have fourth `pad` lanes; CVECTOR has `r/g/b/cd`; DVECTOR has two halfwords. SDK transform/MIMe signatures establish actual boundaries when the game passes those types. |
| S2 | `LIBGPU.H:298–329`, `369–383`, `601–608` | RECT, DR_ENV, DRAWENV, DISPENV and textured-quad packet declarations. In this version DRAWENV is 0x5c and DISPENV 0x14 bytes. DISPENV.pad0 is *inside* its last two bytes, not the eight-byte gap after the environment arrays. |
| S3 | `LIBGS.H:29–50`, `87–96`, `99–144`, `240–256` | SDK coordinate/object/OT/sprite structures and GsMIMEV/GsMIMEN runtime pointer tables. Their complete layouts differ from the game's camera, entity, display, menu and animation-cache owners. |
| S4 | `LIBSND.H:58–116`, `137–151` | VabHdr/ProgAtr/VagAtr and VAB/sequence API declarations. These identify pointed-to sound-bank data and returned IDs, not the KfAudioState glue record. |
| S5 | `LIBSPU.H:175–232`, `293–304` | SpuVoiceAttr, SpuReverbAttr and SpuCommonAttr are distinct API attribute structures; none matches KfAudioState's pointer/ID/listener/voice-slot layout. |
| S6 | `isa board/PSXBIN/BIN/CARD.DOC:308–336` (Shift-JIS) | The early memory-card header/icon/data division. Its title-padding placement differs from the later manual; see the explicit caveat below. |

These are authentic header declarations, not substitute structures reconstructed
from size alone. A declared SDK `pad` must still be preserved by a byte codec;
its name does not authorize overwriting it or asserting that every consumer
ignores it.

## Primary format manuals and version limits

The external documents are Sony-authored manuals hosted by an archive mirror.
They are later corroboration, not evidence that Release 4.4 produced this 1994
game. [File Formats](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf)
is the November 1998 manual for runtime release 4.4; the inspected PDF has
SHA-256 `0f1a35952cc93405a22ded78cd19ef270b09f3accc35f4a6bfadd32e4a9b17a7`.
Printed chapter/page numbers below avoid ambiguity with PDF page numbering.

- **F1 — TMD:** [File Formats, pp. 2-24–2-27](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf#page=68)
  documents the model header, object table and vertex/primitive records. The
  complete correspondence in `tmd.h` is standard; surrounding registry and
  gameplay tables are separate owners.
- **F2 — ANM:** [File Formats, pp. 3-16–3-19](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf#page=195)
  describes TIM-associated **2D image animation**, with ID 0x21, version 3,
  sequence/sprite/CLUT groups. It does not identify the game's asset-relative
  3D morph clips, camera points, or gameplay animation state.
- **F3 — card file:** [Run-Time Library Overview 4.4, p. 5-8, table 5-6](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libovr.pdf)
  specifies the SC prefix, title, pad, CLUT and icons. The game directory and
  save payload follow that standard header and have their own grammar.
- **VDF:** [Data Conversion Utilities, p. 2-22, figure 2-7](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Dataconv.pdf#page=100)
  identifies object index, vertex range and eight-byte delta records. The
  [existing VDF audit](semantic-field-names.md#morph-record-format-follow-up)
  confirms the whole KfMorphObject correspondence across 827 shipped records.
  The asset wrapper and keyframe grammar are separate from that VDF record.

The early CARD.DOC [S6] lists one pad byte before the 64-byte title and 27 after;
the later overview [F3] lists the title immediately after the four-byte prefix,
then 28 pad bytes. Both place the CLUT at +0x60 and the first icon at +0x80.
The early document therefore cannot establish the current title offset by
itself. Retail/header evidence must decide that layout; neither revision gives
meaning to KfSavePayload.unknown_0e0 or unknown_2548. This audit does not change
the already modeled card header.

## Complete owner coverage

Each field listed here retains the `retain_unresolved` decision from the
[field ledger](../unknown-field-review.tsv). The declaration link and the
ledger provide exact offsets, widths, provenance and transport observations.
A standard embedded member is not evidence for a neighboring unknown lane.

| Owner | Unknown fields (79 total) | Format/ownership verdict and evidence |
| --- | --- | --- |
| [KfActor](../../include/kf/game/actor.h) | `unknown_0c`, `unknown_1a`, `unknown_46` | Custom live actor: action, health, cached animation and tile state surround SDK VECTOR/KfRotation members. All three unknown ranges lie outside those members [S1]; no GsDOBJ layout match [S3]. |
| [KfActorDefinition](../../include/kf/game/actor.h) | `unknown_38` | Custom MIXA actor definition: action tables, combat stats, packed sound selectors and attachment triples. The +0x38 lane is outside any SDK vector; a nonzero shipped row also rejects all-zero padding reasoning. |
| [KfActorPlacement](../../include/kf/game/actor.h) | `unknown_07`, `unknown_0e` | Custom MIXA placement expanded by actor_pool_load_placements. The byte flags and tile/local-coordinate grammar are not a TMD object, GsDOBJ or SDK vector [S1,S3,F1]. |
| [KfArmorRecord](../../include/kf/game/equipment.h) | `unknown_00`, `unknown_12` | Custom COM equipment/combat row, selected through item-ID-biased tables. No SDK equipment format was identified. Its nonzero suffix cannot acquire shop-price or reserved-field meaning from SDK documents. |
| [KfAudioState](../../include/kf/lib/audio.h) | `unknown_06`, `unknown_0e` | Custom runtime glue: pointers to VAB/SEQ files, returned SDK IDs, listener transform and ten tracked voices. The two unknown ranges are outside the embedded SDK vectors and outside the pointed-to VabHdr/ProgAtr/VagAtr data [S1,S4]. It is not SpuVoiceAttr or SpuCommonAttr [S5]. |
| [KfCameraPathPoint](../../include/kf/lib/map.h) | `unknown_1a` | Custom serialized camera key point. VECTOR and SVECTOR occupy +0x00..+0x17; speed is +0x18 and unknown_1a follows both complete SDK members [S1]. No TOD/ANM format correspondence is established [F1,F2]. |
| [KfCameraPathState](../../include/kf/lib/map.h) | `unknown_5e` | Custom interpolation state with path pointer, six SDK vector members and game counters. unknown_5e follows point_index, outside every vector. SDK vector padding does not identify this halfword [S1]. |
| [KfCollisionTarget](../../include/kf/game/collision.h) | `unknown_1a` | Custom collision-query output: complete VECTOR at +0, SVECTOR at +0x10, radius at +0x18, then six unknown bytes. The tail is outside the two standard members [S1]. |
| [KfDisplayState](../../include/kf/game/render.h) | `unknown_01` | Custom double-buffer bookkeeping: index, asset pointer, primitive arenas and ordering tables. No DRAWENV, DISPENV or GsOT whole-record correspondence; their authentic members are modeled separately [S2,S3]. |
| [KfDisplayStateOpen](../../include/kf/open/render.h) | `unknown_01` | Custom OPEN double-buffer bookkeeping. Its separate active ordering-table pointer and overlay-specific extent distinguish it from both the GAME aggregate and SDK environment/OT records [S2,S3]. |
| [KfEffectRecord](../../include/kf/game/effect.h) | `unknown_0a`, `unknown_2a` | Custom effect lifecycle record with SDK position and direction/rotation views. unknown_0a precedes VECTOR; unknown_2a follows three unsigned scale halfwords. No whole SVECTOR scale owner or SDK API argument at that address is established [S1]. |
| [KfEffectSprite](../../include/kf/game/render.h) | `unknown_0c`, `unknown_16` | Custom animated-effect descriptor, not a GPU primitive or GsSPRITE [S2,S3]. SDK SVECTOR rotation begins at +0x0e and already includes its pad at +0x14; unknown_0c and unknown_16 are outside it [S1]. |
| [KfFloorItem](../../include/kf/lib/item.h) | `unknown_03`, `unknown_10`, `unknown_15` | Custom expanded floor-item record. Three signed position words plus unknown_10 resemble a possible VECTOR extent, but reviewed consumers read coordinates separately; no complete-object/API evidence establishes that SDK owner [S1]. The copied +3 byte and trailing animation gap remain separate unknowns. |
| [KfFloorItemPlacement](../../include/kf/lib/item.h) | `unknown_03` | Custom MIXA/OPEN placement record with sprite ID, packed appearance, tile bytes and signed local offsets. Loader copies +3 into the runtime item. This is not a GPU sprite or Sony ANM frame [S2,S3,F2]. |
| [KfFloorItemStateOpen](../../include/kf/open/render.h) | `unknown_08`, `unknown_14` | Unresolved aggregate intervals around custom material, texture selectors, count and item array. The CVECTOR inside material is complete; its scope does not extend to either unknown range [S1]. No standard SDK container correspondence. |
| [KfGraphicsRuntimeGame](../../include/kf/game/graphics.h) | `unknown_20108`, `unknown_201f4`, `unknown_241a6`, `unknown_241b6`, `unknown_241cd`, `unknown_241fa` | Unresolved intervals in a curated startup-cleared runtime aggregate. DRAWENV/DISPENV arrays, SDK vectors and known registry/cache members retain their own extents [S1,S2]. The +0x20108 gap starts after both complete DISPENVs; other ranges are outside the SDK members, not their reserved fields. |
| [KfGraphicsRuntimeOpen](../../include/kf/open/render.h) | `unknown_20108`, `unknown_2011c`, `unknown_20124`, `unknown_22078`, `unknown_24786` | Unresolved intervals in the OPEN startup-cleared aggregate. Complete SDK environment/vector members are already accounted for [S1,S2]. Similarity to GAME scratch or registry regions does not prove the five intervals are absent features, spare entries or padding. |
| [KfHudSprite](../../include/kf/game/render.h) | `unknown_01` | Custom state byte plus KfSpriteQuad descriptor. The descriptor feeds a separately constructed GPU packet. No direct POLY/SPRITE/GsSPRITE correspondence for this owner or its +1 byte [S2,S3]. |
| [KfMagicRecord](../../include/kf/game/magic.h) | `unknown_12` | Custom COM magic/combat and sound-selector row. SDK sound APIs interpret the separately supplied program/tone/note values; they do not define this record or its two-byte suffix [S4]. |
| [KfMapEvent](../../include/kf/lib/map.h) | `unknown_0c`, `unknown_0d`, `unknown_11`, `unknown_22`, `unknown_42` | Custom runtime dialogue/animation/collision state expanded from MIXA definitions. VECTOR reference_position and SVECTOR rotation are known subobjects; all five unknown lanes lie outside those subobjects [S1]. Save transport of +0x0d does not identify meaning. |
| [KfMapEventDefinition](../../include/kf/lib/map.h) | `unknown_0b`, `unknown_0c`, `unknown_16` | Custom MIXA event row with character/model indices, dialogue limits, behavior, coordinates and radius. Its three unknown lanes are not fields in TMD, Sony ANM, GsDOBJ or a standard SDK vector [S1,S3,F1,F2]. |
| [KfMapObject](../../include/kf/lib/map.h) | `unknown_01`, `unknown_06`, `unknown_29` | Custom object/action instance built from a placement and definition. Complete VECTOR/KfRotation subobjects coexist with a game-specific link union and action timer. The three unknown lanes are outside the standard vector views [S1]. |
| [KfMapObjectDefinition](../../include/kf/lib/map.h) | `unknown_01`, `unknown_06` | Custom COM behavior/radius row loaded into the game definition table. SDK model formats describe geometry; they do not identify these game interaction bytes [F1]. |
| [KfMapObjectPlacement](../../include/kf/lib/map.h) | `unknown_01` | Custom placement shared by GAME and OPEN, with tile bytes, local coordinates, yaw and an eight-byte game link payload. The +1 lane is not part of a standard vector or TMD object [S1,F1]. |
| [KfMapObjectState](../../include/kf/lib/map.h) | `unknown_25a8` | Unresolved ten-byte interval in a custom definition/live-object/control aggregate. Following allocation sequences are game policy; SDK object-table structures have different members [S3]. |
| [KfNotificationSprite](../../include/kf/game/notify.h) | `unknown_01` | Custom notification visibility plus screen-sprite descriptor. Renderer constructs a GPU packet from selected fields; the source +1 byte has no corresponding packet field [S2,S3]. |
| [KfOpeningEntity](../../include/kf/open/resources.h) | `unknown_01`, `unknown_06`, `unknown_1e` | Custom OPEN entity. Position and scale use authentic VECTOR/SVECTOR types, but rotation is a six-byte KfEulerAngles consumed by game matrix_set_rotation_yxz. unknown_1e is not proved to be SVECTOR.pad; the other two unknown lanes also lie outside SDK members [S1]. |
| [KfOpeningEntityState](../../include/kf/open/resources.h) | `unknown_500`, `unknown_control_50a`, `unknown_control_50c`, `unknown_control_50e` | Unresolved custom pool/control tail after 32 entities. Three clear stores establish control addresses, not their semantics. Corresponding GAME sequence positions are a candidate relationship, not an SDK declaration [S3]. |
| [KfPlayerProgressState](../../include/kf/game/player.h) | `unknown_01` | Custom level/floor-progress bytes, embedded in the saved player record. No standard PlayStation player-progress format was identified; serialization does not turn the +1 byte into a card-header field [F3]. |
| [KfPlayerState](../../include/kf/game/player.h) | `unknown_0f`, `unknown_3a`, `unknown_54`, `unknown_5d`, `unknown_65`, `unknown_72`, `unknown_7b`, `unknown_a3`, `unknown_ce`, `unknown_df` | Custom gameplay/save object containing stats, equipment pointers, timers and embedded camera vectors. All ten unknown ranges are outside its declared VECTOR/SVECTOR subobjects [S1]; neither the SDK card header nor VAB/GPU records describe them [S2,S4,F3]. |
| [KfSavePayload](../../include/kf/game/save.h) | `unknown_0e0`, `unknown_2548` | Custom slot payload after the separate standard card header and game directory. The two holes are not the card-header pad, CLUT, icon data or filesystem FAT [F3]. save_file_write_slot/read_slot establish copied ranges, not unknown-field meaning. |
| [KfWeaponRecord](../../include/kf/game/equipment.h) | `unknown_00`, `unknown_14`, `unknown_22` | Custom COM combat/render row with an SDK SVECTOR at +0x24. Its standard pad is +0x2a, already named through render_rotation; unknown_22 precedes that object. The six-byte translation at +0x1c is KfVec3s, not a padded SVECTOR [S1]. |
| [MenuTileSprite](../../include/kf/game/menu.h) | `unknown_05`, `unknown_07` | Custom 12-byte menu descriptor, not GsSPRITE or a GPU packet [S2,S3]. unknown_05/07 may merit comparison with the game MenuSpriteDef halfword UV fields, but SDK byte UV widths and zero high bytes alone cannot decide the original descriptor widths. |

## What documentation can and cannot settle

The weapon rotation provides a positive control: retail passes the eight-byte
member to SDK RotMatrix, and its complete layout matches SVECTOR [S1]. Its pad
is therefore retained as SDK storage in the Rust codec. Weapon translation is
a separate six-byte triple assembled into a VECTOR; absorbing unknown_22 into
it as an SVECTOR pad would change the owner without supporting evidence.

Likewise, KfEffectSprite.rotation already includes its SDK pad, while its two
unknown neighboring halfwords are outside the object. KfOpeningEntity.rotation
is passed to a game helper taking three halfwords, so the following +0x1e lane
cannot be promoted to SDK padding from proximity. The possible VECTOR shape
in KfFloorItem and possible halfword UV lanes in MenuTileSprite remain source
modeling hypotheses requiring their own consumer/alias evidence.

The local `KfAnimClip.unknown_02` in `src/game/pool.c` is outside the requested
79 header fields, but the standard-format check also leaves it unresolved:
VDF defines the nested morph object, and Sony's 2D ANM format does not match the
surrounding clip grammar. No zero-padding, unused-field or reserved-field rename
is supported by this audit. The remaining work is retail behavioral and
ownership recovery, including indirect and alias accesses, not merely finding
a generic PlayStation struct with the same size.
