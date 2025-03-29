#include "BytecodeInternal.hpp"

std::ostream& operator<<(std::ostream& out, const Bytecode& code) {
  out << "Instruction: ";
  switch (code.type) {
        case Bytecode::None:
            out << "";
            break;
        case Bytecode::Define:
            out << "define " << code.op1() << " of type " << code.opType->typeName;
            if (code.op3Location != Location::None) out << " using " << code.result();
            break;
        case Bytecode::Assign:
            out << "assign " << code.result() << " to " << code.op1();
            break;
        case Bytecode::Address:
            out << "load address of " << code.op1() << " to " << code.result();
            break;
        case Bytecode::Dereference:
            out << "load value at address in " << code.op1() << " to " << code.result();
            break;
        case Bytecode::Index:
            out << "load value at index " << code.op2() << " of " << code.op1() << " to " << code.result();
            break;
        case Bytecode::Function:
            out << "function " << code.op1();
            if (code.op2Location != Location::None) out << " using first argument from " << code.op2();
            if (code.op3Location != Location::None) out << " and store result to " << code.result();
            break;
        case Bytecode::Method:
            out << "method " << code.op1() << " for type " << code.opType->typeName;
            if (code.op2Location != Location::None) out << " using first argument from " << code.op2();
            if (code.op3Location != Location::None) out << " and store result to " << code.result();
            break;
        case Bytecode::Syscall:
            out << "syscall " << code.op1();
            if (code.op2Location != Location::None) out << " using first argument from " << code.op2();
            if (code.op3Location != Location::None) out << " and store result to " << code.result() << " using type " << code.opType->typeName;
            break;
        case Bytecode::Argument:
            out << "argument";
            if (code.op2Location != Location::None) out << " with next argument from " << code.op2();
            out << " using value of " << code.result();
            break;
        case Bytecode::If:
            out << "if with condition " << code.op1();
            break;
        case Bytecode::Else:
            out << "else";
            break;
        case Bytecode::ElseIf:
            out << "else if with condition " << code.op1();
            break;
        case Bytecode::For:
            out << "for with begin statement " << code.op1() << ", loop statement " << code.op2() << ", after statement " << code.op3();
            break;
        case Bytecode::While:
            out << "while with condition " << code.op1();
            break;
        case Bytecode::Do:
            out << "do";
            break;
        case Bytecode::Switch:
            out << "switch with condition " << code.op1();
            break;
        case Bytecode::Case:
            out << "case with value " << code.op1();
            break;
        case Bytecode::GoTo:
            out << "goto";
            break;
        case Bytecode::Break:
            out << "break";
            break;
        case Bytecode::Continue:
            out << "continue";
            break;
        case Bytecode::BeginScope:
            out << "begin scope";
            break;
        case Bytecode::EndScope:
            out << "end scope";
            break;
        case Bytecode::Increment:
            out << "increment ";
            if (code.op1Location != Location::None) out << code.op1() << " after returning value to " << code.result();
            else
            if (code.op2Location != Location::None) out << code.op2() << " before returning value to " << code.result();
            break;
        case Bytecode::Decrement:
            out << "decrement ";
            if (code.op1Location != Location::None) out << code.op1() << " after returning value to " << code.result();
            else
            if (code.op2Location != Location::None) out << code.op2() << " before returning value to " << code.result();
            break;
        case Bytecode::Power:
            out << code.op1() << " to power of " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Multiply:
            out << "multiply " << code.op1() << " by " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Divide:
            out << "divide " << code.op1() << " by " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Modulo:
            out << "modulo " << code.op1() << " of " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Add:
            out << "multiply " << code.op1() << " by " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Subtract:
            out << "subtract " << code.op2() << " from " << code.op1() << " stored to " << code.result();
            break;
        case Bytecode::ShiftRight:
            out << "shift " << code.op1() << " right by " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::ShiftLeft:
            out << "shift " << code.op1() << " left by " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::NAnd:
            out << "logical NOT AND " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinNAnd:
            out << "binary NOT AND " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::And:
            out << "logical AND " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinAnd:
            out << "binary AND " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::XOr:
            out << "logical XOR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinXOr:
            out << "binary XOR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::NOr:
            out << "logical NOT OR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinNOr:
            out << "binary NOT OR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Or:
            out << "logical OR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinOr:
            out << "binary OR " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::NImply:
            out << "logical NOT IMPLY " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Imply:
            out << "logical IMPLY " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinNImply:
            out << "binary NOT IMPLY " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::BinImply:
            out << "binary IMPLY " << code.op1() << " with " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Lesser:
            out << "check if " << code.op1() << " is lesser than " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Greater:
            out << "check if " << code.op1() << " is greater than " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Equals:
            out << "check if " << code.op1() << " is equal to " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::LesserEqual:
            out << "check if " << code.op1() << " is equal or lesser than " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::GreaterEqual:
            out << "check if " << code.op1() << " is equal or greater than " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::NotEqual:
            out << "check if " << code.op1() << " is not equal to " << code.op2() << " stored to " << code.result();
            break;
        case Bytecode::Not:
            out << "logical NOT of " << code.op1();
            break;
        case Bytecode::BinNot:
            out << "binary NOT of " << code.op1();
            break;
        default:
            CodeGeneratorError("Unhandled bytecode instruction in printing!");
    }
  return out << "\n";
}

std::ostream& operator<<(std::ostream& out, const BytecodeOperand& code) {
    switch (code.location) {
        case Location::None:
            return out << "none";
        case Location::Variable:
            return out << "variable: " << code.value.variable;
        case Location::Literal:
            out << "literal: ";
            switch (code.literalType) {
                case Type::floatingPoint:
                    return out << code.value.fl;
                case Type::unsignedInteger:
                    return out << code.value.si;
                case Type::signedInteger:
                    return out << code.value.ui;
                default:
                    CodeGeneratorError("Unhandled type in literal in bytecode operator printing!");
                    return out;
            }
        case Location::String:
            return out << "string: \"" << *code.value.string << "\"";
        case Location::Label:
            return out << "label: " << code.value.ui;
        case Location::Call:
            return out << "call: " << code.value.ui;
        case Location::Temporary:
            return out << "temporary: " << code.value.ui;
        default:
            CodeGeneratorError("Invalid bytecode operand location in printing!");
            return out;
    }
}