/*******************************************************************************
 * @file cpu_api.h
 *
 * @see cpu.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief CPU management functions
 *
 * @details CPU manipulation functions. Wraps inline assembly calls for
 * ease of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_API_H_
#define __CPU_API_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>     /* Generic int types */
#include <stddef.h>     /* Standard definitions */
#include <ctrl_block.h> /* Thread structures */

#ifdef ARCH_I386
#include <../i386/includes/cpu.h>
#else
#error Unknown CPU architecture
#endif

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the current CPU id.
 *
 * @details The function returns the CPU id on which the call is made.
 *
 * @return The current CPU id is returned on succes and -1 is return in case
 * of error.
 */
int32_t cpu_get_id(void);

/**
 * @brief Returns the current page directory physical address.
 *
 * @details Return the current value stored in CR3. No error can be returned.
 *
 * @return The current CR3 value.
 */
uintptr_t cpu_get_current_pgdir(void);

/**
 * @brief Raises CPU interrupt.
 *
 * @details Raises a software CPU interrupt on the desired line.
 *
 * @param[in] interrupt_line The line on which the interrupt should be raised.
 *
 * @return OS_NO_ERR shoudl be return in case of success.
 * - OS_ERR_UNAUTHORIZED_ACTION Is returned if the interrupt line is not
 * correct.
 */
OS_RETURN_E cpu_raise_interrupt(const uint32_t interrupt_line);

/**
 * @brief Returns the CPU current interrupt state.
 *
 * @details Returns the current CPU eflags interrupt enable value.
 *
 * @return The CPU current interrupt state: 1 if enabled, 0 otherwise.
 */
uint32_t cpu_get_interrupt_state(void);

/**
 * @brief Returns the saved interrupt state.
 *
 * @details Returns the saved interrupt state based on the stack state.
 *
 * @param[in] cpu_state The current CPU state.
 * @param[in] stack_state The current stack state.
 *
 * @return The current savec interrupt state: 1 if enabled, 0 otherwise.
 */
uint32_t cpu_get_saved_interrupt_state(const cpu_state_t* cpu_state,
                                       const stack_state_t* stack_state);

/**
 * @brief Initializes the thread's context.
 *
 * @details Initializes the thread's context by populating the virtual CPU
 * structure of the thread and its stack.
 *
 * @param[in] entry_point The thread's entry point.
 * @param[out] thread The thread to initialize.
 */
void cpu_init_thread_context(void (*entry_point)(void),
                             kernel_thread_t* thread);

/**
 * @brief Saves the current thread CPU context.
 *
 * @details Saves the current CPU context for the thread.
 * Registers (and other data) should be saved here.
 *
 * @param[in] cpu_state The current CPU state.
 * @param[in] stack_stack The current thread's stack state.
 * @param[out] thread The thread structure to save the data to.
 */
void cpu_save_context(const cpu_state_t* cpu_state,
                      const stack_state_t* stack_state,
                      kernel_thread_t* thread);

/**
 * @brief Restores the thread's CPU context.
 *
 * @details Restores the thread's CPU context from the thread storage
 * structure. Registers are updated and the execution flow might be
 * updated.
 *
 * @param[out] cpu_state The current CPU state that will be modified.
 * @param[in] stack_state The current stack state.
 * @param[in] thread The thread structure to read the data from.
 */
void cpu_restore_context(cpu_state_t* cpu_state,
                         const stack_state_t* stack_state,
                         const kernel_thread_t* thread);

/**
 * @brief Generates a system call.
 *
 * @details Generates a system call and pass the parameters to the future
 * syscall handler.
 *
 * @param[in] syscall_id The system call function ID to pass to the handler.
 * @param[in] params The pointer to the parameters to pass to the handler.
 */
void cpu_syscall(uint32_t syscall_id, void* params);

/**
 * @brief Retreives system call parameters.
 *
 * @details Retreives system call parameters from the stack of CPU state
 * depending on the architecture.
 *
 * @param[in] cpu_state The cpu state at the moment of the system call.
 * @param[in] stack_state The stack state at the moment of the system call.
 * @param[out] syscall_id The buffer to fill with the system call ID.
 * @param[out] params The buffer to fill with the system call parameters.
 */
void cpu_get_syscall_data(const cpu_state_t* cpu_state,
                          const stack_state_t* stack_state,
                          uint32_t* syscall_id,
                          void** params);

/**
 * @brief Switch the CPU to user mode
 *
 * @details Switches the CPU to user mode (usually reduring the priviledge)
 * level. Its implementation is dependent on the used architecture. Once the
 * call to this function returns, the CPU is in user mode.
 */
void cpu_switch_user_mode(void);

/**
 * @brief Lock a spinlock.
 *
 * @details Lock the spinlock passed in parameters.
 *
 * @param[in, out] lockword The spinlock to use.
 */
void cpu_lock_spinlock(volatile uint32_t* lockword);

/**
 * @brief Compare and swap primitive.
 *
 * @details The compare and swap primitive allows to compare a value stored in
 * the memory and replace it with a new value is the old value equals a certain
 * value.
 *
 * @param[in, out] memory The memory region to be compared and swaped, on
 * 32bits.
 * @param[in] old_val The old value to compare.
 * @param[in] new_val The new value to compare.
 *
 * @return The value stored in the memory is returned.
 */
int32_t cpu_compare_and_swap(volatile int32_t* memory,
                             const int32_t old_val,
                             const int32_t new_val);

/**
 * @brief Atomically fecth the value in the memory and add a givne value to it.
 *
 * @details This primitive fecthes the value stored at the memory region given
 * as parameter. It adds the value provided in parameters and returns the value
 * before the addition operation.
 *
 * @param[out] memory The memory region to fetch and add.
 * @param[in] val The value to add.
 *
 * @returns The previous value contained in the memory region is returned.
 */
int32_t cpu_fetch_and_add(volatile int32_t* memory, const int32_t val);

/**
 * @brief Atomically stores a value in the memory.
 *
 * @details Atomically stores a value in the memory.
 *
 * @param[out] memory The memory region to modify.
 * @param[in] val The value to store.
 */
void cpu_atomic_store(volatile int32_t* memory, const int32_t val);

#endif /* #ifndef __CPU_API_H_ */

/************************************ EOF *************************************/