
#include <lib/stdio.h>
#include <lib/string.h>
#include <io/kernel_output.h>
#include <cpu.h>
#include <ata_pio.h>

#include <Tests/test_bank.h>

#if ATA_PIO_TEST  == 1
void ata_pio_test(void)
{
    ata_pio_device_t dev;
    dev.port = PRIMARY_PORT;
    dev.type = MASTER;

    OS_RETURN_E err;
    uint32_t error = 0;

    char buffer[36] = "Read/Write test UTK ATA-PIO driver\0";
    char recv[36] = {0};
    if((err = ata_pio_write_sector(&dev, 0, (void*)buffer, 35)) != OS_NO_ERR)
    {
        kernel_error("Failed to write 1 [%d]\n", err);
        ++error;
    }
    else
    {
        kernel_debug("[TESTMODE] Wrote: %s\n", buffer);
    }
    if(ata_pio_write_sector(&dev, 1, (void*)buffer, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to write 2\n");
        ++error;
    }
    else
    {
        kernel_debug("[TESTMODE] Wrote: %s\n", buffer);
    }

    if(ata_pio_read_sector(&dev, 1, (void*)recv, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to read 1\n");
        ++error;
    }
    else
    {
        kernel_debug("[TESTMODE] Read: %s\n", recv);
    }
    if(strncmp(recv, buffer, 35) != 0)
    {
        recv[35] = 0;
        buffer[35] = 0;
        kernel_error("Failed to compare 1\n --> %s \n --> %s\n", recv, buffer);
        ++error;
    }

    memset(recv, 0, 36);
    if(ata_pio_read_sector(&dev, 0, (void*)recv, 35) != OS_NO_ERR)
    {
        kernel_error("Failed to read 2\n");
        ++error;
    }
    else
    {
        kernel_debug("[TESTMODE] Read: %s\n", recv);
    }
    if(strncmp(recv, buffer, 35) != 0)
    {
        recv[35] = 0;
        buffer[35] = 0;
        kernel_error("Failed to compare 2\n --> %s \n --> %s\n", recv, buffer);
        ++error;
    }

    if(error == 0)
        kernel_debug("[TESTMODE] ATA tests passed\n");

    /* Kill QEMU */
    cpu_outw(0x2000, 0x604);    
    while(1)
    {
        __asm__ ("hlt");
    }
}
#else 
void ata_pio_test(void)
{
}
#endif