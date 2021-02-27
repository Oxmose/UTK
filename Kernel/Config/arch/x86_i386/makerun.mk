################################################################################
# UTK Makefile
# 
# Created: 27/02/2021
#
# Author: Alexy Torres Aurora Dugo
#
# Run make file, used to build the final kernel image and run it on QEMU or
# virtualbox.
################################################################################

QEMUOPTS = -cpu Nehalem -d guest_errors -rtc base=localtime -m 256M \
           -gdb tcp::1234 -smp 4 -serial stdio

QEMU = qemu-system-i386

######################### Qemu options
run: 	
	@echo "\e[1m\e[94m=== Running on Qemu\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) -kernel ./$(BUILD_DIR)/$(KERNEL).elf

qemu-test-mode:
	@echo "\e[1m\e[94m=== Running on Qemu TEST MODE\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) -kernel ./$(BUILD_DIR)/$(KERNEL).elf -nographic -monitor none

debug:
	@echo "\e[1m\e[94m=== Running on Qemu DEBUG MODE\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) -kernel ./$(BUILD_DIR)/$(KERNEL).elf -S

qemu-grub:
	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Preparing run for target $(target)\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"
	rm -rf ./$(BUILD_DIR)/GRUB
	$(RM) -f ./$(BUILD_DIR)/bootable.iso
	cp -R Config/arch/x86_i386/GRUB ./$(BUILD_DIR)/
	cp ./$(BUILD_DIR)/$(KERNEL).elf ./$(BUILD_DIR)/GRUB/boot/
	grub-mkrescue -o ./$(BUILD_DIR)/bootable.iso ./$(BUILD_DIR)/GRUB