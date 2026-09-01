#ifndef KF_ADDRESS_H
#define KF_ADDRESS_H

/*
 * Retail address claim for a reconstructed function. The macro expands to
 * nothing for the compiler; scripts/kf/model.py reads the source text and binds
 * the function that follows it to that address in the unit's image:
 *
 *     ADDRESS(0x8001499c)
 *     void game_shutdown(void)
 *
 * Claims in one source must ascend, and a unit owns every admitted function
 * between its first and last claim, so a source file mirrors the linked order.
 */
#define ADDRESS(va)

#endif
