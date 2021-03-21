#!/bin/bash

echo -e "\e[1m\e[34m\n#-------------------------------------------------------------------------------\e[22m\e[39m"
echo -e "\e[1m\e[34m| Init build for target x86_i386\e[22m\e[39m"
echo -e "\e[1m\e[34m#-------------------------------------------------------------------------------\n\e[22m\e[39m"

cd "$(dirname "$0")"

diff settings.mk ../../../Sources/global/settings.mk &> /dev/null
if [ $? != 0 ]; then
    cp settings.mk ../../../Sources/global/
    echo -e "\e[1m\e[92m\nUpdated makefile settings\e[22m\e[39m"
fi

diff config.h ../../../Sources/global/config.h &> /dev/null
if [ $? != 0 ]; then
    cp config.h ../../../Sources/global/
    echo -e "\e[1m\e[92m\nUpdated configuration header\e[22m\e[39m"
fi

diff config.inc ../../../Sources/global/config.inc &> /dev/null
if [ $? != 0 ]; then
echo -e "\e[1m\e[92m\nUpdated assembly configuration header\e[22m\e[39m"
    cp config.inc ../../../Sources/global/
fi