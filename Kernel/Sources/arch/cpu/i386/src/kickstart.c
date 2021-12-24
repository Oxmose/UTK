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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
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
#include <futex.h>                 /* FUtex API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
/* None */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define KICKSTART_ASSERT(COND, MSG, ERROR) {                \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "KICKSTRART", MSG, TRUE);              \
    }                                                       \
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

/**
 * @brief Main boot sequence, C kernel entry point.
 *
 * @details Main boot sequence, C kernel entry point. Initializes each basic
 * drivers for the kernel, then init the scheduler and start the system.
 *
 * @warning This function should never return. In case of return, the kernel
 * should be able to catch the return as an error.
 */
void kernel_kickstart(void);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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

    KERNEL_TEST_POINT(boot_test);
    KERNEL_TEST_POINT(output_test);

    KERNEL_DEBUG(KICKSTART_DEBUG_ENABLED, "[KICKSTART] Kickstarting kernel");

    /* Validate architecture support */
    validate_architecture();

    kheap_init();
    KERNEL_SUCCESS("Kernel heap initialized\n");

    KERNEL_TEST_POINT(queue_test);
    KERNEL_TEST_POINT(kqueue_test);
    KERNEL_TEST_POINT(vector_test);
    KERNEL_TEST_POINT(uhashtable_test);

    kernel_interrupt_init();
    KERNEL_SUCCESS("Interrupt manager initialized\n");

    kernel_exception_init();
    KERNEL_SUCCESS("Exception manager initialized\n");

    memory_manager_init();
    KERNEL_SUCCESS("Memory manager initialized\n");

    vga_init();
    err = graphic_set_selected_driver(vga_text_get_driver());
    KICKSTART_ASSERT(err == OS_NO_ERR, "Could not set VGA driver", err);
    KERNEL_SUCCESS("VGA driver initialized\n");

    acpi_init();
    KERNEL_SUCCESS("ACPI initialized\n");
    KERNEL_INFO("Number of detected CPU: %d\n", get_cpu_count());

    pic_init();
    KERNEL_SUCCESS("PIC initialized\n");

    KICKSTART_ASSERT(io_apic_capable() == TRUE,
                     "IO-APIC not supported",
                     OS_ERR_NOT_SUPPORTED);

    pic_disable();
    io_apic_init();
    KERNEL_SUCCESS("IO-APIC initialized\n");
    err = kernel_interrupt_set_driver(io_apic_get_driver());
    KICKSTART_ASSERT(err == OS_NO_ERR, "Could not set IO-APIC driver", err);

    lapic_init();
    KERNEL_SUCCESS("LAPIC initialized\n");

    pit_init();
    KERNEL_SUCCESS("PIT initialized\n");

    rtc_init();
    KERNEL_SUCCESS("RTC initialized\n");

    lapic_timer_init();
    KERNEL_SUCCESS("LAPIC timer initialized\n");

    time_init(lapic_timer_get_driver(), rtc_get_driver());
    KERNEL_SUCCESS("Timer factory initialized\n");

    syscall_init();
    KERNEL_SUCCESS("System calls initialized\n");

    KERNEL_TEST_POINT(bios_call_test);
    KERNEL_TEST_POINT(panic_test);

    futex_init();
    KERNEL_SUCCESS("Futex initialized\n");

    /* Initialize the init ram disk */
    err = initrd_init_device(&initrd_device);
    KICKSTART_ASSERT(err == OS_NO_ERR, "Could not init INITRD", err);

    /* First schedule, we should never return from here */
    sched_init();

    KICKSTART_ASSERT(FALSE,
                     "Kernel returned to kickstart",
                     OS_ERR_UNAUTHORIZED_ACTION);
}

/************************************ EOF *************************************/