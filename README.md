# Dodo-Lang

#### The programming language nobody asked for!

## Technologies:
- C++
- CMake

## About dodo-lang:
The language itself is to be statically typed, have objects and possibly bindings for more advanced features such as graphics. As of now it is being developed for x86-64 linux, with plans for it to work at least on 32 and 64 bit x86 linux and windows, arm is possible too. There will be no need for header files or specific definition order. The compilation order is as follows: .dodo file -- dodo-lang compiler -> .s file -- gnu as -> .o file -- gnu ld -> executable.

#### [Documentation can be found here (WIP!)](./Dodo-lang/documentation/Index.md)

The project is being developed as a SKNI "KOD" project.

## Current branch goals:
- complete rework of type system
- structures
- removal of data kept in strings
- complete replacement of old lexer (done!)

## Currently achieved goals:
- custom type declaration
- fully functional functions with support for value operations
- complex mathematical expressions parsing
- variable mutability/immutability
- conditional statements
- linear analysis of variable lifetimes and usage
- assigned variable locations in registers and on stack
- simulated processor memory in code generator
- a rather high degree of optimization, safe for precalculated constant expressions
- global variables
- pointers to local and global values as well as strings (a "Hello world!" is possible!)
- syscalls

## Short term plans:
- C function calling
- floating point value support
- arrays used via indexes
- possibly direct assembly injection

## Long term goals:
- preprocessor allowing for different code on different targets
- fixed lexer
- structures with methods
- unions and byte fields
