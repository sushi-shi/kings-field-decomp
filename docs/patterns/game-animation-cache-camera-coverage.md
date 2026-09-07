# Animation-cache and camera literal coverage

## Review scope

At `6c85b0c`, reconcile every current literal in GAME `asset_registry.c`,
`pool.c`, `camera_path.c` and OPEN `camera_path.c`. Earlier domain notes
describe these families but do not supply a current token/expression ledger.
Capture all sixteen functions with image-qualified disassembly, CFG, callers,
callees, strings, current matches and history. Check the shipped animation
headers independently before deciding whether more semantic names are justified.

The registry and cache operate on game model archives and per-instance vertex
buffers. Camera functions consume game path points and player/opening state.
Their SDK TMD/GTE, allocation and square-root callees stay external. This
review changes documentation only; source types, constants and behavior remain
unchanged. Force the four units, compare their function words and references,
and run the required inventory, lint, repository tests and full build.

## Serialized reversal and unresolved clip metadata

The shipped size-prefixed model assets contain 813 keyframe references. Every
referenced keyframe's unsigned halfword at +0 is zero. The binder loads that
halfword and, for any nonzero value, replaces the forward Q12 fraction with
4096 minus that fraction. This proves the Boolean reversal predicate. It does
not identify a particular nonzero value as an authored enum member. Retain
the existing `reverse != 0` form and document zero as the ordinary Boolean
boundary; do not invent a separate stored reversal state.

This field is distinct from the clip header's +2 halfword. The latter has no
established consuming read. Its repeated value 20 still does not prove a frame
rate, duration, format code or padding role. The remaining `unknown_02` spelling
therefore remains unresolved. Per-resource hashes, clip/keyframe indices and
raw reversal/duration values are retained in the ignored evidence report.

## Retained contracts

The cache's free/stale/live values are already enum-typed. `pool_allocate`
finds a free record and invalidates its clip without marking it live; successful
binding performs that transition. Null pointers, loop exhaustion and accumulated
phase zero are separate from that lifecycle. The signed release-all loop starts
at capacity minus one and ends at minus one after visiting twelve records.

The three trailing `[1]` arrays describe variable serialized tails whose actual
counts come from their headers. They are not one-entry resource limits. TMD
object zero is the selected subobject, not a named model asset. The two words
saved around the rest-morph blend are the X/Y and Z/alignment halves of one SDK
vector. The blend temporarily includes that leading vector, restores it, and
publishes scratch vertices beginning at the next vector. Keep these ordinary
component indices and the extra-vector arithmetic explicit.

GAME begins a camera path at the player's current camera transform and selects
point zero as its first target, incrementing the index after selection. OPEN
begins at serialized point zero and increments before selecting the first target,
so its first segment goes toward point one. Both retain Q4 interpolation and
their named position/finished-frame sentinels. The remaining zeros select the
first point or initialize the ordinary point index; they are not path states.

The [complete ledger](game-animation-cache-camera-literal-ledger.md) gives a
reason for every retained occurrence, including duplicates on the same line.

## Verification and function verdicts

The four forced unit compiles preserve all sixteen strict-exact functions.
Their 1,070 complete retail words, 31 ordered direct calls and 71 address
materializations agree with both the captured candidate and delinked target,
including delay slots. Source snapshots and runtime objects are unchanged.
The reversal load is GAME `80020774`, the zero branch is `8002077c`, and the
complement is in the jump delay slot at `80020788`; these distinguish this
keyframe field from the unconsumed clip-header halfword. No function is newly
reconstructed or banked.

All 40 retained tokens have current expression-level reasons: registry one,
cache 35, GAME camera one and OPEN camera three. The source-wide census remains
6,589 occurrences, with complete ledgers now covering 46 files and 4,097
occurrences. The ten source lines with fourteen unresolved identifier tokens
remain outstanding.

| Image | VA / bytes | Function | Before / after | Verdict |
| --- | --- | --- | --- | --- |
| GAME.EXE | `0x800204c0 / 156` | `asset_registry_load_tmd_archive` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x8002055c / 64` | `asset_registry_set` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x8002059c / 56` | `asset_registry_select` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x800205d4 / 932` | `render_bind_animated_instance` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x80020978 / 48` | `pool_reset` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x800209a8 / 60` | `pool_mark_allocated` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x800209e4 / 72` | `pool_record_release` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x80020a2c / 108` | `pool_release_all` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x80020a98 / 108` | `pool_release_stale` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x80020b04 / 72` | `pool_allocate` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x800332e4 / 732` | `camera_path_compute_segment` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x800335c0 / 192` | `camera_path_begin` | 100% / 100% | Exact, unchanged |
| GAME.EXE | `0x80033680 / 284` | `camera_path_step` | 100% / 100% | Exact, unchanged |
| OPEN.EXE | `0x80013cf4 / 784` | `opening_camera_path_compute_segment` | 100% / 100% | Exact, unchanged |
| OPEN.EXE | `0x80014004 / 252` | `opening_camera_path_begin` | 100% / 100% | Exact, unchanged |
| OPEN.EXE | `0x80014100 / 360` | `opening_camera_path_step` | 100% / 100% | Exact, unchanged |

Inventory, Ruff and whitespace checks pass. The whole-tree modern check retains
300 errors, with 65/112 variants passing. The full repository run exercised 683
tests in 90.040 seconds: two OPEN ending/transition snapshot checks failed
against concurrently changed shared objects. All eight tests in those two
unchanged test modules pass against an isolated fresh compile of the current
source and copied target objects. The source and header snapshots were checked
against the current tree; no test assertions or shared artifacts were changed.

Full `kf build` retains existing source-data and target-relink failures:
source-data PSX 0/1, GAME 9/42, OPEN 2/19; target relink PSX 1/1, GAME 75/77,
OPEN 34/38. It reports six conflicting section bases and zero artifact failures.
No production code, tests or size assertions were added or changed.
