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
#include <memmgt.h>                /* Memory mapping informations */
#include <acpi.h>                  /* ACPI manager */
#include <pic.h>                   /* PIC driver */
#include <io_apic.h>               /* IO APIC driver */
#include <lapic.h>                 /* LAPIC driver */
#include <rt_clock.h>              /* RTC driver */
#include <pit.h>                   /* PIT driver */
#include <time_management.h>       /* Timer factory */
#include <bsp_api.h>               /* BSP API */
#include <scheduler.h>             /* Kernel scheduler */
#include <syscall.h>               /* System calls manager */
#include <init_rd.h>               /* Init ram disk */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#ifdef TEST_MODE_ENABLED
#include <test_bank.h>
#endif

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
    OS_RETURN_E     err;
    initrd_device_t initrd_device;

    /* Init uart for basic log */
    graphic_set_selected_driver(uart_get_driver());
    uart_init();

    /* Initialize CPU structures */
    cpu_setup_gdt();
    cpu_setup_idt();
    cpu_setup_tss();

#ifdef TEST_MODE_ENABLED
    boot_test();
    output_test();
#endif

    KERNEL_DEBUG(KICKSTART_DEBUG_ENABLED, "[KICKSTART] Kickstarting kernel");

    /* Validate architecture support */
    validate_architecture();

    kheap_init();
    KERNEL_SUCCESS("Kernel heap initialized\n");

#ifdef TEST_MODE_ENABLED
    queue_test();
#endif

    kernel_interrupt_init();
    KERNEL_SUCCESS("Interrupt manager initialized\n");

    kernel_exception_init();
    KERNEL_SUCCESS("Exception manager initialized\n");

    memory_manager_init();
    KERNEL_SUCCESS("Memory manager initialized\n");
    
    vga_init();
    err = graphic_set_selected_driver(vga_text_get_driver());
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not set VGA driver\n");
        KERNEL_PANIC(err);
    }
    KERNEL_SUCCESS("VGA driver initialized\n");

    acpi_init();
    KERNEL_SUCCESS("ACPI initialized\n");
    KERNEL_INFO("Number of detected CPU: %d\n", get_cpu_count());

    pic_init();
    KERNEL_SUCCESS("PIC initialized\n");

    if(io_apic_capable() == 1)
    {
        pic_disable();

        io_apic_init();
        KERNEL_SUCCESS("IO-APIC initialized\n");
        err = kernel_interrupt_set_driver(io_apic_get_driver());
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not set IO-APIC driver\n");
            KERNEL_PANIC(err);
        }

        lapic_init();
        KERNEL_SUCCESS("LAPIC initialized\n");
    }
    else
    {
        err = kernel_interrupt_set_driver(pic_get_driver());
        if(err != OS_NO_ERR)
        {
            KERNEL_ERROR("Could not set PIC driver\n");
            KERNEL_PANIC(err);
        }
    }

    pit_init();
    KERNEL_SUCCESS("PIT initialized\n");

    rtc_init();
    KERNEL_SUCCESS("RTC initialized\n");

    if(io_apic_capable() == 1)
    {
        lapic_timer_init();
        KERNEL_SUCCESS("LAPIC timer initialized\n");

        pit_disable();
        
        time_init(lapic_timer_get_driver(), rtc_get_driver());
    
        KERNEL_SUCCESS("Timer factory initialized\n");
    }
    else 
    {
        time_init(pit_get_driver(), rtc_get_driver());
        KERNEL_SUCCESS("Timer factory initialized\n");
    }    

    syscall_init();
    KERNEL_SUCCESS("System calls initialized\n");

#ifdef TEST_MODE_ENABLED
    bios_call_test();
    panic_test();
#endif

    /* Initialize the init ram disk */
    err = initrd_init_device(&initrd_device);
    if(err != OS_NO_ERR)
    {
        KERNEL_ERROR("Could not initialize init ram disk\n");
        KERNEL_PANIC(err);
    }

    /* First schedule, we should never return from here */
    sched_init();
    KERNEL_SUCCESS("Scheduler initialized\n");


    KERNEL_ERROR("Kernel returned to kickstart\n");
    KERNEL_PANIC(OS_ERR_UNAUTHORIZED_ACTION);
}