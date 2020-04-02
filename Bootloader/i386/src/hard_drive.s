;-------------------------------------------------------------------------------
; @file hard_drive.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief ASM boot code, load sector form disk into memory.
;
; @details NASM boot code, load sector form disk into memory.
; Used to load the kernel into memory.
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 16]

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------

; Load from disk
disk_load_:

	; ah contains the number of sectors to read
	; ds:si contain the DAP
	; dl contains the device to read

	mov ah, 0x42 ; Set BIOS disk read function
	int 0x13     ; Issue read
	jc  disk_load_error_ ; Jump on error (carry)
	mov bx, ax 
	and bx, 0xFF00
	cmp bx, 0
	jne disk_read_error_
	ret

disk_read_error_:
	mov  bx, MSG_DISK_READ_ERROR ; Error
	call boot_sect_out_          ; Output
	; Display error
	mov bx, ax
	call boot_sect_out_hex_
	jmp  hdd_halt_				 ; Halt system

disk_load_error_:
	mov  bx, MSG_DISK_LOAD_ERROR ; Display error
	call boot_sect_out_          ; Output
	; Display error
	mov bx, ax
	call boot_sect_out_hex_
	jmp  hdd_halt_               ; Halt the system

hdd_halt_:
	hlt 
	jmp hdd_halt_

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------
MSG_DISK_READ_ERROR:
	db "[ERROR] Can't load required amount of sector", 0xA, 0xD, 0
MSG_DISK_LOAD_ERROR:
	db "[ERROR] Can't issue load from disk", 0xA, 0xD, 0
