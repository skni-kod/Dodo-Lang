#include "X86_64Convert.hpp"

namespace x86_64 {
    void ConvertBytecode(BytecodeContext& context, Processor& processor, std::ofstream& out) {
        std::vector <AsmInstruction> instructions;

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

            switch (current.type) {
                case Bytecode::Return: {
                    AsmInstructionInfo instruction = {{
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
        }
    }
}