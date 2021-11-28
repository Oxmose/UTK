

#include <test_bank.h>

#if RTC_TEST2 == 1
#include <interrupts.h>
#include <kernel_output.h>
#include <cpu.h>
#include <rt_clock.h>
#include <rtc.h>
#include <panic.h>
#include <interrupt_settings.h>

/** @brief Initial RTC tick rate. */
#define RTC_INIT_RATE 10
/** @brief RTC minimal frequency. */
#define RTC_MIN_FREQ 2
/** @brief RTC maximal frequency. */
#define RTC_MAX_FREQ 8192

void rtc_test2(void)
{
    rtc_set_frequency(RTC_MIN_FREQ - 1);
    /* Kill QEMU */
    kill_qemu();
}
#else 
void rtc_test2(void)
{
}
#endif