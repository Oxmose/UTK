/*******************************************************************************
 * @file rtc.c
 *
 * @see rtc.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2021
 *
 * @version 1.0
 *
 * @brief RTC (Real Time Clock) driver.
 *
 * @details RTC (Real Time Clock) driver. Used as the kernel's time base. Timer
 * source in the kernel. This driver provides basic access to the RTC.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <cpu.h>                /* CPU manipulation */
#include <kernel_output.h>      /* Kernel output methods */
#include <interrupts.h>         /* Interrupts management */
#include <interrupt_settings.h> /* Interrupts settings */
#include <stdint.h>             /* Generic int types */
#include <stddef.h>             /* Standard definition */
#include <time_management.h>    /* Timer factory */
#include <critical.h>           /* Critical sections */
#include <panic.h>              /* Kernel panic */
#include <kernel_error.h>       /* Kernel error codes */
#include <rt_clock.h>           /* BSP RTC declarations */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <rtc.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* RTC settings */
/** @brief Initial RTC tick rate. */
#define RTC_INIT_RATE 10
/** @brief RTC minimal frequency. */
#define RTC_MIN_FREQ 2
/** @brief RTC maximal frequency. */
#define RTC_MAX_FREQ 8192
/** @brief RTC quartz frequency. */
#define RTC_QUARTZ_FREQ 32768

/* CMOS registers  */
/** @brief CMOS seconds register id. */
#define CMOS_SECONDS_REGISTER 0x00
/** @brief CMOS minutes register id. */
#define CMOS_MINUTES_REGISTER 0x02
/** @brief CMOS hours register id. */
#define CMOS_HOURS_REGISTER   0x04
/** @brief CMOS day of the week register id. */
#define CMOS_WEEKDAY_REGISTER 0x06
/** @brief CMOS day register id. */
#define CMOS_DAY_REGISTER     0x07
/** @brief CMOS month register id. */
#define CMOS_MONTH_REGISTER   0x08
/** @brief CMOS year register id. */
#define CMOS_YEAR_REGISTER    0x09
/** @brief CMOS century register id. */
#define CMOS_CENTURY_REGISTER 0x00

/* CMOS setings */
/** @brief CMOS NMI disabler bit. */
#define CMOS_NMI_DISABLE_BIT 0x01
/** @brief CMOS RTC enabler bit. */
#define CMOS_ENABLE_RTC      0x40
/** @brief CMOS A register id. */
#define CMOS_REG_A           0x0A
/** @brief CMOS B register id. */
#define CMOS_REG_B           0x0B
/** @brief CMOS C register id. */
#define CMOS_REG_C           0x0C

/** @brief CMOS CPU command port id. */
#define CMOS_COMM_PORT 0x70
/** @brief CMOS CPU data port id. */
#define CMOS_DATA_PORT 0x71

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Assert macro used by the RTC to ensure correctness of execution.
 *
 * @details Assert macro used by the RTC to ensure correctness of execution.
 * Due to the critical nature of the RTC, any error generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define RTC_ASSERT(COND, MSG, ERROR) {                      \
    if((COND) == FALSE)                                     \
    {                                                       \
        PANIC(ERROR, "RTC", MSG, TRUE);                     \
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
/** @brief Stores the real day time in seconds. */
static uint32_t day_time;

/** @brief Stores the system's current date. */
static date_t date;

/** @brief Keeps track on the RTC enabled state. */
static uint32_t disabled_nesting;

/** @brief Keeps track of the current frequency. */
static uint32_t rtc_frequency;

/** @brief RTC driver instance. */
static kernel_timer_t rtc_driver = {
    .get_frequency  = rtc_get_frequency,
    .set_frequency  = rtc_set_frequency,
    .enable         = rtc_enable,
    .disable        = rtc_disable,
    .set_handler    = rtc_set_handler,
    .remove_handler = rtc_remove_handler,
    .get_irq        = rtc_get_irq

};

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Initial RTC interrupt handler.
 *
 * @details RTC interrupt handler set at the initialization of the RTC.
 * Dummy routine setting EOI.
 *
 * @param[in, out] cpu_state The cpu registers before the interrupt.
 * @param[in] int_id The interrupt line that called the handler.
 * @param[in, out] stack_state The stack state before the interrupt.
 */
static void dummy_handler(cpu_state_t* cpu_state,
                          uintptr_t int_id,
                          stack_state_t* stack_state);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void dummy_handler(cpu_state_t* cpu_state,
                          uintptr_t int_id,
                          stack_state_t* stack_state)
{
    (void)cpu_state;
    (void)int_id;
    (void)stack_state;

    rtc_update_time();

    /* EOI */
    kernel_interrupt_set_irq_eoi(RTC_IRQ_LINE);
}

void rtc_init(void)
{

    OS_RETURN_E err;
    int8_t      prev_ored;
    int8_t      prev_rate;

    /* Init real times */
    day_time     = 0;
    date.weekday = 0;
    date.day     = 0;
    date.month   = 0;
    date.year    = 0;

    /* Init CMOS IRQ8 */
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_B, CMOS_COMM_PORT);
    prev_ored = cpu_inb(CMOS_DATA_PORT);
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_B, CMOS_COMM_PORT);
    cpu_outb(prev_ored | CMOS_ENABLE_RTC, CMOS_DATA_PORT);

    /* Init CMOS IRQ8 rate */
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    prev_rate = cpu_inb(CMOS_DATA_PORT);
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    cpu_outb((prev_rate & 0xF0) | RTC_INIT_RATE, CMOS_DATA_PORT);
    rtc_frequency = (RTC_QUARTZ_FREQ >> (RTC_INIT_RATE - 1));

    /* Set rtc clock interrupt handler */
    err = kernel_interrupt_register_irq_handler(RTC_IRQ_LINE, dummy_handler);
    RTC_ASSERT(err == OS_NO_ERR, "Could not register RTC handler", err);

    /* Set mask before setting IRQ */
    kernel_interrupt_set_irq_mask(RTC_IRQ_LINE, 1);

    /* Just dummy read register C to unlock interrupt */
    cpu_outb(CMOS_REG_C, CMOS_COMM_PORT);
    cpu_inb(CMOS_DATA_PORT);

    disabled_nesting = 1;

    rtc_set_frequency(KERNEL_RTC_TIMER_FREQ);

    rtc_enable();

    rtc_update_time();

    err = time_register_rtc_manager(rtc_update_time);
    RTC_ASSERT(err == OS_NO_ERR, "Could not register RTC manager", err);

    KERNEL_TEST_POINT(rtc_test);
    KERNEL_TEST_POINT(rtc_test2);
    KERNEL_TEST_POINT(rtc_test3);

    KERNEL_DEBUG(RTC_DEBUG_ENABLED, "[RTC] Initialized");
}

void rtc_enable(void)
{
    uint32_t int_state;

    ENTER_CRITICAL(int_state);

    if(disabled_nesting > 0)
    {
        --disabled_nesting;
    }

    KERNEL_DEBUG(RTC_DEBUG_ENABLED,
                 "[RTC] Enable RTC (nesting %d)",
                 disabled_nesting);
    if(disabled_nesting == 0)
    {
        kernel_interrupt_set_irq_mask(RTC_IRQ_LINE, 1);
    }

    EXIT_CRITICAL(int_state);
}

void rtc_disable(void)
{
    uint32_t    int_state;

    ENTER_CRITICAL(int_state);

    if(disabled_nesting < UINT32_MAX)
    {
        ++disabled_nesting;
    }

    KERNEL_DEBUG(RTC_DEBUG_ENABLED,
                 "[RTC] Disable RTC (nesting %d)",
                 disabled_nesting);
    kernel_interrupt_set_irq_mask(RTC_IRQ_LINE, 0);

    EXIT_CRITICAL(int_state);
}

void rtc_set_frequency(const uint32_t frequency)
{
    uint32_t    prev_rate;
    uint32_t    rate;
    uint32_t    int_state;

    RTC_ASSERT((frequency >= RTC_MIN_FREQ && frequency <= RTC_MAX_FREQ),
               "RTC timer frequency out of bound",
               OS_ERR_OUT_OF_BOUND);

    /* Choose the closest rate to the frequency */
    if(frequency < 4)
    {
        rate = 15;
    }
    else if(frequency < 8)
    {
        rate = 14;
    }
    else if(frequency < 16)
    {
        rate = 13;
    }
    else if(frequency < 32)
    {
        rate = 12;
    }
    else if(frequency < 64)
    {
        rate = 11;
    }
    else if(frequency < 128)
    {
        rate = 10;
    }
    else if(frequency < 256)
    {
        rate = 9;
    }
    else if(frequency < 512)
    {
        rate = 8;
    }
    else if(frequency < 1024)
    {
        rate = 7;
    }
    else if(frequency < 2048)
    {
        rate = 6;
    }
    else if(frequency < 4096)
    {
        rate = 5;
    }
    else if(frequency < 8192)
    {
        rate = 4;
    }
    else
    {
        rate = 3;
    }

    ENTER_CRITICAL(int_state);

    /* Disable RTC IRQ */
    rtc_disable();

    /* Set clock frequency */
     /* Init CMOS IRQ8 rate */
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    prev_rate = cpu_inb(CMOS_DATA_PORT);
    cpu_outb((CMOS_NMI_DISABLE_BIT << 7) | CMOS_REG_A, CMOS_COMM_PORT);
    cpu_outb((prev_rate & 0xF0) | rate, CMOS_DATA_PORT);

    rtc_frequency = (RTC_QUARTZ_FREQ >> (rate - 1));

    KERNEL_DEBUG(RTC_DEBUG_ENABLED,
                 "[RTC] New RTC rate set (%d: %dHz)",
                 rate,
                 rtc_frequency);

    EXIT_CRITICAL(int_state);

    /* Enable RTC IRQ */
    rtc_enable();
}

uint32_t rtc_get_frequency(void)
{
    return rtc_frequency;
}

OS_RETURN_E rtc_set_handler(void(*handler)(
                                 cpu_state_t*,
                                 uintptr_t,
                                 stack_state_t*
                                 ))
{
    OS_RETURN_E err;
    uint32_t    int_state;

    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    ENTER_CRITICAL(int_state);

    rtc_disable();

    /* Remove the current handler */
    err = kernel_interrupt_remove_irq_handler(RTC_IRQ_LINE);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        rtc_enable();
        return err;
    }

    err = kernel_interrupt_register_irq_handler(RTC_IRQ_LINE, handler);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(int_state);
        return err;
    }

    KERNEL_DEBUG(RTC_DEBUG_ENABLED,
                 "[RTC] New RTC handler set (0x%p)",
                 handler);

    EXIT_CRITICAL(int_state);

    rtc_enable();

    return err;
}

OS_RETURN_E rtc_remove_handler(void)
{
    KERNEL_DEBUG(RTC_DEBUG_ENABLED, "Default RTC handler set");
    return rtc_set_handler(dummy_handler);
}

uint32_t rtc_get_current_daytime(void)
{
    return day_time;
}

date_t rtc_get_current_date(void)
{
    return date;
}

void rtc_update_time(void)
{
    int8_t nmi_info;
    uint8_t seconds;
    uint8_t minutes;
    uint32_t hours;
    uint8_t century;
    uint8_t reg_b;

    /* Set NMI info bit */
    nmi_info = 0;

    /* Set time */
    /* Select CMOS seconds register and read */
    cpu_outb(nmi_info | CMOS_SECONDS_REGISTER, CMOS_COMM_PORT);
    seconds = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS minutes register and read */
    cpu_outb(nmi_info | CMOS_MINUTES_REGISTER, CMOS_COMM_PORT);
    minutes = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS hours register and read */
    cpu_outb(nmi_info | CMOS_HOURS_REGISTER, CMOS_COMM_PORT);
    hours = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS day register and read */
    cpu_outb(nmi_info | CMOS_DAY_REGISTER, CMOS_COMM_PORT);
    date.day = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS month register and read */
    cpu_outb(nmi_info | CMOS_MONTH_REGISTER, CMOS_COMM_PORT);
    date.month = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS years register and read */
    cpu_outb(nmi_info | CMOS_YEAR_REGISTER, CMOS_COMM_PORT);
    date.year = cpu_inb(CMOS_DATA_PORT);

    /* Select CMOS century register and read */
    if(CMOS_CENTURY_REGISTER != 0)
    {
        cpu_outb(nmi_info | CMOS_CENTURY_REGISTER, CMOS_COMM_PORT);
        century = cpu_inb(CMOS_DATA_PORT);
    }
    else
    {
        century = CURRENT_YEAR / 100;
    }

    /* Convert BCD to binary if necessary */
    cpu_outb(CMOS_REG_B, CMOS_COMM_PORT);
    reg_b = cpu_inb(CMOS_DATA_PORT);

    if((reg_b & 0x04) == 0)
    {
        seconds = (seconds & 0x0F) + ((seconds / 16) * 10);
        minutes = (minutes & 0x0F) + ((minutes / 16) * 10);
        hours = ((hours & 0x0F) + (((hours & 0x70) / 16) * 10)) |
            (hours & 0x80);
        date.day = (date.day & 0x0F) + ((date.day / 16) * 10);
        date.month = (date.month & 0x0F) + ((date.month / 16) * 10);
        date.year = (date.year & 0x0F) + ((date.year / 16) * 10);

        if(CMOS_CENTURY_REGISTER != 0)
        {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    /*  Convert to 24H */
    if((reg_b & 0x02) == 0 && (hours & 0x80) >= 1)
    {
        hours = ((hours & 0x7F) + 12) % 24;
    }

    /* Get year */
    if(CMOS_CENTURY_REGISTER != 0)
    {
        date.year += century * 100;
    }
    else
    {
        date.year = date.year + 2000;
    }

    /* Compute week day and day time */
    date.weekday = ((date.day + date.month + date.year + date.year / 4)
            + 1) % 7 + 1;
    day_time = seconds + 60 * minutes + 3600 * hours;

    /* Clear C Register */
    cpu_outb(CMOS_REG_C, CMOS_COMM_PORT);
    cpu_inb(CMOS_DATA_PORT);

    KERNEL_DEBUG(RTC_DEBUG_ENABLED, "[RTC] Updated RTC");
}

uint32_t rtc_get_irq(void)
{
    return RTC_IRQ_LINE;
}

const kernel_timer_t* rtc_get_driver(void)
{
    return &rtc_driver;
}

/************************************ EOF *************************************/