/*******************************************************************************
 * @file ata_pio.h
 *
 * @see ata_pio.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 1.0
 *
 * @brief ATA (Advanced Technology Attachment) driver.
 *
 * @details ATA (Advanced Technology Attachment) driver. Supports hard drive IO
 * through the CPU PIO. The driver can read and write data. No utility function
 * are provided.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __X86_ATA_PIO_H_
#define __X86_ATA_PIO_H_

#include <lib/stdint.h>    /* Generic int types */
#include <lib/stddef.h>    /* Standard definitions */
#include <sync/critical.h> /* Critical sections */

/* UTK Configuration file */
#include <config.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief ATA primary port PIO id. */
#define ATA_PIO_PRIMARY_PORT_ADDRESS   0x000001F0
/** @brief ATA secondary port PIO id. */
#define ATA_PIO_SECONDARY_PORT_ADDRESS 0x00000170
/** @brief ATA third port PIO id. */
#define ATA_PIO_THIRD_PORT_ADDRESS     0x000001E8
/** @brief ATA fourth port PIO id. */
#define ATA_PIO_FOURTH_PORT_ADDRESS    0x00000168

/** @brief ATA data port offset. */
#define ATA_PIO_DATA_PORT_OFFSET    0x000
/** @brief ATA error port offset. */
#define ATA_PIO_ERROR_PORT_OFFSET   0x001
/** @brief ATA sector count port offset. */
#define ATA_PIO_SC_PORT_OFFSET      0x002
/** @brief ATA sector number port offset. */
#define ATA_PIO_LBALOW_PORT_OFFSET  0x003
/** @brief ATA cylinder low port offset. */
#define ATA_PIO_LBAMID_PORT_OFFSET  0x004
/** @brief ATA cylinder high port offset. */
#define ATA_PIO_LBAHIG_PORT_OFFSET  0x005
/** @brief ATA head port offset. */
#define ATA_PIO_DEVICE_PORT_OFFSET  0x006
/** @brief ATA status port offset. */
#define ATA_PIO_COMMAND_PORT_OFFSET 0x007
/** @brief ATA control port offset. */
#define ATA_PIO_CONTROL_PORT_OFFSET 0x206

/** @brief ATA PIO identify command. */
#define ATA_PIO_IDENTIFY_COMMAND     0xEC
/** @brief ATA PIO read command. */
#define ATA_PIO_READ_SECTOR_COMMAND  0x20
/** @brief ATA PIO write command. */
#define ATA_PIO_WRITE_SECTOR_COMMAND 0x30
/** @brief ATA PIO flush command. */
#define ATA_PIO_FLUSH_SECTOR_COMMAND 0xE7

/** @brief ATA status busy flag. */
#define ATA_PIO_FLAG_BUSY 0x80
/** @brief ATA status error flag. */
#define ATA_PIO_FLAG_ERR  0x01

/** @brief ATA ssupported sector size. */
#define ATA_PIO_SECTOR_SIZE 512

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief ATA PIO device type enumeration. */
enum ATA_PIO_TYPE
{
    /** @brief ATA PIO Master device. */
    MASTER = 0,
    /** @brief ATA PIO Slave device. */
    SLAVE  = 1
};

/**
 * @brief Defines ATA_PIO_TYPE_E type as a shorcut for enum ATA_PIO_TYPE.
 */
typedef enum ATA_PIO_TYPE ATA_PIO_TYPE_E;

/** @brief ATA PIO device port enumeration. */
enum ATA_PIO_PORT
{
    /** @brief ATA PIO Primary devices port. */
    PRIMARY_PORT   = ATA_PIO_PRIMARY_PORT_ADDRESS,
    /** @brief ATA PIO Secondary devices port. */
    SECONDARY_PORT = ATA_PIO_SECONDARY_PORT_ADDRESS,
    /** @brief ATA PIO Third devices port. */
    THIRD_PORT     = ATA_PIO_THIRD_PORT_ADDRESS,
    /** @brief ATA PIO Fourth devices port. */
    FOURTH_PORT    = ATA_PIO_FOURTH_PORT_ADDRESS
};

/**
 * @brief Defines ATA_PIO_PORT_E type as a shorcut for enum ATA_PIO_PORT.
 */
typedef enum ATA_PIO_PORT ATA_PIO_PORT_E;

/** @brief ATA PIO device representation in the driver. */
struct ata_pio_device
{
    /** @brief Device port. */
    ATA_PIO_PORT_E port;
    /** @brief Device type. */
    ATA_PIO_TYPE_E type;

#if MAX_CPU_COUNT > 1
    /** @brief Critical section spinlock. */
    spinlock_t lock;
#endif
};

/**
 * @brief Defines ata_pio_device_t type as a shorcut for struct ata_pio_device.
 */
typedef struct ata_pio_device ata_pio_device_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the ATA PIO driver settings.
 *
 * @details Initializes the ATA PIO driver settings. The driver will detect all
 * the connected ATA device in the system.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_ATA_DEVICE_ERROR is returned if an errored device is detected.
 */
OS_RETURN_E ata_pio_init(void);

/**
 * @brief Identifies a given ATA device if connected.
 *
 * @details Identify the ATA device given as parameter. The function will check
 * the presence of a device conected to the port pointed by the device argument.
 *
 * @param[in] device The device to identify.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_ATA_DEVICE_ERROR is returned if an errored device is detected.
 * - OS_ERR_ATA_DEVICE_NOT_PRESENT is returned if the device was not detected.
 */
OS_RETURN_E ata_pio_identify_device(ata_pio_device_t* device);

/**
 * @brief Reads the content of a sector in a buffer.
 *
 * @details Reads size bytes on the device pointer by the device given as
 * parameter. The number of bytes to read must be less of equal to the size of
 * a sector.
 *
 * @warning The number of bytes to read must be less of equal to the size of
 * a sector.
 *
 * @param[in] device The device to read the data from.
 * @param[in] sector The sector number where the data are located.
 * @param[out] buffer The buffer that is used to store the read data.
 * @param[in] size The number of bytes to read from the device.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_ATA_DEVICE_ERROR is returned if an errored device is detected.
 * - OS_ERR_ATA_DEVICE_NOT_PRESENT is returned if the device was not detected.
 * - OS_ERR_ATA_BAD_SECTOR_NUMBER is returned if the sector given in parameters
 *   is not supported.
 * - OS_ERR_ATA_SIZE_TO_HUGE is returned if the size of the data to read is more
 *   than the size of a sector.
 */
OS_RETURN_E ata_pio_read_sector(ata_pio_device_t* device,
                                const uint32_t sector,
	                            void* buffer, const uint32_t size);

/**
 * @brief Writes the content of the buffer to the device sector.
 *
 * @details Writes size bytes on the device pointer by the device given as
 * parameter. The number of bytes to written must be less of equal to the size
 * of a sector. Padding is added at the end of the sector, all other data
 * present in the sector before the write opperation are overwritten.
 *
 * @param[in] device The device to write the data to.
 * @param[in] sector The sector number where the data are to be written.
 * @param[in] buffer The buffer containing the data to write.
 * @param[in] size The number of bytes to read from the device.
 *
 * @warning Padding is added at the end of the sector, all other data present in
 * the sector before the write opperation are overwritten.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_ATA_DEVICE_ERROR is returned if an errored device is detected.
 * - OS_ERR_ATA_DEVICE_NOT_PRESENT is returned if the device was not detected.
 * - OS_ERR_ATA_BAD_SECTOR_NUMBER is returned if the sector given in parameters
 *   is not supported.
 * - OS_ERR_ATA_SIZE_TO_HUGE is returned if the size of the data to write is
 *   more than the size of a sector.
 */
OS_RETURN_E ata_pio_write_sector(ata_pio_device_t* device,
                                 const uint32_t sector,
	                             const void* buffer, const uint32_t size);

/**
 * @brief Asks the device to flush it's buffer.
 *
 * @details Ask the ATA device to flush the data cache. This is used to ensure
 * correct writing to the device.
 *
 * @param[in] device The device to be flushed.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_ATA_DEVICE_ERROR is returned if an errored device is detected.
 * - OS_ERR_ATA_DEVICE_NOT_PRESENT is returned if the device was not detected.
 */
OS_RETURN_E ata_pio_flush(ata_pio_device_t* device);

#endif /* #ifndef __X86_ATA_PIO_H_ */