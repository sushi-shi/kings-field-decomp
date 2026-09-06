# GAME projection/morph scratch and allocation retry

## Function Match Plan at `40b5700`

The complete graphics-owner pilot now preserves both the corrected projected
sprite and the already exact frame renderer. Its scratch interval remains
opaque: the projection base is `800911b0`, morph base is `800930f0`, and the
next known texture-page table begins at `80095038`. This establishes their
relative addresses, not 1000/1001-element GAME allocation capacities. OPEN's
1000 guard is independent and must not be transplanted into GAME.

The four selected consumers received all six image-qualified semantic views,
full raw bodies, callers, adjacent boundaries, source history and SDK checks.
They are game-specific traversal/cache policy around independently attributed
Sony projection, depth, memory and morph operations, not vendor bodies.

| GAME function | Hex extent; current strict score | Evidence and invariant |
| --- | --- | --- |
| `8001c60c tmd_project_vertices` | `9c`; 100% | Nine call sites supply word vertex counts. 64-byte frame, two address pairs, RotTransPers/ReadSZ2 calls, zero guard owning count-minus-one, eight-byte cursors, packed word screen output and halfword depth/perspective. |
| `8001c6a8 tmd_project_vertices_shift` | `ac`; 100% | Weapon caller supplies word count and shift 3. Same two pairs/calls; 64-byte frame, unsigned-byte shift mask and full signed depth shift before narrowing. |
| `8001c754 tmd_transform_vertices` | `a4`; 100% | Effect-sprite caller supplies word count. 72-byte frame, two pairs, one RotTrans call, halfword X/Y and duplicated Z outputs; zero guard and eight-byte strides. |
| `800205d4 render_bind_animated_instance` | `3a4`; 98.798290% | Five callers pass typed cache slot, u16 asset/clip/phase and fifth-stack vertex count narrowed to u16. 72-byte frame, twelve calls, four address pairs and five reviewed internal J targets. Preserve zero/one/record returns, halfword countdowns, inherited uninitialized s5 keyframe index, two-word vertex copies, saved extra-vector words and full restore tail. |

No strings or candidate outgoing references occur in these four dossiers.
Every return and call retains its architectural delay slot. The SDK header
supplies SVECTOR, VECTOR and CVECTOR and the genuine projection/morph pointer
contracts. `gteMIMefunc` remains the separately attributed 128-byte MSC body;
no SDK source, compiler profile or ownership inventory changes are planned.

First compare the three exact projection writers and partial binder under the
existing complete-owner declaration in temporary source copies. Keep typed
local cursors over the explicitly unresolved byte interval; morph begins at
its independently decoded +1f40 byte offset. This tests address formation,
not a production array capacity or finished scratch representation. Preserve
every operation and compare complete linked words, ordered calls/referents
and restore tails, with a shifted-owner negative control.

Separately, the binder's first canonical divergence is allocation retry:
retail `800206a8` jumps to the count reload at `8002068c` (+b8), while source
jumps to the following allocation call (+bc) and retains the count in s0.
Test spelling that actual retry target as a C label followed by allocation
and a failure-only release/backedge. This is a control-flow-source hypothesis,
not a fake reload, volatile parameter or forced register. Keep the owner
experiment separate from this algorithm spelling. Compare the first raw
divergence after each focused build, preserve all six exact pool siblings,
and bank only strict 100% after full verification.

## First controls and next source hypothesis

The three exact projection writers retain every linked retail word with the
same opaque-span owner view, including the signed shift and all delay slots.
The binder's owner-only copy is still 932 bytes and differs at the same
fourteen words as canonical source: three retry/count words, followed by
eleven metadata/scratch-copy setup and register words. All twelve calls and
four materialized addresses agree. Changing the owner root by four breaks
each raw comparison without changing calls; no capacity claim is introduced.

The explicit retry label restores the retail count reload and backedge and
raises the canonical binder to **99.141630%**. Its six exact pool siblings
remain exact. Next, retail starts the common scratch block by reloading the
clip parameter, then stores the keyframe index in that load's delay window.
Canonical source currently publishes the keyframe before reading/publishing
the clip, yielding the opposite load/store setup. Test publishing clip before
keyframe in C while preserving both halfword widths, values and the following
vertex-copy operation. Compare the resulting instruction schedule and the
complete body; do not change helper argument order or permute locals.

## Exact binder verdict

Publishing clip before keyframe restores the remaining eleven retail words,
including the early clip reload, keyframe store in its load-delay window and
the scratch copy's count/destination registers. Together with the explicit
retry label, canonical `render_bind_animated_instance` is strict **100%**:
all 932 bytes, twelve calls, four address pairs and five internal jumps agree.
The complete 72-byte frame and twelve-word restore/return tail also agree.
No helper signature, parameter width, local declaration order, inherited
keyframe initialization, call set, relocation target or compiler flag changed.

The corrected binder is also byte-exact with the temporary complete owner,
alongside all three exact projection writers. The durable control recompiles
both canonical and owner variants and compares every linked word. A shifted
owner root breaks the raw comparison; a retry jump shifted from the reload
to the allocation call is rejected even though the call set is unchanged.
The temporary registry and scratch casts remain in the probe only: no typed
array capacity, production DATA owner or alias declaration is introduced.

Only the binder changes among all 484 native report rows. GAME advances
**288 -> 289 / 362 exact**, OPEN remains 97/108 and PSX 1/1; overall is
387/471. The six pool lifecycle siblings remain exact, and only
`GAME.EXE:800205d4` is selected for banking. This closes the function through
decoded control flow and publication order, not a compiler-wall attribution.

## Verification

All 25 ordered source/target relocation records agree after normalizing only
intra-function section-label spelling; distinct callees, data identities and
REL addends are preserved. Both focused builds and the recorded match report
7/7 exact in `game.pool`. The canonical and temporary-owner raw controls
agree on every instruction of all four selected functions.

Ruff, all 642 repository tests (77.352s, no skips), eight focused graphics
controls and `git diff --check` pass. The full, freshly compiled animation
oracle passes all 1136 retail/C/Rust cases, including multiple allocation
retries and reinitialization. Retail and compiled code each execute 7,394,785
instructions across that corpus; the explicit SDK boundaries remain unchanged.
This finite behavioral check supplements, rather than replaces, raw exactness.

Full `kf build -j4`, repeated after the oracle rebuild, retains the existing
data/ownership/placement failures: source data 5/60, config-backed SDK data
4/4, target relink 110/116, six section-base conflicts and zero artifact
failures. No new data failure, compiler setting, SDK source, shared type or
production owner claim belongs to this change.
