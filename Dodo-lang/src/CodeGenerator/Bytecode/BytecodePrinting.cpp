#include "BytecodeInternal.hpp"

std::ostream& operator<<(std::ostream& out, const Bytecode& code) {
  out << "Instruction: ";
  switch (code.type) {
        case Bytecode::None:
            out << "";
            break;
      case Bytecode::Define:
            out << "define " << code.op1() << " of type " << code.opType->typeName << std::string(code.opTypeMeta.pointerLevel, '*');
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
        case Bytecode::Member:
            out << "load " << code.op1() << "'s member number " << code.op2() << " address into " << code.result();
            break;
        case Bytecode::Save:
            out << "save the value in  " << code.op1() << " to " << code.result();
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
            out << "argument for call number: " << code.op2Value.ui << " using value from " << code.op3();
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
            out << "add " << code.op1() << " to " << code.op2() << " stored to " << code.result();
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

std::ostream& operator<<(std::ostream& out, const BytecodeOperand& op) {
    switch (op.location) {
        case Location::None:
            return out << "none";
        case Location::Variable:
            return out << "variable: " << op.value.variable;
        case Location::Literal:
            switch (op.literalType) {
                case Type::address:
                    switch (Options::addressSize) {
                        case 1:
                            return out << "fixed address: " << op.value.u8;
                        case 2:
                            return out << "fixed address: " << op.value.u16;
                        case 4:
                            return out << "fixed address: " << op.value.u32;
                        case 8:
                            return out << "fixed address: " << op.value.u64;
                        default:
                            break;
                    }
                case Type::floatingPoint:
                    switch (op.literalSize) {
                        case 2:
                            CodeGeneratorError("16 bit floats not supported in printing!");
                        case 4:
                            return out << "floating point literal: " << op.value.f32;
                        case 8:
                            return out << "floating point literal: " << op.value.f64;
                        default:
                            break;
                    }
                    break;
                case Type::signedInteger:
                    switch (op.literalSize) {
                        case 1:
                            return out << "signed integer literal: " << op.value.i8;
                        case 2:
                            return out << "signed integer literal: " << op.value.i16;
                        case 4:
                            return out << "signed integer literal: " << op.value.i32;
                        case 8:
                            return out << "signed integer literal: " << op.value.i64;
                        default:
                            break;
                    }
                break;
                case Type::unsignedInteger:
                    switch (op.literalSize) {
                        case 1:
                            return out << "unsigned integer literal: " << op.value.u8;
                        case 2:
                            return out << "unsigned integer literal: " << op.value.u16;
                        case 4:
                            return out << "unsigned integer literal: " << op.value.u32;
                        case 8:
                            return out << "unsigned integer literal: " << op.value.u64;
                        default:
                            break;
                    }
                default:
                    out << "\n";
                    CodeGeneratorError("Unhandled type in literal in bytecode operator printing!");
                CodeGeneratorError("Invalid literal size in printing!");
                return out;
            }
        case Location::String:
            return out << "string: \"" << *Strings[op.value.string].ptr << "\"";
        case Location::Label:
            return out << "label: " << op.value.ui;
        case Location::Call:
            out << "call: ";
            if (op.value.function->isMethod) out << op.value.function->parentType->typeName << "::";
            if (op.value.function->isOperator) return PrintOperatorSymbol(op.value.function->overloaded, out << "operator ");
            return out << op.value.function->name;
        case Location::Temporary:
            return out << "temporary: " << op.value.ui;
        default:
            CodeGeneratorError("Invalid bytecode operand location in printing!");
            return out;
    }
}