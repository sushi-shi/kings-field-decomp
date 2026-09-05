# VMANAGER private pitch-table contribution

## Function Match Plan

Baseline: `43e3cbc`. Keep both `note2pitch` and `note2pitch2` instances vendored,
without reconstructing or banking their code. Recover their shared complete
private halfword table from the actual LIBSND provider. Split the surrounding
census gaps without losing their unrelated bytes/references. Review four
HI16/LO16 pairs against the raw instructions and SDK section-base patches.
Compare the full SDK contribution independently with each retail allocation,
including original alignment, private linkage, native objdiff and relinking
both sides. No score waiver, guessed tail or symbol-bounded crop is permitted.

## Independent extent and ownership

The pinned Release 2.5 `LIBSND.LIB/VMANAGER.OBJ` (LNK v2) declares section
`51b0`, `.data`, raw LNK alignment tag eight, and emits exactly one 386-byte
Code record. [Original-linker calibration](../../docs/patterns/psyq-section-alignment.md)
establishes that this tag requires **four-byte** alignment under PSYLINK 1.17.
There are no `.data` exports, local symbol records, reservations or patches.
Its two local text routines reference `(sectbase(51b0)+$0)`. Thus the complete
section can receive a curated **local** identity, `svm_pitch_table`; this is a
semantic name, not a recovered SDK symbol spelling. The `lhu` consumers support
`u16[193]`; the first and last entries are `0x1000` and `0x2000`.

| Artifact | SHA-256 |
| --- | --- |
| VMANAGER.OBJ | `96242c8cb04ee3038d1f91133e4155a9a3356870301523b21fe3778f0ef4c570` |
| Entire `.data` payload | `293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2` |

| Image | Complete allocation, exclusive end | File offset | Preserved census gaps |
| --- | --- | --- | --- |
| GAME | `80056858..800569da` | `45058` | `80056564+2f4`, `800569da+2` |
| OPEN | `80035fd0..80036152` | `247d0` | `80035cdc+2f4`, `80036152+6a` |

The whole payload occurs exactly once in each hash-validated executable and
equals all 386 SDK bytes. Its size is not rounded to its four-byte alignment.
The old gaps contain other references: each prefix is still reached by SDK
functions, and OPEN's suffix has additional candidates. They are retained,
not silently incorporated in this table or called matching data.

`LIBSPU.LIB/S_N2P.OBJ` contains the same complete table, but is only a
corroborating control: the retail consumers are VMANAGER functions, not
`_spu_note2pitch` or `_spu_pitch2note`. Neither identical bytes nor proximity
would establish the correct provider on its own. This campaign imports only
VMANAGER's entire `.data`; its `.sdata`, BSS and text remain separate work.

## Per-function evidence snapshots

All four functions have their full disassembly/CFG, callers, callees, strings,
adjacency, inventory history and match state inspected. Current strict state
is **vendored, no reconstruction unit** for each; none has a game-code score.
The original table references were seeded in `d14f4fd` without owner names.

| Image / function | VA / body size | Table HI / LO | SDK local text offset |
| --- | --- | --- | --- |
| GAME note2pitch | `80042da0 / bc` | `80042e1c / 80042e20` | `a14` |
| OPEN note2pitch | `80022bc0 / bc` | `80022c3c / 80022c40` | `a14` |
| GAME note2pitch2 | `80044ea8 / 104` | `80044f68 / 80044f6c` | `28cc` |
| OPEN note2pitch2 | `80024cc8 / 104` | `80024d88 / 80024d8c` | `28cc` |

`note2pitch` uses private byte note/centre/fine-tuning state, divides by twelve,
forms a sixteen-step semitone index, doubles it and reads an unsigned halfword.
It shifts the result by the octave and returns through `jr ra; andi
v0,v1,0xffff`. It makes no calls and references no strings. `SpuVmKeyOn` calls
it at GAME `80043d1c` / OPEN `80023b3c`, with a NOP call delay slot.
Its neighbors are `SpuVmKeyOffNow` and `SpuVmSelectToneAndVag` (the latter
starts at GAME `80042e5c` / OPEN `80022c7c`). Existing VMANAGER evidence for
these neighbors is retained; no original complete-file boundary is inferred.

`note2pitch2` takes note/fine-tuning arguments through a0/a1, uses the private
tone pointer and 32-byte tone records, narrows the key difference to signed
16 bits, and performs the same halfword lookup and octave shift. Call sites
explicitly narrow arguments; SePitchBend, SsUtKeyOn and SsUtChangePitch finish
the a1 signed extension in the call delay slot. It also makes no calls, has
no strings, and returns with the same 16-bit mask in the return delay slot.
Its neighbors are SePitchBend and SsUtKeyOn. Callers and sites are:

| Caller | GAME call | OPEN call |
| --- | --- | --- |
| SpuVmPitchBend | `80043844` | `80023664` |
| SePitchBend | `80044e7c` | `80024c9c` |
| SsUtKeyOn | `80045330` | `80025150` |
| SsUtChangePitch | `80045598` | `800253b8` |

The table pairs are at function offsets `7c/80` and `c0/c4`, respectively;
the SDK uses patch types 82/84 with zero addends to its `.data` section.
For note2pitch these are record offsets `a90/a94`; for note2pitch2 the later
text record uses `3f0/3f4`. The following ADDU and LHU confirm the referent
and element width. Curated rows are reviewed, with conservative validated
references; none is promoted to proven control flow by an xref filter.

The reviewed cosine reference at GAME `8004f3e4` / OPEN `8002f1b8` numerically
forms this table's base plus `18`, but its real referent remains `rsin_tbl-800`.
Its SDK expression and quadrant index prove that ownership. Preserve that
explicit sine-table identity and negative addend; tests ensure it never becomes
a pitch-table xref merely because of address coincidence.

## Comparison boundary and reproduction

`config_data.source_section` accepts global sections only with the exact
named offset-zero export, and private sections only when **no** SDK exports
exist. The parser still rejects extra local symbols, data patches, reservations
and unknown records. Both generated ELF objects must own exactly one whole
386-byte `.data`, aligned four, with one local STT_OBJECT and no code/relocs.
The provider conversion never reads retail bytes; target carving never reads
the SDK. Both sides independently relink to the image-qualified retail address.
Native objdiff must report all 386 bytes matched, with no fake function score.

Inside `nix develop`, after `kf init --retail-dir <verified-extraction>`:

```sh
python -m unittest tests.test_config_data tests.test_psyq_pitch_table tests.test_psyq_rsin_table
kf sema --image game xref svm_pitch_table
kf sema --image open xref svm_pitch_table
kf build
kf verify reachability --output build/pitch-table-reachability.json
```

Full reachable-byte ownership and linked-executable equality remain open.
Final per-function verdict: each of the four SDK consumers stays vendored and
unchanged; no source body, signature, game progress or banked baseline changes.

## Final verification

Both default native reports match `386/386` data bytes with no functions, and
both target/provider relinks verify every byte. Config SDK comparisons increase
2/2 to **4/4**, adding 772 independently matched bytes. Target relinking rises
108/114 to **110/116**, retaining the same six pre-existing section-base
conflicts. Strict source-owned data remains **8/59**.

All 112 source objects and all 112 module target objects are byte-identical to
the baseline after a full rebuild. Every one of the 484 function scores is
unchanged, preserving **360/471 exact game functions** and thirteen exact
vendor controls. Only the four individual SDK target objects change: the two
GAME low instructions and OPEN note2pitch2's low instruction replace the old
gap-relative `2f4` addend with zero; OPEN note2pitch gains its previously
withheld pair, replacing the linked HI/LO fields with zero addends. All four
now reference `svm_pitch_table`; their relinked words equal retail exactly.

Reached config ranges rise from GAME 359 / OPEN 268 to GAME 360 / OPEN 270.
The independent matches rise from one to two per image. Unmatched config
ranges therefore rise **625 to 626** (GAME 358, OPEN 268): OPEN's retained
suffix is a distinct reached unresolved range, not part of the pitch table.
Neither broad prefix disappears, and GAME's two-byte suffix remains in the
census even though no current reference reaches it. OPEN loses one candidate
reference path through the reviewed note2pitch pair; its indirect control,
other candidate references and unmodelled alternate-stack target remain open.
Source/header `DAT_` occurrences remain 170; there is no source-renaming claim.

All **580 local tests** pass with no skips; Ruff and diff checks pass. The
flake checks pass with 91 expected local-artifact skips, including the new
SDK-record and metadata controls in the sandbox. The mandatory full build
remains red on explicit data, reference-closure and placement failures, not
compiler failures or function regressions. No baseline is relaxed or banked.
