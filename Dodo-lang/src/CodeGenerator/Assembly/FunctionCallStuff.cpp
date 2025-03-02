#include <tuple>
#include "GenerateCodeInternal.hpp"
#include "MemoryStructure.hpp"
#include "GenerateCode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "Misc/Options.hpp"

void AddFunctionHeaders() {
    // in this function we need to find the max stack offset and used registers
    // let's do it all at once
    int64_t maxOffset = 0;
    std::vector<bool> occupied(generatorMemory.registers.size(), false);
    doAddLeave = false;

    // now let's go through every instruction and see what's up
    for (auto& n : finalInstructions) {
        if (n.op1.type == Operand::reg) {
            occupied[n.op1.number] = true;
        }
        else if (n.op1.type == Operand::sta) {
            if (n.op1.offset < maxOffset) {
                maxOffset = n.op1.offset;
            }
        }
        if (n.op2.type == Operand::reg) {
            occupied[n.op2.number] = true;
        }
        else if (n.op2.type == Operand::sta) {
            if (n.op2.offset < maxOffset) {
                maxOffset = n.op2.offset;
            }
        }
        if (n.op3.type == Operand::reg) {
            occupied[n.op3.number] = true;
        }
        else if (n.op3.type == Operand::sta) {
            if (n.op3.offset < maxOffset) {
                maxOffset = n.op3.offset;
            }
        }
        if (n.op4.type == Operand::reg) {
            occupied[n.op4.number] = true;
        }
        else if (n.op4.type == Operand::sta) {
            if (n.op4.offset < maxOffset) {
                maxOffset = n.op4.offset;
            }
        }
    }

    // go through arguments to make sure that arguments are not moved
    for (auto& n : parserFunctions[*lastFunctionName].arguments) {
        if (n.locationType == Operand::reg) {
            occupied[n.locationValue] = false;
        }
    }

    if (not parserFunctions[*lastFunctionName].returnType.empty()) {
        if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
            occupied[0] = false;
        }
        else {
            CodeGeneratorError("Unimplemented: Non x86-64 return variable value restore prevention!");
        }
    }

    // now we need to add occupied registers to the stack, might add checking max size later to optimize
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        if (maxOffset % 8) {
            maxOffset = (maxOffset / 8 - 1) * 8;
        }

        {
            DEPRECATEDInstruction ins;
            ins.type = x86_64::returnPoint;
            finalInstructions.push_back(ins);
        }

        // now go through all the registers and find them a nice place on the stack
        for (uint64_t n = 0; n < occupied.size(); n++) {
            if (occupied[n]) {
                // add the register here
                maxOffset -= 8;
                DEPRECATEDInstruction ins;
                ins.type = x86_64::mov;
                ins.sizeAfter = ins.sizeBefore = 8;
                ins.op1 = {Operand::sta, maxOffset};
                ins.op2 = {Operand::reg, n};
                finalInstructions.insert(finalInstructions.begin(), ins);
                std::swap(ins.op1, ins.op2);
                finalInstructions.push_back(ins);
            }
        }

        if (maxOffset % 16) {
            maxOffset = (maxOffset / 16 - 1) * 16;
        }

        // TODO: add this only in function calls are in the function
        if (maxOffset != 0) {
            DEPRECATEDInstruction ins;
            ins.type = x86_64::sub;
            ins.sizeBefore = ins.sizeAfter = 8;
            ins.op1 = {Operand::reg, uint64_t(x86_64::rsp)};
            ins.op2 = {Operand::imm, uint64_t(-maxOffset)};
            finalInstructions.insert(finalInstructions.begin(), ins);
            doAddLeave = true;
        }
    }
    else {
        CodeGeneratorError("Unimplemented: Function analysis in non x86-64 architecture");
    }
}
