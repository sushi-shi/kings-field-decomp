# Preserve tentative-data linkage through the assembler adapter

This is a compiler-input contract correction, not an attribution of the retail
assembler. No game source, inventory extent, profile or target layout changes.

The pinned maspsx revision `746b895f02929ecd148af7b1f4ff05b69f973878`
already distinguishes `.comm` from `.lcomm` while preprocessing. Its final
allocation pass nevertheless exports every `.bss` object, including private
`.lcomm` objects, and does not explicitly export `.comm` objects placed in
`.sbss`. Section choice is not the linkage rule.

For the control's four-byte `int` objects, pinned GCC 2.5.7 emits
`.lcomm counter,8` followed by `.comm public_word,8`; GCC 2.6.0 emits the same
directives with size 4. The C `static` object uses `.lcomm`; the externally
visible object uses `.comm`. Before the patch, the compiler smoke control fails
with `counter: GLOBAL, size 4`, despite `static int counter;` in its source.
The four-byte symbol size comes from the DATA claim; it does not reduce GCC
2.5.7's eight-byte allocation or establish historical alignment. The two probes
must not be treated as having identical allocation rounding.

`patches/maspsx-private-bss.patch`, applied to the pinned source in the flake,
exports only `.comm` definitions in either allocation section. The optional
`--use-comm-section --use-comm-for-lcomm` path emits `.local` before converting
a private allocation to GNU `.comm`. Public COMMON remains public; private
storage remains locally allocated. The default project's G0 profile and its
packed `.bss` allocation model are unchanged.

## Executable controls

`nix develop -c python3 tests/compiler_mips_smoke.py` checks both GCC probes:

- tentative private/global objects retain LOCAL/GLOBAL binding, four-byte
  claim sizes and existing allocation offsets (0/8 for 2.5.7; 0/4 for 2.6.0);
- two objects with a same-named private `counter` link together with GNU `ld -r`
  into distinct allocations, without a multiple-definition collision;
- a third TU's reference resolves the exported `public_a`, but its reference
  to `counter` remains undefined—it cannot bind to either private object.

Six direct adapter controls cover G0/G8 crossed with default allocation,
public COMMON, and the opt-in local-through-COMMON path. They check binding and
allocation class, including default `.bss`/`.sbss` offsets. These controls run
in `nix flake check -L`; G8 is not enabled for game compilation.

## Retail campaign verification

Recompiling all 117 units changes six source objects and makes eleven existing
private data symbols LOCAL. Every allocated section's type, extent and alignment,
and every named symbol's offset and size remain unchanged. The other 111 source
objects are byte-identical.

The raw MIPS differences are the expected GNU-as section-relative representation
of private references. GAME player turn-limit accesses retain `.bss + 8`;
both PAD units retain buffer/status at `.bss + 8/+16`, with `PadIdentifier`
still exported. OPEN resources retain the location at `.bss + 0` and saved
cursors at `+8/+16`. The GAME formatter retains `+0`; OPEN retains the buffer's
interior `+7`. Relocation kinds/sites, instruction forms, constants, calls,
branches and delay-slot schedules are unchanged. Only relocated low fields
absorb nonzero private-symbol offsets.

All 484 function score rows remain identical, including 354 exact game functions
and both six-function vendor PAD verification units. Strict source data remains
15/63 and target relinking 109/117. The full build still rejects those data
mismatches, eight placement conflicts and unresolved reachability paths.

This fixes accidental symbol export, not original `.sbss` versus external-BSS
allocation topology, complete BSS extents, or whole-program byte closure. Native
ASPSX execution remains unavailable because the supplied executable requires
its key; that limitation does not justify discarding explicit `.lcomm` linkage.
