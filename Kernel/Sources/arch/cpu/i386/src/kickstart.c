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
#include <graphic.h>               /* Output manager */
#include <kernel_output.h>         /* Kernel output */
#include <uart.h>                  /* uart driver */
#include <vga_text.h>              /* VGA drivers */
#include <stddef.h>                /* Standard definitions */
#include <string.h>                /* String manipulations */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
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

static void validate_architecture(void)
{
    /* Read CPUID information */

    /* Check for FPU */

    /* Check for SSE2 */
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

    /* Validate architecture support */
    validate_architecture();

    /* Initialize CPU structures */
    cpu_setup_gdt();
    cpu_setup_idt();
    cpu_setup_tss();

#if TEST_MODE_ENABLED
    boot_test();
    output_test();
    panic_test();
#endif

    /* Start the VGA driver */
    err =  vga_init();
    err |= graphic_set_selected_driver(&vga_text_driver);
    INIT_MSG("VGA driver initialized\n", 
             "Could not initialize VGA driver [%u]\n",
             err, 1);

    KERNEL_SUCCESS("Kernel initialized\n");
    while(1);
}