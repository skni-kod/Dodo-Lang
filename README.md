# Dodo Lang

### The compiler nobody asked for!

## Technologies:
- C++
- CMake

## About Dodo Lang:
It's yet another statically typed language, though it has/will have some features I hope will make it unique enough. There are compound and simple types, but there is no separation between them, as such primitive types can have all the features of objects (with the C++ struct/class approach mind you, not as in object in C#/Java).

#### [Documentation can be found here (Outdated)](./Dodo-lang/documentation/Index.md)

The project is being developed as a SKNI "KOD" project.

## Currently nearly the entire project is being rewritten with those goals:
- complete rework of type system,
- compound types with methods and operator overloads,
- removing string dependency in later parts of the compilation,
- complete replacement of old lexer,
- support for floating point values.

## Current progress of rewriting:
- lexer - ✔
- parser - ✔
- bytecode generator - halfway done
- register/stack analyser - ⛌
- assembly generator - ⛌