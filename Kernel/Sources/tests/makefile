################################################################################
# UTK Makefile
# 
# Created: 23/05/2020
#
# Author: Alexy Torres Aurora Dugo
#
# Tests module makefile. This makefile is used to compile the tests module
# for the desired target.
################################################################################

CPU_ARCH_LIST=x86_i386

BUILD_DIR  = ./build
BIN_DIR    = ./bin

.PHONY: all
all: init build_modules

init: 

	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

build_cpu:
################################################################################
# Build the CPU target arch module
ifeq ($(target), x86_i386)
	@$(MAKE) -C ./x86_i386
else
	@echo "\e[1m\e[31m\n=== ERROR: Unknown architecture $(target_cpu)\e[22m\e[39m"
	@echo "Available architectures: $(CPU_ARCH_LIST)\n"
	@exit 1
endif 

build_gen:
################################################################################
# Build the general test odule
	@$(MAKE) -C ./general

build_modules: build_cpu build_gen
# Merge the two modules
	@ar r $(BIN_DIR)/libtests.a $(BUILD_DIR)/*.o
	@echo "\e[1m\e[92m\n=> Generated Tests module\e[22m\e[39m"
	@echo "\e[1m\e[92m--------------------------------------------------------------------------------\n\e[22m\e[39m"

clean:
################################################################################
# Clean the CPU target arch module
ifeq ($(target_cpu), x86_i386)
	@$(MAKE) -C ./x86_i386 clean
else
	@$(MAKE) -C ./x86_i386 clean
endif 

################################################################################
# Clean the general tests module
	@$(MAKE) -C ./general clean

	$(RM) -rf $(BUILD_DIR) $(BIN_DIR)

	@echo "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
	@echo "\e[1m\e[34m| Cleaned Tests module\e[22m\e[39m"
	@echo "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"