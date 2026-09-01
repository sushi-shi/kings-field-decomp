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

#endif
