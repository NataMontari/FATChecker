#!/bin/bash
for i in {1..100}; do
    cp valid_image.img test_image.img
    dd if=/dev/urandom of=test_image.img bs=512 count=1 seek=$((RANDOM % 100)) conv=notrunc
    ./analyzer test_image.img >> fuzz_log.txt 2>&1
    if [ $? -ne 0 ]; then
        echo "Crash detected on iteration $i" >> fuzz_log.txt
        cp test_image.img crash_$i.img
    fi
done
