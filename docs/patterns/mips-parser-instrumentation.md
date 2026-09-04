# MIPS parser execution instrumentation

The isolated parser runner uses pinned Unicorn 2.1.4 in little-endian MIPS32
mode. It is neither a game boot nor a cycle-accurate R3000 emulator. Executed
load-delay dependencies are audited separately and rejected when the immediate
consumer would observe different R3000 behavior.

## Memory-write hooks alter delayed control flow

A reduction of GAME's animation allocation-success branch demonstrates an
instrumentation defect without any allocator hook or game data:

```text
80020698  bnez v0,800206b0
8002069c  sw   v0,12(s4)
          ...
800206b0  j    800206dc
800206b4  sw   s4,0(s1)
          ...
800206dc  jr   ra
800206e0  nop
```

With valid RAM addresses and nonzero `v0`, raw Unicorn reaches the return.
Installing an otherwise empty `UC_HOOK_MEM_WRITE` makes the same run raise
`UC_ERR_EXCEPTION` at PC zero immediately after observing `800206b0`, before
its store delay slot executes. A code hook alone does not trigger it. Either
store delay can expose the defect when its target starts with another control
transfer. An actor-placement reduction also executes a jump target twice when
the memory-write hook is installed.

The runner therefore does not install that hook or suppress repeated PCs.
Instead it decodes each integer store at the existing instruction boundary,
guards its effective byte span before execution, and verifies the actual RAM
bytes at the next instruction boundary (also on final return). Only after
verification is the write recorded. This leaves CPU instruction execution to
Unicorn and does not patch retail instructions. Unsupported coprocessor or
non-MIPS-I stores fail closed; explicit service-hook writes have their own
guard. `SB`, `SH`, `SW`, and all four little-endian `SWL`/`SWR` alignments have
byte-level controls, including signed displacements and KSEG aliases.

The reductions, allowed-write negative controls, hook returns, and legal
jump-to-own-delay-slot double execution are regression tests in
`tests/test_parser_machine.py`. This instrumentation is bounded semantic
evidence, not a replacement for strict objdiff or hardware validation.
