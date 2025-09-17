#include <GenerateCode.hpp>
#include <iostream>
#include <fstream>
#include <list>

#include "X86_64.hpp"
#include "X86_64Enums.hpp"

namespace x86_64 {
    void ConvertBytecode(Context& context, ParserFunctionMethod* source,
                         std::ofstream& out) {
        std::vector<AsmInstruction> instructions;
        std::vector<AsmOperand> argumentPlaces;
        int32_t argumentOffset = 0;
        int32_t maxOffset = 0;
        // first is offset, second is amount to move by, third is content, fourth is original offset
        std::stack<std::tuple<int64_t, int32_t, AsmOperand, int64_t>> listStack{};

        for (uint32_t index = context.index = 0; index < context.codes.size(); context.index = ++index) {
            uint32_t printingStart = instructions.size();

            auto& current = context.codes[index];
            uint16_t currentSize;
            Type::TypeEnum currentType;
            if (current.opType != nullptr and (current.opTypeMeta.isReference or current.opTypeMeta.pointerLevel or not
                current.opType->isPrimitive)) {
                currentSize = Options::addressSize;
                currentType = Type::address;
            }
            else if (current.opType != nullptr) {
                currentSize = current.opType->typeSize;
                currentType = current.opType->primitiveType;
            }

            if (Options::informationLevel >= Options::full) {
                std::cout << "INFO L3: Processor before instruction " << index << " - " << context.codes[index] << "INFO L3: Registers:\n";
                for (auto& n : context.registers) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: ";
                        PrintRegisterName(n.number, n.content.size, std::cout);
                        std::cout << ": ";
                        n.content.print(std::cout, context);
                        std::cout << "\n";
                    }
                }
                std::cout << "INFO L3: Stack:\n";
                for (auto& n : context.stack) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: " << n.offset << ": ";
                        n.content.print(std::cout, context);
                        if (n.content.op == Location::Variable) std::cout << " (value size: " << n.content.size * 1 <<
                            ")\n";
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
                        context.stack.emplace_back(AsmOperand(current.op1(), context),
                                                     argumentPlaces[current.op3Value.ui].value.offset,
                                                     argumentPlaces[current.op3Value.ui].size);

                    context.getContentRef(argumentPlaces[current.op3Value.ui]) = AsmOperand(current.op1(), context);
                }
                else {
                    context.pushStack(current.op1());
                }
                break;
            case Bytecode::Return: {
                AsmInstructionInfo instruction;
                if (source->returnType.typeName != nullptr) {
                    if (source->parentType == nullptr) source->parentType = &types[*source->returnType.typeName];
                    if (not source->returnType.type.pointerLevel and not source->returnType.type.isReference and
                        source->parentType->isPrimitive and source->parentType->primitiveType == Type::floatingPoint)
                        instruction = {
                            {
                                // variants of the instruction:
                                AsmInstructionVariant(ret, Options::None,
                                                      // operands:
                                                      // non operand values that need to be put into places:
                                                      {
                                                          AsmInstructionResultInput(
                                                              true, AsmOperand(
                                                                  current.op1(), context, Location::reg, XMM0),
                                                              AsmOperand(current.op1(), context))
                                                      })
                            },
                            // data for the operation itself: source 1, source 2, source 3, destination
                            AsmOperand(current.op1(), context)
                        };
                    else
                        instruction = {
                            {
                                // variants of the instruction:
                                AsmInstructionVariant(ret, Options::None,
                                                      // operands:
                                                      // non operand values that need to be put into places:
                                                      {
                                                          AsmInstructionResultInput(
                                                              true, AsmOperand(
                                                                  current.op1(), context, Location::reg, RAX),
                                                              AsmOperand(current.op1(), context))
                                                      })
                            },
                            // data for the operation itself: source 1, source 2, source 3, destination
                            AsmOperand(current.op1(), context)
                        };
                }
                else {
                    instruction = {
                        {
                            // variants of the instruction:
                            AsmInstructionVariant(ret, Options::None,
                                                  // operands:
                                                  // non-operand values that need to be put into places:
                                                  {
                                                  })
                        },
                        // data for the operation itself: source 1, source 2, source 3, destination
                    };
                }
                ExecuteInstruction(context, instruction, instructions, index);

                break;
            }
            case Bytecode::AssignTo: {
                auto sourceOp = AsmOperand(current.op1(), context);
                auto valueOp = AsmOperand(current.op2(), context);

                bool skip = false;

                if (sourceOp.object(context).lastUse <= index) skip = true;

                // let's find out what the value is
                if (valueOp.op == Location::Variable) {
                    valueOp = context.getLocation(valueOp);
                }

                // now let's assign the location or value to the variable
                if (not skip) context.assignVariable(sourceOp, valueOp, instructions);
            }
            break;
            case Bytecode::AssignAt: {
                auto sourceOp = AsmOperand(current.op1(), context);
                auto valueOp = AsmOperand(current.op2(), context);
                auto resultOp = AsmOperand(current.op3(), context);

                bool assignResult = false;

                assignResult = resultOp.object(context).lastUse > index;

                if (valueOp.op == Location::Variable) {
                    if (assignResult and sourceOp.object(context).lastUse > index) {
                        // assigning result but the value is here
                        CodeGeneratorError("Internal: assign at result not implemented!");
                    }
                    else if (assignResult) {
                        CodeGeneratorError("Internal: assign at result not implemented!");
                    }
                    valueOp = context.getLocation(valueOp);
                }

                sourceOp = context.getLocation(sourceOp);

                // now we need to generate a move from the value to the address in source
                if (sourceOp.op == Location::reg) {
                    sourceOp.op = Location::off;
                    sourceOp.value.regOff.addressRegister = sourceOp.value.reg;
                    sourceOp.value.regOff.offset = 0;
                    sourceOp.value.regOff.indexRegister = NO_REGISTER_IN_OFFSET;
                    sourceOp.type = Type::address;
                    sourceOp.size = Options::addressSize;
                }
                else if (sourceOp.op != Location::off) CodeGeneratorError("Internal: non-register address!");

                MoveInfo move = {valueOp, sourceOp};
                x86_64::AddConversionsToMove(move, context, instructions, {}, nullptr);
            }
            break;
            case Bytecode::Jump:
                instructions.emplace_back(jmp, AsmOperand(Location::Label, Type::none, false, AsmOperand::jump,
                                                          current.op1Value));
                break;
            case Bytecode::Label:
                instructions.emplace_back(
                    label, AsmOperand(Location::Label, Type::none, false, AsmOperand::jump, current.op1Value));
                break;
            case Bytecode::BeginScope:
                memorySnapshots.emplace(std::move(context.createSnapshot()));
                break;
            case Bytecode::EndScope:
                context.restoreSnapshot(memorySnapshots.top(), instructions);
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
                        while (not(context.codes[++index].type == Bytecode::EndScope and scopes == 1)) {
                            if (context.codes[index].type == Bytecode::BeginScope) scopes++;
                            else if (context.codes[index].type == Bytecode::EndScope) scopes--;
                        }
                    }
                }
                else {
                    if (condition.op == Location::Variable) condition = context.getLocation(condition);
                    auto falseLabel = AsmOperand(Location::Label, Type::none, false, AsmOperand::jump,
                                                 current.op2Value);

                    // now since it's unoptimized, we just need to check if the condition is 0 and then jump to the false condition label
                    // TODO: add non-explicit condition
                    instructions.emplace_back(cmp, condition,
                                              AsmOperand(Location::Literal, Type::unsignedInteger, false, 1, 0));
                    instructions.emplace_back(je, falseLabel);
                }
            }
            break;
            case Bytecode::Greater:
            case Bytecode::GreaterEqual:
            case Bytecode::Lesser:
            case Bytecode::LesserEqual:
            case Bytecode::Equals:
            case Bytecode::NotEqual: {
                // we need to generate a cmp between the operands
                auto left = AsmOperand(current.op1(), context);
                auto right = AsmOperand(current.op2(), context);
                auto result = AsmOperand(current.op3(), context);

                if (left.type != right.type) CodeGeneratorError("Internal: incompatible type comparison!");
                if (left.type == Type::floatingPoint and left.size == 4) {
                    // for now this will be the unoptimized version that returns a 0 or 1
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(comiss, Options::None,
                                                  AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                  AsmOpDefinition(Location::sta, 4, 4, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comiss, Options::None,
                                                  AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                  AsmOpDefinition(Location::off, 4, 4, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false),
                                                      RegisterRange(RAX, RDI, false, true, false, false),
                                                      RegisterRange(R8, R15, false, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comiss, Options::None,
                                                  AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                  AsmOpDefinition(Location::mem, 4, 4, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comiss, Options::None,
                                                  AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                  AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  })
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else if (left.type == Type::floatingPoint and left.size == 8) {
                    // for now this will be the unoptimized version that returns a 0 or 1
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(comisd, Options::None,
                                                  AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                  AsmOpDefinition(Location::sta, 8, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comisd, Options::None,
                                                  AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                  AsmOpDefinition(Location::off, 8, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false),
                                                      RegisterRange(RAX, RDI, false, true, false, false),
                                                      RegisterRange(R8, R15, false, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comisd, Options::None,
                                                  AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                  AsmOpDefinition(Location::mem, 8, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(comisd, Options::None,
                                                  AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                  AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(XMM0, XMM31, true, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  })
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    // for now this will be the unoptimized version that returns a 0 or 1
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(cmp, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(cmp, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  }),
                            AsmInstructionVariant(cmp, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, left),
                                                      AsmInstructionResultInput(true, 2, right)
                                                  })
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                context.registers[FLAGS].content = {};

                // now we have the comparison done, so let's move its value into a register or memory location
                // TODO: add an option to use memory if no free register found
                AsmOperand reg = context.getFreeRegister(Type::unsignedInteger, 1);
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
                context.registers[reg.value.reg].content = result;
            }
            break;
            case Bytecode::Address: {
                // TODO: add move to stack
                auto sourceOp = AsmOperand(current.op1(), context);
                auto sourceLocation = sourceOp;

                if (sourceLocation.op == Location::Variable) sourceLocation = context.getLocationStackBias(
                    sourceLocation);

                if (sourceLocation.op != Location::sta) {
                    // TODO: make this a function
                    auto temp = context.pushStack(sourceOp);
                    auto move = MoveInfo(sourceLocation, temp);
                    x86_64::AddConversionsToMove(move, context, instructions, sourceOp);
                    sourceLocation = temp;
                }

                //CodeGeneratorError("Internal: non-stack pointer places not supported!");
                AsmOperand reg = context.getFreeRegister(Type::address, 8);

                AsmInstructionInfo instruction = {
                    {
                        // variants of the instruction
                        AsmInstructionVariant(lea, Options::None,
                                              AsmOpDefinition(Location::reg, 8, 8, false, true),
                                              AsmOpDefinition(Location::sta, 1, 8, true, false),
                                              {
                                                  // allowed registers
                                                  RegisterRange(RAX, RDI, true, false, false, false),
                                                  RegisterRange(R8, R15, true, false, false, false)
                                              },
                                              {
                                                  // inputs and outputs
                                                  AsmInstructionResultInput(true, 2, sourceLocation),
                                                  AsmInstructionResultInput(
                                                      false, reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                                  AsmInstructionResultInput(false, 1, {current.op3(), context})
                                              })
                    }
                };
                ExecuteInstruction(context, instruction, instructions, index);
            }
            break;
            case Bytecode::ToReference: {
                auto sourceOp = AsmOperand(current.op1(), context);
                auto resultOp = AsmOperand(current.op3(), context);


                if (sourceOp.op == Location::Variable) {
                    // checking if the source is used later
                    auto obj = context.getVariableObject(current.op1());
                    sourceOp = context.getLocation(sourceOp);
                    if (obj.lastUse > index) {
                        AsmOperand reg = context.getFreeRegister(Type::address, 8);
                        MoveInfo move{sourceOp, reg};
                        x86_64::AddConversionsToMove(move, context, instructions, resultOp);
                    }
                    else {
                        // should work?
                        context.getContentRef(sourceOp) = resultOp;
                    }
                }
            }
            break;
            case Bytecode::Dereference: {
                auto sourceOp = AsmOperand(current.op1(), context);
                auto resultOp = AsmOperand(current.op3(), context);

                if (sourceOp.op == Location::var) sourceOp = context.getLocation(sourceOp);

                auto resultLocation = context.getFreeRegister(resultOp.type, resultOp.size);
                context.getContentRef(resultLocation) = resultOp;

                if (sourceOp.op != Location::reg) {
                    auto sourceContent = sourceOp.op != Location::var ? context.getContent(sourceOp) : sourceOp;
                    MoveInfo move = {sourceOp, sourceOp = context.getFreeRegister(sourceOp.type, sourceOp.size)};
                    x86_64::AddConversionsToMove(move, context, instructions, sourceContent);
                }

                AsmOperand src{};
                src.op = Location::off;
                src.type = resultOp.type;
                src.size = resultOp.size;
                src.value.regOff.indexRegister = NO_REGISTER_IN_OFFSET;
                src.value.regOff.addressRegister = sourceOp.value.reg;

                auto move = MoveInfo(src, resultLocation);
                x86_64::AddConversionsToMove(move, context, instructions, resultOp);
            }
            break;
            case Bytecode::GetIndexAddress: {
                auto arrayOp = AsmOperand(current.op1(), context);
                auto indexOp = AsmOperand(current.op2(), context);
                auto resultOp = AsmOperand(current.op3(), context);

                auto arrayLocation = context.getLocationStackBias(arrayOp);
                //if (arrayLocation.op == Location::reg) {
                //    CodeGeneratorError("Getting indexes from arrays in registers unsupported!");
                //}

                // now getting a register to put the extracted data into
                auto resultLocation = context.getFreeRegister(resultOp.type, resultOp.size);
                context.getContentRef(resultLocation) = resultOp;

                AsmOperand src{};
                src.op = Location::off;
                src.type = resultOp.type;
                src.size = resultOp.size;

                if (indexOp.op == Location::imm) {
                    src.value.regOff.addressRegister = arrayLocation.value.reg;
                    src.value.regOff.offset = resultOp.size * indexOp.value.u32;
                }
                else if (indexOp.op == Location::var) {
                    auto indexLocation = context.getLocationRegisterBias(indexOp);

                    // if the index variable is not
                    if (indexLocation.op != Location::reg) {
                        auto newLocation = context.getFreeRegister(indexLocation.type, indexLocation.size);
                        // this needs to be upsized to the whole register so that it matches the address size
                        newLocation.size = Options::addressSize;
                        auto move = MoveInfo(indexLocation, newLocation);
                        x86_64::AddConversionsToMove(move, context, instructions, indexOp);
                        indexLocation = newLocation;
                    }


                    if (arrayLocation.op == Location::reg) {
                        src.value.regOff.addressRegister = arrayLocation.value.reg;
                    }
                    else CodeGeneratorError("Internal: Unimplemented array location!");

                    src.value.regOff.indexRegister = indexLocation.value.reg;
                    src.value.regOff.indexScale = resultOp.size;
                }
                else CodeGeneratorError("Internal: unsupported data type for index get!");

                AsmInstruction ins{};
                ins.code = lea;
                ins.op1 = resultLocation;
                ins.op2 = src;
                instructions.push_back(ins);
            }
            break;

            case Bytecode::GetIndexValue: {
                auto arrayOp = AsmOperand(current.op1(), context);
                auto indexOp = AsmOperand(current.op2(), context);
                auto resultOp = AsmOperand(current.op3(), context);

                auto arrayLocation = context.getLocation(arrayOp);
                if (arrayLocation.op != Location::reg) {
                    auto newLocation = context.getFreeRegister(arrayLocation.type, arrayLocation.size);
                    auto move = MoveInfo(arrayLocation, newLocation);
                    x86_64::AddConversionsToMove(move, context, instructions, arrayOp);
                    arrayLocation = newLocation;
                }

                // now getting a register to put the extracted data into
                auto resultLocation = context.getFreeRegister(resultOp.type, resultOp.size);
                context.getContentRef(resultLocation) = resultOp;

                if (indexOp.op == Location::imm) {
                    AsmOperand src{};
                    src.op = Location::off;
                    src.type = resultOp.type;
                    src.size = resultOp.size;

                    src.value.regOff.addressRegister = arrayLocation.value.reg;
                    src.value.regOff.offset = resultOp.size * indexOp.value.u32;

                    auto move = MoveInfo(src, resultLocation);
                    x86_64::AddConversionsToMove(move, context, instructions, resultOp);
                }
                else if (indexOp.op == Location::var) {
                    auto indexLocation = context.getLocationRegisterBias(indexOp);

                    // if the index variable is not
                    if (indexLocation.op != Location::reg) {
                        auto newLocation = context.getFreeRegister(indexLocation.type, indexLocation.size);
                        // this needs to be upsized to the whole register so that it matches the address size
                        newLocation.size = Options::addressSize;
                        auto move = MoveInfo(indexLocation, newLocation);
                        x86_64::AddConversionsToMove(move, context, instructions, indexOp);
                        indexLocation = newLocation;
                    }


                    // and doing the move itself
                    AsmOperand src{};
                    src.op = Location::off;
                    src.type = resultOp.type;
                    src.size = resultOp.size;

                    if (arrayLocation.op == Location::reg) {
                        src.value.regOff.addressRegister = arrayLocation.value.reg;
                    }
                    else CodeGeneratorError("Internal: Unimplemented array location!");

                    src.value.regOff.indexRegister = indexLocation.value.reg;
                    src.value.regOff.indexScale = resultOp.size;

                    auto move = MoveInfo(src, resultLocation);
                    x86_64::AddConversionsToMove(move, context, instructions, resultOp);
                }
                else CodeGeneratorError("Internal: unsupported data type for index get!");
            }
            break;
            case Bytecode::Cast: {
                auto sourceOp = AsmOperand(current.op1(), context);
                auto resultOp = AsmOperand(current.op3(), context);

                if (sourceOp.object(context).lastUse <= index) {
                    auto loc = context.getLocation(sourceOp);
                    AsmOperand targetLoc;
                    if ((sourceOp.type == Type::floatingPoint or resultOp.type == Type::floatingPoint) and sourceOp.type
                        != resultOp.type) targetLoc = context.getFreeRegister(resultOp.type, resultOp.size);
                    else targetLoc = loc;
                    MoveInfo move = {
                        sourceOp.copyTo(loc.op, loc.value), resultOp.copyTo(targetLoc.op, targetLoc.value)
                    };
                    x86_64::AddConversionsToMove(move, context, instructions, resultOp, nullptr);
                }
                else {
                    auto loc = context.getFreeRegister(resultOp.type, resultOp.size);
                    auto sloc = context.getLocation(sourceOp);
                    MoveInfo move = {sloc, resultOp.copyTo(loc.op, loc.value)};
                    x86_64::AddConversionsToMove(move, context, instructions, resultOp, nullptr);
                }
            }
            break;
            case Bytecode::LoopLabel:
                break;

            case Bytecode::BraceListElement:
            case Bytecode::BraceListEnd:
            case Bytecode::BraceListStart: {


                    if (current.type == Bytecode::BraceListStart) {

                        auto place = context.pushStack(AsmOperand(current.op3(), context), current.op1Value.i32);
                        listStack.emplace(place.value.offset, current.opTypeMeta.pointerLevel or current.opTypeMeta.isReference ? Options::addressSize : current.opType->typeAlignment, AsmOperand(current.op3(), context), place.value.offset);
                        break;
                    }
                    if (current.type == Bytecode::BraceListEnd) {
                        listStack.pop();
                        break;
                    }

                    AsmOperand target = AsmOperand(Location::sta, current.op1LiteralType, false, current.op1LiteralSize, std::get<0>(listStack.top()));
                    std::get<0>(listStack.top()) += std::get<1>(listStack.top());
                    MoveInfo move = {AsmOperand(current.op1(), context), target};
                    move.source = context.getLocationStackBias(move.source);
                    move.target.type = move.source.type;



                    x86_64::AddConversionsToMove(move, context, instructions, std::get<2>(listStack.top()), nullptr, false);

            }
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
                    auto [args, off] = GetFunctionMethodArgumentLocations(arguments, context);
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
                    MoveInfo move = {
                        context.getLocation(s),
                        argumentPlaces[n].moveAwayOrGetNewLocation(context, instructions, firstIndex,
                                                                   nullptr, true)
                    };
                    x86_64::AddConversionsToMove(move, context, instructions, s, nullptr);
                }

                // now that the arguments are in place, we need to move away the ones that would get overwritten
                // TODO: add caller and callee saved registers
                for (auto& n : context.registers) {
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
                    auto& obj = n.content.object(context);
                    if (obj.lastUse > index and context.getLocationStackBias(n.content).op != Location::sta) {
                        // it exists if it has a stack location, then it's fine.
                        // if it has none, then we need to move it from the register to the stack
                        n.content.copyTo(Location::reg, n.number).moveAwayOrGetNewLocation(
                            context, instructions, index, nullptr, true);
                        //MoveInfo move = {n.content.copyTo(Location::reg, n.number), context.pushStack(n.content, context)};
                        //x86_64::AddConversionsToMove(move, context, instructions, n.content, nullptr);
                    }

                    n.content = {};
                }

                // now doing the call itself and setting return value
                if (context.codes[index].type == Bytecode::Syscall) {
                    instructions.emplace_back(mov,
                                              AsmOperand(Location::reg, Type::unsignedInteger, false, 8, RAX),
                                              AsmOperand(Location::Literal, Type::unsignedInteger, false, 8,
                                                         context.codes[index].op1Value.ui)
                    );
                    instructions.emplace_back(syscall);
                }
                else {
                    instructions.emplace_back(call, AsmOperand(Location::Label, Type::none, true,
                                                               AsmOperand::LabelType::function,
                                                               context.codes[index].op1Value));
                }

                // removing saved values from argument registers
                for (auto& n : argumentPlaces) {
                    if (n.op == Location::reg) context.registers[n.value.reg].content = {};
                }

                auto result = AsmOperand(context.codes[index].op3(), context);
                if (result.op != Location::None) {
                    if (result.type == Type::floatingPoint) context.registers[XMM0].content = result;
                    else context.registers[RAX].content = result;
                }
            }
            break;

            case Bytecode::Member: {
                auto sourceOp = AsmOperand(current.op1(), context);
                int32_t offset = current.op2Value.i32;
                auto resultOp = AsmOperand(current.op3(), context);

                if (sourceOp.op == Location::Variable) sourceOp = context.getLocation(sourceOp);

                // let's get a free register for the pointer for lea
                AsmOperand reg = context.getFreeRegister(Type::address, 8);

                // if it's a register
                // TODO: add more checks
                if (sourceOp.op == Location::reg) {
                    //if (not sourceOp.useAddress) CodeGeneratorError("Using register address not marked as used!");
                    sourceOp.op = Location::off;
                    sourceOp.value.regOff.addressRegister = sourceOp.value.reg;
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
                    {
                        // variants of the instruction
                        AsmInstructionVariant(lea, Options::None,
                                              AsmOpDefinition(Location::reg, 8, 8, false, true),
                                              AsmOpDefinition(Location::off, 1, 8, true, false),
                                              {
                                                  // allowed registers
                                                  RegisterRange(RAX, RDI, true, true, false, false),
                                                  RegisterRange(R8, R15, true, true, false, false)
                                              },
                                              {
                                                  // inputs and outputs
                                                  AsmInstructionResultInput(true, 2, sourceOp),
                                                  AsmInstructionResultInput(
                                                      false, reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                                  AsmInstructionResultInput(false, 1, {current.op3(), context})
                                              }),
                        AsmInstructionVariant(lea, Options::None,
                                              AsmOpDefinition(Location::reg, 8, 8, false, true),
                                              AsmOpDefinition(Location::sta, 1, 8, true, false),
                                              {
                                                  // allowed registers
                                                  RegisterRange(RAX, RDI, true, false, false, false),
                                                  RegisterRange(R8, R15, true, false, false, false)
                                              },
                                              {
                                                  // inputs and outputs
                                                  AsmInstructionResultInput(true, 2, sourceOp),
                                                  AsmInstructionResultInput(
                                                      false, reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                                  AsmInstructionResultInput(false, 1, {current.op3(), context})
                                              }),
                        AsmInstructionVariant(lea, Options::None,
                                              AsmOpDefinition(Location::reg, 8, 8, false, true),
                                              AsmOpDefinition(Location::mem, 1, 8, true, false),
                                              {
                                                  // allowed registers
                                                  RegisterRange(RAX, RDI, true, false, false, false),
                                                  RegisterRange(R8, R15, true, false, false, false)
                                              },
                                              {
                                                  // inputs and outputs
                                                  AsmInstructionResultInput(true, 2, sourceOp),
                                                  AsmInstructionResultInput(
                                                      false, reg, AsmOperand(Location::op, {}, false, 8, 1)),
                                                  AsmInstructionResultInput(false, 1, {current.op3(), context})
                                              })
                    }
                };
                ExecuteInstruction(context, instruction, instructions, index);
            }

            break;
            case Bytecode::Multiply:
                if (currentType == Type::signedInteger) {
                    // TODO: add support for single operand imul
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(imul, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(imul, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, true, false, false),
                                                      RegisterRange(R8, R15, true, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            //AsmInstructionVariant(imul, Options::None,
                            //    AsmOpDefinition(Location::reg, 1, 8, false, true),
                            //    AsmOpDefinition(Location::reg, 1, 8, true, false),
                            //    AsmOpDefinition(Location::imm, 1, 8, true, false),
                            //    { // allowed registers
                            //        RegisterRange(RAX, RDI, true, true, false, false),
                            //        RegisterRange(R8, R15, true, true, false, false)
                            //    },
                            //    { // inputs and outputs
                            //        AsmInstructionResultInput(true,  2, {current.op1(), context}),
                            //        AsmInstructionResultInput(true,  3, {current.op2(), context}),
                            //        AsmInstructionResultInput(false, 1, {current.op3(), context})
                            //}),
                            //AsmInstructionVariant(imul, Options::None,
                            //    AsmOpDefinition(Location::reg, 1, 8, false, true),
                            //    AsmOpDefinition(Location::sta, 1, 8, true, false),
                            //    AsmOpDefinition(Location::imm, 1, 8, true, false),
                            //    { // allowed registers
                            //        RegisterRange(RAX, RDI, true, false, false, false),
                            //        RegisterRange(R8, R15, true, false, false, false)
                            //    },
                            //    { // inputs and outputs
                            //        AsmInstructionResultInput(true,  2, {current.op1(), context}),
                            //        AsmInstructionResultInput(true,  3, {current.op2(), context}),
                            //        AsmInstructionResultInput(false, 1, {current.op3(), context})
                            //})
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    CodeGeneratorError("Unsigned and floating point multiplication is not supported yet!");
                }
                break;
            case Bytecode::Divide:
                if (currentType == Type::signedInteger) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(idiv, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op3(), context}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RDX), {})
                                                  }),
                            AsmInstructionVariant(idiv, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {},
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op3(), context}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RDX), {})
                                                  }),
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                    auto generated = instructions.back();
                    // we also need to generate the correct rdx or ah sign extension value before division
                    switch (current.op2().size) {
                    case 1: instructions.back() = AsmInstruction(cbw);
                        break;
                    case 2: instructions.back() = AsmInstruction(cwd);
                        break;
                    case 4: instructions.back() = AsmInstruction(cdq);
                        break;
                    case 8: instructions.back() = AsmInstruction(cqo);
                        break;
                    default: CodeGeneratorError("Internal: invalid signed division size!");
                    }
                    instructions.push_back(generated);
                }
                else if (currentType == Type::unsignedInteger) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(div, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {
                                                              Location::imm, Type::unsignedInteger, false,
                                                              current.op2().size, 0
                                                          }),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op3(), context}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {})
                                                  }),
                            AsmInstructionVariant(div, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {},
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {
                                                              Location::imm, Type::unsignedInteger, false,
                                                              current.op2().size, 0
                                                          }),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op3(), context}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {})
                                                  }),
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    CodeGeneratorError("Floating point division is not yet supported!");
                }
                break;
            case Bytecode::Modulo:
                if (currentType == Type::signedInteger) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(idiv, Options::None,
                                                  AsmOpDefinition(Location::reg, 2, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RDX), {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(idiv, Options::None,
                                                  AsmOpDefinition(Location::sta, 2, 8, true, false),
                                                  {},
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RAX), {}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::signedInteger, false,
                                                              current.op2().size, RDX), {current.op3(), context})
                                                  }),
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                    auto generated = instructions.back();
                    // we also need to generate the correct rdx or ah sign extension value before division
                    switch (current.op2().size) {
                    case 1: instructions.back() = AsmInstruction(cbw);
                        break;
                    case 2: instructions.back() = AsmInstruction(cwd);
                        break;
                    case 4: instructions.back() = AsmInstruction(cdq);
                        break;
                    case 8: instructions.back() = AsmInstruction(cqo);
                        break;
                    default: CodeGeneratorError("Internal: invalid signed division size!");
                    }
                    instructions.push_back(generated);
                }
                else if (currentType == Type::unsignedInteger) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(div, Options::None,
                                                  AsmOpDefinition(Location::reg, 2, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {
                                                              Location::imm, Type::unsignedInteger, false,
                                                              current.op2().size, 0
                                                          }),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(div, Options::None,
                                                  AsmOpDefinition(Location::sta, 2, 8, true, false),
                                                  {},
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {current.op1(), context}),
                                                      AsmInstructionResultInput(
                                                          true, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {
                                                              Location::imm, Type::unsignedInteger, false,
                                                              current.op2().size, 0
                                                          }),
                                                      AsmInstructionResultInput(true, 1, {current.op2(), context}),
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RAX), {}),
                                                      // remainder, unused for now
                                                      AsmInstructionResultInput(
                                                          false, AsmOperand(
                                                              Location::reg, Type::unsignedInteger, false,
                                                              current.op2().size, RDX), {current.op3(), context})
                                                  }),
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    CodeGeneratorError("Floating point modulo is not yet supported!");
                }
                break;
            case Bytecode::Add:
                if (currentType != Type::floatingPoint) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(add, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(add, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(add, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, true, false, false),
                                                      RegisterRange(R8, R15, true, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(add, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, true),
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, false, true, false, false),
                                                      RegisterRange(R8, R15, false, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(add, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, true),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  })
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    if (currentSize == 4) {
                        // single precision
                        AsmInstructionInfo instruction = {
                            {
                                // variants of the instruction
                                AsmInstructionVariant(addss, Options::None,
                                                      AsmOpDefinition(Location::reg, 4, 4, true, true),
                                                      AsmOpDefinition(Location::reg, 4, 4, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, true, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      }),
                                AsmInstructionVariant(addss, Options::None,
                                                      AsmOpDefinition(Location::reg, 4, 4, true, true),
                                                      AsmOpDefinition(Location::sta, 4, 4, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, false, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      })
                            }
                        };
                        ExecuteInstruction(context, instruction, instructions, index);
                    }
                    else {
                        // double precision
                        AsmInstructionInfo instruction = {
                            {
                                // variants of the instruction
                                AsmInstructionVariant(addsd, Options::None,
                                                      AsmOpDefinition(Location::reg, 8, 8, true, true),
                                                      AsmOpDefinition(Location::reg, 8, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, true, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      }),
                                AsmInstructionVariant(addsd, Options::None,
                                                      AsmOpDefinition(Location::reg, 8, 8, true, true),
                                                      AsmOpDefinition(Location::sta, 8, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, false, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      })
                            }
                        };
                        ExecuteInstruction(context, instruction, instructions, index);
                    }
                }
                break;
            case Bytecode::Subtract:
                if (currentType != Type::floatingPoint) {
                    AsmInstructionInfo instruction = {
                        {
                            // variants of the instruction
                            AsmInstructionVariant(sub, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(sub, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, false, false, false),
                                                      RegisterRange(R8, R15, true, false, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(sub, Options::None,
                                                  AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, true, true, false, false),
                                                      RegisterRange(R8, R15, true, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(sub, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, true),
                                                  AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                      RegisterRange(RAX, RDI, false, true, false, false),
                                                      RegisterRange(R8, R15, false, true, false, false)
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  }),
                            AsmInstructionVariant(sub, Options::None,
                                                  AsmOpDefinition(Location::sta, 1, 8, true, true),
                                                  AsmOpDefinition(Location::imm, 1, 8, true, false),
                                                  {
                                                      // allowed registers
                                                  },
                                                  {
                                                      // inputs and outputs
                                                      AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                      AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                      AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                  })
                        }
                    };
                    ExecuteInstruction(context, instruction, instructions, index);
                }
                else {
                    if (currentSize == 4) {
                        // single precision
                        AsmInstructionInfo instruction = {
                            {
                                // variants of the instruction
                                AsmInstructionVariant(subss, Options::None,
                                                      AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                      AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, true, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      }),
                                AsmInstructionVariant(subss, Options::None,
                                                      AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                      AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, false, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      })
                            }
                        };
                        ExecuteInstruction(context, instruction, instructions, index);
                    }
                    else {
                        // double precision
                        AsmInstructionInfo instruction = {
                            {
                                // variants of the instruction
                                AsmInstructionVariant(subsd, Options::None,
                                                      AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                      AsmOpDefinition(Location::reg, 1, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, true, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      }),
                                AsmInstructionVariant(subsd, Options::None,
                                                      AsmOpDefinition(Location::reg, 1, 8, true, true),
                                                      AsmOpDefinition(Location::sta, 1, 8, true, false),
                                                      {
                                                          // allowed registers
                                                          RegisterRange(XMM0, XMM31, true, false, false, false)
                                                      },
                                                      {
                                                          // inputs and outputs
                                                          AsmInstructionResultInput(true, 1, {current.op1(), context}),
                                                          AsmInstructionResultInput(true, 2, {current.op2(), context}),
                                                          AsmInstructionResultInput(false, 1, {current.op3(), context})
                                                      })
                            }
                        };
                        ExecuteInstruction(context, instruction, instructions, index);
                    }
                }
                break;
            default:
                CodeGeneratorError("Unhandled bytecode type in x86-64 converter!");
            }

            // checking stack offset before clearing to not remove very short-lived variables
            // TODO: maybe iterate to not include unused or something
            if (not context.stack.empty() and context.stack.back().offset < maxOffset) maxOffset = context.stack.back().offset;
            context.cleanUnusedVariables();
            if (Options::informationLevel >= Options::full) {
                for (; printingStart < instructions.size(); printingStart++) {
                    std::cout << "INFO L3: Generated instruction: ";
                    x86_64::PrintInstruction(instructions[printingStart], std::cout);
                }
            }
        }

        if (Options::informationLevel >= Options::full) {
            std::cout << "INFO L3: Processor after the last instruction:\nINFO L3: Registers:\n";
            for (auto& n : context.registers) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: ";
                    PrintRegisterName(n.number, n.content.size, std::cout);
                    std::cout << ": ";
                    n.content.print(std::cout, context);
                    std::cout << "\n";
                }
            }
            std::cout << "INFO L3: Stack:\n";
            for (auto& n : context.stack) {
                if (n.content.op != Location::None) {
                    std::cout << "INFO L3: " << n.offset << ": ";
                    n.content.print(std::cout, context);
                    if (n.content.op == Location::Variable) std::cout << " (value size: " << n.content.
                        object(context).type->typeSize << ")\n";
                    else std::cout << "\n";
                }
            }
        }

        PrintInstructions(instructions, out, maxOffset);
    }
}
