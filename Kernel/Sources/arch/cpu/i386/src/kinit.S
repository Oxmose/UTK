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
MAGIC       equ 0xE85250D6
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

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------
extern kernel_kickstart
extern kernel_preboot

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
align 8
multiboot_header_start:
    dd MAGIC
    dd 0x00000000
    dd multiboot_header_end - multiboot_header_start
    dd -(MAGIC + (multiboot_header_end - multiboot_header_start))
align 8
    ; info request
    dw 1
    dw 0
    dd 4 * 2 + 8
    dd 3
    dd 6
align 8
    ; module align
    dw 6
    dw 0
    dd 8
; Multiboot terminate tag
align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:

section .low_startup_code
align 4
;-------------------------------------------------------------------------------
; Kernel entry point
__kinit:
    ; Make sure interrupts are disabled
    cli

    ; Save Multiboot info to memory
    mov dword [_kernel_multiboot_ptr], ebx

    ; Init pre-boot stack
    mov eax, _kernel_preboot_stack
    add eax, 0x1FC
    mov esp, eax
    mov ebp, esp

    ; Call pre-boot routines
    call kernel_preboot

    ; Map the higher half addresses
    mov eax, (_kinit_pgdir - KERNEL_MEM_OFFSET)
    mov cr3, eax

    ; Enable 4MB pages
    mov eax, cr4
    or  eax, 0x00000010
    mov cr4, eax

    ; Enable paging and write protect
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

_kernel_preboot_stack:
    resb 0x200

; Multiboot structues pointer
_kernel_multiboot_ptr:
    dq 0x0000000000000000

section .high_startup_code
align 4
; High memory loader
__kinit_high:
    ; Init stack
    mov eax, _KERNEL_STACKS_BASE
    mov ebx, KERNEL_STACK_SIZE - 4
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
    times (1024 - KERNEL_START_PAGE_ID - 4) dd 0  ; Pages after the kernel.

; Number of booted CPUs
_kernel_init_cpu_count:
    dd 0x00000000