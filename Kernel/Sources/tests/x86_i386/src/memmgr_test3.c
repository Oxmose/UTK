
#include <test_bank.h>

#if MEMMGR_TEST3  == 1

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <queue.h>
#include <kernel_output.h>
#include <memmgt.h>

extern queue_t* paging_get_free_frames(void);
extern queue_t* paging_get_free_pages(void);
extern void testmode_paging_add_page(uintptr_t start, uint64_t size);
extern queue_t* testmode_paging_get_area(void);

void memmgr_test3(void)
{
    queue_node_t* cursor;
    queue_t* frames;
    queue_t* pages;
    mem_range_t* range;

    kernel_printf("[TESTMODE] Paging Alloc Tests\n");

    frames = paging_get_free_frames();
    pages = paging_get_free_pages();

    kernel_printf("\n[TESTMODE] Init page, frame list\n");

    cursor = pages->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    cursor = frames->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Frame range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

/************************** TEST MGT ****************************************/
    kernel_printf("\n[TESTMODE] Test management\n");

    testmode_paging_add_page(0x4000, 5LL);
    testmode_paging_add_page(0x13000, 20LL);
    testmode_paging_add_page(0x100000, 1);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kernel_printf("[TESTMODE] ---\n");

    testmode_paging_add_page(0x27000, 5);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kernel_printf("[TESTMODE] ---\n");

    testmode_paging_add_page(0x10000, 3);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kernel_printf("[TESTMODE] ---\n");

    testmode_paging_add_page(0x9000, 6);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kernel_printf("[TESTMODE] ---\n");

    testmode_paging_add_page(0xF000, 1);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kernel_printf("[TESTMODE] ---\n");

    testmode_paging_add_page(0x2C000, 212);

    cursor = testmode_paging_get_area()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

/************************** TEST PAGES ****************************************/
    kernel_printf("\n[TESTMODE] Test pages\n");
    uint32_t* page;
    kernel_printf("[TESTMODE]Silent alloc\n");
    for(uint32_t i = 0; i < 100; ++i)
    {
        alloc_kpages(1);
    }
    for(uint32_t i = 0; i < 30; ++i)
    {
        page = alloc_kpages(1);
        kernel_printf("[TESTMODE]Allocated 0x%08x\n", page);
    }

    kernel_printf("[TESTMODE] ---\n");

    cursor = paging_get_free_pages()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    free_kpages((void*)0xe0380000, 2);

    kernel_printf("[TESTMODE] ---\n");

    cursor = paging_get_free_pages()->head;
    while(cursor)
    {
        range = (mem_range_t*)cursor->data;
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        range->base, range->limit);
        cursor = cursor->next;
    }

    kill_qemu();
}
#else
void memmgr_test3(void)
{
}
#endif