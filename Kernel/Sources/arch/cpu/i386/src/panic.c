/*******************************************************************************
 * @file panic.c
 *
 * @see panic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 1.0
 *
 * @brief Panic feature of the kernel.
 *
 * @details Kernel panic functions. Displays the CPU registers, the faulty
 * instruction, the interrupt ID and cause.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <interrupts.h>           /* Interrupt management */
#include <interrupt_settings.h>   /* Interrupt settings */
#include <graphic.h>              /* Graphic API */
#include <kernel_output.h>        /* Kernel output methods */
#include <stdint.h>               /* Generic int types */
#include <stdio.h>                /* Error string */
#include <cpu_api.h>              /* CPU management */
#include <string.h>               /* Memset */
#include <rt_clock.h>             /* RTC driver */
#include <scheduler.h>            /* Scheduler */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <panic.h>

/*******************************************************************************
 * GCONSTANTS
 ******************************************************************************/

/**
 * @brief Defines the NMI Panic code.
 */
#define PANIC_NMI_CODE 0xFFFFFFFF

/** @brief Defines the maximal file name lenght for the panic */
#define PANIC_MAX_FILE_NAME_LENGTH 256

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the current kernel panic error code. */
static uint32_t panic_code = 0;

/** @brief Stores the NMI panic code */
static uint32_t nmi_panic_code = 0;

/** @brief Stores the line at which the kernel panic was called. */
static uint32_t panic_line;

/** @brief Stores the file from which the panic was called */
static char panic_file[PANIC_MAX_FILE_NAME_LENGTH];

/*******************************************************************************
 * STATIC FUNCTIONS DELCARATIONS
 ******************************************************************************/

static void print_panic_header(uintptr_t int_id, 
                               stack_state_t* stack_state, 
                               uint32_t error_code);

static void print_cpu_state(cpu_state_t* cpu_state, stack_state_t* stack_state);

static void print_cpu_flags(stack_state_t* stack_state);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void print_panic_header(uintptr_t int_id, 
                               stack_state_t* stack_state, 
                               uint32_t error_code)
{
    kernel_printf("##############################    KERNEL PANIC    ##########"
                    "####################\n");
    kernel_printf("  ");
    switch(int_id)
    {
        case 0:
            kernel_printf("Division by zero                        ");
            break;
        case 1:
            kernel_printf("Single-step interrupt                   ");
            break;
        case 2:
            kernel_printf("Non maskable interrupt                  ");
            break;
        case 3:
            kernel_printf("Breakpoint                              ");
            break;
        case 4:
            kernel_printf("Overflow                                ");
            break;
        case 5:
            kernel_printf("Bounds                                  ");
            break;
        case 6:
            kernel_printf("Invalid Opcode                          ");
            break;
        case 7:
            kernel_printf("Coprocessor not available               ");
            break;
        case 8:
            kernel_printf("Double fault                            ");
            break;
        case 9:
            kernel_printf("Coprocessor Segment Overrun             ");
            break;
        case 10:
            kernel_printf("Invalid Task State Segment              ");
            break;
        case 11:
            kernel_printf("Segment not present                     ");
            break;
        case 12:
            kernel_printf("Stack Fault                             ");
            break;
        case 13:
            kernel_printf("General protection fault                ");
            break;
        case 14:
            kernel_printf("Page fault                              ");
            break;
        case 16:
            kernel_printf("Math Fault                              ");
            break;
        case 17:
            kernel_printf("Alignment Check                         ");
            break;
        case 18:
            kernel_printf("Machine Check                           ");
            break;
        case 19:
            kernel_printf("SIMD Floating-Point Exception           ");
            break;
        case 20:
            kernel_printf("Virtualization Exception                ");
            break;
        case 21:
            kernel_printf("Control Protection Exception            ");
            break;
        case PANIC_INT_LINE:
            kernel_printf("Panic generated by the kernel           ");
            break;
        default:
            kernel_printf("Unknown reason                          ");
    }

    kernel_printf("        INT ID: 0x%02X                 \n", int_id);
    kernel_printf("  Instruction [EIP]: 0x%p                   Error code: "
                    "0x%p       \n", stack_state->eip, error_code);
    kernel_printf("                                                            "
                    "                   \n");
    kernel_printf("---------------------------------- CPU STATE ---------------"
                    "--------------------\n");
}

static void print_cpu_state(cpu_state_t* cpu_state, stack_state_t* stack_state)
{
    uint32_t CR0;
    uint32_t CR2;
    uint32_t CR3;
    uint32_t CR4;

    __asm__ __volatile__ (
        "mov %%cr0, %%eax\n\t"
        "mov %%eax, %0\n\t"
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %1\n\t"
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %2\n\t"
        "mov %%cr4, %%eax\n\t"
        "mov %%eax, %3\n\t"
    : "=m" (CR0), "=m" (CR2), "=m" (CR3), "=m" (CR4)
    : /* no input */
    : "%eax"
    );

    kernel_printf("  EAX: 0x%p  |  EBX: 0x%p  |  ECX: 0x%p  |  EDX: "
                    "0x%p  \n", cpu_state->eax, cpu_state->ebx,
                    cpu_state->ecx,  cpu_state->edx);
    kernel_printf("  ESI: 0x%p  |  EDI: 0x%p  |  EBP: 0x%p  |  ESP: "
                    "0x%p  \n", cpu_state->esi, cpu_state->edi,
                    cpu_state->ebp, cpu_state->esp);
    kernel_printf("  CR0: 0x%p  |  CR2: 0x%p  |  CR3: 0x%p  |  CR4: "
                    "0x%p  \n\n", CR0, CR2, CR3, CR4);
    kernel_printf("  CS: 0x%04X  |  DS: 0x%04X  |  SS: 0x%04X                  "
                    "                   \n", stack_state->cs & 0xFFFF,
                    cpu_state->ds & 0xFFFF, cpu_state->ss & 0xFFFF);
    kernel_printf("  ES: 0x%04X  |  FS: 0x%04X  |  GS: 0x%04X                  "
                    "                   \n", cpu_state->es & 0xFFFF ,
                    cpu_state->fs & 0xFFFF , cpu_state->gs & 0xFFFF);
    kernel_printf("                                                            "
                    "                   \n");
}

static void print_cpu_flags(stack_state_t* stack_state)
{
    int8_t cf_f = (stack_state->eflags & 0x1);
    int8_t pf_f = (stack_state->eflags & 0x4) >> 2;
    int8_t af_f = (stack_state->eflags & 0x10) >> 4;
    int8_t zf_f = (stack_state->eflags & 0x40) >> 6;
    int8_t sf_f = (stack_state->eflags & 0x80) >> 7;
    int8_t tf_f = (stack_state->eflags & 0x100) >> 8;
    int8_t if_f = (stack_state->eflags & 0x200) >> 9;
    int8_t df_f = (stack_state->eflags & 0x400) >> 10;
    int8_t of_f = (stack_state->eflags & 0x800) >> 11;
    int8_t nf_f = (stack_state->eflags & 0x4000) >> 14;
    int8_t rf_f = (stack_state->eflags & 0x10000) >> 16;
    int8_t vm_f = (stack_state->eflags & 0x20000) >> 17;
    int8_t ac_f = (stack_state->eflags & 0x40000) >> 18;
    int8_t id_f = (stack_state->eflags & 0x200000) >> 21;
    int8_t iopl0_f = (stack_state->eflags & 0x1000) >> 12;
    int8_t iopl1_f = (stack_state->eflags & 0x2000) >> 13;
    int8_t vif_f = (stack_state->eflags & 0x8000) >> 19;
    int8_t vip_f = (stack_state->eflags & 0x100000) >> 20;

    
    kernel_printf("  ");

    if(cf_f != 0)
    {
        kernel_printf("CF ");
    }
    if(pf_f != 0)
    {
        kernel_printf("PF ");
    }
    if(af_f != 0)
    {
        kernel_printf("AF ");
    }
    if(zf_f != 0)
    {
        kernel_printf("ZF ");
    }
    if(sf_f != 0)
    {
        kernel_printf("SF ");
    }
    if(tf_f != 0)
    {
        kernel_printf("TF ");
    }
    if(if_f != 0)
    {
        kernel_printf("IF ");
    }
    if(df_f != 0)
    {
        kernel_printf("DF ");
    }
    if(of_f != 0)
    {
        kernel_printf("OF ");
    }
    if(nf_f != 0)
    {
        kernel_printf("NT ");
    }
    if(rf_f != 0)
    {
        kernel_printf("RF ");
    }
    if(vm_f != 0)
    {
        kernel_printf("VM ");
    }
    if(ac_f != 0)
    {
        kernel_printf("AC ");
    }
    if(vif_f != 0)
    {
        kernel_printf("VF ");
    }
    if(vip_f != 0)
    {
        kernel_printf("VP ");
    }
    if(id_f != 0)
    {
        kernel_printf("ID ");
    }
    if((iopl0_f | iopl1_f) != 0)
    {
        kernel_printf("IO: %d ", (iopl0_f | iopl1_f << 1));
    }
    kernel_printf(" |  EFLAGS: 0x%p\n\n", stack_state->eflags);
}

void panic(cpu_state_t* cpu_state, uintptr_t int_id, stack_state_t* stack_state)
{
    uint32_t      error_code;
    colorscheme_t panic_scheme;

    uint32_t cpu_id;

    uint32_t time;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;

    kernel_process_t* process;
    kernel_thread_t*  thread;    

    time = rtc_get_current_daytime();
    hours = time / 3600;
    minutes = (time / 60) % 60;
    seconds = time % 60;

    cpu_id = cpu_get_id();

    /* If we received an NMI and the error code is NMI_PANIC, we just halt the
     * CPU as the panic screen should have been displayed by the CPU or core
     * that issued the NMI */
    if(nmi_panic_code == PANIC_NMI_CODE)
    {
        while(1)
        {
            cpu_clear_interrupt();
            cpu_hlt();
        }
    }

    cpu_clear_interrupt();

    panic_scheme.background = BG_BLACK;
    panic_scheme.foreground = FG_CYAN;
    panic_scheme.vga_color  = 1;

    graphic_set_color_scheme(panic_scheme);

    /* If int is generated by the kernel, then the error code is contained in
     * the error code memory address, otherwise we use the interrupt error code.
     */
    if(int_id == PANIC_INT_LINE)
    {
        error_code = panic_code;
    }
    else
    {
        error_code = stack_state->error_code;
    }

#ifdef TEST_MODE_ENABLED
    kernel_printf("\n[TESTMODE] PANIC %d\n", error_code);
    kernel_printf("  File: %s at line %d\n", panic_file, panic_line);
    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
#endif

    process = sched_get_current_process();
    thread = sched_get_current_thread();

    print_panic_header(int_id, stack_state, error_code);
    print_cpu_state(cpu_state, stack_state);
    print_cpu_flags(stack_state);

    kernel_printf("------------------------------- ADDITIONAL INFO ------------"
                    "--------------------\n");
    kernel_printf("  Core ID: %u | TID:  %u | PID:  %u | Time of panic: "
                  "%02u:%02u:%02u\n", cpu_id, sched_get_tid(), sched_get_pid(),
                  hours, minutes, seconds);
    kernel_printf("  Error: ");
    perror(error_code);
    kernel_printf(" (%d)\n", error_code);
    kernel_printf("  Process: %s | Thread: %s\n", 
                  process != NULL ? process->name : "NO_PROCESS",
                  thread != NULL ? thread->name : "NO_THREAD");
    kernel_printf("  File: %s at line %d\n", panic_file, panic_line);

    kernel_printf("\n\n\n\n\n");

    /* Hide cursor */
    panic_scheme.background = BG_BLACK;
    panic_scheme.foreground = FG_BLACK;
    panic_scheme.vga_color  = 1;

    graphic_set_color_scheme(panic_scheme);

    /* We will never return from interrupt */
    while(1)
    {
        cpu_clear_interrupt();
        cpu_hlt();
    }
}

void kernel_panic(const uint32_t error_code, 
                  const char* file, 
                  const uint32_t line)
{
    /* Save the error code to memory */
    panic_code = error_code;
    strncpy(panic_file, file, PANIC_MAX_FILE_NAME_LENGTH);
    panic_line = line;

    /* Raise INT, sti ensure 1 cycle of execution where no interrupt occur, 
     * thus, letting int execute
     */
    __asm__ __volatile__("sti\n\t"
                         "int %0" :: "i" (PANIC_INT_LINE));
}