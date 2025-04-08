#include "X86_64Convert.hpp"

namespace x86_64 {
    void ConvertBytecode(BytecodeContext& context, Processor& processor, std::ofstream& out) {
        std::vector <AsmInstruction> instructions;

        for (uint32_t index = 0; index < context.codes.size(); index++) {
            auto& current = context.codes[index];

            switch (current.type) {
                case Bytecode::Return:
                    // TODO: resolve return type info here and pass it on

                    //ExecuteInstruction(context, processor, {
                    //    {
                    //        AsmInstructionVariant(ret, Options::None, {
                    //            AsmInstructionResultInput(true, AsmOperand(Location::reg, Type::none,
                    //                ((current.opTypeMeta.isReference + current.opTypeMeta.pointerLevel) or not current.opType->isPrimitive ? Options::addressSize : current.opType->typeSize), current.op1()), current.op1())
                    //        })
                    //    }
                    //});

                    break;
                default:
                    CodeGeneratorError("Unhandled bytecode type in x86-64 converter!");
            }
        }
    }
}