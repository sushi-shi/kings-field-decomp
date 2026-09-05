# Explicit section extents versus assembler-added tails

## Evidence and campaign plan

At `d3f7b21`, strict source data is 6/59. The independent target relink rejects
six conflicting section placements. Many other comparisons encounter source
section extents rounded to sixteen bytes, even where the retail claim ends
earlier. Do not enlarge targets, crop object files, weaken alignment checks or
change C types to accommodate those tails.

This is an assembler-input contract campaign, not function reconstruction or
historical assembler attribution. Preserve all C sources, claims, profiles,
named symbols, raw function bytes and ordered relocations. First isolate GNU
`as` padding from explicit assembly directives; check the SDK record model;
then run the real compiler pipeline and compare every function with the
pre-campaign snapshot. Keep any newly exposed data mismatches.

## Independent controls

The pinned GNU MIPS assembler documents `-no-pad-sections` as disabling padding
at section ends. Given an eight-byte `jr ra`/delay-slot body, one initialized
word and four bytes of `.space` BSS, the default assembler produces sixteen
bytes in each of `.text`, `.data` and `.bss`. The option produces extents 8, 4
and 4. **All three still require sixteen-byte ELF alignment.** Existing bytes
and instruction encodings are unchanged; extent and alignment are independent.

The complete Release 2.5 `LIBSPU.LIB/S_N2P.OBJ` data record contains 386 bytes
with section alignment eight, ending in the halfword `0x2000`. Its SHA-256 is
`e1b18023a561e14ae4871a7a27eb867455d5424d0a498ba42abc0cabbcf222df`.
The whole-section parser checks every record and rejects unsupported patches or
reservations; this is not a selected symbol slice. A 386-byte section aligned
to eight directly disproves mandatory rounding of the payload to its alignment.
`S_SRMP.OBJ` independently has 760 data bytes aligned to eight, not sixteen;
its SHA-256 is `809188e34e6447ec50aeb16a0dde8cf2d3bcf998d29e481fb129e49ce3bdf9a5`.
Neither object is claimed as newly matched game data by this campaign.

The new compiler smoke control fails against the old pipeline: a nine-byte
initialized array becomes a sixteen-byte `.data` section. Both GCC probes pass
after the change, retaining the complete nine-byte array, including its explicit
zero tail, and the five-byte read-only array. GCC 2.5.7's explicit sixteen-byte
COMMON allocation for a nine-byte tentative object remains sixteen; GCC 2.6.0's
nine-byte allocation remains nine. Compiler allocation rounding is not a GAS tail.

A separate diagnostic assembly control retains an explicitly emitted final
NOP after the return delay slot, seven interior `.balign 8` padding bytes,
explicit final zero words/bytes, a nine-byte BSS reservation, and the complete
`R_MIPS_32 external + 4` relocation. It checks the unchanged alignments as well
as bytes and extents. This assembly is a synthetic toolchain test, not a game
function or a means to bank code.

## Kept pipeline change

`compile_source` passes `-no-pad-sections` to GNU `as`, directly for assembly
and through maspsx's assembler-argument forwarding for C. Every object metadata
file records `gnu_as_section_flags`. No post-assembly bytes or ELF alignments
are rewritten, and no claim participates in selecting a section's end. Source
directives alone determine its explicit extent. The compiler, optimization,
scheduling model, G0 setting and maspsx instruction-expansion profile are unchanged.

This does not establish historical ASPSX identity; the supplied executable is
key-protected. Nor does it assert that all existing RODATA claims exclude
linker padding. Those remain independently reviewed retail models. In particular,
the default data gate must reject a larger target claim even if the extra bytes
are zeros: it cannot silently crop either side to their common prefix.

## Full-corpus result

All 112 source units are rebuilt. Ninety-two objects change, solely through
end-of-section extents and the consequent physical ELF offsets. The removed
automatic tails total 704 `.text` bytes, 261 `.data`, 85 `.rodata`, and 80 bytes
of BSS extent. Every retained runtime-section byte, named symbol value/size/
binding/type, ordered relocation row and section alignment is unchanged. All
484 function-score rows are unchanged, including 360/471 exact game functions
and thirteen exact vendor controls. Nothing is newly banked.

Strict source data moves 6/59 to 8/59 (PSX 0/1, GAME 6/39, OPEN 2/19):

- Newly passing whole data units: GAME player_death, actor and map_object_pool;
  OPEN opening_fade.
- Newly failing whole data units: GAME menu_map_viewer and OPEN render. Their
  existing RODATA claims exceed the emitted string bytes; the old automatic
  padding happened to fill the difference. Claims are not shrunk here.
- GAME item still has matching named BSS layout but invalid source alignment.
  It additionally reports 40 claimed RODATA bytes versus 37 emitted bytes.
- Several switch-table failures now expose their original REL-addend divergence
  instead of stopping first at a larger source extent.

SDK contributions remain 2/2; target relinking stays 108/114 with the same six
conflicts. The 625 reached config ranges without independent comparisons and
the unresolved stack/indirect-reference paths remain. The required full build
continues to reject these strict data, ownership and placement failures. A
cleaner compiler-output contract is not reachable-byte or linked-image closure.

All 573 local repository tests pass without skips, along with both native
compiler smoke probes, Ruff and diff checks. `nix flake check -L` passes,
including the compiler/objdiff controls and 573 sandbox tests with 87 expected
local-artifact skips. No data gate, ownership gate or historical function
baseline is relaxed.
