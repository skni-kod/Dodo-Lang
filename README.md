# Dodo Lang

### The compiler nobody asked for!

## Technologies:
- C++ 23
- CMake

## About Dodo Lang:
It's yet another statically typed language, though it has/will have some features I hope will make it unique enough. There are compound and simple types, but there is no separation between them, as such primitive types can have all the features of objects (with the C++ struct/class approach mind you, not as in object in C#/Java).

#### [Documentation TBA]

The project is being developed as a SKNI "KOD" project. The entire codebase is 100% organic.

## Most of the project has been rewritten (again) achieving following goals:
- complete rework of type system to be more flexible,
- complex and "simple" types with methods and operator overloads,
- removing string dependency in later parts of the compilation,
- complete replacement of old lexer,
- support for floating point values,
- performance and memory usage improvements,
- better optimisation of resulting code.

## Current progress of rewriting:
- lexer - virtually done:
  - minor bugfixes
- parser - virtually done
  - minor bugfixes only
- bytecode generator - mostly done:
  - increment/decrement don't work
  - default values for types don't work
  - complex types don't have default operators
  - global variables are not supported
  - supports array indexing for both r- and l-values
  - supports braced list initialisation for arrays
  - has a relatively nicely working expected vs actual type based type checking
- assembly generator - mostly done
  - basic x86-64 instructions have been defined and are being implemented
  - can output instruction with operands that are defined
  - can convert types
  - can do mostly correct x86-64 unix style calls and syscalls, except for large complex types and float only complex types
  - can do partially loops and conditional statements with memory state preservation
  - moving values and conversions between types largely done
  - can do offsets of registers for indexing
  - still needs a for lot of instructions to be added but it's just a lot of copy paste