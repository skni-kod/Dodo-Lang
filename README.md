# Dodo-Lang

## General info:
The goal of this project is to create a custom programming language compiler written in C++.

## Technologies:
- C++
- GNU Assembler
- GNU Linker
- assembler support will expand in the future

## About dodo-lang:
The language itself is to be statically typed, have objects and possibly bindings for more advanced features such as graphics. As of now it is being developed for x86-64 linux, with plans for it to work at least on 32 and 64 bit x86 linux and windows, arm is possible too. There will be no need for header files or specific definition order. The compilation order is as follows: .dodo file -- dodo-lang compiler -> .s file -- gnu as -> .o file -- gnu ld -> executable.

#### [Documentation can be found here](./Dodo-lang/documentation/Index.md)


The project is being developed as a SKNI "KOD" project.


## Currently achieved goals:
- custom type declaration
- fully functional functions with support for value operations
- complex mathematical expressions parsing
- variable mutability/immutability
- conditional statements
- linear analysis of variable lifetimes and usage
- regNumber variable locations in registers and on stack
- simulated processor memory in code generator
- a rather high degree of optimization, safe for precalculated constant expressions

## Currently in works:
- pointers and global variables

## Short term plans:
- floating point value support
- arrays
- system calls and/or direct assembly injection

## Long term goals:
- preprocessor allowing for different code on different platforms
- C ABI function calls
- structures with methods
- unions and byte fields
