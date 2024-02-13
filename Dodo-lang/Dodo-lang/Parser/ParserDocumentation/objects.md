### Object parser

This part of the parser runs once the basic types have been established.

###### struct and class

For example in C++ struct and class are very similar but have some different behaviours. We do not strive to add needless complexity, struct and classes are basically the same with one difference: class members are private by default and struct members are public by default.

###### Inheritance

Yes, it will exist in the future.

###### Syntax

Objects are defines similarly to C++:

```
struct Structure {
    // public by default
    i32 val = 15;
    i8 val2 = 7;
    i24 val3 = 53;
    
    ...
}
```

Data spacing works similarly to C/C++. The non power of 2 divisible types exist to allow for byte precision data packing. Outside of structures they are expanded to nearest power of 2.