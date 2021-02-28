;-------------------------------------------------------------------------------
;
; File: kinit.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 27/02/2021
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; INCLUDES
;-------------------------------------------------------------------------------
%include "config.inc"

;-------------------------------------------------------------------------------
; ARCH
;-------------------------------------------------------------------------------
[bits 32]

;-------------------------------------------------------------------------------
; DEFINES
;-------------------------------------------------------------------------------

; Multiboot header values
FLAGS       equ ((1<<0) | (1<<1))
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

;-------------------------------------------------------------------------------
; MACRO DEFINE
;-------------------------------------------------------------------------------
; Kernel memory layout 
KERNEL_START_PAGE_ID equ (KERNEL_MEM_OFFSET >> 22)
__kinit_low          equ (__kinit - KERNEL_MEM_OFFSET)

;-------------------------------------------------------------------------------
; EXTERN DATA
;-------------------------------------------------------------------------------
extern _START_BSS_ADDR
extern _END_BSS_ADDR
extern _KERNEL_STACKS_BASE
extern _KERNEL_MULTIBOOT_PTR
extern cpu_info

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------
extern kernel_kickstart

;-------------------------------------------------------------------------------
; EXPORTED FUNCTIONS
;-------------------------------------------------------------------------------
global __kinit
global __kinit_low

;-------------------------------------------------------------------------------
; EXPORTED DATA
;-------------------------------------------------------------------------------
global _kernel_multiboot_ptr
global _kernel_init_cpu_count

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

section .startup_code
align 4
;-------------------------------------------------------------------------------
; Kernel entry point
__kinit:
    ; Make sure interrupts are disabled
    cli

    ; Map the higher half addresses
    mov eax, (_kinit_pgdir - KERNEL_MEM_OFFSET)
    mov cr3, eax

    ; Enable 4MB pages
    mov eax, cr4
    or  eax, 0x00000010
    mov cr4, eax

    ; Enable paging
    mov eax, cr0
    or  eax, 0x80010000
    mov cr0, eax

    ; Init BSS
    mov  edi, _START_BSS_ADDR
    xor  esi, esi
__bss_init:
    mov  [edi], esi
    add  edi, 4
    cmp  edi, _END_BSS_ADDR
    jb   __bss_init

    ; Load high mem kernel entry point
    mov eax, __kinit_high
    jmp eax

; High memory loader 
__kinit_high:
    ; Save Multiboot info to memory
    mov [_kernel_multiboot_ptr], ebx

    ; Init stack
    mov eax, _KERNEL_STACKS_BASE
    mov ebx, KERNEL_STACK_SIZE
    add eax, ebx
    mov esp, eax 
    mov ebp, esp

    ; Clear flags
    push  0
    popfd

    ; Update the booted CPU count
    mov eax, 1
    mov [_kernel_init_cpu_count], eax

    ; Jump to kickstart
    jmp kernel_kickstart

__kinit_end:
    ; Disable interrupt and loop forever
    cli 
    hlt
    jmp __kinit_end

;-------------------------------------------------------------------------------
; CPUID probing function
__get_cpuid_flag:
    pushfd                                  ;Save EFLAGS
    pushfd                                  ;Store EFLAGS
    xor    dword [esp], 0x00200000          ;Invert the ID bit in stored EFLAGS
    popfd                                   ;Load stored EFLAGS
    pushfd                                  ;Store EFLAGS again
    pop    eax                              ;eax = modified EFLAGS
    xor    eax, [esp]                       ;eax = whichever bits were changed
    popfd                                   ;Restore original EFLAGS
    and    eax, 0x00200000                  ;eax = 0 CPUID is not available
    mov    edi, cpu_info
    or     [edi], eax                       ; Store information in the structure
    ret

;-------------------------------------------------------------------------------
; DATA
;-------------------------------------------------------------------------------
section .data 

;-------------------------------------------------------------------------------
; Kernel initial page directory: 
; 16MB mapped for high addresses
; 4MB mapped 1-1 for low addresses
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
    times (1024 - KERNEL_START_PAGE_ID - 4) dd 0  ; Pages after the kernel image.

; Multiboot structues pointer
_kernel_multiboot_ptr:
    dd 0x00000000

; Number of booted CPUs
_kernel_init_cpu_count:
    dd 0x00000000