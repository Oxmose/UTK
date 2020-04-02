
[bits 32]
[org 0x100000]
MODULEALIGN    equ 1<<0
MEMINFO        equ 1<<1

FLAGS       equ MODULEALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
  align 4
  multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

mov  eax, 17
mov  ebx, 0
mov  ecx, MSG_HELLO
call boot_sect_out_pm_	

entry:
    hlt
    jmp entry

%include "src/boot_sect_output_pm.s"

MSG_HELLO: 
	db "Dummy Kernel Hello", 0

times 188416-($-$$) db 0