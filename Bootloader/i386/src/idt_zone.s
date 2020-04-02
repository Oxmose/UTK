;-------------------------------------------------------------------------------
; @file idt_zone.s
;
; @author Alexy Torres Aurora Dugo
;
; @date 06/03/2020
;
; @version 2.0
;
; @brief IDT data stored in the boot stage
;
; @details IDT data stored in the boot stage
;
; @copyright Alexy Torres Aurora Dugo
;-------------------------------------------------------------------------------

[bits 32]

;-------------------------------------------------------------------------------
; DEFINE Section
;-------------------------------------------------------------------------------

CODE32 equ 0x0008

;-----------------------------------------------------------
; DATA Section
;-----------------------------------------------------------

align 8
idt_base_:

dw int_0_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_1_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_2_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_3_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_4_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_5_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_6_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_7_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_8_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_9_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_10_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_11_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_12_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_13_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_14_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_15_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_16_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_17_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_18_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_19_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_20_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_21_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_22_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_23_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_24_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_25_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_26_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_27_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_28_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_29_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_30_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_31_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_32_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_33_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_34_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_35_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_36_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_37_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_38_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_39_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_40_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_41_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_42_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_43_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_44_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_45_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_46_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_47_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_48_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_49_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_50_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_51_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_52_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_53_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_54_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_55_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_56_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_57_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_58_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_59_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_60_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_61_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_62_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_63_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_64_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_65_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_66_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_67_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_68_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_69_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_70_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_71_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_72_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_73_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_74_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_75_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_76_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_77_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_78_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_79_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_80_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_81_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_82_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_83_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_84_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_85_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_86_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_87_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_88_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_89_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_90_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_91_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_92_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_93_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_94_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_95_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_96_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_97_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_98_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_99_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_100_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_101_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_102_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_103_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_104_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_105_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_106_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_107_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_108_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_109_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_110_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_111_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_112_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_113_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_114_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_115_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_116_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_117_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_118_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_119_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_120_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_121_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_122_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_123_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_124_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_125_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_126_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_127_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_128_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_129_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_130_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_131_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_132_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_133_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_134_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_135_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_136_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_137_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_138_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_139_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_140_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_141_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_142_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_143_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_144_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_145_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_146_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_147_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_148_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_149_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_150_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_151_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_152_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_153_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_154_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_155_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_156_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_157_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_158_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_159_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_160_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_161_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_162_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_163_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_164_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_165_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_166_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_167_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_168_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_169_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_170_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_171_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_172_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_173_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_174_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_175_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_176_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_177_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_178_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_179_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_180_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_181_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_182_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_183_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_184_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_185_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_186_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_187_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_188_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_189_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_190_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_191_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_192_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_193_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_194_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_195_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_196_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_197_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_198_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_199_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_200_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_201_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_202_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_203_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_204_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_205_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_206_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_207_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_208_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_209_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_210_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_211_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_212_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_213_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_214_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_215_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_216_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_217_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_218_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_219_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_220_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_221_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_222_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_223_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_224_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_225_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_226_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_227_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_228_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_229_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_230_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_231_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_232_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_233_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_234_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_235_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_236_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_237_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_238_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_239_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_240_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_241_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_242_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_243_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_244_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_245_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_246_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_247_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_248_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_249_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_250_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_251_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_252_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_253_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_254_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
dw int_255_   ; Low 16 Bits of the handler address
dw CODE32    ; Kernel CS
db 0x00      ; Zero
db 0x8E      ; 0x0E : Interrupt gate, 0x80 : PL0, present
dw 0x0000    ; High 16 Bits of the handler address

; ----
idt_ptr_:                          ; IDT pointer for 16bit access
dw idt_ptr_ - idt_base_ - 1    ; IDT size
dd idt_base_                   ; IDT base

int_0_:
	add eax, 0
	hlt
	jmp int_0_
int_1_:
	add eax, 1
	hlt
	jmp int_1_
int_2_:
	add eax, 2
	hlt
	jmp int_2_
int_3_:
	add eax, 3
	hlt
	jmp int_3_
int_4_:
	add eax, 4
	hlt
	jmp int_4_
int_5_:
	add eax, 5
	hlt
	jmp int_5_
int_6_:
	add eax, 6
	hlt
	jmp int_6_
int_7_:
	add eax, 7
	hlt
	jmp int_7_
int_8_:
	add eax, 8
	hlt
	jmp int_8_
int_9_:
	add eax, 9
	hlt
	jmp int_9_
int_10_:
	add eax, 10
	hlt
	jmp int_10_
int_11_:
	add eax, 11
	hlt
	jmp int_11_
int_12_:
	add eax, 12
	hlt
	jmp int_12_
int_13_:
	add eax, 13
	hlt
	jmp int_13_
int_14_:
	add eax, 14
	hlt
	jmp int_14_
int_15_:
	add eax, 15
	hlt
	jmp int_15_
int_16_:
	add eax, 16
	hlt
	jmp int_16_
int_17_:
	add eax, 17
	hlt
	jmp int_17_
int_18_:
	add eax, 18
	hlt
	jmp int_18_
int_19_:
	add eax, 19
	hlt
	jmp int_19_
int_20_:
	add eax, 20
	hlt
	jmp int_20_
int_21_:
	add eax, 21
	hlt
	jmp int_21_
int_22_:
	add eax, 22
	hlt
	jmp int_22_
int_23_:
	add eax, 23
	hlt
	jmp int_23_
int_24_:
	add eax, 24
	hlt
	jmp int_24_
int_25_:
	add eax, 25
	hlt
	jmp int_25_
int_26_:
	add eax, 26
	hlt
	jmp int_26_
int_27_:
	add eax, 27
	hlt
	jmp int_27_
int_28_:
	add eax, 28
	hlt
	jmp int_28_
int_29_:
	add eax, 29
	hlt
	jmp int_29_
int_30_:
	add eax, 30
	hlt
	jmp int_30_
int_31_:
	add eax, 31
	hlt
	jmp int_31_
int_32_:
	add eax, 32
	hlt
	jmp int_32_
int_33_:
	add eax, 33
	hlt
	jmp int_33_
int_34_:
	add eax, 34
	hlt
	jmp int_34_
int_35_:
	add eax, 35
	hlt
	jmp int_35_
int_36_:
	add eax, 36
	hlt
	jmp int_36_
int_37_:
	add eax, 37
	hlt
	jmp int_37_
int_38_:
	add eax, 38
	hlt
	jmp int_38_
int_39_:
	add eax, 39
	hlt
	jmp int_39_
int_40_:
	add eax, 40
	hlt
	jmp int_40_
int_41_:
	add eax, 41
	hlt
	jmp int_41_
int_42_:
	add eax, 42
	hlt
	jmp int_42_
int_43_:
	add eax, 43
	hlt
	jmp int_43_
int_44_:
	add eax, 44
	hlt
	jmp int_44_
int_45_:
	add eax, 45
	hlt
	jmp int_45_
int_46_:
	add eax, 46
	hlt
	jmp int_46_
int_47_:
	add eax, 47
	hlt
	jmp int_47_
int_48_:
	add eax, 48
	hlt
	jmp int_48_
int_49_:
	add eax, 49
	hlt
	jmp int_49_
int_50_:
	add eax, 50
	hlt
	jmp int_50_
int_51_:
	add eax, 51
	hlt
	jmp int_51_
int_52_:
	add eax, 52
	hlt
	jmp int_52_
int_53_:
	add eax, 53
	hlt
	jmp int_53_
int_54_:
	add eax, 54
	hlt
	jmp int_54_
int_55_:
	add eax, 55
	hlt
	jmp int_55_
int_56_:
	add eax, 56
	hlt
	jmp int_56_
int_57_:
	add eax, 57
	hlt
	jmp int_57_
int_58_:
	add eax, 58
	hlt
	jmp int_58_
int_59_:
	add eax, 59
	hlt
	jmp int_59_
int_60_:
	add eax, 60
	hlt
	jmp int_60_
int_61_:
	add eax, 61
	hlt
	jmp int_61_
int_62_:
	add eax, 62
	hlt
	jmp int_62_
int_63_:
	add eax, 63
	hlt
	jmp int_63_
int_64_:
	add eax, 64
	hlt
	jmp int_64_
int_65_:
	add eax, 65
	hlt
	jmp int_65_
int_66_:
	add eax, 66
	hlt
	jmp int_66_
int_67_:
	add eax, 67
	hlt
	jmp int_67_
int_68_:
	add eax, 68
	hlt
	jmp int_68_
int_69_:
	add eax, 69
	hlt
	jmp int_69_
int_70_:
	add eax, 70
	hlt
	jmp int_70_
int_71_:
	add eax, 71
	hlt
	jmp int_71_
int_72_:
	add eax, 72
	hlt
	jmp int_72_
int_73_:
	add eax, 73
	hlt
	jmp int_73_
int_74_:
	add eax, 74
	hlt
	jmp int_74_
int_75_:
	add eax, 75
	hlt
	jmp int_75_
int_76_:
	add eax, 76
	hlt
	jmp int_76_
int_77_:
	add eax, 77
	hlt
	jmp int_77_
int_78_:
	add eax, 78
	hlt
	jmp int_78_
int_79_:
	add eax, 79
	hlt
	jmp int_79_
int_80_:
	add eax, 80
	hlt
	jmp int_80_
int_81_:
	add eax, 81
	hlt
	jmp int_81_
int_82_:
	add eax, 82
	hlt
	jmp int_82_
int_83_:
	add eax, 83
	hlt
	jmp int_83_
int_84_:
	add eax, 84
	hlt
	jmp int_84_
int_85_:
	add eax, 85
	hlt
	jmp int_85_
int_86_:
	add eax, 86
	hlt
	jmp int_86_
int_87_:
	add eax, 87
	hlt
	jmp int_87_
int_88_:
	add eax, 88
	hlt
	jmp int_88_
int_89_:
	add eax, 89
	hlt
	jmp int_89_
int_90_:
	add eax, 90
	hlt
	jmp int_90_
int_91_:
	add eax, 91
	hlt
	jmp int_91_
int_92_:
	add eax, 92
	hlt
	jmp int_92_
int_93_:
	add eax, 93
	hlt
	jmp int_93_
int_94_:
	add eax, 94
	hlt
	jmp int_94_
int_95_:
	add eax, 95
	hlt
	jmp int_95_
int_96_:
	add eax, 96
	hlt
	jmp int_96_
int_97_:
	add eax, 97
	hlt
	jmp int_97_
int_98_:
	add eax, 98
	hlt
	jmp int_98_
int_99_:
	add eax, 99
	hlt
	jmp int_99_
int_100_:
	add eax, 100
	hlt
	jmp int_100_
int_101_:
	add eax, 101
	hlt
	jmp int_101_
int_102_:
	add eax, 102
	hlt
	jmp int_102_
int_103_:
	add eax, 103
	hlt
	jmp int_103_
int_104_:
	add eax, 104
	hlt
	jmp int_104_
int_105_:
	add eax, 105
	hlt
	jmp int_105_
int_106_:
	add eax, 106
	hlt
	jmp int_106_
int_107_:
	add eax, 107
	hlt
	jmp int_107_
int_108_:
	add eax, 108
	hlt
	jmp int_108_
int_109_:
	add eax, 109
	hlt
	jmp int_109_
int_110_:
	add eax, 110
	hlt
	jmp int_110_
int_111_:
	add eax, 111
	hlt
	jmp int_111_
int_112_:
	add eax, 112
	hlt
	jmp int_112_
int_113_:
	add eax, 113
	hlt
	jmp int_113_
int_114_:
	add eax, 114
	hlt
	jmp int_114_
int_115_:
	add eax, 115
	hlt
	jmp int_115_
int_116_:
	add eax, 116
	hlt
	jmp int_116_
int_117_:
	add eax, 117
	hlt
	jmp int_117_
int_118_:
	add eax, 118
	hlt
	jmp int_118_
int_119_:
	add eax, 119
	hlt
	jmp int_119_
int_120_:
	add eax, 120
	hlt
	jmp int_120_
int_121_:
	add eax, 121
	hlt
	jmp int_121_
int_122_:
	add eax, 122
	hlt
	jmp int_122_
int_123_:
	add eax, 123
	hlt
	jmp int_123_
int_124_:
	add eax, 124
	hlt
	jmp int_124_
int_125_:
	add eax, 125
	hlt
	jmp int_125_
int_126_:
	add eax, 126
	hlt
	jmp int_126_
int_127_:
	add eax, 127
	hlt
	jmp int_127_
int_128_:
	add eax, 128
	hlt
	jmp int_128_
int_129_:
	add eax, 129
	hlt
	jmp int_129_
int_130_:
	add eax, 130
	hlt
	jmp int_130_
int_131_:
	add eax, 131
	hlt
	jmp int_131_
int_132_:
	add eax, 132
	hlt
	jmp int_132_
int_133_:
	add eax, 133
	hlt
	jmp int_133_
int_134_:
	add eax, 134
	hlt
	jmp int_134_
int_135_:
	add eax, 135
	hlt
	jmp int_135_
int_136_:
	add eax, 136
	hlt
	jmp int_136_
int_137_:
	add eax, 137
	hlt
	jmp int_137_
int_138_:
	add eax, 138
	hlt
	jmp int_138_
int_139_:
	add eax, 139
	hlt
	jmp int_139_
int_140_:
	add eax, 140
	hlt
	jmp int_140_
int_141_:
	add eax, 141
	hlt
	jmp int_141_
int_142_:
	add eax, 142
	hlt
	jmp int_142_
int_143_:
	add eax, 143
	hlt
	jmp int_143_
int_144_:
	add eax, 144
	hlt
	jmp int_144_
int_145_:
	add eax, 145
	hlt
	jmp int_145_
int_146_:
	add eax, 146
	hlt
	jmp int_146_
int_147_:
	add eax, 147
	hlt
	jmp int_147_
int_148_:
	add eax, 148
	hlt
	jmp int_148_
int_149_:
	add eax, 149
	hlt
	jmp int_149_
int_150_:
	add eax, 150
	hlt
	jmp int_150_
int_151_:
	add eax, 151
	hlt
	jmp int_151_
int_152_:
	add eax, 152
	hlt
	jmp int_152_
int_153_:
	add eax, 153
	hlt
	jmp int_153_
int_154_:
	add eax, 154
	hlt
	jmp int_154_
int_155_:
	add eax, 155
	hlt
	jmp int_155_
int_156_:
	add eax, 156
	hlt
	jmp int_156_
int_157_:
	add eax, 157
	hlt
	jmp int_157_
int_158_:
	add eax, 158
	hlt
	jmp int_158_
int_159_:
	add eax, 159
	hlt
	jmp int_159_
int_160_:
	add eax, 160
	hlt
	jmp int_160_
int_161_:
	add eax, 161
	hlt
	jmp int_161_
int_162_:
	add eax, 162
	hlt
	jmp int_162_
int_163_:
	add eax, 163
	hlt
	jmp int_163_
int_164_:
	add eax, 164
	hlt
	jmp int_164_
int_165_:
	add eax, 165
	hlt
	jmp int_165_
int_166_:
	add eax, 166
	hlt
	jmp int_166_
int_167_:
	add eax, 167
	hlt
	jmp int_167_
int_168_:
	add eax, 168
	hlt
	jmp int_168_
int_169_:
	add eax, 169
	hlt
	jmp int_169_
int_170_:
	add eax, 170
	hlt
	jmp int_170_
int_171_:
	add eax, 171
	hlt
	jmp int_171_
int_172_:
	add eax, 172
	hlt
	jmp int_172_
int_173_:
	add eax, 173
	hlt
	jmp int_173_
int_174_:
	add eax, 174
	hlt
	jmp int_174_
int_175_:
	add eax, 175
	hlt
	jmp int_175_
int_176_:
	add eax, 176
	hlt
	jmp int_176_
int_177_:
	add eax, 177
	hlt
	jmp int_177_
int_178_:
	add eax, 178
	hlt
	jmp int_178_
int_179_:
	add eax, 179
	hlt
	jmp int_179_
int_180_:
	add eax, 180
	hlt
	jmp int_180_
int_181_:
	add eax, 181
	hlt
	jmp int_181_
int_182_:
	add eax, 182
	hlt
	jmp int_182_
int_183_:
	add eax, 183
	hlt
	jmp int_183_
int_184_:
	add eax, 184
	hlt
	jmp int_184_
int_185_:
	add eax, 185
	hlt
	jmp int_185_
int_186_:
	add eax, 186
	hlt
	jmp int_186_
int_187_:
	add eax, 187
	hlt
	jmp int_187_
int_188_:
	add eax, 188
	hlt
	jmp int_188_
int_189_:
	add eax, 189
	hlt
	jmp int_189_
int_190_:
	add eax, 190
	hlt
	jmp int_190_
int_191_:
	add eax, 191
	hlt
	jmp int_191_
int_192_:
	add eax, 192
	hlt
	jmp int_192_
int_193_:
	add eax, 193
	hlt
	jmp int_193_
int_194_:
	add eax, 194
	hlt
	jmp int_194_
int_195_:
	add eax, 195
	hlt
	jmp int_195_
int_196_:
	add eax, 196
	hlt
	jmp int_196_
int_197_:
	add eax, 197
	hlt
	jmp int_197_
int_198_:
	add eax, 198
	hlt
	jmp int_198_
int_199_:
	add eax, 199
	hlt
	jmp int_199_
int_200_:
	add eax, 200
	hlt
	jmp int_200_
int_201_:
	add eax, 201
	hlt
	jmp int_201_
int_202_:
	add eax, 202
	hlt
	jmp int_202_
int_203_:
	add eax, 203
	hlt
	jmp int_203_
int_204_:
	add eax, 204
	hlt
	jmp int_204_
int_205_:
	add eax, 205
	hlt
	jmp int_205_
int_206_:
	add eax, 206
	hlt
	jmp int_206_
int_207_:
	add eax, 207
	hlt
	jmp int_207_
int_208_:
	add eax, 208
	hlt
	jmp int_208_
int_209_:
	add eax, 209
	hlt
	jmp int_209_
int_210_:
	add eax, 210
	hlt
	jmp int_210_
int_211_:
	add eax, 211
	hlt
	jmp int_211_
int_212_:
	add eax, 212
	hlt
	jmp int_212_
int_213_:
	add eax, 213
	hlt
	jmp int_213_
int_214_:
	add eax, 214
	hlt
	jmp int_214_
int_215_:
	add eax, 215
	hlt
	jmp int_215_
int_216_:
	add eax, 216
	hlt
	jmp int_216_
int_217_:
	add eax, 217
	hlt
	jmp int_217_
int_218_:
	add eax, 218
	hlt
	jmp int_218_
int_219_:
	add eax, 219
	hlt
	jmp int_219_
int_220_:
	add eax, 220
	hlt
	jmp int_220_
int_221_:
	add eax, 221
	hlt
	jmp int_221_
int_222_:
	add eax, 222
	hlt
	jmp int_222_
int_223_:
	add eax, 223
	hlt
	jmp int_223_
int_224_:
	add eax, 224
	hlt
	jmp int_224_
int_225_:
	add eax, 225
	hlt
	jmp int_225_
int_226_:
	add eax, 226
	hlt
	jmp int_226_
int_227_:
	add eax, 227
	hlt
	jmp int_227_
int_228_:
	add eax, 228
	hlt
	jmp int_228_
int_229_:
	add eax, 229
	hlt
	jmp int_229_
int_230_:
	add eax, 230
	hlt
	jmp int_230_
int_231_:
	add eax, 231
	hlt
	jmp int_231_
int_232_:
	add eax, 232
	hlt
	jmp int_232_
int_233_:
	add eax, 233
	hlt
	jmp int_233_
int_234_:
	add eax, 234
	hlt
	jmp int_234_
int_235_:
	add eax, 235
	hlt
	jmp int_235_
int_236_:
	add eax, 236
	hlt
	jmp int_236_
int_237_:
	add eax, 237
	hlt
	jmp int_237_
int_238_:
	add eax, 238
	hlt
	jmp int_238_
int_239_:
	add eax, 239
	hlt
	jmp int_239_
int_240_:
	add eax, 240
	hlt
	jmp int_240_
int_241_:
	add eax, 241
	hlt
	jmp int_241_
int_242_:
	add eax, 242
	hlt
	jmp int_242_
int_243_:
	add eax, 243
	hlt
	jmp int_243_
int_244_:
	add eax, 244
	hlt
	jmp int_244_
int_245_:
	add eax, 245
	hlt
	jmp int_245_
int_246_:
	add eax, 246
	hlt
	jmp int_246_
int_247_:
	add eax, 247
	hlt
	jmp int_247_
int_248_:
	add eax, 248
	hlt
	jmp int_248_
int_249_:
	add eax, 249
	hlt
	jmp int_249_
int_250_:
	add eax, 250
	hlt
	jmp int_250_
int_251_:
	add eax, 251
	hlt
	jmp int_251_
int_252_:
	add eax, 252
	hlt
	jmp int_252_
int_253_:
	add eax, 253
	hlt
	jmp int_253_
int_254_:
	add eax, 254
	hlt
	jmp int_254_
int_255_:
	add eax, 255
	hlt
	jmp int_255_

dw 0xDEAD