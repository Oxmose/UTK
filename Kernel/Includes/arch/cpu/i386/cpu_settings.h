/*******************************************************************************
 * @file cpu_settings.h
 * 
 * @see cpu_settings.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.0
 *
 * @brief i386 CPU abstraction functions and definitions. 
 * 
 * @details i386 CPU abstraction: setting functions and structures, used to set 
 * the GDT, IDT and TSS of the CPU. This file also ontains the delarations of 
 * the 256 interrupt handlers of the i386 interrupts.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_CPU_SETTINGS_H_
#define __I386_CPU_SETTINGS_H_

#include <lib/stdint.h>  /* Generic int types */
#include <cpu_structs.h> /* CPU structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/***************************
 * GDT Settings
 **************************/

/** @brief Kernel's 32 bits code segment descriptor. */
#define KERNEL_CS_32 0x08
/** @brief Kernel's 32 bits data segment descriptor. */
#define KERNEL_DS_32 0x10
/** @brief Kernel's 16 bits ode segment descriptor. */
#define KERNEL_CS_16 0x18
/** @brief Kernel's 16 bits data segment descriptor. */
#define KERNEL_DS_16 0x20

/** @brief Select the thread code segment. */
#define THREAD_KERNEL_CS KERNEL_CS_32
/** @brief Select the thread code segment. */
#define THREAD_KERNEL_DS KERNEL_DS_32

/** @brief Kernel's 32 bits code segment base address. */
#define KERNEL_CODE_SEGMENT_BASE_32  0x00000000
/** @brief Kernel's 32 bits code segment limit address. */
#define KERNEL_CODE_SEGMENT_LIMIT_32 0x000FFFFF
/** @brief Kernel's 32 bits data segment base address. */
#define KERNEL_DATA_SEGMENT_BASE_32  0x00000000
/** @brief Kernel's 32 bits data segment limit address. */
#define KERNEL_DATA_SEGMENT_LIMIT_32 0x000FFFFF

/** @brief Kernel's 16 bits code segment base address. */
#define KERNEL_CODE_SEGMENT_BASE_16  0x00000000
/** @brief Kernel's 16 bits code segment limit address. */
#define KERNEL_CODE_SEGMENT_LIMIT_16 0x000FFFFF
/** @brief Kernel's 16 bits data segment base address. */
#define KERNEL_DATA_SEGMENT_BASE_16  0x00000000
/** @brief Kernel's 16 bits data segment limit address. */
#define KERNEL_DATA_SEGMENT_LIMIT_16 0x000FFFFF

/** @brief Kernel's TSS segment descriptor. */
#define TSS_SEGMENT 0x28

/***************************
 * GDT Flags
 **************************/

/** @brief GDT granularity flag: 4K block. */
#define GDT_FLAG_GRANULARITY_4K   0x800000
/** @brief GDT granularity flag: 1B block. */
#define GDT_FLAG_GRANULARITY_BYTE 0x000000
/** @brief GDT size flag: 16b protected mode. */
#define GDT_FLAG_16_BIT_SEGMENT   0x000000
/** @brief GDT size flag: 32b protected mode. */
#define GDT_FLAG_32_BIT_SEGMENT   0x400000
/** @brief GDT size flag: 64b protected mode. */
#define GDT_FLAG_64_BIT_SEGMENT   0x200000
/** @brief GDT AVL flag. */
#define GDT_FLAG_AVL              0x100000
/** @brief GDT segment present flag. */
#define GDT_FLAG_SEGMENT_PRESENT  0x008000
/** @brief GDT privilege level flag: Ring 0 (kernel). */
#define GDT_FLAG_PL0              0x000000
/** @brief GDT privilege level flag: Ring 1 (kernel-). */
#define GDT_FLAG_PL1              0x002000
/** @brief GDT privilege level flag: Ring 2 (kernel--). */
#define GDT_FLAG_PL2              0x004000
/** @brief GDT privilege level flag: Ring 3 (user). */
#define GDT_FLAG_PL3              0x006000
/** @brief GDT data type flag: code. */
#define GDT_FLAG_CODE_TYPE        0x001000
/** @brief GDT data type flag: data. */
#define GDT_FLAG_DATA_TYPE        0x001000
/** @brief GDT data type flag: system. */
#define GDT_FLAG_SYSTEM_TYPE      0x000000
/** @brief GDT TSS flag. */
#define GDT_FLAG_TSS              0x09

/** @brief GDT access byte flag: executable. */
#define GDT_TYPE_EXECUTABLE       0x8
/** @brief GDT access byte flag: growth direction up. */
#define GDT_TYPE_GROW_UP          0x4
/** @brief GDT access byte flag: growth direction down. */
#define GDT_TYPE_GROW_DOWN        0x0
/** @brief GDT access byte flag: conforming code. */
#define GDT_TYPE_CONFORMING       0x4
/** @brief GDT access byte flag: protected. */
#define GDT_TYPE_PROTECTED        0x0
/** @brief GDT access byte flag: readable. */
#define GDT_TYPE_READABLE         0x2
/** @brief GDT access byte flag: writable. */
#define GDT_TYPE_WRITABLE         0x2
/** @brief GDT access byte flag: accessed byte. */
#define GDT_TYPE_ACCESSED         0x1


/***************************
 * IDT Flags
 **************************/

/** @brief IDT flag: storage segment. */
#define IDT_FLAG_STORAGE_SEG 0x10
/** @brief IDT flag: privilege level, ring 0. */
#define IDT_FLAG_PL0         0x00
/** @brief IDT flag: privilege level, ring 1. */
#define IDT_FLAG_PL1         0x20
/** @brief IDT flag: privilege level, ring 2. */
#define IDT_FLAG_PL2         0x40
/** @brief IDT flag: privilege level, ring 3. */
#define IDT_FLAG_PL3         0x60
/** @brief IDT flag: interrupt present. */
#define IDT_FLAG_PRESENT     0x80

/** @brief IDT flag: interrupt type task gate. */
#define IDT_TYPE_TASK_GATE 0x05
/** @brief IDT flag: interrupt type interrupt gate. */
#define IDT_TYPE_INT_GATE  0x0E
/** @brief IDT flag: interrupt type trap gate. */
#define IDT_TYPE_TRAP_GATE 0x0F


/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Assembly interrupt handler for line 0. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_0(void);
/**
 * @brief Assembly interrupt handler for line 1. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_1(void);
/**
 * @brief Assembly interrupt handler for line 2. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_2(void);
/**
 * @brief Assembly interrupt handler for line 3. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_3(void);
/**
 * @brief Assembly interrupt handler for line 4. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_4(void);
/**
 * @brief Assembly interrupt handler for line 5. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_5(void);
/**
 * @brief Assembly interrupt handler for line 6. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_6(void);
/**
 * @brief Assembly interrupt handler for line 7. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_7(void);
/**
 * @brief Assembly interrupt handler for line 8. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_8(void);
/**
 * @brief Assembly interrupt handler for line 9. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_9(void);
/**
 * @brief Assembly interrupt handler for line 10. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_10(void);
/**
 * @brief Assembly interrupt handler for line 11. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_11(void);
/**
 * @brief Assembly interrupt handler for line 12. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_12(void);
/**
 * @brief Assembly interrupt handler for line 13. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_13(void);
/**
 * @brief Assembly interrupt handler for line 14. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_14(void);
/**
 * @brief Assembly interrupt handler for line 15. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_15(void);
/**
 * @brief Assembly interrupt handler for line 16. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_16(void);
/**
 * @brief Assembly interrupt handler for line 17. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_17(void);
/**
 * @brief Assembly interrupt handler for line 18. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_18(void);
/**
 * @brief Assembly interrupt handler for line 19. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_19(void);
/**
 * @brief Assembly interrupt handler for line 20. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_20(void);
/**
 * @brief Assembly interrupt handler for line 21. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_21(void);
/**
 * @brief Assembly interrupt handler for line 22. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_22(void);
/**
 * @brief Assembly interrupt handler for line 23. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_23(void);
/**
 * @brief Assembly interrupt handler for line 24. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_24(void);
/**
 * @brief Assembly interrupt handler for line 25. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_25(void);
/**
 * @brief Assembly interrupt handler for line 26. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_26(void);
/**
 * @brief Assembly interrupt handler for line 27. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_27(void);
/**
 * @brief Assembly interrupt handler for line 28. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_28(void);
/**
 * @brief Assembly interrupt handler for line 29. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_29(void);
/**
 * @brief Assembly interrupt handler for line 30. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_30(void);
/**
 * @brief Assembly interrupt handler for line 31. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_31(void);
/**
 * @brief Assembly interrupt handler for line 32. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_32(void);
/**
 * @brief Assembly interrupt handler for line 33. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_33(void);
/**
 * @brief Assembly interrupt handler for line 34. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_34(void);
/**
 * @brief Assembly interrupt handler for line 35. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_35(void);
/**
 * @brief Assembly interrupt handler for line 36. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_36(void);
/**
 * @brief Assembly interrupt handler for line 37. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_37(void);
/**
 * @brief Assembly interrupt handler for line 38. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_38(void);
/**
 * @brief Assembly interrupt handler for line 39. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_39(void);
/**
 * @brief Assembly interrupt handler for line 40. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_40(void);
/**
 * @brief Assembly interrupt handler for line 41. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_41(void);
/**
 * @brief Assembly interrupt handler for line 42. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_42(void);
/**
 * @brief Assembly interrupt handler for line 43. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_43(void);
/**
 * @brief Assembly interrupt handler for line 44. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_44(void);
/**
 * @brief Assembly interrupt handler for line 45. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_45(void);
/**
 * @brief Assembly interrupt handler for line 46. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_46(void);
/**
 * @brief Assembly interrupt handler for line 47. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_47(void);
/**
 * @brief Assembly interrupt handler for line 48. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_48(void);
/**
 * @brief Assembly interrupt handler for line 49. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_49(void);
/**
 * @brief Assembly interrupt handler for line 50. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_50(void);
/**
 * @brief Assembly interrupt handler for line 51. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_51(void);
/**
 * @brief Assembly interrupt handler for line 52. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_52(void);
/**
 * @brief Assembly interrupt handler for line 53. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_53(void);
/**
 * @brief Assembly interrupt handler for line 54. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_54(void);
/**
 * @brief Assembly interrupt handler for line 55. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_55(void);
/**
 * @brief Assembly interrupt handler for line 56. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_56(void);
/**
 * @brief Assembly interrupt handler for line 57. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_57(void);
/**
 * @brief Assembly interrupt handler for line 58. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_58(void);
/**
 * @brief Assembly interrupt handler for line 59. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_59(void);
/**
 * @brief Assembly interrupt handler for line 60. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_60(void);
/**
 * @brief Assembly interrupt handler for line 61. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_61(void);
/**
 * @brief Assembly interrupt handler for line 62. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_62(void);
/**
 * @brief Assembly interrupt handler for line 63. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_63(void);
/**
 * @brief Assembly interrupt handler for line 64. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_64(void);
/**
 * @brief Assembly interrupt handler for line 65. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_65(void);
/**
 * @brief Assembly interrupt handler for line 66. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_66(void);
/**
 * @brief Assembly interrupt handler for line 67. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_67(void);
/**
 * @brief Assembly interrupt handler for line 68. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_68(void);
/**
 * @brief Assembly interrupt handler for line 69. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_69(void);
/**
 * @brief Assembly interrupt handler for line 70. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_70(void);
/**
 * @brief Assembly interrupt handler for line 71. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_71(void);
/**
 * @brief Assembly interrupt handler for line 72. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_72(void);
/**
 * @brief Assembly interrupt handler for line 73. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_73(void);
/**
 * @brief Assembly interrupt handler for line 74. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_74(void);
/**
 * @brief Assembly interrupt handler for line 75. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_75(void);
/**
 * @brief Assembly interrupt handler for line 76. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_76(void);
/**
 * @brief Assembly interrupt handler for line 77. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_77(void);
/**
 * @brief Assembly interrupt handler for line 78. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_78(void);
/**
 * @brief Assembly interrupt handler for line 79. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_79(void);
/**
 * @brief Assembly interrupt handler for line 80. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_80(void);
/**
 * @brief Assembly interrupt handler for line 81. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_81(void);
/**
 * @brief Assembly interrupt handler for line 82. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_82(void);
/**
 * @brief Assembly interrupt handler for line 83. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_83(void);
/**
 * @brief Assembly interrupt handler for line 84. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_84(void);
/**
 * @brief Assembly interrupt handler for line 85. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_85(void);
/**
 * @brief Assembly interrupt handler for line 86. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_86(void);
/**
 * @brief Assembly interrupt handler for line 87. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_87(void);
/**
 * @brief Assembly interrupt handler for line 88. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_88(void);
/**
 * @brief Assembly interrupt handler for line 89. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_89(void);
/**
 * @brief Assembly interrupt handler for line 90. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_90(void);
/**
 * @brief Assembly interrupt handler for line 91. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_91(void);
/**
 * @brief Assembly interrupt handler for line 92. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_92(void);
/**
 * @brief Assembly interrupt handler for line 93. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_93(void);
/**
 * @brief Assembly interrupt handler for line 94. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_94(void);
/**
 * @brief Assembly interrupt handler for line 95. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_95(void);
/**
 * @brief Assembly interrupt handler for line 96. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_96(void);
/**
 * @brief Assembly interrupt handler for line 97. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_97(void);
/**
 * @brief Assembly interrupt handler for line 98. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_98(void);
/**
 * @brief Assembly interrupt handler for line 99. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_99(void);
/**
 * @brief Assembly interrupt handler for line 100. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_100(void);
/**
 * @brief Assembly interrupt handler for line 101. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_101(void);
/**
 * @brief Assembly interrupt handler for line 102. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_102(void);
/**
 * @brief Assembly interrupt handler for line 103. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_103(void);
/**
 * @brief Assembly interrupt handler for line 104. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_104(void);
/**
 * @brief Assembly interrupt handler for line 105. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_105(void);
/**
 * @brief Assembly interrupt handler for line 106. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_106(void);
/**
 * @brief Assembly interrupt handler for line 107. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_107(void);
/**
 * @brief Assembly interrupt handler for line 108. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_108(void);
/**
 * @brief Assembly interrupt handler for line 109. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_109(void);
/**
 * @brief Assembly interrupt handler for line 110. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_110(void);
/**
 * @brief Assembly interrupt handler for line 111. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_111(void);
/**
 * @brief Assembly interrupt handler for line 112. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_112(void);
/**
 * @brief Assembly interrupt handler for line 113. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_113(void);
/**
 * @brief Assembly interrupt handler for line 114. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_114(void);
/**
 * @brief Assembly interrupt handler for line 115. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_115(void);
/**
 * @brief Assembly interrupt handler for line 116. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_116(void);
/**
 * @brief Assembly interrupt handler for line 117. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_117(void);
/**
 * @brief Assembly interrupt handler for line 118. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_118(void);
/**
 * @brief Assembly interrupt handler for line 119. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_119(void);
/**
 * @brief Assembly interrupt handler for line 120. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_120(void);
/**
 * @brief Assembly interrupt handler for line 121. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_121(void);
/**
 * @brief Assembly interrupt handler for line 122. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_122(void);
/**
 * @brief Assembly interrupt handler for line 123. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_123(void);
/**
 * @brief Assembly interrupt handler for line 124. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_124(void);
/**
 * @brief Assembly interrupt handler for line 125. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_125(void);
/**
 * @brief Assembly interrupt handler for line 126. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_126(void);
/**
 * @brief Assembly interrupt handler for line 127. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_127(void);
/**
 * @brief Assembly interrupt handler for line 128. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_128(void);
/**
 * @brief Assembly interrupt handler for line 129. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_129(void);
/**
 * @brief Assembly interrupt handler for line 130. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_130(void);
/**
 * @brief Assembly interrupt handler for line 131. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_131(void);
/**
 * @brief Assembly interrupt handler for line 132. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_132(void);
/**
 * @brief Assembly interrupt handler for line 133. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_133(void);
/**
 * @brief Assembly interrupt handler for line 134. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_134(void);
/**
 * @brief Assembly interrupt handler for line 135. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_135(void);
/**
 * @brief Assembly interrupt handler for line 136. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_136(void);
/**
 * @brief Assembly interrupt handler for line 137. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_137(void);
/**
 * @brief Assembly interrupt handler for line 138. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_138(void);
/**
 * @brief Assembly interrupt handler for line 139. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_139(void);
/**
 * @brief Assembly interrupt handler for line 140. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_140(void);
/**
 * @brief Assembly interrupt handler for line 141. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_141(void);
/**
 * @brief Assembly interrupt handler for line 142. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_142(void);
/**
 * @brief Assembly interrupt handler for line 143. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_143(void);
/**
 * @brief Assembly interrupt handler for line 144. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_144(void);
/**
 * @brief Assembly interrupt handler for line 145. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_145(void);
/**
 * @brief Assembly interrupt handler for line 146. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_146(void);
/**
 * @brief Assembly interrupt handler for line 147. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_147(void);
/**
 * @brief Assembly interrupt handler for line 148. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_148(void);
/**
 * @brief Assembly interrupt handler for line 149. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_149(void);
/**
 * @brief Assembly interrupt handler for line 150. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_150(void);
/**
 * @brief Assembly interrupt handler for line 151. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_151(void);
/**
 * @brief Assembly interrupt handler for line 152. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_152(void);
/**
 * @brief Assembly interrupt handler for line 153. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_153(void);
/**
 * @brief Assembly interrupt handler for line 154. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_154(void);
/**
 * @brief Assembly interrupt handler for line 155. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_155(void);
/**
 * @brief Assembly interrupt handler for line 156. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_156(void);
/**
 * @brief Assembly interrupt handler for line 157. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_157(void);
/**
 * @brief Assembly interrupt handler for line 158. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_158(void);
/**
 * @brief Assembly interrupt handler for line 159. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_159(void);
/**
 * @brief Assembly interrupt handler for line 160. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_160(void);
/**
 * @brief Assembly interrupt handler for line 161. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_161(void);
/**
 * @brief Assembly interrupt handler for line 162. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_162(void);
/**
 * @brief Assembly interrupt handler for line 163. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_163(void);
/**
 * @brief Assembly interrupt handler for line 164. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_164(void);
/**
 * @brief Assembly interrupt handler for line 165. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_165(void);
/**
 * @brief Assembly interrupt handler for line 166. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_166(void);
/**
 * @brief Assembly interrupt handler for line 167. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_167(void);
/**
 * @brief Assembly interrupt handler for line 168. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_168(void);
/**
 * @brief Assembly interrupt handler for line 169. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_169(void);
/**
 * @brief Assembly interrupt handler for line 170. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_170(void);
/**
 * @brief Assembly interrupt handler for line 171. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_171(void);
/**
 * @brief Assembly interrupt handler for line 172. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_172(void);
/**
 * @brief Assembly interrupt handler for line 173. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_173(void);
/**
 * @brief Assembly interrupt handler for line 174. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_174(void);
/**
 * @brief Assembly interrupt handler for line 175. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_175(void);
/**
 * @brief Assembly interrupt handler for line 176. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_176(void);
/**
 * @brief Assembly interrupt handler for line 177. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_177(void);
/**
 * @brief Assembly interrupt handler for line 178. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_178(void);
/**
 * @brief Assembly interrupt handler for line 179. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_179(void);
/**
 * @brief Assembly interrupt handler for line 180. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_180(void);
/**
 * @brief Assembly interrupt handler for line 181. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_181(void);
/**
 * @brief Assembly interrupt handler for line 182. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_182(void);
/**
 * @brief Assembly interrupt handler for line 183. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_183(void);
/**
 * @brief Assembly interrupt handler for line 184. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_184(void);
/**
 * @brief Assembly interrupt handler for line 185. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_185(void);
/**
 * @brief Assembly interrupt handler for line 186. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_186(void);
/**
 * @brief Assembly interrupt handler for line 187. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_187(void);
/**
 * @brief Assembly interrupt handler for line 188. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_188(void);
/**
 * @brief Assembly interrupt handler for line 189. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_189(void);
/**
 * @brief Assembly interrupt handler for line 190. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_190(void);
/**
 * @brief Assembly interrupt handler for line 191. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_191(void);
/**
 * @brief Assembly interrupt handler for line 192. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_192(void);
/**
 * @brief Assembly interrupt handler for line 193. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_193(void);
/**
 * @brief Assembly interrupt handler for line 194. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_194(void);
/**
 * @brief Assembly interrupt handler for line 195. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_195(void);
/**
 * @brief Assembly interrupt handler for line 196. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_196(void);
/**
 * @brief Assembly interrupt handler for line 197. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_197(void);
/**
 * @brief Assembly interrupt handler for line 198. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_198(void);
/**
 * @brief Assembly interrupt handler for line 199. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_199(void);
/**
 * @brief Assembly interrupt handler for line 200. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_200(void);
/**
 * @brief Assembly interrupt handler for line 201. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_201(void);
/**
 * @brief Assembly interrupt handler for line 202. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_202(void);
/**
 * @brief Assembly interrupt handler for line 203. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_203(void);
/**
 * @brief Assembly interrupt handler for line 204. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_204(void);
/**
 * @brief Assembly interrupt handler for line 205. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_205(void);
/**
 * @brief Assembly interrupt handler for line 206. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_206(void);
/**
 * @brief Assembly interrupt handler for line 207. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_207(void);
/**
 * @brief Assembly interrupt handler for line 208. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_208(void);
/**
 * @brief Assembly interrupt handler for line 209. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_209(void);
/**
 * @brief Assembly interrupt handler for line 210. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_210(void);
/**
 * @brief Assembly interrupt handler for line 211. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_211(void);
/**
 * @brief Assembly interrupt handler for line 212. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_212(void);
/**
 * @brief Assembly interrupt handler for line 213. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_213(void);
/**
 * @brief Assembly interrupt handler for line 214. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_214(void);
/**
 * @brief Assembly interrupt handler for line 215. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_215(void);
/**
 * @brief Assembly interrupt handler for line 216. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_216(void);
/**
 * @brief Assembly interrupt handler for line 217. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_217(void);
/**
 * @brief Assembly interrupt handler for line 218. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_218(void);
/**
 * @brief Assembly interrupt handler for line 219. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_219(void);
/**
 * @brief Assembly interrupt handler for line 220. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_220(void);
/**
 * @brief Assembly interrupt handler for line 221. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_221(void);
/**
 * @brief Assembly interrupt handler for line 222. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_222(void);
/**
 * @brief Assembly interrupt handler for line 223. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_223(void);
/**
 * @brief Assembly interrupt handler for line 224. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_224(void);
/**
 * @brief Assembly interrupt handler for line 225. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_225(void);
/**
 * @brief Assembly interrupt handler for line 226. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_226(void);
/**
 * @brief Assembly interrupt handler for line 227. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_227(void);
/**
 * @brief Assembly interrupt handler for line 228. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_228(void);
/**
 * @brief Assembly interrupt handler for line 229. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_229(void);
/**
 * @brief Assembly interrupt handler for line 230. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_230(void);
/**
 * @brief Assembly interrupt handler for line 231. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_231(void);
/**
 * @brief Assembly interrupt handler for line 232. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_232(void);
/**
 * @brief Assembly interrupt handler for line 233. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_233(void);
/**
 * @brief Assembly interrupt handler for line 234. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_234(void);
/**
 * @brief Assembly interrupt handler for line 235. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_235(void);
/**
 * @brief Assembly interrupt handler for line 236. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_236(void);
/**
 * @brief Assembly interrupt handler for line 237. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_237(void);
/**
 * @brief Assembly interrupt handler for line 238. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_238(void);
/**
 * @brief Assembly interrupt handler for line 239. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_239(void);
/**
 * @brief Assembly interrupt handler for line 240. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_240(void);
/**
 * @brief Assembly interrupt handler for line 241. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_241(void);
/**
 * @brief Assembly interrupt handler for line 242. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_242(void);
/**
 * @brief Assembly interrupt handler for line 243. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_243(void);
/**
 * @brief Assembly interrupt handler for line 244. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_244(void);
/**
 * @brief Assembly interrupt handler for line 245. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_245(void);
/**
 * @brief Assembly interrupt handler for line 246. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_246(void);
/**
 * @brief Assembly interrupt handler for line 247. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_247(void);
/**
 * @brief Assembly interrupt handler for line 248. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_248(void);
/**
 * @brief Assembly interrupt handler for line 249. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_249(void);
/**
 * @brief Assembly interrupt handler for line 250. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_250(void);
/**
 * @brief Assembly interrupt handler for line 251. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_251(void);
/**
 * @brief Assembly interrupt handler for line 252. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_252(void);
/**
 * @brief Assembly interrupt handler for line 253. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_253(void);
/**
 * @brief Assembly interrupt handler for line 254. 
 * Saves the context and calls the generic interrupt handler
 */
extern void interrupt_handler_254(void);
/**
 * @brief Assembly interrupt handler for line 255. 
 * Saves the context and callss the generic interrupt handler
 */
extern void interrupt_handler_255(void);

/**
 * @brief Setups the kernel's GDT in memory and loads it in the GDT register.
 * 
 * @details Setups a GDT for the kernel. Fills the entries in the GDT table and 
 * load the new GDT in the CPU's GDT register. 
 * Once done, the function sets the segment registers (CS, DS, ES, FS, GS, SS)
 * of the CPU according to the kernel's settings.
 */
void cpu_setup_gdt(void);

/**
 * @brief Setups the generic kernel's IDT in memory and loads it in the IDT 
 * register.
 * 
 * @details Setups a simple IDT for the kernel. Fills the entries in the IDT 
 * table by adding basic support to the x86 exception (interrutps 0 to 32).
 * The rest of the interrupts are not set.
 */
void cpu_setup_idt(void);

/**
 *  @brief Setups the main CPU TSS for the kernel. 
 * 
 * @details Initializes the main CPU's TSS with kernel settings in memory and
 * loads it in the TSS register.
 */
void cpu_setup_tss(void);

#endif /* #ifndef __I386_CPU_SETTINGS_H_ */
