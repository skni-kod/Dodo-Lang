## Function definitions

Dodo-lang centers around the use of functions. They can be defined only in global scope.

### Implemented instructions

As of now there are only a few possible instructions that can be used inside functions, but that pool will expand soon:
- variable declaration
- variable assignment
- function calls
- return statements

### Variable declaration

Local variables are stored on stack. They are by default immutable and require explicit marking to change that. When defining a variable programmer has the option to assign it's value. If no value is supplied the variable is given value of 0. The value can be passed in the same way as all standard expressions.

The stack offsets of variables are calculated at compile time. There are features in place to ensure efficient stack space usage such as packing variable into free slots between others and variable function call stack pointer offset. The stack is also often used when converting values to larger sizes, but this usage is temporary and ceased as soon as it's not required.

### Variable assignment

Values are assigned the same way as in declaration, however with the difference that the target must be mutable and there is the option to use double operators such as "+=" that during parsing are converted from "var <op>= <exp>" to var = "var <op> (<exp>)";

### Function calls

Function calls without any assignment ignore the return value and work the same otherwise.

### Return statements

They return the value from function or entire program. Values are calculated exactly the same as in variable declarations.

### Syntax

```
// Functions can be defined like this
i32 Function1() {

    // variables can be mutable or immutable
    let i32 var2 = 5;
    // changing immutable variables is illegal
    var2 += 5; <--- nope

    // to make a variable mutable add mut keyword after let
    let mut i32 var3 = 5;
    var3 += 5; <--- dope

    // function calls can be performed without catching
    Function4(var2, var3);
    // or their output can be used
    let var5 = Function4(var2, var3);
    
    // return statement ends the function
    return var5 * 21 / 37;
}

// functions can also take arguments 
// and be defined in any order without headers
i32 Function4(i32 var6, mut u32 var7) {
    // arguments are also mutable and immutable
    var7 += 42;
    return var6 + var7 / var6;
}

```

### [Return to index](./Index.md)