# OPEN sound extent and retail-carved initialized packing

## Function Match Plan

Baseline: `66b4756`. Correct the OPEN scene-zero SoundRef extent using its
three-byte consumer and independent compiler measurement. Retain the following
retail byte separately in the census and in the complete module section.
Replace automatic zero-filled initialized packing with actual retail reads,
including the five existing packing ranges. Keep every function body, type,
initializer, profile, claim address, relocation site and alignment unchanged.
Require nonzero-byte controls and full-corpus verification; do not treat this
as closure of the opening module's separate section-base conflict.

The project matcher evidence pass was completed before editing: hash-checked
OPEN image, full disassembly/CFG, both xref directions, strings, current match
states, the sole controller call site, neighbors, source history and earlier
scene ownership dossiers. No game was booted. The only C edit changes the
sound's DATA size, not its declaration, initializer or consumers.

## Three-byte object, separate following byte

OPEN `80035874` contains bytes `09 00 43 00`. The first three are program 9,
tone 0 and note `43`; the final zero is not a SoundRef field. The common
audio.h type has exactly three `u8` fields, and the pinned compiler independently
reports sizeof(SoundRef) = 3. The preceding complete camera path ends at
`80035874`, while the next independent overlay-rectangle array begins at
`80035878`. Its own reference is `800149cc/800149d0` in opening_scene3_run.

The old census explicitly called the four-byte allocation a sound reference
plus an alignment byte. The old source-size annotation nevertheless made that
four-byte claim appear to agree with the C object. The independently measured
source size introduced in `66b4756` exposed the error. The corrected identity
and DATA claim own **three bytes**; a separate one-byte unclassified census
row at `80035877`, file offset `24077`, preserves the remaining byte. No
interior global, fourth field, fake initializer or new address-derived identity
is introduced. Scope stays global and the logical scene-zero owner stays.

There is only one direct sound reference, at `800142fc/80014300`, with zero
owner-relative addend. No reference directly targets +3, and the following
array is independently addressed. The revised module has 1047 bytes of named
DATA objects plus this one byte of inter-object packing, still **1048 complete
section bytes**. The 988-byte first physical run and 60-byte second run remain;
the intervening `19b4` retail bytes do not become fabricated module padding.

## Function evidence snapshots and verdicts

### OPEN opening_scene0_run — 80014268 / 174

Strict baseline and final: **100%**. Signature `void(void)`, 32-byte frame
with s0-s2 and ra saved. Its sole external caller is opening_run at `800157d0`,
with a NOP delay slot and no argument/result contract. The controller loads
scene-zero resources, finds entity IDs 11/12, sets yaw `c00`, starts the
seventeen-point camera path, polls input and advances the path. After point
seven, its first entity turn calls `sound_ref_play` at `80014304` using the
sound address from `800142fc/80014300` and volume 100 in the call delay slot.
It then counter-rotates yaw by 40 and applies signed 256-step lighting fades.
The eleven direct calls, twelve validated non-call references including two
internal jumps, and zero strings are unchanged. `jr ra` at `800143d4` restores
sp by 32 in its delay slot. Neighbors are the camera-path step ending at
`80014268` and the fade drawer starting at `800143dc`.

### OPEN sound_ref_play — 8001a220 / 48

Strict baseline and final: **100%**. Signature
`void(const SoundRef *, s16 volume)`, with the controller as its sole caller.
The three unsigned byte loads are exactly:

```
8001a234  lbu v1,0(a0)
8001a238  lbu a2,1(a0)
8001a23c  lbu a3,2(a0)
```

The active VAB ID is a signed halfword at audio_state+4. Volume is sign-extended
and stored into both later O32 argument slots at sp+16 and sp+20. The call to
audio_play_voice is at `8001a250`, with program moved to a1 in its delay slot.
The 32-byte frame and `jr ra; addiu sp,sp,32` return at `8001a260/264` remain.
There are one direct call, one validated data pair and no strings. It follows
audio_set_listener_transform and precedes audio_play_voice at `8001a268`.
This project-specific sound tuple/glue is game code, not an SDK body; both
functions are absent from the vendored inventory. The underlying SDK services
remain external and are not reconstructed or counted as progress.

All seven functions in opening_scenes retain their prior native scores:

| Function | Final strict native score |
| --- | --- |
| opening_scene0_run | 100% |
| opening_scene1_draw_fade | 100% |
| opening_scene1_run | 100% |
| opening_entity_transition | 99.921260% |
| opening_scene3_run | 99.931370% |
| opening_ending_scene_run | 100% |
| opening_ending_scroll_run | 97.129630% |

The three non-exact siblings retain their existing frame/instruction residues;
no new compiler-mechanism attribution or code-generation trial is made. Detailed
unchanged per-function evidence is in [the ownership pilot](open_opening_scenes.md).

## Initialized packing must come from the image

Previously `_module_data` inserted zero bytes whenever the working member
alignment required packing. It now requires an explicit retail reader. The
packing range must equal the physical interval from the previous loaded
object's end to the next object's start, and the read must be complete.
Missing readers, truncated payloads and different physical gaps are rejected.
An alignment requirement alone is not permission to synthesize initialized
bytes or bridge a larger ownership gap. BSS remains allocation-only and is
not given executable initializers.

The six current ranges, totaling twelve bytes, are independently checked:

| Image / module | Gap VA / size | Retail bytes |
| --- | --- | --- |
| GAME player_use_item | `80055846 / 2` | `00 00` |
| GAME cd_file | `80057b41 / 3` | `00 00 00` |
| GAME actor | `8005617a / 2` | `00 00` |
| OPEN opening_scenes | `80035877 / 1` | `00` |
| OPEN resources | `800372e1 / 3` | `00 00 00` |
| OPEN resources | `800372e7 / 1` | `00` |

The five pre-existing gaps retain their census rows and classifications. Their
zero values do not justify assuming all future gaps are zero. Synthetic tests
retain `f1 2a 80` and exercise one-byte values 00/7f/ff, without shifting named
symbols or the subsequent `R_MIPS_32 external+4` relocation. Separate controls
reject unavailable/short/oversized padding and a packing length inconsistent
with the physical claim interval. No gap receives an invented global identity.

## Verification

All 112 compiled source objects are byte-identical to the baseline. Only the
opening_scenes module target changes, solely its SoundRef symbol size from
four to three. Every runtime section, payload, extent, alignment and ordered
relocation is unchanged. All **484 function-score rows** remain unchanged,
preserving **360/471 exact game functions** and thirteen exact vendor controls.
All 100 independently measured source DATA sizes now agree with their claims.

The complete source/target opening data bytes still match their two respective
retail intervals, including the separately accounted zero at `80035877`.
Its owned-symbol-size error is gone, but its conflicting section bases remain.
Strict source data is still **8/60**, SDK data **4/4**, and target relinking
**110/116**, with the same six conflicts. Unmatched reached config ranges stay
623; source/header DAT_ occurrences stay 167. These are not full coverage or
linked-executable equality claims, and no function is newly banked.

Reproduce in `nix develop` after `kf init --retail-dir <verified-extraction>`:

```sh
kf sema --image open disasm sound_ref_play --blocks
kf try --unit open.opening_scenes
kf build compare
python -m unittest tests.test_delink tests.test_open_opening_scenes
python -m unittest discover -s tests
kf build
```

The full build remains red on strict data, reference closure and placement;
no failure gate, compiler profile or alignment requirement is relaxed.

All **596 local tests** pass without skips, with Ruff and diff checks clean.
`nix flake check -L` passes, including the new nonzero/physical-gap controls;
the sandbox suite has 101 expected local-tool/artifact skips. Focused matches
preserve their existing exact/non-exact verdicts. No generated artifacts or
baseline changes are committed.
