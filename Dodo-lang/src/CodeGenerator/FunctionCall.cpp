#include <fstream>
#include "GenerateCode.hpp"
#include "ParserVariables.hpp"

std::string GenerateFunctionCall(std::ofstream& out, StackVector& variables,
                                 const std::string& functionName, uint16_t outputSize, uint8_t outputType, const ParserValue* arguments, RegisterNames outputLocation) {

    const ParserFunction& function = parserFunctions[functionName];


    // pushing arguments onto stack
    variables.addLevel();
    if (not function.arguments.empty()) {
        for (uint64_t n = 0; n < function.arguments.size(); n++) {
            if (arguments == nullptr) {
                CodeError("Function argument mismatch!");
            }
            if (arguments->right.get() == nullptr) {
                CodeError("Invalid argument value!");
            }
            const auto& target = parserTypes[function.arguments[n].typeName];
            std::string sourceRegister = CalculateExpression(variables, out, target.size, *arguments->right, target.type, {1, 0, 0, 0});
            StackVariable var;
            var.typeName = function.arguments[n].typeName;
            var.singleSize = target.size;
            var.type = target.type;
            var.amount = 1;
            var.isMutable = function.arguments[n].isMutable;
            variables.push(var);
            if (n == 0) {
                variables.alignTo16();
            }
            out << "mov" << AddInstructionPostfix(target.size) << "    " << sourceRegister << ", " << variables.vec.back().back().getAddress() << "\n";
            // get the next argument
            arguments = arguments->left.get();
        }
    }

    // calculating final stack offset
    int64_t stackOffset = variables.lastOffset();
    if (functionName == "main") {
        CodeError("Illegal call to main!");
    }

    // get the offset aligned to 16
    if (stackOffset % 16 != 0) {
        stackOffset = (stackOffset / 16 + 1) * 16;
    }

    // change %rsp to the right value
    if (stackOffset > variables.registerOffset) {
        out << "addq    $" << (stackOffset - variables.registerOffset) << ", %rsp\n";
        variables.registerOffset = stackOffset;
    }
    else if (stackOffset < variables.registerOffset) {
        out << "subq    $" << (variables.registerOffset - stackOffset) << ", %rsp\n";
        variables.registerOffset = stackOffset;
    }
    variables.registerOffset = stackOffset;
    // get rid of the level for arguments
    variables.popLevel();

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