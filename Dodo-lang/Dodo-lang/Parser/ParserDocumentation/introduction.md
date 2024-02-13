# Welcome to Dodo-lang parser documentation!

### What's that?

The parser is arguably the most important part of dodo-lang (to be updated if this changes), responsible for changing raw token input from parser into a logical whole that is then translated into assembly (or machine code if we feel ambitious). It works by performing many passes with tokens given to it by a coroutine and performing next steps, that allow for another one.

### Parts
- [type parser](types.md)
- [object parser](objects.md)
- function parser

### Output

This element outputs a nice struct with nested members containing the whole program's logic that should be easy to translate into the actual executable.