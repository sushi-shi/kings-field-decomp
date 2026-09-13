# CPE2X reserved header words

The pinned `CPE2X.EXE` has SHA-256
`8ee3df02d30d9269bba8c570d69f3c9d2b59aff98af0fbf796367526bc02ef20`.
Its original 16-bit header writer starts at MZ load-image offset `0x3250` and
passes its header to `fwrite` at `0x337c`. It leaves the two reserved words at
EXE file offsets `0x08..0x0f` untouched in its stack buffer.

Run the isolated writer control with:

```sh
nix develop -c python3 -m unittest tests.test_cpe2x_header -v
```

The control runs the original field stores and string copies with stack seeds
`0xa5` and `0x5a`. It intercepts diagnostic/file calls and captures the buffer
before `fwrite`; it does not modify the converter or produce a game executable.
The signature, entry point and load extent are written normally, while both
reserved words retain the supplied seed. This proves the writer's omission,
not the cause of a particular previous DOS stack value.

During source-export verification, two OPEN builds had identical executable
contents except for file bytes `0x08` and `0x09` (`4a 10` versus `22 08`).
The exporter therefore requires identical native CPE linker outputs and
compares all EXE bytes outside these two reserved words. Any difference in
them is reported separately as a failure of whole-file equality. Other header
fields, including GP and stack settings, remain strictly compared even though
the historical writer also leaves some of them unwritten.

This is a source-export equivalence check, not a retail matching gate. No
executable bytes are rewritten, no objdiff score is changed, and no result is
banked by the exporter.
