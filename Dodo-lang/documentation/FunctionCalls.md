## Function calls

Function calls are performed similarly to other languages, with differences in value preservation.

### Calling convention

Passing of arguments in x86-64 is done according to System V AMD ABI. As of now only integer and pointer arguments are implemented. The difference is the approach to preservation of register values. Instead of the caller being responsible for it, the callee is required to save the state on any non argument and if the function returns a value, register A is also overwritten, the saved values are returned to the non argument and non value return registers.
Right now, only register passing in supported but stack passing will be added at a later time since 6 arguments is plenty for now.

### Value return

Value is returned in the register %r/e/ax/l with the appropriate size used for given return type, the remaining content of the register is unknown, so for example when return value is sized 4 bytes, the first 4 bits are the value, but the last 4 are unknown and could be zeroes or something else.

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