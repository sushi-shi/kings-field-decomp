# Minimal NONE2 SDK startup family, identified in functions_vendored.tsv:
# GAME 8003ac54..8003ac6b; OPEN 8001aa74..8001aa8b.
# Reconstructed assembly, not a preserved SDK source file or game progress.
# Include before SDK libraries. Current -G0 C inputs contribute no .sdata;
# this zero-sized anchor therefore names the start of that section.

	.sdata
__overlay_sdata:
	.text
	.set noreorder
	.globl __main
__main:
	jr $31
	nop
	.globl __SN_ENTRY_POINT
__SN_ENTRY_POINT:
	la $28,__overlay_sdata
	j main
	nop
