;-------------------------------------------------------------------------------
; @file boot_sect_output.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, display message in real mode
;
; @details NASM boot code, display message sent throught BX register
; for the boot sector routines
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 16]

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
boot_sect_out_: ; Print string pointed by BX
	pusha	    ; Save registers

boot_sect_out_loop_:        ; Display loop
	mov cx, [bx]            ; Load character into cx
	cmp cl, 0		        ; Compare for NULL end
	je  boot_sect_out_end_  ; If NULL exit

	mov al, cl              ; Printf current character
	mov ah, 0x0e            ; SAME
	int 0x10                ; SAME

	add bx, 1			    ; Move to next character
	jmp boot_sect_out_loop_ ; Loop

boot_sect_out_end_:
	popa                    ; Restore registers
	ret

boot_sect_out_hex_:
	pusha                   ; Save registers              
	mov   dx, 0
	push  bx
	mov   bx, HEX_SEQ
	call  boot_sect_out_
	pop   bx

boot_sect_out_hex_loop_:
	cmp dx, 4
	je boot_sect_out_hex_end_

	mov cx, bx
	and cx, 0xF000
	shr cx, 12

	cmp cx, 9
	jle boot_sect_out_hex_num_

	add cx, 7

boot_sect_out_hex_num_:

	add cx, 48
	mov al, cl
	mov ah, 0x0e
	int 0x10

	shl bx, 4
	add dx, 1
	jmp boot_sect_out_hex_loop_

boot_sect_out_hex_end_:
	popa                   ; Restore registers
	ret

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------
HEX_SEQ:
	db "0x", 0
