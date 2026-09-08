# Typed weapon render fields

## Function Match Plan

Start at `00a68b6`, with the shared type still leaving proven rendering fields
opaque. All six GAME semantic queries cover the three functions below plus
`render_frame`, `common_resources_load` and both neighboring geometry emitters.
Source history, SDK contracts and the existing equipment/render evidence were
reviewed. The selected GCC 2.5.7 profile and relocation inventories are retained.

`render_weapon` loads an unsigned projection distance at record+0x10, signed
translation halfwords at +0x1c/+0x1e/+0x20 and supplies record+0x24 to the SDK
`RotMatrix` SVECTOR input. Its depth calculation rereads +0x20 with `lhu` and
explicit sign extension before shifting by five. The fixed 0x2c stride leaves
room for the complete eight-byte SDK rotation; the loader negates its Y
component at +0x26. That establishes the former candidate `mirrored_angle` as
render rotation Y. The earlier combat members and the unresolved spans at
+0x14..+0x1b and +0x22..+0x23 keep their extents.

First hypothesis: type the translation as the existing six-byte `KfVec3s`,
projection distance as `u16` and rotation as authentic `SVECTOR`. Propagate the
rotation name into the loading loop. Use the actual mutable weapon-array
pointer in the equipped slot and renderer so the SDK's non-const pointer
contract is met without a const-removing cast. The equip function obtains
that pointer directly from `weapon_records`; no immutable storage is exposed.

Compare the signed field forms first. Any change at the depth halfword or
mirroring load must be judged by raw load/extension instructions rather than
score alone. Retain original load ordering across calls, the 176-word copy,
sixteen-record mirroring loop, all constants, references and delay slots.

Vendor negative control: the renderer's player/asset/animation data and the
loader's resource-specific copy plus sixteen angle negations are game logic,
absent from the vendored census. SDK math bodies remain supplied SDK functions.
No selected function has strings or outgoing candidate references.

| GAME function | VA / bytes | Strict before | Blocks/JAL/branches/returns | Final verdict |
| --- | --- | ---: | --- | --- |
| `weapon_records_load_and_mirror_angles` | `800150a8 / 54` | 100 | 5/0/2/1 | 100%; raw unchanged |
| `player_equip_weapon` | `80016a30 / f4` | 100 | 7/4/3/1 | 100%; raw unchanged |
| `render_weapon` | `8001f798 / 118` | 100 | 4/10/2/1 | 100%; raw unchanged |

Require fresh focused builds and exact raw/relocation equality for the banked
functions, followed by full build, existing tests, lint and diff checks. The
existing whole-image closure failures remain separately recorded.

## Final verification

The direct signed fields reproduce both exact listings without additional
numeric casts. In particular, the depth calculation retains `lhu` plus the
explicit sign-extension shift sequence, and rotation mirroring retains its
unsigned load and halfword store. There is no remaining codegen divergence.

All 20 functions in the three selected units have unchanged linked words,
ordered calls and data referents. Nineteen remain retail-exact and the existing
partial `player_move_horizontal` is unchanged. Overall strict status
remains 439/471 (GAME 332/362, OPEN 106/108, PSX 1/1).

Nine C pointer casts and one redundant scalar cast disappear. The C pointer
count is now 530, a reduction of 276 from the initial 806. The full AST census
has 789 written casts, including 41 header casts; all 112 image variants parse
without errors. The renderer now contains no casts.

All 713 repository tests pass with nine skips. Ruff and `git diff --check`
pass. The source/header changes received fresh focused builds and full
`kf build`; the existing data/reference/placement gates still fail in all
three images, with no artifact failure. No toolchain, relocation, ownership
or closure gate is changed. Bank only the three selected exact functions.
