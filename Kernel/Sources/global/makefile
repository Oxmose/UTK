################################################################################
# UTK Makefile
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

.PHONY: all
all: init module

init: 
	@mkdir -p $(BUILD_DIR)

module: 
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(KERNEL_NAME).elf $(DEP_MODULES) $(DEP_LIBS) -Map=$(BUILD_DIR)/output.map 

# Clean 
clean:
	$(RM) -rf $(BUILD_DIR)