################################################################################
# LUTk Makefile
# 
# Created: 23/05/2020
#
# Author: Alexy Torres Aurora Dugo
#
# Global module makefile. This makefile is used to link all modules together
################################################################################

# Dependencies 
include ./dependencies.mk
include settings.mk

# Variables definitions
BUILD_DIR  = ./build

MODULES=../arch ../user

.PHONY: all
all: init module

init: 
	@mkdir -p $(BUILD_DIR)

module: 
	$(LD) $(LDFLAGS) ../user/build/*.o -o $(BUILD_DIR)/$(KERNEL_NAME).elf $(DEP_MODULES) $(DEP_LIBS) 
#$(OBJCOPY) -O binary $(BUILD_DIR)/$(KERNEL_NAME).elf $(BUILD_DIR)/$(KERNEL_NAME).bin

# Clean 
clean:
	$(RM) -rf $(BUILD_DIR)