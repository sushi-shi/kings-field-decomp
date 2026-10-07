#ifndef KF_OPEN_STARTUP_LAYOUT_H
#define KF_OPEN_STARTUP_LAYOUT_H

/*
 * OPEN's startup memory map as compile-time numbers. Retail main folds these
 * and the counts derived from them into lui/ori immediates, so the original C
 * saw integers, not linker symbols. OPEN_BSS_START equals the retail .bss
 * start; OPEN_HEAP_START lies above all referenced static storage, but its
 * derivation is unresolved. The native build refreshes values that its own
 * link layout makes stale (docs/patterns/startup-address-origins.md).
 */
#define OPEN_BSS_START 0x800377a0u
#define OPEN_HEAP_START 0x80080100u

#endif
