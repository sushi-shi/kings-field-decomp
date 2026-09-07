# Ground-effect helpers and branch roles

## Function Match Plan

Replace the numeric helper suffixes with ground-trail and ground-branch names.
Carry the existing four branch roles through a halfword enum field, constructor
local and word-argument child helper. Encode roles at the variadic boundary.
Keep timer values, generation counts and parent indices in their own domains.

All addresses identify GAME.EXE. The current source/history and cached retail
CFG/disassembly, callers, callees, data references and match dossiers were
reviewed alongside [effect update constants](game-effect-update-constants.md)
and the [effect protocol](game-effect-protocol.md). The two helpers are adjacent
between the direction transform and dispatcher; that adjacency is not a claim
about original translation-unit ownership. Neither helper references strings.

| Function | Address / extent | Evidence snapshot and intended change |
| --- | --- | --- |
| `effect_pool_construct` | `0x80036f44 / 0x82c` | `lhu` at `0x800371ac` reads the sixth argument; `sh` at `0x800371b8` stores record +0x3a. The following halfword comparison skips sound for leaf 255. Decode that same read into the branch-role enum. |
| `effect_spawn_trail_kind13` → `effect_spawn_ground_trail` | `0x800387bc / 0xf8` | Rotates/scales the direction offset, adds parent X/Z and constructs kind 0x13 (decimal 19); its sixth argument is the parent pool index. Four proven calls from the Moonlight dispatcher path. Rename only; preserve the index domain. |
| `effect_spawn_ground_kind6` → `effect_spawn_ground_branch` | `0x800388b4 / 0x184` | Calls sine/cosine, samples the validated floor-height grid and constructs kind 6. Fourth argument is saved as a word and forwarded at `0x80038a18` in the constructor call delay slot. Type the parameter while retaining legacy word width. |
| `effect_update_dispatch` | `0x80038a38 / 0x180c` | Root spawns roles 1/2 at quarter/three-quarter turns; each side spawns leaf 255. Three proven branch-helper call instructions represent four source calls because the leaf paths share a tail. Both role switches read record +0x3a as a halfword. Update helper names and inherit typed field comparisons. |
| `magic_cast` | `0x8003a2a0 / 0x4c0` | Both Fire Wall construction paths supply zero in the sixth argument slot for the root role. Encode the named enum as an integer variadic argument. |

## Representation and scope

`KfEffectGroundBranchRole` uses unsigned-halfword storage. Its four values keep
the existing meanings: root 0, quarter-turn branch 1, three-quarter-turn branch
2, leaf 255. The helper parameter uses `KF_ENUM_PARAM(..., s32)` and forwards
`KF_ENUM_ENCODE(s32, branch_role)`, retaining the word value until the constructor
reads its low halfword. The direct root calls encode to `u16`, which promotes to
integer for the variadic call. No raw enum object crosses that boundary.

The independent frames-remaining sentinel also equals 255 but is not a branch
role. Generation count shares the propagation union and remains a count. The
trail helper's parent index and both helpers' geometry remain unchanged.

Cached match states are historical: both helpers and the constructor were 100%,
the dispatcher 96.93957%, and casting 98.89145%. These are not verification of
current source. Existing baseline/evidence records retain their historical
names and hashes; current canonical identities and shared declarations receive
the helper names. No relocation target address or claim changes.

Builds, compiler checks, tests, post-edit matches and banking remain deferred
until naming is finished, as requested.

## Source result

All five planned functions have the same final verdict: source naming and type
propagation reviewed; compiler and post-edit match verification deferred. Both
helper definitions, shared declarations, all eight source call sites and two
canonical identities use the semantic names. The branch field and constructor
local use the halfword enum, the child helper retains its legacy word argument,
and both direct root calls encode their variadic values. The two dispatcher
switches and constructor leaf comparison use the typed field directly.

The field extent, claims, encoded constants and curated evidence tiers remain
unchanged. Source/reference review and `git diff --check` completed. Full literal
accounting reconciles all 111 C files and 5,997 retained numeric/character
occurrences with their reasons, including the changed constructor expression.
No build, compiler check, test, post-edit match or banking ran. The remaining
unresolved fields and resource identities keep the broader naming goal open.
