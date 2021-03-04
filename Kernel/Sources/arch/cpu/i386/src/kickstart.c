/*******************************************************************************
 * @file kernel_kickstart.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/03/2021
 *
 * @version 1.0
 *
 * @brief Kernel's main boot sequence.
 *
 * @warning At this point interrupts should be disabled.
 *
 * @details Kernel's booting sequence. Initializes the rest of the kernel and
 * performs GDT, IDT and TSS initialization. Initializes the hardware and 
 * software core of the kernel before calling the scheduler.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <cpu_settings.h>          /* CPU management */
#include <cpu.h>                   /* CPU management */
#include <graphic.h>               /* Output manager */
#include <kernel_output.h>         /* Kernel output */
#include <uart.h>                  /* uart driver */
#include <vga_text.h>              /* VGA drivers */
#include <stddef.h>                /* Standard definitions */
#include <string.h>                /* String manipulations */
#include <kheap.h>                 /* Kernel heap */
#include <interrupts.h>            /* Interrupt manager */
#include <exceptions.h>            /* Exception manager */
#include <panic.h>                 /* Kernel panic */
#include <memmgt.h>               /* Memory mapping informations */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None. */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* TODO Add panic when needed */
#define INIT_MSG(msg_success, msg_error, error, panic) \
    {                                                  \
        if (error != OS_NO_ERR)                        \
        {                                              \
            KERNEL_ERROR(msg_error, error);            \
        }                                              \
        else if (strlen(msg_success) != 0)             \
        {                                              \
            KERNEL_SUCCESS(msg_success);               \
        }                                              \
    }


#define CONCAT_STR(buff, idx, str) \
    {                              \
        strcpy(buff + idx, str);   \
        idx += strlen(str);        \
    }

/**
 * @brief Checks the architecture's feature and requirements for UTK.
 *
 * @details Checks the architecture's feature and requirements for UTK. If a 
 * requirement is not met, a kernel panic is raised.
 */
static void validate_architecture(void)
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

    KERNEL_DEBUG("[KICKSTART] Detecting cpu\n"); 

    ret = cpu_cpuid(CPUID_GETVENDORSTRING, (uint32_t*)regs);

    /* Check if CPUID return more that one available function */
    if(ret != 0)
    {
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
    }
    else
    {
        KERNEL_ERROR("Architecture does not support CPUID\n");
        /* TODO Kernel panic */
        while(1);
    }

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
    if((regs[3] & EDX_SEP) != EDX_SEP)
    {
        KERNEL_ERROR("Architecture does not support SYSENTER\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_FPU) != EDX_FPU)
    {
        KERNEL_ERROR("Architecture does not support FPU\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_TSC) != EDX_TSC)
    {
        KERNEL_ERROR("Architecture does not support TSC\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_APIC) != EDX_APIC)
    {
        KERNEL_ERROR("Architecture does not support APIC\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_FXSR) != EDX_FXSR)
    {
        KERNEL_ERROR("Architecture does not support FX instructions\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_SSE) != EDX_SSE)
    {
        KERNEL_ERROR("Architecture does not support SSE\n");
        /* TODO Kernel panic */
        while(1);
    }
    if((regs[3] & EDX_SSE2) != EDX_SSE2)
    {
        KERNEL_ERROR("Architecture does not support SSE2\n");
        /* TODO Kernel panic */
        while(1);
    }

    /* Might be used in future to check extended features */
    (void)regs_ext;

    KERNEL_DEBUG("Validating architecture end\n");
}

/**
 * @brief Main boot sequence, C kernel entry point.
 *
 * @details Main boot sequence, C kernel entry point. Initializes each basic
 * drivers for the kernel, then init the scheduler and start the system.
 *
 * @warning This function should never return. In case of return, the kernel
 * should be able to catch the return as an error.
 */
void kernel_kickstart(void)
{
    OS_RETURN_E   err;

    /* Init uart for basic log */
    graphic_set_selected_driver(&uart_text_driver);
    uart_init();

    /* Initialize CPU structures */
    cpu_setup_gdt();
    cpu_setup_idt();
    cpu_setup_tss();

#ifdef TEST_MODE_ENABLED
    boot_test();
    output_test();
    panic_test();
#endif

    KERNEL_DEBUG("[KICKSTART] Kickstarting kernel\n");

    /* Start the VGA driver */
    err =  vga_init();
    err |= graphic_set_selected_driver(&vga_text_driver);
    INIT_MSG("VGA driver initialized\n", 
             "Could not initialize VGA driver [%u]\n",
             err, 1);

    /* Validate architecture support */
    validate_architecture();

    err = kheap_init();
    INIT_MSG("Kernel heap initialized\n",
             "Could not initialize kernel heap [%u]\n",
             err, 1);

#ifdef TEST_MODE_ENABLED
    queue_test();
#endif

    err = memory_map_init();
    INIT_MSG("",
             "Could not get memory map [%u]\n",
             err, 1);

    err = kernel_interrupt_init();
    INIT_MSG("Interrupt manager initialized\n",
             "Could not initialize interrupt manager [%u]\n",
             err, 1);

    err = kernel_exception_init();
    INIT_MSG("Exception manager initialized\n",
             "Could not initialize exception manager [%u]\n",
             err, 1);
        

    KERNEL_SUCCESS("Kernel initialized\n");
    
    while(1);
    
    /* We should never get here once the scheduler has been called */
    kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
}