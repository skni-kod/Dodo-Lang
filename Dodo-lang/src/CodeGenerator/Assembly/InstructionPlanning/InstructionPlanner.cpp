#include "InstructionPlanningInternal.hpp"
#include "X86_64.hpp"

// target resolution functions
std::vector <MoveInfo> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc) {
    switch (Options::targetArchitecture) {
        case Options::TargetArchitecture::x86_64:
            return x86_64::AddConvertionsToMove(move, context, proc);
        default:
            CodeGeneratorError("Internal: invalid architecture in move conversion!");
    }
    return {};
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
        // TODO: add variant resolution
    }

    // after that all the needed moves need to be prepared
    std::vector <MoveInfo> moves, results;
    auto& selected = *instruction.selected;

    // defining what values need to be moved where before
    for (auto& n : instruction.selected->resultsAndInputs) {
        if (n.isInput) {
            if (n.isFixedLocation) moves.emplace_back(n.value, n.fixedLocation);
            else moves.emplace_back(n.value, n.value.CopyTo(Location::Operand, n.operandNumber));
        }
        else {
            if (n.isFixedLocation) results.emplace_back(n.fixedLocation, n.value);
            else results.emplace_back(n.value.CopyTo(Location::Operand, n.operandNumber), n.value);
        }
    }

    bool foundOutput = false;
    uint8_t inputCounter = 0;
    if (selected.op1.isInput) {
        moves.emplace_back(instruction.source1, instruction.source1.CopyTo(Location::op, 1));
        inputCounter++;
    }
    if (selected.op1.isOutput) {
        foundOutput = true;
        results.emplace_back(instruction.destination.CopyTo(Location::op, 1), instruction.destination);
    }

    if (selected.op2.isInput) {
        if (inputCounter) moves.emplace_back(instruction.source2, instruction.source2.CopyTo(Location::op, 2));
        else moves.emplace_back(instruction.source1, instruction.source1.CopyTo(Location::op, 2));
        inputCounter++;
    }

    if (not foundOutput and selected.op2.isOutput) {
        foundOutput = true;
        results.emplace_back(instruction.destination.CopyTo(Location::op, 2), instruction.destination);
    }

    if (selected.op3.isInput) {
        if (inputCounter == 2) moves.emplace_back(instruction.source3, instruction.source3.CopyTo(Location::op, 3));
        else if (inputCounter == 1) moves.emplace_back(instruction.source2, instruction.source2.CopyTo(Location::op, 3));
        else moves.emplace_back(instruction.source1, instruction.source1.CopyTo(Location::op, 3));
        inputCounter++;
    }

    if (not foundOutput and selected.op3.isOutput) {
        foundOutput = true;
        results.emplace_back(instruction.destination.CopyTo(Location::op, 3), instruction.destination);
    }

    if (selected.op4.isInput) {
        if (inputCounter == 3) moves.emplace_back(instruction.source4, instruction.source4.CopyTo(Location::op, 4));
        else if (inputCounter == 2) moves.emplace_back(instruction.source3, instruction.source3.CopyTo(Location::op, 4));
        else if (inputCounter == 1) moves.emplace_back(instruction.source2, instruction.source2.CopyTo(Location::op, 4));
        else moves.emplace_back(instruction.source1, instruction.source1.CopyTo(Location::op, 4));
    }

    if (not foundOutput and selected.op4.isOutput) {
        foundOutput = true;
        results.emplace_back(instruction.destination.CopyTo(Location::op, 4), instruction.destination);
    }

    // now checking for things that might need to be moved in the locations
    // TODO: think this through
    for (auto& n : moves) {
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
    for (auto& n : results) {
        // TODO: the same here
    }

    // now all the moves should be in the vectors and can be executed, though I probably forgot about something
    // let's get their actual instruction representations
    // the possibility of these moves needing to use occupied locations needs to be taken into consideration
    std::vector <std::vector <MoveInfo>> actualMoves;

    for (auto& n : moves) {
        actualMoves.push_back(AddConvertionsToMove(n, context, processor));
    }
}