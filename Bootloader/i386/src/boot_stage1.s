;-------------------------------------------------------------------------------
; @file boot_stage1.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, bootstrap the kernel and call the entry point.
;
; @details NASM boot code, bootstrap the kernel and call the entry point.
; Set flat mode GDT
; Set PM stack
; Switch to protected mode
; Call kernel loader
; Jump to kernel
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 16]
[org 0x8000]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

STACK_BASE_PM  equ 0x6C00 ; Kernel stack base
STACK_SIZE_PM  equ 0x4000 ; 16K kernel stack

LOADER_STAGE   equ 0xC000  ; Loader entry point

CONF_ADDR      equ 0x1000  ; Configuration entry point

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
boot_1_:
	; Save boot drive
	mov [BOOT_DRIVE], al

	push ax
	mov ax, 0x0 ; Reinit
	mov es, ax  ; memory
	mov ds, ax  ; segments
	pop ax

	; New line
	mov  bx, MSG_BOOTSTAGE1_ENDLINE
	call boot_sect_out_

	; Message from bootloader
	mov bx, MSG_BOOTSTAGE1_WELCOME
	call boot_sect_out_

	; Enable A20 gate
	call enable_a20_
	mov bx, MSG_BOOTSTAGE1_A20_ENABLER_PASS
	call boot_sect_out_

	; Set basic GDT
    lgdt [gdt16_ptr_]

	mov bx, MSG_BOOTSTAGE1_GDT_LOADED
	call boot_sect_out_

	; Detect hardware
	call detect_hardware_
	mov bx, MSG_BOOTSTAGE1_HW_DETECTED
	call boot_sect_out_

	; Set BIOS VGA cursor position for further print
	mov ah, 0x02
	mov bh, 0x00
	mov dx, 0x0E00
	int 0x10 

	; Set basic IDT
	lidt [idt_ptr_]

	; Set PMode
	mov eax, cr0
	inc eax
	mov cr0, eax

	jmp dword CODE32:boot_1_pm_  ; Jump to PM mode

; We should never get here
boot_1_halt_:
	hlt
	jmp boot_1_halt_

; Include 16bits assembly code
%include "src/a20_enabler.s"
%include "src/boot_sect_output.s"
%include "src/multiboot.s"

[bits 32]
boot_1_pm_:
	cli
	; Load segment registers
	mov  ax, DATA32
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	; Load stack
	mov esp, STACK_BASE_PM
	mov ebp, esp

	mov  eax, 9
	mov  ebx, 0
	mov  ecx, MSG_BOOTSTAGE1_IDT_LOADED
	call boot_sect_out_pm_

	mov  eax, 10
	mov  ebx, 0
	mov  ecx, MSG_BOOTSTAGE1_PM_WELCOME
	call boot_sect_out_pm_

	; Initialize interrupt 
	call interrupt_init_idt_

	; Initialize PIC
	call interrupt_init_pic_

	; Initialize PIT 
	call interrupt_init_pit_
	
	; Wait for autoboot
	call boot_1_autoboot_

	; Call kernel loader 
	mov  al, [BOOT_DRIVE] ; Save boot device ID
	call LOADER_STAGE

	; Jump to kernel, entry point is in eax
	call boot_sect_clear_screen_
	mov ebx, multiboot_info_
	jmp eax

; We should never get here
boot_1_halt_pm_:
	hlt
	jmp boot_1_halt_pm_

boot_1_autoboot_:
	pusha

	mov edx, 5
boot_1_wait_loop_:
	mov  eax, 13
	mov  ebx, 0
	mov  ecx, MSG_BOOTSTAGE1_AUTOBOOT
	call boot_sect_out_pm_

	mov  eax, 13
	mov  ecx, edx 
	add  ecx, 48
	mov  ebx, 9
	mov  [MSG_BOOTSTAGE1_VAR], cl
	mov  ecx, MSG_BOOTSTAGE1_VAR
	call boot_sect_out_pm_

	mov eax, 13
	mov ebx, 11
	mov ecx, CONF_ADDR + 16
	call boot_sect_out_pm_	

	mov eax, 1000
	call interrupt_pit_wait_
	dec edx 
	cmp edx, 0
	jne boot_1_wait_loop_

	popa 
	ret


%include "src/boot_sect_output_pm.s"

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

; Messages
MSG_BOOTSTAGE1_WELCOME: 
	db "Bootstage -> 1" , 0xA, 0xD, 0
MSG_BOOTSTAGE1_A20_ENABLER_PASS:
	db "[OK] A20 gate enabled", 0xA, 0xD, 0
MSG_BOOTSTAGE1_GDT_LOADED:
	db "[OK] 16 bits GDT loaded", 0xA, 0xD, 0
MSG_BOOTSTAGE1_HW_DETECTED:
	db "[OK] Hardware detected", 0xA, 0xD, 0
MSG_BOOTSTAGE1_IDT_LOADED:
	db "[OK] IDT loaded", 0
MSG_BOOTSTAGE1_PM_WELCOME: 
	db "[OK] Protected mode enabled", 0
MSG_BOOTSTAGE1_AUTOBOOT:
	db "Autoboot ", 0
MSG_BOOTSTAGE1_ENDLINE:
	db 0xA, 0xD, 0
MSG_BOOTSTAGE1_VAR:
	db 0, 0

; All boot settings are here
BOOT_DRIVE: db 0 ; Boot device ID

%include "src/gdt_zone.s"
%include "src/interrupt.s"

; Pad rest of the memory
times 16382-($-$$) db 0
dw 0xDEAD
