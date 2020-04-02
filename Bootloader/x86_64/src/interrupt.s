;-------------------------------------------------------------------------------
; @file interrupts.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 20/03/2020
;
; @version 1.0
;
; @brief NASM boot code, manages interrupts in the bootloader.
;
; @details NASM boot code, manages interrupts in the bootloader.
; Sets PIC
; Sets PIT
; Sets keyboard interrupt
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------
PICM_COMM     equ 0x20
PICS_COMM     equ 0xA0
PICM_DATA     equ 0x21
PICS_DATA     equ 0xA1

PIC_OFFSET    equ 0x20
PIC_INIT      equ 0x10
PIC_ICW4      equ 0x01
PIC_MASTER    equ 0x04
PIC_SLAVE     equ 0x02
PIC_8086      equ 0x01

PIC_EOI       equ 0x20

PIT_COMM      equ 0x34
PIT_DATA      equ 0x40

PIT_SET_FREQ  equ 0x43

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------

;-------------------------
; Initializes the IDT
;-------------------------
interrupt_init_idt_:
	push eax
	push ebx

	; Initialize the PIT entry (0x20)
	mov eax, idt_base_
	add eax, 0x100
	mov ebx, interrupt_pit_handler_ ; Load low 16 bits of handler
	and ebx, 0x0000FFFF
	or  ebx, (CODE32 << 16)         ; GDT segment 32 bits 
	mov [eax], ebx 
	add eax, 0x4
	mov ebx, interrupt_pit_handler_ ; Load high 16 bits of handler
	and ebx, 0xFFFF0000
	or  ebx, 0x00008E00             ; 0x0E : Interrupt gate, 0x80 : PL0, present
	mov [eax], ebx 

	; Initialize the spurious entry (0x2F) 
	mov eax, idt_base_
	add eax, 0x178
	mov ebx, interrupt_spur_handler_ ; Load low 16 bits of handler
	and ebx, 0x0000FFFF
	or  ebx, (CODE32 << 16)          ; GDT segment 32 bits 
	mov [eax], ebx 
	add eax, 0x4
	mov ebx, interrupt_spur_handler_ ; Load high 16 bits of handler
	and ebx, 0xFFFF0000
	or  ebx, 0x00008E00              ; 0x0E : Interrupt gate, 0x80 : PL0, present
	mov [eax], ebx 

	pop ebx 
	pop eax	
	ret

;-------------------------
; Initializes the PIC
;-------------------------
interrupt_init_pic_:
	push eax

	; Master PIC init sequence
	mov  al, PIC_INIT | PIC_ICW4
	out PICM_COMM, al
	out PICS_COMM, al

	; PIC offset
	mov  al, PIC_OFFSET
	out PICM_DATA, al 
	add  al, 0x8
	out PICS_DATA, al 

	; PIC chaining 
	mov  al, PIC_MASTER
	out PICM_DATA, al 
	mov  al, PIC_SLAVE
	out PICS_DATA, al

	; Set PIC mode 
	mov  al, PIC_8086
	out PICM_DATA, al
	out PICS_DATA, al 

	; Set IRQ masks
	mov  al, 0xFF 
	out PICM_DATA, al
	out PICS_DATA, al

	pop eax
	ret

;-------------------------
; Set PIC EOI
;-------------------------
interrupt_pic_eoi_:
	push eax 

	cmp eax, 8
	mov al, PIC_EOI
	jb  intrrupt_picm_send_eoi_

	out PICS_COMM, al 
	jmp intrrupt_pic_send_eoi_end_

intrrupt_picm_send_eoi_:
	out PICM_COMM, al 

intrrupt_pic_send_eoi_end_:
	pop eax
	ret

;-------------------------
; Set PIC mask
;-------------------------
interrupt_pic_set_mask_:
	push eax 
	push ebx
	push ecx 

	cmp eax, 8
	jb  interrupt_pic_set_mmask_

	; Set slave port 
	mov  ecx, eax 
	mov  ebx, 1
	shl  ebx, cl
	in   al, PICS_DATA
	or   al, bl
	out PICS_DATA, al
	jmp interrupt_pic_set_mask_end_

interrupt_pic_set_mmask_:
	; Set master port 
	mov  ecx, eax 
	mov  ebx, 1
	shl  ebx, cl
	in   al, PICM_DATA
	or   al, bl
	out PICM_DATA, al
	jmp interrupt_pic_set_mask_end_

interrupt_pic_set_mask_end_:
	pop ecx
	pop ebx
	pop eax 
	ret


;-------------------------
; Clear PIC mask
;-------------------------
interrupt_pic_clear_mask_:
	push eax 
	push ebx
	push ecx 

	cmp eax, 8
	jb  interrupt_pic_clear_mmask_

	; Clear slave port 
	mov  ecx, eax 
	mov  ebx, 1
	shl  ebx, cl
	not  ebx
	in   al, PICS_DATA
	and  al, bl
	out PICS_DATA, al
	jmp interrupt_pic_clear_mask_end_

interrupt_pic_clear_mmask_:
	; Clear master port 
	mov  ecx, eax 
	mov  ebx, 1
	shl  ebx, cl
	not  ebx
	in   al, PICM_DATA
	and  al, bl
	out PICM_DATA, al
	jmp interrupt_pic_clear_mask_end_

interrupt_pic_clear_mask_end_:
	pop ecx
	pop ebx
	pop eax 
	ret

;-------------------------
; Initializes the PIT
;-------------------------
interrupt_init_pit_:
	push eax

	; Disable PIT
	mov  eax, 0x0
	call interrupt_pic_set_mask_

	; Set PIT preiod to 1ms
	mov  al, PIT_SET_FREQ
	out PIT_COMM, al 
	mov  al, 0xA9
	out PIT_DATA, al
	mov  al, 0x04
	out  PIT_DATA, al
 
	pop eax
	ret

;-------------------------
; Enables the PIT
;-------------------------
interrupt_pit_enable_:
	push eax

	; Enables PIT
	mov  eax, 0x0
	call interrupt_pic_clear_mask_

	pop eax
	ret

;-------------------------
; Disables the PIT
;-------------------------
interrupt_pit_disable_:
	push eax

	; Disable PIT
	mov  eax, 0x0
	call interrupt_pic_set_mask_

	pop eax
	ret
	

;-------------------------
; Wait PIT
;-------------------------
interrupt_pit_wait_:
	push eax 

	mov [wait_counter_], eax 

	call interrupt_pit_enable_
	sti 

interrupt_pit_wait_loop_:
	mov eax, [wait_counter_]
	cmp eax, 0
	jne interrupt_pit_wait_loop_

	cli 
	call interrupt_pit_disable_

	pop eax
	ret 

interrupt_pit_handler_:
	push eax

	mov eax, [wait_counter_]
	dec eax 
	mov [wait_counter_], eax

	mov eax, 0
	call interrupt_pic_eoi_

	pop eax

	iret

interrupt_spur_handler_:
	iret

%include "src/idt_zone.s"

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------

MSG_PT:
	db "H", 0

wait_counter_:
	dd 0x00000000