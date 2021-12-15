<p align="center">
	<img src="https://github.com/Oxmose/UTK-Reboot/raw/main/Doc/logo/utk_logo.png" width="200">
</p>

## UTK - Utility Kernel

* UTK is a kernel created for training and educational purposes. It is designed to execute in kernel mode only.

* UTK has a configuration file allowing the kernel to be customized depending on the system it will run on.

* UTK can be build with its own bootloader or use GRUB as bootloader.

----------

*UTK Build status*


| Status | Main | Dev |
| --- | --- | --- |
| Travis CI | [![Build Status](https://app.travis-ci.com/Oxmose/UTK-Reboot.svg?branch=main)](https://app.travis-ci.com/Oxmose/UTK-Reboot) | [![Build Status](https://app.travis-ci.com/Oxmose/UTK-Reboot.svg?branch=dev)](https://app.travis-ci.com/Oxmose/UTK-Reboot) |
| Codacy | [![Codacy Badge](https://app.codacy.com/project/badge/Grade/ae0df892bb124da8b16d015e4f4f2aeb)](https://www.codacy.com/gh/Oxmose/UTK-Reboot/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Oxmose/UTK-Reboot&amp;utm_campaign=Badge_Grade)| N/A |


----------

## Architecture supported
| i386 | x86_64 | ARM |
| --- | --- | --- |
|  YES |   TBD | TBD |

### Memory Management

* Higher-half kernel.
* Virtual/physical memory allocator
* Process isolation

### Synchronization

* Futex based synchronization.
* Mutex: Non recursive/Recursive - Priority inheritance capable. Futex based.
* Semaphore: FIFO based, priority of the locking thread is not relevant to select the next thread to unlock. Futex based.
* Spinlocks: Disables interrupt on single-core architecture.

### Scheduler

* Priority based scheduler (threads can change their priority at execution time).
* Round Robin for all the threads having the same priority.

### Process / Thread management

* Fork / WaitPID
* Kernel threads support
* System calls

### BSP Support: i386/x86_64

* PIC and IO-APIC support (PIC is used as a fallback when IO-APIC is disabled or not present).
* Local APIC support (can be disabled manually of is disabled if not detected or IO-APIC is disabled),
* PIT support (may be used as fallback for scheduler timer when LAPIC is disabled, is not, the PIT can be used as an auxiliary timer source).
* RTC support.
* Basic ACPI support (simple parsing used to enable multicore features).
* SMBIOS support.
* Serial output support.
* Interrupt API (handlers can be set by the user).
* 80x25 16colors VGA support.
* Time management API.

----------

## Roadmap
You can see the planned tasks / features in the project's kanban on GitHub.

----------
## Build and run
To build UTK, choose the architecture you want and execute.

Architecture list to use in the TARGET flag:
* x86_i386
### Compilation
make target=[TARGET] TESTS=[TRUE/FALSE] DEBUG=[TRUE/FALSE]

### Execution
make target=[TARGET] run

### Tests and Debug
* The user can compile with the TESTS flag set to TRUE to enable internal testing
* The user can compile with the DEBUG flag set to TRUE to enable debuging support (-O0 -g3)