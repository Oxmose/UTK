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

######################### Modules selection
ARCH_DEP = x86
CPU_DEP  = i386
MODULES  = io lib memory interrupt core time user comm sync .

TESTS_DIR  = Tests/Tests
TEST_ARCH_DIR = Tests/Tests/i386
TESTS_INC  = Tests

ifeq ($(TESTS), TRUE)
MODULES += ../$(TESTS_DIR) ../$(TEST_ARCH_DIR)
endif

SRC_DEP = arch/cpu/$(CPU_DEP) arch/$(ARCH_DEP) $(MODULES)

######################### Files options
# Kernel name
KERNEL = kernel.bin

# Source directories definition
SRC_DIR    = Sources
BUILD_DIR  = Build
BIN_DIR    = Bin

INC_DIR    = Includes
INC_ARCH   = $(INC_DIR)/arch/$(ARCH_DEP)
INC_CPU    = $(INC_DIR)/arch/cpu/$(CPU_DEP)

CONFIG_DIR  = Config/i386
LINKER_FILE = $(CONFIG_DIR)/linker.ld

SRC_DIRS = $(SRC_DEP:%=$(SRC_DIR)/%)
OBJ_DIRS = $(SRC_DEP:%=$(BUILD_DIR)/%)

C_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
A_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
A_OBJS = $(A_SRCS:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)


######################### Toolchain options
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-i386

DEBUG_FLAGS = -O0 -g
EXTRA_FLAGS = -O2 -g

CFLAGS = -m32 -std=c11 -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -fno-pie \
		 -no-pie -MD -ffreestanding
		 
ifeq ($(DEBUG), TRUE)
CFLAGS += $(DEBUG_FLAGS)
else
CFLAGS += $(EXTRA_FLAGS)
endif

ASFLAGS = -g -f elf -w+gnu-elf-extensions
LDFLAGS = -T $(LINKER_FILE) -melf_i386 -no-pie
QEMUOPTS = -cpu Nehalem -d guest_errors -rtc base=localtime -m 256M \
           -gdb tcp::1234 -smp 4 -drive format=raw,file=../Peripherals/hdd_primary_master.img

######################### Compile options
.PHONY: all
all: init kernel.bin

init:
	@echo "\e[1m\e[34m=== Building UTK for i386 architecture\e[22m\e[39m"
	@echo "\e[1m\e[34mModules: $(MODULES)\e[22m\e[39m"

	@mkdir -p $(OBJ_DIRS)
	@mkdir -p $(BIN_DIR)

# kernel generation
kernel.bin: $(LINKER_FILE) compile_asm compile_cc
	@echo  "\e[32m=== Creating Binary file \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	@$(LD) $(LDFLAGS) $(C_OBJS) $(A_OBJS) -o $(BIN_DIR)/$(KERNEL)
	@echo

compile_asm: $(A_OBJS)
	@echo "\e[1m\e[94m=== Compiled ASM sources\e[22m\e[39m"
	@echo

compile_cc: $(C_OBJS)
	@echo "\e[1m\e[94m=== Compiled C sources\e[22m\e[39m"
	@echo

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
ifeq ($(DEBUG), TRUE)
	@echo -n "DEBUG "
endif
ifeq ($(TESTS), TRUE)
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(INC_ARCH) -I $(INC_CPU) -I $(CONFIG_DIR) -I $(TESTS_INC)
else
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(INC_ARCH) -I $(INC_CPU) -I $(CONFIG_DIR)
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	@echo "\e[1m\e[94m=== Cleaning Object files and Binaries\e[22m\e[39m"
	@echo

	@$(RM) -rf $(BIN_DIR) $(BUILD_DIR)
	@$(RM) -f $(TESTS_DIR)/*.o $(TESTS_DIR)/*.d 
	@$(RM) -f $(TEST_ARCH_DIR)/*.o $(TEST_ARCH_DIR)/*.d
	@$(RM) -rf ./GRUB

# Check header files modifications
-include $(C_OBJS:.o=.d)
-include $(A_OBJS:.o=.d)

######################### Qemu options
run:
	@echo "\e[1m\e[94m=== Running on Qemu\e[22m\e[39m"
	rm -rf ./GRUB
	cp -R ../Bootloader/GRUB .
	cp $(BIN_DIR)/$(KERNEL) ./GRUB/boot/
	@$(RM) -f ./$(BIN_DIR)/bootable.iso
	grub-mkrescue -o ./$(BIN_DIR)/bootable.iso ./GRUB
	@$(QEMU) $(QEMUOPTS) -boot d -cdrom ./$(BIN_DIR)/bootable.iso -serial stdio

qemu-test-mode:
	@echo "\e[1m\e[94m=== Running on Qemu TEST MODE\e[22m\e[39m"
	rm -rf ./GRUB
	cp -R ../Bootloader/GRUB .
	cp $(BIN_DIR)/$(KERNEL) ./GRUB/boot/
	@$(RM) -f ./$(BIN_DIR)/bootable.iso
	grub-mkrescue -o ./$(BIN_DIR)/bootable.iso ./GRUB
	@$(QEMU) $(QEMUOPTS) -boot d -cdrom ./$(BIN_DIR)/bootable.iso -serial stdio -nographic -monitor none

debug:
	@echo "\e[1m\e[94m=== Running on Qemu TEST MODE\e[22m\e[39m"
	rm -rf ./GRUB
	cp -R ../Bootloader/GRUB .
	cp $(BIN_DIR)/$(KERNEL) ./GRUB/boot/
	@$(RM) -f ./$(BIN_DIR)/bootable.iso
	grub-mkrescue -o ./$(BIN_DIR)/bootable.iso ./GRUB
	@$(QEMU) $(QEMUOPTS) -boot d -cdrom ./$(BIN_DIR)/bootable.iso -monitor telnet:127.0.0.1:1235,server,nowait -serial stdio -S 

######################### Image file options
bootable: all
	@echo "\e[1m\e[94m=== Creating Bootable ISO\e[22m\e[39m"
	rm -rf ./GRUB
	cp -R ../Bootloader/GRUB .
	cp $(BIN_DIR)/$(KERNEL) ./GRUB/boot/
	@$(RM) -f ./$(BIN_DIR)/bootable.iso
	grub-mkrescue -o ./$(BIN_DIR)/bootable.iso ./GRUB
