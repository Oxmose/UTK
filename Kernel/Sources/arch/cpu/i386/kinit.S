;-------------------------------------------------------------------------------
;
; File: kinit.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 14/12/2017
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
; ARCH
;-------------------------------------------------------------------------------
[bits 32]

;-------------------------------------------------------------------------------
; DEFINES
;-------------------------------------------------------------------------------

; Multiboot header values
FLAGS       equ (1<<0) | (1<<1)
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

; Kernel memory layout 
KERNEL_MEM_OFFSET    equ 0xE0000000 ; WARNING This value should be updated to 
                                    ; fit other configuration files
KERNEL_STACK_SIZE    equ 0x2000 ; WARNING This value should be updated to fit 
                                ; other configuration files
KERNEL_START_PAGE_ID equ (KERNEL_MEM_OFFSET >> 22)
__kinit_low          equ (__kinit - KERNEL_MEM_OFFSET)



;-------------------------------------------------------------------------------
; MACRO DEFINE
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; EXTERN DATA
;-------------------------------------------------------------------------------
extern cpu_stacks
extern init_cpu_count

extern _kernel_bss_start
extern _kernel_bss_end

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------

extern serial_init
extern cpu_setup_gdt
extern cpu_setup_idt
extern cpu_setup_tss

;-------------------------------------------------------------------------------
; EXPORTED FUNCTIONS
;-------------------------------------------------------------------------------
global __kinit

;-------------------------------------------------------------------------------
; CODE
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    ; In case it is not grub that loaded us
    mov eax, __kinit_low
    jmp eax

section .text
align 4
;-------------------------------------------------------------------------------
; Kernel entry point
__kinit:
    ; Make sure interrupts are disabled and clear flags
    push  0
    popfd

    ; Map the higher half addresses
    mov eax, (_kinit_pgdir - KERNEL_MEM_OFFSET)
    mov cr3, eax

    ; Enable 4MB pages
    mov eax, cr4
    or  eax, 0x00000010
    mov cr4, eax

    ; Enable paging
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; Load high mem kernel entry point
    mov eax, __kinit_high
    jmp eax

; High memory loader 
__kinit_high:
    ; Init stack
    mov eax, cpu_stacks
    add eax, KERNEL_STACK_SIZE
    mov esp, eax 
    mov ebp, esp

    ; Init BSS
    mov  edi, _kernel_bss_start
    xor  esi, esi
__bss_init:
    mov  [edi], esi
    add  edi, 4
    cmp  edi, _kernel_bss_end
    jb   __bss_init

    ; Update the booted CPU count
    mov eax, 1
    mov [init_cpu_count], eax

    ; Setup system pre-kickstart
    call serial_init
    call cpu_setup_gdt
    call cpu_setup_idt
    call cpu_setup_tss

    ; Kickstart kernel


__kinit_end:
    ; Disable interrupt and loop forever
    cli 
    hlt
    jmp __kinit_end

   
;-------------------------------------------------------------------------------
; DATA
;-------------------------------------------------------------------------------

section .data

;-------------------------------------------------------------------------------
; Kernel initial page directory

align 0x1000
_kinit_pgdir:
    ; First 4MB R/W Present.
    dd 0x00000083
    ; Pages before kernel space.
    times (KERNEL_START_PAGE_ID - 1) dd 0         
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00800083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00C00083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x01000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x01400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x018000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x01C00083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x02000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x02400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x02800083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x02C00083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x0300083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x03400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x03800083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x03C00083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x04000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x04400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x04800083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x04C00083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x05000083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x05400083
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x05800083
    times (1024 - KERNEL_START_PAGE_ID - 23) dd 0  ; Pages after the kernel image.