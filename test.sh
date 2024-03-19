#!/bin/bash

ok=0
cpt=0

for file in test/good/*; do
    ./bin/tpcas < "$file"
    if [ $? -eq 0 ]; then
        echo "Test passed: $file : 0"
        ok=$((ok + 1))
    else
        echo "Test failed: $file : 1"
    fi
    cpt=$((cpt + 1))
done

echo " "

for file in test/syn-err/*; do
    ./bin/tpcas < "$file"
    if [ $? -eq 0 ]; then
        echo "Test failed: $file : 0"
    else
        echo "Test passed: $file : 1"
        ok=$((ok + 1))
    fi
    cpt=$((cpt + 1))
done

echo "Result: $ok / $cpt"
