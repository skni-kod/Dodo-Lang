#include "X86_64Assembly.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "../TheGenerator.hpp"
#include "Assembly/LinearAnalysis.hpp"
#include <fstream>

namespace x86_64 {
    void ConvertBytecode(const Bytecode& bytecode, uint64_t index) {
        switch (bytecode.code) {
            case Bytecode::add:
                GenerateInstruction({
                    x86_64::add, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                }, index);
                break;
            case Bytecode::subtract:
                GenerateInstruction({
                    x86_64::sub, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                }, index);
                break;
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
                // move returned value into register a
                MoveValue(bytecode.source, "%0", bytecode.source, bytecode.type.size);
                // insert a return statement
                GenerateInstruction({x86_64::ret}, index);
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
            {
                auto& var = variableLifetimes[bytecode.target];
                if (Optimizations::skipUnusedVariables and var.usageAmount == 0) {
                        return;
                }

                MoveValue(bytecode.source, (var.assignStatus == VariableStatistics::AssignStatus::reg ?
                                            "%" + std::to_string(var.assigned) :
                                            "@" + std::to_string(AddStackVariable(bytecode.target)->offset)), bytecode.target, bytecode.type.size);
                break;
            }
            case Bytecode::assign:
                AssignExpressionToVariable(bytecode.source, bytecode.target);
                break;
            case Bytecode::addLabel:

                break;
            default:
                CodeGeneratorError("Invalid bytecode code!");
                break;
        }
    }
}