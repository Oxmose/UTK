#!/bin/bash

echo -e "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
echo -e "\e[1m\e[34m| Creating init ram disk for architecture x86_i386\e[22m\e[39m"
echo -e "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

# Create master block 
echo -n "UTKINIRD" > $1/utk.initrd
echo -n -e '\x00\x06\x00\x00' >> $1/utk.initrd
for (( i=0; i<500; i++ ))
do  
    echo -n -e '\xBE' >> $1/utk.initrd
done

# Create free memory
for (( i=0; i<1024; i++ ))
do  
    echo -n -e '\x00' >> $1/utk.initrd
done