# Typed resource allocation boundaries

## Function Match Plan

Baseline is `bba263a`. Image-specific retail disassembly/CFG, signatures,
callers, callees, strings, match state and pre-edit objects are captured under
`build/cast-model/resource-boundaries/`. Existing CD/resource dossiers, source
history, the allocation APIs and adjacent loader bodies constrain this change.
All ten functions below are game-owned file/resource policies calling vendor
CD, TIM, audio and allocator services; no vendored body is reconstructed.

Both CD allocators write a generic pointer through a `void **` output. Their
callers currently pass addresses of `u8 *` or `u_long *` locals through casts.
The same stream carries heterogeneous length-prefixed data; a TIM allocation
is generic storage until passed to its SDK-typed decoder. First hypothesis:
keep these output locals as `void *`, matching the actual allocation contract,
and retain their addresses and lifetime across calls. The established GNU C
probe defines byte arithmetic on `void *`; all existing chunk increments
remain byte counts. These are generic serialized buffers, not a replacement
for a supported record type. Typed record decoders and their casts at payload
boundaries remain explicit in this stage.

A separate `void *` allocation local plus a typed cursor would change which
pointer escapes to the allocator. Retail repeatedly reloads and stores the
escaped stream slot even after its last decoded payload, so preserve the one
observed stream variable. Do not introduce another carrier to retain stores.
The three TIM output locals need no pointer arithmetic. C's implicit conversion
from a generic allocation pointer to the decoder's authentic SDK type applies.

The magic loader reads 120 words into the known 24-record table. Review its
sole caller's incompatible u8/word argument as part of this boundary pass;
a complete table model, if needed, has its own field and consumer evidence.
No call set, constant, CFG, delay slot, relocation, profile or resource path
will change. Incoming candidate pointers remain hypotheses. Allocator return
values keep their existing status contract, and no status is invented from a
pointer result.

| Image / function | VA / bytes | Strict before | Blocks/JAL/returns | Planned source change | Final verdict |
| --- | --- | ---: | --- | --- | --- |
| GAME `cd_file_load_allocated` | `8001acf0 / 170` | 100 | 13/9/1 | Review allocation output/status contract; body unchanged | 100%; raw unchanged |
| GAME `common_resources_load` | `8001b180 / 210` | 100 | 7/10/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| GAME `magic_load_records` | `8003a274 / 2c` | 100 | 3/0/1 | Review complete 480-byte copy and input boundary | 100%; raw unchanged |
| OPEN `cd_file_load_allocated` | `80016014 / 148` | 100 | 11/7/1 | Review allocation output/status contract; body unchanged | 100%; raw unchanged |
| OPEN `opening_resources_load_scene0` | `80016348 / 1c8` | 100 | 1/18/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| OPEN `opening_resources_load_scene1` | `80016510 / b4` | 100 | 1/8/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| OPEN `opening_resources_load_scene3` | `800165c4 / f0` | 100 | 1/12/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| OPEN `opening_resources_load_ending` | `800166b4 / 134` | 100 | 1/12/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| OPEN `opening_resources_load_ending_entities` | `800167e8 / 58` | 100 | 1/3/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |
| OPEN `opening_resources_load_ending_sequence` | `80016840 / 9c` | 100 | 1/7/1 | Generic allocation/stream local; remove void-double-pointer reinterpretation | 100%; raw unchanged |

Rebuild both resource units from changed source and compare all their functions'
linked words, calls and ordered referents against baseline and retail. Preserve
every exact result. Then run the complete build, existing compiler/ownership
controls and repository tests, lint and diff checks. No new test should merely
assert the spelling of an eliminated cast. Bank only directly changed exact
functions, after raw comparison and verification.

## Result

All 25 functions across the two resource units and the two reviewed GAME
callee units preserve their linked words, calls and ordered references. The
24 exact functions remain exact; the existing `magic_cast` partial is
unchanged. All seven directly edited loader functions remain exact. The
repository still has 439/471 exact functions, with no score movement.

Thirteen output-address casts are removed while retaining the allocator's
`void **` contract and each escaped stack slot. Chunk and TIM data remain
generic allocated storage until their typed consumer calls. The native GNU
C probe emits the same byte increments, loads and stores; no pointer escape,
extra local, call, cursor update or payload boundary is removed to influence
the result. No header, function signature or curated owner changes here.

The fresh AST census parses all 112 image variants without errors and reports
no incompatible-pointer diagnostics. The magic loader now receives generic
payload storage through the valid C conversion to its word input; its
complete typed-table representation remains part of the next model review.
There are 743 written casts, including 43 header checks/conversions. C pointer
casts fall to 482, down 324 from the initial 806.

All 713 repository tests pass with nine skips. Ruff and diff checks pass.
The full build was run after the source changes and keeps the pre-existing
data/reference/placement failures without artifact failures or exact-function
regressions. Only the seven directly edited exact loaders are banked.
