This project is based on chibicc. About half commits are learning commits. After those, compared with original project, I wrote scope management and function call by myself. Some other codes maybe differ with chibicc but do almost same thing. As a beginner, I try to write my own codes even if sometimes they may be unmature.

MiniCcompiler dynamiclly allocates addresses in frame for local variables, that means if there is a "int a=0;" in a for-loop, when for-loop ends, address of a will be reallocated to other possible variable. This makes smaller stack frame. 

MiniCcompiler push every function arguments to stack, rather than push from 7th argument according to System V AMD64 ABI. This makes funciton call easy to understand. 

In additional, I wrote shell file to automatically test C source code and compare with gcc's answer. Directory Negative_test contains tests that should not pass. Directory differ_tests has different behaviour in MiniCcompiler and gcc. File scope_test exactly shows how MiniCcompiler dynamiclly allocates addresses in frame for local variables.