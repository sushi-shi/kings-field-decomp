# GCC 2.5.7 address-input quantities

OPEN `display_initialize` (`80016adc`, 472 retail bytes) uses four `dfe`
stores relative to the first `DRAWENV.dtd` address. The current source finds
that root during CSE1, then changes those stores to absolute addresses during
CSE2. In the adjacent exact `render_initialize` (`80016908`, 468 bytes), the
light-matrix root remains in a register across three calls. Both functions
use the same complete `open_graphics_runtime` owner. Their differing results
do not establish that keeping any base live prevents member-address folding.

The optional trace compiler now observes the address and base quantity at
entry to `find_best_addr`'s address search, after its existing constant/stack
early return and before its preliminary fold. The production compiler and
`-O2 -G0 -mcpu=r2000` profile are unchanged. This explains the pinned Decompals
probe; it does not prove the historical compiler that emitted retail.

## Event and report contract

`cse.address` records the untouched input address in `x`, its directly
identified `REG` or `PLUS(REG, CONST_INT)` base in `pseudo`, and the actual
`qty_const` entry in `y`. The three integer values are the quantity number,
`REGNO_QTY_VALID_P` result, and mode agreement. Mode agreement is `-1` when
the quantity is unassigned. No quantity arrays are indexed in that case.
A null `y` differs from a non-null `CONST_INT 0`.

The reason is `store` only when the observed address location is the direct
destination address of the instruction's simple `SET`; other memory inputs
are marked `other`. The hook reads existing state only. It does not allocate
RTL, invoke recognition, repeat a folding decision, or change an instruction.
All observation work is guarded by the existing opt-in trace switch.

The existing `address_lifetime` feature adds `address_inputs` for CSE1 and
CSE2. It selects only the member-store UIDs identified in the corresponding
stage snapshot and only their `store` events. It groups observations by direct
register versus expression, quantity state, and mode agreement. Quantity
states distinguish a recorded constant, a valid quantity without a constant,
an unassigned quantity, and an unidentified base. Event evidence retains
register, quantity and UID identities; frontier values exclude those IDs.

Counts describe observations, including repeated CSE path scans. They are not
counts of stores. `stores_without_observation` records incomplete coverage;
status is `observed`, `partial`, or `unavailable`. Old traces without this hook,
and addresses skipped by the early return, do not become evidence of an
absent constant. A recorded constant alone is not a prediction of folding.

## Controls and actual OPEN results

The synthetic `member_root_and_offset` control writes two real word fields,
passes the object to a call, restores both fields, and passes it again. Its
four CSE1 address inputs are registers with recorded constants. In CSE2, both
root stores still enter as registers and both offset stores enter as
expressions; all four bases have recorded constants. Only the offset stores
become absolute through `cse.fold-address`. The root remains live across one
call. The dynamic-pointer control instead records twelve valid quantities
without constants across both CSE passes. The existing folded-member control
records four expression inputs with constants in CSE2 and four absolute
rewrites.

Independent debugger captures from the uninstrumented debug compiler agree
with every selected input address, base register, quantity number, validity
and recorded constant: 15 distinct states in canonical display initialization,
eight in exact matrix initialization, ten in the retained explicit-INTRO
display control, and eight in a small SDK `MATRIX` control. Repeated compiler
invocations in the debugger logs are compared as distinct state sets, not
claimed as additional optimization passes.

Canonical display's four selected stores produce eight CSE1 observations:
four valid quantities without constants and four unassigned quantities on
the later path scan. CSE2 observes four expressions with recorded constants.
The selected matrix stores instead observe two direct-register addresses
with constants in each pass. These facts agree with the already documented
store forms and explain why address input, recorded quantity, final address,
and allocation lifetime must be inspected separately.

## Verification and reproduction

The new compiler-source fingerprint is
`222b6cc36272847ffde09ad4b7f2db81632fe7fd282bfc6df1f155939b63cb12`.
The previous instrumented package is retained separately. Native, debug,
trace-disabled and two trace-enabled builds produce byte-identical whole
objects for the synthetic corpus and both OPEN units; repeated traces are
identical. Parser tests cover member selection, other memory inputs, missing
hooks, zero constants, mode mismatch, unassigned quantities and identity
renumbering without frontier changes.

The raw audit exposed an existing smoke-harness omission: `--open-units`
did not forward the configured assembler division-expansion flag. The harness
now uses each unit's `aspsx_version` and `maspsx_flags` and records both in its
parity report. The corrected run preserves all eleven canonical function
bodies, ordered calls and referents, including all nine raw-exact siblings.
The earlier run still established compiler parity, but its ending size and
score did not represent the production assembler profile.

```sh
nix develop -c nix build .#gcc257Instrumented --out-link build/gcc257-address-compiler
nix develop -c python tests/gcc257_trace_smoke.py \
  --debug build/gcc257/debug/bin/cc1psx-257-debug \
  --instrumented build/gcc257-address-compiler/bin/cc1psx-257-trace \
  --output build/gcc257-address-verification --open-units
```

Generated plans, traces, debugger comparisons, raw audits and verification
logs are under `build/gcc257/address-quantity-instrumentation/`. This tooling
change does not close either remaining OPEN function or alter any banked
source. See [display address evidence](open-display-init-pointers.md) and
[the original trace contract](gcc257-instrumentation.md).

Repository lint, 710 tests (nine skips), `nix flake check -L`, and
`git diff --check` pass. A full `kf build` recompiles the production units and
preserves the function totals: PSX 1/1, OPEN 106/108, GAME 329/362. Its final
gates still fail on existing data placement, extent and ownership issues.
For example, PSX's eight-byte datum at `80010224` conflicts with the ELF
section's 16-byte alignment. The same failure is recorded in the earlier
graphics-owner and status-render full-build logs; the production compiler,
source, configuration and data-comparison rules are unchanged by this work.
The full build is therefore not reported as passing, and no function is
banked by this instrumentation change.
