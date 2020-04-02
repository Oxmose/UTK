;-------------------------------------------------------------------------------
; @file x86_64_manager.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 21/03/2020
;
; @version 1.0
;
; @brief NASM boot code, sets the page table at 0x80000, enable paging and 
; switch to long mode
;
; @details NASM boot code, sets the page table at 0x80000 and enable paging and 
; switch to long mode.
; The first GB of memory is mapped 1:1
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

X86_64_PAGING_PML4T_ADDR equ 0x80000
X86_64_PAGING_PDPT0_ADDR equ 0x81000
X86_64_PAGING_PDT0_ADDR  equ 0x82000
X86_64_PAGING_PDT1_ADDR  equ 0x83000

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------

;-------------------------
; Initializes the paging structures
;-------------------------
x86_64_paging_init_:
	push eax
	push ebx

	; Set the PML4T
	mov ebx, X86_64_PAGING_PDPT0_ADDR ; First entry to PDPT0
	or  ebx, 0x3                      ; Present and writable
	mov eax, X86_64_PAGING_PML4T_ADDR
	mov [eax], ebx 

	; Blank the rest of the PML4T
	xor ebx, ebx
x86_64_paging_init_blank_pml4t_:
	add eax, 8
	cmp eax, X86_64_PAGING_PDPT0_ADDR
	mov [eax], ebx
	jne x86_64_paging_init_blank_pml4t_

	; Set the PDPT0
	mov ebx, X86_64_PAGING_PDT0_ADDR  ; First entry to PDT0
	or  ebx, 0x3                      ; Present and writable
	mov eax, X86_64_PAGING_PDPT0_ADDR
	mov [eax], ebx 

	; Blank the rest of the PDPT0
	xor ebx, ebx
x86_64_paging_init_blank_pdpt0_:
	add eax, 8
	cmp eax, X86_64_PAGING_PDT0_ADDR
	mov [eax], ebx
	jne x86_64_paging_init_blank_pdpt0_
	
	; Set the first 512 PDT0
	mov ebx, 0x83 ; Present, writable, 2MB page
	mov eax, X86_64_PAGING_PDT0_ADDR

x86_64_paging_init_blank_pdt0_:
	mov [eax], ebx 
	add eax, 8
	add ebx, 0x200000
	cmp eax, X86_64_PAGING_PDT1_ADDR
	jne x86_64_paging_init_blank_pdt0_

	pop ebx 
	pop eax	
	ret


;-------------------------
; Long mode switch
;-------------------------
x86_64_switch_:
	mov [x86_64_manager_kernel_entry_], eax 
	mov [x86_64_manager_multiboot_ptr_], ebx

	; Set CR3
	mov eax, X86_64_PAGING_PML4T_ADDR
	mov cr3, eax

	; Enable PAE
	mov eax, cr4 
	or  eax, 0x20
	mov cr4, eax 

	; Switch to compatibility mode 
	mov ecx, 0xC0000080
	rdmsr
	or  eax, 0x00000100
	wrmsr

	; Enable paging 
	mov eax, cr0
	or  eax, 0x80000000
	mov cr0, eax 

	; Far jump to 64 bit mode
	jmp CODE64:x86_64_switch_lm_entry_

[bits 64]
x86_64_switch_lm_entry_:
	cli
	mov rax, 0x00000000FFFFFFFF
	and rsp, rax

	; Jump to kernel, entry point is in rax
	call boot_sect_clear_screen_lm_
	mov rax, [x86_64_manager_kernel_entry_]
	mov rbx, [x86_64_manager_multiboot_ptr_]
	jmp  rax

x86_64_switch_lm_halt_:
	cli 
	hlt 
	jmp x86_64_switch_lm_halt_

%include "src/boot_sect_output_lm.s"

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

x86_64_manager_multiboot_ptr_:
	dq 0x0000000000000000

x86_64_manager_kernel_entry_:
	dq 0x0000000000000000