#include "GenerateCode.hpp"
#include "ParserVariables.hpp"
#include "StackVector.hpp"
#include "GeneratorSettings.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <utility>

namespace fs = std::filesystem;

// takes a mathematical expression and returns the value to put wherever the user wants
// output type refers to the expected type of value to come out, namely a signed, unsigned or float
std::string CalculateExpression(StackVector& variables, std::ofstream& out, uint16_t outputSize,
                                const ParserValue& expression, uint8_t outputType,
                                ValueType returnValueLocations = {1, 1, 1, 1}, RegisterNames registers = {}) {

    if (expression.nodeType == ParserValue::Node::empty) {
        // shouldn't happen afaik
        CodeError("Empty node in wrong place!");
    }

    // if given node is a constant just return it's value
    if (expression.nodeType == ParserValue::constant) {
        uint64_t number = ConvertValue(*expression.value, 64);
        if (returnValueLocations.val) {
            return "$" + std::to_string(number);
        }
        else if (returnValueLocations.reg) {
            out << "mov"  << AddInstructionPostfix(outputSize) << "    $" << std::to_string(number) << ", " << registers.registerBySize(outputSize) << "\n";
            return registers.registerBySize(outputSize);
        }
        else {
            CodeError("Unsupported value return type for node type!");
            return "";
        }
    }

    // if given node is a variable
    if (expression.nodeType == ParserValue::Node::variable) {
        auto& var = variables.find(*expression.value);
        if (not returnValueLocations.reg) {
            // strictly stack
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType, false, &variables);
        }
        if (not returnValueLocations.sta) {
            // strictly register
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType, true, nullptr, true, registers);
        }
        else {
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType);
        }
    }

    // if given node is an actual multi thingy expression
    if (expression.nodeType == ParserValue::Node::operation) {
        // for now functions are going to be separate
        if (expression.operationType == ParserValue::Operation::functionCall) {
            if (returnValueLocations.reg) {
                return GenerateFunctionCall(out, variables, *expression.value,
                                     outputSize,  outputType, registers);
            }
            else if (returnValueLocations.sta) {
                StackVariable var;
                var.singleSize = outputSize;
                var.amount = 1;
                const StackVariable& pushed = variables.push(var);
                return GenerateFunctionCall(out, variables, *expression.value,
                                     outputSize, outputType, pushed.getAddressAsRegisterNames());
            }
            else {
                CodeError("Unsupported function return type!");
            }
        }



        uint8_t largestSize = outputSize;
        bool leftLarger = false;
        bool rightLarger = false;
        if (expression.left->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.left->value);
            if (var.singleSize > largestSize) {
                largestSize = var.singleSize;
                leftLarger = true;
            }
        }
        if (expression.right->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.right->value);
            if (var.singleSize > largestSize) {
                largestSize = var.singleSize;
                rightLarger = true;
            }
        }

        std::string left = CalculateExpression(variables, out, largestSize, *expression.left, outputType, {0, 1, 1, 1});
        std::string right;
        if (expression.operationType == ParserValue::Operation::division) {
            right = CalculateExpression(variables, out, largestSize, *expression.right, outputType, {1, 0, 0, 0}, {"%bl", "%bx", "%ebx", "%rbx"});
        }
        else {
            right = CalculateExpression(variables, out, largestSize, *expression.right, outputType, {0, 1, 1, 1});
        }


        out << "mov" << AddInstructionPostfix(largestSize) << "    " << left << ", " << AddRegisterA(largestSize) << "\n";
        // freeing temp variables
        if (left[0] != '%' and left[0] != '$' and variables.findByOffset(left).name.empty()) {
            variables.free(left);
        }

        // operations really only change this part
        switch (expression.operationType) {
            case ParserValue::Operation::addition:
                out << "add" << AddInstructionPostfix(largestSize) << "    " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::subtraction:
                out << "sub" << AddInstructionPostfix(largestSize) << "    " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::multiplication:
                if (outputType == ParserValue::Value::unsignedInteger) {
                    out << "mul" << AddInstructionPostfix(largestSize) << "    " << right << "\n";
                }
                else if (outputType == ParserValue::Value::signedInteger) {
                    out << "imul" << AddInstructionPostfix(largestSize) << "   " << right << ", " << registers.registerBySize(largestSize) << "\n";
                }

                break;
            case ParserValue::Operation::division:
                if (outputType == ParserValue::Value::unsignedInteger) {
                    out << "movq    $0, %rdx\n";
                    out << "div" << AddInstructionPostfix(largestSize) << "    " << right << ", " << registers.registerBySize(largestSize) << "\n";
                }
                else if (outputType == ParserValue::Value::signedInteger) {
                    out << "movq    $0, %rdx\n";
                    out << "idiv" << AddInstructionPostfix(largestSize) << "   " << right << ", " << registers.registerBySize(largestSize) << "\n";
                }
                break;
            default:
                CodeError("Unsupported operation type!");
        }


        // freeing temp variables
        if (right[0] != '%' and right[0] != '$' and variables.findByOffset(right).name.empty()) {
            variables.free(right);
        }

        std::string reg = ConvertValueInRegister(out, largestSize, outputSize, outputType, outputType, registers);

        if (returnValueLocations.reg) {
            return reg;
        }
        else if (returnValueLocations.sta) {
            StackVariable var;
            var.singleSize = largestSize;
            var.amount = 1;

            std::string varAddress =  variables.pushAndStr(var);
            out << "mov" << AddInstructionPostfix(outputSize) << "    " << reg << ", " << varAddress << "\n";
            return varAddress;
        }
        else {
            CodeError("Unsupported value return type for node type!");
        }
    }

    CodeError("Unhandled expression path!");
    return "$0";
}

void GenerateFunction(const std::string& identifier, std::ofstream& out) {
    StackVector variables;
    const ParserFunction* function = &parserFunctions[identifier];

    if (setting::OutputDodoInstructions) {
        out << "\n" << setting::CommentCharacter << " Declaration of function: " << function->returnType << " " << identifier << "(...)\n";
    }

    if (identifier == "main") {
        out << "_start:\n";
    }
    else {
        out << identifier << ":\n";
    }
    if (TargetArchitecture == "X86-64") {
        out << "pushq   %rbp\n";
        out << "movq    %rsp, %rbp\n";
    }

    bool didReturn = false;
    for (const auto& current : function->instructions) {
        switch (current.type) {
            case FunctionInstruction::Type::declaration: {
                const DeclarationInstruction &dec = *current.Variant.declarationInstruction;
                if (setting::OutputDodoInstructions) {
                    if (dec.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = expression\n";
                    } else if (dec.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = constant\n";
                    } else if (dec.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = variable\n";
                    } else {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << "\n";
                    }
                }
                StackVariable var;
                const ParserType& type = parserTypes[dec.typeName];
                var.singleSize = type.size;
                var.amount = 1;
                var.name = dec.name;
                var.type = type.type;
                var.typeName = dec.typeName;
                var.isMutable = dec.isMutable;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    std::string source = CalculateExpression(variables, out, var.singleSize, dec.expression, type.type, {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", " << variables.pushAndStr(var) << "\n";
                }
                else {
                    // declaring with a zero to avoid trash or exploits(?), also for debugging
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    $0, " << variables.pushAndStr(var) << "\n";
                }
                break;
            }
            case FunctionInstruction::Type::returnValue: {
                const ReturnInstruction &ret = *current.Variant.returnInstruction;
                if (setting::OutputDodoInstructions) {
                    if (ret.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Expression return\n";
                    } else if (ret.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Constant return\n";
                    } else if (ret.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Variable return\n";
                    } else {
                        out << setting::CommentCharacter << " Valueless return\n";
                    }
                }
                if (identifier == "main") {
                    if (TargetArchitecture == "X86-64") {

                        std::string source = CalculateExpression(variables, out, 8, ret.expression, parserTypes[function->returnType].type);
                        out << "movq    " << source << ", %rdi\n";

                        // interrupt number
                        out << "movq    $60, %rax\n";
                        // call out to the kernel to end this misery
                        out << "syscall\n";
                    }
                } else {
                    if (TargetArchitecture == "X86-64") {
                        ParserType& type = parserTypes[function->returnType];

                        std::string source = CalculateExpression(variables, out, type.size, ret.expression, type.type);
                        if (source != "%rax") {
                            out << "mov" << AddInstructionPostfix(type.size) << "    " << source << ", " << AddRegisterA(type.size) << "\n";
                        }
                        out << "popq    %rbp\n";
                        out << "ret\n";
                    }
                }
                didReturn = true;
                break;
            }
            case FunctionInstruction::Type::valueChange:
            {
                const ValueChangeInstruction &val = *current.Variant.valueChangeInstruction;
                if (setting::OutputDodoInstructions) {
                    if (val.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name << " = expression\n";
                    } else if (val.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name << " = constant\n";
                    } else if (val.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name << " = variable\n";
                    } else {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name << "\n";
                    }
                }
                StackVariable &var = variables.find(val.name);
                const ParserType& type = parserTypes[var.typeName];
                if (not var.isMutable) {
                    CodeError("Variable: " + var.name + " is immutable!");
                }

                if (val.expression.nodeType != ParserValue::Node::empty) {
                    // TODO: add memory to memory move, movsq didn't work in visualizer so I didn't risk it
                    std::string source = CalculateExpression(variables, out, var.singleSize, val.expression, type.type,
                                                             {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                        << var.getAddress() << "\n";
                } else {
                    CodeError("No expression in value assignment!");
                }
                break;
            }
            case FunctionInstruction::Type::functionCall:
            {
                const FunctionCallInstruction &fun = *current.Variant.functionCallInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " Function call to: " << fun.functionName << "(...)\n";
                }

                GenerateFunctionCall(out, variables, fun.functionName, 8, 0);
                break;
            }
        }
    }

    if (not didReturn) {
        if (identifier == "main") {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $0, %rdi\n";
                out << "movq    $60, %rax\n";
                out << "syscall\n";
            }
        }
        else {
            if (function->returnType != "void" and not function->returnType.empty()) {
                // TODO: make this smarter
                CodeError("Value not returned at the end of a function!");
            }
        }
    }
}

void GenerateCode() {
    doneParsing = true;
    std::ofstream out;
    if (!fs::is_directory("build")) {
        fs::create_directory("build");
    }
    out.open("build/out.s");
    if (!out.is_open()) {
        CodeError("Failed to open output .asm file");
    }

    out << "# Generated by dodo lang compiler by SKNI \"KOD\"\n";
    out << "# Target architecture: " << TargetArchitecture << "\n";
    out << "# Target system: " << TargetSystem << "\n";

    out << ".data\n";
    // ...
    out << ".text\n";
    // ...
    out << ".global _start\n";
    // ...

    // TODO: add a check to see if given function is used at all
    for (auto& n : parserFunctions.map) {
        GenerateFunction(n.second.name, out);
    }
}


