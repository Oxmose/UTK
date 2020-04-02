/***************************************************************************//**
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

/* Put tests declarations here */
void serial_test(void);
void idt_test(void);
void gdt_test(void);
void tss_test(void);
#endif /* __TEST_BANK_H_ */