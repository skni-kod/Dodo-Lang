## System calls

System calls via numbers are supported.

### Calling convention

Argument passing is done fully via the System V AMD ABI, allowing for correct interfacing with external functions. This will soon be extended to general external function support.

### Value return

Value is returned in the register %r/e/ax/l with the appropriate size used for given return type.

### Syntax

```
syscall(<syscall number>, <arg1>, <arg2>, ... ,<argn>);
```

### [Return to index](./Index.md)