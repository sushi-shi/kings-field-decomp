# Reconstruction sources

King's Field is three independently linked decompilation targets in one
repository. Put target-specific reconstruction in `psx`, `game`, or `open`.
Promote code to `shared` only after cross-image identity and compatible build
context are established.

Do not reconstruct functions listed in
`config/retail/functions_vendored.tsv`. They are external Sony/Psy-Q inputs,
not decomp targets.

The initial objdiff topology is one function per object. Historical
translation-unit ownership will be added as curated metadata rather than
inferred from neighbouring linked addresses.
