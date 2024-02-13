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

Type and name are to be mandatory. The ":" operand indicates that this is a simple type without additional complexity.

###### Complex types

Complex types can contain custom behaviours such as different reactions to operators or additional checks at value assignments. This is not yet supported. For example you could want to create a type containing an integer value that is not a zero:

```
type NonZero {
    SIGNED_INTEGER(4); // this defines value held in type
    
    // constructors may be overloaded to have different
    NonZero(const SIGNED_INTEGER(4) value) {
        if (value == 0) {
            // this needs to be thought on, might be inefficient
            throw CONSTRUCTOR_EXCEPTION;
        } 
        DATA = value;
    }
    
    // like in rust unsafe means the programmer takes responsibility
    unsafe NonZero(const SIGNED_INTEGER(4) value) {
        DATA = value;
    }
    
    // copies from the same type use the default copy constructor
    
    ...
}
```

Only one value type can exist inside a type, for multiples there are more complex data structures.

###### Custom behaviour behaviour

Type with a custom behaviour have some quirks. When defining behaviour only data type and this type objects can be used. When a variable of size different than one in any of the definitions is used to set value or for operations it's shortened or expanded to size of the closest candidate. Literal size is calculated as smallest data type size that can fit it or can be defined explicitly.