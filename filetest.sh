#!/bin/bash

# search every .c file in ./test
for file in ./test/*.c; do
    # get filename without extension
    filename=$(basename -- "$file")
    filename="${filename%.*}"

    # compile and run with gcc.
    gcc -o gcc_output "$file"
    ./gcc_output
    gcc_retval=$?

    # compile with miniCcompiler and run
    ./miniCcompiler "$file" > tmp.s
    gcc -static -o miniC_output tmp.s
    ./miniC_output
    miniC_retval=$?

    # compare return value
    if [ "$gcc_retval" -ne "$miniC_retval" ]; then
        echo "Test failed for $file: gcc returned $gcc_retval, miniCcompiler returned $miniC_retval"
        exit 1
    else
        echo "Test passed for $file"
    fi

    # clear files
    rm -f gcc_output miniC_output tmp.s
done

echo "All tests passed"