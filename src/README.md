# Reconstruction sources

King's Field is three independently linked decompilation targets in one
repository. Put target-specific reconstruction in `psx`, `game`, or `open`.
Promote code to `shared` only after cross-image identity and compatible build
context are established.

The initial objdiff topology is one function per object. Historical
translation-unit ownership will be added as curated metadata rather than
inferred from neighbouring linked addresses.
