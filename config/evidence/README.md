# Reviewed evidence

This directory preserves the small, reviewable inputs behind
the curated retail inventories; it does not contain Sony binaries.

`game_match_campaign_30.tsv` records the pre-edit MIPS evidence and final
matching verdicts for the first GAME reconstruction campaign.
`game_semantic_math_lifecycle.tsv` records the assembly, CFG, xref, SDK-header,
and vendored-negative-control evidence behind the first lifecycle/frame-pacer
and fixed-point helper names. These campaign tables support identities in
`function_identities.tsv` and `data_identities.tsv`; they are not recovered
original symbols.

`game_semantic_audio_control.tsv` records the per-function retail, caller,
string, shared-state, and Psy-Q 2.5 header evidence for GAME audio
initialization, VAB loading, and sequence lifecycle policy. API-name candidates
without an exact FID remain explicitly qualified in that ledger.

`game_semantic_audio_spatial.tsv` records positional sound, listener snapshots,
and the ten-slot voice manager. The companion `vmanager-key-utilities` rows in
`overlay_lineage.tsv` prove that the called `SsUtKeyOn`/`SsUtKeyOff` bodies are
shared SDK code in GAME and OPEN even though their version-skewed bytes evade
the exact Release 2.5 FIDs.

`game_semantic_camera_events.tsv` records the camera-path interpolator and the
adjacent map-event family. It includes the stack-owned path-state layout, the
eight-record event-pool boundary, five shared global/static candidates, the
ambiguous short FID on the current-event setter, and six rejected false
control-flow xrefs. The two families share an investigation band, not proven
translation-unit ownership.

`psyq_release_25_text_sections.tsv` is the `.text` subset of the relocation-
aware object match report produced during the original King's Field
investigation. Each address was found by masking only link-editable bits in a
Psy-Q object and comparing every remaining bit against a hash-verified retail
PS-X EXE. Duplicate `S_R`/`S_W` rows intentionally retain an archive ambiguity.

`psyq_release_25_complete_objects.tsv` records six reviewed 16-byte `LIBAPI`
members in `PSX.EXE`. These were below the original matcher's 32-byte cutoff,
so their complete-byte comparisons were reviewed separately.

`kf-vendored-seed` does not blindly trust either table. It extracts the named
members from the hash-pinned Release 2.5 archive with `psy-k`, reconstructs
interleaved `.text` records, applies the retained MIPS relocation masks, checks
sizes/bit counts/relocation counts, and verifies each recorded occurrence in
the clean retail executables. It also checks XDEF offsets before using a symbol
as a function name.

The external SDK archive is supplied by the Nix environment and remains
hash-pinned there. The executable inputs are user-supplied; their SHA-256,
header, load address, and load size must match `scripts/kf/retail.py` before
the evidence or the secondary Psy-Q 2.60 signature lane is accepted.
