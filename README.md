> [!NOTE]
> this language is being archived in favour of the vastly superior [deeprose3](https://github.com/omnomctr/deeprose3) version. 

# Deeprose-1
This is my first iteration of my Lisp programming language. Its based off of this great [book](https://www.buildyourownlisp.com/). This is the first iteration and as such is limited in many features.
It doesn't have macros, gc (or any pass-by-reference arguments), tail-end recursion, some somewhat insecure C code and a very small standard library.

# Installation 
If you want to install it, you'll need to put [mpc](https://github.com/orangeduck/mpc)'s mpc.h and mpc.c file into the repository, set a $DRLIBPATH for the path of the stdlib.deeprose, and install [editline](https://archlinux.org/packages/extra/x86_64/editline/).
