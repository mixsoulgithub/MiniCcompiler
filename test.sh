#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./miniCcompiler "$input" > tmp.s || (echo "fail at the test : $input" && exit)
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

assert 0 "{-1+1;}"
# assert 42 "{43-1;}"
# assert 0 "{12-3-5-4;}" #left-assoc
# assert 0 "{  12-3- 5- 4 ;}" # space
# assert 12 "{3*4;}" #mul
# assert 1 "{3/2;}" #div
# assert 2 "{3-(2- 1);}" #paren
# assert 5 "{1+2*3-4/2;}" #priority
# assert 4 "{((1-1+3)/(2-1)-3)*2+4;}" #complex
# assert 2 "{+2;}" #unary
assert 3 "{--+3;}"
# assert 6 "{3++3;}"
assert 1 "{3+(-2);}" 
# assert 1 "{2<3;}"
# assert 0 '{0==1;}'
# assert 1 '{42==42;}'
# assert 1 '{0!=1;}'
# assert 0 '{42!=42;}'

# assert 1 '{0<1;}'
# assert 0 '{1<1;}'
# assert 0 '{2<1;}'
# assert 1 '{0<=1;}'
# assert 1 '{1<=1;}'
# assert 0 '{2<=1;}'

# assert 1 '{1>0;}'
# assert 0 '{1>1;}'
# assert 0 '{1>2;}'
# assert 1 '{1>=0;}'
# assert 1 '{1>=1;}'
# assert 0 '{1>=2;}'

# assert 3 '{1;2;3;}'
# assert 3 '{;;3;}'

# assert 3 '{a=3; a;}'
# assert 8 '{a=3; z=5; a+z;}'
# assert 6 '{a=b=3; a+b;}'

# assert 3 '{ab=3; ab;}'
# assert 8 '{_agoodname=3; z1=5; _agoodname+z1;}'
# assert 6 '{a1=b1=3; a1+b1;}'

# assert 3 '{foo=3; return foo;}'
# assert 8 '{foo123=3; bar=5;{;;}; return foo123+bar;}'

# assert 1 '{{return 1;} 2; 3;}'
assert 2 '{ return 2;}'
# assert 3 '{1; 2; return 3;}'

# assert 3 '{ if (0) return 2; }'
# assert 3 '{ if (1-1==1) return 2; return 3; }'
# assert 2 '{ if (1) return 2; return 3; }'
# assert 2 '{ if (2-1) return 2; return 3; }'
# assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
# assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'
# assert 2 '{ if (1) { a=1; } else { a=2; }; if (a==1) {a=2;}; return a; }'

# assert 55 '{ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
# assert 3 '{ for ( ; a<1 ; a=a+1) {return 3;} return 5; }'
# assert 89 '{ i=0;j1=0;j2=1; for (i=0; i<10; i=i+1) {tmp=j2;j2=j1+j2;j1=tmp;} return j2; }' #fibonacci
# assert 10 '{ i=0; while(i<10) { i=i+1; } return i; }'

# assert 3 '{ x=3; return *&x; }'
# assert 3 '{ x=3; y=&x; z=&y; return **z; }'
# assert 5 '{ x=3; y=5; return *(&x-1); }'
# assert 3 '{ x=3; y=5; return *(&y+1); }'
# assert 5 '{ x=3; y=5; return *(&x+-1); }'
# assert 5 '{ x=3; y=&x; *y=5; return x; }'
# assert 7 '{ x=3; y=5; *(&x-1)=7; return y; }'
assert 7 '{ x=3; y=5; *(&y+2-1)=7; return x; }'
assert 5 '{ x=3; return &x+2-&x+3; }'

echo OK