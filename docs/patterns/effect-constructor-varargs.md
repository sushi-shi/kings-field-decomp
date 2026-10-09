# Effect constructor argument access

`GAME.EXE:80036f44`, `effect_pool_construct` (2092 bytes), remains a manual
argument walker. The cleanup checklist's previous zero count omitted this
function. Its 100% result does not establish that the original developers
spelled the argument accesses as raw pointer arithmetic.

## Retail and caller evidence

The image-qualified disassembly, CFG, 29 proven incoming call sites, outgoing
calls, references, strings, adjacent pool functions and source history were
reviewed on `01e309d0`. Retail uses a 56-byte frame. The delay slot of the
free-slot call at `80036f74` sets `s1 = sp + 72`, the fifth argument's home.
The common initialization separately loads that fifth argument, a direction
pointer, at `80036fb0`. Optional values then use offsets 4, 8 and 12 from
`s1`. These offsets select argument positions; they are not object fields.

The existing GAME signature has five fixed arguments: id, type, kind,
position pointer, direction pointer. Caller macros in `game/effect.h` group
the optional tail by kind for modern checking and expand to ordinary arguments
for the target compiler. The optional contract is:

| Kind family | Arguments after direction | Retail consumption |
| --- | --- | --- |
| Wind Cutter | Duration, sound request | Ignores duration; reads sound as a word |
| Lightning Bolt and its alternate | Duration, sound request | Low duration halfword, full sound word |
| Ground branch | Branch role | Low halfword |
| Actor spawner | Duration | Low halfword |
| Scatter projectile | Generations, duration, scale | Three low halfwords |
| Map emitter, map switch, swinging hazards, Moonlight | Rotation pointer | Pointer to an SDK `SVECTOR` |
| Light Needle and physical projectile | Rotation pointer, sound request | Pointer and full sound word |
| Ground trail | Parent effect index | Low byte |
| Radial blast and alternate | Sound request | Full word |
| Homing projectile and alternate | Rotation pointer, target mode, sound request | Pointer, low byte, full word |

Other kind arms consume no optional values. The integer arguments undergo
default promotion; a standard API reconstruction reads `int` and narrows to
`u8` or `u16` at the consumer, rather than requesting an unpromoted byte or
halfword from `va_arg`. Wind Cutter must advance past its unused duration
argument before reading the sound request. This skipped argument is visible
in the shared caller protocol, not a padding value invented for compilation.

There are seven direct call sites: one pool lookup and six sound calls. The
45-entry dispatch table, game-owned effect records, magic data and player
state establish game-specific behavior. The constructor has no vendored
identity or string references; its external audio routines remain SDK/game
API calls according to their existing attribution.

## Native compiler controls

All controls use `probe-gcc257-o2-g0`, native ASPSX 1.07 and the same safe
retail target. Candidate sources and native objects are saved under
`build/effect-varargs-audit/`; comparison uses ordinary objdiff, not `--loose`.
The table's sizes are read from the candidate object, not the target's
function metadata in the comparison report.

| Source control | Candidate bytes | Strict objdiff score |
| --- | ---: | ---: |
| Unchanged raw-access source | 2092 | 100% |
| Five fixed arguments; `va_start` after the pool lookup | 2084 | 97.587000% |
| Five fixed arguments; `va_start` before the pool lookup | 2096 | 98.868070% |
| Same before-call source; preserved Psy-Q 4.5 header | 2116 | 97.912050% |
| Four fixed arguments; direction read through `va_arg` | 2088 | 96.613770% |

The five-fixed-argument before-call candidate is the closest standard-API
control. Its first real instruction difference is at constructor +0x24:
it initializes `s1 = sp + 76`, pointing to the first optional argument.
Retail initializes `s1 = sp + 72` later, in the call delay slot. Subsequent
loads use corresponding displaced offsets, and scatter/homing arms retain
cursor increments absent from retail. Narrow assignments still select the
observed `lhu`/`lbu` widths. The known CFG successor lists agree by block order;
this does not itself prove semantics for the indirect dispatch.

Starting the cursor after the lookup reduces the saved-register frame to
48 bytes. Treating direction as the first variadic argument additionally
spills the fourth register argument, which retail does not do. That control
does not support changing the current five-fixed-argument signature.

The later donor header is preserved in
[rood-reverse](https://github.com/ser-pounce/rood-reverse/blob/7c8c221cd5064515bee691f4854648f31268cf2b/include/psx/stdarg.h).
It identifies itself as Run-time Library Release 4.5 and advances by rounded
word size before reading at the previous cursor. It is evidence of an SDK
macro form, not proof of the missing King's Field-era header. The supplied
Release 2.5 media omit `STDARG.H`; the project's current header is likewise
a reconstruction whose exact menu/notification/formatter controls do not
prove its suitability for this constructor.

## Preserved Runtime Library 3.0 control

A follow-up review on cleanup input `f773175a` checked three byte-identical
local extractions labelled Runtime Library 3.0: the PSX 3.0 include tree and
the ISO/LZH runtime include trees. The original CRLF header has SHA-256
`43d39ef8cd688afd32994e98ae28cd5f8e956695e4cb1cd193d365b135a566e9`.
The directly inspected `psx-3.0-1995-04-08.zip` container has SHA-256
`416241637cdb0273b6bd936b0ab5627c8ba7225ee2b98066cf3b50fb09e57899`;
its `psx/INCLUDE/STDARG.H` entry matches that header hash. The ZIP entry's
1995-04-08 timestamp is archive metadata, not proof of its original release
date or applicability to King's Field's earlier build. The other extracted
copies corroborate the bytes; their extraction paths alone do not establish
independent provenance.

Its `va_list` is `char *`, while the pinned later 4.5 donor uses `void *`.
Their rounded-size, start, advance/read, and end macro tokens agree. Both
start after the last named argument. Neither is a witness for a cursor
starting on the direction argument itself. Only CRLF-to-LF normalization was
needed for the Unix probe preprocessor; macro tokens were preserved.

The isolated five-fixed-argument control calls `va_start` before the free-slot
lookup, consumes promoted integers before narrowing, reads rotation pointers
with their pointer type, skips Wind Cutter's duration, and calls `va_end`.
It uses the current production constructor body with these argument-access
changes, not the old investigation's superseded declarations. The actual
profile is GCC 2.5.7, `-O2 -G0 -mcpu=r2000`, with native ASPSX 1.07.

| Control | Candidate bytes | Ordinary objdiff | First raw difference |
| --- | ---: | ---: | --- |
| Preserved 3.0 header | 2116 | 97.912050% | +0x24: retail `move s5,a0`; candidate `addiu s1,sp,76` |

The candidate retains a 56-byte frame, but initializes the optional cursor
early. Retail sets `s1 = sp + 72` at +0x34 in the lookup call's delay slot.
The first differing words are `0080a821` and `27b1004c`, respectively.
The known CFG block counts agree (52/52); the dispatch's unresolved indirect
jump still prevents a full reachability/equivalence claim. This later header
control reproduces the existing 4.5 control's size and percentage. It supplies
additional header evidence, not a new exact replacement or compiler attribution.

Fresh image-qualified constructor and caller evidence confirms the production
constructor remains strict 100%. The native control object, compile metadata,
header/container hashes, and raw verdict are retained locally under
`build/cleanup-evidence/effect-arguments/sdk30/`; none are production or banked
inputs. This documentation update leaves all executable inputs unchanged.

## Verdict and cleanup accounting

The optional argument values and widths are supported. Original cursor/API
spelling remains unresolved. The four production pool functions retain their
existing bodies; no source-only indexed `va_arg` macro, wrong `va_start`
parameter, fake local or signature change is retained to force agreement.
The standard-API candidate is an experiment, not a banked replacement.

`void* views = 14` counts every `void *` spelling in C sources, including
appropriate allocators and generic file buffers. `byte-array views = 4`
counts only the narrow `(u8 *)`-followed-by-`&`-or-`(` regex: three constructor
argument reads and one complete item-stock clearing loop. It misses the
constructor's analogous halfword/pointer accesses and is not a count of
unknown data owners. Neither metric establishes a goal of zero by itself.
