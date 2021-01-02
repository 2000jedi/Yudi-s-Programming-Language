# Yudi's Programming Language

Yudi's Programming Language (YCPL) is a new programming language that enforces memory safety and type safety. The specification of the language is defined under `spec/specs.pdf`. This branch builds an interpreter that interprets the program.

This project is still under development and not all function modules defined in specification coherce with the implementation. Below is the list of things undergoing construction.

- [x] Rework Scanner.
- [ ] Rework Parser. (IP)
- [x] Implementation for basic arithmetics and variable definition.
- [x] Global Variables.
- [x] Control-Flow definition including loops and functions.
- [x] Basic `print` function for debugging.
- [x] Class definition. `TODO(yyang): debug`
- [ ] Union defintion (support for pattern-matching with tagged union).
- [ ] Support for Standard Library.
- [x] Rework `NameSpace`.
- [ ] Generics.
- [ ] Better error messages.

## File and Directory Layout

- `spec/`: Directory that includes specification.
    - `specs.tex`: Source LaTeX code for specification.
    - `specs.pdf`: Compiled readable speicification file.
- `src/`: Directory that includes source code.
- `sample/`: Directory that includes sample programs.
    - `factorial.yc`: factorial from 1 to 10.
    - `cast.yc`: conversion between basic types.
- `input.yc`: Sample ycpl program used for debugging.
- `Makefile`
- `LICENSE`
- `README.md`: Documentation.

## Installation

The interpreter requires any form of a C++ compiler. It has been tested under Linux and Mac OS.

To compile the project

> make

To test all sample programs under `sample/`

> make test
