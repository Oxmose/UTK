/***************************************************************************//**
 * @file cpu.c
 *
 * @see cpu.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
 *
 * @version 1.0
 *
 * @brief X86 CPU management functions
 *
 * @details X86 CPU manipulation functions. Wraps inline assembly calls for ease
 * of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stdint.h>           /* Generic int types */
#include <lib/stddef.h>           /* Standard definitions */
#include <lib/string.h>           /* String manipulation */
#include <io/kernel_output.h>     /* Kernel output methods */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <cpu.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* May not be static since is used as extern in ASM */
/** @brief CPU info storage, stores basix CPU information. */
cpu_info_t cpu_info;

/** @brief Stores the SSE state */
uint8_t sse_enabled = 0;

/** @brief Stores a pointer to the SSE region that should be used to save the
 * SSE registers.
 */
uint8_t* sse_save_region[MAX_CPU_COUNT] = {NULL};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E cpu_get_info(cpu_info_t* info)
{
    if(info == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    memcpy(info, &cpu_info, sizeof(cpu_info_t));

    return OS_NO_ERR;
}

uint8_t cpu_is_sse_enabled(void)
{
    return sse_enabled;
}

int32_t cpu_cpuid_capable(void)
{
    return ((cpu_info.cpu_flags & CPU_FLAG_CPU_CPUID_CAPABLE) >> 21) & 0x1;
}

OS_RETURN_E cpu_detect(const uint32_t print)
{
    #if CPU_DEBUG 
    kernel_serial_debug("Detecting cpu\n");
    #endif
    if(cpu_cpuid_capable() == 1)
    {
        /* eax, ebx, ecx, edx */
        int32_t regs[4];
        uint32_t ret;

        ret = cpu_cpuid(CPUID_GETVENDORSTRING, (uint32_t*)regs);


        /* Check if CPUID return more that one available function */
        if(ret != 0 && print != 0)
        {

            kernel_info("CPU Vendor: ");

            for(int8_t j = 0; j < 4; ++j)
            {
                kernel_printf("%c", (char)((regs[1] >> (j * 8)) & 0xFF));
            }
            for(int8_t j = 0; j < 4; ++j)
            {

                kernel_printf("%c", (char)((regs[3] >> (j * 8)) & 0xFF));
            }
            for(int8_t j = 0; j < 4; ++j)
            {

                kernel_printf("%c", (char)((regs[2] >> (j * 8)) & 0xFF));
            }
        }
        else if(ret == 0)
        {
            if(print != 0)
            {
                kernel_info("Failed to get CPUID data");
            }
            return OS_ERR_UNAUTHORIZED_ACTION;
        }

        kernel_printf("\n");

        /* If we have general CPUID features */
        if(ret >= 0x01)
        {
            /* Get CPU features */
            cpu_cpuid(CPUID_GETFEATURES, (uint32_t*)regs);

            /* Save and display */
            cpu_info.cpuid_data[0] = regs[2];
            cpu_info.cpuid_data[1] = regs[3];

            if(print != 0)
            {

                kernel_info("CPU Features: ");

                if((regs[2] & ECX_SSE3) == ECX_SSE3)
                { kernel_printf("SSE3 - "); }
                if((regs[2] & ECX_PCLMULQDQ) == ECX_PCLMULQDQ)
                { kernel_printf("PCLMULQDQ - "); }
                if((regs[2] & ECX_DTES64) == ECX_DTES64)
                { kernel_printf("DTES64 - "); }
                if((regs[2] & ECX_MONITOR) == ECX_MONITOR)
                { kernel_printf("MONITOR - "); }
                if((regs[2] & ECX_DS_CPL) == ECX_DS_CPL)
                { kernel_printf("DS_CPL - "); }
                if((regs[2] & ECX_VMX) == ECX_VMX)
                { kernel_printf("VMX - "); }
                if((regs[2] & ECX_SMX) == ECX_SMX)
                { kernel_printf("SMX - "); }
                if((regs[2] & ECX_EST) == ECX_EST)
                { kernel_printf("EST - "); }
                if((regs[2] & ECX_TM2) == ECX_TM2)
                { kernel_printf("TM2 - "); }
                if((regs[2] & ECX_SSSE3) == ECX_SSSE3)
                { kernel_printf("SSSE3 - "); }
                if((regs[2] & ECX_CNXT_ID) == ECX_CNXT_ID)
                { kernel_printf("CNXT_ID - "); }
                if((regs[2] & ECX_FMA) == ECX_FMA)
                { kernel_printf("FMA - "); }
                if((regs[2] & ECX_CX16) == ECX_CX16)
                { kernel_printf("CX16 - "); }
                if((regs[2] & ECX_XTPR) == ECX_XTPR)
                { kernel_printf("XTPR - "); }
                if((regs[2] & ECX_PDCM) == ECX_PDCM)
                { kernel_printf("PDCM - "); }
                if((regs[2] & ECX_PCID) == ECX_PCID)
                { kernel_printf("PCID - "); }
                if((regs[2] & ECX_DCA) == ECX_DCA)
                { kernel_printf("DCA - "); }
                if((regs[2] & ECX_SSE41) == ECX_SSE41)
                { kernel_printf("SSE41 - "); }
                if((regs[2] & ECX_SSE42) == ECX_SSE42)
                { kernel_printf("SSE42 - "); }
                if((regs[2] & ECX_X2APIC) == ECX_X2APIC)
                { kernel_printf("X2APIC - "); }
                if((regs[2] & ECX_MOVBE) == ECX_MOVBE)
                { kernel_printf("MOVBE - "); }
                if((regs[2] & ECX_POPCNT) == ECX_POPCNT)
                { kernel_printf("POPCNT - "); }
                if((regs[2] & ECX_TSC) == ECX_TSC)
                { kernel_printf("TSC - "); }
                if((regs[2] & ECX_AESNI) == ECX_AESNI)
                { kernel_printf("AESNI - "); }
                if((regs[2] & ECX_XSAVE) == ECX_XSAVE)
                { kernel_printf("XSAVE - "); }
                if((regs[2] & ECX_OSXSAVE) == ECX_OSXSAVE)
                { kernel_printf("OSXSAVE - "); }
                if((regs[2] & ECX_AVX) == ECX_AVX)
                { kernel_printf("AVX - "); }
                if((regs[2] & ECX_F16C) == ECX_F16C)
                { kernel_printf("F16C - "); }
                if((regs[2] & ECX_RDRAND) == ECX_RDRAND)
                { kernel_printf("RDRAND - "); }
                if((regs[3] & EDX_FPU) == EDX_FPU)
                { kernel_printf("FPU - "); }
                if((regs[3] & EDX_VME) == EDX_VME)
                { kernel_printf("VME - "); }
                if((regs[3] & EDX_DE) == EDX_DE)
                { kernel_printf("DE - "); }
                if((regs[3] & EDX_PSE) == EDX_PSE)
                { kernel_printf("PSE - "); }
                if((regs[3] & EDX_TSC) == EDX_TSC)
                { kernel_printf("TSC - "); }
                if((regs[3] & EDX_MSR) == EDX_MSR)
                { kernel_printf("MSR - "); }
                if((regs[3] & EDX_PAE) == EDX_PAE)
                { kernel_printf("PAE - "); }
                if((regs[3] & EDX_MCE) == EDX_MCE)
                { kernel_printf("MCE - "); }
                if((regs[3] & EDX_CX8) == EDX_CX8)
                { kernel_printf("CX8 - "); }
                if((regs[3] & EDX_APIC) == EDX_APIC)
                { kernel_printf("APIC - "); }
                if((regs[3] & EDX_SEP) == EDX_SEP)
                { kernel_printf("SEP - "); }
                if((regs[3] & EDX_MTRR) == EDX_MTRR)
                { kernel_printf("MTRR - "); }
                if((regs[3] & EDX_PGE) == EDX_PGE)
                { kernel_printf("PGE - "); }
                if((regs[3] & EDX_MCA) == EDX_MCA)
                { kernel_printf("MCA - "); }
                if((regs[3] & EDX_CMOV) == EDX_CMOV)
                { kernel_printf("CMOV - "); }
                if((regs[3] & EDX_PAT) == EDX_PAT)
                { kernel_printf("PAT - "); }
                if((regs[3] & EDX_PSE36) == EDX_PSE36)
                { kernel_printf("PSE36 - "); }
                if((regs[3] & EDX_PSN) == EDX_PSN)
                { kernel_printf("PSN - "); }
                if((regs[3] & EDX_CLFLUSH) == EDX_CLFLUSH)
                { kernel_printf("CLFLUSH - "); }
                if((regs[3] & EDX_DS) == EDX_DS)
                { kernel_printf("DS - "); }
                if((regs[3] & EDX_ACPI) == EDX_ACPI)
                { kernel_printf("ACPI - "); }
                if((regs[3] & EDX_MMX) == EDX_MMX)
                { kernel_printf("MMX - "); }
                if((regs[3] & EDX_FXSR) == EDX_FXSR)
                { kernel_printf("FXSR - "); }
                if((regs[3] & EDX_SSE) == EDX_SSE)
                { kernel_printf("SSE - "); }
                if((regs[3] & EDX_SSE2) == EDX_SSE2)
                { kernel_printf("SSE2 - "); }
                if((regs[3] & EDX_SS) == EDX_SS)
                { kernel_printf("SS - "); }
                if((regs[3] & EDX_HTT) == EDX_HTT)
                { kernel_printf("HTT - "); }
                if((regs[3] & EDX_TM) == EDX_TM)
                { kernel_printf("TM - "); }
                if((regs[3] & EDX_PBE) == EDX_PBE)
                { kernel_printf("PBE - "); }

                /* Check for extended features */
                cpu_cpuid(CPUID_INTELEXTENDED_AVAILABLE, (uint32_t*)regs);
                if((uint32_t)regs[0] >= (uint32_t)CPUID_INTELFEATURES)
                {
                    cpu_cpuid(CPUID_INTELFEATURES, (uint32_t*)regs);

                    if((regs[3] & EDX_SYSCALL) == EDX_SYSCALL)
                    { kernel_printf("SYSCALL - "); }
                    if((regs[3] & EDX_MP) == EDX_MP)
                    { kernel_printf("MP - "); }
                    if((regs[3] & EDX_XD) == EDX_XD)
                    { kernel_printf("XD - "); }
                    if((regs[3] & EDX_MMX_EX) == EDX_MMX_EX)
                    { kernel_printf("MMX_EX - "); }
                    if((regs[3] & EDX_FXSR) == EDX_FXSR)
                    { kernel_printf("FXSR - "); }
                    if((regs[3] & EDX_FXSR_OPT) == EDX_FXSR_OPT)
                    { kernel_printf("FXSR_OPT - "); }
                    if((regs[3] & EDX_1GB_PAGE) == EDX_1GB_PAGE)
                    { kernel_printf("1GB_PAGE - "); }
                    if((regs[3] & EDX_RDTSCP) == EDX_RDTSCP)
                    { kernel_printf("RDTSCP - "); }
                    if((regs[3] & EDX_64_BIT) == EDX_64_BIT)
                    { kernel_printf("X64 - "); }
                    if((regs[3] & EDX_3DNOW_EX) == EDX_3DNOW_EX)
                    { kernel_printf("3DNOW_EX - "); }
                    if((regs[3] & EDX_3DNOW) == EDX_3DNOW)
                    { kernel_printf("3DNOW - "); }
                    if((regs[2] & ECX_LAHF_LM) == ECX_LAHF_LM)
                    { kernel_printf("LAHF_LM - "); }
                    if((regs[2] & ECX_CMP_LEG) == ECX_CMP_LEG)
                    { kernel_printf("CMP_LEG - "); }
                    if((regs[2] & ECX_SVM) == ECX_SVM)
                    { kernel_printf("SVM - "); }
                    if((regs[2] & ECX_EXTAPIC) == ECX_EXTAPIC)
                    { kernel_printf("EXTAPIC - "); }
                    if((regs[2] & ECX_CR8_LEG) == ECX_CR8_LEG)
                    { kernel_printf("CR8_LEG - "); }
                    if((regs[2] & ECX_ABM) == ECX_ABM)
                    { kernel_printf("ABM - "); }
                    if((regs[2] & ECX_SSE4A) == ECX_SSE4A)
                    { kernel_printf("SSE4A - "); }
                    if((regs[2] & ECX_MISASSE) == ECX_MISASSE)
                    { kernel_printf("MISALIGNED_SSE - "); }
                    if((regs[2] & ECX_PREFETCH) == ECX_PREFETCH)
                    { kernel_printf("PREFETCH - "); }
                    if((regs[2] & ECX_OSVW) == ECX_OSVW)
                    { kernel_printf("OSVW - "); }
                    if((regs[2] & ECX_IBS) == ECX_IBS)
                    { kernel_printf("IBS - "); }
                    if((regs[2] & ECX_XOP) == ECX_XOP)
                    { kernel_printf("XOP - "); }
                    if((regs[2] & ECX_SKINIT) == ECX_SKINIT)
                    { kernel_printf("SKINIT - "); }
                    if((regs[2] & ECX_WDT) == ECX_WDT)
                    { kernel_printf("WDT - "); }
                    if((regs[2] & ECX_LWP) == ECX_LWP)
                    { kernel_printf("LWP - "); }
                    if((regs[2] & ECX_FMA4) == ECX_FMA4)
                    { kernel_printf("FMA4 - "); }
                    if((regs[2] & ECX_TCE) == ECX_TCE)
                    { kernel_printf("TCE - "); }
                    if((regs[2] & ECX_NODEIDMSR) == ECX_NODEIDMSR)
                    { kernel_printf("NODE_ID_MSR - "); }
                    if((regs[2] & ECX_TBM) == ECX_TBM)
                    { kernel_printf("TMB - "); }
                    if((regs[2] & ECX_TOPOEX) == ECX_TOPOEX)
                    { kernel_printf("TOPOEX - "); }
                    if((regs[2] & ECX_PERF_CORE) == ECX_PERF_CORE)
                    { kernel_printf("PERF_CORE - "); }
                    if((regs[2] & ECX_PERF_NB) == ECX_PERF_NB)
                    { kernel_printf("PERF_NB - "); }
                    if((regs[2] & ECX_DBX) == ECX_DBX)
                    { kernel_printf("DBX - "); }
                    if((regs[2] & ECX_PERF_TSC) == ECX_PERF_TSC)
                    { kernel_printf("TSC - "); }
                    if((regs[2] & ECX_PCX_L2I) == ECX_PCX_L2I)
                    { kernel_printf("PCX_L2I - "); }
                }

                kernel_printf("UTK");
            }
        }

        if(print != 0)
        {
            kernel_printf("\n");
        }
    }
    else
    {
        if(print != 0)
        {
            kernel_info("CPUID not available\n");
        }
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    #if CPU_DEBUG 
    kernel_serial_debug("Detecting cpu end\n");
    #endif

    return OS_NO_ERR;
}