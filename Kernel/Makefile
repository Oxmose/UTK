################################################################################
# UTK Makefile
# 
# Created: 23/05/2020
#
# Author: Alexy Torres Aurora Dugo
#
# Main makefile. This makefile will proceed to compile all required modules
# and configurations.
# Once compiled it will produce the final binary file for the desired 
# architecture
################################################################################

CPU_ARCH_LIST= i386
TARGET_LIST= x86_i386
SOURCE_DIR= ./Sources
CONFIG_DIR= ./Config

BUILD_DIR = build

MODULES= -larch

KERNEL = utk

.PHONY: all
all: init build_modules build_kernel

init: 
	@mkdir -p $(BUILD_DIR)

ifeq ($(target), x86_i386)
	@cp $(CONFIG_DIR)/arch/x86_i386/settings.mk Sources/global/
	@cp $(CONFIG_DIR)/arch/x86_i386/config.h Sources/global/
	@cp $(CONFIG_DIR)/arch/x86_i386/config.inc Sources/global/
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif

build_modules:

# Build the architecture module
ifeq ($(target), x86_i386)
	@make -C $(SOURCE_DIR)/arch target_board=x86 target_cpu=i386
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif

# Build the user module 
	@make -C $(SOURCE_DIR)/user
# Build the io module 
	@make -C $(SOURCE_DIR)/io
# Build the core module 
	@make -C $(SOURCE_DIR)/core
# Build the lob module 
	@make -C $(SOURCE_DIR)/lib

build_kernel:
	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Building kernel for target $(target)\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

	@make -C $(SOURCE_DIR)/global KERNEL_NAME=$(KERNEL)

	@cp -r $(SOURCE_DIR)/global/build/* $(BUILD_DIR)

	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Generated kernel for target $(target)\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

clean:
# Dummy settings 
	@touch Sources/global/settings.mk
# Clean the architecture module
ifeq ($(target), x86_i386)
	@make -C $(SOURCE_DIR)/arch target_board=x86 target_cpu=i386 clean
else 
	@make -C $(SOURCE_DIR)/arch target_board=x86 target_cpu=i386 clean
endif

# Clean general modules
	@make -C $(SOURCE_DIR)/user clean
	@make -C $(SOURCE_DIR)/lib clean
	@make -C $(SOURCE_DIR)/io clean
	@make -C $(SOURCE_DIR)/core clean
	@make -C $(SOURCE_DIR)/global clean

# Clean kernel build directory
	rm -rf  $(BUILD_DIR) $(BIN_DIR)

	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Cleaned kernel \e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"
	
	@rm -f Sources/global/settings.mk
	@rm -f Sources/global/Config.h
	@rm -f Sources/global/Config.inc

run:
ifeq ($(target), x86_i386)
	@make run -f $(CONFIG_DIR)/arch/$(target)/makerun.mk BUILD_DIR=$(BUILD_DIR) KERNEL=$(KERNEL)
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif

debug:
ifeq ($(target), x86_i386)
	@make debug -f $(CONFIG_DIR)/arch/$(target)/makerun.mk BUILD_DIR=$(BUILD_DIR) KERNEL=$(KERNEL)
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif

qemu-test-mode:
ifeq ($(target), x86_i386)
	@make qemu-test-mode -f $(CONFIG_DIR)/arch/$(target)/makerun.mk BUILD_DIR=$(BUILD_DIR) KERNEL=$(KERNEL)
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif

qemu-grub:
ifeq ($(target), x86_i386)
	@make qemu-test-mode -f $(CONFIG_DIR)/arch/$(target)/makerun.mk BUILD_DIR=$(BUILD_DIR) KERNEL=$(KERNEL)
else 
	@echo "\e[1m\e[31m\n=== ERROR: Unknown target $(target)\e[22m\e[39m"
	@echo "Available targets: $(TARGET_LIST)\n"
	@exit 1
endif