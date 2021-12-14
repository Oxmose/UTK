/*******************************************************************************
 * @file bsp_api.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2021
 *
 * @version 1.0
 *
 * @brief BSP API declarations.
 *
 * @details BSP API declarations.. This file contains all the routines 
 * aivailable for the system to manipulate the BSP.
 ******************************************************************************/

#ifndef __BOARD_BSP_API_H_
#define __BOARD_BSP_API_H_

#include <stdint.h> /* Generic types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the number of CPU detected on the system. 
 * 
 * @details Returns the number of CPU detected on the system. This function must 
 * be called after the init_acpi function.
 *
 * @return The number of CPU detected in the system, -1 is returned on error.
 */
int32_t get_cpu_count(void);

#endif /* #ifndef __BOARD_BSP_API_H_ */