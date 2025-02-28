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

assert 42 "{43-1;}"
assert 0 "{12-3-5-4;}" #left-assoc
assert 0 "{  12-3- 5- 4 ;}" # space
assert 12 "{3*4;}" #mul
assert 1 "{3/2;}" #div
assert 2 "{3-(2- 1);}" #paren
assert 5 "{1+2*3-4/2;}" #priority
assert 4 "{((1-1+3)/(2-1)-3)*2+4;}" #complex
assert 2 "{+2;}" #unary
assert 3 "{--+3;}"
assert 6 "{3++3;}"
assert 1 "{3+(-2);}" 
assert 1 "{2<3;}"
assert 0 '{0==1;}'
assert 1 '{42==42;}'
assert 1 '{0!=1;}'
assert 0 '{42!=42;}'

assert 1 '{0<1;}'
assert 0 '{1<1;}'
assert 0 '{2<1;}'
assert 1 '{0<=1;}'
assert 1 '{1<=1;}'
assert 0 '{2<=1;}'

assert 1 '{1>0;}'
assert 0 '{1>1;}'
assert 0 '{1>2;}'
assert 1 '{1>=0;}'
assert 1 '{1>=1;}'
assert 0 '{1>=2;}'

assert 3 '{1;2;3;}'
assert 3 '{;;3;}'

assert 3 '{a=3; a;}'
assert 8 '{a=3; z=5; a+z;}'
assert 6 '{a=b=3; a+b;}'

assert 3 '{ab=3; ab;}'
assert 8 '{_agoodname=3; z1=5; _agoodname+z1;}'
assert 6 '{a1=b1=3; a1+b1;}'

assert 3 '{foo=3; return foo;}'
assert 8 '{foo123=3; bar=5;{;;}; return foo123+bar;}'

assert 1 '{{return 1;} 2; 3;}'
assert 2 '{1; return 2; 3;}'
assert 3 '{1; 2; return 3;}'

assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1==1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'
assert 2 '{ if (1) { a=1; } else { a=2; }; if (a==1) {a=2;}; return a; }'

echo OK