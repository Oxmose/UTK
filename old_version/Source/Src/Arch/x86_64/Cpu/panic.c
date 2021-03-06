/***************************************************************************//**
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

#include <Interrupt/interrupts.h> /* cpu_state_t, stack_state_t,
                                   * PANIC_INT_LINE */
#include <IO/graphic.h>           /* color_scheme_t */
#include <IO/kernel_output.h>     /* kernel_printf */
#include <Lib/stdint.h>           /* Generic int types */
#include <Cpu/cpu.h>              /* hlt cpu_clear_interrupt */
#include <Cpu/bios_call.h>        /* bios_call() */
#include <Core/scheduler.h>       /* sched_get_tid */
#include <Drivers/lapic.h>            /* lapic_get_id() */
#include <Drivers/acpi.h>             /* acpi_get_detected_cpu_count() */
#include <Drivers/vga_text.h>     /* vga_text_driver */
#include <Drivers/rtc.h>              /* RTC time */
/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Cpu/panic.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Stores the current kernel panic error code. */
static uint32_t panic_code = 0;

/** @brief Stores the NMI panic code */
static uint32_t nmi_panic_code = 0;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void panic(cpu_state_t* cpu_state, address_t int_id, stack_state_t* stack_state)
{
    uint64_t CR0;
    uint64_t CR2;
    uint64_t CR3;
    uint64_t CR4;

    uint32_t      error_code;
    uint32_t      current_cpu_id;
    colorscheme_t panic_scheme;
    //uint32_t      i;
    //uint32_t      cpu_count;

    //const uint32_t*      cpu_ids;
    //const local_apic_t** cpu_lapics;

    uint32_t time;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;

    int8_t cf_f = (stack_state->rflags & 0x1);
    int8_t pf_f = (stack_state->rflags & 0x4) >> 2;
    int8_t af_f = (stack_state->rflags & 0x10) >> 4;
    int8_t zf_f = (stack_state->rflags & 0x40) >> 6;
    int8_t sf_f = (stack_state->rflags & 0x80) >> 7;
    int8_t tf_f = (stack_state->rflags & 0x100) >> 8;
    int8_t if_f = (stack_state->rflags & 0x200) >> 9;
    int8_t df_f = (stack_state->rflags & 0x400) >> 10;
    int8_t of_f = (stack_state->rflags & 0x800) >> 11;
    int8_t nt_f = (stack_state->rflags & 0x4000) >> 14;
    int8_t rf_f = (stack_state->rflags & 0x10000) >> 16;
    int8_t vm_f = (stack_state->rflags & 0x20000) >> 17;
    int8_t ac_f = (stack_state->rflags & 0x40000) >> 18;
    int8_t id_f = (stack_state->rflags & 0x200000) >> 21;
    int8_t iopl0_f = (stack_state->rflags & 0x1000) >> 12;
    int8_t iopl1_f = (stack_state->rflags & 0x2000) >> 13;
    int8_t vif_f = (stack_state->rflags & 0x8000) >> 19;
    int8_t vip_f = (stack_state->rflags & 0x100000) >> 20;

    time = rtc_get_current_daytime();
    hours = time / 3600;
    minutes = (time / 60) % 60;
    seconds = time % 60;

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

    current_cpu_id = cpu_get_id();

    cpu_clear_interrupt();

    /* Kill other CPUs */
    //cpu_ids        = acpi_get_cpu_ids();
    //cpu_lapics     = acpi_get_cpu_lapics();
    //cpu_count      = acpi_get_detected_cpu_count();
    nmi_panic_code = PANIC_NMI_CODE;
#if 0
    for(i = 0; i < (uint32_t)cpu_count; ++i)
    {
        if(cpu_ids[i] != current_cpu_id)
        {
            lapic_send_ipi(cpu_lapics[i]->apic_id, PANIC_INT_LINE);
        }
    }

    /* Do the switch */
    bios_int_regs_t regs;

    regs.ax = BIOS_CALL_SET_VGA_TEXT_MODE;
    bios_call(BIOS_INTERRUPT_VGA, &regs);

    graphic_set_selected_driver(&vga_text_driver);
#endif
    panic_scheme.background = BG_BLACK;
    panic_scheme.foreground = FG_CYAN;
    panic_scheme.vga_color  = 1;

    graphic_set_color_scheme(panic_scheme);

    /* Test mode probing */
    if(panic_code == 666)
    {
        kernel_printf("\n[TESTMODE] PANIC\n");
    }


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

    __asm__ __volatile__ (
        "mov %%cr0, %%rax\n\t"
        "mov %%rax, %0\n\t"
        "mov %%cr2, %%rax\n\t"
        "mov %%rax, %1\n\t"
        "mov %%cr3, %%rax\n\t"
        "mov %%rax, %2\n\t"
        "mov %%cr4, %%rax\n\t"
        "mov %%rax, %3\n\t"
    : "=m" (CR0), "=m" (CR2), "=m" (CR3), "=m" (CR4)
    : /* no input */
    : "%rax"
    );

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

    kernel_printf("        INT ID: 0x%02X                 \n", int_id);
    kernel_printf("  Instruction [RIP]: 0x%P           Error code: "
                    "0x%08X       \n", stack_state->rip, error_code);
    kernel_printf("                                                            "
                    "                   \n");
    kernel_printf("---------------------------------- CPU STATE ---------------"
                    "--------------------\n");
    kernel_printf("RAX: 0x%P  |  RBX: 0x%P  |  RCX: 0x%P\n", 
                  cpu_state->rax, cpu_state->rbx, cpu_state->rcx);
    kernel_printf("RDX: 0x%P  |  RSI: 0x%P  |  RDI: 0x%P\n", 
                  cpu_state->rdx,  cpu_state->rsi, cpu_state->rdi);
    kernel_printf("RBP: 0x%P  |  RSP: 0x%P  |  R8:  0x%P\n",
                  cpu_state->rbp, cpu_state->rsp, cpu_state->r8);
    kernel_printf("R9:  0x%P  |  R10: 0x%P  |  R11: 0x%P\n", 
                  cpu_state->r9, cpu_state->r10,  cpu_state->r11);
    kernel_printf("R12: 0x%P  |  R13: 0x%P  |  R14: 0x%P\n", 
                  cpu_state->r12, cpu_state->r13, cpu_state->r14);
    kernel_printf("R15: 0x%P\n", cpu_state->r15);
    kernel_printf("CR0: 0x%P  |  CR2: 0x%P  |  CR3: 0x%P\n", CR0, CR2, CR3);
    kernel_printf("CR4: 0x%P  |  EFLAGS: 0x%P\n", CR4, stack_state->rflags);
    kernel_printf("CS: 0x%04X | DS: 0x%04X | SS: 0x%04X | ", 
                  stack_state->cs & 0xFFFF,
                  cpu_state->ds & 0xFFFF, 
                  cpu_state->ss & 0xFFFF);
    kernel_printf("ES: 0x%04X | FS: 0x%04X | GS: 0x%04X\n", 
                  cpu_state->es & 0xFFFF ,
                  cpu_state->fs & 0xFFFF , 
                  cpu_state->gs & 0xFFFF);
    kernel_printf("                                                            "
                    "                   \n");
    kernel_printf("CF: %d | PF: %d | AF: %d | ZF: %d | SF: %d | "
                  "TF: %d | IF: %d | DF: %d |", cf_f, pf_f, af_f, zf_f, 
                  sf_f, tf_f, if_f, df_f);
    kernel_printf(" OF: %d | NT: %d\nRF: %d | VM: %d | "
                  "AC: %d | VF: %d | VP: %d | " 
                  "ID: %d |", of_f, nt_f, rf_f, vm_f, 
                  ac_f, vif_f, vip_f, id_f);
    kernel_printf(" IO: %d\n\n", (iopl0_f | iopl1_f << 1));
    kernel_printf("------------------------------- ADDITIONAL INFO ------------"
                    "--------------------\n");
    kernel_printf("  Core ID: %u  |  Thread:  %u  |  Time of panic: "
                  "%02u:%02u:%02u\n", current_cpu_id, sched_get_tid(), hours, 
                  minutes, seconds);
    kernel_printf("\n         THE KERNEL HAS BEEN PUT IN SLEEP MODE |" 
                  " PLEASE RESTART MANUALLY      ");

    /* Hide cursor */
    panic_scheme.background = BG_BLACK;
    panic_scheme.foreground = FG_BLACK;
    panic_scheme.vga_color  = 1;

    graphic_set_color_scheme(panic_scheme);

    kernel_printf(" ");

    /* We will never return from interrupt */
    while(1)
    {
        cpu_clear_interrupt();
        cpu_hlt();
    }
}

void kernel_panic(const uint32_t error_code)
{
    /* Save the error code to memory */
    panic_code = error_code;

    /* Raise INT */
    __asm__ __volatile__("int %0" :: "i" (PANIC_INT_LINE));
}

/*
#=============================    KERNEL PANIC    ============================#
|                                                                             |
| Reason: Panic generated by the kernel           INT ID: 0x2a                |
| Instruction [EIP]: 0x00207b1b                   Error code: 0x00000004      |
|                                                                             |
|================================= CPU STATE =================================|
|                                                                             |
| RAX: 0x00000004  |  EBX: 0x000002ca  |  ECX: 0x000000c8  |  EDX: 0x00000000 |
| ESI: 0x00000000  |  EDI: 0x88888889  |  EBP: 0x003098b8  |  ESP: 0x00309838 |
| CR0: 0x00000000  |  CR2: 0x00000000  |  CR3: 0x00000000  |  CR4: 0x00202f60 |
| EFLAGS: 0x00000216  |                                                       |
|                                                                             |
|============================= SEGMENT REGISTERS =============================|
|                                                                             |
| CS: 0x0008  |  DS: 0x0010  |  SS: 0x0010                                    |
| ES: 0x0010  |  FS: 0x0010  |  GS: 0x0010                                    |
|                                                                             |
|============================== ADDITIONAL INFO ==============================|
|                                                                             |
| Core ID: 0x00000000                                                         |
| Thread:  000000003                                                          |
| Inst:    c3 66 90 66 (Address: 0x00207b1b)                                  |
|                                                                             |
|                         LET'S HOPE IT WON'T EXPLODE                         |
#=============================================================================#
*/