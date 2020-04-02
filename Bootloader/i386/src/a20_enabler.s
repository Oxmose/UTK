;-------------------------------------------------------------------------------
; @file boot_stage1.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief NASM boot code, enables a20 gate
;
; @details NASM boot code, enables a20 gate
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 16]

;-------------------------------------------------------------------------------
; TEXT Section
;-------------------------------------------------------------------------------
enable_a20_:
	call check_a20_enabled_ ; Test if A20 is already enabled
	cmp  ax, 1			    ; If enabled continue, else try again
	je   enable_a20_end_

	mov  bx, MSG_A20_ENABLER_1_FAIL
	call boot_sect_out_

	call enable_a20_bios_   ; Call the bios A20 enabler
	call check_a20_enabled_ ; Test if A20 is enabled
	cmp  ax, 1			    ; If enabled continue, else try again
	je   enable_a20_end_

	mov  bx, MSG_A20_ENABLER_2_FAIL
	call boot_sect_out_

	call enable_a20_keyboard_ ; Call the keyboard A20 enabler
	call check_a20_enabled_   ; Test if A20 is enabled
	cmp  ax, 1			      ; If enabled continue, else try again
	je   enable_a20_end_

	mov  bx, MSG_A20_ENABLER_3_FAIL
	call boot_sect_out_

	call enable_a20_fast_   ; Call the fast A20 enabler
	call check_a20_enabled_ ; Test if A20 is enabled
	cmp  ax, 1			    ; If enabled continue, else try again
	je   enable_a20_end_

	mov  bx, MSG_A20_ENABLER_4_FAIL
	call boot_sect_out_

	jmp a20_enabler_halt_ ; Halt because or failure

check_a20_enabled_:
	pushf   ; Save
	push ds ; registers
	push es ; used
	push di ; by
	push si ; check

	xor ax, ax      ; Clear ax
	mov es, ax      ; Set ES 0x0000
	mov di, 0x0500  ; Set ES:DI 0x0000:0x0500

	mov ax, 0xFFFF  ; Set AX 0xFFFF
	mov ds, ax		; Set DS 0xFFFF
	mov si, 0x510   ; Set DS:SI 0xFFFF:0x0510

	; Now since ES:DI and DS:SI point to the same area, we write two different
	; values and we check the last value at this adress to know if thewrapping
	; around was enabled (A20 disabled)

	mov  al, [es:di] ; Save old value
	push ax         ; at ES:DI

	mov  al, [ds:si] ; Save old value
	push ax         ; at DS:SI

	mov byte [es:di], 0x00 ; Write new value
	mov byte [ds:si], 0xFF ; Write new value

	cmp byte [es:di], 0xFF ; Compare the wrapp aroud result case

	pop ax          ; Restore old value
	mov [ds:si], al ; at DS:SI

	pop ax          ; Restore old value
	mov [es:di], al ; At ES:DI

	mov ax, 0				   ; A20 disabled
	je  check_a20_enabled_end_ ; If not equal, A20 is enabled
	mov ax, 1                  ; A20 enabled

check_a20_enabled_end_:
	pop si ; Restore
	pop di ; registers
	pop es ; used
	pop ds ; by
	popf   ; check

enable_a20_bios_:
	mov ax, 0x2401 ; Set bios function
	int 0x15	   ; Issue command
	ret

enable_a20_keyboard_:
	call wait_8042_comm_  ; Wait keyboard to be ready
	mov  al, 0xAD         ; Prepare disable command
	out  0x64, al         ; Disable keyboard

	call wait_8042_comm_  ; Wait keyboatd to de ready
	mov  al, 0xD0         ; Prepare to read from input
	out  0x64, al         ; Ask to read from input

	call wait_8042_data_  ; Wait keyboard to be ready
	in   al, 0x60         ; Read input from keyboard
	push ax               ; Save read data

	call wait_8042_comm_  ; Wait keyboard to be ready
	mov  al, 0xD1         ; Prepare to write to keyboard
	out  0x64, al         ; Ask to write to keyboard

	call wait_8042_comm_  ; Wait keyboard to be ready
	pop  ax               ; Restore read data
	or   al, 2            ; Set bit 1 (A20 gate enable)
	out  0x60, al         ; Write to keyboard

	call wait_8042_comm_  ; Wait keyboard to be ready
	mov  al, 0xAE         ; Prepare to enable keyboard
	out  0x64, al         ; Enable keyboard

	call wait_8042_comm_  ; Wait keyboard to be ready

	ret

enable_a20_fast_:
	in  al, 0x92  ; Read A20 state
	or  al, 2     ; Set A20 on
	out 0x92, al  ; Write new value
	ret

enable_a20_end_:
	ret

; Keyboard comm ready wait
wait_8042_comm_:
	in   al, 0x64        ; Get register value
	test al, 2           ; Check ready bit
	jnz  wait_8042_comm_ ; Loop if not ready
	ret

; Keyboard data ready wait
wait_8042_data_:
	in   al, 0x64        ; Get register value
	test al, 1           ; Check ready bit
	jz   wait_8042_data_ ; Loop if not ready
	ret

a20_enabler_halt_:
	hlt 
	jmp a20_enabler_halt_

;-------------------------------------------------------------------------------
; DATA Section
;-------------------------------------------------------------------------------
; Messages
MSG_A20_ENABLER_1_FAIL:
	db "[INFO] A20 gate disabled, enabling using BIOS method", 0xA, 0xD, 0
MSG_A20_ENABLER_2_FAIL:
	db "[ERROR] Failure, enabling using keyboard method", 0xA, 0xD, 0
MSG_A20_ENABLER_3_FAIL:
	db "[ERROR] Failure, enabling using fast A20 method", 0xA, 0xD, 0
MSG_A20_ENABLER_4_FAIL:
	db "[ERROR] Failure, A20 could not be enabled, halting", 0xA, 0xD, 0
