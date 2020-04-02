#!/bin/bash
echo
echo -e "\e[1m\e[34m#################################\e[22m\e[39m"
echo -e "\e[1m\e[34m#       UTK Builder I386        #\e[22m\e[39m"
echo -e "\e[1m\e[34m#################################\e[22m\e[39m"
echo

function display_usage() 
{
    
    echo -e "\e[1m\e[31mUsage: $0 [QEMU | HW] [GRUB | UTK] [k_name]\e[22m\e[39m"
    echo
    echo -e "\e[1m\e[31m\t [QEMU | HW]\e[22m\e[39m"
    echo -e "\e[1m\e[31m\t\t- Choose QEMU to build a QEMU image\e[22m\e[39m"
    echo -e "\e[1m\e[31m\t\t- Choose HW to build a bootable image\e[22m\e[39m"
    echo
    echo -e "\e[1m\e[31m\t [GRUB | UTK]\e[22m\e[39m"
    echo -e "\e[1m\e[31m\t\t- Choose GRUB to use GRUB as bootloader\e[22m\e[39m"
    echo -e "\e[1m\e[31m\t\t- Choose UTK to use UTK as bootloader\e[22m\e[39m"
    echo
    echo -e "\e[1m\e[31m\t [k_name]\e[22m\e[39m"
    echo -e "\e[1m\e[31m\t\t-Set the name of the kernel\e[22m\e[39m"
    exit -1
}

function build_kernel()
{
    echo -e "\e[1m\e[34m========== Building Kernel\e[22m\e[39m"
    echo
    cd Source
    {
        make clean && make arch=i386 
    } > /dev/null
    if [ $? != 0 ]
    then
        exit $?
    fi

    cd ..
}

function build_bootloader()
{
    echo -e "\e[1m\e[34m========== Building Bootloader\e[22m\e[39m"
    echo

    cd Bootloader/i386

    k_size=$(wc -c < ../../build/kernel.bin)
    k_size_sect=$(( $k_size / 512  * 2))
    k_left=$(( $k_size % 512 ))
    if [ $k_left -ne 0 ]
    then 
        k_size_sect=$(( $k_size_sect + 1 ))
    fi
    echo "[bits 32]"                              > src/configuration.s 
    echo "dd $k_size_sect       ; Kernel size in sectors" >> src/configuration.s 
    echo "dd 0x00100000 ; Kernel start address"   >> src/configuration.s 
    echo "dd 0x00100000 ; Kernel entry point"     >> src/configuration.s 
    echo "dd $(( ${#1} + 1 )) ; Kernel name size"       >> src/configuration.s 
    echo "db \"$1\", 0     ; Kernel name"            >> src/configuration.s 
    echo "times 510-(\$-\$\$) db 0xFF"               >> src/configuration.s 
    echo "dw 0xE621"                              >> src/configuration.s 

    {
        make clean && make 
    } > /dev/null

    if [ $? != 0 ]
    then
        exit $?
    fi

    echo -e "\e[1m\e[34m========== Merging Images\e[22m\e[39m"
    cp bin/bootloader.bin ../../build/bootloader.bin 
    cd ../../build 
    cat bootloader.bin kernel.bin > os_binary.bin
    cd ..
}

PLATFORM=0
BOOTLOADER=0

if [ "$#" -ne 3 ]; then
    display_usage
fi

if [ "$1" = "QEMU" ]
then 
    PLATFORM=1
elif [ "$1" = "HW" ]
then 
    PLATFORM=2
else
    display_usage
fi

if [ "$2" = "GRUB" ]
then 
    BOOTLOADER=1
elif [ "$2" = "UTK" ]
then 
    BOOTLOADER=2
else
    display_usage
fi

if [ $PLATFORM = 1 ]
then 
    echo -e "\e[32m\tSelected platform: QEMU\e[22m\e[39m"
elif [ $PLATFORM = 2 ]
then 
    echo -e "\e[32m\tSelected platform: HW\e[22m\e[39m"
else 
    display_usage
fi

if [ $BOOTLOADER = 1 ]
then 
    echo -e "\e[32m\tSelected bootloader: GRUB\e[22m\e[39m"
elif [ $BOOTLOADER = 2 ]
then 
    echo -e "\e[32m\tSelected bootloader: UTK\e[22m\e[39m"
else 
    display_usage
fi

echo 

rm -rf build
mkdir -p build 

#####################################
# Building Kernel
#####################################
build_kernel

if [ $PLATFORM -eq 1 ] && [ $BOOTLOADER -eq 1 ]
then 
    cp Source/Bin/kernel.bin ../QEMU_GRUB.bin

    echo -e "\e[1m\e[34m========== Image generated: QEMU_GRUB.bin\e[22m\e[39m"
    exit 0
fi

if [ $PLATFORM -eq 1 ] && [ $BOOTLOADER -eq 2 ]
then 
    objcopy -O binary Source/Bin/kernel.bin build/kernel.bin
    if [ $? != 0 ]
    then
        exit $?
    fi

    build_bootloader $3

    echo -e "\e[1m\e[34m========== Creating QEMU Disk\e[22m\e[39m"
    {
        qemu-img create -f raw QEMU_UTK.img 64M
        if [ $? != 0 ]
        then
            exit $?
        fi
        dd status=noxfer conv=notrunc if=build/os_binary.bin of=QEMU_UTK.img
        if [ $? != 0 ]
        then
            exit $?
        fi
    } > /dev/null

    echo
    echo -e "\e[1m\e[34m========== Image generated: QEMU_UTK.bin\e[22m\e[39m"
    exit 0
fi

if [ $PLATFORM -eq 2 ] && [ $BOOTLOADER -eq 1 ]
then 
    echo -e "\e[1m\e[34m========== Creating ISO Image\e[22m\e[39m"
    cd Source/
    {
        make arch=i386 bootable
        if [ $? != 0 ]
        then
            exit $?
        fi
    } > /dev/null
    cd ..
    mv Image/bootable.iso HW_GRUB.iso
    echo
    echo -e "\e[1m\e[34m========== Image generated:HW_GRUB.iso\e[22m\e[39m"
    exit 0
fi

if [ $PLATFORM -eq 2 ] && [ $BOOTLOADER -eq 2 ]
then 
    objcopy -O binary Source/Bin/kernel.bin build/kernel.bin
    if [ $? != 0 ]
    then
        exit $?
    fi

    build_bootloader $3

    echo
    echo -e "\e[1m\e[34m========== Creating Binaries\e[22m\e[39m"
    {
        mkdir -p build/deploy
        cp build/os_binary.bin
        dd bs=1024 count=1440 if=/dev/zero of=build/deploy/os_image.img
        dd if=build/os_binary.bin of=build/deploy/os_image.img seek=0 conv=notrunc
        xorriso -as mkisofs -hard-disk-boot -U -b os_image.img -hide os_image.img -V "UTK HW" -iso-level 3 -o ./HW_UTK.iso ./build/deploy

    } > /dev/null

    echo
    echo -e "\e[1m\e[34m========== Image generated: HW_UTK.iso \e[22m\e[39m"
    exit 0
fi