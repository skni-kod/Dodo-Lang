#include <GenerateCode.hpp>
#include <iostream>
#include <fstream>

#include "X86_64.hpp"

namespace x86_64 {
    void ConvertBytecode(BytecodeContext& context, Processor& processor, ParserFunctionMethod* source, std::ofstream& out) {
        std::vector <AsmInstruction> instructions;
        std::vector <AsmOperand> argumentPlaces;
        int32_t argumentOffset = 0;
        int32_t maxOffset = 0;

        for (uint32_t index = 0; index < context.codes.size(); index++) {
            auto& current = context.codes[index];
            uint16_t currentSize;
            Type::TypeEnum currentType;
            if (current.opTypeMeta.isReference or current.opTypeMeta.pointerLevel or not current.opType->isPrimitive) {
                currentSize = Options::addressSize;
                currentType = Type::address;
            }
            else {
                currentSize = current.opType->typeSize;
                currentType = current.opType->primitiveType;
            }

            if (Options::informationLevel >= Options::full) {
                std::cout << "INFO L3: Processor before instruction " << index << ":\nINFO L3: Registers:\n";
                for (auto& n : processor.registers) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: ";
                        PrintRegisterName(n.number, n.content.size, std::cout);
                        std::cout << ": ";
                        n.content.print(std::cout, context);
                        std::cout << "\n";
                    }
                }
                std::cout << "INFO L3: Stack:\n";
                for (auto& n : processor.stack) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: " << n.offset << ": ";
                        n.content.print(std::cout, context);
                        std::cout << "\n";
                    }
                }
            }

            switch (current.type) {
                case Bytecode::Define:
                    // TODO: make the variables appear not on stack
                    if (current.op3().location == Location::Argument) {
                        if (current.op3Value.ui == 0) {
                            auto [args, off] = GetFunctionMethodArgumentLocations(*source);
                            argumentPlaces = std::move(args);
                            argumentOffset = off;
                        }
                        if (argumentPlaces[current.op3Value.ui].op == Location::sta)
                            processor.stack.emplace_back(AsmOperand(current.op1(), context),
                            argumentPlaces[current.op3Value.ui].value.offset, argumentPlaces[current.op3Value.ui].size);

                        processor.getContentRef(argumentPlaces[current.op3Value.ui]) = AsmOperand(current.op1(), context);
                    }
                    else processor.pushStack(current.op1(), context);
                    break;
                case Bytecode::Return: {
                    AsmInstructionInfo instruction;
                    if (source->parentType == nullptr) source->parentType = &types[*source->returnType.typeName];
                    if (not source->returnType.type.pointerLevel and not source->returnType.type.isReference and
                    source->parentType->isPrimitive and source->parentType->primitiveType == Type::floatingPoint)
                        instruction = {{
                        // variants of the instruction:
                        AsmInstructionVariant(ret, Options::None,
                            // operands:
                            // non operand values that need to be put into places:
                            {
                                AsmInstructionResultInput(true, AsmOperand(current.op1(), context, Location::reg, XMM0), AsmOperand(current.op1(), context))
                        })},
                        // data for the operation itself: source 1, source 2, source 3, destination
                        AsmOperand(current.op1(), context)
                    };
                    else instruction = {{
                        // variants of the instruction:
                        AsmInstructionVariant(ret, Options::None,
                            // operands:
                            // non operand values that need to be put into places:
                            {
                                AsmInstructionResultInput(true, AsmOperand(current.op1(), context, Location::reg, RAX), AsmOperand(current.op1(), context))
                        })},
                        // data for the operation itself: source 1, source 2, source 3, destination
                        AsmOperand(current.op1(), context)
                    };


                    ExecuteInstruction(context, processor, instruction, instructions, index);

                    break;
                }
                case Bytecode::AssignTo: {
                    // we need to find where this variable actually is
                    // TODO: add overwrite check
                    auto sourceOp = AsmOperand(current.op1(), context);
                    auto value = AsmOperand(current.op2(), context);
                    auto result = AsmOperand(current.op3(), context);
                    if (result.object(context).uses == 0) {
                        processor.assignVariable(sourceOp, processor.getLocation(value), sourceOp);
                    }
                    else if (sourceOp.object(context).lastUse > index) CodeGeneratorError("Internal: unimplemented source overwrite check!");
                    else if (result.object(context).lastUse > index) CodeGeneratorError("Internal: unimplemented value overwrite check!");
                    else {
                        processor.assignVariable(sourceOp, processor.getLocation(value), result);
                    }
                }

                    break;
                case Bytecode::Add:
                    if (currentType != Type::floatingPoint) {
                        AsmInstructionInfo instruction = {
                            { // variants of the instruction
                                AsmInstructionVariant(add, Options::None,
                                    AsmOpDefinition(Location::reg, 1, 8, true, true),
                                    AsmOpDefinition(Location::imm, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, false, false, false),
                                        RegisterRange(R8, R15, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(add, Options::None,
                                    AsmOpDefinition(Location::reg, 1, 8, true, true),
                                    AsmOpDefinition(Location::mem, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, false, false, false),
                                        RegisterRange(R8, R15, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(add, Options::None,
                                    AsmOpDefinition(Location::reg, 1, 8, true, true),
                                    AsmOpDefinition(Location::reg, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, true, false, false),
                                        RegisterRange(R8, R15, true, true, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(add, Options::None,
                                    AsmOpDefinition(Location::mem, 1, 8, true, true),
                                    AsmOpDefinition(Location::reg, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, false, true, false, false),
                                        RegisterRange(R8, R15, false, true, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(add, Options::None,
                                    AsmOpDefinition(Location::mem, 1, 8, true, true),
                                    AsmOpDefinition(Location::imm, 1, 8, true, false),
                                    { // allowed registers
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                })
                            }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
                    }
                    else {
                        if (currentSize == 4) {
                            // single precision
                            AsmInstructionInfo instruction = {
                            { // variants of the instruction
                                AsmInstructionVariant(addss, Options::None,
                                    AsmOpDefinition(Location::reg, 1, 8, true, true),
                                    AsmOpDefinition(Location::reg, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(XMM0, XMM31, true, true, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(addss, Options::None,
                                    AsmOpDefinition(Location::reg, 1, 8, true, true),
                                    AsmOpDefinition(Location::sta, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(XMM0, XMM31, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                })
                            }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
                        }
                        else {
                            // double precision
                            AsmInstructionInfo instruction = {
                                { // variants of the instruction
                                    AsmInstructionVariant(addsd, Options::None,
                                        AsmOpDefinition(Location::reg, 1, 8, true, true),
                                        AsmOpDefinition(Location::reg, 1, 8, true, false),
                                        { // allowed registers
                                            RegisterRange(XMM0, XMM31, true, true, false, false)
                                        },
                                        { // inputs and outputs
                                            AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                            AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                            AsmInstructionResultInput(false, 1, {current.op3(), context})
                                    }),
                                    AsmInstructionVariant(addsd, Options::None,
                                        AsmOpDefinition(Location::reg, 1, 8, true, true),
                                        AsmOpDefinition(Location::sta, 1, 8, true, false),
                                        { // allowed registers
                                            RegisterRange(XMM0, XMM31, true, false, false, false)
                                        },
                                        { // inputs and outputs
                                            AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                            AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                            AsmInstructionResultInput(false, 1, {current.op3(), context})
                                    })
                                }};
                            ExecuteInstruction(context, processor, instruction, instructions, index);
                        }
                    }
                    break;
                default:
                    CodeGeneratorError("Unhandled bytecode type in x86-64 converter!");
            }

            // checking stack offset before clearing to not remove very short-lived variables
            // TODO: maybe iterate to not include unused or something
            if (not processor.stack.empty() and processor.stack.back().offset < maxOffset) maxOffset = processor.stack.back().offset < maxOffset;
            processor.cleanUnusedVariables(context, index);

        }

        if (Options::informationLevel >= Options::full) {
            std::cout << "INFO L3: Processor after the last instruction:\nINFO L3: Registers:\n";
            for (auto& n : processor.registers) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: ";
                    PrintRegisterName(n.number, n.content.size, std::cout);
                    std::cout << ": ";
                    n.content.print(std::cout, context);
                    std::cout << "\n";
                }
            }
            std::cout << "INFO L3: Stack:\n";
            for (auto& n : processor.stack) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: " << n.offset << ": ";
                    n.content.print(std::cout, context);
                    std::cout << "\n";
                }
            }
        }

        PrintInstructions(instructions, out, maxOffset);
    }
}
