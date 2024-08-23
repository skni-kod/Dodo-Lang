## Value conversions

When an expression is processed the size and type of the location it's assigned to later is used for conversions. So for example if a 4 byte variable is assigned a value of addition of 2 8 byte variables they are first converted  to the destination's type, after that down to 4 bytes and only then are they added and placed in their location.

### Type conversion
As of now type conversion between signed and unsigned integers is a simple value copy, as such negative values will produce high unsigned values and vice versa.

This behaviour might be changed in the future;