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

#endif /* __TEST_BANK_H_ */