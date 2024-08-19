#include "GenerateCode.hpp"
#include "ParserVariables.hpp"
#include "StackVector.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

const char *CodeException::what() {
    return "Code generation failed!";
}

uint64_t ConvertNumber(const std::string& value) {
    // TODO: Make this support all the numeric types
    return std::stoll(value);
}

void CodeError(const std::string message) {
    std::cout << "ERROR! Code generation failed : " << message << "\n";
    throw new CodeException;
}

uint64_t GetTypeSize(const std::string& typeName) {
    if (parserTypes.isKey(typeName)) {
        return parserTypes[typeName].size;
    }
    else {
        CodeError("Variable of non-existent type!");
    }
}

void GenerateFunction(const std::string& identifier, std::ofstream& out) {
    if (identifier == "main") {
        out << "_start:\n";
        if (TargetArchitecture == "X86-64") {
            out << "pushq   %rbp\n";
            out << "movq    %rsp, %rbp\n";
        }
        else if (TargetArchitecture == "X86-32") {
            out << "pushd   %ebp\n";
            out << "movd    %esp, %ebp\n";
        }
    }
    else {
        out << identifier << ":\n";
        if (TargetArchitecture == "X86-64") {
            out << "pushq   %rbp\n";
            out << "movq    %rsp, %rbp\n";
        }
        else if (TargetArchitecture == "X86-32") {
            out << "pushd   %ebp\n";
            out << "movd    %esp, %ebp\n";
        }
    }
    StackVector variables;

    const ParserFunction* function = &parserFunctions[identifier];
    // return logic will be changed, probably
    bool didReturn = false;
    for (const auto& current : function->instructions) {
        switch (current.type) {
            case FunctionInstruction::Type::declaration:
            {
                const DeclarationInstruction& dec = *current.Variant.declarationInstruction;
                StackVariable var;
                var.size = GetTypeSize(dec.typeName);
                var.amount = 1;
                var.offset = variables.lastOffset() + (var.size * var.amount);
                var.name = dec.name;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    // TODO: Change this into a dedicated function with all the checks etc
                    if (dec.expression.nodeType == ParserValue::Node::constant) {
                        if (var.size > 4) {
                            if (var.offset % 8 != 0) {
                                var.offset = (var.offset / 8 + 1) * 8;
                            }
                            uint64_t number = ConvertNumber(*dec.expression.value);
                            out << "movq    $" << number << ", -" << var.offset << "(%rbp)\n";
                        }
                        else if (var.size > 2) {
                            if (var.offset % 4 != 0) {
                                var.offset = (var.offset / 4 + 1) * 4;
                            }
                            uint64_t number = ConvertNumber(*dec.expression.value);
                            if (number > 4294967295) {
                                CodeError("Invalid number for this size of variable!");
                            }
                            out << "movl    $" << number << ", -" << var.offset << "(%rbp)\n";
                        }
                        else if (var.size > 1) {
                            if (var.offset % 2 != 0) {
                                var.offset = (var.offset / 2 + 1) * 2;
                            }
                            uint64_t number = ConvertNumber(*dec.expression.value);
                            if (number > 65535) {
                                CodeError("Invalid number for this size of variable!");
                            }
                            out << "movw    $" << number << ", -" << var.offset << "(%rbp)\n";
                        }
                        else {
                            uint64_t number = ConvertNumber(*dec.expression.value);
                            if (number > 255) {
                                CodeError("Invalid number for this size of variable!");
                            }
                            out << "movb    $" << number << ", -" << var.offset << "(%rbp)\n";
                        }
                    }
                    else if (dec.expression.nodeType == ParserValue::Node::variable) {
                        // TODO: Add variable copying here or in the complete function idk,
                        //  also don't forget size check or maybe movb to rax ensures all is well?
                    }
                }
                else {
                    // declaring with a zero to avoid trash or exploits(?), also for debugging
                    if (var.size > 4) {
                        if (var.offset % 8 != 0) {
                            var.offset = (var.offset / 8 + 1) * 8;
                        }
                        out << "movq    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 2) {
                        if (var.offset % 4 != 0) {
                            var.offset = (var.offset / 4 + 1) * 4;
                        }
                        out << "movl    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 1) {
                        if (var.offset % 2 != 0) {
                            var.offset = (var.offset / 2 + 1) * 2;
                        }
                        out << "movw    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else {
                        out << "movb    $0, -" << var.offset << "(%rbp)\n";
                    }
                }
                variables.vec.push_back(var);
                break;
            }
            case FunctionInstruction::Type::returnValue:
                const ReturnInstruction& ret = *current.Variant.returnInstruction;
                // TODO: Change 0 to value
                if (identifier == "main") {
                    if (TargetArchitecture == "X86-64") {
                        // interrupt number
                        out << "movq    $60, %rax\n";
                        if (ret.expression.nodeType == ParserValue::Node::empty) {
                            // shouldn't happen afaik
                            CodeError("Empty return statement!");
                        }
                        if (ret.expression.nodeType == ParserValue::Node::constant) {
                            out << "movq    $" << ConvertNumber(*ret.expression.value) << ", %rdi\n";
                        }
                        else if (ret.expression.nodeType == ParserValue::Node::variable) {
                            auto& var = variables.find(*ret.expression.value);
                            if (var.size > 4) {
                                out << "movq    -" << var.offset << "(%rbp), %rdi\n";
                            }
                            else if (var.size > 2) {
                                // might be a stupid way
                                out << "movq    $0, %rdi\n";
                                out << "movl    -" << var.offset << "(%rbp), %edi\n";
                            }
                            else if (var.size > 1) {
                                // might be an even dumber way
                                out << "movq    $0, %rbx\n";
                                out << "movw    -" << var.offset << "(%rbp), %bx\n";
                                out << "movq    %rbx, %rdi\n";
                            }
                            else {
                                // might be an even dumber way
                                out << "movq    $0, %rbx\n";
                                out << "movb    -" << var.offset << "(%rbp), %bl\n";
                                out << "movq    %rbx, %rdi\n";
                            }
                        }
                        else {
                            // because complex expressions are not yet supported
                            out << "movq    $0, %rdi\n";
                        }
                        // call out to the kernel to end this misery
                        out << "syscall\n";
                    }
                    else if (TargetArchitecture == "X86-32") {
                        // 32 bit not yet supported
                        out << "movl    $1, %eax\n";
                        out << "movl    $0, %ebx\n";
                        out << "int     $80\n";
                    }
                }
                else {
                    if (TargetArchitecture == "X86-64") {
                        out << "movq    $0, %rax\n";
                        out << "popq    %rbp\n";
                        out << "ret\n";
                    }
                    else if (TargetArchitecture == "X86-32") {
                        out << "movl    $0, %eax\n";
                        out << "popl    %ebp\n";
                        out << "ret\n";
                    }
                }
                didReturn = true;
                break;
        }
    }

    if (not didReturn) {
        if (identifier == "main") {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $60, %rax\n";
                out << "movq    $0, %rdi\n";
                out << "syscall\n";
            }
            else if (TargetArchitecture == "X86-32") {
                out << "movl    $1, %eax\n";
                out << "movl    $0, %ebx\n";
                out << "int     $80\n";
            }
        }
        else {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $0, %rax\n";
                out << "popq    %rbp\n";
                out << "ret\n";
            }
            else if (TargetArchitecture == "X86-32") {
                out << "movl    $0, %eax\n";
                out << "popl    %ebp\n";
                out << "ret\n";
            }
        }
    }
}

void GenerateCode() {
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
    GenerateFunction("main", out);
}


