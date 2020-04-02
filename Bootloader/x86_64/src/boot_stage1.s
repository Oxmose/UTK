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
; Enable flat paging
; Switch to 
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

	; Check for long mode availability
	call boot_1_check_lm_

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

	; Init paging for 64 bits mode 
	call x86_64_paging_init_

	; Enable paging and switch 64 bits mode 
	; Will also jump to kernel
	mov ebx, multiboot_info_
	jmp x86_64_switch_

; We should never get here
boot_1_halt_pm_:
	hlt
	jmp boot_1_halt_pm_

;-------------------------
; Wait for boot
;-------------------------

boot_1_autoboot_:
	pusha

	mov edx, 2
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

;-------------------------
; Check for long mode availability
;-------------------------
boot_1_check_lm_:
	push eax 
	push ebx 
	push ecx 
	push edx

	; 1) Check CPUID availability

	; Get flags
	pushfd 
	pop eax 

	; Update flags 
	mov  ebx, eax 
	xor  eax, 0x00200000
	push eax
	popfd 

	; Get flags
	pushfd 
	pop eax 

	; Update back flags 
	push ebx 
	popfd 

	; Compare eax and ecx, should have one bit flipped
	cmp eax, ecx
	mov ecx, MSG_BOOTSTAGE1_NO_CPUID
	je  boot_1_check_lm_error_


	; 2) Check CPUID extended features
	mov eax, 0x80000000
	cpuid 
	cmp eax, 0x80000001
	mov ecx, MSG_BOOTSTAGE1_NO_CPUID_EXT
	jb  boot_1_check_lm_error_


	; 3) Detect LM with CPUID
	mov eax, 0x80000001
	cpuid 
	and edx, 0x20000000
	cmp edx, 0
	mov ecx, MSG_BOOTSTAGE1_LM_NOT_SUPPORTED
	je boot_1_check_lm_error_

	pop edx 
	pop ecx 
	pop ebx 
	pop eax
	ret

boot_1_check_lm_error_:
	mov  eax, 12
	mov  ebx, 0
	call boot_sect_out_pm_
	jmp  boot_1_halt_pm_

%include "src/boot_sect_output_pm.s"
%include "src/interrupt.s"
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
MSG_BOOTSTAGE1_NO_CPUID:
	db "[ERROR] CPUID not supported", 0
MSG_BOOTSTAGE1_NO_CPUID_EXT:
	db "[ERROR] CPUID EXT supported", 0
MSG_BOOTSTAGE1_LM_NOT_SUPPORTED:
	db "[ERROR] 64Bits mode not supported, please use the 32Bits version", 0
MSG_BOOTSTAGE1_AUTOBOOT:
	db "Autoboot ", 0
MSG_BOOTSTAGE1_ENDLINE:
	db 0xA, 0xD, 0
MSG_BOOTSTAGE1_VAR:
	db 0, 0

; All boot settings are here
BOOT_DRIVE: db 0 ; Boot device ID

%include "src/gdt_zone.s"

; This uses 64 bits directive
%include "src/x86_64_manager.s"

; Pad rest of the memory
times 16382-($-$$) db 0
dw 0xDEAD
