/*******************************************************************************
 * @file mmio.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 04/04/2020
 *
 * @version 2.0
 *
 * @brief Kernel's memory mapped IO functions.
 * 
 * @details Kernel's memory mapped IO functions. Memory mapped IOs, avoids 
 * compilers to reorganize memory access.
 * So instead of doing : *addr = value, do
 * mapped_io_write(addr, value)
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __IO_MMIO_H_
#define __IO_MMIO_H_

#include <lib/stdint.h> /* Generic types */

/*******************************************************************************
 * DEFINITIONS
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
 * @brief Memory mapped IO byte write access.
 *
 * @details Memory mapped IOs byte write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_8(void* volatile addr,
                                         const uint8_t value)
{
    *(volatile uint8_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO half word write access.
 *
 * @details Memory mapped IOs half word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_16(void* volatile addr,
                                          const uint16_t value)
{
    *(volatile uint16_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO word write access.
 *
 * @details Memory mapped IOs word write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_32(void* volatile addr,
                                          const uint32_t value)
{
    *(volatile uint32_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO double word write access.
 *
 * @details Memory mapped IOs double word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_64(void* volatile addr,
                                          const uint64_t value)
{
    *(volatile uint64_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO byte read access.
 *
 * @details Memory mapped IOs byte read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint8_t mapped_io_read_8(const volatile void* addr)
{
    return *(volatile uint8_t*)(addr);
}

/**
 * @brief Memory mapped IO half word read access.
 *
 * @details Memory mapped IOs half word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint16_t mapped_io_read_16(const volatile void* addr)
{
    return *(volatile uint16_t*)(addr);
}

/**
 * @brief Memory mapped IO word read access.
 *
 * @details Memory mapped IOs word read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint32_t mapped_io_read_32(const volatile void* addr)
{
    return *(volatile uint32_t*)(addr);
}

/**
 * @brief Memory mapped IO double word read access.
 *
 * @details Memory mapped IOs double word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint64_t mapped_io_read_64(const volatile void* addr)
{
    return *(volatile uint64_t*)(addr);
}

#endif /* #ifndef __IO_MMIO_H_ */