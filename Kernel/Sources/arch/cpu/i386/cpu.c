/*******************************************************************************
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
#include <thread.h>               /* CPU specific thread settings */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <acpi.h>                 /* ACPI driver */
#include <lapic.h>                /* LAPIC driver */
#include <core/panic.h>           /* Kernel panic */
#include <interrupt/exceptions.h> /* Expection management */
#include <cpu_structs.h>          /* CPU structures */
#include <memory/paging.h>        /* Memory management */
#include <core/scheduler.h>       /* Kernel scheduler */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <cpu.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief CPU info storage, stores basix CPU information. */
cpu_info_t cpu_info;

/** @brief Stores the SSE state. */
uint8_t sse_enabled = 0;

/** @brief Stores a pointer to the SSE region that should be used to save the
 * SSE registers.
 */
static uint8_t* sse_save_region[MAX_CPU_COUNT] = {NULL};

/** @brief Number of detected CPU. */
static int32_t  cpu_count;

/** @brief Main kernel CPU id. */
uint32_t main_core_id;

/** @brief CPU IDs array. */
static const uint32_t* cpu_ids;

/** @brief CPU LAPICs array. */
static const local_apic_t** cpu_lapics;

/** @brief Number of CPU that completed the init sequence. */
static volatile uint32_t init_seq_end;

/** @brief AP boot code location. */
extern uint8_t init_ap_code;

/** @brief Booted CPU count */
volatile uint32_t init_cpu_count;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/** @brief Extern ASM function to relocate AP boot code. */
extern void __cpu_smp_loader_init(void);

/**
 * @brief Handles a sse use exception (coprocessor not available)
 *
 * @details Handles a sse use exception (coprocessor not available). This will
 * clear the CR0.TS bit to allow the use of SSE and save the old SSE content
 * if needed.
 *
 * @param cpu_state The cpu registers structure.
 * @param int_id The exception number.
 * @param stack_state The stack state before the exception that contain cs, eip,
 * error code and the eflags register value.
 */
static void sse_use_exception_handler(cpu_state_t* cpu_state,
                                      uint32_t int_id,
                                      stack_state_t* stack_state)
{
    uint8_t* fxregs_addr;
    uint32_t cpu_id;
    kernel_thread_t* current_thread;

    (void)cpu_state;
    (void)stack_state;
    /* Check the interrupt line */
    if(int_id != DEVICE_NOT_FOUND_LINE)
    {
        kernel_panic(OR_ERR_UNAUTHORIZED_INTERRUPT_LINE);
    }

    /* Update the CR0.TS bit */
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n\t"
        "and $0xFFFFFFF7, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
    :::"eax");

    cpu_id = cpu_get_id();
    current_thread = sched_get_self();

    /* Check if there is a SSE to save */
    if(sse_save_region[cpu_id] != NULL && 
       sse_save_region[cpu_id] != current_thread->thread_storage)
    {
        /* Align */
        fxregs_addr = (uint8_t*)((((uintptr_t)current_thread->thread_storage) & 
                                0xFFFFFFF0) +
                                16);
        __asm__ __volatile__("fxsave %0"::"m"(*fxregs_addr));
#if TEST_MODE_ENABLED
        kernel_serial_debug("[TESTMODE] SSE Context switch SAVE\n");
#endif
    }

    if(sse_save_region[cpu_id] != current_thread->thread_storage)
    {
        /* Restore the current SSE context */
        fxregs_addr = (uint8_t*)((((uintptr_t)current_thread->thread_storage) & 
                                0xFFFFFFF0) +
                                16);
        __asm__ __volatile__("fxsave %0"::"m"(*fxregs_addr));

        /* Update the save region */
        sse_save_region[cpu_id] = current_thread->thread_storage;

#if TEST_MODE_ENABLED
        kernel_serial_debug("[TESTMODE] SSE Context switch RESTORE\n");
#endif
    }
}

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


int32_t cpu_get_id(void)
{   
    uint32_t i;

    /* If lapic is not activated but we only use one CPU */
    if(MAX_CPU_COUNT == 1)
    {
        return 0;
    }    
    /* If not init */
    if(acpi_check_lapic_id(0) == OS_ACPI_NOT_INITIALIZED)
    {
        return 0;
    }

    const local_apic_t** lapics = acpi_get_cpu_lapics();
    const int32_t lapic_id = lapic_get_id();
    
    for(i = 0; i < MAX_CPU_COUNT; ++i)
    {
        if(lapics[i]->apic_id == lapic_id)
        {
            return i;
        }
    }

    return 0;
}

void cpu_init_thread_context(void (*entry_point)(void), 
                             const uintptr_t stack_index, 
                             const uintptr_t free_table_page,
                             const uintptr_t page_table_address,
                             kernel_thread_t* thread)
{
    /* Set EIP, ESP and EBP */
    thread->cpu_context.eip = (uintptr_t)entry_point;
    thread->cpu_context.esp = (uintptr_t)&thread->stack[stack_index - 17];
    thread->cpu_context.ebp = (uintptr_t)&thread->stack[stack_index - 1];

    /* Set CR3 and free page table */
    thread->cpu_context.cr3 = page_table_address;
    thread->free_page_table = free_table_page;    

    /* Init thread stack */
    thread->stack[stack_index - 1]  = THREAD_INIT_EFLAGS;
    thread->stack[stack_index - 2]  = THREAD_INIT_CS;
    thread->stack[stack_index - 3]  = thread->cpu_context.eip;
    thread->stack[stack_index - 4]  = 0; /* UNUSED (error) */
    thread->stack[stack_index - 5]  = 0; /* UNUSED (int id) */
    thread->stack[stack_index - 6]  = THREAD_INIT_DS;
    thread->stack[stack_index - 7]  = THREAD_INIT_ES;
    thread->stack[stack_index - 8]  = THREAD_INIT_FS;
    thread->stack[stack_index - 9]  = THREAD_INIT_GS;
    thread->stack[stack_index - 10] = THREAD_INIT_SS;
    thread->stack[stack_index - 11] = THREAD_INIT_EAX;
    thread->stack[stack_index - 12] = THREAD_INIT_EBX;
    thread->stack[stack_index - 13] = THREAD_INIT_ECX;
    thread->stack[stack_index - 14] = THREAD_INIT_EDX;
    thread->stack[stack_index - 15] = THREAD_INIT_ESI;
    thread->stack[stack_index - 16] = THREAD_INIT_EDI;
    thread->stack[stack_index - 17] = thread->cpu_context.ebp;
    thread->stack[stack_index - 18] = thread->cpu_context.esp;
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

void cpu_save_context(const uint32_t first_sched,
                      const cpu_state_t* cpu_state, 
                      const stack_state_t* stack_state, 
                      kernel_thread_t* thread)
{
    (void)stack_state;
    /* Save the actual ESP (not the fist time since the first schedule should
     * dissociate the boot sequence (pointed by the current esp) and the IDLE
     * thread.*/
    if(first_sched == 1)
    {
        thread->cpu_context.esp = cpu_state->esp;
    }
}

void cpu_restore_context(cpu_state_t* cpu_state, 
                         const stack_state_t* stack_state, 
                         const kernel_thread_t* thread)
{
    (void)stack_state;

    /* Update esp */
    cpu_state->esp = thread->cpu_context.esp;

    /* On context restore, the CR0.TS bit is set to catch FPU/SSE use */
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n\t"
        "or  $0x00000008, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
    :::"eax");
}

void cpu_update_pgdir(const uintptr_t new_pgdir)
{
    /* Update CR3 */
    __asm__ __volatile__("mov %%eax, %%cr3": :"a"(new_pgdir));
}

void cpu_set_next_thread_instruction(const cpu_state_t* cpu_state,
                                     stack_state_t* stack_state, 
                                     const uintptr_t next_inst)
{
    (void) cpu_state;
    /* Set next instruction */
    stack_state->eip = next_inst;
}

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

    return kernel_interrupt_set_irq_eoi(interrupt_line);
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

OS_RETURN_E cpu_enable_sse(void)
{
    /* Check for SSE support */
    if((cpu_info.cpuid_data[1] & EDX_SSE) != EDX_SSE) 
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
    /* Enables SSE and FPU */
    __asm__ __volatile__(
        "fninit\t\n"
        "mov %%cr0, %%eax\n\t"
        "and $0xFFFFFFFB, %%eax\n\t"
        "or  $0x00000002, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        "mov %%cr4, %%eax\n\t"
        "or  $0x00000600, %%eax\n\t"
        "mov %%eax, %%cr4\n\t"
    :::"eax");

    sse_enabled = 1;

    /* Sets the SSE exception to catch SSE uses */
    return kernel_exception_register_handler(DEVICE_NOT_FOUND_LINE,
                                             sse_use_exception_handler);
}

OS_RETURN_E cpu_smp_init(void)
{
    uint32_t i;
    OS_RETURN_E err;

    /* Get the number of core of the system */
    cpu_count = acpi_get_detected_cpu_count();

    /* One core detected, nothing to do */
    if(cpu_count <= 1)
    {
        return OS_NO_ERR;
    }

    init_seq_end = 0;

    kernel_info("Init %d CPU cores\n", cpu_count);

    main_core_id = cpu_get_id();

    kernel_info("Main core ID %d\n", main_core_id);

    /* Get LAPIC ids */
    cpu_ids    = acpi_get_cpu_ids();
    cpu_lapics = acpi_get_cpu_lapics();

    /* Map needed memory */
    err = kernel_mmap_hw((void*)&init_ap_code, 
                         (void*)&init_ap_code, 0x1000, 0, 1);

    if(OS_NO_ERR != err)
    {
        return err;
    }

    /* Init startup code in low mem */
    __cpu_smp_loader_init();

    /* Init each sleeping core */
    for(i = 0; i < (uint32_t)cpu_count; ++i)
    {
        uint32_t current_cpu_init;

        current_cpu_init = init_cpu_count;
        if(i == main_core_id) continue;


        err = lapic_send_ipi_init(cpu_lapics[i]->apic_id);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot send INIT IPI [%d]\n", err);
            kernel_panic(err);
        }

        kernel_interrupt_restore(1);
        time_wait_no_sched(20);
        kernel_interrupt_disable();

        /* Send startup */
        err = lapic_send_ipi_startup(cpu_lapics[i]->apic_id, 
                                     ((uintptr_t)&init_ap_code) >> 12);
        if(err != OS_NO_ERR)
        {
            kernel_error("Cannot send STARTUP IPI [%d]\n", err);
            kernel_panic(err);
        }

        kernel_interrupt_restore(1);
        time_wait_no_sched(30);
        kernel_interrupt_disable();

        if(current_cpu_init == init_cpu_count)
        {
            /* Send startup */
            err = lapic_send_ipi_startup(cpu_lapics[i]->apic_id,
                                         ((uintptr_t)&init_ap_code) >> 12);
            if(err != OS_NO_ERR)
            {
                kernel_error("Cannot send STARTUP IPI [%d]\n", err);
                kernel_panic(err);
            }
        }

        /* Wait for the current AP to Init */
        while(current_cpu_init == init_cpu_count);
    }

    init_seq_end = 1;

    /* Make sure all the AP are initialized, we should never block here */
    while(init_cpu_count < (uint32_t)cpu_count);

#if TEST_MODE_ENABLED
    cpu_smp_test();
#endif

    return err;
}

uint32_t cpu_get_booted_cpu_count(void)
{
    return init_cpu_count;
}

void cpu_ap_core_init(void)
{
    OS_RETURN_E err;
    uint32_t cpu_id = cpu_get_id();

    /* Init local APIC */
    err = lapic_init();
    if(err != OS_NO_ERR)
    {
        kernel_error("Local APIC Initialization error %d [CPU %d]\n", err,
                     cpu_id);
        kernel_panic(err);
    }

    /* Init LAPIC TIMER */
    err = lapic_ap_timer_init();
    if(err != OS_NO_ERR)
    {
        kernel_error("Local APIC TIMER Initialization error %d [CPU %d]\n",
                     err, cpu_id);
        kernel_panic(err);
    }
    
    kernel_info("CPU %d booted, idling...\n", cpu_id);

    /* Update booted cpu count */
    ++init_cpu_count;

#if TEST_MODE_ENABLED
    cpu_smp_test();
#endif

    while(init_seq_end == 0); 

    /* Init Scheduler */
    err = sched_init_ap();
    kernel_error("End of kernel reached by AP Core %d [%d]\n", cpu_id, err);
    kernel_panic(err);

}