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
- type declaration
- functions with various instructions and value returns
- addition, subtraction, multiplication and division operations with more easily addable
- complex mathematical expressions parsing
- variable storage on stack
- variable mutability/immutability and value modification
- function calls with arguments and value returns
- signed and unsigned integers operations and conversions
- conditional statements
- while and for loops
- an optimized assembly generator with lineara analysis and memory content simulation

## Short term plans:

- floating point values
- pointers and arrays
- syscalls and/or direct assembly injection
- macro like system
