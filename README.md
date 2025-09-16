# Dodo Lang

### The compiler nobody asked for!

## Technologies:
- C++
- CMake

## About Dodo Lang:
It's yet another statically typed language, though it has/will have some features I hope will make it unique enough. There are compound and simple types, but there is no separation between them, as such primitive types can have all the features of objects (with the C++ struct/class approach mind you, not as in object in C#/Java).

#### [Documentation TBA]

The project is being developed as a SKNI "KOD" project.

## Currently nearly the entire project is being rewritten with those goals:
- complete rework of type system,
- compound types with methods and operator overloads,
- removing string dependency in later parts of the compilation,
- complete replacement of old lexer,
- support for floating point values.

## Current progress of rewriting:
- lexer - virtually done:
- parser - virtually done
    - methods might need some work
- bytecode generator - mostly done:
  - increment/decrement don't work
  - default values for types don't work
  - complex types don't have default operators
  - global variables are not supported
  - supports array indexing for both r- and l-values
  - supports braced list initialisation for arrays
  - brackets work somewhat
- assembly generator - partially done
  - all basic x86-64 instructions have been defined and are being implemented
  - can output instruction with operands that are defined
  - can convert types
  - can do mostly correct x86-64 unix style calls and syscalls
  - can do loops and conditional statements with memory state preservation
  - moving values and conversions between types largely done
  - can do offsets of registers for indexing