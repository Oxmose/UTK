/*******************************************************************************
 * @file kheap.c
 *
 * @see kheap.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 10/01/2018
 *
 * @version 1.0
 *
 * @brief Kernel's heap allocator.
 *
 * @details Kernel's heap allocator. Allow to dynamically allocate and dealocate
 * memory on the kernel's heap.
 *
 * @warning This allocator is not suited to allocate memory for the process, you
 * should only use it for the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/stdint.h>       /* Generic int types */
#include <lib/stdlib.h>       /* atoi */
#include <lib/string.h>       /* memset */
#include <io/kernel_output.h> /* kernel_success */
#include <sync/critical.h>    /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <memory/kheap.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Kernel's heap initialization state. */
static uint32_t init = 0;

/* Heap position in memory */
/** @brief Start address of the kernel's heap. */
extern uint8_t _kernel_heap_start;
/** @brief End address of the kernel's heap. */
extern uint8_t _kernel_heap_end;

/* Heap data */
/** @brief Kernel's heap free memory chunks. */
static mem_chunk_t* free_chunk[NUM_SIZES] = { NULL };
/** @brief Kernel's heap first memory chunk. */
static mem_chunk_t* first_chunk;
/** @brief Kernel's heap last memory chunk. */
static mem_chunk_t* last_chunk;

/** @brief Quantity of free memory in the kernel's heap. */
static uint32_t mem_free;
/** @brief Quantity of used memory in the kernel's heap. */
uint32_t kheap_mem_used;
/** @brief Quantity of memory used to store meta data in the kernel's heap. */
static uint32_t mem_meta;

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define CONTAINER(C, l, v) ((C*)(((char*)v) - (uintptr_t)&(((C*)0)->l)))

#define LIST_INIT(v, l) list_init(&v->l)

#define LIST_REMOVE_FROM(h, d, l)					    \
{								                    	\
    __typeof__(**h) **h_ = h, *d_ = d;					\
  	list_t* head = &(*h_)->l;					        \
  	remove_from(&head, &d_->l);					        \
  	if (head == NULL)                                   \
    {						                            \
  	    *h_ = NULL;							            \
  	}                                                   \
    else                                                \
    {							                        \
        *h_ = CONTAINER(__typeof__(**h), l, head);	    \
    }								                    \
}

#define LIST_PUSH(h, v, l)			    	  \
{									          \
  	__typeof__(*v) **h_ = h, *v_ = v;		  \
  	list_t* head = &(*h_)->l;				  \
    if (*h_ == NULL)                          \
    {                                         \
         head = NULL;	               	      \
    }                                         \
  	push(&head, &v_->l);					  \
  	*h_ = CONTAINER(__typeof__(*v), l, head); \
}

#define LIST_POP(h, l)						       \
__extension__                                      \
({									               \
    __typeof__(**h) **h_ = h;					   \
  	list_t* head = &(*h_)->l;					   \
  	list_t* res = pop(&head);					   \
  	if (head == NULL)                              \
    {						                       \
  	     *h_ = NULL;					           \
  	}                                              \
    else                                           \
    {							                   \
  	    *h_ = CONTAINER(__typeof__(**h), l, head); \
  	}							                   \
  	CONTAINER(__typeof__(**h), l, res);			   \
})

#define LIST_ITERATOR_BEGIN(h, l, it)				            	    \
{								                                    	\
    __typeof__(*h) *h_ = h;						                        \
  	list_t* last_##it = h_->l.prev, *iter_##it = &h_->l, *next_##it;	\
    do                                                                  \
    {								                                    \
  	     if (iter_##it == last_##it)                                    \
         {				                                                \
             next_##it = NULL;					                        \
  	     }                                                              \
         else                                                           \
         {							                                    \
             next_##it = iter_##it->next;				                \
         }								                                \
         __typeof__(*h)* it = CONTAINER(__typeof__(*h), l, iter_##it);  \

#define LIST_ITERATOR_END(it)						\
    } while((iter_##it = next_##it));				\
}

#define LIST_ITERATOR_REMOVE_FROM(h, it, l) LIST_REMOVE_FROM(h, iter_##it, l)

__inline__ static void list_init(list_t* node)
{
    node->next = node;
    node->prev = node;
}

__inline__ static void insert_before(list_t* current, list_t* new)
{
    list_t* current_prev = current->prev;
    list_t* new_prev = new->prev;

    current_prev->next = new;
    new->prev = current_prev;
    new_prev->next = current;
    current->prev = new_prev;
}

__inline__ static void insert_after(list_t* current, list_t* new)
{
    list_t *current_next = current->next;
    list_t *new_prev = new->prev;

    current->next = new;
    new->prev = current;
    new_prev->next = current_next;
    current_next->prev = new_prev;
}

__inline__ static void remove(list_t* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node;
    node->prev = node;
}

__inline__ static void push(list_t** list, list_t* node)
{

    if (*list != NULL)
    {
	       insert_before(*list, node);
    }

    *list = node;
}

__inline__ static list_t* pop(list_t** list)
{

    list_t* top = *list;
    list_t* next_top = top->next;

    remove(top);

    if (top == next_top)
    {
	       *list = NULL;
    }
    else
    {
	       *list = next_top;
    }

    return top;
}

__inline__ static void remove_from(list_t** list, list_t* node)
{
    if (*list == node)
    {
	       pop(list);
    }
    else
    {
	       remove(node);
    }
}

static void memory_chunk_init(mem_chunk_t* chunk)
{
    LIST_INIT(chunk, all);
    chunk->used = 0;
    LIST_INIT(chunk, free);
}

static uint32_t memory_chunk_size(const mem_chunk_t* chunk)
{

    int8_t* end = (int8_t*)(chunk->all.next);
    int8_t* start = (int8_t*)(&chunk->all);
    return (end - start) - HEADER_SIZE;
}

/**
 * @brief Returns the slot of a memory chunk for the desired size.
 *
 * @details Returns the slot of a memory chunk for the desired size.
 *
 * @param[in] size The size of the chunk to get the slot of.
 *
 * @return The slot of a memory chunk for the desired size.
 */
static int32_t memory_chunk_slot(uint32_t size)
{
    int32_t n = -1;
    while(size > 0)
    {
	    ++n;
    	size /= 2;
    }
    return n;
}

/**
 * @brief Removes a memory chunk in the free memory chunks list.
 *
 * @details Removes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be removed from the list.
 */
static void remove_free(mem_chunk_t* chunk)
{
    uint32_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);

    LIST_REMOVE_FROM(&free_chunk[n], chunk, free);
    mem_free -= len - HEADER_SIZE;
}

/**
 * @brief Pushes a memory chunk in the free memory chunks list.
 *
 * @details Pushes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be placed in the list.
 */
static void push_free(mem_chunk_t *chunk)
{
    uint32_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);

    LIST_PUSH(&free_chunk[n], chunk, free);
    mem_free += len - HEADER_SIZE;
}

OS_RETURN_E kheap_init(void)
{
    mem_chunk_t* second;
    uint32_t len;
    int32_t  n;

    void* mem = &_kernel_heap_start;
    uint32_t size = &_kernel_heap_end - &_kernel_heap_start;
    int8_t* mem_start = (int8_t*)(((uintptr_t)mem + ALIGN - 1) & (~(ALIGN - 1)));
    int8_t* mem_end = (int8_t*)(((uintptr_t)mem + size) & (~(ALIGN - 1)));

    mem_free = 0;
    kheap_mem_used = 0;
    mem_meta = 0;
    first_chunk = NULL;
    last_chunk = NULL;

    first_chunk = (mem_chunk_t*)mem_start;
    second = first_chunk + 1;

    last_chunk = ((mem_chunk_t*)mem_end) - 1;

    memory_chunk_init(first_chunk);
    memory_chunk_init(second);

    memory_chunk_init(last_chunk);
    insert_after(&first_chunk->all, &second->all);
    insert_after(&second->all, &last_chunk->all);

    first_chunk->used = 1;
    last_chunk->used = 1;

    len = memory_chunk_size(second);
    n   = memory_chunk_slot(len);

    LIST_PUSH(&free_chunk[n], second, free);
    mem_free = len - HEADER_SIZE;
    mem_meta = sizeof(mem_chunk_t) * 2 + HEADER_SIZE;

    init = 1;

#if KHEAP_KERNEL_DEBUG == 1
    kernel_serial_debug("Kernel Heap Initialized at 0x%p\n", mem_start);
#endif 

#if TEST_MODE_ENABLED
    kheap_test();
#endif

    return OS_NO_ERR;
}

void* kmalloc(size_t size)
{
    size_t       n;
    mem_chunk_t* chunk;
    mem_chunk_t* chunk2;
    size_t       size2;
    size_t       len;
    uint32_t     int_state;

    if(init == 0)
    {
        return NULL;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    size = (size + ALIGN - 1) & (~(ALIGN - 1));

	if (size < MIN_SIZE)
    {
         size = MIN_SIZE;
    }

	n = memory_chunk_slot(size - 1) + 1;

	if (n >= NUM_SIZES)
    {

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return NULL;
    }

	while(free_chunk[n] == 0)
    {
		++n;
		if (n >= NUM_SIZES)
        {

#if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(int_state, &lock);
#else
            EXIT_CRITICAL(int_state);
#endif

            return NULL;
        }
    }

	chunk = LIST_POP(&free_chunk[n], free);
    size2 = memory_chunk_size(chunk);
    len = 0;

	if (size + sizeof(mem_chunk_t) <= size2)
    {
		chunk2 = (mem_chunk_t*)(((int8_t*)chunk) + HEADER_SIZE + size);

		memory_chunk_init(chunk2);

		insert_after(&chunk->all, &chunk2->all);

		len = memory_chunk_size(chunk2);
		n = memory_chunk_slot(len);

		LIST_PUSH(&free_chunk[n], chunk2, free);

		mem_meta += HEADER_SIZE;
		mem_free += len - HEADER_SIZE;
    }

	chunk->used = 1;

    mem_free -= size2;
    kheap_mem_used += size2 - len - HEADER_SIZE;

#if KHEAP_KERNEL_DEBUG == 1
    kernel_serial_debug("Kheap allocated 0x%p -> %uB (%uB free, %uB used)\n",
                        chunk->data,
                        size2 - len - HEADER_SIZE,
                        mem_free, kheap_mem_used);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return chunk->data;
}

void kfree(void* ptr)
{
    uint32_t     used;
    mem_chunk_t* chunk;
    mem_chunk_t* next;
    mem_chunk_t* prev;
    uint32_t     int_state;

    if(init == 0 || ptr == NULL)
    {
        return;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    chunk = (mem_chunk_t*)((int8_t*)ptr - HEADER_SIZE);
    next = CONTAINER(mem_chunk_t, all, chunk->all.next);
    prev = CONTAINER(mem_chunk_t, all, chunk->all.prev);

    used = memory_chunk_size(chunk);
    kheap_mem_used -= used;

    if (next->used == 0)
    {
		remove_free(next);
		remove(&next->all);

		mem_meta -= HEADER_SIZE;
		mem_free += HEADER_SIZE;
    }

    if (prev->used == 0)
    {
		remove_free(prev);
		remove(&chunk->all);

		push_free(prev);
		mem_meta -= HEADER_SIZE;
		mem_free += HEADER_SIZE;
    }
    else
    {
		chunk->used = 0;
		LIST_INIT(chunk, free);
		push_free(chunk);
    }

#if KHEAP_KERNEL_DEBUG == 1
    kernel_serial_debug("Kheap freed 0x%p -> %uB\n", ptr, used);
#endif

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &lock);
#else
    EXIT_CRITICAL(int_state);
#endif

}