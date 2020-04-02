;-------------------------------------------------------------------------------
; @file boot_sect_output_pm.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, display message sent throught ECX register
;
; @details NASM boot code, display message sent throught ECX register
; Display message in PM mode using VGA buffer
; Line is contained in eax, column is contained in ebx
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------
VGA_BUFFER equ 0x000B8000

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
boot_sect_out_pm_: ; Print string pointed by BX
	pusha	   ; Save registers

	; Compute start address
	mov edx, 160
	mul edx      ; eax now contains the offset in lines
	add eax, ebx ; Offset is half complete
	add eax, ebx ; Offset is complete

	add eax, VGA_BUFFER ; VGA buffer address
	mov ebx, ecx

boot_sect_out_pm_loop_:      ; Display loop
	mov cl, [ebx]            ; Load character into cx
	cmp cl, 0		         ; Compare for NULL end
	je boot_sect_out_pm_end_ ; If NULL exit
	mov ch, 0x0007
	mov [eax], cx             ; Printf current character
	add eax, 2                ; Move cursor

	add ebx, 1			       ; Move to next character
	jmp boot_sect_out_pm_loop_ ; Loop

boot_sect_out_pm_end_:
	popa ; Restore registers
	ret

boot_sect_clear_screen_:
	pusha 
	mov eax, VGA_BUFFER
	mov ebx, 80
	mov ecx, 24
	mov edx, 0x0
boot_sect_clear_screen_loop_:
	mov [eax], edx
	sub ebx, 2
	add eax, 4
	cmp ebx, 0
	jne boot_sect_clear_screen_loop_
	mov ebx, 80
	dec ecx
	cmp ecx, 0
	jne boot_sect_clear_screen_loop_
		
	popa 
	ret
