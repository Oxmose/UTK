;*******************************************************************************
;* @file config.inc
;*
;* @author Alexy Torres Aurora Dugo
;*
;* @date 27/02/2021
;*
;* @version 1.0
;*
;* @brief X86 i386 global configuration file for assembly.
;*
;* @details X86 i386 global configuration file for assembly. This file contains 
;* all the settings that the user can set before generating the kernel's binary.
;******************************************************************************/

; Kernel memory offset 
; WARNING This value should be updated to fit other configuration files
KERNEL_MEM_OFFSET equ 0xE0000000 

; Kernel stack default size
; WARNING This value should be updated to fit other configuration files
KERNEL_STACK_SIZE equ 0x1000