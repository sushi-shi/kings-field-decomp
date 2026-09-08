# Typed menu ordering-table index

## Function Match Plan

GAME `primitive_buffer_commit_poly_ft4` (`8002ad1c`, 0x50 bytes) is 100%
exact. It has one block, one SDK `AddPrim` call and one return, with five
validated data-address pairs and no strings. Twenty-one confirmed calls from
menu drawing routines pass an ordering-table index. Retail shifts that index
left two bits, loads the existing `u32 *` ordering table and passes their sum
to `AddPrim` in its delay slot. It then advances the current `POLY_FT4` by
40 bytes and publishes the new heterogeneous primitive-buffer byte cursor.

The index is already an element index into a typed table. Replace its explicit
byte scaling and two pointer casts with direct typed indexing. Keep the cast
that publishes the packet pointer as a generic allocation cursor: that changes
the active representation of reusable storage. Preserve the exact SDK types,
all six ordered references, call and return delay slots, and pointer increment.

The six-view dossier, adjacent begin/list-init functions, source history and
pre-edit unit objects are under `build/cast-model/typed-ordering-index/`.
This is game menu buffer policy around separately vendored `AddPrim`, not SDK
body reconstruction. Compare every linked word and physical referent from the
first divergence; retain the existing exact function if an unattributed
instruction residue remains. Verify the entire affected unit, full build,
target-C census, all repository tests, Ruff and diff checks before banking.


## Result

Direct typed indexing removes both pointer casts and the explicit byte-shift
statement while retaining all twenty instructions of the function. The
published primitive cursor still uses an explicit generic byte-buffer view.
All sixteen functions in the unit retain their complete linked words, calls
and ordered referents; fourteen remain exact and two existing partials are
unchanged. The edited function is 100% and is the sole bank selection.

All 112 target-C variants parse without errors. The census is 711 written
casts, including 46 in headers; C pointer casts total 451, down 355 from the
initial 806. The full build retains 439/471 exact functions, the documented
closure/placement gates and zero artifact failures.

All 714 repository tests pass (nine environment-dependent skips), as do
Ruff and `git diff --check`.
