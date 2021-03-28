/*******************************************************************************
 * @file cpu.h
 *
 * @see cpu.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief i386 CPU management functions
 *
 * @details i386 CPU manipulation functions. Wraps inline assembly calls for 
 * ease of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __I386_CPU_H_
#define __I386_CPU_H_

#include <stdint.h>       /* Generic int types */
#include <stddef.h>       /* Standard definition */
#include <cpu_settings.h> /* CPU Structures */
#include <ctrl_block.h>   /* Scheduler thrad structures */

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
 * @brief Returns the highest support CPUID feature request ID.
 *
 * @details Returns the highest supported input value for CPUID instruction.
 * ext canbe either 0x0 or 0x80000000 to return highest supported value for
 * basic or extended CPUID information.  Function returns 0 if CPUID
 * is not supported or whatever CPUID returns in eax register.  If sig
 * pointer is non-null, then first four bytes of the SIG
 * (as found in ebx register) are returned in location pointed by sig.
 *
 * @param[in] ext The opperation code for the CPUID instruction.
 * @return The highest supported input value for CPUID instruction.
 */
inline static uint32_t cpu_get_cpuid_max (const uint32_t ext)
{
    uint32_t regs[4];

    /* Host supports CPUID.  Return highest supported CPUID input value. */
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(ext));

    return regs[0];
}

/**
 * @brief Returns the CPUID data for a requested leaf.
 *
 * @details Returns CPUID data for requested CPUID leaf, as found in returned
 * eax, ebx, ecx and edx registers.  The function checks if CPUID is
 * supported and returns 1 for valid CPUID information or 0 for
 * unsupported CPUID leaf. All pointers are required to be non-null.
 *
 * @param[in] code The opperation code for the CPUID instruction.
 * @param[out] regs The register used to store the CPUID instruction return.
 * @return 1 in case of succes, 0 otherwise.
 */
inline static int32_t cpu_cpuid(const uint32_t code,
                                    uint32_t regs[4])
{
    uint32_t ext = code & 0x80000000;
    uint32_t maxlevel = cpu_get_cpuid_max(ext);

    if (maxlevel == 0 || maxlevel < code)
    {
        return 0;
    }
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(code));
    return 1;
}

/** @brief Clears interupt bit which results in disabling interrupts. */
inline static void cpu_clear_interrupt(void)
{
    __asm__ __volatile__("cli":::"memory");
}

/** @brief Sets interrupt bit which results in enabling interupts. */
inline static void cpu_set_interrupt(void)
{
    __asm__ __volatile__("sti":::"memory");
}

/** @brief Halts the CPU for lower energy consuption. */
inline static void cpu_hlt(void)
{
    __asm__ __volatile__ ("hlt":::"memory");
}

/**
 * @brief Returns the current CPU flags.
 *
 * @return The current CPU flags.
 */
inline static uint32_t cpu_save_flags(void)
{
    uint32_t flags;

    __asm__ __volatile__(
        "pushfl\n"
        "\tpopl    %0\n"
        : "=g" (flags)
        :
        : "memory"
    );

    return flags;
}

/**
 * @brief Restores CPU flags
 *
 * @param[in] flags The flags to be restored.
 */
inline static void cpu_restore_flags(const uint32_t flags)
{
    __asm__ __volatile__(
        "pushl    %0\n"
        "\tpopfl\n"
        :
        : "g" (flags)
        : "memory"
    );
}

/**
 * @brief Writes byte on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
inline static void cpu_outb(const uint8_t value, const uint16_t port)
{
    __asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Writes word on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
inline static void cpu_outw(const uint16_t value, const uint16_t port)
{
    __asm__ __volatile__("outw %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Writes long on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
inline static void cpu_outl(const uint32_t value, const uint16_t port)
{
    __asm__ __volatile__("outl %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Reads byte on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
inline static uint8_t cpu_inb(const uint16_t port)
{
    uint8_t rega;
    __asm__ __volatile__("inb %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Reads word on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
inline static uint16_t cpu_inw(const uint16_t port)
{
    uint16_t rega;
    __asm__ __volatile__("inw %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Reads long on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
inline static uint32_t cpu_inl(const uint16_t port)
{
    uint32_t rega;
    __asm__ __volatile__("inl %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Reads the TSC value of the CPU.
 *
 * @details Reads the current value of the CPU's time-stamp counter and store
 * into EDX:EAX. The time-stamp counter contains the amount of clock ticks that
 * have elapsed since the last CPU reset. The value is stored in a 64-bit MSR
 * and is incremented after each clock cycle.
 *
 * @return The CPU's TSC time stamp.
 */
inline static uint64_t cpu_rdtsc(void)
{
    uint64_t ret;
    __asm__ __volatile__ ( "rdtsc" : "=A"(ret) );
    return ret;
}

/**
 * @brief Checks the architecture's feature and requirements for UTK.
 *
 * @details Checks the architecture's feature and requirements for UTK. If a 
 * requirement is not met, a kernel panic is raised.
 */
void validate_architecture(void);

#endif /* #ifndef __I386_CPU_H_ */
