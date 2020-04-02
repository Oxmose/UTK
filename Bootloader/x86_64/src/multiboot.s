;-------------------------------------------------------------------------------
; @file multiboot.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 1.0
;
; @brief ASM boot code, detect hardware.
;
; @details NASM boot code, detect hardware and fills a multiboot compatible 
; structure.
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------
[bits 16]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------
MEM_DETECT_POINTER equ 0x1400

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------

detect_hardware_:
	push eax

	; Detect memory map
	call detect_memory_
	
	; Add boot devices 
	call detect_boot_device_

	; Update the flags  (loader name)
	mov eax, [multiboot_flags_]
	or  eax, 0x200
	mov [multiboot_flags_], eax

	pop eax
	ret

; ---------------------------------------

detect_boot_device_:
	push eax

	; Save boot device 
	mov eax, [BOOT_DRIVE]
	and eax, 0x000000FF
	mov [multiboot_boot_dev_], eax 

	; Update the flags 
	mov eax, [multiboot_flags_]
	or  eax, 0x2
	mov [multiboot_flags_], eax

	pop eax
	ret

; ---------------------------------------

; Detect memory
detect_memory_:
	pusha
	
	; Prepare for 0xE820 function
	mov di,  mem_buffer_
	xor ax,  ax
	mov es,  ax
	xor ebx, ebx

	mov eax, MEM_DETECT_POINTER
	mov [multiboot_mmap_addr_], eax

detect_memory_call_:
	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x534D4150
	int 0x15

	; Check error 
	jc  detect_memory_error_
	cmp eax, 0x534D4150
	jne detect_memory_error_

	; Check if we read all the list 
	mov eax, [first_loop_]
	cmp eax, 1
	je  detect_memory_call_next_
	cmp ebx, 0
	je  detect_memory_end_
	
detect_memory_call_next_:

	xor eax, eax
	mov [first_loop_], eax 

	push ebx

	; Copy entry size
	mov eax, [multiboot_mmap_length_]
	add eax, MEM_DETECT_POINTER
	mov ebx, 20                       ; Multiboot has 20Bytes entries
	mov [eax], ebx
	add eax, 0x4

	; Check entry type
	mov ebx, [mem_buffer_ + 16]
	cmp ebx, 1
	jne detect_memory_type_nonusable_

	push eax
	mov eax, [mem_buffer_ + 8]
	call detect_memory_increment_
	pop eax

detect_memory_type_nonusable_:
	; Copy entry start address
	mov ebx, mem_buffer_
	mov edx, 0
detect_memory_copy_buffer_:
	cmp edx, 20
	je detect_memory_copy_buffer_end_

	mov ecx, [ebx]
	mov [eax], ecx
	inc eax
	inc ebx
	inc edx
	jmp detect_memory_copy_buffer_
detect_memory_copy_buffer_end_:

	pop ebx

	; Update new length
	sub eax, MEM_DETECT_POINTER
	mov [multiboot_mmap_length_], eax	

	jmp detect_memory_call_

detect_memory_end_:
	; Update the flags 
	mov eax, [multiboot_flags_]
	or  eax, 0x41
	mov [multiboot_flags_], eax

	popa
	ret

; ---------------------------------------

detect_memory_increment_:
	pusha 

	cmp eax, 0x100000
	jb  detect_memory_increment_lower_

	; Set upper BIOS memory
	mov ebx, [multiboot_mem_up_]
	add ebx, eax
	mov [multiboot_mem_up_], ebx	
	jmp detect_memory_increment_end_

detect_memory_increment_lower_:
	; Set lower BIOS memory
	mov ebx, [multiboot_mem_low_]
	add ebx, eax
	mov [multiboot_mem_low_], ebx	

detect_memory_increment_end_:
	popa
	ret 

; ---------------------------------------

detect_memory_error_:
	mov  bx, MSG_MULTIBOOT_MEM_ERROR ; Display error
	call boot_sect_out_              ; Output
	; Display error
	mov  bx, ax
	call boot_sect_out_hex_
	shr  eax, 16
	mov  bx, ax
	call boot_sect_out_hex_
	jmp  multiboot_halt_    


multiboot_halt_:
	cli 
	hlt 
	jmp multiboot_halt_
;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

MSG_MULTIBOOT_MEM_ERROR:
	db "[ERROR] Can't detect memory map", 0xA, 0xD, 0
MSG_MULTIBOOT_MEM_OK:
	db "END", 0xA, 0xD, 0

MSG_MULTIBOOT_LOADER_NAME:
	db "UTK Loader", 0

align 4
first_loop_:
	dd 0x00000001

mem_buffer_:
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000

multiboot_info_:

multiboot_flags_:
	dd 0x00000000

multiboot_mem_low_:
	dd 0x00000000
multiboot_mem_up_:
	dd 0x00000000

multiboot_boot_dev_:
	dd 0x00000000

multiboot_cmd_line_:
	dd 0x00000000

multiboot_mods_count_:
	dd 0x00000000
multiboot_mods_addr_:
	dd 0x00000000

multiboot_elf_aout_:
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000
	dd 0x00000000

multiboot_mmap_length_:
	dd 0x00000000
multiboot_mmap_addr_:
	dd 0x00000000

multiboot_drives_length_:
	dd 0x00000000
multiboot_drives_addr_:
	dd 0x00000000

multiboot_rom_cfg_table_:
	dd 0x00000000

multiboot_bootloader_name_:
	dd MSG_MULTIBOOT_LOADER_NAME

multiboot_apm_table_:
	dd 0x00000000

multiboot_vbe_ctrl_info_:
	dd 0x00000000
multiboot_vbe_mode_info_:
	dd 0x00000000
multiboot_vbe_mode_:
	dw 0x0000
multiboot_vbe_int_seg_:
	dw 0x0000
multiboot_vbe_int_off_:
	dw 0x0000
multiboot_vbe_int_len_:
	dw 0x0000

multiboot_framebufffer_addr_:
	dd 0x00000000
	dd 0x00000000
multiboot_framebufffer_pitch_:
	dd 0x00000000
multiboot_framebufffer_width_:
	dd 0x00000000
multiboot_framebufffer_height_:
	dd 0x00000000
multiboot_framebufffer_bpp_:
	db 0x00
multiboot_framebufffer_type:
	db 0x00

multiboot_palette_addr_:
	dd 0x00000000
multiboot_palette_num_colors_:
	dw 0x0000