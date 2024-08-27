#include <fstream>
#include "GenerateCode.hpp"
#include "ParserVariables.hpp"

std::string GenerateFunctionCall(std::ofstream& out, StackVector variables,
                                 const std::string& functionName, uint16_t outputSize, uint8_t outputType, RegisterNames outputLocation) {

    const ParserFunction& function = parserFunctions[functionName];
    uint64_t stackOffset = variables.lastOffset();
    if (functionName == "main") {
        CodeError("Illegal call to main!");
    }

    // get the offset aligned to 16
    if (stackOffset % 16 != 0) {
        stackOffset = (stackOffset / 16 + 1) * 16;
    }

    // change %rsp to the right value
    if (stackOffset > variables.registerOffset) {
        out << "subq    $" << (stackOffset - variables.registerOffset) << ", %rsp\n";
        variables.registerOffset = stackOffset;
    }
    else if (stackOffset < variables.registerOffset) {
        out << "addq    $" << (variables.registerOffset - stackOffset) << ", %rsp\n";
        variables.registerOffset = stackOffset;
    }

    // call the function
    out << "call    " << functionName << "\n";

    // if return is needed do it
    if (function.returnValueType != ParserFunction::Subtype::none) {
        const ParserType& type = parserTypes[function.returnType];

        std::string convertedValue = ConvertValueInRegister(out, type.size, outputSize, type.type, outputType);
        // return the register if it's already there
        if (convertedValue == outputLocation.registerBySize(outputSize)) {
            return convertedValue;
        }

        // move the value to correct destination
        out << "mov" << AddInstructionPostfix(outputSize) << "    " << convertedValue << ", " << outputLocation.registerBySize(outputSize) << "\n";
        return outputLocation.registerBySize(outputSize);
    }

    return "";
}