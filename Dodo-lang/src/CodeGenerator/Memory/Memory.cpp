#include "Memory.hpp"

// the simplest function, works only on integers,
// uses is multiplied since it's unlikely there would be more than 1 use per index leading to lots of 0'os
inline bool VariableSortFunctionSimple(VariableObject*& a, VariableObject*& b) {
    if ((a->uses * 255) / (a->lastUse - a->firstUse + 1) > (b->uses * 255) / (b->lastUse - b->firstUse + 1)) return true;
    return false;
}

void CalculateMemoryAssignments(Processor& proc, BytecodeContext& context) {

    // this will work similarly to the grouping linear analysis of the olden days

    // first off let's get a nice list of all the variables we'll calculate storing locations for
    // global variables are kept at their addresses

    context.activeLevels.clear();
    uint64_t amount = context.temporaries.size();
    for (auto& n : context.localVariables) {
        amount += n.size();
    }

    // now actually assigning stuff to the new vector
    std::vector <VariableObject*> variables;
    variables.reserve(amount);

    for (auto& n : context.temporaries) {
        if (n.uses) variables.push_back(&n);
    }

    for (auto& n : context.localVariables) {
        for (auto& m : n) {
            if (m.uses) variables.push_back(&m);
        }
    }

    // we also need to know which registers can even be used for this storage
    struct RegisterUsage {
        std::vector<std::pair<uint64_t, uint64_t>> uses;
        uint16_t number;
        explicit RegisterUsage(const uint16_t number) : number(number) {}
        bool canStore(const Processor& proc, VariableObject*& var) const {
            return proc.registers[number].canBeLongStored(*var);
        }
    };
    std::vector <RegisterUsage> registers;
    for (auto& n : proc.registers) {
        if (not n.isReservedRegister)
            registers.emplace_back(n.number);
    }

    // now we have a nice vector of unsorted variables, let's sort it
    std::sort(variables.begin(), variables.end(), VariableSortFunctionSimple);

    // now that it's sorted locations can be assigned
    // let's go in order and find out where things can be
    for (auto& n : variables) {
        // TODO: add non-primitive register storage
        if (not n->type->isPrimitive and not (n->meta.isReference + n->meta.pointerLevel)) n->location.location = Location::Stack;
        else n->location.location = Location::Register;

        // now let's go through the registers and see if this variable fits
        bool fit = false;
        for (auto& reg : registers) {
            if (reg.canStore(proc, n)) {
                fit = true;
                // now seeing if the variable can fit in the periods the register was unused
                for (auto& use : reg.uses) {
                    if ((use.first <= n->lastUse and use.first >= n->firstUse) or
                        (use.second <= n->lastUse and use.second >= n->firstUse) or
                        (use.first <= n->firstUse and use.second >= n->lastUse)) {
                        fit = false;
                        break;
                    }
                }

                if (fit) {
                    n->location.number = reg.number;
                    reg.uses.emplace_back(n->firstUse, n->lastUse);
                    break;
                }
            }
        }

        if (not fit) n->location.location = Location::Stack;

        // now that all the variables that could be in registers are there it's time to put things on stack
        // is there a need though?
        // at this point simulating the stack to see where to place the variables might be pointless,
        // and it might be better to do so during assembly generation
    }

}