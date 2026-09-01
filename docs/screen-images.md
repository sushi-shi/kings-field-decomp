# Full-screen and TALK images

`screen_show_image_until_input` (`GAME.EXE 0x8002c794`) CD-loads an arbitrary
TIM path into `asset_load_buffer`, uploads it, draws a full-screen
semitransparent `POLY_FT4`, fades its RGB value from `0x20` toward `0x7f`, and
waits for one active-low pad press/release cycle. Its callers use it for TALK
images and other full-screen prompts, so the identity is intentionally not
dialogue-specific.

`talk_show_indexed_image` (`0x8002c9d4`) patches the mutable loaded string
`talk_image_path_template` (`TALK\\C00\\T00000.TIM`) before invoking that
generic screen routine. The four call sites prove byte-sized path digits and a
signed group value divided into decimal tens/ones. Narrative meanings for the
individual digits are not yet proven, so the signature names path positions
instead of inventing chapter or speaker semantics.

The render state also resolves two globals that were previously isolated byte
candidates: `display_draw_environments` is two adjacent Psy-Q `DRAWENV`
records of `0x5c` bytes each, selected with `display_buffer_index`. Embedded
`dfe`, `isbg`, and background-color writes are fields of those records, not
separate globals.

Per-function evidence is in
`config/evidence/game_semantic_screen_talk.tsv`.
