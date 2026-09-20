# Semantic constants in the source port

This cleanup names executable decisions and format fields without changing their
values, arithmetic, rounding points, branch conditions, or update order. It is
based on `port`, independently of the cast-cleanup worktree.

## Ownership

- GAME owns floor-specific actor/event identities, dialogue exchanges, trigger
  regions, warp connections, sound cues, spawn positions, and menu labels.
  A floor-local definition number is not a global monster or character ID.
- Resource loaders name STAT template fields, floor-item placement fields, TMD
  fields, sound resource chunk lengths, and resource-path components at their
  decoding boundary. TMD offsets come from its fixed-width file-record types.
- The renderer owns texture-page/palette encoding, color conversion scales,
  texture transparency markers, projection limits, and lighting arithmetic.
  Shader constants describe the same uniform encodings as the native caller.
- Platform code owns timing units, input thresholds, extraction limits, ISO/CD
  record fields, path capacities, and storage result codes.
- The audio mixer and codec bridge name MIDI/ADPCM/ADSR encodings and scales.
  `kf_codec::audio::midi` and `kf_codec::tim` own parser constants reused by the bridge.
  The bridge's `MusicEventKind` values mirror `KfMusicEventKind` in the C header;
  browser save-result values mirror `kf::SaveFileResult`.

Equal values are not necessarily the same quantity. PCM normalization and
envelope normalization remain separate; SHA-256's byte offset for its length
field is not used as a bit-shift name. Texture alpha values classify transparent,
semitransparent, and opaque texels; they are not ordinary material opacity.

## Preserved audio quirk

GAME `audio_play_spatial` reads `SoundRef` byte one as a tone selector plus a
high-bit control. Its low nibble reaches voice selection; the high-bit branch
would increase the gain used to narrow stereo panning, not the voice volume.
The field is therefore named `tone_and_flags`, with a tone-index mask and a
pan-narrowing flag. The serialized record remains three bytes.

The Japanese retail GAME instructions explicitly compare `(byte & 0x80)` to
`1`, an impossible condition:

- `GAME:0x80032e58`: load comparison value `1`.
- `GAME:0x80032e64`: load sound-reference byte one.
- `GAME:0x80032e6c`: mask with `0x80`.
- `GAME:0x80032e70`: branch around the gain adjustment if unequal.
- `GAME:0x80032e78`: add `36` on the unreachable path.
- `GAME:0x80032f8c`: mask the voice's tone argument with `0x0f`.

This was checked against hash-validated retail data with image-specific semantic
queries. Naming the flag describes the guarded calculation; it does not prove
the original author's intended source spelling. The comparison remains `== 1`.
Changing it to a nonzero test would be a behavior change, not warning cleanup.

OPEN's spatial function is different: it passes the selector unchanged and its
near/equal-pan path uses distance gain directly. This cleanup retains both facts.

The effect constructor's `id` also requires care: effect updates pass that value
to `player_apply_damage` as `multiplier_tenths`. The three weapon-magic calls that
passed `10` now use the existing `KF_PLAYER_DAMAGE_MULTIPLIER_ONE`; they do not
introduce a fictional actor-owner identity.

## Deliberately retained literals and unknowns

Ordinary zero/one arithmetic, signs, decimal digit conversion, byte-wise endian
packing, vector components, and pixel placement remain literal where clear.
Independent layout assertions keep their numeric expected sizes. Authored data
such as sine/root lookup tables, light directions, sound program/note tuples,
camera paths, dither matrices, and named Japanese atlas sequences retain their
values as data; each entry does not get a separate alias.

Menu callers still perform the original partial-row writes. Replacing a whole
row could change glyphs deliberately retained from the preceding label. For
example, the MP label reuses the HP label's second glyph, and total defense
reuses total attack's prefix.

Unresolved map attributes/operations, unknown storage extents, and the initial
moonlight control bytes remain explicitly unresolved rather than receiving
invented semantic identities. Naming does not establish those meanings. The
historical, uncalled Rust gameplay/runtime models are not treated as a second
game implementation to maintain; the codec pass targets the parser/bridge paths
used by the actual application.

## Verification

Focused and holistic reviews checked values, signedness, scope, partial glyph
updates, and boundary encodings. Findings about shared identities, misleading
units, and remaining linked codec/browser literals were applied.

Linux and WASM application builds and the native package/launcher flake checks
passed. Existing gameplay switch/enum/initialization warnings remain a separate
cleanup; the impossible retail panning comparison is documented, not suppressed.
No unit-test campaign, automated gameplay, or new retail/runtime comparison was
introduced. Building C++/WASM does not compile or exercise embedded GLSL; its
changes were reviewed statically, not claimed as a new visual check.
