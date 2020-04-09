/*******************************************************************************
 * @file test_bank.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 22/09/2018
 *
 * @version 1.0
 *
 * @brief Kernel's main test bank.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __TEST_BANK_H_
#define __TEST_BANK_H_

/* Set here the test that should run (only one at a time) */
#define SERIAL_TEST 0
#define IDT_TEST 0
#define GDT_TEST 0
#define TSS_TEST 0
#define BOOT_TEST 0
#define OUTPUT_TEST 0
#define VGA_TEST 0
#define KHEAP_TEST 0
#define INTERRUPT_TEST 0
#define EXCEPTION_TEST 0
#define MEMALLOC_TEST 0
#define PAGING_TEST 0
#define ACPI_TEST 0
#define PIC_TEST 0
#define IO_APIC_TEST 0
#define LAPIC_TEST 0
#define PIT_TEST 0
#define RTC_TEST 0
#define LAPIC_TIMER_TEST 0
#define TIME_TEST 0
#define PANIC_TEST 0
#define BIOS_CALL_TEST 0
#define VESA_TEXT_TEST 0
#define ATA_PIO_TEST 0
#define CPU_SMP_TEST 0
#define SSE_TEST 0
#define CRITICAL_TEST 0
#define DIV_BY_ZERO_TEST 0
#define KERNEL_QUEUE_TEST 0
#define SCHEDULER_LOAD_TEST 0
#define SCHEDULER_LOAD_MC_TEST 0
#define SCHEDULER_PREEMT_TEST 0
#define SCHEDULER_SLEEP_TEST 0
#define SCHEDULER_SLEEP_MC_TEST 0
#define MUTEX_TEST 0
#define SEMAPHORE_TEST 0
#define SEMAPHORE_MC_TEST 0
#define MAILBOX_TEST 0
#define USERQUEUE_TEST 0
#define PAGING_ALLOC_TEST 0
#define MUTEX_MC_TEST 0
#define SPINLOCK_TEST 0

/* Put tests declarations here */
void serial_test(void);
void idt_test(void);
void gdt_test(void);
void tss_test(void);
void boot_test(void);
void output_test(void);
void vga_test(void);
void kheap_test(void);
void interrupt_test(void);
void exception_test(void);
void memalloc_test(void);
void paging_test(void);
void acpi_test(void);
void pic_test(void);
void io_apic_test(void);
void lapic_test(void);
void pit_test(void);
void rtc_test(void);
void lapic_timer_test(void);
void time_test(void);
void panic_test(void);
void bios_call_test(void);
void vesa_text_test(void);
void ata_pio_test(void);
void cpu_smp_test(void);
void sse_test(void);
void critical_test(void);
void div_by_zero_test(void);
void kernel_queue_test(void);
void scheduler_load_test(void);
void scheduler_load_mc_test(void);
void scheduler_preemt_test(void);
void scheduler_sleep_test(void);
void scheduler_sleep_mc_test(void);
void mutex_test(void);
void mutex_mc_test(void);
void semaphore_test(void);
void semaphore_mc_test(void);
void mailbox_test(void);
void userqueue_test(void);
void spinlock_test(void);

#endif /* __TEST_BANK_H_ */