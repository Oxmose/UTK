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

#include <cpu.h>                  /* CPU management */
#include <pic.h>                  /* PIC driver */
#include <pit.h>                  /* PIT driver */
#include <rtc.h>                  /* RTC driver */
#include <acpi.h>                 /* ACPI management */
#include <vesa.h>                 /* VESA driver */
#include <lapic.h>                /* LAPIC driver */
#include <serial.h>               /* Serial driver */
#include <io_apic.h>              /* IO-APIC driver */
#include <ata_pio.h>              /* ATA PIO driver */
#include <keyboard.h>             /* Keyboard driver */
#include <vga_text.h>             /* VGA display driver */
#include <lib/stddef.h>           /* Standard definitions */
#include <lib/string.h>           /* String manipulation */
#include <core/panic.h>           /* Kernel panic */
#include <core/scheduler.h>       /* Kernel scheduler */
#include <memory/kheap.h>         /* Kernel heap */
#include <memory/paging.h>        /* Memory paging management */
#include <memory/meminfo.h>       /* Memory information */
#include <memory/memalloc.h>      /* Memory pools */
#include <io/kernel_output.h>     /* Kernel output methods */
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

#define INIT_MSG(msg_success, msg_error, error, panic) \
    {                                                  \
        if (error != OS_NO_ERR)                        \
        {                                              \
            kernel_error(msg_error, error);            \
            if (panic == 1)                            \
            {                                          \
                kernel_panic(error);                   \
            }                                          \
        }                                              \
        else if (strlen(msg_success) != 0)             \
        {                                              \
            kernel_success(msg_success);               \
        }                                              \
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
    colorscheme_t new_scheme;
    colorscheme_t buffer;

#if TEST_MODE_ENABLED
    boot_test();
    output_test();
    panic_test();
#endif

#if DISPLAY_TYPE != DISPLAY_SERIAL
    err = vga_init();
    err |= graphic_set_selected_driver(&vga_text_driver);
    INIT_MSG("VGA driver initialized", "Could not initialize VGA driver [%u]",
             err, 1);
#else
    err = graphic_set_selected_driver(&serial_text_driver);
    INIT_MSG("Serial driver initialized",
             "Could not initialize serial driver [%u]",
             err, 1);
#endif

#if KERNEL_DEBUG == 1
    kernel_serial_debug("Kickstarting the kernel\n");
#endif
    graphic_clear_screen();
    kernel_printf("\r ============================== Kickstarting UTK "
                  "==============================\n");

    err = cpu_detect(1);
    INIT_MSG("", "Error while detecting CPU [%u]\n", err, 1);

    err = kheap_init();
    INIT_MSG("Kernel heap initialized\n",
             "Could not initialize kernel heap [%u]\n",
             err, 1);

    err = kernel_interrupt_init();
    INIT_MSG("Kernel interrupt manager initialized\n",
             "Could not initialize kernel interrupt manager [%u]\n",
             err, 1);

    err = kernel_exception_init();
    INIT_MSG("Kernel exception manager initialized\n",
             "Could not initialize kernel exception manager [%u]\n",
             err, 1);

    err = memory_map_init();
    INIT_MSG("",
             "Could not get memory map [%u]\n",
             err, 1);

    err = memalloc_init();
    INIT_MSG("Memory pools initialized\n",
             "Could not initialize memory pools [%u]\n",
             err, 1);

    err = paging_init();
    INIT_MSG("",
             "Could not initialize kernel page directory [%u]\n",
             err, 1);

    err = vga_map_memory();
    INIT_MSG("",
             "Could not map VGA memory [%u]\n",
             err, 1);

    err = paging_enable();
    INIT_MSG("Paging enabled\n",
             "Could not enable paging [%u]\n",
             err, 1);

#if TEST_MODE_ENABLED
    paging_test();
    bios_call_test();
    kernel_queue_test();
#endif

#if ((DISPLAY_TYPE == DISPLAY_VESA || DISPLAY_TYPE == DISPLAY_VESA_BUF) || \
     (TEST_MODE_ENABLED == 1 && VESA_TEXT_TEST == 1))
    err = vesa_init();
    INIT_MSG("VESA driver initialized\n",
             "Could not initialize VESA driver [%u]\n",
             err, 1);

    err = vesa_text_vga_to_vesa();
    INIT_MSG("",
             "Could not switch to VESA driver [%u]\n",
             err, 1);

#if TEST_MODE_ENABLED
    vesa_text_test();
#endif
#endif

    err = acpi_init();
    INIT_MSG("ACPI initialized\n",
             "Could not initialize ACPI [%u]\n",
             err, 1);

    err = pic_init();
    INIT_MSG("PIC initialized\n",
             "Could not initialize PIC [%u]\n",
             err, 1);

    if (io_apic_capable())
    {
        err = io_apic_init();
        INIT_MSG("IO-APIC initialized\n",
                 "Could not initialize IO-APIC [%u]\n",
                 err, 1);

        err = kernel_interrupt_set_driver(&io_apic_driver);
        INIT_MSG("",
                 "Could not set IO-APIC driver [%u]\n",
                 err, 1);

        err = pic_disable();
        INIT_MSG("",
                 "Could not disable PIC [%u]\n",
                 err, 1);

        err = lapic_init();
        INIT_MSG("LAPIC initialized\n",
                 "Could not disable LAPIC [%u]\n",
                 err, 1);
    }
    else
    {
        err = kernel_interrupt_set_driver(&pic_driver);
        INIT_MSG("",
                 "Could not set PIC driver [%u]\n",
                 err, 1);
    }

    err = pit_init();
    INIT_MSG("PIT initialized\n",
             "Could not initialize PIT driver [%u]\n",
             err, 1);

    err = rtc_init();
    INIT_MSG("RTC initialized\n",
             "Could not initialize RTC driver [%u]\n",
             err, 1);

    if (io_apic_capable())
    {
        err = lapic_timer_init();
        INIT_MSG("LAPIC timer initialized\n",
                 "Could not initialize LAPIC timer driver [%u]\n",
                 err, 1);

        err = time_init(&lapic_timer_driver, &rtc_driver, &pit_driver);
        INIT_MSG("Timer factory initialized\n",
                 "Could not initialize timer factory [%u]\n",
                 err, 1);
    }
    else
    {
        err = time_init(&pit_driver, &rtc_driver, NULL);
        INIT_MSG("Timer factory initialized\n",
                 "Could not initialize timer factory [%u]\n",
                 err, 1);
    }

    err = cpu_enable_sse();
    INIT_MSG("SSE initialized\n",
             "Could not initialize SSE support [%u]\n",
             err, 1);

    err = keyboard_init();
    INIT_MSG("Keyboard initialized\n",
             "Could not initialize keyboard driver [%u]\n",
             err, 1);

    err = ata_pio_init();
    INIT_MSG("ATA-PIO initialized\n",
             "Could not initialize ATA-PIO driver [%u]\n",
             err, 1);

    err = cpu_smp_init();
    INIT_MSG("SMP initialized\n",
             "Could not initialize SMP [%u]\n",
             err, 1);

    err = sched_init();
    INIT_MSG("Scheduler initialized\n",
             "Could not initialize scheduler [%u]\n",
             err, 1);

# if DISPLAY_TYPE == DISPLAY_VESA_BUF
    /* Create the VESA double buffer thread */
    err = sched_create_kernel_thread(NULL, 
                                     KERNEL_HIGHEST_PRIORITY,
                                     "vesa_buf", 
                                     0x1000,
                                     0, 
                                     vesa_double_buffer_thread,
                                     (void*)0);
    INIT_MSG("VESA buffer initialized\n",
             "Could not initialize VESA buffer [%u]\n",
             err, 1);
#endif

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

        /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("\n ================================ UTK Started "
                  "================================ \n\n");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    /* First schedule, we should never return from here */
    sched_schedule();

    INIT_MSG("",
             "Kernel returned to kickstart\n", OS_ERR_UNAUTHORIZED_ACTION, 1);
}