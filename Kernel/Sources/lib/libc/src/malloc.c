/*******************************************************************************
 * @file malloc.c
 *
 * @see stdlib.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 28/02/2021
 *
 * @version 1.0
 *
 * @brief User's heap allocator.
 *
 * @details User's heap allocator. Allow to dynamically allocate and dealocate
 * memory on the User's heap.
 *
 * @warning This allocator is not suited to allocate memory for the process, you
 * should only use it for the User.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* Included headers */
#include <stdint.h>        /* Generic int types */
#include <stdlib.h>        /* atoi */
#include <string.h>        /* memset */
#include <kernel_output.h> /* Kernel output manager */
#include <mutex.h>         /* Mutex API */
#include <syscall.h>       /* Memory management syscalls */
#include <memmgt.h>        /* Memory management API */
#include <panic.h>         /* Panic API */

/* Configuration files */
#include <config.h>
#include <test_bank.h>

/* Header file */
#include <stdlib.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Num size. */
#define NUM_SIZES 32

/** @brief Memory chunk alignement. */
#define ALIGN 4

/** @brief Chink minimal size. */
#define MIN_SIZE sizeof(list_t)

    /** @brief Header size. */
#define HEADER_SIZE __builtin_offsetof(mem_chunk_t, data)

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Kernel's heap allocator list node. */
typedef struct list
{
    /** @brief Next node of the list. */
    struct list* next;
    /** @brief Previous node of the list. */
    struct list* prev;
} list_t;

/** @brief Kernel's heap allocator memory chunk representation. */
typedef struct
{
    /** @brief Memory chunk list. */
    list_t all;

    /** @brief Used flag. */
    bool_t used;

    /** @brief If used, the union contains the chunk's data, else a list of free
     * mem.
     */
    union {
           uint8_t* data;
           list_t   free;
    };
} mem_chunk_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define CONTAINER(C, l, v) ((C*)(((char*)v) - (uintptr_t)&(((C*)0)->l)))

#define LIST_INIT(v, l) list_init(&v->l)

#define LIST_REMOVE_FROM(h, d, l)                           \
{                                                           \
    __typeof__(**h) **h_ = h, *d_ = d;                      \
    list_t* head = &(*h_)->l;                               \
    remove_from(&head, &d_->l);                             \
    if (head == NULL)                                       \
    {                                                       \
          *h_ = NULL;                                       \
    }                                                       \
    else                                                    \
    {                                                       \
        *h_ = CONTAINER(__typeof__(**h), l, head);          \
    }                                                       \
}

#define LIST_PUSH(h, v, l)                          \
{                                                   \
    __typeof__(*v) **h_ = h, *v_ = v;               \
    list_t* head = &(*h_)->l;                       \
    if (*h_ == NULL)                                \
    {                                               \
        head = NULL;                                \
    }                                               \
    push(&head, &v_->l);                            \
    *h_ = CONTAINER(__typeof__(*v), l, head);       \
}

#define LIST_POP(h, l)                                  \
__extension__                                           \
({                                                      \
    __typeof__(**h) **h_ = h;                           \
    list_t* head = &(*h_)->l;                           \
    list_t* res = pop(&head);                           \
    if (head == NULL)                                   \
    {                                                   \
           *h_ = NULL;                                  \
    }                                                   \
    else                                                \
    {                                                   \
          *h_ = CONTAINER(__typeof__(**h), l, head);    \
    }                                                   \
    CONTAINER(__typeof__(**h), l, res);                 \
})

#define LIST_ITERATOR_BEGIN(h, l, it)                                       \
{                                                                           \
    __typeof__(*h) *h_ = h;                                                 \
      list_t* last_##it = h_->l.prev, *iter_##it = &h_->l, *next_##it;      \
    do                                                                      \
    {                                                                       \
           if (iter_##it == last_##it)                                      \
         {                                                                  \
             next_##it = NULL;                                              \
           }                                                                \
         else                                                               \
         {                                                                  \
             next_##it = iter_##it->next;                                   \
         }                                                                  \
         __typeof__(*h)* it = CONTAINER(__typeof__(*h), l, iter_##it);      \

#define LIST_ITERATOR_END(it)                       \
    }while((iter_##it = next_##it));                \
}

#define LIST_ITERATOR_REMOVE_FROM(h, it, l) LIST_REMOVE_FROM(h, iter_##it, l)

/**
 * @brief Assert macro used by the malloc manager to ensure correctness of
 * execution.
 *
 * @details Assert macro used by the malloc manager to ensure correctness of
 * execution. Due to the critical nature of the malloc manager, any error
 * generates a kernel panic.
 *
 * @param[in] COND The condition that should be true.
 * @param[in] MSG The message to display in case of kernel panic.
 * @param[in] ERROR The error code to use in case of kernel panic.
 */
#define MALLOC_ASSERT(COND, MSG, ERROR) {                     \
    if((COND) == FALSE)                                       \
    {                                                         \
        PANIC(ERROR, "MALLOC", MSG, TRUE);                    \
    }                                                         \
}

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/** @brief Heap initialization state. */
static bool_t init = FALSE;

/* Heap data */
/** @brief Heap free memory chunks. */
static mem_chunk_t* free_chunk[NUM_SIZES] = { NULL };
/** @brief Heap first memory chunk. */
static mem_chunk_t* first_chunk;
/** @brief Heap last memory chunk. */
static mem_chunk_t* last_chunk;

/** @brief Quantity of free memory in the Heap. */
static uint32_t mem_free;
/** @brief Quantity of used memory in the Heap. */
static uint32_t kheap_mem_init;
/** @brief Quantity of memory used to store meta data in the Heap. */
static uint32_t mem_meta;

/** @brief Malloc mutex */
static mutex_t lock;


/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Initializes the memory list.
 *
 * @details Initializes the memory list with the basic node value.
 *
 * @param[out] node The list's node to initialize.
 */
inline static void list_init(list_t* node);

/**
 * @brief Inserts a node before the current node in the list.
 *
 * @details Inserts a node before the current node in the list.
 *
 * @param[in,out] current The current node.
 * @param[in,out] new The new node to insert before the current node.
 */
inline static void insert_before(list_t* current, list_t* new);

/**
 * @brief Inserts a node after the current node in the list.
 *
 * @param[in,out] current The current node.
 * @param[in,out] new The new node to insert after the current node.
 */
inline static void insert_after(list_t* current, list_t* new);

/**
 * @brief Removes a node from the list.
 *
 * @details Removes a node from the list.
 *
 * @param[out] node The node to remove from the list.
 */
inline static void remove(list_t* node);

/**
 * @brief Pushes a node at the end of the list.
 *
 * @details Pushes a node at the end of the list.
 *
 * @param[out] list The list to be pushed.
 * @param[in] node The node to push to the list.
 */
inline static void push(list_t** list, list_t* node);

/**
 * @brief Pops a node from the list.
 *
 * @details Pops a node from the list and returns it.
 *
 * @param[out] list The list to be poped from.
 *
 * @return The node poped from the list is returned.
 */
inline static list_t* pop(list_t** list);


/**
 * @brief Removes a node from the list.
 *
 * @details Removes a node from the list.
 *
 * @param[out] list The list to remove the node from.
 * @param[out] node The node to remove from the list.
 */
inline static void remove_from(list_t** list, list_t* node);


/**
 * @brief Initializes a memory chunk structure.
 *
 * @details Initializes a memory chunk structure.
 *
 * @param[out] chunk The chunk structure to initialize.
 */
inline static void memory_chunk_init(mem_chunk_t* chunk);

/**
 * @brief Returns the size of a memory chunk.
 *
 * @param chunk The chunk to get the size of.
 *
 * @return The size of the memory chunk is returned.
 */
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

/**
 * @brief Pushes a memory chunk in the free memory chunks list.
 *
 * @details Pushes a memory chunk in the free memory chunks list.
 *
 * @param[in, out] chunk The chunk to be placed in the list.
 */
inline static void push_free(mem_chunk_t *chunk);

/**
 * @brief Initializes the user heap.
 *
 * @details Initializes the user heap. Mmeory is requested to the kernel and
 * mapped for the process.
 */
static void user_heap_init(void);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

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
    chunk->used = FALSE;
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

/**
 * @brief Initializes the process's heap.
 *
 * @details Setups process heap management. It will also allign process heap
 * start and initialize the basic heap parameters such as its size.
 */
static void user_heap_init(void)
{
    mem_chunk_t* second;
    uint32_t     len;
    int32_t      n;
    void*        mem;
    uint32_t     size;
    int8_t*      mem_start;
    int8_t*      mem_end;
    OS_RETURN_E  err;

    memmgt_page_alloc_param_t alloc_param;

    /* Require memory for the heap */
    alloc_param.page_count = PROCESS_HEAP_SIZE / KERNEL_FRAME_SIZE;
    if(PROCESS_HEAP_SIZE % KERNEL_FRAME_SIZE != 0)
    {
        ++alloc_param.page_count;
    }
    syscall_do(SYSCALL_PAGE_ALLOC, &alloc_param);

    if(alloc_param.error != OS_NO_ERR)
    {
        kernel_error("Could not initialize user heap %d\n", alloc_param.error);
        return;
    }

    mem       = alloc_param.start_addr;
    size      = PROCESS_HEAP_SIZE;
    mem_start = (int8_t*)(((uintptr_t)mem + ALIGN - 1) & (~(ALIGN - 1)));
    mem_end   = (int8_t*)(((uintptr_t)mem + size) & (~(ALIGN - 1)));

    mem_free       = 0;
    kheap_mem_init = 0;
    mem_meta       = 0;
    first_chunk    = NULL;
    last_chunk     = NULL;

    first_chunk = (mem_chunk_t*)mem_start;
    second      = first_chunk + 1;

    last_chunk = ((mem_chunk_t*)mem_end) - 1;

    memory_chunk_init(first_chunk);
    memory_chunk_init(second);

    memory_chunk_init(last_chunk);
    insert_after(&first_chunk->all, &second->all);
    insert_after(&second->all, &last_chunk->all);

    first_chunk->used = TRUE;
    last_chunk->used  = TRUE;

    len = memory_chunk_size(second);
    n   = memory_chunk_slot(len);

    LIST_PUSH(&free_chunk[n], second, free);
    mem_free = len;
    kheap_mem_init = mem_free;
    mem_meta = sizeof(mem_chunk_t) * 2 + HEADER_SIZE;

    err = mutex_init(&lock, MUTEX_PRIORITY_ELEVATION_NONE, 0);
    MALLOC_ASSERT(err == OS_NO_ERR,
                  "Could not initialize user heap lock.",
                  err);

    init = TRUE;

    KERNEL_DEBUG(USER_HEAP_DEBUG_ENABLED,
                 "User Heap Initialized at 0x%p",
                 mem_start);
}

void* malloc(size_t size)
{
    size_t       n;
    mem_chunk_t* chunk;
    mem_chunk_t* chunk2;
    size_t       size2;
    size_t       len;
    OS_RETURN_E  err;


    /* TODO, this should be removed as init should be done in crto0 */
    if(init == FALSE)
    {
        user_heap_init();
    }

    if(size == 0)
    {
        return NULL;
    }

    err = mutex_lock(&lock);
    MALLOC_ASSERT(err == OS_NO_ERR,
                  "Could not lock user heap lock.",
                  err);

    size = (size + ALIGN - 1) & (~(ALIGN - 1));

    if (size < MIN_SIZE)
    {
         size = MIN_SIZE;
    }

    n = memory_chunk_slot(size - 1) + 1;

    if (n >= NUM_SIZES)
    {
        err = mutex_unlock(&lock);
        MALLOC_ASSERT(err == OS_NO_ERR,
                      "Could not unlock user heap lock.",
                      err);

        return NULL;
    }

    while(free_chunk[n] == 0)
    {
        ++n;
        if (n >= NUM_SIZES)
        {
            err = mutex_unlock(&lock);
            MALLOC_ASSERT(err == OS_NO_ERR,
                          "Could not unlock user heap lock.",
                          err);

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

    chunk->used = TRUE;

    mem_free -= size2;

    KERNEL_DEBUG(USER_HEAP_DEBUG_ENABLED,
                 "User heap allocated 0x%p -> %uB (%uB free, %uB used)",
                 chunk->data,
                 size2 - len - HEADER_SIZE,
                 mem_free,
                 kheap_mem_init - mem_free);

    err = mutex_unlock(&lock);
    MALLOC_ASSERT(err == OS_NO_ERR,
                  "Could not unlock user heap lock.",
                  err);

    return chunk->data;
}

void free(void* ptr)
{
    uint32_t     used;
    mem_chunk_t* chunk;
    mem_chunk_t* next;
    mem_chunk_t* prev;
    OS_RETURN_E  err;

    if(init == 0 || ptr == NULL)
    {
        return;
    }

    err = mutex_lock(&lock);
    MALLOC_ASSERT(err == OS_NO_ERR,
                  "Could not lock user heap lock.",
                  err);

    chunk = (mem_chunk_t*)((int8_t*)ptr - HEADER_SIZE);
    next = CONTAINER(mem_chunk_t, all, chunk->all.next);
    prev = CONTAINER(mem_chunk_t, all, chunk->all.prev);

    used = memory_chunk_size(chunk);

    if (next->used == FALSE)
    {
        remove_free(next);
        remove(&next->all);

        mem_meta -= HEADER_SIZE;
        mem_free += HEADER_SIZE;
    }

    if (prev->used == FALSE)
    {
        remove_free(prev);
        remove(&chunk->all);

        push_free(prev);
        mem_meta -= HEADER_SIZE;
        mem_free += HEADER_SIZE;
    }
    else
    {
        chunk->used = FALSE;
        LIST_INIT(chunk, free);
        push_free(chunk);
    }

    KERNEL_DEBUG(USER_HEAP_DEBUG_ENABLED,
                 "Heap freed 0x%p -> %uB",
                 ptr,
                 used);

    err = mutex_unlock(&lock);
    MALLOC_ASSERT(err == OS_NO_ERR,
                  "Could not unlock user heap lock.",
                  err);
}

/************************************ EOF *************************************/