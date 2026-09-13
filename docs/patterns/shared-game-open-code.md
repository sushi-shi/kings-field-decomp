# Shared GAME and OPEN implementations

The [complete accounting](../shared-function-accounting.tsv) covers all **470
reconstructed game-function occurrences** in GAME (362) and OPEN (108), against
master `6f96b76874dd`. Each row records the image and address, original source,
current implementation, counterpart where applicable, and final disposition.
PSX's entry point and identified SDK/library functions are outside this comparison.

| Disposition | Implementations or pairs | Image/function occurrences |
| --- | ---: | ---: |
| Existing shared same-name pairs | 10 | 20 |
| Newly consolidated same-name pairs | 44 | 88 |
| Shared word-copy body with different entry names/types | 1 | 2 |
| Reusable helpers used by only one image | 9 | 9 |
| Same-name pairs with distinct behavior | 15 | 30 |
| Differently named related implementations retained | 11 | 22 |
| Other image-specific implementations | — | 299 |
| Total | | 470 |

There are now **64 shared implementations**, covering 119 image/function
occurrences. This includes five GAME-only angle/vector helpers and four OPEN-only
utilities: CVECTOR interpolation, matrix diagnostics, flat-color quads, and
right-shifted perspective projection. A reusable function need not survive in
both linked programs to belong in shared code. Conversely, this organization
does not prove the original source file boundaries or explain why a function is
absent from an image.

## Source organization

The allocator was already compiled for both images from `src/game/memory.c`;
it now lives in `src/shared/memory.c`. The complete GAME math unit also moves
to `src/shared/matrix_rotation.c`, keeping its GAME-only functions with their
family. Existing matrix interpolation remains in `src/shared/matrix_lerp.c`.

The other shared bodies use literal `.inc` implementation includes at their
original positions. This preserves the original compilation units, function
order, pooled strings, globals, compiler profiles, and link membership. Splitting
these into new linked modules would change those properties. Each shared function
retains its image-qualified `ADDRESS_AT` claims; the manifest still enforces a
contiguous function run per unit. DATA and RODATA ownership stays in the including
unit. The scanner rejects storage claims in fragments and records the fragment's
actual file and line in `build/gen/bindings.tsv`.

`include/kf/shared_graphics.h` selects the corresponding GAME or OPEN state
members and color-preset type. It adds no runtime dispatch, pointer indirection,
casts, or replacement structures. The display frame code also retains the
different unidentified per-image counters explicitly. Source sharing does not
merge either image's storage or function identity.

`map_resource_copy_words` and `resource_stream_copy_words` retain their curated
names and respectively unsigned/signed word counts. Their decrement-until-zero
loop has one implementation with image-selected declarations. This avoids
changing the interface merely to make two definitions look identical.

The clean C and C++ generators process fragments with the same claim/comment
and type transformations as ordinary source. Exported fragments contain no
matching claims. Build dependency scanning and progress hashes include their
transitive contents. Cast, Boolean, parameter, and cleanliness accounting also
include fragments, so moving code cannot hide audit sites.

## Differences retained

The 15 same-name pairs left in the image directories are deliberate:

| Functions | Difference |
| --- | --- |
| `main` | Initialization, entry contract, and next-overlay result |
| `cd_file_load_allocated`, `cd_file_load_into` | Error-screen, loaded-status, retry, and return handling |
| `render_initialize`, `display_initialize` | Buffer/table initialization and OPEN ending-mode display behavior |
| `tmd_project_vertices` | OPEN capacity diagnostic and runtime shift versus GAME's fixed shift |
| `render_enqueue_tmd`, `render_enqueue_map`, `render_enqueue_sprite` | Primitive allocation, failure handling, and emission paths |
| `render_map_cell` | GAME illusion-staff remapping and different visibility parameter promotions |
| `audio_initialize`, `audio_load_vab`, `audio_close_vab` | Volume/reverb/lifecycle differences, including OPEN's settling delay and bank check |
| `audio_play_spatial`, `audio_play_voice` | Spatialization and GAME's player audio-effects gate |

The whole-source comparison also checked differently named candidates. Camera
paths have different start/advance semantics; map windows, entity pools, resource
loaders, fades, and sequence lifecycles belong to their respective scene/gameplay
owners. The TSV records these analogues in both directions. Common camera-path
advance/publish operations already have shared inline definitions in `game_map.h`.
OPEN's gradient quad fixes the ending scene's ordering depth, and its checked
primitive allocator retains the diagnostic loop and allocation counter. These
remain with their existing owners.

## Evidence and verification

The audit compared every OPEN definition with all GAME definitions, including
token-normalized bodies and differently named nearest candidates. The selected
families were checked against image-specific disassembly/CFG, calls, data/string
references, current matches, and source history. Names or equal virtual addresses
were not treated as evidence of equivalent behavior. SDK calls remain library
boundaries; no vendored body was reconstructed or counted as new game work.

Verification compares pre-refactor and rebuilt whole ELF objects, including raw
instructions, symbol tables, data, and ordered relocations, then compares native
CPE linker outputs. All **100 GAME/OPEN objects and three CPE outputs are
byte-identical**. No gameplay correction or new retail-exact claim is part of
this refactor. The existing data-owner/section-placement failures reported by
`kf analyze` remain separate from executable build success.
All 101 units retain their function, DATA, and RODATA claims. Strict function
scores remain **458/471 exact**, including PSX; this campaign adds no matched
functions and regresses none.

The C export is independently compared against the corresponding unstripped
historical-compiler build; the C++ export is compiled and linked for PS1 and its
codec library is built. Neither generated tree gains tests. Runtime interaction
was not exercised by this static source-sharing campaign.
