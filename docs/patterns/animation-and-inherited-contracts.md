# Animation result and inherited behavior review

Reviewed input: `43a786dd`. All addresses below belong to GAME.EXE.
Retail files were hash-verified with `kf init`; fresh image-qualified
address, CFG/disassembly, incoming/outgoing references, strings and match
views were collected for the four reviewed functions and cache services.
This is a contract/retention review, with no function-body changes.

## Animation binder: retain the mixed pointer contract

`render_bind_animated_instance` (`800205d4 / 3a4`) has three distinct retail
return values. The existing source, public declaration and curated function
identity already agree:

| Path | Decisive instructions | Result and ownership |
| --- | --- | --- |
| Static asset | `80020660` jumps to the restore tail; its delay slot sets `v0 = 1` | The named static sentinel is success, but is not a cache record. An occupied owner slot is released first. |
| Pool exhaustion | `8002067c` branches to the restore tail; its delay slot sets `v0 = 0` | Failure; no record is installed. |
| Animated asset | `80020940` stores live state; `80020944` copies `s4` to `v0` | The live record pointer is returned. New ownership was installed by `sw s4,0(s1)` at `800206b4`. |

The five direct retail callers pass the caller's record-pointer slot in
`a0`, asset/clip/phase in `a1`–`a3`, and vertex count on the O32 stack.
The binder narrows that fifth argument with `lhu` at `80020618`.
Every caller compares the returned register only with zero:

| Caller | Call site | First result use |
| --- | --- | --- |
| `render_actor` | `8001eaf4` | `bnez v0` at `8001eafc`; zero selects static-vertex fallback |
| `render_effect` | `8001f05c` | `bnez v0` at `8001f064`; zero selects static-vertex fallback |
| `render_map_event` | `8001f1b4` | `bnez v0` at `8001f1bc`; zero selects static-vertex fallback |
| `render_weapon` | `8001f858` | `beqz v0` at `8001f860`; zero skips drawing |
| `render_hud_models` | `8001f978` | `beqz v0` at `8001f980`; zero skips drawing |

No caller dereferences the return value. The record itself is accessed through
the owner slot. `animation_cache_release` reads the record's backpointer at
`800209f4` and clears that owner slot at `800209fc`. The static success value
is never stored there, so it must never be passed to a cache lifecycle service.

The fresh incoming inventory contains five proven calls and five validated
internal branch references, with no candidate address/pointer references.
A byte survey of the entire initialized GAME load image finds no stored
literal `0x800205d4`, including unaligned occurrences. This checks potential
stored-pointer evidence beyond the source callers; it does not exclude
computed addresses or callbacks written into RAM. No claim of universal
indirect-call absence is made, and no narrower signature is introduced.

**Verdict:** retain `KfAnimationCacheRecord *`, the name and all three results.
Boolean conversion would discard the live pointer result. An integer status
or new wrapper type has no stronger source evidence. Clarify the public
comment to distinguish the static sentinel from owned records; preserve the
existing identity, signatures, callers and ABI. Original source spelling of
the sentinel remains unresolved.

## Four inherited questions: retain behavior individually

These verdicts preserve retail behavior for reconstruction. They do not
establish portable C semantics for uninitialized reads or missing returns,
and they do not mark the warnings resolved.

### Animation keyframe index reads incoming `s5`

The binder saves `s5` at `800205e8`, then its first use as an index is
`addiu s5,s5,1` at `80020790` or `addiu s5,s5,-1` at `800207a4`.
There is no initialization along the preceding paths. It is narrowed for
comparison at `800207c4` and stored at record offset 6 by `800208ac`.
Nonempty clips do not eliminate that incoming-register dependency.

**Verdict:** retain the uninitialized local and concise retail comment.
The original source/compiler explanation remains open. Initializing it
changes the reconstruction's behavior; an intentional port repair needs
animation/cache regression coverage. The separate zero-keyframe asset
precondition remains the question recorded in
[compiler-warning-triage.md](compiler-warning-triage.md).

### Effect collision class zero leaves `v0 = 1`

`effect_map_collision` (`80037850 / 76c`) masks the active effect's type to
two bits at `80037f38`. For class zero, the delay slot at `80037f4c` loads
`v0 = 1`; the failed class-one comparison reaches the common restore tail
through `80037f58`. The tail does not overwrite `v0`.
Classes one through three instead call `collision_query_world` and return
its result. This specific terminal path can be read directly despite the
earlier diagonal-cell switch being an unresolved successor in the navigator.

**Verdict:** retain the documented missing return. Do not substitute zero,
or claim that writing an explicit `return 1` recovers original source.
Whether class-zero effects should report collision is an intentional gameplay
question for the port; static inspection does not prove its desired behavior.

### Save-status success falls through

`memory_card_show_status_message` (`8002c510 / d0`) calls
`menu_load_message_image` at `8002c5b8`. It overwrites `v0` with `-1` at
`8002c5cc` only when the loader returned one. The other path preserves the
loader's register value through the restore tail; the current C falls through.

Five direct call sites discard this result: `8002b568`, `8002b6c8`,
`8002bca8`, `8002be20`, and `8002be4c`. Four overwrite `v0` with a local
status discriminator before reading it; `8002be20` reaches a jump whose delay
slot sets `v0 = 0`. The 15 candidate pointer rows at `8001248c..800124c4`
are the internal status jump table: their raw words all target labels within
`8002c550..8002c5b0`, not the function entry. The table is read and jumped
through at `8002c534..8002c54c`. Candidate labels are not promoted to callback
claims or evidence that an indirect caller consumes the return value.

**Verdict:** retain the signed return declaration and missing success return.
Discarded direct results explain current usage, not the original API spelling.
Returning zero or changing to `void` requires a separate source/ABI decision;
the cleanup does neither. Possible computed indirect uses remain unproved.

### Boss-death effect receives unwritten direction bytes

`actor_update_boss_death_sequence` (`8002f8cc / 1bc`) passes `sp + 24`
as the fifth argument at `8002fa10..8002fa18`; its preceding instructions
never initialize that eight-byte direction. Only the position x/y/z at
`sp + 32/36/40` are written, leaving the position pad unwritten too.
The constructor's common path reads/copies the direction with the unaligned
word pairs at `80036fb8..80036fd4`, so the buffer is actually read.

The caller selects kind 44. The raw constructor table at `80012c28` directs
it to `800375e8`, which selects model 17 and changes the stored kind to 18
at `800375fc`. The update table at `80012cf8` directs kind 18 to
`800395b0`: that radial-blast arm scales, computes power/radius and applies
damage, without reading the copied direction. This describes the observed
path; it does not make the uninitialized copy valid portable C or prove
that no other consumer can observe the record bytes.

**Verdict:** retain the unwritten buffer and source comment. Do not add
zero initialization as warning cleanup. The original source explanation and
any intended port initialization remain separate questions.

## Match and verification scope

All four reviewed functions remain strict objdiff 100% at the reviewed
input. The cache allocate/release services and all five binder callers were
inspected as ABI evidence. The only source change is a public header comment;
there are no new signatures, function bodies, identities, claims or banks.
The existing overall strict result remains 465/471, including unrelated
baseline residues. The full three-image build passes after the comment change;
all 97 ELF comparison objects and all three linked CPE/EXE pairs remain
byte-identical to the captured clean baseline. Ruff, local documentation links
and diff whitespace checks pass. The 885-test gate (no skips) and 97/97 type
checks passed for `43a786dd`; this batch changes only comments/documentation
and preserves those tested executable inputs. Existing data/placement
analysis debt remains open.
