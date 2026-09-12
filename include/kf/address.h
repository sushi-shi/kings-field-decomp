#ifndef KF_ADDRESS_H
#define KF_ADDRESS_H

/*
 * Retail claim for a reconstructed function: its start address and its retail
 * body size in bytes. The macro expands to nothing for the compiler;
 * scripts/kf/model.py reads the source text and binds the function that
 * follows it to that address in the unit's image, and the loader checks the
 * size against the admitted census so a claim cannot drift from the body it
 * names:
 *
 *     ADDRESS(0x8001499c, 0x38)
 *     void game_shutdown(void)
 *
 * Claims in one source must ascend, and a unit owns every admitted function
 * between its first and last claim, so a source file mirrors the linked order.
 */
#define ADDRESS(va, size)

/*
 * Per-image retail claim for a function that one source reconstructs into more
 * than one linked program. A shared source (under src/shared/) precedes each
 * definition with one ADDRESS_AT() per image it appears in, spelling that
 * image's start address and retail body size:
 *
 *     ADDRESS_AT("GAME", 0x800202fc, 0x68)
 *     ADDRESS_AT("OPEN", 0x80019598, 0x68)
 *     void matrix_interpolate(...)
 *
 * The source is listed as one unit per image in config/units.toml; model.py
 * binds each unit's functions to the ADDRESS_AT() addresses whose image matches
 * that unit, so the C text is written once while the addresses stay
 * image-qualified. A source may not mix ADDRESS_AT() with the name-rebinding
 * bind mode. Claims for a given image still ascend and cover a contiguous run.
 */
#define ADDRESS_AT(image, va, size)

/*
 * Retail claim for a global a unit owns: the datum's address and its curated
 * size. It precedes the DEFINITION (never an extern) and binds the declarator
 * to the matching data_identities.tsv row, so the module target object carries
 * the datum in .data (retail bytes, storage=load) or .bss (storage=bss):
 *
 *     DATA(0x80057b0c, 0x4)
 *     static u32 frame_pacer_vsync_count = 0;
 *
 * Data claims ascend inside a source and a datum belongs to one unit; every
 * other unit declares it extern. Load-image data must be explicitly
 * initialized in source: GCC 2.5.7 prints an uninitialized global as `.comm`,
 * which maspsx turns into `.bss`, so `= 0` is what places a zero word in .data
 * the way the retail image has it.
 */
#define DATA(va, size)

/*
 * Read-only contribution claim, at most one per translation unit: the retail
 * address range that holds this unit's switch jump tables and string
 * literals, in the order the compiler emitted them. The delinker carves it
 * into the module object's .rodata, turns entries that point into the unit's
 * own code into .text-relative words, and resolves the code's references to
 * .rodata offsets, so objdiff compares the tables and literals as well:
 *
 *     RODATA(0x8001235c, 0x148)
 */
#define RODATA(va, size)

#endif
