################################################################################
# Created: 21/09/2018
#
# Original author: Alexy Torres Aurora Dugo
#
# Last modified: 21/09/2018
#
# Last author: Alexy Torres Aurora Dugo
#
# UTK's main makefile
################################################################################

MK_DIR=./Makefiles/

######################### Compile options
.PHONY: all
all: init

init:

ifeq ($(arch), i386)
	@$(MAKE) -f ./$(MK_DIR)/Makei386.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386]\n"
	@exit 1
endif 

clean:

ifeq ($(arch), i386)
	@$(MAKE) clean -f ./$(MK_DIR)/Makei386.mk
else
	@$(MAKE) clean -f ./$(MK_DIR)/Makei386.mk
endif 

run:
ifeq ($(arch), i386)
	@$(MAKE) run -f ./$(MK_DIR)/Makei386.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386]\n"
	@exit 1
endif 

qemu-test-mode:
ifeq ($(arch), i386)
	@$(MAKE) qemu-test-mode -f ./$(MK_DIR)/Makei386.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386]\n"
	@exit 1
endif 

debug:
ifeq ($(arch), i386)
	@$(MAKE) debug -f ./$(MK_DIR)/Makei386.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386]\n"
	@exit 1
endif 

######################### Image file options
bootable: all
ifeq ($(arch), i386)
	@$(MAKE) bootable -f ./$(MK_DIR)/Makei386.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386]\n"
	@exit 1
endif 