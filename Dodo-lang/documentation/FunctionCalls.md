## Function calls

Function calls are performed similarly to other languages.

### Calling convention

Passing of arguments is done differently than most modern languages and is a subject to future changes if such a need arises. Arguments are passed in their order from function prototype, with the first one always starting after an offset aligned to 16. Then after all the arguments are added the positive offset from function's base pointer is calculated, so there is no need to move the values into the heap after the function.

### Value return

Value is returned in the register %r/e/ax/l with the appropriate size used for given return type

### Syntax

```
// Functions can be called as is like
Function1();

// Arguments can be passed as constants, variables or more complex expressions like
Function2(3, var4, var5 + 6);

// Value from function can be returned either as a part of an expression or by itself
mut i32 var7 = Function8();
var7 += Function9(10 * 11, 12) / 13;
```

### [Return to index](./Index.md)