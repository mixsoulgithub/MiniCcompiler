#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./miniCcompiler "$input" > tmp.s || exit
    gcc -static -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input == $actual"
    else
        echo "$input == $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "12-3-5-4"

echo OK