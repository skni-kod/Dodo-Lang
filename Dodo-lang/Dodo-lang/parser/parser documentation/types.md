### Type parser

This part of the parser is the first to run. It finds type definitions and marks them.

This is an example of a type declaration, subject to change:

```
type i32 : SIGNED_INTEGER(4);
```

Options are:
- SIGNED_INTEGER
- UNSIGNED_INTEGER
- FLOATING_POINT

These types determine the data types and sizes used in assembly and cannot be mixed. 

There are plans for adding custom behaviour to these types, but as of now it's not implemented yet, adding those would make these useful in any way.