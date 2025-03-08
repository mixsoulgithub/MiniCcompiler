#!/bin/bash
# cat <<EOF | gcc -xc -c -o tmp2.o -
# int ret3() { return 3; }
# int ret5() { return 5; }
# EOF

assert(){
    expected="$1"
    input="$2"

    ./miniCcompiler "$input" > tmp.s || (echo "fail at the test : $input" && exit)
    gcc -static -o tmp tmp.s 
    ./tmp
    actual="$?"

    if [ "$actual" != "$expected" ]; then
        echo "$input == $expected expected, but got $actual"
        exit 1
    fi
}

# assert 0 "{-1+1;}"
# assert 42 "{43-1;}"
# assert 0 "{12-3-5-4;}" #left-assoc
# assert 0 "{  12-3- 5- 4 ;}" # space
# assert 12 "{3*4;}" #mul
# assert 1 "{3/2;}" #div
# assert 2 "{3-(2- 1);}" #paren
# assert 5 "{1+2*3-4/2;}" #priority
# assert 4 "{((1-1+3)/(2-1)-3)*2+4;}" #complex
# assert 2 "{+2;}" #unary
# assert 3 "{--+3;}"
# assert 6 "{3++3;}"
# assert 1 "{3+(-2);}" 
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
# assert 2 '{ return 2;}'
# assert 3 '{1; 2; return 3;}'

# assert 2 '{int a=2; return a;}'
# assert 2 '{int a=2, b=a; return b;}'

# assert 3 '{ if (0) return 2; }'
# assert 3 '{ if (1-1==1) return 2; return 3; }'
# assert 2 '{ if (1) return 2; return 3; }'
# assert 2 '{ if (2-1) return 2; return 3; }'
# assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
# assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'
# assert 2 '{ if (1) { a=1; } else { a=2; }; if (a==1) {a=2;}; return a; }'

# assert 55 '{ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
# assert 3 '{ for (;;) return 3; return 5; }'

# assert 10 '{ int i=0; while(i<10) i=i+1; return i; }'

# assert 3 '{ {1; {2;} return 3;} }'

# assert 10 '{ int i=0; while(i<10) i=i+1; return i; }'
# assert 55 '{ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

# assert 3 'int main(){ int x=3; return *&x; }'
# assert 3 'int main(){ int x=3; int *y=&x; int **z=&y; return **z; }'
# assert 5 'int main(){ int x=3; int y=5; return *(&x-1); }'
# assert 3 'int main(){ int x=3; int y=5; return *(&y+1); }'
# assert 5 'int main(){ int x=3; int y=5; return *(&x+(-1)); }'
# assert 5 'int main(){ int x=3; int *y=&x; *y=5; return x; }'
# assert 7 'int main(){ int x=3; int y=5; *(&x-1)=7; return y; }'
# assert 7 'int main(){ int x=3; int y=5; *(&y+2-1)=7; return x; }'
# assert 5 'int main(){ int x=3; return (&x+2)-&x+3; }'
# assert 8 'int main(){ int x, y; x=3; y=5; return x+y; }'
# assert 8 'int main(){ int x=3, y=5; return x+y; }'

# assert 3 '{ return ret3(); }'
# assert 5 'int ret5(){return 5;} int main(){ return ret5(); }'
assert 3 'int a(){int a=3; return a;} int main(){ int a=3;return a;}'
assert 5 'int* addr(){int a=5; return &a;} int main(){ return *addr(); }'
# assert 5 'int** addr(){int a=5; int* y=&a; return &y;} int main(){ return **addr(); }'
assert 5 'int* addr(){int a=5; return &a;} int main(){ return *addr(); }'
assert 6 'int a(){int a=5; return a+1;} int main(){ return a(); }'
assert 6 'int a(){int a=5; return a+1;} int main(){ int a; return a(); }'
assert 10 'int a(){int a=5; int b=10; if(a==5){int b=5;a=a+b;} return a;} int main(){ int a=1;if(a==1){return a();}}'
assert 1 'int main(){int a=1; for(int i=0;i<5;i=i+1){int a=a+i;} return a;}'
assert 2 'int main(){int a=1; while(a==1){return 2;} return 1;}'

echo OK