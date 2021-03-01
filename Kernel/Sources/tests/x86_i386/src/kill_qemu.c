#ifdef TEST_MODE_ENABLED

void kill_qemu(void)
{   
    while(1)
    {
        __asm__ __volatile__("outw %0, %1" : : "a" (0x2000), "Nd" (0x604));
        __asm__ ("hlt");
    }
}

#endif