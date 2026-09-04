# Psy-Q CPE residue at the GAME initialized-data boundary

`GAME.EXE` contains these bytes at virtual address `0x80057e68` (file offset
`0x46668`):

```text
43 50 45 01 08 00 03 90 00 00 00 00
```

They decode as the standard Psy-Q CPE v1 prefix: `CPE\x01`, select unit zero,
then the start of a 32-bit register-value record. The controlled PSYLINK 1.17
probe in `tests/psylink_order_smoke.py` now checks that every produced CPE has
the same six-byte magic/select-unit prefix. These are conversion residue in
the final partially occupied PS-X EXE load page, not a meaningful source
initializer.

This matters because referenced storage can begin inside that padded load
page. `player_update` treats `0x80057e68` and `0x80057e70` as signed motion
limits, writes both on every ordinary-update path before reading either, and
is their only referencing function. The two four-byte objects are eight bytes
apart, matching the old GCC/maspsx common allocation observed by the build
pipeline. They are therefore modeled as separate BSS objects even though their
addresses fall below the PS-X EXE header's page-rounded load end. All of their
references are confined to `player_update`, and file-static tentative BSS words
receive the observed eight-byte alignment in the pinned GCC/maspsx pipeline.
The source therefore defines both limits privately in `player_update.c` rather
than leaking declarations through a shared header.

Do not classify a referenced address as initialized source data merely because
it falls inside the page-rounded PS-X EXE payload. At a suspected tail
boundary, inspect the raw bytes, prove write-before-read behavior and xref
ownership, and compare the spacing with the compiler's common allocation.
