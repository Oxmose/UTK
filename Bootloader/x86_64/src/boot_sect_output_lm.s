;-------------------------------------------------------------------------------
; @file boot_sect_output_lm.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, display message sent throught rcx register
;
; @details NASM boot code, display message sent throught rcx register
; Display message in LM mode using VGA buffer
; Line is contained in rax, column is contained in rbx
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 64]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------
VGA_BUFFER equ 0x00000000000B8000

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
boot_sect_out_lm_: ; Print string pointed by BX
	push rax	   ; Save registers
	push rbx
	push rcx 
	push rdx 

	; Compute start address
	mov rdx, 160
	mul rdx      ; rax now contains the offset in lines
	add rax, rbx ; Offset is half complete
	add rax, rbx ; Offset is complete

	add rax, VGA_BUFFER ; VGA buffer address
	mov rbx, rcx

boot_sect_out_lm_loop_:      ; Display loop
	mov cl, [rbx]            ; Load character into cx
	cmp cl, 0		         ; Compare for NULL end
	je boot_sect_out_lm_end_ ; If NULL exit
	mov ch, 0x0007
	mov [rax], cx             ; Printf current character
	add rax, 2                ; Move cursor

	add rbx, 1			       ; Move to next character
	jmp boot_sect_out_lm_loop_ ; Loop

boot_sect_out_lm_end_:
	pop rax 
	pop rbx 
	pop rcx
	pop rdx ; Restore registers
	ret

boot_sect_clear_screen_lm_:
	push rax	   ; Save registers
	push rbx
	push rcx 
	push rdx 

	mov rax, VGA_BUFFER
	mov rbx, 80
	mov rcx, 24
	mov rdx, 0x0
boot_sect_clear_screen_lm_loop_:
	mov [rax], rdx
	sub rbx, 2
	add rax, 4
	cmp rbx, 0
	jne boot_sect_clear_screen_lm_loop_
	mov rbx, 80
	dec rcx
	cmp rcx, 0
	jne boot_sect_clear_screen_lm_loop_
		
	pop rax 
	pop rbx 
	pop rcx
	pop rdx

	ret
