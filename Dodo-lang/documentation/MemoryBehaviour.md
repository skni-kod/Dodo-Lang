## Memory behaviour

During assembly generation Dodo-Lang compiler uses a processor memory simulation to track the contents of the registers. This approach allows for a significant reduction in outputted instructions.

### Linear analysis

After conversion of the parsed code to an intermediate bytecode, linear analysis takes place. 

Variables have their assignment number and instance numbers assigned, this allows for tracking where a given value needs to be discarded and allows for support of multiple variables with the same name in the same function. When a variable is needed, but not using its main type, a converted variable is created and stored, if the same converted value needs to be used multiple times, it keeps existing. After a new assignment all converted values and old main values are discarded.

In its new and more efficient form it takes all of a variable's main type assignments and groups them together. Converted variable instances are treated separately. After this the values are sorted by their usage, with the most used being first. After that it's calculated which variables are most frequently used, those are assigned to allowed registers and the less used ones are assigned to stack if needed. Grouped variables remain in the same place. There can be multiple groups/converted variables assigned to one register/stack location if their lifetimes don't collide.

### Content simulation

During bytecode conversion to assembly the memory is simulated and variables whose lifetime expired are removed to clear space. Also, when a variable is created not in it's assigned spot its value is not moved if it's not required. This allows to limit moves in some cases, also this allows for use of instructions with a very specific register requirement efficiently. Before things like jumps, labels and similar values are returned to their locations to prevent values being assigned from wrong sources.

### [Return to index](./Index.md)