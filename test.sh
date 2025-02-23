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

assert 42 "43-1"
assert 0 "12-3-5-4" #left-assoc
assert 0 "  12-3- 5- 4 " # space
assert 12 "3*4" #mul
assert 1 "3/2" #div
assert 2 "3-(2- 1)" #paren
assert 5 "1+2*3-4/2" #priority
assert 4 "((1-1+3)/(2-1)-3)*2+4" #complex
assert 2 "+2" #unary
assert 3 "--+3"
assert 6 "3++3"
assert 1 "3+(-2)" 
assert 1 "2<3"
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'

assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'

assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'

echo OK