/*******************************************************************************
 * @file dev_manager.h
 *
 * @see dev_manager.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 31/12/2021
 *
 * @version 1.0
 *
 * @brief Kernel's device manager.
 *
 * @details Kernel's device manager. Defines the functions and structures used
 * by the kernel to manage the devices connected to the system.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __FS_DEV_MANAGER_H_
#define __FS_DEV_MANAGER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <kernel_error.h> /* Kernel error codes */
#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definitions */

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
 * @brief Initializes the device manager.
 *
 * @details Initializes the device manager. This function will detect the
 * devices present in the system and allocate them a virtual descriptor file.
 */
void dev_manager_init(void);

#endif /* #ifndef __FS_DEV_MANAGER_H_ */

/************************************ EOF *************************************/