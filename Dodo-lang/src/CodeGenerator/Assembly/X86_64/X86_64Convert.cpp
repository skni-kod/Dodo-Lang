#include "X86_64Convert.hpp"

#include "X86_64.hpp"

namespace x86_64 {
    void ConvertBytecode(BytecodeContext& context, Processor& processor, ParserFunctionMethod* source, std::ofstream& out) {
        std::vector <AsmInstruction> instructions;
        std::vector <AsmOperand> argumentPlaces;
        int32_t argumentOffset = 0;

        for (uint32_t index = 0; index < context.codes.size(); index++) {
            auto& current = context.codes[index];
            // uint16_t currentSize;
            // Type::TypeEnum currentType;
            // if (current.opTypeMeta.isReference or current.opTypeMeta.pointerLevel or not current.opType->isPrimitive) {
            //     currentSize = Options::addressSize;
            //     currentType = Type::address;
            // }
            // else {
            //     currentSize = current.opType->typeSize;
            //     currentType = current.opType->primitiveType;
            // }

            // TODO: add printing functions for AsmOperand
            if (Options::informationLevel >= Options::full) {
                std::cout << "INFO L3: Processor before instruction " << index << ":\nINFO L3: Registers:\n";
                for (auto& n : processor.registers) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: ";
                        PrintRegisterName(n.number, n.content.size, std::cout);
                        std::cout << "\n";
                    }
                }
                std::cout << "INFO L3: Stack:\n";
                for (auto& n : processor.stack) {
                    if (n.content.op != Location::None) {
                        std::cout << "INFO L3: " << n.offset << ": \n";
                    }
                }
            }

            switch (current.type) {
                case Bytecode::Define:
                    if (current.op3().location == Location::Argument) {
                        if (current.op3Value.ui == 0) {
                            auto [args, off] = GetFunctionMethodArgumentLocations(*source);
                            argumentPlaces = std::move(args);
                            argumentOffset = off;
                        }
                        if (argumentPlaces[current.op3Value.ui].op == Location::sta)
                            processor.stack.emplace(processor.stack.begin(),AsmOperand(current.op1(), context),
                            argumentPlaces[current.op3Value.ui].value.offset, argumentPlaces[current.op3Value.ui].size);

                        processor.getContentRef(argumentPlaces[current.op3Value.ui]) = AsmOperand(current.op1(), context);
                    }
                    else processor.pushStack(current.op1(), context);
                    break;
                case Bytecode::Return: {
                    AsmInstructionInfo instruction;
                    // TODO: add return type check for floats
                    instruction = {{
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
                default:
                    CodeGeneratorError("Unhandled bytecode type in x86-64 converter!");
            }

            // TODO: add removing old values here
        }

        PrintInstructions(instructions, out);
    }
}
