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

void kill_qemu(void);

#ifdef ARCH_I386

#define UART_TEST 0
#define IDT_TEST 0
#define GDT_TEST 0
#define TSS_TEST 0
#define BOOT_TEST 0
#define INTERRUPT_TEST 0
#define EXCEPTION_TEST 0
#define MEMMGR_TEST 0
#define PAGING_TEST 0
#define BIOS_CALL_TEST 0
#define ACPI_TEST 0
#define PIC_TEST 0
#define IO_APIC_TEST 0
#define LAPIC_TEST 0
#define PIT_TEST 0
#define RTC_TEST 0

void uart_test(void);
void idt_test(void);
void gdt_test(void);
void tss_test(void);
void boot_test(void);
void interrupt_test(void);
void exception_test(void);
void memmgr_test(void);
void paging_test(void);
void bios_call_test(void);
void acpi_test(void);
void pic_test(void);
void io_apic_test(void);
void lapic_test(void);
void pit_test(void);
void rtc_test(void);

#endif 

#define OUTPUT_TEST 0
#define KHEAP_TEST 0
#define PANIC_TEST 0
#define QUEUE_TEST 0
#define TIME_TEST 0

void output_test(void);
void kheap_test(void);
void panic_test(void);
void queue_test(void);
void time_test(void);

#endif /* __TEST_BANK_H_ */
