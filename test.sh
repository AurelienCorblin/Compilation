#!/bin/bash

ok=0
cpt=0

for file in test/good/*; do
    ./bin/tpcc < "$file" 
    if [ $? -eq 0 ]; then
        echo "Test passed: $file"
        ok=$((ok + 1))
    else
        echo "Test failed: $file"
    fi
    cpt=$((cpt + 1))
done

echo " "

for file in test/syn-err/*; do
    ./bin/tpcc < "$file" 2> /dev/null
    if [ $? -eq 1 ]; then
        echo "Test passed: $file"
        ok=$((ok + 1))
    else
        echo "Test failed: $file" 
    fi
    cpt=$((cpt + 1))
done

echo " "

for file in test/sem-err/*; do
    ./bin/tpcc < "$file" 2> /dev/null
    if [ $? -eq 2 ]; then
        echo "Test passed: $file"
        ok=$((ok + 1))
    else
        echo "Test failed: $file" 
    fi
    cpt=$((cpt + 1))
done

echo " "

for file in test/warn/*; do
    ./bin/tpcc < "$file" 
    if [ $? -eq 0 ]; then
        echo "Test passed: $file"
        ok=$((ok + 1))
    else
        echo "Test failed: $file" 
    fi
    cpt=$((cpt + 1))
done

echo "Result: $ok / $cpt"
