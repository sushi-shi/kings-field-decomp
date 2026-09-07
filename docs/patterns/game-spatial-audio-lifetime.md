# GAME spatial-audio distance lifetime

## Function Match Plan

At `3c38b54`, review GAME `80032cf0 audio_play_spatial`, 712 retail bytes,
strict 97.724720%, in `game.audio` under the unchanged GCC 2.5.7 probe.
Hash-validate retail and read all six image-qualified semantic views, all
178 words, both direct callers, both adjacent wrappers, the preceding VAB
closer, the game angle/voice callees, pinned SDK declarations and history.
The source has 704 bytes and a 48-byte frame versus retail's 56-byte frame.
Its first divergence is the frame/saved-register sequence; the distance
result uses v0, whereas retail uses s1 and later replaces it with attenuation.

Both wrappers preserve the two pointers and sign-extend the third argument
to s16. The default wrapper supplies signed distances 16000 and 28000;
the explicit wrapper forwards the fifth word from its caller stack. Keep
the u32 success result, signed word positions, halfword volume/yaw, packed
three-byte SoundRef and the unchanged KfAudioState owner.

Retail has 35 blocks, eighteen conditional branches, two internal jumps,
six divide traps, five calls and one return. The out-of-range path returns
zero through the shared epilogue without playing a voice; its branch delay
slot computes the attenuation numerator's subtraction. Seven validated
data-address pairs refer to the existing audio state, with no candidate
outgoing references, local strings or unresolved transfers. Call targets are
SquareRoot0, vector_xz_to_angle, rsin, rcos and audio_play_voice, in that order.
The three SDK math bodies are independently attributed to Release 2.5
LIBGTE (MSC+0x478, GEO+0x930 and GEO+0xa38); this distance/pan policy is not
vendored and is absent from that roster.

The exact OPEN counterpart `80019f44` also evolves one word from distance
to attenuation. Its different panning policy and address lifetimes are not
transferred. GAME retail places the scaled square-root result in s1 at
+0xa4, consumes it in the cutoff/subtraction, and overwrites s1 with the
quotient at +0xe8; only that attenuation is used afterward. The original
source introduced separate distance/attenuation locals in `7c7c843`.

Test using the existing attenuation local for the distance result and range
test, then overwriting it with the same Q7 ratio. Remove the separate distance
local only. Preserve every expression, predicate, clamp, call, memory width
and referent, including the retail `(tone & 0x80) == 1` comparison. No forced
register, dummy lifetime, header, compiler-profile or relocation change.
Compare from the first raw divergence and retain only evidence-supported
source. The other fourteen GAME audio bodies and five exact OPEN spatial
functions are controls. Require strict 100% and all raw words/references for
closure, followed by full build, Ruff, existing tests and whitespace checks.

The nearby GAME initializer was also scouted: its 41 words differ only in
the order of -1 and 9 materialization. This repeats the known observation in
`game-view-transform.md`; no constant-carrier local is introduced or initializer
source changed.

## Exact result

The single evolving local reaches strict **100%**, from 97.724720%.
The body grows from 704 to the retail 712 bytes and the frame from 48 to
56 bytes. The distance result now uses s1 before that register receives
attenuation, restoring the complete register/save/restore sequence. No
padding, forced register or additional operation is needed. This is a source
lifetime witness, not an attribution of a particular optimizer mechanism.

Fresh compilation and numeric relocation resolution reproduce all 178 retail
words, five ordered calls and seven data-address pairs. The target object
itself also relinks to the hash-verified retail body. The complete body hash is
`34e1297d570c0b731e2b33e800d2fab9ccb32ef5c37f4e2c556e4758799f34c1`.
The range-rejection path, all divide checks, original tone-bit comparison,
six outgoing voice arguments and the restoring return delay slot agree.

All other fourteen GAME audio bodies keep their previous resolved words and
ordered references. Fourteen GAME audio functions and all five OPEN spatial
functions are independently raw-exact. A separate compilation using the
committed `3c38b54` player/map headers plus only this source change produces
identical whole-unit text and non-debug relocations to the live build. This
isolates the result from concurrent player-vertical-state naming. Every
reported exact control's object also agrees with the fresh compilation.

Across all 484 report rows, only GAME spatial audio changes. GAME advances
310 -> 311/362 exact; OPEN stays 98/108 and PSX 1/1, totaling 410/471.
Ruff, all 678 existing tests (80.782 seconds), and whitespace checks pass.
Full `kf build` was run and remains red on existing data/ownership/placement
gaps: source data PSX 0/1, GAME 9/42, OPEN 2/19; target relinks 1/1, 75/77
and 34/38; six conflicting-section cases and zero artifact failures.
No tests, tooling, compiler profile, shared layout or curated reference changes
belong to this campaign.

Selected banking of this function and the preceding exact actor distance
refuses the concurrent unstaged player-vertical-state inputs and actor
damage-naming hunk. Do not override that guard or stage those independent
edits. The verified audio reconstruction is committed separately; these two
baseline updates remain pending clean campaign inputs and revalidation.
