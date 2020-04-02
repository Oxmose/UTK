;-------------------------------------------------------------------------------
; @file loader.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 15/03/2020
;
; @version 1.0
;
; @brief NASM boot code, load the kernel thanks to INT13H
;
; @details NASM boot code, load the kernel thanks to INT13H
; Jump from PM to RM
; Copy kernel parts from 0x1_FC00 to 0x9_FC00
; Jump from RM to PM
; Copy kernel to high memory
; Loop until done
;
; @warning Paging should be disabled, GDT and IDT should be flat 1:1 mapped.
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]
[org 0xC000]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

CONF_ADDR          equ 0x1000  ; Configuration entry point

START_SECTOR       equ 66      ; First sector of the kernel

COPY_REGION_START      equ 0x10000    ; Copy region base
COPY_REGION_END        equ 0x80000    ; 512K copy region size
COPY_REGION_SEGOFF     equ 0x10000000 ; Copy region base segment:offset
COPY_REGION_SEGOFF_END equ 0x80000000 ; Copy region base segment:offset

STACK_BASE_RM  equ 0x7C00  ; Stack base for real mode

MEM_DETECT_POINTER equ 0x80000

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
loader_:
	pusha 
	mov [save_stack_], esp

	; Save boot drive
	mov [BOOT_DRIVE], al

	; Read from configuration
	mov eax, [CONF_ADDR]
	mov [toload_], eax 
	mov eax, [CONF_ADDR + 4]
	mov [curraddr_], eax 

	mov  eax, 13
	mov  ebx, 0
	mov  ecx, MSG_LOADER_WELCOME
	call boot_sect_out_pm_	

	mov eax, 13
	mov ebx, 10
	mov ecx, CONF_ADDR + 16
	call boot_sect_out_pm_	

loader_prepare_rm_:
	; Ensure interrupt are disabled 
	cli

	; Ensure paging is disabled 
	mov eax, cr0
	and eax, 0x7FF7FFFF
	mov cr0, eax

	; Save stack 
	mov [save_stack_], esp

	; Far jump to 16 bit PM
	jmp word CODE16:loader_16b_pm_entry_

[bits 16]
loader_16b_pm_entry_:
	; Loader segment registers
	mov ax, DATA16
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Load RM IDT
	lidt [ivt_ptr_]

	; Disable PM
	mov eax, cr0
	and eax, 0xFFFFFFFE
	mov cr0, eax

	; Far jump to 16 bit RM
	jmp word 0x0000:loader_rm_entry_

loader_rm_entry_:
	; Loader segment registers
	mov ax, 0x0000
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Set RM stack base
	mov bp, STACK_BASE_RM  
	mov sp, bp  
	
	; Enable interrupts
	sti 	

loader_loadblock_:
	; Are we done loading
	mov eax, [toload_]
	cmp eax, 0x0 
	je  loader_copy_high_

	; Get at most 64 sectors
	cmp eax, 65
	jb  loader_sect_set_
	mov eax, 64

loader_sect_set_:
	mov [scnt], ax

	mov eax, [loaded_]
	mov [sstart0], eax
	
	mov dl, [BOOT_DRIVE]         ; Load the device to read from
	mov si, DAPACK

	call disk_load_ ; Load

	mov eax, 0x1 
	mov [do_cpy_], eax

	; Update copied blocks 
	mov bx,  [scnt]
	and ebx, 0x000000FF
	mov eax, [loaded_]
	add eax, ebx
	mov [loaded_], eax    ; Update next sector to load
	mov eax, [toload_]
	sub eax, ebx
	mov [toload_], eax    ; update the number of sectors to load

	; Can we load more in low memory
	mov eax, [offseg]
	add eax, 0x00008000
	mov ebx, eax 
	and ebx, 0x000FFFFF
	cmp ebx, 0x00010000
	jb  loader_add_seg_end_
	and eax, 0xF000FFFF
	add eax, 0x10000000

loader_add_seg_end_:

	cmp eax, COPY_REGION_SEGOFF_END
	jae loader_copy_high_

	; Update the target memory 
	mov [offseg], eax 

	; Load new chunk
	jmp loader_loadblock_

loader_copy_high_:
	; Switch to PM
	mov ax, 0x0 ; Reinit
	mov es, ax  ; memory
	mov ds, ax  ; segments

	; Set basic IDT
	lidt [idt_ptr_]

	; Set PMode
	mov eax, cr0
	inc eax
	mov cr0, eax

	jmp dword CODE32:loader_copy_high_pm_  ; Jump to PM mode

[bits 32]
loader_copy_high_pm_:
	cli 

	; Load segment registers
	mov  ax, DATA32
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	; Load stack
	mov esp, [save_stack_]
	mov ebp, esp

	mov eax, [do_cpy_]
	cmp eax, 0
	je loader_end_load_

	; Relocate code
	mov  esi, COPY_REGION_START                     ; Source address
	mov  edi, [curraddr_]                           ; Destination address
	mov  ecx, (COPY_REGION_END - COPY_REGION_START) ; Size

	cld                                ; Clear direction flag
	rep  movsb                         ; Copy the code to low memory space

	; Clear buffer (optional if performance issues)
	xor eax, eax 
	mov eax, COPY_REGION_START
	mov ecx, COPY_REGION_END
loader_blank_loop_:
	mov byte [eax], 0x00
	inc eax
	cmp ecx, eax
	jne loader_blank_loop_


	; Update loaded address
	mov eax, [curraddr_]
	add eax, (COPY_REGION_END - COPY_REGION_START)
	mov [curraddr_], eax

	; Update the target memory 
	mov eax, COPY_REGION_SEGOFF
	mov [offseg], eax 

	mov eax, 0x0 
	mov [do_cpy_], eax

	; Are we done loading
	mov  eax, [toload_]
	cmp  eax, 0x0 

	jne  loader_prepare_rm_

loader_end_load_:
	; Seach for multiboot header in kernel (in 8 first K)
	mov eax, [CONF_ADDR + 4]
	mov ecx, eax 
	add ecx, 0x2000
loader_search_kernel_:
	mov ebx, [eax]
	cmp ebx, 0x1BADB002
	je  loader_boot_kernel_
	add eax, 4
	cmp eax, ecx 
	je  loader_end_load_halt_
	jmp loader_search_kernel_
	
loader_boot_kernel_:
	mov [kernel_entry_], eax
	popa 
	mov eax, [kernel_entry_]
	add eax, 12
	ret 

loader_end_load_halt_:
	cli
	mov  eax, 16
	mov  ebx, 0
	mov  ecx, MSG_LOADER_NO_KERNEL
	call boot_sect_out_pm_	
	hlt 
	jmp loader_end_load_halt_

[bits 16]
loader_rm_hlt_:          
	cli 
	hlt 
	jmp loader_rm_hlt_

; 16 Bits code 
%include "src/boot_sect_output.s"
%include "src/hard_drive.s"       

; 32 Bits code 
[bits 32]
%include "src/gdt_zone.s"
%include "src/idt_zone.s"
%include "src/boot_sect_output_pm.s"

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

; Messages
MSG_LOADER_WELCOME: 
	db "Loader -> ", 0
MSG_LOADER_ENDLINE:
	db 0xA, 0xD, 0
MSG_LOADER_POINT:
	db ".", 0
MSG_LOADER_NO_KERNEL: 
	db "No kernel found, halting.", 0
; Real Mode IVT
ivt_ptr_:                          
	dw 0x03FF ; IVT size
	dd 0x0000 ; IVT base

; Loader data
loaded_ :   
	dd START_SECTOR 
toload_ :   
	dd 0x0 
curraddr_ : 
	dd 0x0
do_cpy_:
	dd 0x0


save_stack_:
  dd 0x00000000	

; All boot settings are here
BOOT_DRIVE: db 0 ; Boot device ID

kernel_entry_:
	dd 0x00000000

; LBA packet
align 4
DAPACK:
	     db 0x10 ; Packet length
		 db 0x00 ; Always 0
scnt:	 dw 0x0000 ; Sectors to read
offseg:  dd COPY_REGION_SEGOFF ; Offset
sstart0: dw 0x0000 ; Start sector 15:0
sstart1: dw 0x0000 ; Start sector 31:16
sstart2: dw 0x0000 ; Start sector 47:32
sstart3: dw 0x0000 ; Start sector 63:47

times 16382-($-$$) db 0
dw 0xCAFE
