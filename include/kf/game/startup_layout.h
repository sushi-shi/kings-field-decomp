#ifndef KF_GAME_STARTUP_LAYOUT_H
#define KF_GAME_STARTUP_LAYOUT_H

/*
 * GAME's startup memory map, read from the program's own PSYLINK map.
 * GAME_BSS_START is the linked .bss section start and GAME_HEAP_START the
 * initial heap address above all static storage. Retail main folds both into
 * lui/ori immediates, so the original C saw numbers taken from a link map, not
 * linker symbols. The checked-in values are the retail link's. Each build
 * re-derives them from its own first link and relinks (scripts/psxbuild/
 * link.py): any layout change makes fixed values stale, and stale values
 * zero a live word and put the heap over this build's globals. How the retail heap start was chosen is unresolved
 * (docs/patterns/startup-address-origins.md).
 */
#define GAME_BSS_START 0x80058060u
#define GAME_HEAP_START 0x800a0980u

#endif
