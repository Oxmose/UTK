#!/bin/bash

function testcase() {
    entry=$1

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### Test $i/$total : $filename_up \e[39m"
    # Select the test
    sed -i "s/$filename_up 0/$filename_up 1/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 0/TEST_MODE_ENABLED 1/g' ../Config/i386/config.h
    # Execute the test
    rm -f *.out
    cd ../
    make arch=i386 TESTS=TRUE && (make arch=i386 qemu-test-mode > test.out)
    mv test.out ./Tests/test.out
    
    cd Tests
    # Filter output
    grep "\[TESTMODE\]\|ERROR" test.out > filtered.out
    #Compare output
    diff filtered.out Refs/$filename.valid >> /dev/null
    if (( $? != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$((error + 1))
        cat filtered.out
        mv filtered.out $filename.error
    else
        echo -e "\e[92mPASSED\e[39m"
        success=$((success + 1))
    fi
    #Clean data
    rm *.out
    rm Tests/*.o
    #Restore non testmode
    sed -i "s/$filename_up 1/$filename_up 0/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 1/TEST_MODE_ENABLED 0/g' ../Config/i386/config.h
}

error=0
success=0
total=0
i=1

for entry in $1
do
    total=$((total + 1))

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}
    sed -i "s/$filename_up 1/$filename_up 0/g" Tests/test_bank.h
done

for entry in "./Tests"/$1
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
