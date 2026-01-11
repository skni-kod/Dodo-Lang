#include "Lifetime.hpp"

#include <stack>

#include "StaticAnalysis.hpp"

void CalculateLifetimes(Context& context) {
    // resetting values
    for (auto& n : context.localVariables)
        for (auto& m : n) {
            m.uses = 0;
            m.firstUse = 0;
            m.lastUse = 0;
            m.isPointedTo = false;
            m.isReservedForArray = false;
            m.isDestructible = false;
        }
    for (auto& m : context.temporaries) {
        m.uses = 0;
        m.firstUse = 0;
        m.lastUse = 0;
        m.isPointedTo = false;
        m.isReservedForArray = false;
        m.isDestructible = false;
    }

    std::stack<uint64_t> scopeLevels{};
    scopeLevels.push(0);
    uint64_t scopeLevelCounter = 1;

    struct LifetimeScopeLevel {
        uint64_t scopeLevel = 0;
        std::vector <VariableObject*> toExtend;
    };

    struct LifetimePointer {
        uint64_t scopeLevel = 0;
        VariableObject* toExtend = nullptr;
    };

    std::vector <LifetimeScopeLevel> scopes;
    std::vector <LifetimePointer> pointedTo;

    uint64_t scopeLevel = 0;
    // now going through all the instructions and updating lifetimes accordingly
    for (uint64_t n = 0; n < context.codes.size(); n++) {
        auto& current = context.codes[n];

        // if it's a definition, skip it since it doesn't mean it's used
        if (current.type == Bytecode::Define) continue;
        if (current.type == Bytecode::BeginScope) {
            scopeLevel++;
            scopes.emplace_back(scopeLevel);
            scopeLevels.push(scopeLevelCounter++);
        }
        else if (current.type == Bytecode::EndScope) {

            if (not scopes.empty() and scopeLevel == scopes.back().scopeLevel) {
                // in that case the extension ended
                for (auto& m : scopes.back().toExtend)
                    m->lastUse = n;

                scopes.pop_back();
            }
            scopeLevel--;
            for (int64_t k = 0; k < pointedTo.size(); k++)
                if (pointedTo[k].scopeLevel == scopeLevels.top()) {
                    pointedTo.erase(pointedTo.begin() + k);
                    k--;
                }
            scopeLevels.pop();
        }
        else {
            // extending lifetimes of all things that are pointed to
            for (auto& k : pointedTo) {
                k.toExtend->lastUse = n;
            }
        }

        // now the thing is, if a scope starts, then every variable defined outside that is used inside must have its life extended
        if (current.type != Bytecode::LoopLabel) {
            if (current.op1Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op1());
                obj.use(n);
                if (current.op1Value.variable.type != VariableLocation::Temporary) {
                    for (auto& m : scopes)
                        m.toExtend.push_back(&obj);
                    if (current.type == Bytecode::Address) {
                        // copied from down
                        if (n <= context.codes.size() - 3
                        and context.codes[n + 1].type == Bytecode::Argument
                        and context.codes[n + 1].op3Value.ui == context.codes[n].op3Value.ui
                        and context.codes[n + 2].type == Bytecode::Method
                        and context.codes[n + 2].op1Value.function->isDestructor)
                            continue;

                        // first off see if it's already there
                        bool found = false;
                        uint64_t level = current.op1Value.variable.level;
                        for (auto& k : pointedTo) {
                            if (k.scopeLevel == level) {
                                found = true;
                                break;
                            }
                        }

                        if (not found) {
                            pointedTo.emplace_back(level, &obj);
                        }
                    }
                }
            }
            if (current.op2Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op2());
                obj.use(n);
                if (current.op2Value.variable.type != VariableLocation::Temporary) for (auto& m : scopes)
                    m.toExtend.push_back(&obj);
            }
            if (current.type == Bytecode::Argument and current.op3Location == Location::Variable) {
                auto& obj = context.getVariableObject(current.op3());
                obj.use(n);
                if (current.op3Value.variable.type != VariableLocation::Temporary) for (auto& m : scopes)
                    m.toExtend.push_back(&obj);
            }
        }
    }

    // extending the lifetime of things that got their address taken to the end of function
    for (auto& n : context.codes)
        if (n.type == Bytecode::Address) {
            auto var = context.getVariableObject(n.op1());
            if (var.identifier != nullptr or var.isReservedForArray)
                var.lastUse = context.codes.size() - 1;
        }

    for (std::size_t n = 0; n < context.codes.size(); n++)
        if (context.codes[n].type == Bytecode::Address) {

            // not counting calls to destructors and constructors
            // TODO: add copy constructor check here when its instruction order is known
            // update above too
            if (n <= context.codes.size() - 3
                and context.codes[n + 1].type == Bytecode::Argument
                and context.codes[n + 1].op3Value.ui == context.codes[n].op3Value.ui
                and context.codes[n + 2].type == Bytecode::Method
                and context.codes[n + 2].op1Value.function->isDestructor)
                continue;

            auto var = context.getVariableObject(context.codes[n].op1());
            if (var.identifier != nullptr or var.isReservedForArray) {
                var.lastUse = context.codes.size() - 1;
                var.isPointedTo = true;
            }
        }
}

void AnalyseAndOptimize(Context& context) {

    // first off let's do the initial calculation
    CalculateLifetimes(context);

    // now run the static analysis
    RunStaticAnalysis(context);

    // and recalculate with modified bytecodes
    CalculateLifetimes(context);

    // and again
    RemoveUnusedInstructions(context);

    // and once again recalculate
    CalculateLifetimes(context);
}
