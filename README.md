# ACC: A C Compiler

## Overview
- An implementation of C89 compiler
- The compiler itself is written in C89
- Self-hosting
- Compiles C source code to assembly language
- Uses platform's assembler and linker
- Uses platform's libc

## Features
- C Preprocessor
- Integers and floating point numbers
- Pointers and arrays
- Structs, unions and enumerators
- Variadic functions
- Function pointers
- Multiple diagnostic messages
- (*Not all of language features are supported yet*)

## Build
- `$ make`
    - Builds acc
- `$ make test`
    - Builds acc and runs test
- `$ make test_all`
    - Builds acc and runs test
    - Builds the second generation compiler using the first compiler
    - Builds the third generation compiler using the second compiler
    - Runs binary comparison between the second and the third compiler

## Platforms
- MacOS with clang

## License
- MIT License

## Reference
- [C11 Specification](https://port70.net/~nsz/c/c11/n1570.html)
- [ANSI C Yacc grammar](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html)
- [Dave Prosser's C Preprocessing Algorithm](https://www.spinellis.gr/blog/20060626/)
- [Crafting Interpreters](https://www.amazon.com/gp/product/B09BCCVLCL/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
- [Engineering a Compiler](https://www.amazon.com/gp/product/B00J5AS70G/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
- [Beginning x64 Assembly Programming](https://www.amazon.com/gp/product/B07ZVKM3CC/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
- [lcc](https://github.com/drh/lcc)
- [chibicc](https://github.com/rui314/chibicc)