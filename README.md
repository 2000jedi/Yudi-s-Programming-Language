# Yudi's Programming Language

Yudi's Programming Language (YCPL) is a new programming language that enforces memory safety and type safety. The specification of the language is defined under `spec/specs.pdf`. Compiler for YCPL is constructed with C++ and LLVM.

This project is still under development and not all function modules defined in specification coherce with the implementation. Below is the list of things undergoing construction.

- [x] Scanner, Parser, and AST constructor.
- [x] Implementation for basic arithmetics and variable definition.
- [ ] Global Variables.
- [x] Control-Flow definition including loops and functions.
- [x] Basic `printf` function for debugging.
- [ ] Class definition.
- [ ] Enum defintion (support for pattern-matching with tagged union).
- [ ] Support for Standard Library.
- [ ] Generics.

## File and Directory Layout

- `spec/`: Directory that includes specification.
    - `specs.tex`: Source LaTeX code for specification.
    - `specs.pdf`: Compiled readable speicification file.
- `src/`: Directory that includes source code for compiler.
- `input.yc`: Sample ycpl program.
- `Makefile`
- `LICENSE`
- `README.md`: Project documentation file.

## Installation

The compiler requires `clang` and `LLVM` version 10 installed. It has been tested under several OS.

- [X] Arch Linux
- [ ] Mac OS (under development)
- [X] Ubuntu
- [ ] Windows (it will never be supported)

To compile the project

> make


