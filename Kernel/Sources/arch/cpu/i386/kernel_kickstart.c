/*******************************************************************************
 * @file kernel_kickstart.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/12/2017
 *
 * @version 1.0
 *
 * @brief Kernel's main boot sequence.
 *
 * @warning At this point interrupts should be disabled.
 *
 * @details Kernel's booting sequence. Initializes the rest of the kernel after
 *  GDT, IDT and TSS initialization. Initializes the hardware and software
 * core of the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <io/kernel_output.h>     /* Kernel output methods */ 
#include <lib/stddef.h>           /* Standard definitions */
#include <lib/string.h>           /* String manipulation */
#include <vga_text.h>             /* VGA display driver */
#include <cpu.h>                  /* CPU management */
#include <memory/kheap.h>         /* Kernel heap */
#include <memory/meminfo.h>       /* Memory information */
#include <memory/memalloc.h>      /* Memory pools */
#include <interrupt/interrupts.h> /* Kernel interrupt manager */
#include <interrupt/exceptions.h> /* Kernel exception manager */

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

/* TODO: remove */
extern void kernel_panic(uint32_t);

#define INIT_MSG(msg_success, msg_error, error, panic) {     \
    if(error != OS_NO_ERR)                                   \
    {                                                        \
        kernel_error(msg_error, error);                      \
        if(panic == 1)                                       \
        {                                                    \
            kernel_panic(error);                             \
        }                                                    \
    }                                                        \
    else if(strlen(msg_success) != 0)                        \
    {                                                        \
        kernel_success(msg_success);                         \
    }                                                        \
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
    OS_RETURN_E err;
    (void)err;

    #if TEST_MODE_ENABLED
    boot_test();
    output_test();
    #endif

    /* Init VGA display */
    err = vga_init();
    err |= graphic_set_selected_driver(&vga_text_driver); 
    INIT_MSG("VGA driver initialized", "Could not initialize VGA driver", 
             err, 1);

    #if KERNEL_DEBUG == 1
    kernel_serial_debug("Kickstarting the kernel\n");
    #endif
    graphic_clear_screen();
    kernel_printf("\r ============================== Kickstarting UTK "
                  "==============================\n");

    err = cpu_detect(1);
    INIT_MSG("", "Error while detecting CPU: %d. HALTING\n",err, 1);

    err = kheap_init(); 
    INIT_MSG("Kernel heap initialized\n", "Could not initialize kernel heap\n", 
             err, 1);

    err = kernel_interrupt_init(); 
    INIT_MSG("Kernel interrupt manager initialized\n", 
             "Could not initialize kernel interrupt manager\n",
             err, 1);

    err = kernel_exception_init(); 
    INIT_MSG("Kernel exception manager initialized\n", 
             "Could not initialize kernel exception manager\n",
             err, 1);

    err = memory_map_init(); 
    INIT_MSG("", 
             "Could not get memory map\n",
             err, 1);
    
    err = memalloc_init(); 
    INIT_MSG("Memory pools initialized\n", 
             "Could not initialize memory pools\n",
             err, 1);
}