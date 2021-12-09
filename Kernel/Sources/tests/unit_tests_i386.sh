#!/bin/bash

function testcase() {
    entry=$1

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### Test $i/$total : $filename_up \e[39m"
    # Select the test
    sed -i "s/ $filename_up 0/ $filename_up 1/g" includes/test_bank.h
    # Execute the test
    {
    rm -f *.out
    cd ../../
    make target=x86_i386 TESTS=TRUE && (make target=x86_i386 qemu-test-mode | tee test.out)
    mv test.out Sources/tests/test.out
    cd Sources/tests
    } > /dev/null
    # Filter output
    grep "\[TESTMODE\]\|ERROR" test.out > filtered.out
    #Compare output
    diff -b filtered.out refs/general/$filename.valid >> /dev/null
    if (( $? != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$((error + 1))
        cat filtered.out
        echo "================="
        diff -b filtered.out refs/general/$filename.valid  
        mv filtered.out errors/$filename.error
    else
        echo -e "\e[92mPASSED\e[39m"
        success=$((success + 1))
    fi
    #Clean data
    rm *.out
    #Restore non testmode
    sed -i "s/ $filename_up 1/ $filename_up 0/g" includes/test_bank.h
}

function testcase_arch() {
    entry=$1

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### Test $i/$total : $filename_up \e[39m"
    # Select the test
    sed -i "s/ $filename_up 0/ $filename_up 1/g" includes/test_bank.h
    # Execute the test
    {
    rm -f *.out
    cd ../../
    make target=x86_i386 TESTS=TRUE && (make target=x86_i386 qemu-test-mode | tee test.out)
    mv test.out Sources/tests/test.out
    cd Sources/tests
    } > /dev/null
    # Filter output
    grep "\[TESTMODE\]\|ERROR" test.out > filtered.out
    #Compare output
    diff -b filtered.out refs/x86_i386/$filename.valid >> /dev/null
    if (( $? != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$((error + 1))
        cat filtered.out
        mv filtered.out errors/$filename.error
    else
        echo -e "\e[92mPASSED\e[39m"
        success=$((success + 1))
    fi
    #Clean data
    rm *.out
    #Restore non testmode
    sed -i "s/ $filename_up 1/ $filename_up 0/g" includes/test_bank.h
}

error=0
success=0
total=0
i=1

for entry in "./x86_i386/src/"*.c
do
    if [ $(basename -- "$entry") != "kill_qemu.c" ]; then
        total=$((total + 1))

        filename=$(basename -- "$entry")
        filename="${filename%.*}"
        filename_up=${filename^^}
        sed -i "s/ $filename_up 1/ $filename_up 0/g" includes/test_bank.h
    fi
done
for entry in "./general/src/"/*.c
do
    total=$((total + 1))

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}
    sed -i "s/ $filename_up 1/ $filename_up 0/g" includes/test_bank.h
done

make -C ../../ clean
mkdir -p errors

echo -e "\e[94m=== Arch specific tests\e[39m"
for entry in "./x86_i386/src/"/*.c
do
    if [ $(basename -- "$entry") != "kill_qemu.c" ]; then
        testcase_arch $entry

        i=$((i + 1))
    fi
done

echo -e "\e[94m=== Generic tests\e[39m"
for entry in "./general/src/"/*.c
do
    testcase $entry

    i=$((i + 1))
done

echo ""
echo -e "\e[94m################################### RESULTS ###################################\e[39m"
echo ""
if (( error != 0 ))
then
    echo -e "\e[31m $error ERRORS \e[39m"
    echo -e "\e[92m $success SUCCESS \e[39m"
    exit -1
else
    echo -e "\e[92m 0 ERROR \e[39m"
    echo -e "\e[92m $success SUCCESS \e[39m"
fi
