/*******************************************************************************
 * @file stdint.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief Kernel's basic types.
 *
 * @details Define basics int types for the kernel.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#ifndef __LIB_STDINT_H_
#define __LIB_STDINT_H_


/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Defines the TRUE value. */
#define TRUE 1

/** @brief Defines the FALSE value. */
#define FALSE 0

/* Limits of exact-width integer types */
/** @brief int8_t minimal value. */
#define INT8_MIN (-128)
/** @brief int16_t minimal value. */
#define INT16_MIN (-32768)
/** @brief int32_t minimal value. */
#define INT32_MIN (-2147483647 - 1)
/** @brief int64_t minimal value. */
#define INT64_MIN  (-9223372036854775807LL - 1)

/** @brief int8_t maximal value. */
#define INT8_MAX 127
/** @brief int16_t maximal value. */
#define INT16_MAX 32767
/** @brief int32_t maximal value. */
#define INT32_MAX 2147483647
/** @brief int64_t maximal value. */
#define INT64_MAX 9223372036854775807LL

/** @brief uint8_t maximal value. */
#define UINT8_MAX  0xff
/** @brief uint16_t maximal value. */
#define UINT16_MAX 0xffff
/** @brief uint32_t maximal value. */
#define UINT32_MAX 0xffffffff
/** @brief uint64_t maximal value. */
#define UINT64_MAX 0xffffffffffffffffULL

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Defines int8_t type as an exact width 8 bits integer.
 */
typedef signed char        int8_t;
/**
 * @brief Defines uint8_t type as an exact width 8 bits integer.
 */
typedef unsigned char      uint8_t;
/**
 * @brief Defines int16_t type as an exact width 16 bits integer.
 */
typedef short              int16_t;
/**
 * @brief Defines uint16_t type as an exact width 16 bits integer.
 */
typedef unsigned short     uint16_t;
/**
 * @brief Defines int32_t type as an exact width 32 bits integer.
 */
typedef int                int32_t;
/**
 * @brief Defines uint32_t type as an exact width 32 bits integer.
 */
typedef unsigned           uint32_t;
/**
 * @brief Defines int64_t type as an exact width 64 bits integer.
 */
typedef signed long long   int64_t;
/**
 * @brief Defines uint64_t type as an exact width 64 bits integer.
 */
typedef unsigned long long uint64_t;

/** @brief Defines the boot_t type. */
typedef uint8_t bool_t;

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Imported global variables */
/* None */

/* Exported global variables */
/* None */

/* Static global variables */
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

#endif /* #ifndef __LIB_STDINT_H_ */

/* EOF */