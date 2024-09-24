#include "X86_64Assembly.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "../TheGenerator.hpp"
#include <fstream>

namespace x86_64 {
    void ConvertBytecode(const Bytecode& bytecode) {
        switch (bytecode.code) {
            case Bytecode::add:

                break;
            case Bytecode::subtract:

                break;
            case Bytecode::multiply:

                break;
            case Bytecode::divide:

                break;
            case Bytecode::callFunction:

                break;
            case Bytecode::moveArgument:

                break;
            case Bytecode::prepareArguments:

                break;
            case Bytecode::returnValue:
                // op1 contains the thing that needs to be moved to register a
                GenerateInstruction({x86_64::ret,
                                     bytecode.source,
                                     std::vector<OpCombination> {
                    OpCombination(OpCombination::Type::reg, {x86_64::rax})}
                                             });
                break;
            case Bytecode::pushLevel:
                generatorMemory.pushLevel();
                break;
            case Bytecode::popLevel:
                generatorMemory.popLevel();
                break;
            case Bytecode::jumpConditionalFalse:

                break;
            case Bytecode::jumpConditionalTrue:

                break;
            case Bytecode::jump:

                break;
            case Bytecode::compare:

                break;
            case Bytecode::declare:

                break;
            case Bytecode::assign:

                break;
            case Bytecode::addLabel:

                break;
            default:
                CodeGeneratorError("Invalid bytecode code!");
                break;
        }
    }

    void WriteToFile(std::ofstream& out, const std::string& instruction,
                     const DataLocation& op1) {
        //switch(options::assemblyFlavor) {
            //case options::AssemblyFlavor::ATnT:
                //return;
        //}
        out << instruction;
        if (instruction.size() >= options::spaceOnLeft) {
            out << " ";
        }
        else {
            for (uint8_t n = instruction.size(); n < options::spaceOnLeft; n++) {
                out << " ";
            }
        }
        out << op1 << "\n";
    }

    void WriteToFile(std::ofstream& out, std::string instruction,
                     const DataLocation& op1, const DataLocation& op2) {
        out << instruction;
        if (instruction.size() >= options::spaceOnLeft) {
            out << " ";
        }
        else {
            for (uint8_t n = instruction.size(); n < options::spaceOnLeft; n++) {
                out << " ";
            }
        }
        out << op2 << ", " << op1 << "\n";
    }

    void WriteToFile(std::ofstream& out, const std::string& instruction,
                     const DataLocation& op1, const DataLocation& op2, const DataLocation& op3) {
        out << instruction;
        if (instruction.size() >= options::spaceOnLeft) {
            out << " ";
        }
        else {
            for (uint8_t n = instruction.size(); n < options::spaceOnLeft; n++) {
                out << " ";
            }
        }
        out << op2 << ", " << op1 << ", " << op3 << "\n";
    }

    void WriteToFile(std::ofstream& out, const std::string& instruction,
                     const DataLocation& op1, const DataLocation& op2, const DataLocation& op3, const DataLocation& op4) {
        out << instruction;
        if (instruction.size() >= options::spaceOnLeft) {
            out << " ";
        }
        else {
            for (uint8_t n = instruction.size(); n < options::spaceOnLeft; n++) {
                out << " ";
            }
        }
        out << op2 << ", " << op1 << ", " << op3 << ", " << op4 << "\n";
    }


    void EmitAssemblyFromCode(const Instruction& ins, std::ofstream& out) {
        // movs, movz, mul, imul, div, idiv, cxtx, call, ret, push, pop, add, sub, syscall, jump, jc, cmp
        switch (ins.type) {
            case mov:
                WriteToFile(out, ins.addPostfix1("mov"), ins.op1, ins.op2);
                break;
        }
    }
}