#!/bin/bash

cd Kernel/Sources/tests
chmod +x ./unit_tests_i386.sh
./unit_tests_i386.sh
val=$?
cd ../..
exit $val