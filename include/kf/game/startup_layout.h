#ifndef KF_GAME_STARTUP_LAYOUT_H
#define KF_GAME_STARTUP_LAYOUT_H

/*
 * GAME's startup memory map as compile-time numbers. Retail main folds these
 * and the counts derived from them into lui/ori immediates, so the original C
 * saw integers, not linker symbols. GAME_BSS_START equals the retail .bss
 * start; GAME_HEAP_START lies above all referenced static storage, but its
 * derivation is unresolved. The native build refreshes values that its own
 * link layout makes stale (docs/patterns/startup-address-origins.md).
 */
#define GAME_BSS_START 0x80058060u
#define GAME_HEAP_START 0x800a0980u

#endif
