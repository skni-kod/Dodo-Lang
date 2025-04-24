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

        for (uint32_t index = processor.index = 0; index < context.codes.size(); processor.index = ++index) {
            uint32_t printingStart = instructions.size();

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
                        n.content.print(std::cout, context, processor);
                        std::cout << "\n";
                    }
                }
                std::cout << "INFO L3: Stack:\n";
                for (auto& n : processor.stack) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: " << n.offset << ": ";
                        n.content.print(std::cout, context, processor);
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
                    // TODO: are local variable definitions even needed?
                    //else processor.pushStack(current.op1(), context);
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
                    auto sourceOp = AsmOperand(current.op1(), context);
                    auto valueOp = AsmOperand(current.op2(), context);

                    bool skip = false;

                    if (sourceOp.object(context, processor).lastUse <= index) skip = true;

                    // let's find out what the value is
                    if (valueOp.op == Location::Variable) {


                        valueOp = processor.getLocation(valueOp);
                    }

                    // now let's assign the location or value to the variable
                    if (not skip) processor.assignVariable(sourceOp, valueOp, context, instructions);
                }
                    break;

                case Bytecode::Cast: {
                    auto sourceOp = AsmOperand(current.op1(), context);
                    auto resultOp = AsmOperand(current.op3(), context);

                    if (sourceOp.object(context, processor).lastUse <= index) {
                        auto loc = processor.getLocation(sourceOp);
                        AsmOperand targetLoc;
                        if ((sourceOp.type == Type::floatingPoint or resultOp.type == Type::floatingPoint) and sourceOp.type != resultOp.type) targetLoc = processor.getFreeRegister(resultOp.type, resultOp.size);
                        else targetLoc = loc;
                        MoveInfo move = {sourceOp.copyTo(loc.op, loc.value), resultOp.copyTo(targetLoc.op, targetLoc.value)};
                        x86_64::AddConversionsToMove(move, context, processor, instructions, resultOp, nullptr);
                    }
                    else {
                        auto loc = processor.getFreeRegister(resultOp.type, resultOp.size);
                        MoveInfo move = {sourceOp, resultOp.copyTo(loc.op, loc.value)};
                        x86_64::AddConversionsToMove(move, context, processor, instructions, resultOp, nullptr);
                    }
                }
                    break;
                case Bytecode::Argument: {
                    // since there is an argument, there must be a call after it, so let's gather the arguments and the call
                    std::vector<Bytecode*> arguments;
                    do arguments.push_back(&context.codes[index++]);
                    while (context.codes[index].type == Bytecode::Argument);

                    // now we have arguments gathered, so let's get the call itself
                    switch (context.codes[index].type) {
                        case Bytecode::Syscall: CodeGeneratorError("Internal: syscalls not implemented!");
                            break;
                        case Bytecode::Method: CodeGeneratorError("Internal: methods not implemented!");
                            break;
                        case Bytecode::Function: {
                                auto [args, off] = GetFunctionMethodArgumentLocations(*context.codes[index].op1Value.function);
                                argumentPlaces = std::move(args);
                                argumentOffset = off;
                            }
                            break;
                            default: CodeGeneratorError("Internal: somehow a non-callable has arguments!");
                    }

                    // now we have the argument places ready and can move then there
                    // first let's place the arguments in correct places
                    for (uint32_t n = 0; n < arguments.size(); n++) {
                        auto s = AsmOperand(arguments[n]->op3(), context);
                        MoveInfo move = {processor.getLocation(s), argumentPlaces[n]};
                        x86_64::AddConversionsToMove(move, context, processor, instructions, s, nullptr);
                    }

                    // now that the arguments are in place, we need to move away the ones that would get overwritten
                    // TODO: add caller and callee saved registers
                    for (auto& n : processor.registers) {
                        // only variables get preserved
                        if (n.content.op != Location::Variable) {
                            n.content = {};
                            continue;
                        }

                        bool evacuate = true;
                        for (auto& m : argumentPlaces) {
                            if (m.op == Location::reg and m.value.reg == n.number) {
                                evacuate = false;
                                break;
                            }
                        }
                        if (not evacuate) continue;

                        // now we need to know if the thing there is even needed
                        auto& obj = n.content.object(context, processor);
                        if (obj.lastUse > index and obj.assignedOffset == 0) {
                            // it exists if it has a stack location, then it's fine.
                            // if it has none, then we need to move it from the register to the stack
                            MoveInfo move = {n.content.copyTo(Location::reg, n.number), processor.pushStack(n.content, context)};
                            x86_64::AddConversionsToMove(move, context, processor, instructions, n.content, nullptr);
                        }

                        n.content = {};
                    }

                    // now doing the call itself and setting return value
                    if (context.codes[index].type == Bytecode::Syscall) {

                    }
                    else {
                        instructions.emplace_back(call, AsmOperand(Location::Label, Type::none, true, AsmOperand::LabelType::function, context.codes[index].op1Value));
                        auto result = AsmOperand(context.codes[index].op3(), context);
                        if (result.op != Location::None) {
                            if (result.type == Type::floatingPoint) processor.registers[XMM0].content = result;
                            else processor.registers[RAX].content = result;
                        }
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
                                    AsmOpDefinition(Location::reg, 4, 4, true, true),
                                    AsmOpDefinition(Location::reg, 4, 4, true, false),
                                    { // allowed registers
                                        RegisterRange(XMM0, XMM31, true, true, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                        AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(addss, Options::None,
                                    AsmOpDefinition(Location::reg, 4, 4, true, true),
                                    AsmOpDefinition(Location::sta, 4, 4, true, false),
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
                                        AsmOpDefinition(Location::reg, 8, 8, true, true),
                                        AsmOpDefinition(Location::reg, 8, 8, true, false),
                                        { // allowed registers
                                            RegisterRange(XMM0, XMM31, true, true, false, false)
                                        },
                                        { // inputs and outputs
                                            AsmInstructionResultInput(true,  1, {current.op1(), context}),
                                            AsmInstructionResultInput(true,  2, {current.op2(), context}),
                                            AsmInstructionResultInput(false, 1, {current.op3(), context})
                                    }),
                                    AsmInstructionVariant(addsd, Options::None,
                                        AsmOpDefinition(Location::reg, 8, 8, true, true),
                                        AsmOpDefinition(Location::sta, 8, 8, true, false),
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
                case Bytecode::Subtract:
                    if (currentType != Type::floatingPoint) {
                        AsmInstructionInfo instruction = {
                            { // variants of the instruction
                                AsmInstructionVariant(sub, Options::None,
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
                                AsmInstructionVariant(sub, Options::None,
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
                                AsmInstructionVariant(sub, Options::None,
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
                                AsmInstructionVariant(sub, Options::None,
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
                                AsmInstructionVariant(sub, Options::None,
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
                                AsmInstructionVariant(subss, Options::None,
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
                                AsmInstructionVariant(subss, Options::None,
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
                                    AsmInstructionVariant(subsd, Options::None,
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
                                    AsmInstructionVariant(subsd, Options::None,
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
            if (Options::informationLevel >= Options::full) {
                for (; printingStart < instructions.size(); printingStart++) {
                    std::cout << "INFO L3: Generated instruction: ";
                    x86_64::PrintInstruction(instructions[printingStart], std::cout);
                }
            }

        }

        if (Options::informationLevel >= Options::full) {
            std::cout << "INFO L3: Processor after the last instruction:\nINFO L3: Registers:\n";
            for (auto& n : processor.registers) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: ";
                    PrintRegisterName(n.number, n.content.size, std::cout);
                    std::cout << ": ";
                    n.content.print(std::cout, context, processor);
                    std::cout << "\n";
                }
            }
            std::cout << "INFO L3: Stack:\n";
            for (auto& n : processor.stack) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: " << n.offset << ": ";
                    n.content.print(std::cout, context, processor);
                    std::cout << "\n";
                }
            }
        }

        PrintInstructions(instructions, out, maxOffset);
    }
}
