#include "Lifetime.hpp"

#include <stack>

void CalculateLifetimes(BytecodeContext& context) {

    // resetting global variable use counters
    for (auto& n : globalVariableObjects) {
        n.uses = 0;
        n.firstUse = 0;
        n.lastUse = 0;
        n.assignedOffset = 0;
        n.isPointedTo = false;
    }

    struct LoopLevel {
        uint64_t scopeLevel = 0;
        std::vector <VariableObject*> toExtend;
    };

    std::vector <LoopLevel> loops;

    uint64_t scopeLevel = 0;
    // now going through all the instructions and updating lifetimes accordingly
    for (uint64_t n = 0; n < context.codes.size(); n++) {
        auto& current = context.codes[n];

        // if it's a definition, skip it since it doesn't mean it's used
        if (current.type == Bytecode::Define) continue;
        else if (current.type == Bytecode::BeginScope) scopeLevel++;
        else if (current.type == Bytecode::EndScope) {
            scopeLevel--;
            if (not loops.empty() and scopeLevel == loops.back().scopeLevel) {
                // in that case the extension ended
                for (auto& m : loops.back().toExtend)
                    m->lastUse = n;

                loops.pop_back();
            }
        }

        // now the thing is, if a loop scope starts, then every variable defined outside that is used inside must have its life extended
        else if (current.type == Bytecode::LoopLabel) loops.emplace_back(scopeLevel);
        else {
            if (current.op1Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op1());
                obj.use(n);
                if (current.op1Value.variable.type != VariableLocation::Temporary) for (auto& m : loops) m.toExtend.push_back(&obj);
            }
            if (current.op2Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op2());
                obj.use(n);
                if (current.op2Value.variable.type != VariableLocation::Temporary) for (auto& m : loops) m.toExtend.push_back(&obj);
            }
            if (current.type == Bytecode::Argument and current.op3Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op3());
                obj.use(n);
                if (current.op3Value.variable.type != VariableLocation::Temporary) for (auto& m : loops) m.toExtend.push_back(&obj);
            }
        }


    }
}
