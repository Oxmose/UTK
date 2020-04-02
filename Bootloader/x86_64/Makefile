################################################################################
# Created: 06/03/2020
#
# Original author: Alexy Torres Aurora Dugo
#
# UTK Bootloader makefile for x86_64 architecture
################################################################################

SRC_DIR    = src
BUILD_DIR  = build
BIN_DIR    = bin

BOOTLOADER = bootloader.bin
BOOTLOADER_IMG = bootloader.img

# Build system image
all: init $(BOOTLOADER)

# Run QEMU on our system
run:
	qemu-system-x86_64 -gdb tcp::1234 -smp 4 -m 8G -drive format=raw,file=$(BIN_DIR)/$(BOOTLOADER_IMG)

debug:
	qemu-system-x86_64 -gdb tcp::1234 -smp 4 -m 8G -drive format=raw,file=$(BIN_DIR)/$(BOOTLOADER_IMG) -S 

bootable:
	dd if=$(BIN_DIR)/$(BOOTLOADER) of=padded.iso bs=4M conv=sync && sync
	rm -f new.vdi
	VBoxManage convertfromraw padded.iso new.vdi --format VDI --uuid 42dd6ae3-5fab-45da-ba4b-f480709132c3
	VBoxManage startvm --putenv VBOX_GUI_DBG_ENABLED=true UTK_BOOT

# Initializes the build
init:
	@echo "\e[1m\e[34m============ Building x86_64 bootloader ============\e[22m\e[39m"
	@echo
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)

# Build system image
$(BOOTLOADER): $(BUILD_DIR)/bootstage01.bin
	@echo  "\e[32mCreating Binary file \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	
	@cat $^ > $(BIN_DIR)/$(BOOTLOADER)
	@qemu-img create -f raw $(BIN_DIR)/$(BOOTLOADER_IMG) 4M
	@dd status=noxfer conv=notrunc if=$(BIN_DIR)/$(BOOTLOADER) of=$(BIN_DIR)/$(BOOTLOADER_IMG)

	@echo
	@echo "\e[1m\e[94m============ Bootloader generated ============\e[22m\e[39m"
	@echo

$(BUILD_DIR)/bootstage01.bin: $(BUILD_DIR)/boot_stage0.bin $(BUILD_DIR)/boot_stage1.bin $(BUILD_DIR)/loader.bin $(BUILD_DIR)/configuration.bin $(BUILD_DIR)/dummy.bin
	@echo  "\e[32mMerging stages...\e[22m\e[39m"
	@cat $^ > $@

# Build boot sector
$(BUILD_DIR)/%.bin:$(SRC_DIR)/%.s
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	@nasm $< -f bin -o $@ 

# Clean
clean:
	rm -rf *.o *.bin *.vdi *.img *.iso $(BUILD_DIR) $(BIN_DIR)
