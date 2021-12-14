# UTK - Utility Kernel

![UTK Logo](https://github.com/Oxmose/UTK-Reboot/raw/main/Doc/logo/utk_logo.png | width=200)

* UTK is a kernel created for training and educational purposes. It is designed to execute in kernel mode only. 

* UTK has a configuration file allowing the kernel to be customized depending on the system it will run on.

* UTK can be build with its own bootloader or use GRUB as bootloader.

----------

*UTK Build status*


| Status | Master | Dev |
| --- | --- | --- |
| Travis CI | [![Build Status](https://www.travis-ci.com/Oxmose/UTK-Reboot.svg?branch=main)](https://www.travis-ci.com/Oxmose/UTK-Reboot) | [![Build Status](https://www.travis-ci.com/Oxmose/UTK-Reboot.svg?branch=dev)](https://www.travis-ci.com/Oxmose/UTK-Reboot) |
| Codacy | [![Codacy Badge](https://app.codacy.com/project/badge/Grade/ae0df892bb124da8b16d015e4f4f2aeb)](https://www.codacy.com/gh/Oxmose/UTK-Reboot/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Oxmose/UTK-Reboot&amp;utm_campaign=Badge_Grade)| N/A |


----------

## Architecture supported
| i386 | x86_64 | ARMv7 (v15) |
| --- | --- | --- |
|  YES |   SOON |      LATTER |

### Multicore support

* BMP support
* CPU thread affinity

### Memory Management

* Higher-half kernel.
* Virtual/physical memory allocator

### Synchronization

* Mutex: Non recursive/Recursive - Priority inheritance capable.
* Semaphore: FIFO based, priority of the locking thread is not relevant to select the next thread to unlock.
* Spinlocks: Disables interrupt on monocore systems, Test And Set on multicore systems.
* Message queues and mailboxes

### Scheduler

* Priority based scheduler (threads can change their priority at execution time).
* Round Robin for all the threads having the same priority.

### BSP Support: i386/x86_64

* PIC and IO-APIC support (PIC is used as a fallback when IO-APIC is disabled or not present).
* Local APIC support (can be disabled manually of is disabled if not detected or IO-APIC is disabled),
* PIT support (may be used as fallback for scheduler timer when LAPIC is disabled, is not, the PIT can be used as an auxiliary timer source).
* RTC support.
* Basic ACPI support (simple parsing used to enable multicore features).
* SMBIOS support.
* Serial output support.
* Interrupt API (handlers can be set by the user).
* 80x25 16colors VGA support. VESA Graphic support.
* Time management API.
* Keyboard (ASCII QWERTY) and mouse drivers.

## Build and run
To build UTK, choose the architecture you want and execute:

./build_[ARCH].sh [QEMU | HW] [GRUB | UTK] [k_name]

	 [QEMU | HW]
		- Choose QEMU to build a QEMU image
		- Choose HW to build a bootable image

	 [GRUB | UTK]
		- Choose GRUB to use GRUB as bootloader
		- Choose UTK to use UTK as bootloader

	 [k_name]
		-Set the name of the kernel
