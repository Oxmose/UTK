#!/bin/bash

echo -e "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
echo -e "\e[1m\e[34m| Creating init ram disk for architecture x86_i386\e[22m\e[39m"
echo -e "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

# Create tar file
echo "Creating TAR with files:" 
curr_dir=$(pwd)
cd $1/../Config/arch/x86_i386/initrd/
tar --sort=name -cvf $curr_dir/$1/initrd.tar *
cd $curr_dir

filesize=$(wc -c < "$1/initrd.tar")

# Add header size
filesize=$(( $filesize + 512))

echo -e "\nInitrd size: $filesize"

# Create master block 
echo -n "UTKINIRD" > $1/utk.initrd              # Magic
filesize_hex=''
while [ $filesize -gt 0 ]
do
    nible=$(( $filesize % 256 ))
    new_str=$(echo "obase=16; $nible" | bc)
    if [ $nible -lt 16 ];
    then
        new_str=$(echo "0$new_str")
    fi
    filesize_hex=$(echo "$filesize_hex\x$new_str")
    filesize=$(( $filesize / 256 ))
done
while [ ${#filesize_hex} -lt 32 ]
do 
    filesize_hex=$(echo "$filesize_hex\x00")
done
echo -n -e $filesize_hex >> $1/utk.initrd         # Size
for (( i=0; i<62; i++ ))                       # Free space
do  
    echo -n -e '\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE' >> $1/utk.initrd
done

cat $1/initrd.tar >> $1/utk.initrd
rm $1/initrd.tar