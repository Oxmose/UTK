/*******************************************************************************
 * @file test_bank.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief Kernel's main test bank.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __TEST_BANK_H_
#define __TEST_BANK_H_

#include <config.h>

#ifdef TEST_MODE_ENABLED

#define KERNEL_TEST_POINT(func) { \
    func();                       \
}

void kill_qemu(void);

#ifdef ARCH_I386

#define UART_TEST 0
#define IDT_TEST 0
#define GDT_TEST 0
#define TSS_TEST 0
#define BOOT_TEST 0
#define INTERRUPT_TEST 0
#define EXCEPTION_TEST 0
#define PAGING_TEST 0
#define BIOS_CALL_TEST 0
#define ACPI_TEST 0
#define PIC_TEST 0
#define PIC_TEST2 0
#define PIC_TEST3 0
#define IO_APIC_TEST 0
#define IO_APIC_TEST2 0
#define LAPIC_TEST 0
#define LAPIC_TEST2 0
#define PIT_TEST 0
#define PIT_TEST2 0
#define PIT_TEST3 0
#define RTC_TEST 0
#define RTC_TEST2 0
#define RTC_TEST3 0
#define LAPIC_TIMER_TEST 0

void uart_test(void);
void idt_test(void);
void gdt_test(void);
void tss_test(void);
void boot_test(void);
void interrupt_test(void);
void exception_test(void);
void paging_test(void);
void bios_call_test(void);
void acpi_test(void);
void pic_test(void);
void pic_test2(void);
void pic_test3(void);
void io_apic_test(void);
void io_apic_test2(void);
void lapic_test(void);
void lapic_test2(void);
void pit_test(void);
void pit_test2(void);
void pit_test3(void);
void rtc_test(void);
void rtc_test2(void);
void rtc_test3(void);
void lapic_timer_test(void);

#endif

#define OUTPUT_TEST 0
#define KHEAP_TEST 0
#define PANIC_TEST 0
#define QUEUE_TEST 0
#define TIME_TEST 0
#define USTAR_TEST 0
#define FORK_TEST 0
#define USER_HEAP_TEST 0
#define MEMORY_USAGE_TEST 0
#define VECTOR_TEST 0
#define UHASHTABLE_TEST 0
#define CRITICAL_TEST 0
#define SCHEDULER_LOAD_TEST 0
#define SCHEDULER_PREEMPT_TEST 0
#define SCHEDULER_SLEEP_TEST 0
#define FUTEX_TEST 0
#define MUTEX_TEST 0
#define SEMAPHORE_TEST 0
#define SPINLOCK_TEST 0

void output_test(void);
void kheap_test(void);
void panic_test(void);
void queue_test(void);
void time_test(void);
void ustar_test(void);
void fork_test(void);
void user_heap_test(void);
void memory_usage_test(void);
void vector_test(void);
void uhashtable_test(void);
void critical_test(void);
void scheduler_load_test(void);
void scheduler_preempt_test(void);
void scheduler_sleep_test(void);
void futex_test(void);
void mutex_test(void);
void semaphore_test(void);
void spinlock_test(void);

#else
#define KERNEL_TEST_POINT(func)
#endif

#endif /* __TEST_BANK_H_ */
