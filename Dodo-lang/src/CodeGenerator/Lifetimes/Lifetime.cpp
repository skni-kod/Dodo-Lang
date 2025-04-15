#include "Lifetime.hpp"

void CalculateLifetimes(BytecodeContext& context) {

    // resetting global variable use counters
    for (auto& n : globalVariableObjects) {
        n.uses = 0;
        n.firstUse = 0;
        n.lastUse = 0;
        n.assignedOffset = 0;
        n.isPointedTo = false;
    }

    // now going through all the instructions and updating lifetimes accordingly
    for (uint64_t n = 0; n < context.codes.size(); n++) {
        auto& current = context.codes[n];

        // if it's a definition skip it since it doesn't actually mean it's used
        if (current.type == Bytecode::Define) continue;

        if (current.op1Location == Location::Variable) context.getVariableObject(current.op1()).use(n);
        if (current.op2Location == Location::Variable) context.getVariableObject(current.op2()).use(n);
        if (current.type == Bytecode::Argument and
            current.op3Location == Location::Variable) context.getVariableObject(current.op3()).use(n);
    }
}