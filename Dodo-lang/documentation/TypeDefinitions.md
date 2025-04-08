## Type definitions

The programmer has the ability to define any custom basic type. The long term goal is to allow for custom behaviour of given type, allowing it to interact with others in unique ways, for example behaving as a signed or unsigned number depending on context.

### Type value types

There are 3 possible types of values. They are limited to certain fixed sizes that correspond with register and instruction sizes with plans to add other size support for arrays and such in the future. The types are as follows:
- UNSIGNED_INTEGER
- SIGNED_INTEGER
- FLOATING_POINT

### Syntax

```
// use the keyword type with type identifier and size
// there will be an optional bracket with methods in the future
type MyType : UNSIGNED_INTEGER(8);
```

### [Return to index](./Index.md)