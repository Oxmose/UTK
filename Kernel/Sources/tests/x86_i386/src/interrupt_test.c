
#include <test_bank.h>

#if INTERRUPT_TEST == 1

#include <interrupts.h>
#include <panic.h>
#include <kernel_output.h>
#include <cpu.h>
#include <cpu_settings.h>
#include <interrupt_settings.h>

static volatile uint32_t counter = 0;

void test_sw_interupts(void);

static void incrementer_handler(cpu_state_t* cpu,  uint32_t int_id,
                                stack_state_t* stack)
{
    (void)cpu;
    (void)int_id;
    (void)stack;

    if(counter < UINT32_MAX)
    {
        counter += int_id;
    }
}

static void decrementer_handler(cpu_state_t* cpu,  uint32_t int_id,
                                stack_state_t* stack)
{
    (void)cpu;
    (void)int_id;
    (void)stack;

    if(counter > 0)
    {
        counter -= int_id;
    }
}

static void test_sw_interupts_lock(void)
{
    OS_RETURN_E err;
    uint32_t int_state;

    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE, incrementer_handler)))
    {
        kernel_error("TEST_SW_INT_LOCK INIT\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK INIT\n");
    }

    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE + 1, decrementer_handler)))
    {
        kernel_error("TEST_SW_INT_LOCK INIT\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK INIT\n");
    }

    uint32_t cnt_val;

    /* Should not raise any interrupt */
    cnt_val = counter;
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 0 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 0\n");
    }

    kernel_interrupt_restore(1);

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 1 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 1\n");
    }

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val + 3 * MIN_INTERRUPT_LINE != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 2 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 2\n");
    }

    cnt_val = counter;

    kernel_interrupt_disable();
    int_state = 0;

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 3 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 3\n");
    }

    kernel_interrupt_restore(int_state);

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 4 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 4\n");
    }

    kernel_interrupt_restore(int_state);

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 5 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 5\n");
    }

    kernel_interrupt_restore(1);

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val + MIN_INTERRUPT_LINE != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 6 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 6\n");
    }

    kernel_interrupt_restore(1);
    kernel_interrupt_restore(1);
    int_state = kernel_interrupt_disable();

    cnt_val = counter;

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE));

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT_LOCK 7 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 7\n");
    }


    if((err = kernel_interrupt_remove_int_handler(MIN_INTERRUPT_LINE)))
    {
        kernel_error("TEST_SW_INT_LOCK 8\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 8\n");
    }

    if((err = kernel_interrupt_remove_int_handler(MIN_INTERRUPT_LINE + 1)))
    {
        kernel_error("TEST_SW_INT_LOCK 9\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT_LOCK 9\n");
    }

    kernel_printf("[TESTMODE] Interrupt lock OK\n");
}

void test_sw_interupts(void)
{
    uint32_t cnt_val;
    uint32_t i;
    uint32_t int_state;
    OS_RETURN_E err;

    /* We just dont care about HW interrupt from PIC, disable them */
    cpu_outb(0xFF, 0x21);
    cpu_outb(0xFF, 0xA1);

    /* TEST REGISTER < MIN */
    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE - 1, incrementer_handler))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_INT 0\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 0\n");
    }

    /* TEST REGISTER > MAX */
    if((err = kernel_interrupt_register_int_handler(MAX_INTERRUPT_LINE + 1, incrementer_handler))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_INT 1\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 1\n");
    }

    /* TEST REMOVE < MIN */
    if((err = kernel_interrupt_remove_int_handler(MIN_INTERRUPT_LINE - 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_INT 2\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 2\n");
    }

    /* TEST REMOVE > MAX */
    if((err = kernel_interrupt_remove_int_handler(MAX_INTERRUPT_LINE + 1))
     != OR_ERR_UNAUTHORIZED_INTERRUPT_LINE)
    {
        kernel_error("TEST_SW_INT 3\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 3\n");
    }

    /* TEST NULL HANDLER */
    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE, NULL))
     != OS_ERR_NULL_POINTER)
    {
        kernel_error("TEST_SW_INT 4\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 4\n");
    }

    /* TEST REMOVE WHEN NOT REGISTERED */
    if((err = kernel_interrupt_remove_int_handler(MIN_INTERRUPT_LINE))
     != OS_ERR_INTERRUPT_NOT_REGISTERED)
    {
        kernel_error("TEST_SW_INT 5\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 5\n");
    }

    /* TEST REGISTER WHEN ALREADY REGISTERED */
    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE, incrementer_handler))
     != OS_NO_ERR)
    {
        kernel_error("TEST_SW_INT 6\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 6\n");
    }

    if((err = kernel_interrupt_register_int_handler(MIN_INTERRUPT_LINE, incrementer_handler))
     != OS_ERR_INTERRUPT_ALREADY_REGISTERED)
    {
        kernel_error("TEST_SW_INT 7\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 7\n");
    }

    /* INIT THINGS */
    if((err = kernel_interrupt_remove_int_handler(MIN_INTERRUPT_LINE)) != OS_NO_ERR)
    {
        kernel_error("TEST_SW_INT 8\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 8\n");
    }

    cnt_val = 0;
    counter = 0;

    /* REGISTER FROM MIN TO MAX INC AND SEND INT, TEST COUNTER */
    for(i = MIN_INTERRUPT_LINE; i <= MAX_INTERRUPT_LINE; ++i)
    {
        if(i == PANIC_INT_LINE ||
           i == PIC_SPURIOUS_IRQ_MASTER + INT_PIC_IRQ_OFFSET ||
           i == PIC_SPURIOUS_IRQ_SLAVE + INT_PIC_IRQ_OFFSET ||
           i == 0xFF)
        {
            continue;
        }
        err = kernel_interrupt_register_int_handler(i, incrementer_handler);
        if(err != OS_NO_ERR)
        {
            kernel_error("TEST_SW_INT 9 [%d]\n", err);
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] TEST_SW_INT 9\n");
        }
        cnt_val += i;
    }

    kernel_interrupt_restore(1);

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 0));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 1));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 2));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 3));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 4));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 5));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 6));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 7));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 8));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 9));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 11));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 12));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 13));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 14));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 15));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 16));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 17));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 18));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 19));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 20));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 21));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 22));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 24));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 25));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 26));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 27));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 28));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 29));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 30));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 32));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 33));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 34));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 35));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 36));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 37));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 38));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 39));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 40));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 41));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 42));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 43));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 44));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 45));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 46));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 47));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 48));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 49));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 50));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 51));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 52));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 53));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 54));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 55));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 56));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 57));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 58));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 59));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 60));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 61));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 62));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 63));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 64));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 65));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 66));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 67));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 68));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 69));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 70));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 71));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 72));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 73));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 74));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 75));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 76));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 77));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 78));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 79));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 80));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 81));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 82));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 83));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 84));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 85));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 86));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 87));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 88));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 89));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 90));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 91));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 92));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 93));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 94));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 95));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 96));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 97));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 98));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 99));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 100));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 101));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 102));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 103));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 104));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 105));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 106));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 107));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 108));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 109));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 110));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 111));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 112));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 113));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 114));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 115));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 116));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 117));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 118));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 119));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 120));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 121));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 122));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 123));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 124));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 125));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 126));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 127));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 128));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 129));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 130));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 131));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 132));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 133));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 134));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 135));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 136));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 137));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 138));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 139));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 140));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 141));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 142));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 143));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 144));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 145));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 146));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 147));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 148));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 149));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 150));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 151));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 152));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 153));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 154));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 155));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 156));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 157));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 158));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 159));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 160));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 161));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 162));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 163));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 164));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 165));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 166));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 167));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 168));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 169));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 170));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 171));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 172));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 173));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 174));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 175));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 176));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 177));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 178));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 179));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 180));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 181));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 182));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 183));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 184));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 185));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 186));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 187));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 188));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 189));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 190));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 191));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 192));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 193));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 194));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 195));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 196));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 197));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 198));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 199));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 200));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 201));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 202));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 203));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 204));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 205));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 206));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 207));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 208));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 209));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 210));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 211));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 212));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 213));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 214));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 215));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 216));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 217));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 218));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 219));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 220));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 221));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 222));

    int_state = kernel_interrupt_disable();

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT 10\n");
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 10\n");
    }

    for(i = MIN_INTERRUPT_LINE; i <= MAX_INTERRUPT_LINE; ++i)
    {
        if(i == PANIC_INT_LINE ||
           i == PIC_SPURIOUS_IRQ_MASTER + INT_PIC_IRQ_OFFSET ||
           i == PIC_SPURIOUS_IRQ_SLAVE + INT_PIC_IRQ_OFFSET ||
           i == 0xFF)
        {
            continue;
        }
        if((err = kernel_interrupt_remove_int_handler(i)) != OS_NO_ERR)
        {
            kernel_error("TEST_SW_INT 11\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] TEST_SW_INT 11\n");
        }
    }

    /* REGISTER FROM MIN TO MAX DEC AND SEND INT, TEST COUNTER */
    for(i = MIN_INTERRUPT_LINE; i <= MAX_INTERRUPT_LINE; ++i)
    {
        if(i == PANIC_INT_LINE ||
           i == PIC_SPURIOUS_IRQ_MASTER + INT_PIC_IRQ_OFFSET ||
           i == PIC_SPURIOUS_IRQ_SLAVE + INT_PIC_IRQ_OFFSET ||
           i == 0xFF)
        {
            continue;
        }
        if((err = kernel_interrupt_register_int_handler(i, decrementer_handler)) != OS_NO_ERR)
        {
            kernel_error("TEST_SW_INT 12\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] TEST_SW_INT 12\n");
        }
        cnt_val -= i;
    }

    kernel_interrupt_restore(int_state);

    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 0));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 1));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 2));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 3));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 4));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 5));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 6));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 7));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 8));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 9));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 11));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 12));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 13));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 14));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 15));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 16));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 17));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 18));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 19));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 20));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 21));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 22));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 24));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 25));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 26));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 27));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 28));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 29));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 30));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 32));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 33));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 34));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 35));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 36));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 37));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 38));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 39));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 40));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 41));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 42));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 43));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 44));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 45));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 46));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 47));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 48));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 49));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 50));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 51));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 52));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 53));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 54));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 55));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 56));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 57));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 58));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 59));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 60));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 61));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 62));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 63));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 64));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 65));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 66));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 67));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 68));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 69));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 70));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 71));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 72));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 73));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 74));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 75));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 76));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 77));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 78));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 79));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 80));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 81));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 82));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 83));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 84));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 85));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 86));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 87));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 88));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 89));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 90));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 91));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 92));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 93));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 94));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 95));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 96));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 97));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 98));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 99));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 100));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 101));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 102));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 103));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 104));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 105));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 106));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 107));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 108));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 109));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 110));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 111));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 112));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 113));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 114));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 115));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 116));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 117));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 118));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 119));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 120));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 121));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 122));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 123));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 124));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 125));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 126));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 127));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 128));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 129));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 130));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 131));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 132));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 133));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 134));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 135));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 136));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 137));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 138));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 139));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 140));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 141));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 142));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 143));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 144));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 145));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 146));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 147));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 148));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 149));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 150));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 151));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 152));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 153));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 154));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 155));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 156));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 157));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 158));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 159));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 160));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 161));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 162));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 163));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 164));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 165));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 166));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 167));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 168));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 169));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 170));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 171));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 172));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 173));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 174));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 175));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 176));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 177));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 178));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 179));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 180));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 181));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 182));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 183));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 184));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 185));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 186));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 187));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 188));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 189));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 190));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 191));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 192));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 193));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 194));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 195));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 196));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 197));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 198));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 199));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 200));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 201));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 202));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 203));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 204));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 205));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 206));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 207));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 208));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 209));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 210));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 211));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 212));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 213));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 214));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 215));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 216));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 217));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 218));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 219));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 220));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 221));
    __asm__ __volatile__("int %0" :: "i" (MIN_INTERRUPT_LINE + 222));

    int_state = kernel_interrupt_disable();

    if(cnt_val != counter)
    {
        kernel_error("TEST_SW_INT 13 %d != %d\n", cnt_val, counter);
        kill_qemu();
    }
    else
    {
        kernel_printf("[TESTMODE] TEST_SW_INT 13\n");
    }

    for(i = MIN_INTERRUPT_LINE; i <= MAX_INTERRUPT_LINE; ++i)
    {
        if(i == PANIC_INT_LINE ||
           i == PIC_SPURIOUS_IRQ_MASTER + INT_PIC_IRQ_OFFSET ||
           i == PIC_SPURIOUS_IRQ_SLAVE + INT_PIC_IRQ_OFFSET ||
           i == 0xFF)
        {
            continue;
        }
        if((err = kernel_interrupt_remove_int_handler(i)) != OS_NO_ERR)
        {
            kernel_error("TEST_SW_INT 14\n");
            kill_qemu();
        }
        else
        {
            kernel_printf("[TESTMODE] TEST_SW_INT 14\n");
        }
    }

    kernel_printf("[TESTMODE] Software interrupt tests passed\n");
}

void interrupt_test(void)
{
    kernel_printf("[TESTMODE] Software interrupt\n");
    test_sw_interupts();


    kernel_printf("[TESTMODE] Software interrupt lock\n");
    test_sw_interupts_lock();

    kernel_printf("[TESTMODE] PASSED\n");

    /* Kill QEMU */
    kill_qemu();
}
#else
void interrupt_test(void)
{
}
#endif