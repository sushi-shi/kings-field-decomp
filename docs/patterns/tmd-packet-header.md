# GAME TMD packet-header investigation

## Function Match Plan

This continuation starts at clean `c35fdfc`; the full objective remains all
29 GAME parsers/serializers at strict 100%, currently 23. The previous turn
made concrete progress by recovering the shared map runtime owner and one
exact auxiliary consumer. This pass tests a different outstanding source
type question, not a repeat of the rejected per-mode inline helpers.

Target: GAME `tmd_prepare_primitive_indices`, `8001c2b0`, `300` bytes,
strict 98.333336%. Retail is revalidated with `kf init`. The image-qualified
address, complete disassembly/CFG, xrefs/callees, strings and match state are
read again, together with all three callers (`tmd_register`, archive load,
registry set), neighboring view setup/register routines and source history.
The function takes no arguments, returns void and makes no calls. Its eight
mode bodies shift the proven halfword vertex/normal fields by three. Both
disk counts narrow to halfwords; loop tests mask to sixteen bits. It has
three current-TMD address pairs, a 29-row switch table, seven reviewed
internal jumps and an indirect dispatch whose table entries are not direct
call evidence. No strings occur.

Retail loads the packet length byte at `8001c2fc`, then the complete header
word at `8001c300`, and shifts that word by 24 before masking the mode with
`fd`. It reserves eight stack bytes at entry and restores them in the return
delay slot, without reading or writing the frame. The existing scalar header
reconstruction has no frame; its primitive-loop entry also schedules the
packet addition rather than the countdown addition in the branch delay slot.
All packet bodies and ordered referents otherwise agree.

The vendor negative control is the separately identified Sony LIBGPU TMD
archive member: `OpenTMD`, `ReadTMD`, `get_tmd_addr` and `unpack_packet` live
at their independent GAME addresses. This game-specific index-to-eight-byte-
offset pass does not match those SDK operations or their call topology. The
pinned SDK include search supplies no `olen`/`ilen` packet-header typedef.
The existing packet format model and direct byte/word accesses support a
project fixed-width header view; no vendor signature is invented.

First hypothesis: model the complete four-byte packet header as a word/byte
union, preserving the independent source length-byte read and mode-word
load. Compare a real header-value decode with the existing scalar decode.
This is a bounded type experiment, not permission to add an unused local,
artificial stack padding, volatile storage or forced assembly. A header
view must represent the actual `olen`, `ilen`, flags and mode bytes and be
used for decoding. Whether it explains the eight-byte frame is unproven.
Do not retain a redundant wrapper merely because it adds stack space.

Rebuild and inspect raw instructions/ordered relocations after each focused
change, then use strict objdiff. Any retained result requires layout controls,
full retail/C/Rust TMD comparison, regression checks, repository tests/lint,
full build and banking only verified 100%. A non-exact result gets its actual
residual instructions recorded without attributing a compiler mechanism.

## Header experiment results

The word/byte union and whole-header union copy emit exactly the existing
scalar candidate. An inline mode helper taking the header by value also
emits the existing candidate. A pointer-taking helper does produce the
eight-byte frame, but keeps a header store and byte reload in it. Using
the directly evidenced word shift in that helper removes the byte reload,
but keeps the stack store and changes the header/length scheduling. Neither
pointer helper matches retail, which never touches its frame. All header
type/helper trials are removed; no artificial frame or unused variable is
kept. The experiment narrows the helper hypothesis but establishes no
historical compiler mechanism.

The pointer/word trial uses the following complete header view and helper
under the unchanged `game.render` profile (`kf try --unit game.render`):

```c
typedef union KfTmdPacketHeader {
    u32 word;
    struct { u8 olen, ilen, flags, mode; } bytes;
} KfTmdPacketHeader;

static inline u8 tmd_packet_mode(const KfTmdPacketHeader *header)
{
    return (header->word >> 24) & 0xfd;
}
```

In the existing function, replace `u32 word` with that header value, assign
`header = *(KfTmdPacketHeader *)packet` where the word was loaded, and switch
on `tmd_packet_mode(&header)`. Leave the independent `packet[1]` length read
and every other operation unchanged. The generated header sequence includes
`sw v1,0(sp)`; retail has no such store. The byte-helper control returns
`header->bytes.mode & 0xfd` and additionally reloads `lbu v0,3(sp)`. The
by-value control instead takes `KfTmdPacketHeader header`, reads `header.word`
and passes the value; it reproduces the original frame-free candidate.

## Item-count setup follow-up plan

The remaining small source question is GAME `item_load_floor_placements`,
`80020b4c`, `1b0` bytes, strict 98.888885%. The image-qualified evidence
pass, caller (`map_resources_load` at `8001b664`), adjacent functions and
source history are refreshed before editing. One O32 placement-pointer
argument is read and no return value is consumed. There is one `rand` call,
three validated global address pairs, no strings, a 40-byte frame and the
restore in the return delay slot. Twelve-byte input records, 24-byte output
records, unsigned ID/count and tile bytes, signed local halfwords, and the
2000/100 scale factors are fixed by the complete body. The custom two-pass
placement expansion is not a vendored SDK body.

Retail clears the global halfword count through `a0` before saving the
input start and then transfers its address to `v1` for the nonempty count
loop. The current C initializes the count pointer before clearing through
it. Test the independently visible global reset followed by the count-loop
pointer setup, keeping every increment and the two source walks unchanged.
Unlike the earlier all-global counter trial, only the reset is a direct
global access; the loop still uses its real count pointer. This must not
add duplicate assignments, fake carriers or alter the input/output contract.

## Item result and ownership boundary

The direct reset followed by a loop-local count pointer materializes the
count address twice. It still does not emit the retail `move v1,a0`, so the
change is removed. This differs from the previous all-global experiment,
but reaches the same negative conclusion about independent address lowering.

The owner check does find the already documented wider graphics relationship:
the floor traversal loads the active texture-page address `8009505a` and
derives the first item `80095098` by adding 62. This is not a new proof that
count plus pool alone is the complete object. The intervening span includes
material fields, notification globals and texture selectors. The existing
[material investigation](render-material.md) and
[notification investigation](notification-state.md) show that narrower and
wider aggregate trials lose banked exact consumers. No new complete-extent
or field evidence resolves that conflict here; no overlapping floor-item
owner or artificial six-byte prefix is introduced.

## Final verdicts

| GAME function | Strict score | Result |
| --- | ---: | --- |
| `tmd_prepare_primitive_indices` | 98.333336% | Unchanged. Value header forms emit the existing scalar candidate. Pointer helpers add real stack traffic, unlike retail's unused frame. All trials removed. |
| `item_load_floor_placements` | 98.888885% | Unchanged. Independent count reset adds an address pair and does not explain the count-pointer transfer. Trial removed. |

No C source, shared type, inventory, compiler option or Rust implementation
change is retained. Nothing is newly banked. The parser census remains
23/29 strict-exact with 99.493564% size-weighted similarity. These experiments
exclude specific proposed source shapes; they do not prove that 100% is
impossible, identify the original compiler, or prove an optimizer limitation.

## Verification

After removing the trials, the C sources, headers and configuration are
byte-identical to `c35fdfc`. Both affected candidates are explicitly rebuilt,
then a full reconfigured `kf build` and `kf check` pass. All 60 data-owning
units match, and there are no lost banked functions. Fresh focused matches
confirm the two scores above. Python passes 363 tests and 229 subtests;
`ruff check scripts tests` and whitespace checks pass.

The complete TMD oracle is rerun on the restored candidate: all 250 payloads
(246 shipped, four synthetic) agree between retail MIPS, reconstructed C
MIPS and Rust. The shipped corpus contains 1,911 objects and 117,119 packets.
This pass does not rerun the other nine full suites or change their boundaries;
their previous all-suite result belongs to the unchanged `c35fdfc` code.

As before, PS1 function bodies run on Linux in isolated Unicorn little-endian
MIPS memory, with candidate relocations applied and identical seeded inputs.
The TMD test compares complete mutated payload bytes against retail and the
native Rust driver, not just decoded counts. It is neither a PS1 boot nor a
proof of arbitrary-input or hardware-timing equivalence.
