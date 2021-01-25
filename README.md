# Yudi's Programming Language

Yudi's Programming Language (YCPL) is a new programming language that enforces memory safety and type safety. The specification of the language is defined under `spec/specs.pdf`. This branch builds an interpreter that interprets the program.

## TODO-List

This project is still under development and not all function modules defined in specification coherce with the implementation. Below is the todo-list of the project.

- [x] Compiler front-end.
- [x] Implementation for basic arithmetics and control flow.
    - [ ] Implementation for `deepcopy`.
- [x] Class definition. (require tests)
- [ ] Union defintion (support for pattern-matching with tagged union). (ip)
- [ ] Support for Standard Library.
- [ ] Generics.
- [x] Simple error message.
- [ ] Rework error message format (err.cpp / err.hpp)
- [x] Memory management. (require tests)

## File and Directory Layout

- `spec/`: Directory that includes specification.
    - `specs.tex`: Source LaTeX code for specification.
    - `specs.pdf`: Compiled readable speicification file.
- `src/`: Directory that includes source code.
- `sample/`: Directory that includes sample programs.
    - `factorial.yc`: factorial from 1 to 10.
    - `cast.yc`: conversion between basic types.
    - `copy_move_deep.yc`: illustration difference between copy, move, and deepcopy.
    - `union.yc`: demo of tagged union.
- `input.yc`: Sample program used for debugging.
- `Makefile`
- `LICENSE`
- `README.md`: Documentation.

## Installation

The interpreter requires any form of a C++ compiler. It has been tested under WSL, Arch Linux and Mac OS.

To compile the project

> make

To test all sample programs under `sample/`

> make test
