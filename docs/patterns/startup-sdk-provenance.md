# Startup and allocator SDK provenance audit

The allocator entry points are Sony library/BIOS code, already excluded from
game progress. The audited GAME/OPEN startup bodies and shared arena wrappers
do not match the supplied SDK. Their absence from those archives does not
prove independent authorship or exclude an unavailable startup template.

## Scope and results

The audit hash-validated all three retail images with `kf init`, read the
startup and memory-family disassembly and references, and rebuilt function
signatures from the pinned Release 2.5 media. The general library directory
provides 1,226 signatures; H2000 provides 179. Each comparison masks only
instruction bits covered by the SDK object's actual relocation records.

Nineteen functions received a focused check: PSX `main`, both overlay `main`
and `repeat_store_word` functions, and seven memory-family functions in each
overlay. None matched either corpus. A broader check covered all 471 current
game-source claims. It found 72 ambiguous signature hits at eight addresses,
all explained by generic store/return or call/return sequences below. No new
provider attribution follows from those hits.

All 266 files in the preserved SDK tree were also searched for the complete,
identical 36-byte GAME/OPEN repeated-store body. There were no occurrences.
The 78 C/header/assembly/make/link files contain no `InitHeap`, `sectstart`,
`sectend`, `801f8000`, `67fe8` or `70218` source spelling. This is a bounded
negative result for this media tree, not a complete historical SDK-source
search. Different compiler output or an unavailable example remains possible.

## Positive provider controls

| Image / address | Provider | Evidence and verdict |
| --- | --- | --- |
| PSX `800101e4`, GAME `8005040c`, OPEN `80030150` | `LIBAPI.LIB/C57.OBJ`, `InitHeap` | All 16 text bytes agree, including trailing padding. The two overlay rows are upgraded from later wildcard signatures to exact Release 2.5 member text. |
| GAME `800502dc`, OPEN `80030050` | `LIBAPI.LIB/C51.OBJ`, `malloc` | All 16 text bytes agree, including trailing padding; existing exact attribution confirmed. |
| PSX `800100f8/80010100` | `LIBSN.LIB/SNMAIN.OBJ`, `__main` and entry | Existing masked object attribution; section-boundary patches initialize BSS and the heap. |
| GAME `8003ac54/8003ac5c`, OPEN `8001aa74/8001aa7c` | `H2000/LIB2000/NONE2.OBJ`, `__main` and entry | Existing 24-byte masked object attribution; establishes GP and transfers to the program's `main`, with no heap setup. |

`C57.OBJ` SHA-256 is
`b6d724543f288d82ca8a9931dfbd384dc646270bfcbe8d97c78d3f86848b7149`.
Its complete text is `a0000a24 08004001 39000924 00000000` in file byte order:
load the A0 BIOS vector into `t2`, jump through it, and select service `0x39`
in the delay slot, followed by padding. `C51.OBJ` uses service `0x33` instead;
its SHA-256 is
`f27e521d39e000877852efd6c28c2e53a66199cea3acc22606244bded0eaf797`.
These are BIOS trampolines. The BIOS allocator implementation is not present
in these EXE function bodies and has not been reconstructed as game code.

The SDK `bzero` (`C40`) and `memset` (`C43`) controls are also BIOS trampolines,
using services `0x28` and `0x2b`. They do not explain `repeat_store_word`.
`SNMAIN` clears successive words with a four-byte pointer increment; the
game helper repeatedly writes the same word without advancing its pointer.

## Ambiguous all-source hits

| Retail functions | Actual referent, preserved by the audit |
| --- | --- |
| GAME `8001c138`, OPEN `80016eb8`, `tmd_set_current_vertices` | Stores into the corresponding graphics-runtime object. Four unrelated SDK setters have the same masked four-instruction shape. |
| GAME `8003379c`, `map_event_set_current` | Stores into `map_runtime_state+0x220`; the same generic setter collision. |
| GAME `8001c5ec`, OPEN `8001736c`, `tmd_release_last_allocation`; GAME `8002c2e4`, `save_workspace_release` | Calls the corresponding `memory_release_last`. |
| GAME `8002cab4`, unresolved wrapper | Calls `actor_pool_find_free`. |
| GAME `800365f8`, `map_load_floor_wrapper` | Calls `map_load_floor`. |

Each of the five call wrappers matches twelve unrelated SDK function shapes
after masking the call target. The decoded call/data targets distinguish them;
the pattern collisions do not establish library ownership.

## Existing reconstructed library sources

There are already 13 source claims overlapping the vendored inventory, all
in the three explicit `scope = "vendored"` units under `vendor/src/`:
GAME's LIBETC interrupt tail and GAME/OPEN's LIBETC PAD front ends.
They are retained reference reconstructions, excluded from the game progress
and banking ledger. The native EXE builder also excludes these units and uses
the SDK archives. No ordinary game-source unit claims a currently identified
vendored function.

## Final provenance verdicts

- `InitHeap`, `malloc`, `bzero`, `memset`, and SDK entry routines: library
  ownership established; BIOS services distinguished from their EXE stubs.
- PSX/GAME/OPEN `main`: program-specific overlay control is established;
  independent authorship of every initialization statement is not.
- Both `repeat_store_word` functions: behavior and cross-overlay identity are
  established; no supplied SDK implementation matches; original source
  provenance remains unresolved. Placement/FID absence alone is insufficient
  to label it definitively game-authored.
- The seven shared memory functions: no supplied SDK match; their mutable
  arena object, allocation modes, LIFO records and overlay-specific budgets
  establish a separate policy layer above BIOS `malloc`/`InitHeap`.

Generated evidence is under `build/startup-address-audit/sdk-provenance/`.
To reproduce the signature checks, run `kf-fid-census --exe-dir <disc>` once
with `--sdk-lib-dir "$PSYQ_LIB"` and once with
`--sdk-lib-dir "$PSYQ_H2000_LIB"`, using separate `--output` and
`--corpus-output` paths; intersect the hits with the image-qualified manifest
claims. For the byte controls, extract `LIBAPI.LIB` with `psyk extract`, read
`C57.OBJ`/`C51.OBJ` with `psyk list --code`, and compare their 16-byte text to
the listed image-qualified addresses, including the return delay slots and
padding. No runtime execution is part of this audit.
