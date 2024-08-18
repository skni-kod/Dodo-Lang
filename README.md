# Dodo-Lang

## General info:
The goal of this project is to create a simple, C++ style programming language compiler written in C++.

## Technologies:
- C++
- GNU Assembler
- GNU Linker

## About dodo-lang:
The language itself is to be statically typed, have objects and possibly bindings for more advanced features such as graphics. As of now it is being developed for x86-64 linux, with plans for it to work at least on 32 and 64 bit x86 linux and windows, arm is possible too. There will be no need for header files or specific definition order. The compilation order is as follows: .dodo file -- dodo-lang compiler -> .s file -- gnu as -> .o file -- gnu ld -> executable.

Documentation will be added in the future when the projects achieves any real functionality


The project is being developed as a SKNI "KOD" project.


## Currenty achieved goals:
- base types parsing
- very simple function parsing
- compilation chain to generate an executable