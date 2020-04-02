
[bits 64]
[org 0x100000]
MODULEALIGN    equ 1<<0
MEMINFO        equ 1<<1

CODE32 equ 0x0008
DATA32 equ 0x0010

FLAGS       equ MODULEALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
  align 4
  multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

mov  rax, 17
mov  rbx, 0
mov  rcx, MSG_HELLO_64
call boot_sect_out_lm_	

; Go back to compatibility mode 
mov  rax, CODE32
push rax

xor  rcx, rcx
mov  ecx, compat_mode
push rcx

o64 retf

[bits 32]
compat_mode:
cli


mov  eax, 18
mov  ebx, 0
mov  ecx, MSG_HELLO_32
call boot_sect_out_pm_

entry:
    cli
    hlt
    jmp entry


%include "src/boot_sect_output_pm.s"

%include "src/boot_sect_output_lm.s"

MSG_HELLO_64: 
	db "Dummy Kernel Hello 64 bits", 0
MSG_HELLO_32: 
	db "Dummy Kernel Hello 32 bits", 0

times 188416-($-$$) db 0