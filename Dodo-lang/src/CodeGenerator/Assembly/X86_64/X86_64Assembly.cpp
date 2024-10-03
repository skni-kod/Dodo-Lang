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
                // move returned value into register a
                MoveValue(bytecode.source, "%0");
                // insert a return statement
                GenerateInstruction({x86_64::ret});
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
}