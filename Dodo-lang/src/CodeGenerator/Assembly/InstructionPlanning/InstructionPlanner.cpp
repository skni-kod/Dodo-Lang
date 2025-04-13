#include <GenerateCode.hpp>

#include "InstructionPlanningInternal.hpp"
#include "X86_64.hpp"

// target resolution functions
std::vector <AsmInstruction> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc) {
    switch (Options::targetArchitecture) {
        case Options::TargetArchitecture::x86_64:
            return x86_64::AddConversionsToMove(move, context, proc);
        default:
            CodeGeneratorError("Internal: invalid architecture in move conversion!");
    }
    return {};
}

AsmOpDefinition& GetOpDefinition(AsmInstructionVariant& selected, uint8_t number) {
    switch (number) {
        case 1: return selected.op1;
        case 2: return selected.op2;
        case 3: return selected.op3;
        case 4: return selected.op4;
    }
    CodeGeneratorError("Internal: invalid op number!");
    return selected.op1;
}

bool IsRegisterAllowed(std::vector <RegisterRange>& allowedRegisters, uint8_t opNumber, uint8_t registerNumber) {
    for (auto& n : allowedRegisters) {
        if ((opNumber == 1 and n.canBeOp1) or
            (opNumber == 2 and n.canBeOp2) or
            (opNumber == 3 and n.canBeOp3) or
            (opNumber == 4 and n.canBeOp4)) {
            if (n.first <= registerNumber and n.last >= registerNumber) return true;
        }
    }
    return false;
}

// This function is responsible for resolving instruction definition variants,
// planning moves, conversions and passing off the tasks to platform dependent functions
// this file is to be platform-agnostic to make introducing other targets easier
void ExecuteInstruction(BytecodeContext& context, Processor& processor, AsmInstructionInfo& instruction, std::vector<AsmInstruction>& instructions, uint32_t index) {

    // first off choosing the best candidate for the instruction if there are multiple
    if (instruction.variants.size() == 1) {
        instruction.selected = &instruction.variants[0];
    }
    else {
        // this takes the variants and calculates the estimated cost of performing the instruction
        // by doing this the optimal operand combination can be found and used later
        uint16_t minCost = 0xFFFF;
        instruction.selected = nullptr;
        // different costs for different actions to make it select faster of possible instructions
        #define CONVERSION_COST 9
        #define MOVE_COST 6
        #define MEMORY_COST 6
        #define STACK_COST 3
        #define REGISTER_COST 1
        #define IMMEDIATE_COST 1
        for (auto& n : instruction.variants) {
            // first off let's prepare the variable
            uint16_t cost = 0;

            if (n.minimumVersion > Options::architectureVersion) continue;

            bool isValid = true;
            for (auto& thing : n.resultsAndInputs) {
                if (thing.isInput) {
                    if (thing.isFixedLocation) {
                        if (processor.getContentRef(thing.fixedLocation) != thing.value) cost += MOVE_COST;
                        if (thing.value.type != thing.fixedLocation.type) cost += CONVERSION_COST;
                        if (thing.value.size != thing.fixedLocation.size) cost += CONVERSION_COST;
                    }
                    else {
                        if (thing.value.op == Location::Variable) thing.value = processor.getLocation(thing.value);
                        auto& def = GetOpDefinition(n, thing.operandNumber);
                        if (not def.isInput) continue;
                        if (def.opType == Location::imm and thing.value.op != Location::imm) {
                            isValid = false;
                            continue;
                        }
                       if (def.opType == Location::imm) {
                            cost += IMMEDIATE_COST;
                            continue;
                       }
                       if (def.opType != thing.value.op) cost += MOVE_COST;
                       if (def.sizeMax < thing.value.size or def.sizeMin > thing.value.size) cost += CONVERSION_COST;
                       if (def.opType != thing.value.op) cost += MOVE_COST;
                       if (def.opType == Location::reg) {
                           if (thing.value.op == Location::reg) {
                               if (IsRegisterAllowed(n.allowedRegisters, thing.operandNumber, thing.value.value.reg)) cost += REGISTER_COST;
                               // moving between different types of registers seems to be costly, for example x86-64 GP register to an XMM
                               else cost += REGISTER_COST + MOVE_COST + CONVERSION_COST;
                           }
                           else if (thing.value.op == Location::sta) cost += STACK_COST;
                           else cost += MEMORY_COST;
                       }
                    }
                }
            }

            if (isValid and cost < minCost) {
                minCost = cost;
                instruction.selected = &n;
            }


        }
        if (instruction.selected == nullptr) CodeGeneratorError("Internal: could not find a viable instruction variant!");
    }

    // after that all the needed moves need to be prepared
    std::vector <MoveInfo> moves, results;
    auto& selected = *instruction.selected;

    // first off we need to resolve inputs and outputs
    AsmOperand ops[4];

    // let's go through input/result table and see what is used
    for (auto& n : selected.resultsAndInputs) {
        if (n.isInput) {
            // if it's a fixed location to set then we move the value there
            if (n.isFixedLocation) moves.emplace_back(n.value, n.fixedLocation);
            else moves.emplace_back(n.value, AsmOperand(Location::op, n.value.type, false, n.value.size, n.operandNumber));
        }
        else {
            // outputs need to be moved to results
            if (n.isFixedLocation) results.emplace_back(n.value, n.fixedLocation);
            else results.emplace_back(n.value, AsmOperand(Location::op, n.value.type, false, n.value.size, n.operandNumber));
        }
    }

    // now that we have all the changes saved we need to resolve the operands
    for (auto& [source, target] : moves) {
        // if it's a variable get it's location
        if (source.op == Location::Variable) source = processor.getLocation(source);

        if (target.op == Location::op) {
            // if it's an operand we need to check if it is in the correct place
            AsmOperand& op = ops[target.value.ui - 1];
            AsmOpDefinition& def = GetOpDefinition(selected, target.value.ui);

            // now that we have references we can nicely check where this operand should be
            if (source.op == Location::imm) {
                if (def.opType == Location::imm) {
                    // if we just need to put an immediate as op1 then it's simple
                    target = op = source;
                }
                else CodeGeneratorError("Internal: unimplemented immediate to somewhere move!");
            }
            else if (source.op == Location::sta) {
                if (def.opType == Location::sta) {
                    // another simple case
                    target = op = source;
                }
                else CodeGeneratorError("Internal: unimplemented stack to somewhere move!");
            }
            else if (source.op == Location::reg) {
                if (def.opType == Location::reg) {
                    // registers are a more complex thing
                    if (IsRegisterAllowed(selected.allowedRegisters, target.value.ui, source.value.reg))
                        target = op = source;
                    else CodeGeneratorError("Internal: unimplemented register to valid register move!");
                }
                else CodeGeneratorError("Internal: unimplemented register to somewhere move!");
            }
            else CodeGeneratorError("Internal: unimplemented operand source case move!");

        }
    }

    // also checking for output operands here
    for (auto& n : results) {
        if (n.target.op == Location::op) {
            // TODO: add the case where operand in only an output
            n.target = ops[n.target.value.ui - 1];
        }
    }

    // now checking for things that might need to be moved in the locations
    // TODO: think this through
    for (auto& n : moves) {
        if (n.source == n.target) continue;
        auto content = processor.getContent(n.target, context);

        if (content.op == Location::None) continue;

        // when it gets here we know that there is something there
        if (content.op == Location::Variable) {
            auto& object = content.object(context);

            // that means it escaped clearing and can be removed safely
            if (object.lastUse < index) {
                object = {};
                continue;
            }

            // in that case the value is an operand or is used later
            if (object.lastUse >= index) {
                // TODO: add searching for uses in other moves

                // now checking if the value is in it's place

            }
        }

    }

    // now all the moves should be in the vectors and can be executed, though I probably forgot about something
    // let's get their actual instruction representations
    // the possibility of these moves needing to use occupied locations needs to be taken into consideration
    std::vector <std::vector <AsmInstruction>> actualMoves;

    for (auto& n : moves) {
        actualMoves.push_back(AddConvertionsToMove(n, context, processor));
    }

    // TODO: do something here

    for (auto& n : actualMoves) {
        for (auto& m : n) {
            instructions.push_back(m);
        }
    }

    // adding the instruction itself
    AsmInstruction ins;
    ins.code = selected.code;
    ins.op1 = ops[0];
    ins.op2 = ops[1];
    ins.op3 = ops[2];
    ins.op4 = ops[3];

    instructions.push_back(ins);

    for (auto& n : results) {
        processor.getContentRef(n.target) = n.source;
    }
}