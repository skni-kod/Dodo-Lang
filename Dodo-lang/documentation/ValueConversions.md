## Value conversions

When an expression is processed the size and type of the location it's assigned to later is used for conversions. So for example if a 4 byte variable is assigned a value of addition of 2 8 byte variables they are first converted  to the destination's type, after that down to 4 bytes and only then are they added and placed in their location.

Sizes are determined at every operation level, so that all the values are cast to the highest size in the operation and then downsized after.

### Type conversion
As of now type conversion between signed and unsigned integers is a simple value copy, as such negative values will produce high unsigned values and vice versa.
This behaviour might be changed in the future;

Floating point numbers are not supported yet.

### Size conversion
Size conversions between integers are very simple. If the source value is bigger than the target, part of the source is moved to the target, that can lead to value loss/sign changes so beware. 

When converting to a larger integer the source is sized up, if it's an unsigned integer it's upsized with zero extension and when signed it's upsized with leading 1/0 extension to keep its sign.

Floating point values are not yet supported.

### [Return to index](./Index.md)