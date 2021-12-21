/*******************************************************************************
 * @file kheap.c
 *
 * @see kheap.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
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

#include <stdint.h>        /* Generic int types */
#include <stdlib.h>        /* atoi */
#include <string.h>        /* memset */
#include <kernel_output.h> /* Kernel output manager */
#include <critical.h>      /* Critical section manager */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#include <test_bank.h>

/* Header file */
#include <kheap.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Kernel's heap allocator list node. */
struct list
{
    /** @brief Next node of the list. */
    struct list* next;
    /** @brief Previous node of the list. */
    struct list* prev;
};

/**
 * @brief Defines list_t type as a shorcut for struct list.
 */
typedef struct list list_t;

/** @brief Kernel's heap allocator memory chunk representation. */
struct mem_chunk
{
    /** @brief Memory chunk list. */
    list_t all;

    /** @brief Used flag. */
    int8_t used;
    union

    /** @brief If used, the union contains the chunk's data, else a list of free
     * mem.
     */
    {
	       uint8_t* data;
	       list_t   free;
    };
};

/**
 * @brief Defines mem_chunk_t type as a shorcut for struct mem_chunk.
 */
typedef struct mem_chunk mem_chunk_t;

/** @brief Kernel's heap allocator settings. */
enum heap_enum
{
    /** @brief Num size. */
    NUM_SIZES   = 32,

    /** @brief Memory chunk alignement. */
    ALIGN       = 4,

    /** @brief Chink minimal size. */
    MIN_SIZE    = sizeof(list_t),

    /** @brief Header size. */
    HEADER_SIZE = __builtin_offsetof(mem_chunk_t, data),
};

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Heap position in memory */
/** @brief Start address of the kernel's heap. */
extern uint8_t _KERNEL_HEAP_BASE;
/** @brief End address of the kernel's heap. */
extern uint8_t _KERNEL_HEAP_SIZE;

/** @brief Kernel's heap initialization state. */
static bool_t init = FALSE;

/* Heap data */
/** @brief Kernel's heap free memory chunks. */
static mem_chunk_t* free_chunk[NUM_SIZES] = { NULL };
/** @brief Kernel's heap first memory chunk. */
static mem_chunk_t* first_chunk;
/** @brief Kernel's heap last memory chunk. */
static mem_chunk_t* last_chunk;

/** @brief Quantity of free memory in the kernel's heap. */
static uint32_t mem_free;
/** @brief Quantity of initial free memory in the kernel's heap. */
static uint32_t kheap_init_free;
/** @brief Quantity of memory used to store meta data in the kernel's heap. */
static uint32_t mem_meta;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATION
 ******************************************************************************/

inline static void list_init(list_t* node);

inline static void insert_before(list_t* current, list_t* new);

inline static void insert_after(list_t* current, list_t* new);

inline static void remove(list_t* node);

inline static void push(list_t** list, list_t* node);

inline static list_t* pop(list_t** list);

inline static void remove_from(list_t** list, list_t* node);

inline static void memory_chunk_init(mem_chunk_t* chunk);

inline static uint32_t memory_chunk_size(const mem_chunk_t* chunk);

/**
 * @brief Returns the slot of a memory chunk for the desired size.
 *
 * @details Returns the slot of a memory chunk for the desired size.
 *
 * @param[in] size The size of the chunk to get the slot of.
 *
 * @return The slot of a memory chunk for the desired size.
 */
inline static int32_t memory_chunk_slot(uint32_t size);

/**
 * @brief Removes a memory chunk in the free memory chunks list.
 *
 * @details Removes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be removed from the list.
 */
inline static void remove_free(mem_chunk_t* chunk);

/**
 * @brief Pushes a memory chunk in the free memory chunks list.
 *
 * @details Pushes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be placed in the list.
 */
inline static void push_free(mem_chunk_t *chunk);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define CONTAINER(C, l, v) ((C*)(((char*)v) - (uintptr_t)&(((C*)0)->l)))

#define LIST_INIT(v, l) list_init(&v->l)

#define LIST_REMOVE_FROM(h, d, l)                        \
{                                                        \
    __typeof__(**h) **h_ = h, *d_ = d;                    \
      list_t* head = &(*h_)->l;                            \
      remove_from(&head, &d_->l);                            \
      if (head == NULL)                                   \
    {                                                    \
          *h_ = NULL;                                        \
      }                                                   \
    else                                                \
    {                                                    \
        *h_ = CONTAINER(__typeof__(**h), l, head);        \
    }                                                    \
}

#define LIST_PUSH(h, v, l)                      \
{                                              \
      __typeof__(*v) **h_ = h, *v_ = v;          \
      list_t* head = &(*h_)->l;                  \
    if (*h_ == NULL)                          \
    {                                         \
         head = NULL;                             \
    }                                         \
      push(&head, &v_->l);                      \
      *h_ = CONTAINER(__typeof__(*v), l, head); \
}

#define LIST_POP(h, l)                               \
__extension__                                      \
({                                                   \
    __typeof__(**h) **h_ = h;                       \
      list_t* head = &(*h_)->l;                       \
      list_t* res = pop(&head);                       \
      if (head == NULL)                              \
    {                                               \
           *h_ = NULL;                               \
      }                                              \
    else                                           \
    {                                               \
          *h_ = CONTAINER(__typeof__(**h), l, head); \
      }                                               \
      CONTAINER(__typeof__(**h), l, res);               \
})

#define LIST_ITERATOR_BEGIN(h, l, it)                                    \
{                                                                        \
    __typeof__(*h) *h_ = h;                                                \
      list_t* last_##it = h_->l.prev, *iter_##it = &h_->l, *next_##it;    \
    do                                                                  \
    {                                                                    \
           if (iter_##it == last_##it)                                    \
         {                                                                \
             next_##it = NULL;                                            \
           }                                                              \
         else                                                           \
         {                                                                \
             next_##it = iter_##it->next;                                \
         }                                                                \
         __typeof__(*h)* it = CONTAINER(__typeof__(*h), l, iter_##it);  \

#define LIST_ITERATOR_END(it)                        \
    }while((iter_##it = next_##it));                \
}

#define LIST_ITERATOR_REMOVE_FROM(h, it, l) LIST_REMOVE_FROM(h, iter_##it, l)

inline static void list_init(list_t* node)
{
    node->next = node;
    node->prev = node;
}

inline static void insert_before(list_t* current, list_t* new)
{
    list_t* current_prev = current->prev;
    list_t* new_prev = new->prev;

    current_prev->next = new;
    new->prev = current_prev;
    new_prev->next = current;
    current->prev = new_prev;
}

inline static void insert_after(list_t* current, list_t* new)
{
    list_t *current_next = current->next;
    list_t *new_prev = new->prev;

    current->next = new;
    new->prev = current;
    new_prev->next = current_next;
    current_next->prev = new_prev;
}

inline static void remove(list_t* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node;
    node->prev = node;
}

inline static void push(list_t** list, list_t* node)
{

    if (*list != NULL)
    {
           insert_before(*list, node);
    }

    *list = node;
}

inline static list_t* pop(list_t** list)
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

inline static void remove_from(list_t** list, list_t* node)
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

inline static void memory_chunk_init(mem_chunk_t* chunk)
{
    LIST_INIT(chunk, all);
    chunk->used = 0;
    LIST_INIT(chunk, free);
}

inline static uint32_t memory_chunk_size(const mem_chunk_t* chunk)
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
inline static int32_t memory_chunk_slot(uint32_t size)
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
inline static void remove_free(mem_chunk_t* chunk)
{
    uint32_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);

    LIST_REMOVE_FROM(&free_chunk[n], chunk, free);
    mem_free -= len;
}

/**
 * @brief Pushes a memory chunk in the free memory chunks list.
 *
 * @details Pushes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be placed in the list.
 */
inline static void push_free(mem_chunk_t *chunk)
{
    uint32_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);

    LIST_PUSH(&free_chunk[n], chunk, free);
    mem_free += len;
}

void kheap_init(void)
{
    mem_chunk_t* second;
    uint32_t len;
    int32_t  n;

    void* mem = &_KERNEL_HEAP_BASE;
    uint32_t size = (uint32_t)&_KERNEL_HEAP_SIZE;
    int8_t* mem_start = (int8_t*)(((uintptr_t)mem + ALIGN - 1) & (~(ALIGN - 1)));
    int8_t* mem_end = (int8_t*)(((uintptr_t)mem + size) & (~(ALIGN - 1)));

    mem_free = 0;
    kheap_init_free = 0;
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
    mem_free = len;
    kheap_init_free = mem_free;
    mem_meta = sizeof(mem_chunk_t) * 2 + HEADER_SIZE;

    init = TRUE;

    KERNEL_DEBUG(KHEAP_DEBUG_ENABLED,
                 "[KHEAP] Kernel Heap Initialized at 0x%p",
                 mem_start);

    KERNEL_TEST_POINT(kheap_test);
}

void* kmalloc(size_t size)
{
    size_t       n;
    mem_chunk_t* chunk;
    mem_chunk_t* chunk2;
    size_t       size2;
    size_t       len;
    uint32_t     int_state;

    if(init == FALSE || size == 0)
    {
        return NULL;
    }

    ENTER_CRITICAL(int_state);

    size = (size + ALIGN - 1) & (~(ALIGN - 1));

    if (size < MIN_SIZE)
    {
         size = MIN_SIZE;
    }

    n = memory_chunk_slot(size - 1) + 1;

    if (n >= NUM_SIZES)
    {
        EXIT_CRITICAL(int_state);
        return NULL;
    }

    while(free_chunk[n] == 0)
    {
        ++n;
        if (n >= NUM_SIZES)
        {
            EXIT_CRITICAL(int_state);
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
        mem_free += len;
    }

    chunk->used = 1;

    mem_free -= size2;

    KERNEL_DEBUG(KHEAP_DEBUG_ENABLED,
                 "[KHEAP] Kheap allocated 0x%p -> %uB (%uB free, %uB used)",
                 chunk->data,
                 size2 - len - HEADER_SIZE,
                 mem_free,
                 kheap_init_free - mem_free);

    EXIT_CRITICAL(int_state);

    return chunk->data;
}

void kfree(void* ptr)
{
    uint32_t     used;
    mem_chunk_t* chunk;
    mem_chunk_t* next;
    mem_chunk_t* prev;
    uint32_t     int_state;

    if(init == FALSE || ptr == NULL)
    {
        return;
    }

    ENTER_CRITICAL(int_state);

    chunk = (mem_chunk_t*)((int8_t*)ptr - HEADER_SIZE);
    next = CONTAINER(mem_chunk_t, all, chunk->all.next);
    prev = CONTAINER(mem_chunk_t, all, chunk->all.prev);

    used = memory_chunk_size(chunk);

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

    KERNEL_DEBUG(KHEAP_DEBUG_ENABLED,
                 "[KHEAP] Kheap freed 0x%p -> %uB",
                 ptr,
                 used);

    EXIT_CRITICAL(int_state);
}

uint32_t kheap_get_free(void)
{
    return mem_free;
}