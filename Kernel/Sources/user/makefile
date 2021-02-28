################################################################################
# LUTk Makefile
# 
# Created: 23/05/2020
#
# Author: Alexy Torres Aurora Dugo
#
# User module makefile. This makefile is used to compile the user module.
################################################################################

# Dependencies 
include ./dependencies.mk
include ../global/settings.mk

# Variables definitions
SRC_DIR    = src
BUILD_DIR  = ./build
INC_DIR    = includes

SRC_DEP = $(sort $(dir $(wildcard $(SRC_DIR)/*/)))

SRC_DIRS = $(sort $(dir $(wildcard $(SRC_DIR)/*/)))

C_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)*.c))
A_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)*.S))
C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
A_OBJS = $(A_SRCS:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

.PHONY: all
all: init module

init: 
	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Compiling User module\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

	@mkdir -p $(BUILD_DIR)

module: compile_asm compile_cc
	@echo "\e[1m\e[92m=> Generated User module\e[22m\e[39m"
	@echo "\e[1m\e[92m--------------------------------------------------------------------------------\n\e[22m\e[39m"

# Assembly sources compilation
compile_asm: $(A_OBJS)
	@echo "\e[1m\e[94m=> Compiled ASM sources\e[22m\e[39m"
	@echo

# C sources compilation
compile_cc: $(C_OBJS)
	@echo "\e[1m\e[94m=> Compiled C sources\e[22m\e[39m"
	@echo

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
ifeq ($(DEBUG), TRUE)
	@echo -n "  DEBUG "
endif
	@echo  "\e[32m  $< \e[22m\e[39m=> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) $(DEP_INCLUDES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@echo  "\e[32m  $< \e[22m\e[39m=> \e[1m\e[94m$@\e[22m\e[39m"
	$(AS) $(ASFLAGS) $< -o $@

# Clean 
clean:
	$(RM) -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Cleaned User module\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"