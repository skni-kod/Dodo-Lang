## Value conversions

When an expression is processed the size and type of the location it's assigned to later is used for conversions. So for example if a 4 byte variable is assigned a value of addition of 2 8 byte variables they are first converted  to the destination's type, after that down to 4 bytes and only then are they added and placed in their location.

Sizes are determined at every operation level, so that all the values are cast to highest size in the operation and then downsized after.

### Type conversion
As of now type conversion between signed and unsigned integers is a simple value copy, as such negative values will produce high unsigned values and vice versa.

This behaviour might be changed in the future;

### Size conversion
Size conversion can occur between any 1, 2, 4 or 8 byte values. For integer types reducing size is done by taking the appropriate amount of bytes from the right (for example taking %eax from value in %rax). Increasing the size is done differently depending on whether the integer is signed or not.

Unsigned integer values increase their size by putting $0 in second 1 byte register or doing an AND operation with a certain mask (for example 0x0000FFFF for 2 to 4 byte conversion) for values initially in registers. When a value is taken from the stack a $0 is placed into the target register and then value is copied to the correct smaller register from the stack.

Signed integer values are converted up by the use of "movs" to convert 1 and 2 byte values to 4 bytes and/or additional conversion instructions (like "cltq) to preserve the correct sign.

### [Return to index](./Index.md)