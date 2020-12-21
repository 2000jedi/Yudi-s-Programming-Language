# Yudi's Programming Language

Yudi's Programming Language (YCPL) is a new programming language that enforces memory safety and type safety. The specification of the language is defined under `spec/specs.pdf`. This branch builds an interpreter that interprets the program.

This project is still under development and not all function modules defined in specification coherce with the implementation. Below is the list of things undergoing construction.

- [x] Scanner, Parser, and AST constructor.
- [x] Implementation for basic arithmetics and variable definition.
- [x] Global Variables.
- [x] Control-Flow definition including loops and functions.
- [x] Basic `print` function for debugging.
- [ ] Class definition. `TODO(yyang): debug`
- [ ] Union defintion (support for pattern-matching with tagged union).
- [ ] Support for Standard Library.
- [x] Rework `NameSpace`.
- [ ] Generics.
- [ ] Better error messages.

## File and Directory Layout

- `spec/`: Directory that includes specification.
    - `specs.tex`: Source LaTeX code for specification.
    - `specs.pdf`: Compiled readable speicification file.
- `src/`: Directory that includes source code for interpreter.
- `input.yc`: Sample ycpl program.
- `Makefile`
- `LICENSE`
- `README.md`: Documentation.

## Installation

The interpreter requires any form of a C++ compiler, including `gcc` and `llvm`. It has been tested under several OS.

- [x] Arch Linux
- [x] Mac OS (under development)
- [x] Ubuntu
- [ ] Windows (it will never be supported)

To compile the project

> make
