#include "X86_64Assembly.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "../TheGenerator.hpp"
#include "Assembly/LinearAnalysis.hpp"
#include <fstream>

namespace x86_64 {
    void ConvertBytecode(Bytecode& bytecode, uint64_t index) {
        switch (bytecode.code) {
            case Bytecode::add:
                if (Optimizations::swapExpressionOperands) {
                    if (bytecode.target.starts_with("$")) {
                        std::swap(bytecode.source, bytecode.target);
                    }
                }
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
            case Bytecode::multiply:
                if (Optimizations::swapExpressionOperands) {
                    if (bytecode.target.starts_with("$")) {
                        std::swap(bytecode.source, bytecode.target);
                    }
                }
                if (bytecode.type.type == ParserType::unsignedInteger) {
                    GenerateInstruction({
                    x86_64::mul, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }})}
                    }, index);
                }
                else if (bytecode.type.type == ParserType::signedInteger) {
                    // works much better if swapping in enabled
                    if (bytecode.type.size <= 4 and bytecode.source.starts_with("$")) {
                        // three operand form
                        GenerateInstruction({
                    x86_64::imul, bytecode.type.size, bytecode.target, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                    }, index);
                    }
                    else {
                        // 2 operand form
                        GenerateInstruction({
                    x86_64::imul, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                    }, index);
                    }
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid type for multiplication!");
                }
                break;
            case Bytecode::divide:
                if (bytecode.type.size == 1) {
                    // TODO: add something that does a movzbw on al to ax to set zeroes to ah
                    std::cout << "INFO L1: Due to the nature of 8 bit integer division result value may be invalid!\n";
                }
                if (bytecode.type.type == ParserType::unsignedInteger) {
                    GenerateInstruction({
                    x86_64::div, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }})}
                    }, index);
                }
                else if (bytecode.type.type == ParserType::signedInteger) {
                    GenerateInstruction({
                    x86_64::idiv, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-1" }})}
                    }, index);
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid type for division!");
                }
                break;
            case Bytecode::callFunction:

                break;
            case Bytecode::moveArgument:

                break;
            case Bytecode::prepareArguments:

                break;
            case Bytecode::returnValue:
                // move returned value into register a
                MoveValue(bytecode.source, "%0", bytecode.source, bytecode.type.size, index);
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
                                            "@" + std::to_string(AddStackVariable(bytecode.target)->offset)), bytecode.target, bytecode.type.size, index);
                break;
            }
            case Bytecode::assign:
                AssignExpressionToVariable(bytecode.source, bytecode.target);
                break;
            case Bytecode::addLabel:

                break;
            case Bytecode::moveValue:
            {
                auto target = generatorMemory.findThing(bytecode.target);
                if (target.type == Operand::reg) {
                    MoveValue(bytecode.source, "%" + std::to_string(target.number),
                              "%" + std::to_string(target.number), bytecode.type.size, index);
                }
                else if (target.type == Operand::reg) {
                    MoveValue(bytecode.source, "@" + std::to_string(target.offset),
                              "@" + std::to_string(target.offset), bytecode.type.size, index);
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid operand target for move!");
                }
            }
                break;
            default:
                CodeGeneratorError("Invalid bytecode code!");
                break;
        }
    }
}