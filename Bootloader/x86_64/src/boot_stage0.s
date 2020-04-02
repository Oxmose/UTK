;-------------------------------------------------------------------------------
; @file boot_stage0.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, bootstrap the real bootloader
;
; @details NASM boot code, bootstrap the real bootloader
; Populate the memory with the actual bootloader
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 16]
[org 0x7C00] ; BIOS offset

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

BOOT_STAGE1           equ 0x8000  ; Next stage entry point
BOOT_STAGE1_SIZE      equ 32      ; Next stage sector size
BOOT_STAGE1_SECTOR    equ 1       ; Start sector of the stage 1

LOADER_STAGE          equ 0xC000  ; Loader entry point
LOADER_STAGE_SIZE     equ 32      ; Loader sector size
LOADER_STAGE_SECTOR   equ 33      ; Start sector of loader

CONF_ADDR             equ 0x1000  ; Configuration entry point
CONF_SIZE             equ 1       ; Next stage sector size
CONF_SECTOR           equ 65      ; Start sector of configuration

STACK_BASE_RM         equ 0x7C00  ; Stack base for real mode

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
; Get boot drive from BIOS
mov [BOOT_DRIVE], dl

; Clear screen
mov al, 0x03
mov ah, 0x00
int 0x10

; Invisible cursor
mov ah, 0x01
mov cx, 0x2607
int 0x10

; Set RM stack base
mov bp, STACK_BASE_RM   ; Set stack base
mov sp, bp              ; Set stack pointer

; Init segments
xor ax, ax
mov es, ax
mov ds, ax

; Canonical CS:EIP
jmp boot_0_

boot_0_:
	; Welcome message
	mov  bx, MSG_BOOTSTAGE0_WELCOME
	call boot_sect_out_

	; Disable interrupts
	cli

; Load the rest of the bootloader in the memory
boot_0_loader_:
	mov ax, BOOT_STAGE1_SIZE   ; Load the number of sectors to read
	mov [scnt], ax
	mov ax, 0
	mov [segn], ax
	mov ax, BOOT_STAGE1        ; Load the pointer
	mov [offn], ax

	mov ax, BOOT_STAGE1_SECTOR ; Load the start sector
	mov [sstart0], ax
	mov ax, 0
	mov [sstart1], ax
	mov [sstart2], ax
	mov [sstart3], ax
	
	mov dl, [BOOT_DRIVE]         ; Load the device to read from
	mov si, DAPACK

	call disk_load_ ; Load stage 1

	; Message load first part of the bootloader
	mov  bx, MSG_BOOTSTAGE0_LOADING1_BOOT
	call boot_sect_out_

	mov ax, CONF_SIZE           ; Load the number of sectors to read
	mov [scnt], ax
	mov ax, 0
	mov [segn], ax
	mov ax, CONF_ADDR           ; Load the pointer
	mov [offn], ax

	mov ax, CONF_SECTOR ; Load the start sector
	mov [sstart0], ax
	
	mov dl, [BOOT_DRIVE]         ; Load the device to read from
	mov si, DAPACK

	call disk_load_ ; Load loader

	; Message load loader part of the bootloader
	mov  bx, MSG_BOOTSTAGE0_LOADING2_BOOT
	call boot_sect_out_

	mov ax, LOADER_STAGE_SIZE   ; Load the number of sectors to read
	mov [scnt], ax
	mov ax, 0
	mov [segn], ax
	mov ax, LOADER_STAGE        ; Load the pointer
	mov [offn], ax

	mov ax, LOADER_STAGE_SECTOR ; Load the start sector
	mov [sstart0], ax

	mov dl, [BOOT_DRIVE]         ; Load the device to read from
	mov si, DAPACK

	call disk_load_ ; Load loader
	; Message load configuration of the bootloader
	mov  bx, MSG_BOOTSTAGE0_LOADING3_BOOT
	call boot_sect_out_

	; Go to boot stage 1
	mov  al, [BOOT_DRIVE] ; Save boot device ID
	call BOOT_STAGE1      ; Go to stage 1

; Halt function
boot_0_halt_:
	hlt
	jmp boot_0_halt_

%include "src/boot_sect_output.s" ; Output on screen
%include "src/hard_drive.s"       ; Load from disk

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

; All messages of the boot sequence are here
MSG_BOOTSTAGE0_WELCOME:
	db "Bootstage -> 0", 0xA, 0xD, 0
MSG_BOOTSTAGE0_LOADING1_BOOT:
	db "[OK] Loaded bootstage 1", 0xA, 0xD, 0
MSG_BOOTSTAGE0_LOADING2_BOOT:
	db "[OK] Loaded kloader", 0xA, 0xD, 0
MSG_BOOTSTAGE0_LOADING3_BOOT:
	db "[OK] Loaded configuration", 0xA, 0xD, 0
MSG_BOOTSTAGE0_ENDLINE:
	db 0xA, 0xD, 0

; All boot settings are here
BOOT_DRIVE: db 0 ; Boot device ID

align 2
DAPACK:
	     db 0x10 ; Packet length
		 db 0x00 ; Always 0
scnt:	 dw 0x0000 ; Sectors to read
offn:    dw 0x0000 ; Offset
segn:    dw 0x0000 ; Segment
sstart0: dw 0x0000 ; Start sector 15:0
sstart1: dw 0x0000 ; Start sector 31:16
sstart2: dw 0x0000 ; Start sector 47:32
sstart3: dw 0x0000 ; Start sector 63:47

; Boot sector padding
times 510-($-$$) db 0
dw 0xAA55
