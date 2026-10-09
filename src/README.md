# Reconstruction sources

King's Field is three independently linked decompilation targets in one
repository. Put target-specific reconstruction in `psx`, `game`, or `open`.
Reusable helpers live in `lib`, including helpers used by only one image.
Sharing a source between images requires compatible signatures, behavior,
and build contexts; differences remain explicit image variants.

Do not reconstruct functions listed in
`config/retail/functions_vendored.tsv`. They are external Sony/Psy-Q inputs,
not decomp targets.

`config/units.toml` enrols source units in linked order for each image.
A unit may own several contiguous function claims and its data contribution.
Module boundaries remain curated hypotheses: neighbouring linked addresses
alone do not establish an original translation unit. See
[claims and ownership](../docs/build-system.md#unit-manifest-and-address-claims).
