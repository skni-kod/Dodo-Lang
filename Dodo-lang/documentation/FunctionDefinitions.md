## Function definitions

Dodo-lang centers around the use of functions. They can be defined only in global scope.

### Implemented instructions

As of now there are only a few possible instructions that can be used inside functions, but that pool will expand soon:
- variable declaration
- variable assignment
- function calls
- return statements
- mathematical expressions in all of them

### Variable declaration

Local variables have their designated spaces regNumber during the linear analysis. They are regNumber to registers starting at r8 in x86-64 and stack for excess variables. As of now, during the grouping analysis the variables with the highest frequency of use compared to their lifetime being regNumber to registers and the less accessed ones are put on the stack.

### Variable assignment

Values of the same type as the mainline variable replace it after the assignment. If the value needs to be converted from the mainline type it's not grouped in analysis and is stored in another place.

### Function calls

Function calls without any assignment ignore the return value and work the same otherwise.

### Return statements

They return the value from function or entire program. Values are calculated exactly the same as in variable declarations with the result put into the correct return register and then a jump to return label occurs.

### Syntax

```
// Functions can be defined like this
i32 Function1() {

    // variables can be mutable or immutable
    let i32 var2 = 5;
    // changing immutable variables is illegal
    var2 += 5; <--- nope

    // to make a variable mutable add mut keyword after let
    mut i32 var3 = 5;
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
    // but right now argument immutability is not respected in the generator 
    // and that will change
    var7 += 42;
    return var6 + var7 / var6;
}

```

### [Return to index](./Index.md)