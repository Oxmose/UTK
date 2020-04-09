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
FLAGS       equ ((1<<0) | (1<<1))
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

; Kernel memory layout 
KERNEL_MEM_OFFSET    equ 0xE0000000 ; WARNING This value should be updated to 
                                    ; fit other configuration files
KERNEL_STACK_SIZE    equ 0x4000 ; WARNING This value should be updated to fit 
                                ; other configuration files
KERNEL_START_PAGE_ID equ (KERNEL_MEM_OFFSET >> 22)
__kinit_low          equ (__kinit - KERNEL_MEM_OFFSET)

; Kernel's 32 bits code and data segment descriptor
KERNEL_CS_32 equ 0x08
KERNEL_DS_32 equ 0x10

CODE_LOCATION equ 0x4000
%define OFFSET_ADDR(addr)  (((addr) - __ap_loader_start) + CODE_LOCATION)

;-------------------------------------------------------------------------------
; MACRO DEFINE
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; EXTERN DATA
;-------------------------------------------------------------------------------
extern cpu_stacks
extern cpu_stack_size

extern cpu_info
extern init_cpu_count

extern multiboot_data_ptr

extern _kernel_bss_start
extern _kernel_bss_end

extern cpu_gdt_ptr
extern cpu_idt_ptr

extern init_cpu_count

extern kernel_pgdir

;-------------------------------------------------------------------------------
; EXTERN FUNCTIONS
;-------------------------------------------------------------------------------

extern serial_init
extern cpu_setup_gdt
extern cpu_setup_idt
extern cpu_setup_tss
extern kernel_kickstart
extern cpu_ap_core_init

;-------------------------------------------------------------------------------
; EXPORTED FUNCTIONS
;-------------------------------------------------------------------------------
global __kinit
global __cpu_smp_loader_init

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
    or  eax, 0x80010000
    mov cr0, eax

    ; Init BSS
    mov  edi, _kernel_bss_start
    xor  esi, esi
__bss_init:
    mov  [edi], esi
    add  edi, 4
    cmp  edi, _kernel_bss_end
    jb   __bss_init

    ; Load high mem kernel entry point
    mov eax, __kinit_high
    jmp eax

; High memory loader 
__kinit_high:
    ; Save Multiboot info to memory
    mov [multiboot_data_ptr], ebx

    ; Init stack
    mov eax, cpu_stacks
    add eax, KERNEL_STACK_SIZE
    mov esp, eax 
    mov ebp, esp

    ; Update the booted CPU count
    mov eax, 1
    mov [init_cpu_count], eax

    ; Setup system pre-kickstart
    call __get_cpuid_flag
    call serial_init
    call cpu_setup_gdt
    call cpu_setup_idt
    call cpu_setup_tss

    ; Kickstart kernel
    call kernel_kickstart

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
; Loader initialization
__cpu_smp_loader_init:
    push esi 
    push edi 
    push ecx 

    ; Relocate boot code to low memory space

    mov  esi, __ap_loader_start                     ; Source address
    mov  edi, CODE_LOCATION                         ; Destination address
    mov  ecx, (__ap_loader_end - __ap_loader_start) ; Size

    cld                                ; Clear direction flag
    rep  movsb                         ; Copy the code to low memory space
    
    pop ecx 
    pop esi 
    pop edi        
    ret

;-------------------------------------------------------------------------------
; AP CPU bootstrap code
[bits 16]
__ap_loader_start:

    cli
    ; Set the GDT for the current CPU
    lgdt [OFFSET_ADDR(gdt16_ptr)]

    ; Set PM
    mov eax, cr0
    ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    or eax, 0x1
    mov cr0, eax

    jmp  dword KERNEL_CS_32:OFFSET_ADDR(__ap_loader_pm)  ; Jump to PM mode

[bits 32]
__ap_loader_pm:

    ; Init segment registers
    mov ax, KERNEL_DS_32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set CR3
    mov eax, kernel_pgdir
    add eax, -KERNEL_MEM_OFFSET
    mov cr3, eax

    ; Enable paging and set correct CR0
    mov eax, 0x80010001
    mov cr0, eax

    ; Get stack index
    mov ebx, [init_cpu_count]
    add ebx, 1

    ; Now set the cpu stack
    mov eax, [cpu_stack_size]
    mul ebx

    ; Init stack pointer
    mov ebx, cpu_stacks
    add eax, ebx
    mov esp, eax

    ; Init base pointer
    mov ebp, esp

    ; Init CPU IDT
    lidt [cpu_idt_ptr]

    ; Init CPU GDT
    lgdt [cpu_gdt_ptr]

    mov eax, cpu_ap_core_init
    call eax

__ap_loader_error:
    ; Disable interrupt and loop forever
    cli 
    hlt
    jmp __ap_loader_error

    gdt16:                                     ; GDT descriptor table
        .null:
            dd 0x00000000
            dd 0x00000000

        .code_32:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x9A
            db 0xCF
            db 0x00

        .data_32:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x92
            db 0xCF
            db 0x00

        .code_16:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x9A
            db 0x0F
            db 0x00

        .data_16:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x92
            db 0x0F
            db 0x00

    gdt16_ptr:                                 ; GDT pointer for 16bit access
        dw gdt16_ptr - gdt16 - 1               ; GDT limit
        dd gdt16                               ; GDT base address

__ap_loader_end:
    
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