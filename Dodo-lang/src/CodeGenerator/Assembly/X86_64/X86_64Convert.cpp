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
            if (current.opType != nullptr and (current.opTypeMeta.isReference or current.opTypeMeta.pointerLevel or not current.opType->isPrimitive)) {
                currentSize = Options::addressSize;
                currentType = Type::address;
            }
            else if (current.opType != nullptr) {
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
                        if (n.content.op == Location::Variable) std::cout << " (value size: " << n.content.object(context, processor).type->typeSize << ")\n";
                        else std::cout << "\n";
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
                    else {
                        processor.pushStack(current.op1(), context);
                        processor.stack.back().content.object(context, processor).assignedOffset = processor.stack.back().offset;
                    }
                    break;
                case Bytecode::Return: {
                    AsmInstructionInfo instruction;
                    if (source->returnType.typeName != nullptr) {
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
                    }
                    else {
                        instruction = {{
                            // variants of the instruction:
                            AsmInstructionVariant(ret, Options::None,
                                // operands:
                                // non-operand values that need to be put into places:
                                {
                            })},
                            // data for the operation itself: source 1, source 2, source 3, destination
                            };
                    }
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
                case Bytecode::AssignAt: {
                    auto sourceOp = AsmOperand(current.op1(), context);
                    auto valueOp = AsmOperand(current.op2(), context);
                    auto resultOp = AsmOperand(current.op3(), context);

                    bool skip = true;
                    bool assignResult = false;

                    assignResult = resultOp.object(context, processor).lastUse > index;

                    if (valueOp.op == Location::Variable) {
                        if (assignResult and sourceOp.object(context, processor).lastUse > index) {
                            // assigning result but the value is here
                            CodeGeneratorError("Internal: assign at result not implemented!");
                        }
                        else if (assignResult) {
                            CodeGeneratorError("Internal: assign at result not implemented!");
                        }
                        valueOp = processor.getLocation(valueOp);
                    }

                    sourceOp = processor.getLocation(sourceOp);

                    // now we need to generate a move from the value to the address in source
                    if (sourceOp.op == Location::reg) {
                        sourceOp.op = Location::off;
                        sourceOp.value.regOff.regNumber = sourceOp.value.reg;
                        sourceOp.value.regOff.offset = 0;
                        sourceOp.type = Type::address;
                        sourceOp.size = 0;
                    }
                    else if (sourceOp.op != Location::off) CodeGeneratorError("Internal: non-register address!");

                    MoveInfo move = {valueOp, sourceOp};
                    x86_64::AddConversionsToMove(move ,context, processor, instructions, {}, nullptr);
                }
                    break;
                case Bytecode::Jump:
                    instructions.emplace_back(jmp, AsmOperand(Location::Label, Type::none, false, AsmOperand::jump, current.op1Value));
                    break;
                case Bytecode::Label:
                    instructions.emplace_back(label, AsmOperand(Location::Label, Type::none, false, AsmOperand::jump, current.op1Value));
                    break;
                case Bytecode::BeginScope:
                    memorySnapshots.emplace(std::move(processor.createSnapshot(context)));
                    break;
                case Bytecode::EndScope:
                    processor.restoreSnapshot(memorySnapshots.top(), instructions, context);
                    memorySnapshots.pop();
                    break;
                case Bytecode::If: {
                    // an if statement
                    auto condition = AsmOperand(current.op1(), context);
                    if (condition.op == Location::imm) {
                        bool skip;
                        if (condition.type != Type::floatingPoint) {
                            if (condition.size == 4) skip = condition.value.f32 == 0;
                            else skip = condition.value.f64 == 0;
                        }
                        else {
                            if (condition.size == 1) skip = condition.value.u8 == 0;
                            else if (condition.size == 2) skip = condition.value.u16 == 0;
                            else if (condition.size == 4) skip = condition.value.u32 == 0;
                            else skip = condition.value.u64 == 0;
                        }

                        // it's a literal, so i it's 0 then we skip over and if not, then we continue
                        if (skip) {
                            // let's go and skip everything until we get the end of scope
                            uint32_t scopes = 0;
                            while (not (context.codes[++index].type == Bytecode::EndScope and scopes == 1)) {
                                if (context.codes[index].type == Bytecode::BeginScope) scopes++;
                                else if (context.codes[index].type == Bytecode::EndScope) scopes--;
                            }
                        }
                    }
                    else {
                        if (condition.op == Location::Variable) condition = processor.getLocation(condition);
                        auto falseLabel = AsmOperand(Location::Label, Type::none, false, AsmOperand::jump, current.op2Value);

                        // now since it's unoptimized, we just need to check if the condition is 0 and then jump to the false condition label
                        // TODO: add non-explicit condition
                        instructions.emplace_back(cmp, condition, AsmOperand(Location::Literal, Type::unsignedInteger, false, 1, 0));
                        instructions.emplace_back(je, falseLabel);
                    }

                }
                    break;
                case Bytecode::Greater:
                case Bytecode::GreaterEqual:
                case Bytecode::Lesser:
                case Bytecode::LesserEqual:
                case Bytecode::Equals:
                case Bytecode::NotEqual:
                    {
                    // we need to generate a cmp between the operands
                    auto left = AsmOperand(current.op1(), context);
                    auto right = AsmOperand(current.op2(), context);
                    auto result = AsmOperand(current.op3(), context);

                    if (left.type != right.type) CodeGeneratorError("Internal: incompatible type comparison!");
                    if (left.type == Type::floatingPoint and left.size == 4) {
                        // for now this will be the unoptimized version that returns a 0 or 1
                        AsmInstructionInfo instruction = {
                        { // variants of the instruction
                            AsmInstructionVariant(comiss, Options::None,
                                AsmOpDefinition(Location::reg, 4, 4, true, false),
                                AsmOpDefinition(Location::sta, 4, 4, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comiss, Options::None,
                                AsmOpDefinition(Location::reg, 4, 4, true, false),
                                AsmOpDefinition(Location::off, 4, 4, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false),
                                    RegisterRange(RAX, RDI, false, true, false, false),
                                    RegisterRange(R8, R15, false, true, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comiss, Options::None,
                                AsmOpDefinition(Location::reg, 4, 4, true, false),
                                AsmOpDefinition(Location::mem, 4, 4, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comiss, Options::None,
                                AsmOpDefinition(Location::reg, 4, 4, true, false),
                                AsmOpDefinition(Location::reg, 4, 4, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, true, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            })
                        }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
                    }
                    else if (left.type == Type::floatingPoint and left.size == 8) {
                        // for now this will be the unoptimized version that returns a 0 or 1
                        AsmInstructionInfo instruction = {
                        { // variants of the instruction
                            AsmInstructionVariant(comisd, Options::None,
                                AsmOpDefinition(Location::reg, 8, 8, true, false),
                                AsmOpDefinition(Location::sta, 8, 8, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comisd, Options::None,
                                AsmOpDefinition(Location::reg, 8, 8, true, false),
                                AsmOpDefinition(Location::off, 8, 8, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false),
                                    RegisterRange(RAX, RDI, false, true, false, false),
                                    RegisterRange(R8, R15, false, true, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comisd, Options::None,
                                AsmOpDefinition(Location::reg, 8, 8, true, false),
                                AsmOpDefinition(Location::mem, 8, 8, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(comisd, Options::None,
                                AsmOpDefinition(Location::reg, 8, 8, true, false),
                                AsmOpDefinition(Location::reg, 8, 8, true, false),
                                { // allowed registers
                                    RegisterRange(XMM0, XMM31, true, true, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            })
                        }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
                    }
                    else {
                        // for now this will be the unoptimized version that returns a 0 or 1
                        AsmInstructionInfo instruction = {
                        { // variants of the instruction
                            AsmInstructionVariant(cmp, Options::None,
                                AsmOpDefinition(Location::reg, 1, 8, true, false),
                                AsmOpDefinition(Location::imm, 1, 8, true, false),
                                { // allowed registers
                                    RegisterRange(RAX, RDI, true, false, false, false),
                                    RegisterRange(R8, R15, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(cmp, Options::None,
                                AsmOpDefinition(Location::reg, 1, 8, true, false),
                                AsmOpDefinition(Location::sta, 1, 8, true, false),
                                { // allowed registers
                                    RegisterRange(RAX, RDI, true, false, false, false),
                                    RegisterRange(R8, R15, true, false, false, false)
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            }),
                            AsmInstructionVariant(cmp, Options::None,
                                AsmOpDefinition(Location::sta, 1, 8, true, false),
                                AsmOpDefinition(Location::imm, 1, 8, true, false),
                                { // allowed registers
                                },
                                { // inputs and outputs
                                    AsmInstructionResultInput(true,  1, left),
                                    AsmInstructionResultInput(true,  2, right)
                            })
                        }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
                    }
                    processor.registers[EFLAGS].content = {};

                    // now we have the comparison done, so let's move its value into a register or memory location
                    // TODO: add an option to use memory if no free register found
                    AsmOperand reg = processor.getFreeRegister(Type::unsignedInteger, 1);
                    // now we need to do a conditional set
                    switch (current.type) {
                        case Bytecode::Greater:
                            if (left.type == Type::signedInteger) instructions.emplace_back(setg, reg);
                            else instructions.emplace_back(seta, reg);
                            break;
                        case Bytecode::Lesser:
                            if (left.type == Type::signedInteger) instructions.emplace_back(setl, reg);
                            else instructions.emplace_back(setb, reg);
                            break;
                        case Bytecode::GreaterEqual:
                            if (left.type == Type::signedInteger) instructions.emplace_back(setge, reg);
                            else instructions.emplace_back(setae, reg);
                            break;
                        case Bytecode::LesserEqual:
                            if (left.type == Type::signedInteger) instructions.emplace_back(setle, reg);
                            else instructions.emplace_back(setbe, reg);
                            break;
                        case Bytecode::Equals:
                            instructions.emplace_back(sete, reg);
                            break;
                        case Bytecode::NotEqual:
                            instructions.emplace_back(setne, reg);
                            break;
                            default:
                            CodeGeneratorError("Internal: forgot to add setcc for condition!");
                    }
                    processor.registers[reg.value.reg].content = result;
                }
                    break;
                case Bytecode::Address: {
                    // TODO: add move to stack
                    auto sourceOp = AsmOperand(current.op1(), context);

                    if (sourceOp.op == Location::Variable) sourceOp = processor.getLocationStackBias(sourceOp);

                    if (sourceOp.op != Location::sta) CodeGeneratorError("Internal: non-stack pointer places not supported!");
                    AsmOperand reg = processor.getFreeRegister(Type::address, 8);

                    AsmInstructionInfo instruction = {
                            { // variants of the instruction
                                AsmInstructionVariant(lea, Options::None,
                                    AsmOpDefinition(Location::reg, 8, 8, false, true),
                                    AsmOpDefinition(Location::sta, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, false, false, false),
                                        RegisterRange(R8, R15, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  2, sourceOp),
                                        AsmInstructionResultInput(false,  reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                })
                            }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);
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
                        auto sloc = processor.getLocation(sourceOp);
                        MoveInfo move = {sloc, resultOp.copyTo(loc.op, loc.value)};
                        x86_64::AddConversionsToMove(move, context, processor, instructions, resultOp, nullptr);
                    }
                }
                    break;
                case Bytecode::LoopLabel:
                    break;
                case Bytecode::Syscall:
                case Bytecode::Function:
                case Bytecode::Method:
                case Bytecode::Argument: {
                    // since there is an argument, there must be a call after it, so let's gather the arguments and the call
                    uint64_t firstIndex = index;
                    std::vector<Bytecode*> arguments;
                    while (context.codes[index].type == Bytecode::Argument) arguments.push_back(&context.codes[index++]);

                    // now we have arguments gathered, so let's get the call itself
                    switch (context.codes[index].type) {
                        case Bytecode::Syscall: {
                            auto [args, off] = GetFunctionMethodArgumentLocations(arguments, context, processor);
                            argumentPlaces = std::move(args);
                            argumentOffset = off;
                        }
                            break;
                        case Bytecode::Method:
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
                        // TODO: add forbidden locations
                        MoveInfo move = {processor.getLocation(s), argumentPlaces[n].moveAwayOrGetNewLocation(context, processor, instructions, firstIndex, nullptr, true)};
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
                        if (obj.lastUse > index and processor.getLocationStackBias(n.content).op != Location::sta) {
                            // it exists if it has a stack location, then it's fine.
                            // if it has none, then we need to move it from the register to the stack
                            n.content.copyTo(Location::reg, n.number).moveAwayOrGetNewLocation(context, processor, instructions, index, nullptr, true);
                            //MoveInfo move = {n.content.copyTo(Location::reg, n.number), processor.pushStack(n.content, context)};
                            //x86_64::AddConversionsToMove(move, context, processor, instructions, n.content, nullptr);
                        }

                        n.content = {};
                    }

                    // now doing the call itself and setting return value
                    if (context.codes[index].type == Bytecode::Syscall) {
                        instructions.emplace_back(mov,
                        AsmOperand(Location::reg, Type::unsignedInteger, false, 8, RAX),
                        AsmOperand(Location::Literal, Type::unsignedInteger, false, 8, context.codes[index].op1Value.ui)
                            );
                        instructions.emplace_back(syscall);
                    }
                    else {
                        instructions.emplace_back(call, AsmOperand(Location::Label, Type::none, true, AsmOperand::LabelType::function, context.codes[index].op1Value));
                    }

                    // removing saved values from argument registers
                    for (auto& n : argumentPlaces) {
                        if (n.op == Location::reg) processor.registers[n.value.reg].content = {};
                    }

                    auto result = AsmOperand(context.codes[index].op3(), context);
                    if (result.op != Location::None) {
                        if (result.type == Type::floatingPoint) processor.registers[XMM0].content = result;
                        else processor.registers[RAX].content = result;
                    }
                }
                    break;

                case Bytecode::Member: {
                    auto sourceOp = AsmOperand(current.op1(), context);
                    int32_t offset = current.op2Value.i32;
                    auto resultOp = AsmOperand(current.op3(), context);

                    if (sourceOp.op == Location::Variable) sourceOp = processor.getLocation(sourceOp);

                    // let's get a free register for the pointer for lea
                    AsmOperand reg = processor.getFreeRegister(Type::address, 8);

                    // if it's a register
                    // TODO: add more checks
                    if (sourceOp.op == Location::reg) {
                        //if (not sourceOp.useAddress) CodeGeneratorError("Using register address not marked as used!");
                        sourceOp.op = Location::off;
                        sourceOp.value.regOff.regNumber = sourceOp.value.reg;
                        sourceOp.value.regOff.offset = offset;
                        sourceOp.type = Type::address;
                        sourceOp.size = 0;
                    }
                    // in case of a stack address, change it to be the correct one
                    else if (sourceOp.op == Location::sta) {
                        sourceOp.value.offset += offset;
                    }

                    // now the actual leaq
                     AsmInstructionInfo instruction = {
                            { // variants of the instruction
                                AsmInstructionVariant(lea, Options::None,
                                    AsmOpDefinition(Location::reg, 8, 8, false, true),
                                    AsmOpDefinition(Location::off, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, true, false, false),
                                        RegisterRange(R8, R15, true, true, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  2, sourceOp),
                                        AsmInstructionResultInput(false,  reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(lea, Options::None,
                                    AsmOpDefinition(Location::reg, 8, 8, false, true),
                                    AsmOpDefinition(Location::sta, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, false, false, false),
                                        RegisterRange(R8, R15, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  2, sourceOp),
                                        AsmInstructionResultInput(false,  reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                }),
                                AsmInstructionVariant(lea, Options::None,
                                    AsmOpDefinition(Location::reg, 8, 8, false, true),
                                    AsmOpDefinition(Location::mem, 1, 8, true, false),
                                    { // allowed registers
                                        RegisterRange(RAX, RDI, true, false, false, false),
                                        RegisterRange(R8, R15, true, false, false, false)
                                    },
                                    { // inputs and outputs
                                        AsmInstructionResultInput(true,  2, sourceOp),
                                        AsmInstructionResultInput(false,  reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                        AsmInstructionResultInput(false, 1, {current.op3(), context})
                                })
                            }};
                        ExecuteInstruction(context, processor, instruction, instructions, index);

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
                                    AsmOpDefinition(Location::sta, 1, 8, true, false),
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
                                    AsmOpDefinition(Location::sta, 1, 8, true, true),
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
                                    AsmOpDefinition(Location::sta, 1, 8, true, true),
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
                                    AsmOpDefinition(Location::sta, 1, 8, true, false),
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
                                    AsmOpDefinition(Location::sta, 1, 8, true, true),
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
                                    AsmOpDefinition(Location::sta, 1, 8, true, true),
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
                    if (n.content.op == Location::Variable) std::cout << " (value size: " << n.content.object(context, processor).type->typeSize << ")\n";
                    else std::cout << "\n";
                }
            }
        }

        PrintInstructions(instructions, out, maxOffset);
    }
}
