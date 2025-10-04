#include <GenerateCode.hpp>
#include <iostream>

#include "ErrorHandling.hpp"
#include "InstructionPlanningInternal.hpp"
#include "X86_64.hpp"
#include "X86_64Enums.hpp"

// target resolution functions
void AddConversionsToMove(MoveInfo& move, Context& context, std::vector<AsmInstruction>& instructions, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters, bool setContent) {
    switch (Options::targetArchitecture) {
        case Options::TargetArchitecture::x86_64:
            x86_64::AddConversionsToMove(move, context, instructions, contentToSet, forbiddenRegisters, setContent);
            break;
        default:
            CodeGeneratorError("Internal: invalid architecture in move conversion!");
    }
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

AsmOperand GetFreeRegister(Context& context, std::vector <RegisterRange>& allowedRegisters, uint8_t opNumber, std::vector<AsmOperand>* forbiddenRegisters) {
    for (auto& n : allowedRegisters) {
        if ((opNumber == 1 and n.canBeOp1) or
            (opNumber == 2 and n.canBeOp2) or
            (opNumber == 3 and n.canBeOp3) or
            (opNumber == 4 and n.canBeOp4)) {
            for (uint16_t m = n.first; m <= n.last; m++) {
                if (context.registers[m].content.op != Location::Variable) {
                    if (forbiddenRegisters != nullptr) {
                        bool isValid = true;
                        for (auto& k : *forbiddenRegisters) {
                            if (k.value.reg == m) {
                                isValid = false;
                            }
                        }
                        if (not isValid) continue;
                    }
                    return {Location::reg, Type::none, false, context.registers[m].size, m};
                }
            }
        }
    }
    CodeGeneratorError("Internal: could not find a free register!");
    return {};
}

// This function is responsible for resolving instruction definition variants,
// planning moves, conversions and passing off the tasks to platform dependent functions
// this file is to be platform-agnostic to make introducing other targets easier
void ExecuteInstruction(Context& context, AsmInstructionInfo& instruction, std::vector<AsmInstruction>& instructions, uint32_t index) {

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
                        if (context.getContentRef(thing.fixedLocation) != thing.value) cost += MOVE_COST;
                        if (thing.value.type != thing.fixedLocation.type) cost += CONVERSION_COST;
                        if (thing.value.size != thing.fixedLocation.size) cost += CONVERSION_COST;
                    }
                    else {

                        auto& def = GetOpDefinition(n, thing.operandNumber);
                        if (thing.value.op == Location::Variable) {
                            if (def.opType != Location::reg)
                                thing.value = context.getLocationStackBias((thing.value));
                            else
                                thing.value = context.getLocation(thing.value);
                        }
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
                               else cost += REGISTER_COST + MOVE_COST + CONVERSION_COST;
                           }
                           else if (thing.value.op == Location::sta) cost += STACK_COST;
                           else if (thing.value.op == Location::imm) cost += REGISTER_COST;
                           else cost += MEMORY_COST;
                       }
                       else if (def.opType == Location::sta && thing.value.op != Location::sta) {
                           if (thing.value.op == Location::reg) cost += REGISTER_COST;
                           else if (thing.value.op == Location::imm) cost += MOVE_COST;
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

    std::vector <AsmOperand> forbiddenRegisters{};
    for (auto& n : moves)
        if (n.target.is(Location::reg))
            forbiddenRegisters.push_back(n.target);

    // now that we have all the changes saved we need to resolve the operands
    for (uint16_t k = 0; k < moves.size(); k++) {
        auto& source = moves[k].source;
        auto& target = moves[k].target;
        // if it's a variable get it's location
        if (source.op == Location::Variable)
            source = context.getLocation(source);

        if (target.op == Location::op) {
            // if it's an operand we need to check if it is in the correct place
            AsmOperand& op = ops[target.value.ui - 1];
            AsmOpDefinition& def = GetOpDefinition(selected, target.value.ui);

            // if the input operand is also used as output we might need to copy it
            if (def.isOutput and source.anyOf(Location::var, Location::sta, Location::mem, Location::off))
                source = source.moveAwayOrGetNewLocation(context, instructions, index);

            // now that we have references we can nicely check where this operand should be
            if (source.op == Location::imm) {
                if (def.opType == Location::imm) {
                    // if we just need to put an immediate as op1, then it's straightforward
                    target = op = source;
                }
                else if (def.opType == Location::sta) {
                    // TODO: what about size?
                    target = op = source.copyTo(Location::sta, context.pushStackTemp(def.sizeMax, def.sizeMax).value.offset);
                }
                else if (def.opType == Location::reg) {
                    // TODO: what about size?
                    // TODO: forbidden registers
                    target = op = source.copyTo(Location::reg, GetFreeRegister(context, selected.allowedRegisters, target.value.ui, nullptr).value.reg);
                }
                else CodeGeneratorError("Internal: unimplemented immediate to somewhere move!");
            }
            else if (source.op == Location::sta) {
                if (def.opType == Location::sta) {
                    // another simple case
                    target = op = source;
                }
                else if (def.opType == Location::reg) {
                    // in that case we need to move it into a correct register first
                    auto where = GetFreeRegister(context, selected.allowedRegisters, target.value.ui, nullptr);
                    op = target = target.copyTo(Location::reg, where.value.ui);
                }
                else CodeGeneratorError("Internal: unimplemented stack to somewhere move!");
            }
            else if (source.op == Location::reg or source.op == Location::off) {
                if (def.opType == source.op) {
                    // registers are a more complex thing
                    if (IsRegisterAllowed(selected.allowedRegisters, target.value.ui, source.value.reg))
                        target = op = source;
                    else CodeGeneratorError("Internal: unimplemented register to valid register move!");
                }
                else if (def.opType == Location::Stack and source.op == Location::reg) {
                    auto content = context.getContent(source);
                    auto location = context.getLocationStackBias(content);
                    if (location.op == Location::reg) {
                        location = context.pushStack(content);
                        MoveInfo move = {source, location};
                        AddConversionsToMove(move, context, instructions, content);
                    }
                    target = op = source;
                }
                else CodeGeneratorError("Internal: unimplemented register to somewhere move!");
                if (source.op == Location::off) {
                    moves.erase(moves.begin() + k);
                    k--;
                    op.size = Options::addressSize;
                }
            }
            else CodeGeneratorError("Internal: unimplemented operand source case move!");
        }
        else if (target.is(Location::reg)) {
            auto& content = context.registers[target.value.reg].content;
            if (content.is(Location::var))
                // TODO: what about not putting it to registers we'll need?
                AsmOperand(Location::reg, content.type, false, content.size, target.value).moveAwayOrGetNewLocation(context, instructions, index, &forbiddenRegisters);
        }
        else Unimplemented();
    }

    // also checking for output operands here
    for (auto& n : results) {
        auto& source = n.source;
        auto& target = n.target;
        if (n.target.op == Location::op) {
            AsmOperand& op = ops[target.value.ui - 1];
            if (op.op == Location::None) CodeGeneratorError("Internal: empty operand for move!");

            n.target = ops[target.value.ui - 1];
        }
        else if (source.op == Location::op) {
            AsmOpDefinition& def = GetOpDefinition(selected, source.value.ui);
            if (def.isOutput and not def.isInput) {
                // it's only an output so it need to be included in operands
                AsmOperand& op = ops[source.value.ui - 1];
                op = target;
            }
            else CodeGeneratorError("Internal: invalid instruction target!");
        }
        //else {
        //    CodeGeneratorError("Internal: unhandled output!");
        //}
    }

    // now checking for things that might need to be moved in the locations
    for (auto& n : moves) {
        if (n.source == n.target) continue;
        auto content = context.getContent(n.target);

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

    for (auto& n : moves) {
        auto content = n.source;
        if (content.op != Location::Variable and content.op != Location::String and content.op != Location::imm)
            content = context.getContent(content);

        AddConversionsToMove(n, context, instructions, content, nullptr);
    }

    // adding the instruction itself
    AsmInstruction ins;
    ins.code = selected.code;
    ins.op1 = ops[0];
    ins.op2 = ops[1];
    ins.op3 = ops[2];
    ins.op4 = ops[3];

    for (auto& n : results) {
        // TODO: find out why an operand can get through, could be something with function calls
        if (n.source.is(Location::op))
            n.source = ops[n.source.value.ui - 1];
        if (n.target.is(Location::op))
            n.target = ops[n.target.value.ui - 1];

        if (n.source.op == Location::Variable and n.source.object(context).lastUse <= index) continue;
        if (n.target.op == Location::Variable or context.getContent(n.target).is(Location::Variable)) {
            auto loc = n.target.moveAwayOrGetNewLocation(context, instructions, index, nullptr);
            context.getContentRef(loc) = n.source;
        }
        else {
            context.getContentRef(n.target) = n.source;
        }
    }

    instructions.push_back(ins);
}