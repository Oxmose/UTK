/*******************************************************************************
 * @file cpu.c
 *
 * @see cpu.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief i386 CPU management functions
 *
 * @details i386 CPU manipulation functions. Wraps inline assembly calls for
 * ease of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definitions */
#include <string.h>             /* String manipulation */
#include <kernel_output.h>      /* Kernel output methods */
#include <interrupt_settings.h> /* Interrupt settings */
#include <panic.h>              /* Kernel panic */
#include <cpu_settings.h>       /* CPU structures */
#include <interrupts.h>         /* Interrupt manager */
#include <cpu_api.h>            /* CPU API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <cpu.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/


/** @brief CPU flags interrupt enabled flag. */
#define CPU_EFLAGS_IF 0x000000200
/** @brief CPU flags interrupt enabled bit shift. */
#define CPU_EFLAGS_IF_SHIFT 9

/** @brief CPUID capable flags. */
#define CPU_FLAG_CPU_CPUID_CAPABLE 0x00200000

/** @brief Request vendor string. */
#define CPUID_GETVENDORSTRING          0x00000000
/** @brief Request capabled CPUID features. */
#define CPUID_GETFEATURES              0x00000001
/** @brief Request TLB. */
#define CPUID_GETTLB                   0x00000002
/** @brief Request serial. */
#define CPUID_GETSERIAL                0x00000003
/** @brief Request extended CPUID features. */
#define CPUID_INTELEXTENDED_AVAILABLE  0x80000000
/** @brief Request Intel CPUID features. */
#define CPUID_INTELFEATURES            0x80000001
/** @brief Request Intel brand string. */
#define CPUID_INTELBRANDSTRING         0x80000002
/** @brief Request Intel brand string extended. */
#define CPUID_INTELBRANDSTRINGMORE     0x80000003
/** @brief Request Intel brand string end. */
#define CPUID_INTELBRANDSTRINGEND      0x80000004

/****************************
 * General Features
 ***************************/

/** @brief CPUID Streaming SIMD Extensions 3 flag. */
#define ECX_SSE3      (1 << 0)
/** @brief CPUID PCLMULQDQ Instruction flag. */
#define ECX_PCLMULQDQ (1 << 1)
/** @brief CPUID 64-Bit Debug Store Area flag. */
#define ECX_DTES64    (1 << 2)
/** @brief CPUID MONITOR/MWAIT flag. */
#define ECX_MONITOR   (1 << 3)
/** @brief CPUID CPL Qualified Debug Store flag. */
#define ECX_DS_CPL    (1 << 4)
/** @brief CPUID Virtual Machine Extensions flag. */
#define ECX_VMX       (1 << 5)
/** @brief CPUID Safer Mode Extensions flag. */
#define ECX_SMX       (1 << 6)
/** @brief CPUID Enhanced SpeedStep Technology flag. */
#define ECX_EST       (1 << 7)
/** @brief CPUID Thermal Monitor 2 flag. */
#define ECX_TM2       (1 << 8)
/** @brief CPUID Supplemental Streaming SIMD Extensions 3 flag. */
#define ECX_SSSE3     (1 << 9)
/** @brief CPUID L1 Context ID flag. */
#define ECX_CNXT_ID   (1 << 10)
/** @brief CPUID Fused Multiply Add flag. */
#define ECX_FMA       (1 << 12)
/** @brief CPUID CMPXCHG16B Instruction flag. */
#define ECX_CX16      (1 << 13)
/** @brief CPUID xTPR Update Control flag. */
#define ECX_XTPR      (1 << 14)
/** @brief CPUID Perf/Debug Capability MSR flag. */
#define ECX_PDCM      (1 << 15)
/** @brief CPUID Process-context Identifiers flag. */
#define ECX_PCID      (1 << 17)
/** @brief CPUID Direct Cache Access flag. */
#define ECX_DCA       (1 << 18)
/** @brief CPUID Streaming SIMD Extensions 4.1 flag. */
#define ECX_SSE41     (1 << 19)
/** @brief CPUID Streaming SIMD Extensions 4.2 flag. */
#define ECX_SSE42     (1 << 20)
/** @brief CPUID Extended xAPIC Support flag. */
#define ECX_X2APIC    (1 << 21)
/** @brief CPUID MOVBE Instruction flag. */
#define ECX_MOVBE     (1 << 22)
/** @brief CPUID POPCNT Instruction flag. */
#define ECX_POPCNT    (1 << 23)
/** @brief CPUID Local APIC supports TSC Deadline flag. */
#define ECX_TSC       (1 << 24)
/** @brief CPUID AESNI Instruction flag. */
#define ECX_AESNI     (1 << 25)
/** @brief CPUID XSAVE/XSTOR States flag. */
#define ECX_XSAVE     (1 << 26)
/** @brief CPUID OS Enabled Extended State Management flag. */
#define ECX_OSXSAVE   (1 << 27)
/** @brief CPUID AVX Instructions flag. */
#define ECX_AVX       (1 << 28)
/** @brief CPUID 16-bit Floating Point Instructions flag. */
#define ECX_F16C      (1 << 29)
/** @brief CPUID RDRAND Instruction flag. */
#define ECX_RDRAND    (1 << 30)
/** @brief CPUID Floating-Point Unit On-Chip flag. */
#define EDX_FPU       (1 << 0)
/** @brief CPUID Virtual 8086 Mode Extensions flag. */
#define EDX_VME       (1 << 1)
/** @brief CPUID Debugging Extensions flag. */
#define EDX_DE        (1 << 2)
/** @brief CPUID Page Size Extension flag. */
#define EDX_PSE       (1 << 3)
/** @brief CPUID Time Stamp Counter flag. */
#define EDX_TSC       (1 << 4)
/** @brief CPUID Model Specific Registers flag. */
#define EDX_MSR       (1 << 5)
/** @brief CPUID Physical Address Extension flag. */
#define EDX_PAE       (1 << 6)
/** @brief CPUID Machine-Check Exception flag. */
#define EDX_MCE       (1 << 7)
/** @brief CPUID CMPXCHG8 Instruction flag. */
#define EDX_CX8       (1 << 8)
/** @brief CPUID APIC On-Chip flag. */
#define EDX_APIC      (1 << 9)
/** @brief CPUID SYSENTER/SYSEXIT instructions flag. */
#define EDX_SEP       (1 << 11)
/** @brief CPUID Memory Type Range Registers flag. */
#define EDX_MTRR      (1 << 12)
/** @brief CPUID Page Global Bit flag. */
#define EDX_PGE       (1 << 13)
/** @brief CPUID Machine-Check Architecture flag. */
#define EDX_MCA       (1 << 14)
/** @brief CPUID Conditional Move Instruction flag. */
#define EDX_CMOV      (1 << 15)
/** @brief CPUID Page Attribute Table flag. */
#define EDX_PAT       (1 << 16)
/** @brief CPUID 36-bit Page Size Extension flag. */
#define EDX_PSE36     (1 << 17)
/** @brief CPUID Processor Serial Number flag. */
#define EDX_PSN       (1 << 18)
/** @brief CPUID CLFLUSH Instruction flag. */
#define EDX_CLFLUSH   (1 << 19)
/** @brief CPUID Debug Store flag. */
#define EDX_DS        (1 << 21)
/** @brief CPUID Thermal Monitor and Clock Facilities flag. */
#define EDX_ACPI      (1 << 22)
/** @brief CPUID MMX Technology flag. */
#define EDX_MMX       (1 << 23)
/** @brief CPUID FXSAVE and FXSTOR Instructions flag. */
#define EDX_FXSR      (1 << 24)
/** @brief CPUID Streaming SIMD Extensions flag. */
#define EDX_SSE       (1 << 25)
/** @brief CPUID Streaming SIMD Extensions 2 flag. */
#define EDX_SSE2      (1 << 26)
/** @brief CPUID Self Snoop flag. */
#define EDX_SS        (1 << 27)
/** @brief CPUID Multi-Threading flag. */
#define EDX_HTT       (1 << 28)
/** @brief CPUID Thermal Monitor flag. */
#define EDX_TM        (1 << 29)
/** @brief CPUID Pending Break Enable flag. */
#define EDX_PBE       (1 << 31)

/****************************
 * Extended Features
 ***************************/
/** @brief CPUID SYSCALL/SYSRET flag. */
#define EDX_SYSCALL   (1 << 11)
/** @brief CPUID Multiprocessor flag. */
#define EDX_MP        (1 << 19)
/** @brief CPUID Execute Disable Bit flag. */
#define EDX_XD        (1 << 20)
/** @brief CPUID MMX etended flag. */
#define EDX_MMX_EX    (1 << 22)
/** @brief CPUID FXSAVE/STOR available flag. */
#define EDX_FXSR     (1 << 24)
/** @brief CPUID FXSAVE/STOR optimized flag. */
#define EDX_FXSR_OPT  (1 << 25)
/** @brief CPUID 1 GB Pages flag. */
#define EDX_1GB_PAGE  (1 << 26)
/** @brief CPUID RDTSCP and IA32_TSC_AUX flag. */
#define EDX_RDTSCP    (1 << 27)
/** @brief CPUID 64-bit Architecture flag. */
#define EDX_64_BIT    (1 << 29)
/** @brief CPUID 3D Now etended flag. */
#define EDX_3DNOW_EX  (1 << 30)
/** @brief CPUID 3D Now flag. */
#define EDX_3DNOW     (1 << 31)
/** @brief CPUID LAHF Available in long mode flag */
#define ECX_LAHF_LM   (1 << 0)
/** @brief CPUID Hyperthreading not valid flag */
#define ECX_CMP_LEG   (1 << 1)
/** @brief CPUID Secure Virtual Machine flag */
#define ECX_SVM       (1 << 2)
/** @brief CPUID Extended API space flag */
#define ECX_EXTAPIC   (1 << 3)
/** @brief CPUID CR8 in protected mode flag */
#define ECX_CR8_LEG   (1 << 4)
/** @brief CPUID ABM available flag */
#define ECX_ABM       (1 << 5)
/** @brief CPUID SSE4A flag */
#define ECX_SSE4A     (1 << 6)
/** @brief CPUID Misaligne SSE mode flag */
#define ECX_MISASSE   (1 << 7)
/** @brief CPUID Prefetch flag */
#define ECX_PREFETCH  (1 << 8)
/** @brief CPUID OS Visible workaround flag */
#define ECX_OSVW      (1 << 9)
/** @brief CPUID Instruction based sampling flag */
#define ECX_IBS       (1 << 10)
/** @brief CPUID XIO intruction set flag */
#define ECX_XOP       (1 << 11)
/** @brief CPUID SKINIT instructions flag */
#define ECX_SKINIT    (1 << 12)
/** @brief CPUID watchdog timer flag */
#define ECX_WDT       (1 << 13)
/** @brief CPUID Light weight profiling flag */
#define ECX_LWP       (1 << 15)
/** @brief CPUID 4 operand fuxed multiply add flag */
#define ECX_FMA4      (1 << 16)
/** @brief CPUID Translation cache extension flag */
#define ECX_TCE       (1 << 17)
/** @brief CPUID NODE_ID MSR flag */
#define ECX_NODEIDMSR (1 << 19)
/** @brief CPUID Trailing bit manipulation flag */
#define ECX_TBM       (1 << 21)
/** @brief CPUID Topology extension flag */
#define ECX_TOPOEX    (1 << 22)
/** @brief CPUID Core performance counter extensions flag */
#define ECX_PERF_CORE (1 << 23)
/** @brief CPUID NB performance counter extensions flag */
#define ECX_PERF_NB   (1 << 24)
/** @brief CPUID Data breakpoint extensions flag */
#define ECX_DBX       (1 << 26)
/** @brief CPUID Performance TSC flag */
#define ECX_PERF_TSC  (1 << 27)
/** @brief CPUID L2I perf counter extensions flag */
#define ECX_PCX_L2I   (1 << 28)

/****************************
 * CPU Vendor signatures
 ***************************/

/** @brief CPUID Vendor signature AMD EBX. */
#define SIG_AMD_EBX 0x68747541
/** @brief CPUID Vendor signature AMD ECX. */
#define SIG_AMD_ECX 0x444d4163
/** @brief CPUID Vendor signature AMD EDX. */
#define SIG_AMD_EDX 0x69746e65

/** @brief CPUID Vendor signature Centaur EBX. */
#define SIG_CENTAUR_EBX   0x746e6543
/** @brief CPUID Vendor signature Centaur ECX. */
#define SIG_CENTAUR_ECX   0x736c7561
/** @brief CPUID Vendor signature Centaur EDX. */
#define SIG_CENTAUR_EDX   0x48727561

/** @brief CPUID Vendor signature Cyrix EBX. */
#define SIG_CYRIX_EBX 0x69727943
/** @brief CPUID Vendor signature Cyrix ECX. */
#define SIG_CYRIX_ECX 0x64616574
/** @brief CPUID Vendor signature Cyrix EDX. */
#define SIG_CYRIX_EDX 0x736e4978

/** @brief CPUID Vendor signature Intel EBX. */
#define SIG_INTEL_EBX 0x756e6547
/** @brief CPUID Vendor signature Intel ECX. */
#define SIG_INTEL_ECX 0x6c65746e
/** @brief CPUID Vendor signature Intel EDX. */
#define SIG_INTEL_EDX 0x49656e69

/** @brief CPUID Vendor signature TM1 EBX. */
#define SIG_TM1_EBX   0x6e617254
/** @brief CPUID Vendor signature TM1 ECX. */
#define SIG_TM1_ECX   0x55504361
/** @brief CPUID Vendor signature TM1 EDX. */
#define SIG_TM1_EDX   0x74656d73

/** @brief CPUID Vendor signature TM2 EBX. */
#define SIG_TM2_EBX   0x756e6547
/** @brief CPUID Vendor signature TM2 ECX. */
#define SIG_TM2_ECX   0x3638784d
/** @brief CPUID Vendor signature TM2 EDX. */
#define SIG_TM2_EDX   0x54656e69

/** @brief CPUID Vendor signature NSC EBX. */
#define SIG_NSC_EBX   0x646f6547
/** @brief CPUID Vendor signature NSC ECX. */
#define SIG_NSC_ECX   0x43534e20
/** @brief CPUID Vendor signature NSC EDX. */
#define SIG_NSC_EDX   0x79622065

/** @brief CPUID Vendor signature NextGen EBX. */
#define SIG_NEXGEN_EBX    0x4778654e
/** @brief CPUID Vendor signature NextGen ECX. */
#define SIG_NEXGEN_ECX    0x6e657669
/** @brief CPUID Vendor signature NextGen EDX. */
#define SIG_NEXGEN_EDX    0x72446e65

/** @brief CPUID Vendor signature Rise EBX. */
#define SIG_RISE_EBX  0x65736952
/** @brief CPUID Vendor signature Rise ECX. */
#define SIG_RISE_ECX  0x65736952
/** @brief CPUID Vendor signature Rise EDX. */
#define SIG_RISE_EDX  0x65736952

/** @brief CPUID Vendor signature SIS EBX. */
#define SIG_SIS_EBX   0x20536953
/** @brief CPUID Vendor signature SIS ECX. */
#define SIG_SIS_ECX   0x20536953
/** @brief CPUID Vendor signature SIS EDX. */
#define SIG_SIS_EDX   0x20536953

/** @brief CPUID Vendor signature UMC EBX. */
#define SIG_UMC_EBX   0x20434d55
/** @brief CPUID Vendor signature UMC ECX. */
#define SIG_UMC_ECX   0x20434d55
/** @brief CPUID Vendor signature UMC EDX. */
#define SIG_UMC_EDX   0x20434d55

/** @brief CPUID Vendor signature VIA EBX. */
#define SIG_VIA_EBX   0x20414956
/** @brief CPUID Vendor signature VIA ECX. */
#define SIG_VIA_ECX   0x20414956
/** @brief CPUID Vendor signature VIA EDX. */
#define SIG_VIA_EDX   0x20414956

/** @brief CPUID Vendor signature Vortex EBX. */
#define SIG_VORTEX_EBX    0x74726f56
/** @brief CPUID Vendor signature Vortex ECX. */
#define SIG_VORTEX_ECX    0x436f5320
/** @brief CPUID Vendor signature Vortex EDX. */
#define SIG_VORTEX_EDX    0x36387865

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define CPU_ASSERT(COND, MSG, ERROR) {                      \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "CPU", MSG, TRUE);                     \
    }                                                       \
}

#define CONCAT_STR(buff, idx, str) {                        \
        strcpy(buff + idx, str);                            \
        idx += strlen(str);                                 \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E cpu_raise_interrupt(const uint32_t interrupt_line)
{
    if(interrupt_line > MAX_INTERRUPT_LINE)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    switch(interrupt_line)
    {
        case 0:
            __asm__ __volatile__("int %0" :: "i" (0));
            break;
        case 1:
            __asm__ __volatile__("int %0" :: "i" (1));
            break;
        case 2:
            __asm__ __volatile__("int %0" :: "i" (2));
            break;
        case 3:
            __asm__ __volatile__("int %0" :: "i" (3));
            break;
        case 4:
            __asm__ __volatile__("int %0" :: "i" (4));
            break;
        case 5:
            __asm__ __volatile__("int %0" :: "i" (5));
            break;
        case 6:
            __asm__ __volatile__("int %0" :: "i" (6));
            break;
        case 7:
            __asm__ __volatile__("int %0" :: "i" (7));
            break;
        case 8:
            __asm__ __volatile__("int %0" :: "i" (8));
            break;
        case 9:
            __asm__ __volatile__("int %0" :: "i" (9));
            break;
        case 10:
            __asm__ __volatile__("int %0" :: "i" (10));
            break;
        case 11:
            __asm__ __volatile__("int %0" :: "i" (11));
            break;
        case 12:
            __asm__ __volatile__("int %0" :: "i" (12));
            break;
        case 13:
            __asm__ __volatile__("int %0" :: "i" (13));
            break;
        case 14:
            __asm__ __volatile__("int %0" :: "i" (14));
            break;
        case 15:
            __asm__ __volatile__("int %0" :: "i" (15));
            break;
        case 16:
            __asm__ __volatile__("int %0" :: "i" (16));
            break;
        case 17:
            __asm__ __volatile__("int %0" :: "i" (17));
            break;
        case 18:
            __asm__ __volatile__("int %0" :: "i" (18));
            break;
        case 19:
            __asm__ __volatile__("int %0" :: "i" (19));
            break;
        case 20:
            __asm__ __volatile__("int %0" :: "i" (20));
            break;
        case 21:
            __asm__ __volatile__("int %0" :: "i" (21));
            break;
        case 22:
            __asm__ __volatile__("int %0" :: "i" (22));
            break;
        case 23:
            __asm__ __volatile__("int %0" :: "i" (23));
            break;
        case 24:
            __asm__ __volatile__("int %0" :: "i" (24));
            break;
        case 25:
            __asm__ __volatile__("int %0" :: "i" (25));
            break;
        case 26:
            __asm__ __volatile__("int %0" :: "i" (26));
            break;
        case 27:
            __asm__ __volatile__("int %0" :: "i" (27));
            break;
        case 28:
            __asm__ __volatile__("int %0" :: "i" (28));
            break;
        case 29:
            __asm__ __volatile__("int %0" :: "i" (29));
            break;
        case 30:
            __asm__ __volatile__("int %0" :: "i" (30));
            break;
        case 31:
            __asm__ __volatile__("int %0" :: "i" (31));
            break;
        case 32:
            __asm__ __volatile__("int %0" :: "i" (32));
            break;
        case 33:
            __asm__ __volatile__("int %0" :: "i" (33));
            break;
        case 34:
            __asm__ __volatile__("int %0" :: "i" (34));
            break;
        case 35:
            __asm__ __volatile__("int %0" :: "i" (35));
            break;
        case 36:
            __asm__ __volatile__("int %0" :: "i" (36));
            break;
        case 37:
            __asm__ __volatile__("int %0" :: "i" (37));
            break;
        case 38:
            __asm__ __volatile__("int %0" :: "i" (38));
            break;
        case 39:
            __asm__ __volatile__("int %0" :: "i" (39));
            break;
        case 40:
            __asm__ __volatile__("int %0" :: "i" (40));
            break;
        case 41:
            __asm__ __volatile__("int %0" :: "i" (41));
            break;
        case 42:
            __asm__ __volatile__("int %0" :: "i" (42));
            break;
        case 43:
            __asm__ __volatile__("int %0" :: "i" (43));
            break;
        case 44:
            __asm__ __volatile__("int %0" :: "i" (44));
            break;
        case 45:
            __asm__ __volatile__("int %0" :: "i" (45));
            break;
        case 46:
            __asm__ __volatile__("int %0" :: "i" (46));
            break;
        case 47:
            __asm__ __volatile__("int %0" :: "i" (47));
            break;
        case 48:
            __asm__ __volatile__("int %0" :: "i" (48));
            break;
        case 49:
            __asm__ __volatile__("int %0" :: "i" (49));
            break;
        case 50:
            __asm__ __volatile__("int %0" :: "i" (50));
            break;
        case 51:
            __asm__ __volatile__("int %0" :: "i" (51));
            break;
        case 52:
            __asm__ __volatile__("int %0" :: "i" (52));
            break;
        case 53:
            __asm__ __volatile__("int %0" :: "i" (53));
            break;
        case 54:
            __asm__ __volatile__("int %0" :: "i" (54));
            break;
        case 55:
            __asm__ __volatile__("int %0" :: "i" (55));
            break;
        case 56:
            __asm__ __volatile__("int %0" :: "i" (56));
            break;
        case 57:
            __asm__ __volatile__("int %0" :: "i" (57));
            break;
        case 58:
            __asm__ __volatile__("int %0" :: "i" (58));
            break;
        case 59:
            __asm__ __volatile__("int %0" :: "i" (59));
            break;
        case 60:
            __asm__ __volatile__("int %0" :: "i" (60));
            break;
        case 61:
            __asm__ __volatile__("int %0" :: "i" (61));
            break;
        case 62:
            __asm__ __volatile__("int %0" :: "i" (62));
            break;
        case 63:
            __asm__ __volatile__("int %0" :: "i" (63));
            break;
        case 64:
            __asm__ __volatile__("int %0" :: "i" (64));
            break;
        case 65:
            __asm__ __volatile__("int %0" :: "i" (65));
            break;
        case 66:
            __asm__ __volatile__("int %0" :: "i" (66));
            break;
        case 67:
            __asm__ __volatile__("int %0" :: "i" (67));
            break;
        case 68:
            __asm__ __volatile__("int %0" :: "i" (68));
            break;
        case 69:
            __asm__ __volatile__("int %0" :: "i" (69));
            break;
        case 70:
            __asm__ __volatile__("int %0" :: "i" (70));
            break;
        case 71:
            __asm__ __volatile__("int %0" :: "i" (71));
            break;
        case 72:
            __asm__ __volatile__("int %0" :: "i" (72));
            break;
        case 73:
            __asm__ __volatile__("int %0" :: "i" (73));
            break;
        case 74:
            __asm__ __volatile__("int %0" :: "i" (74));
            break;
        case 75:
            __asm__ __volatile__("int %0" :: "i" (75));
            break;
        case 76:
            __asm__ __volatile__("int %0" :: "i" (76));
            break;
        case 77:
            __asm__ __volatile__("int %0" :: "i" (77));
            break;
        case 78:
            __asm__ __volatile__("int %0" :: "i" (78));
            break;
        case 79:
            __asm__ __volatile__("int %0" :: "i" (79));
            break;
        case 80:
            __asm__ __volatile__("int %0" :: "i" (80));
            break;
        case 81:
            __asm__ __volatile__("int %0" :: "i" (81));
            break;
        case 82:
            __asm__ __volatile__("int %0" :: "i" (82));
            break;
        case 83:
            __asm__ __volatile__("int %0" :: "i" (83));
            break;
        case 84:
            __asm__ __volatile__("int %0" :: "i" (84));
            break;
        case 85:
            __asm__ __volatile__("int %0" :: "i" (85));
            break;
        case 86:
            __asm__ __volatile__("int %0" :: "i" (86));
            break;
        case 87:
            __asm__ __volatile__("int %0" :: "i" (87));
            break;
        case 88:
            __asm__ __volatile__("int %0" :: "i" (88));
            break;
        case 89:
            __asm__ __volatile__("int %0" :: "i" (89));
            break;
        case 90:
            __asm__ __volatile__("int %0" :: "i" (90));
            break;
        case 91:
            __asm__ __volatile__("int %0" :: "i" (91));
            break;
        case 92:
            __asm__ __volatile__("int %0" :: "i" (92));
            break;
        case 93:
            __asm__ __volatile__("int %0" :: "i" (93));
            break;
        case 94:
            __asm__ __volatile__("int %0" :: "i" (94));
            break;
        case 95:
            __asm__ __volatile__("int %0" :: "i" (95));
            break;
        case 96:
            __asm__ __volatile__("int %0" :: "i" (96));
            break;
        case 97:
            __asm__ __volatile__("int %0" :: "i" (97));
            break;
        case 98:
            __asm__ __volatile__("int %0" :: "i" (98));
            break;
        case 99:
            __asm__ __volatile__("int %0" :: "i" (99));
            break;
        case 100:
            __asm__ __volatile__("int %0" :: "i" (100));
            break;
        case 101:
            __asm__ __volatile__("int %0" :: "i" (101));
            break;
        case 102:
            __asm__ __volatile__("int %0" :: "i" (102));
            break;
        case 103:
            __asm__ __volatile__("int %0" :: "i" (103));
            break;
        case 104:
            __asm__ __volatile__("int %0" :: "i" (104));
            break;
        case 105:
            __asm__ __volatile__("int %0" :: "i" (105));
            break;
        case 106:
            __asm__ __volatile__("int %0" :: "i" (106));
            break;
        case 107:
            __asm__ __volatile__("int %0" :: "i" (107));
            break;
        case 108:
            __asm__ __volatile__("int %0" :: "i" (108));
            break;
        case 109:
            __asm__ __volatile__("int %0" :: "i" (109));
            break;
        case 110:
            __asm__ __volatile__("int %0" :: "i" (110));
            break;
        case 111:
            __asm__ __volatile__("int %0" :: "i" (111));
            break;
        case 112:
            __asm__ __volatile__("int %0" :: "i" (112));
            break;
        case 113:
            __asm__ __volatile__("int %0" :: "i" (113));
            break;
        case 114:
            __asm__ __volatile__("int %0" :: "i" (114));
            break;
        case 115:
            __asm__ __volatile__("int %0" :: "i" (115));
            break;
        case 116:
            __asm__ __volatile__("int %0" :: "i" (116));
            break;
        case 117:
            __asm__ __volatile__("int %0" :: "i" (117));
            break;
        case 118:
            __asm__ __volatile__("int %0" :: "i" (118));
            break;
        case 119:
            __asm__ __volatile__("int %0" :: "i" (119));
            break;
        case 120:
            __asm__ __volatile__("int %0" :: "i" (120));
            break;
        case 121:
            __asm__ __volatile__("int %0" :: "i" (121));
            break;
        case 122:
            __asm__ __volatile__("int %0" :: "i" (122));
            break;
        case 123:
            __asm__ __volatile__("int %0" :: "i" (123));
            break;
        case 124:
            __asm__ __volatile__("int %0" :: "i" (124));
            break;
        case 125:
            __asm__ __volatile__("int %0" :: "i" (125));
            break;
        case 126:
            __asm__ __volatile__("int %0" :: "i" (126));
            break;
        case 127:
            __asm__ __volatile__("int %0" :: "i" (127));
            break;
        case 128:
            __asm__ __volatile__("int %0" :: "i" (128));
            break;
        case 129:
            __asm__ __volatile__("int %0" :: "i" (129));
            break;
        case 130:
            __asm__ __volatile__("int %0" :: "i" (130));
            break;
        case 131:
            __asm__ __volatile__("int %0" :: "i" (131));
            break;
        case 132:
            __asm__ __volatile__("int %0" :: "i" (132));
            break;
        case 133:
            __asm__ __volatile__("int %0" :: "i" (133));
            break;
        case 134:
            __asm__ __volatile__("int %0" :: "i" (134));
            break;
        case 135:
            __asm__ __volatile__("int %0" :: "i" (135));
            break;
        case 136:
            __asm__ __volatile__("int %0" :: "i" (136));
            break;
        case 137:
            __asm__ __volatile__("int %0" :: "i" (137));
            break;
        case 138:
            __asm__ __volatile__("int %0" :: "i" (138));
            break;
        case 139:
            __asm__ __volatile__("int %0" :: "i" (139));
            break;
        case 140:
            __asm__ __volatile__("int %0" :: "i" (140));
            break;
        case 141:
            __asm__ __volatile__("int %0" :: "i" (141));
            break;
        case 142:
            __asm__ __volatile__("int %0" :: "i" (142));
            break;
        case 143:
            __asm__ __volatile__("int %0" :: "i" (143));
            break;
        case 144:
            __asm__ __volatile__("int %0" :: "i" (144));
            break;
        case 145:
            __asm__ __volatile__("int %0" :: "i" (145));
            break;
        case 146:
            __asm__ __volatile__("int %0" :: "i" (146));
            break;
        case 147:
            __asm__ __volatile__("int %0" :: "i" (147));
            break;
        case 148:
            __asm__ __volatile__("int %0" :: "i" (148));
            break;
        case 149:
            __asm__ __volatile__("int %0" :: "i" (149));
            break;
        case 150:
            __asm__ __volatile__("int %0" :: "i" (150));
            break;
        case 151:
            __asm__ __volatile__("int %0" :: "i" (151));
            break;
        case 152:
            __asm__ __volatile__("int %0" :: "i" (152));
            break;
        case 153:
            __asm__ __volatile__("int %0" :: "i" (153));
            break;
        case 154:
            __asm__ __volatile__("int %0" :: "i" (154));
            break;
        case 155:
            __asm__ __volatile__("int %0" :: "i" (155));
            break;
        case 156:
            __asm__ __volatile__("int %0" :: "i" (156));
            break;
        case 157:
            __asm__ __volatile__("int %0" :: "i" (157));
            break;
        case 158:
            __asm__ __volatile__("int %0" :: "i" (158));
            break;
        case 159:
            __asm__ __volatile__("int %0" :: "i" (159));
            break;
        case 160:
            __asm__ __volatile__("int %0" :: "i" (160));
            break;
        case 161:
            __asm__ __volatile__("int %0" :: "i" (161));
            break;
        case 162:
            __asm__ __volatile__("int %0" :: "i" (162));
            break;
        case 163:
            __asm__ __volatile__("int %0" :: "i" (163));
            break;
        case 164:
            __asm__ __volatile__("int %0" :: "i" (164));
            break;
        case 165:
            __asm__ __volatile__("int %0" :: "i" (165));
            break;
        case 166:
            __asm__ __volatile__("int %0" :: "i" (166));
            break;
        case 167:
            __asm__ __volatile__("int %0" :: "i" (167));
            break;
        case 168:
            __asm__ __volatile__("int %0" :: "i" (168));
            break;
        case 169:
            __asm__ __volatile__("int %0" :: "i" (169));
            break;
        case 170:
            __asm__ __volatile__("int %0" :: "i" (170));
            break;
        case 171:
            __asm__ __volatile__("int %0" :: "i" (171));
            break;
        case 172:
            __asm__ __volatile__("int %0" :: "i" (172));
            break;
        case 173:
            __asm__ __volatile__("int %0" :: "i" (173));
            break;
        case 174:
            __asm__ __volatile__("int %0" :: "i" (174));
            break;
        case 175:
            __asm__ __volatile__("int %0" :: "i" (175));
            break;
        case 176:
            __asm__ __volatile__("int %0" :: "i" (176));
            break;
        case 177:
            __asm__ __volatile__("int %0" :: "i" (177));
            break;
        case 178:
            __asm__ __volatile__("int %0" :: "i" (178));
            break;
        case 179:
            __asm__ __volatile__("int %0" :: "i" (179));
            break;
        case 180:
            __asm__ __volatile__("int %0" :: "i" (180));
            break;
        case 181:
            __asm__ __volatile__("int %0" :: "i" (181));
            break;
        case 182:
            __asm__ __volatile__("int %0" :: "i" (182));
            break;
        case 183:
            __asm__ __volatile__("int %0" :: "i" (183));
            break;
        case 184:
            __asm__ __volatile__("int %0" :: "i" (184));
            break;
        case 185:
            __asm__ __volatile__("int %0" :: "i" (185));
            break;
        case 186:
            __asm__ __volatile__("int %0" :: "i" (186));
            break;
        case 187:
            __asm__ __volatile__("int %0" :: "i" (187));
            break;
        case 188:
            __asm__ __volatile__("int %0" :: "i" (188));
            break;
        case 189:
            __asm__ __volatile__("int %0" :: "i" (189));
            break;
        case 190:
            __asm__ __volatile__("int %0" :: "i" (190));
            break;
        case 191:
            __asm__ __volatile__("int %0" :: "i" (191));
            break;
        case 192:
            __asm__ __volatile__("int %0" :: "i" (192));
            break;
        case 193:
            __asm__ __volatile__("int %0" :: "i" (193));
            break;
        case 194:
            __asm__ __volatile__("int %0" :: "i" (194));
            break;
        case 195:
            __asm__ __volatile__("int %0" :: "i" (195));
            break;
        case 196:
            __asm__ __volatile__("int %0" :: "i" (196));
            break;
        case 197:
            __asm__ __volatile__("int %0" :: "i" (197));
            break;
        case 198:
            __asm__ __volatile__("int %0" :: "i" (198));
            break;
        case 199:
            __asm__ __volatile__("int %0" :: "i" (199));
            break;
        case 200:
            __asm__ __volatile__("int %0" :: "i" (200));
            break;
        case 201:
            __asm__ __volatile__("int %0" :: "i" (201));
            break;
        case 202:
            __asm__ __volatile__("int %0" :: "i" (202));
            break;
        case 203:
            __asm__ __volatile__("int %0" :: "i" (203));
            break;
        case 204:
            __asm__ __volatile__("int %0" :: "i" (204));
            break;
        case 205:
            __asm__ __volatile__("int %0" :: "i" (205));
            break;
        case 206:
            __asm__ __volatile__("int %0" :: "i" (206));
            break;
        case 207:
            __asm__ __volatile__("int %0" :: "i" (207));
            break;
        case 208:
            __asm__ __volatile__("int %0" :: "i" (208));
            break;
        case 209:
            __asm__ __volatile__("int %0" :: "i" (209));
            break;
        case 210:
            __asm__ __volatile__("int %0" :: "i" (210));
            break;
        case 211:
            __asm__ __volatile__("int %0" :: "i" (211));
            break;
        case 212:
            __asm__ __volatile__("int %0" :: "i" (212));
            break;
        case 213:
            __asm__ __volatile__("int %0" :: "i" (213));
            break;
        case 214:
            __asm__ __volatile__("int %0" :: "i" (214));
            break;
        case 215:
            __asm__ __volatile__("int %0" :: "i" (215));
            break;
        case 216:
            __asm__ __volatile__("int %0" :: "i" (216));
            break;
        case 217:
            __asm__ __volatile__("int %0" :: "i" (217));
            break;
        case 218:
            __asm__ __volatile__("int %0" :: "i" (218));
            break;
        case 219:
            __asm__ __volatile__("int %0" :: "i" (219));
            break;
        case 220:
            __asm__ __volatile__("int %0" :: "i" (220));
            break;
        case 221:
            __asm__ __volatile__("int %0" :: "i" (221));
            break;
        case 222:
            __asm__ __volatile__("int %0" :: "i" (222));
            break;
        case 223:
            __asm__ __volatile__("int %0" :: "i" (223));
            break;
        case 224:
            __asm__ __volatile__("int %0" :: "i" (224));
            break;
        case 225:
            __asm__ __volatile__("int %0" :: "i" (225));
            break;
        case 226:
            __asm__ __volatile__("int %0" :: "i" (226));
            break;
        case 227:
            __asm__ __volatile__("int %0" :: "i" (227));
            break;
        case 228:
            __asm__ __volatile__("int %0" :: "i" (228));
            break;
        case 229:
            __asm__ __volatile__("int %0" :: "i" (229));
            break;
        case 230:
            __asm__ __volatile__("int %0" :: "i" (230));
            break;
        case 231:
            __asm__ __volatile__("int %0" :: "i" (231));
            break;
        case 232:
            __asm__ __volatile__("int %0" :: "i" (232));
            break;
        case 233:
            __asm__ __volatile__("int %0" :: "i" (233));
            break;
        case 234:
            __asm__ __volatile__("int %0" :: "i" (234));
            break;
        case 235:
            __asm__ __volatile__("int %0" :: "i" (235));
            break;
        case 236:
            __asm__ __volatile__("int %0" :: "i" (236));
            break;
        case 237:
            __asm__ __volatile__("int %0" :: "i" (237));
            break;
        case 238:
            __asm__ __volatile__("int %0" :: "i" (238));
            break;
        case 239:
            __asm__ __volatile__("int %0" :: "i" (239));
            break;
        case 240:
            __asm__ __volatile__("int %0" :: "i" (240));
            break;
        case 241:
            __asm__ __volatile__("int %0" :: "i" (241));
            break;
        case 242:
            __asm__ __volatile__("int %0" :: "i" (242));
            break;
        case 243:
            __asm__ __volatile__("int %0" :: "i" (243));
            break;
        case 244:
            __asm__ __volatile__("int %0" :: "i" (244));
            break;
        case 245:
            __asm__ __volatile__("int %0" :: "i" (245));
            break;
        case 246:
            __asm__ __volatile__("int %0" :: "i" (246));
            break;
        case 247:
            __asm__ __volatile__("int %0" :: "i" (247));
            break;
        case 248:
            __asm__ __volatile__("int %0" :: "i" (248));
            break;
        case 249:
            __asm__ __volatile__("int %0" :: "i" (249));
            break;
        case 250:
            __asm__ __volatile__("int %0" :: "i" (250));
            break;
        case 251:
            __asm__ __volatile__("int %0" :: "i" (251));
            break;
        case 252:
            __asm__ __volatile__("int %0" :: "i" (252));
            break;
        case 253:
            __asm__ __volatile__("int %0" :: "i" (253));
            break;
        case 254:
            __asm__ __volatile__("int %0" :: "i" (254));
            break;
        case 255:
            __asm__ __volatile__("int %0" :: "i" (255));
            break;
    }

    kernel_interrupt_set_irq_eoi(interrupt_line);

    return OS_NO_ERR;
}

uint32_t cpu_get_interrupt_state(void)
{
    return ((cpu_save_flags() & CPU_EFLAGS_IF) != 0);
}

uint32_t cpu_get_saved_interrupt_state(const cpu_state_t* cpu_state,
                                       const stack_state_t* stack_state)
{
    (void) cpu_state;
    return stack_state->eflags & CPU_EFLAGS_IF;
}

void cpu_init_thread_context(void (*entry_point)(void),
                             kernel_thread_t* thread)
{
    uintptr_t stack_index;

    stack_index = thread->kstack_size / sizeof(uintptr_t);

    /* Set EIP, ESP and EBP */
    thread->cpu_context.eip = (uintptr_t)entry_point;
    thread->cpu_context.esp = thread->kstack + (stack_index - 18) *
                              sizeof(uintptr_t);
    thread->cpu_context.ebp = thread->kstack + (stack_index - 1) *
                               sizeof(uintptr_t);

    /* Init thread kernel stack */
    ((uintptr_t*)thread->kstack)[stack_index - 1]  = K_THREAD_INIT_EFLAGS;
    ((uintptr_t*)thread->kstack)[stack_index - 2]  = K_THREAD_INIT_CS;
    ((uintptr_t*)thread->kstack)[stack_index - 3]  = thread->cpu_context.eip;
    ((uintptr_t*)thread->kstack)[stack_index - 4]  = 0; /* UNUSED (error) */
    ((uintptr_t*)thread->kstack)[stack_index - 5]  = 0; /* UNUSED (int id) */
    ((uintptr_t*)thread->kstack)[stack_index - 6]  = K_THREAD_INIT_DS;
    ((uintptr_t*)thread->kstack)[stack_index - 7]  = K_THREAD_INIT_ES;
    ((uintptr_t*)thread->kstack)[stack_index - 8]  = K_THREAD_INIT_FS;
    ((uintptr_t*)thread->kstack)[stack_index - 9]  = K_THREAD_INIT_GS;
    ((uintptr_t*)thread->kstack)[stack_index - 10] = K_THREAD_INIT_SS;
    ((uintptr_t*)thread->kstack)[stack_index - 11] = K_THREAD_INIT_EAX;
    ((uintptr_t*)thread->kstack)[stack_index - 12] = K_THREAD_INIT_EBX;
    ((uintptr_t*)thread->kstack)[stack_index - 13] = K_THREAD_INIT_ECX;
    ((uintptr_t*)thread->kstack)[stack_index - 14] = K_THREAD_INIT_EDX;
    ((uintptr_t*)thread->kstack)[stack_index - 15] = K_THREAD_INIT_ESI;
    ((uintptr_t*)thread->kstack)[stack_index - 16] = K_THREAD_INIT_EDI;
    ((uintptr_t*)thread->kstack)[stack_index - 17] = thread->cpu_context.ebp;
    ((uintptr_t*)thread->kstack)[stack_index - 18] =
                                        thread->kstack + (stack_index - 17) *
                                        sizeof(uintptr_t);
}

uintptr_t cpu_get_current_pgdir(void)
{
    uintptr_t current_pgdir;

    /* Init thread context */
    __asm__ __volatile__(
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=m" (current_pgdir)
        : /* no input */
        : "%eax"
    );

    return current_pgdir;
}

void cpu_save_context(const cpu_state_t* cpu_state,
                      const stack_state_t* stack_state,
                      kernel_thread_t* thread)
{
    (void)stack_state;
    thread->cpu_context.esp = (uintptr_t)&cpu_state->esp;
}

void cpu_restore_context(cpu_state_t* cpu_state,
                         const stack_state_t* stack_state,
                         const kernel_thread_t* thread)
{
    (void)stack_state;
    (void)cpu_state;

    /* On context restore, the CR0.TS bit is set to catch FPU/SSE use
     * TODO: Set it back when FPU saving is supported
     */
#if 0
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n\t"
        "or  $0x00000008, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
    :::"eax");
#endif
    __asm__ __volatile__(
        "mov  %%eax, %%cr3\n\t"
        "mov  %%edx, %%esp\n\t"
        "pop  %%esp\n\t"
        "pop  %%ebp\n\t"
        "pop  %%edi\n\t"
        "pop  %%esi\n\t"
        "pop  %%edx\n\t"
        "pop  %%ecx\n\t"
        "pop  %%ebx\n\t"
        "pop  %%eax\n\t"
        "pop  %%ss\n\t"
        "pop  %%gs\n\t"
        "pop  %%fs\n\t"
        "pop  %%es\n\t"
        "pop  %%ds\n\t"
        "add  $8, %%esp\n\t"
        "iret\n\t"
        : :"a"(thread->process->page_dir), "d"(thread->cpu_context.esp));

    CPU_ASSERT(FALSE,
               "Returned from context restore",
               OS_ERR_UNAUTHORIZED_ACTION);
}

void cpu_syscall(uint32_t syscall_id, void* params)
{
    __asm__ __volatile__(
        "mov %0, %%ecx\n\t"
        "mov %1, %%edx\n\t"
        "int %2\n\t"
        :
        : "r" (syscall_id), "r" (params), "i" (SYSCALL_INT_LINE)
        : "ecx", "edx");
}

void cpu_get_syscall_data(const cpu_state_t* cpu_state,
                          const stack_state_t* stack_state,
                          uint32_t* syscall_id,
                          void** params)
{
    (void)stack_state;

    /* On the i386, the function ID is in ECX and the prameters in EDX */
    if(syscall_id != NULL)
    {
        *syscall_id = cpu_state->ecx;
    }
    if(params != NULL)
    {
        *params = (void*)cpu_state->edx;
    }
}

void cpu_switch_user_mode(void)
{
    /* Setup stack requirement and performs switch to user mode */
    __asm__ __volatile__(
        "cli                             \n\t"
        "mov %0, %%ax                    \n\t"
        "mov %%ax, %%ds                  \n\t"
        "mov %%ax, %%es                  \n\t"
        "mov %%ax, %%fs                  \n\t"
        "mov %%ax, %%gs                  \n\t"
        "mov %%esp, %%eax                \n\t"
        "push %0                         \n\t"
        "push %%eax                      \n\t"
        "pushf                           \n\t"
        "pop %%eax                       \n\t"
        "or $0x200, %%eax                \n\t"
        "push %%eax                      \n\t"
        "push %1                         \n\t"
        "push $user_mode_entry           \n\t"
        "iret                            \n\t"
        "user_mode_entry:                \n\t"
        :
        : "i" (USER_DS_32 | 0x3) , "i" (USER_CS_32 | 0x3)
        : "eax");
}

void validate_architecture(void)
{
    /* eax, ebx, ecx, edx */
    volatile int32_t  regs[4];
    volatile int32_t  regs_ext[4];
    uint32_t          ret;

#if KERNEL_LOG_LEVEL >= INFO_LOG_LEVEL
    uint32_t output_buff_index;
    char     output_buff[512];
    char     vendor_str[26] = "CPU Vendor:             \n\0";

    output_buff_index = 0;
#endif

    KERNEL_DEBUG(KICKSTART_DEBUG_ENABLED, "[KICKSTART] Detecting cpu");

    ret = cpu_cpuid(CPUID_GETVENDORSTRING, (uint32_t*)regs);

    CPU_ASSERT(ret != 0,
               "CPU does not support CPUID",
               OS_ERR_NOT_SUPPORTED);

    /* Check if CPUID return more that one available function */

#if KERNEL_LOG_LEVEL >= INFO_LOG_LEVEL
    for(int8_t j = 0; j < 4; ++j)
    {
        vendor_str[12 + j] = (char)((regs[1] >> (j * 8)) & 0xFF);
    }
    for(int8_t j = 0; j < 4; ++j)
    {
        vendor_str[16 + j] = (char)((regs[3] >> (j * 8)) & 0xFF);
    }
    for(int8_t j = 0; j < 4; ++j)
    {
        vendor_str[20 + j] = (char)((regs[2] >> (j * 8)) & 0xFF);
    }

    KERNEL_INFO(vendor_str);
#endif

    /* Get CPUID features */
    cpu_cpuid(CPUID_GETFEATURES, (uint32_t*)regs);

#if KERNEL_LOG_LEVEL >= INFO_LOG_LEVEL
    memset(output_buff, 0, 512 * sizeof(char));
    strncpy(output_buff, "CPU Features: ", 14);
    output_buff_index = 14;

    if((regs[2] & ECX_SSE3) == ECX_SSE3)
    { CONCAT_STR(output_buff, output_buff_index, "SSE3 - "); }
    if((regs[2] & ECX_PCLMULQDQ) == ECX_PCLMULQDQ)
    { CONCAT_STR(output_buff, output_buff_index, "PCLMULQDQ - "); }
    if((regs[2] & ECX_DTES64) == ECX_DTES64)
    { CONCAT_STR(output_buff, output_buff_index, "DTES64 - "); }
    if((regs[2] & ECX_MONITOR) == ECX_MONITOR)
    { CONCAT_STR(output_buff, output_buff_index, "MONITOR - "); }
    if((regs[2] & ECX_DS_CPL) == ECX_DS_CPL)
    { CONCAT_STR(output_buff, output_buff_index, "DS_CPL - "); }
    if((regs[2] & ECX_VMX) == ECX_VMX)
    { CONCAT_STR(output_buff, output_buff_index, "VMX - "); }
    if((regs[2] & ECX_SMX) == ECX_SMX)
    { CONCAT_STR(output_buff, output_buff_index, "SMX - "); }
    if((regs[2] & ECX_EST) == ECX_EST)
    { CONCAT_STR(output_buff, output_buff_index, "EST - "); }
    if((regs[2] & ECX_TM2) == ECX_TM2)
    { CONCAT_STR(output_buff, output_buff_index, "TM2 - "); }
    if((regs[2] & ECX_SSSE3) == ECX_SSSE3)
    { CONCAT_STR(output_buff, output_buff_index, "SSSE3 - "); }
    if((regs[2] & ECX_CNXT_ID) == ECX_CNXT_ID)
    { CONCAT_STR(output_buff, output_buff_index, "CNXT_ID - "); }
    if((regs[2] & ECX_FMA) == ECX_FMA)
    { CONCAT_STR(output_buff, output_buff_index, "FMA - "); }
    if((regs[2] & ECX_CX16) == ECX_CX16)
    { CONCAT_STR(output_buff, output_buff_index, "CX16 - "); }
    if((regs[2] & ECX_XTPR) == ECX_XTPR)
    { CONCAT_STR(output_buff, output_buff_index, "XTPR - "); }
    if((regs[2] & ECX_PDCM) == ECX_PDCM)
    { CONCAT_STR(output_buff, output_buff_index, "PDCM - "); }
    if((regs[2] & ECX_PCID) == ECX_PCID)
    { CONCAT_STR(output_buff, output_buff_index, "PCID - "); }
    if((regs[2] & ECX_DCA) == ECX_DCA)
    { CONCAT_STR(output_buff, output_buff_index, "DCA - "); }
    if((regs[2] & ECX_SSE41) == ECX_SSE41)
    { CONCAT_STR(output_buff, output_buff_index, "SSE41 - "); }
    if((regs[2] & ECX_SSE42) == ECX_SSE42)
    { CONCAT_STR(output_buff, output_buff_index, "SSE42 - "); }
    if((regs[2] & ECX_X2APIC) == ECX_X2APIC)
    { CONCAT_STR(output_buff, output_buff_index, "X2APIC - "); }
    if((regs[2] & ECX_MOVBE) == ECX_MOVBE)
    { CONCAT_STR(output_buff, output_buff_index, "MOVBE - "); }
    if((regs[2] & ECX_POPCNT) == ECX_POPCNT)
    { CONCAT_STR(output_buff, output_buff_index, "POPCNT - "); }
    if((regs[2] & ECX_TSC) == ECX_TSC)
    { CONCAT_STR(output_buff, output_buff_index, "TSC - "); }
    if((regs[2] & ECX_AESNI) == ECX_AESNI)
    { CONCAT_STR(output_buff, output_buff_index, "AESNI - "); }
    if((regs[2] & ECX_XSAVE) == ECX_XSAVE)
    { CONCAT_STR(output_buff, output_buff_index, "XSAVE - "); }
    if((regs[2] & ECX_OSXSAVE) == ECX_OSXSAVE)
    { CONCAT_STR(output_buff, output_buff_index, "OSXSAVE - "); }
    if((regs[2] & ECX_AVX) == ECX_AVX)
    { CONCAT_STR(output_buff, output_buff_index, "AVX - "); }
    if((regs[2] & ECX_F16C) == ECX_F16C)
    { CONCAT_STR(output_buff, output_buff_index, "F16C - "); }
    if((regs[2] & ECX_RDRAND) == ECX_RDRAND)
    { CONCAT_STR(output_buff, output_buff_index, "RDRAND - "); }
    if((regs[3] & EDX_FPU) == EDX_FPU)
    { CONCAT_STR(output_buff, output_buff_index, "FPU - "); }
    if((regs[3] & EDX_VME) == EDX_VME)
    { CONCAT_STR(output_buff, output_buff_index, "VME - "); }
    if((regs[3] & EDX_DE) == EDX_DE)
    { CONCAT_STR(output_buff, output_buff_index, "DE - "); }
    if((regs[3] & EDX_PSE) == EDX_PSE)
    { CONCAT_STR(output_buff, output_buff_index, "PSE - "); }
    if((regs[3] & EDX_TSC) == EDX_TSC)
    { CONCAT_STR(output_buff, output_buff_index, "TSC - "); }
    if((regs[3] & EDX_MSR) == EDX_MSR)
    { CONCAT_STR(output_buff, output_buff_index, "MSR - "); }
    if((regs[3] & EDX_PAE) == EDX_PAE)
    { CONCAT_STR(output_buff, output_buff_index, "PAE - "); }
    if((regs[3] & EDX_MCE) == EDX_MCE)
    { CONCAT_STR(output_buff, output_buff_index, "MCE - "); }
    if((regs[3] & EDX_CX8) == EDX_CX8)
    { CONCAT_STR(output_buff, output_buff_index, "CX8 - "); }
    if((regs[3] & EDX_APIC) == EDX_APIC)
    { CONCAT_STR(output_buff, output_buff_index, "APIC - "); }
    if((regs[3] & EDX_SEP) == EDX_SEP)
    { CONCAT_STR(output_buff, output_buff_index, "SEP - "); }
    if((regs[3] & EDX_MTRR) == EDX_MTRR)
    { CONCAT_STR(output_buff, output_buff_index, "MTRR - "); }
    if((regs[3] & EDX_PGE) == EDX_PGE)
    { CONCAT_STR(output_buff, output_buff_index, "PGE - "); }
    if((regs[3] & EDX_MCA) == EDX_MCA)
    { CONCAT_STR(output_buff, output_buff_index, "MCA - "); }
    if((regs[3] & EDX_CMOV) == EDX_CMOV)
    { CONCAT_STR(output_buff, output_buff_index, "CMOV - "); }
    if((regs[3] & EDX_PAT) == EDX_PAT)
    { CONCAT_STR(output_buff, output_buff_index, "PAT - "); }
    if((regs[3] & EDX_PSE36) == EDX_PSE36)
    { CONCAT_STR(output_buff, output_buff_index, "PSE36 - "); }
    if((regs[3] & EDX_PSN) == EDX_PSN)
    { CONCAT_STR(output_buff, output_buff_index, "PSN - "); }
    if((regs[3] & EDX_CLFLUSH) == EDX_CLFLUSH)
    { CONCAT_STR(output_buff, output_buff_index, "CLFLUSH - "); }
    if((regs[3] & EDX_DS) == EDX_DS)
    { CONCAT_STR(output_buff, output_buff_index, "DS - "); }
    if((regs[3] & EDX_ACPI) == EDX_ACPI)
    { CONCAT_STR(output_buff, output_buff_index, "ACPI - "); }
    if((regs[3] & EDX_MMX) == EDX_MMX)
    { CONCAT_STR(output_buff, output_buff_index, "MMX - "); }
    if((regs[3] & EDX_FXSR) == EDX_FXSR)
    { CONCAT_STR(output_buff, output_buff_index, "FXSR - "); }
    if((regs[3] & EDX_SSE) == EDX_SSE)
    { CONCAT_STR(output_buff, output_buff_index, "SSE - "); }
    if((regs[3] & EDX_SSE2) == EDX_SSE2)
    { CONCAT_STR(output_buff, output_buff_index, "SSE2 - "); }
    if((regs[3] & EDX_SS) == EDX_SS)
    { CONCAT_STR(output_buff, output_buff_index, "SS - "); }
    if((regs[3] & EDX_HTT) == EDX_HTT)
    { CONCAT_STR(output_buff, output_buff_index, "HTT - "); }
    if((regs[3] & EDX_TM) == EDX_TM)
    { CONCAT_STR(output_buff, output_buff_index, "TM - "); }
    if((regs[3] & EDX_PBE) == EDX_PBE)
    { CONCAT_STR(output_buff, output_buff_index, "PBE - "); }

    /* Check for extended features */
    cpu_cpuid(CPUID_INTELEXTENDED_AVAILABLE, (uint32_t*)regs_ext);
    if((uint32_t)regs_ext[0] >= (uint32_t)CPUID_INTELFEATURES)
    {
        cpu_cpuid(CPUID_INTELFEATURES, (uint32_t*)regs_ext);

        if((regs_ext[3] & EDX_SYSCALL) == EDX_SYSCALL)
        { CONCAT_STR(output_buff, output_buff_index, "SYSCALL - "); }
        if((regs_ext[3] & EDX_MP) == EDX_MP)
        { CONCAT_STR(output_buff, output_buff_index, "MP - "); }
        if((regs_ext[3] & EDX_XD) == EDX_XD)
        { CONCAT_STR(output_buff, output_buff_index, "XD - "); }
        if((regs_ext[3] & EDX_MMX_EX) == EDX_MMX_EX)
        { CONCAT_STR(output_buff, output_buff_index, "MMX_EX - "); }
        if((regs_ext[3] & EDX_FXSR) == EDX_FXSR)
        { CONCAT_STR(output_buff, output_buff_index, "FXSR - "); }
        if((regs_ext[3] & EDX_FXSR_OPT) == EDX_FXSR_OPT)
        { CONCAT_STR(output_buff, output_buff_index, "FXSR_OPT - "); }
        if((regs_ext[3] & EDX_1GB_PAGE) == EDX_1GB_PAGE)
        { CONCAT_STR(output_buff, output_buff_index, "1GB_PAGE - "); }
        if((regs_ext[3] & EDX_RDTSCP) == EDX_RDTSCP)
        { CONCAT_STR(output_buff, output_buff_index, "RDTSCP - "); }
        if((regs_ext[3] & EDX_64_BIT) == EDX_64_BIT)
        { CONCAT_STR(output_buff, output_buff_index, "X64 - "); }
        if((regs_ext[3] & EDX_3DNOW_EX) == EDX_3DNOW_EX)
        { CONCAT_STR(output_buff, output_buff_index, "3DNOW_EX - "); }
        if((regs_ext[3] & EDX_3DNOW) == EDX_3DNOW)
        { CONCAT_STR(output_buff, output_buff_index, "3DNOW - "); }
        if((regs_ext[2] & ECX_LAHF_LM) == ECX_LAHF_LM)
        { CONCAT_STR(output_buff, output_buff_index, "LAHF_LM - "); }
        if((regs_ext[2] & ECX_CMP_LEG) == ECX_CMP_LEG)
        { CONCAT_STR(output_buff, output_buff_index, "CMP_LEG - "); }
        if((regs_ext[2] & ECX_SVM) == ECX_SVM)
        { CONCAT_STR(output_buff, output_buff_index, "SVM - "); }
        if((regs_ext[2] & ECX_EXTAPIC) == ECX_EXTAPIC)
        { CONCAT_STR(output_buff, output_buff_index, "EXTAPIC - "); }
        if((regs_ext[2] & ECX_CR8_LEG) == ECX_CR8_LEG)
        { CONCAT_STR(output_buff, output_buff_index, "CR8_LEG - "); }
        if((regs_ext[2] & ECX_ABM) == ECX_ABM)
        { CONCAT_STR(output_buff, output_buff_index, "ABM - "); }
        if((regs_ext[2] & ECX_SSE4A) == ECX_SSE4A)
        { CONCAT_STR(output_buff, output_buff_index, "SSE4A - "); }
        if((regs_ext[2] & ECX_MISASSE) == ECX_MISASSE)
        { CONCAT_STR(output_buff, output_buff_index, "MISALIGNED_SSE - "); }
        if((regs_ext[2] & ECX_PREFETCH) == ECX_PREFETCH)
        { CONCAT_STR(output_buff, output_buff_index, "PREFETCH - "); }
        if((regs_ext[2] & ECX_OSVW) == ECX_OSVW)
        { CONCAT_STR(output_buff, output_buff_index, "OSVW - "); }
        if((regs_ext[2] & ECX_IBS) == ECX_IBS)
        { CONCAT_STR(output_buff, output_buff_index, "IBS - "); }
        if((regs_ext[2] & ECX_XOP) == ECX_XOP)
        { CONCAT_STR(output_buff, output_buff_index, "XOP - "); }
        if((regs_ext[2] & ECX_SKINIT) == ECX_SKINIT)
        { CONCAT_STR(output_buff, output_buff_index, "SKINIT - "); }
        if((regs_ext[2] & ECX_WDT) == ECX_WDT)
        { CONCAT_STR(output_buff, output_buff_index, "WDT - "); }
        if((regs_ext[2] & ECX_LWP) == ECX_LWP)
        { CONCAT_STR(output_buff, output_buff_index, "LWP - "); }
        if((regs_ext[2] & ECX_FMA4) == ECX_FMA4)
        { CONCAT_STR(output_buff, output_buff_index, "FMA4 - "); }
        if((regs_ext[2] & ECX_TCE) == ECX_TCE)
        { CONCAT_STR(output_buff, output_buff_index, "TCE - "); }
        if((regs_ext[2] & ECX_NODEIDMSR) == ECX_NODEIDMSR)
        { CONCAT_STR(output_buff, output_buff_index, "NODE_ID_MSR - "); }
        if((regs_ext[2] & ECX_TBM) == ECX_TBM)
        { CONCAT_STR(output_buff, output_buff_index, "TMB - "); }
        if((regs_ext[2] & ECX_TOPOEX) == ECX_TOPOEX)
        { CONCAT_STR(output_buff, output_buff_index, "TOPOEX - "); }
        if((regs_ext[2] & ECX_PERF_CORE) == ECX_PERF_CORE)
        { CONCAT_STR(output_buff, output_buff_index, "PERF_CORE - "); }
        if((regs_ext[2] & ECX_PERF_NB) == ECX_PERF_NB)
        { CONCAT_STR(output_buff, output_buff_index, "PERF_NB - "); }
        if((regs_ext[2] & ECX_DBX) == ECX_DBX)
        { CONCAT_STR(output_buff, output_buff_index, "DBX - "); }
        if((regs_ext[2] & ECX_PERF_TSC) == ECX_PERF_TSC)
        { CONCAT_STR(output_buff, output_buff_index, "TSC - "); }
        if((regs_ext[2] & ECX_PCX_L2I) == ECX_PCX_L2I)
        { CONCAT_STR(output_buff, output_buff_index, "PCX_L2I - "); }
    }

    output_buff[output_buff_index - 2] = '\n';
    output_buff[output_buff_index - 1] = 0;
    KERNEL_INFO(output_buff);
#endif

    /* Validate features */
    CPU_ASSERT((regs[3] & EDX_SEP) == EDX_SEP,
               "CPU does not support SYSENTER",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_FPU) == EDX_FPU,
               "CPU does not support FPU",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_TSC) == EDX_TSC,
               "CPU does not support TSC",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_APIC) == EDX_APIC,
               "CPU does not support APIC",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_FXSR) == EDX_FXSR,
               "CPU does not support FX instructions",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_SSE) == EDX_SSE,
               "CPU does not support SSE",
               OS_ERR_NOT_SUPPORTED);
    CPU_ASSERT((regs[3] & EDX_SSE2) == EDX_SSE2,
               "CPU does not support SSE2",
               OS_ERR_NOT_SUPPORTED);

    /* Might be used in future to check extended features */
    (void)regs_ext;

    KERNEL_DEBUG(CPU_DEBUG_ENABLED,
                 "[CPU] Validating architecture end");
}

int32_t cpu_compare_and_swap(volatile int32_t* memory,
                             const int32_t oldval,
                             const int32_t newval)
{
    int32_t prev;
    __asm__ __volatile__ (
            "lock cmpxchg %2, %1\n\t"
            : "=a" (prev), "+m" (*memory)
            : "r" (newval), "0" (oldval)
            : "memory");
    return prev;
}

int32_t cpu_fetch_and_add(volatile int32_t* memory, const int32_t val)
{
    int32_t prev;
    __asm__ __volatile__ (
            "lock xadd %0, %1\n\t"
            : "=r" (prev), "+m" (*memory)
            : "0" (val), "m" (*memory)
            : "memory");
    return prev;
}
void cpu_atomic_store(volatile int32_t* memory, const int32_t val)
{
    /* x86 guarantees that aligned loads and stores up to 64 bits are atomic */
    *memory = val;
}

/************************************ EOF *************************************/