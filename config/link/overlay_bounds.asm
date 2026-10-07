; Zero-byte labels for the overlay's linked .bss boundaries.

	section .bss_start
	xdef BSS_START
BSS_START:

	section .bss_end
	xdef BSS_END
BSS_END:
