/*******************************************************************************
 * @file ata_pio.c
 *
 * @see ata_pio.h
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

#include <lib/stdint.h>       /* Generic int types */
#include <lib/stddef.h>       /* Standard definitions */
#include <cpu.h>              /* CPU managment */
#include <io/kernel_output.h> /* Kernel output methods */
#include <sync/critical.h>    /* Critical sections */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <ata_pio.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#define DETECT_DEVICE(device, err) {                                           \
    err = ata_pio_identify_device(device);                                     \
    if(err == OS_NO_ERR)                                                       \
    {                                                                          \
        kernel_info("Found ATA device %s at 0x%x\n",((device->type == MASTER) ?\
                                                     "MASTER" : "SLAVE"),      \
                                                     device->port);            \
    }                                                                          \
    else if (err != OS_ERR_ATA_DEVICE_NOT_PRESENT)                             \
    {                                                                          \
        kernel_error("ATA device %s at 0x%x error [%d]\n",                     \
                                                    ((device->type == MASTER) ?\
                                                     "MASTER" : "SLAVE"),      \
                                                     device->port,             \
                                                     err);                     \
    }                                                                          \
}

#define INIT_DEVICE(device) {           \
    device->lock = SPINLOCK_INIT_VALUE; \
}

OS_RETURN_E ata_pio_init(void)
{
#if (ATA_PIO_DETECT_PRIMARY_PORT || ATA_PIO_DETECT_SECONDARY_PORT || \
         ATA_PIO_DETECT_THIRD_PORT || ATA_PIO_DETECT_FOURTH_PORT)
    ata_pio_device_t  device;
    ata_pio_device_t* ptr;
#endif
    OS_RETURN_E       err;

    err = OS_NO_ERR;
#if (ATA_PIO_DETECT_PRIMARY_PORT || ATA_PIO_DETECT_SECONDARY_PORT || \
         ATA_PIO_DETECT_THIRD_PORT || ATA_PIO_DETECT_FOURTH_PORT)
    ptr = &device;
#endif 
    
#if MAX_COU_COUNT > 1
    device.lock = SPINLOCK_INIT_VALUE;
#endif

    /* Check for all devices */
#if ATA_PIO_DETECT_PRIMARY_PORT == 1
    device.port = PRIMARY_PORT;
    device.type = MASTER;
    DETECT_DEVICE(ptr, err);
    device.type = SLAVE;
    DETECT_DEVICE(ptr, err);
#endif

#if ATA_PIO_DETECT_SECONDARY_PORT == 1
    device.port = SECONDARY_PORT;
    device.type = MASTER;
    DETECT_DEVICE(ptr, err);
    device.type = SLAVE;
    DETECT_DEVICE(ptr, err);
#endif

#if ATA_PIO_DETECT_THIRD_PORT == 1
    device.port = THIRD_PORT;
    device.type = MASTER;
    DETECT_DEVICE(ptr, err);
    device.type = SLAVE;
    DETECT_DEVICE(ptr, err);
#endif

#if ATA_PIO_DETECT_FOURTH_PORT == 1
    device.port = FOURTH_PORT;
    device.type = MASTER;
    DETECT_DEVICE(ptr, err);
    device.type = SLAVE;
    DETECT_DEVICE(ptr, err);
#endif

    if(err == OS_ERR_ATA_DEVICE_NOT_PRESENT)
    {
        err = OS_NO_ERR;
    }

#if TEST_MODE_ENABLED
    ata_pio_test();
#endif

    return err;
}

OS_RETURN_E ata_pio_identify_device(ata_pio_device_t* device)
{
    uint8_t  status;
    uint16_t i;
    char     ata_pio_str[513] = {0};
    uint16_t ata_pio_index;

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("IDENTIFY ATA 0x%p %s\n",
                        device->port,
                        ((device->type == MASTER) ? "MASTER" : "SLAVE"));
#endif

    /* Select slave or master */
    cpu_outb(device->type == MASTER ? 0xA0 : 0xB0,
         device->port + ATA_PIO_DEVICE_PORT_OFFSET);

    /* Check is the device is connected */
    cpu_outb(0x00, device->port + ATA_PIO_CONTROL_PORT_OFFSET);

    status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    if(status == 0xFF)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device not present\n");
#endif
        return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }

    /* Select slave or master */
    cpu_outb(device->type == MASTER ? 0xA0 : 0xB0,
         device->port + ATA_PIO_DEVICE_PORT_OFFSET);

    /* Write 0 to registers */
    cpu_outb(0x00, device->port + ATA_PIO_SC_PORT_OFFSET);
    cpu_outb(0x00, device->port + ATA_PIO_LBALOW_PORT_OFFSET);
    cpu_outb(0x00, device->port + ATA_PIO_LBAMID_PORT_OFFSET);
    cpu_outb(0x00, device->port + ATA_PIO_LBAHIG_PORT_OFFSET);

    /* Send the identify command */
    cpu_outb(ATA_PIO_IDENTIFY_COMMAND,
             device->port + ATA_PIO_COMMAND_PORT_OFFSET);

    /* Get the IDENTIFY status */
    status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    if(status == 0x00)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device not present\n");
#endif
        return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }

    /* Wait until device is ready */
    while(((status & ATA_PIO_FLAG_BUSY) == ATA_PIO_FLAG_BUSY) &&
          ((status & ATA_PIO_FLAG_ERR) != ATA_PIO_FLAG_ERR))
    {
        status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    }

    if((status & ATA_PIO_FLAG_ERR) == ATA_PIO_FLAG_ERR)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device error 0x%p (%s)\n", device->port,
                                                  ((device->type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
#endif
        return OS_ERR_ATA_DEVICE_ERROR;
    }

    /* The device data information is now ready to be read */
    ata_pio_index = 0;
    for(i = 0; i < 256; ++i)
    {
        uint16_t data;

        data = cpu_inw(device->port + ATA_PIO_DATA_PORT_OFFSET);
        ata_pio_str[ata_pio_index++] = (data >> 8) & 0xFF;
        ata_pio_str[ata_pio_index++] = data & 0xFF;
    }
    (void)ata_pio_str;

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA STR: %s\n", ata_pio_str);
#endif

    return OS_NO_ERR;
}

OS_RETURN_E ata_pio_read_sector(ata_pio_device_t* device, const uint32_t sector,
                                void* buffer, const uint32_t size)
{
    uint32_t    i;
    uint8_t     status;
    OS_RETURN_E err;
    uint32_t    int_state;

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA read request device 0x%p %s, sector 0x%p,\
size %d\n", device->port, ((device->type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
#endif

    err = OS_NO_ERR;

    /* Check sector */
    if(sector > 0x0FFFFFFF)
    {
        return OS_ERR_ATA_BAD_SECTOR_NUMBER;
    }

    /* Check read size */
    if(size > ATA_PIO_SECTOR_SIZE)
    {
        return OS_ERR_ATA_SIZE_TO_HUGE;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &device->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Set sector to read */
    cpu_outb((device->type == MASTER ? 0xE0 : 0xF0) |
             ((sector & 0x0F000000) >> 24) ,
            device->port + ATA_PIO_DEVICE_PORT_OFFSET);

    /* Clear error */
    cpu_outb(0, device->port + ATA_PIO_ERROR_PORT_OFFSET);

    /* Set number of sector to read */
    cpu_outb(1, device->port + ATA_PIO_SC_PORT_OFFSET);

    /* Set LBA values */
    cpu_outb(sector & 0x000000FF, device->port + ATA_PIO_LBALOW_PORT_OFFSET);
    cpu_outb((sector & 0x0000FF00) >> 8,
            device->port + ATA_PIO_LBAMID_PORT_OFFSET);
    cpu_outb((sector & 0x00FF0000) >> 16,
            device->port + ATA_PIO_LBAHIG_PORT_OFFSET);

    /* Send Read sector command */
    cpu_outb(ATA_PIO_READ_SECTOR_COMMAND,
            device->port + ATA_PIO_COMMAND_PORT_OFFSET);

    /* Wait until device is ready */
    status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    if(status == 0x00)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device not present\n");
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &device->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }
    while(((status & ATA_PIO_FLAG_BUSY) == ATA_PIO_FLAG_BUSY)
       && ((status & ATA_PIO_FLAG_ERR) != ATA_PIO_FLAG_ERR))
    {
        status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    }

    /* Check error status */
    if((status & ATA_PIO_FLAG_ERR) == ATA_PIO_FLAG_ERR)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device read error 0x%p (%s)\n", device->port,
                                                  ((device->type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &device->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_ATA_DEVICE_ERROR;
    }

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA read device 0x%p %s, sector 0x%p, size %d\n",
                        device->port,
                        ((device->type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
#endif

    /* Read data and copy to buffer */
    for(i = 0; i < size; i += 2)
    {
        uint16_t data;
        data = cpu_inw(device->port + ATA_PIO_DATA_PORT_OFFSET);

        ((uint8_t*)buffer)[i] = data & 0xFF;

        if(i + 1 < size)
        {
            ((uint8_t*)buffer)[i + 1] = (data >> 8) & 0xFF;
        }
    }

    /* Read the rest of the sector to release the memoey for next command */
    for(i = size + (size % 2); i < ATA_PIO_SECTOR_SIZE; i += 2)
    {
        cpu_inw(device->port + ATA_PIO_DATA_PORT_OFFSET);
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &device->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return err;
}

OS_RETURN_E ata_pio_write_sector(ata_pio_device_t* device,
                                 const uint32_t sector,
                                 const void* buffer, const uint32_t size)
{
    uint32_t i;
    uint32_t int_state;

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA write request device 0x%p %s, sector 0x%p,\
size %d\n", device->port, ((device->type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
#endif

    /* Check sector */
    if(sector > 0x0FFFFFFF)
    {
        return OS_ERR_ATA_BAD_SECTOR_NUMBER;
    }

    /* Check write size */
    if(size > ATA_PIO_SECTOR_SIZE)
    {
        return OS_ERR_ATA_SIZE_TO_HUGE;
    }

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &device->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Set sector to write */
    cpu_outb((device->type == MASTER ? 0xE0 : 0xF0) |
             ((sector & 0x0F000000) >> 24) ,
            device->port + ATA_PIO_DEVICE_PORT_OFFSET);

    /* Clear error */
    cpu_outb(0, device->port + ATA_PIO_ERROR_PORT_OFFSET);

    /* Set number of sector to write */
    cpu_outb(1, device->port + ATA_PIO_SC_PORT_OFFSET);

    /* Set LBA values */
    cpu_outb(sector & 0x000000FF,
            device->port + ATA_PIO_LBALOW_PORT_OFFSET);
    cpu_outb((sector & 0x0000FF00) >> 8,
            device->port + ATA_PIO_LBAMID_PORT_OFFSET);
    cpu_outb((sector & 0x00FF0000) >> 16,
            device->port + ATA_PIO_LBAHIG_PORT_OFFSET);

    /* Send write sector command */
    cpu_outb(ATA_PIO_WRITE_SECTOR_COMMAND,
            device->port + ATA_PIO_COMMAND_PORT_OFFSET);

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA write device 0x%p %s, sector 0x%p, size %d\n",
                        device->port,
                        ((device->type == MASTER) ? "MASTER" : "SLAVE"),
                        sector,
                        size);
#endif

    /* Write to disk */
    for(i = 0; i < size; i += 2)
    {
        uint16_t data;
        data = ((uint8_t*)buffer)[i] & 0x00FF;
        if(i + 1 < size)
        {
            data |= ((uint16_t)((uint8_t*)buffer)[i + 1]) << 8;
        }
        cpu_outw(data, device->port + ATA_PIO_DATA_PORT_OFFSET);
    }

    /* Add padding to the sector */
    for(i = size + (size % 2); i < ATA_PIO_SECTOR_SIZE; i += 2)
    {
        cpu_outw(0x0000, device->port + ATA_PIO_DATA_PORT_OFFSET);
    }
    
#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &device->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    /* Flush write */
    return ata_pio_flush(device);
}

OS_RETURN_E ata_pio_flush(ata_pio_device_t* device)
{
    uint8_t     status;
    uint32_t    int_state;
    OS_RETURN_E err;

    err = OS_NO_ERR;

#if ATA_PIO_KERNEL_DEBUG == 1
    kernel_serial_debug("ATA flush request device 0x%p %s\n",
                        device->port,
                        ((device->type == MASTER) ? "MASTER" : "SLAVE"));
#endif

#if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(int_state, &device->lock);
#else
    ENTER_CRITICAL(int_state);
#endif

    /* Set device */
    cpu_outb((device->type == MASTER ? 0xE0 : 0xF0),
         device->port + ATA_PIO_DEVICE_PORT_OFFSET);

    /* Send write sector command */
    cpu_outb(ATA_PIO_FLUSH_SECTOR_COMMAND,
            device->port + ATA_PIO_COMMAND_PORT_OFFSET);

    /* Wait until device is ready */
    status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);

    if(status == 0x00)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA device not present\n");
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &device->lock);
#else
     EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_ATA_DEVICE_NOT_PRESENT;
    }

    /* Check error status */
    if(status == 0)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA flush write error 0x%p (%s)\n", device->port,
                                                  ((device->type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &device->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_ATA_DEVICE_ERROR;
    }

    while(((status & ATA_PIO_FLAG_BUSY) == ATA_PIO_FLAG_BUSY)
       && ((status & ATA_PIO_FLAG_ERR) != ATA_PIO_FLAG_ERR))
    {
        status = cpu_inb(device->port + ATA_PIO_COMMAND_PORT_OFFSET);
    }

    /* Check error status */
    if((status & ATA_PIO_FLAG_ERR) == 1)
    {
#if ATA_PIO_KERNEL_DEBUG == 1
        kernel_serial_debug("ATA flush write error 0x%p (%s)\n", device->port,
                                                  ((device->type == MASTER) ?
                                                  "MASTER" : "SLAVE"));
#endif

#if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(int_state, &device->lock);
#else
        EXIT_CRITICAL(int_state);
#endif

        return OS_ERR_ATA_DEVICE_ERROR;
    }

#if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(int_state, &device->lock);
#else
    EXIT_CRITICAL(int_state);
#endif

    return err;
}