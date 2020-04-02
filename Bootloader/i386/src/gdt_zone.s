;-------------------------------------------------------------------------------
; @file gdt_zone.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief GDT data stored in the boot stage
;
; @details GDT data stored in the boot stage
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

CODE32 equ 0x0008
DATA32 equ 0x0010
CODE16 equ 0x0018
DATA16 equ 0x0020

;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------
gdt16_:                                     ; GDT descriptor table
	.null_:
		dd 0x00000000
		dd 0x00000000

	.code_32_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	.data_32_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00

	.code_16_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0x0F
		db 0x00

	.data_16_:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0x0F
		db 0x00

gdt16_ptr_:                                ; GDT pointer for 16bit access
	dw gdt16_ptr_ - gdt16_ - 1             ; GDT limit
	dd gdt16_                              ; GDT base address
