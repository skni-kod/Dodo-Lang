#include "X86_64.hpp"

namespace x86_64 {

    std::ostream& PrintRegisterName(uint16_t number, uint8_t size, std::ostream& out) {
        switch (number) {
            case RAX:
                if (size == 8) return out << "rax";
                if (size == 4) return out << "eax";
                if (size == 2) return out << "ax";
                if (size == 1) return out << "al";
            break;
            case RBX:
                if (size == 8) return out << "rbx";
                if (size == 4) return out << "ebx";
                if (size == 2) return out << "bx";
                if (size == 1) return out << "bl";
            break;
            case RCX:
                if (size == 8) return out << "rcx";
                if (size == 4) return out << "ecx";
                if (size == 2) return out << "cx";
                if (size == 1) return out << "cl";
            break;
            case RDX:
                if (size == 8) return out << "rdx";
                if (size == 4) return out << "edx";
                if (size == 2) return out << "dx";
                if (size == 1) return out << "dl";
            break;
            case RSI:
                if (size == 8) return out << "rsi";
                if (size == 4) return out << "esi";
                if (size == 2) return out << "si";
                if (size == 1) return out << "sil";
            break;
            case RDI:
                if (size == 8) return out << "rdi";
                if (size == 4) return out << "edi";
                if (size == 2) return out << "di";
                if (size == 1) return out << "dil";
            break;
            case RSP:
                if (size == 8) return out << "rsp";
                if (size == 4) return out << "esp";
                if (size == 2) return out << "sp";
                if (size == 1) return out << "spl";
            break;
            case RBP:
                if (size == 8) return out << "rbp";
                if (size == 4) return out << "ebp";
                if (size == 2) return out << "bp";
                if (size == 1) return out << "bpl";
            break;
            case R8:
                if (size == 8) return out << "r8";
                if (size == 4) return out << "r8d";
                if (size == 2) return out << "r8w";
                if (size == 1) return out << "r8b";
            break;
            case R9:
                if (size == 8) return out << "r9";
                if (size == 4) return out << "r9d";
                if (size == 2) return out << "r9w";
                if (size == 1) return out << "r9b";
            break;
            case R10:
                if (size == 8) return out << "r10";
                if (size == 4) return out << "r10d";
                if (size == 2) return out << "r10w";
                if (size == 1) return out << "r10b";
            break;
            case R11:
                if (size == 8) return out << "r11";
                if (size == 4) return out << "r11d";
                if (size == 2) return out << "r11w";
                if (size == 1) return out << "r11b";
            break;
            case R12:
                if (size == 8) return out << "r12";
                if (size == 4) return out << "r12d";
                if (size == 2) return out << "r12w";
                if (size == 1) return out << "r12b";
            break;
            case R13:
                if (size == 8) return out << "r13";
                if (size == 4) return out << "r13d";
                if (size == 2) return out << "r13w";
                if (size == 1) return out << "r13b";
            break;
            case R14:
                if (size == 8) return out << "r14";
                if (size == 4) return out << "r14d";
                if (size == 2) return out << "r14w";
                if (size == 1) return out << "r14b";
            break;
            case R15:
                if (size == 8) return out << "r15";
                if (size == 4) return out << "r15d";
                if (size == 2) return out << "r15w";
                if (size == 1) return out << "r15b";
            break;
            case ZMM0:
                if (size == 64) return out << "ZMM0";
                if (size == 32) return out << "YMM0";
                if (size == 16) return out << "XMM0";
                if (size == 8 ) return out << "XMM0";
                if (size == 4 ) return out << "XMM0";
                if (size == 2 ) return out << "XMM0";
                if (size == 1 ) return out << "XMM0";
            break;
            case ZMM1:
                if (size == 64) return out << "ZMM1";
                if (size == 32) return out << "YMM1";
                if (size == 16) return out << "XMM1";
                if (size == 8 ) return out << "XMM1";
                if (size == 4 ) return out << "XMM1";
                if (size == 2 ) return out << "XMM1";
                if (size == 1 ) return out << "XMM1";
            break;
            case ZMM2:
                if (size == 64) return out << "ZMM2";
                if (size == 32) return out << "YMM2";
                if (size == 16) return out << "XMM2";
                if (size == 8 ) return out << "XMM2";
                if (size == 4 ) return out << "XMM2";
                if (size == 2 ) return out << "XMM2";
                if (size == 1 ) return out << "XMM2";
            break;
            case ZMM3:
                if (size == 64) return out << "ZMM3";
                if (size == 32) return out << "YMM3";
                if (size == 16) return out << "XMM3";
                if (size == 8 ) return out << "XMM3";
                if (size == 4 ) return out << "XMM3";
                if (size == 2 ) return out << "XMM3";
                if (size == 1 ) return out << "XMM3";
            break;
            case ZMM4:
                if (size == 64) return out << "ZMM4";
                if (size == 32) return out << "YMM4";
                if (size == 16) return out << "XMM4";
                if (size == 8 ) return out << "XMM4";
                if (size == 4 ) return out << "XMM4";
                if (size == 2 ) return out << "XMM4";
                if (size == 1 ) return out << "XMM4";
            break;
            case ZMM5:
                if (size == 64) return out << "ZMM5";
                if (size == 32) return out << "YMM5";
                if (size == 16) return out << "XMM5";
                if (size == 8 ) return out << "XMM5";
                if (size == 4 ) return out << "XMM5";
                if (size == 2 ) return out << "XMM5";
                if (size == 1 ) return out << "XMM5";
            break;
            case ZMM6:
                if (size == 64) return out << "ZMM6";
                if (size == 32) return out << "YMM6";
                if (size == 16) return out << "XMM6";
                if (size == 8 ) return out << "XMM6";
                if (size == 4 ) return out << "XMM6";
                if (size == 2 ) return out << "XMM6";
                if (size == 1 ) return out << "XMM6";
            break;
            case ZMM7:
                if (size == 64) return out << "ZMM7";
                if (size == 32) return out << "YMM7";
                if (size == 16) return out << "XMM7";
                if (size == 8 ) return out << "XMM7";
                if (size == 4 ) return out << "XMM7";
                if (size == 2 ) return out << "XMM7";
                if (size == 1 ) return out << "XMM7";
            break;
            case ZMM8:
                if (size == 64) return out << "ZMM8";
                if (size == 32) return out << "YMM8";
                if (size == 16) return out << "XMM8";
                if (size == 8 ) return out << "XMM8";
                if (size == 4 ) return out << "XMM8";
                if (size == 2 ) return out << "XMM8";
                if (size == 1 ) return out << "XMM8";
            break;
            case ZMM9:
                if (size == 64) return out << "ZMM9";
                if (size == 32) return out << "YMM9";
                if (size == 16) return out << "XMM9";
                if (size == 8 ) return out << "XMM9";
                if (size == 4 ) return out << "XMM9";
                if (size == 2 ) return out << "XMM9";
                if (size == 1 ) return out << "XMM9";
            break;
            case ZMM10:
                if (size == 64) return out << "ZMM10";
                if (size == 32) return out << "YMM10";
                if (size == 16) return out << "XMM10";
                if (size == 8 ) return out << "XMM10";
                if (size == 4 ) return out << "XMM10";
                if (size == 2 ) return out << "XMM10";
                if (size == 1 ) return out << "XMM10";
            break;
            case ZMM11:
                if (size == 64) return out << "ZMM11";
                if (size == 32) return out << "YMM11";
                if (size == 16) return out << "XMM11";
                if (size == 8 ) return out << "XMM11";
                if (size == 4 ) return out << "XMM11";
                if (size == 2 ) return out << "XMM11";
                if (size == 1 ) return out << "XMM11";
            break;
            case ZMM12:
                if (size == 64) return out << "ZMM12";
                if (size == 32) return out << "YMM12";
                if (size == 16) return out << "XMM12";
                if (size == 8 ) return out << "XMM12";
                if (size == 4 ) return out << "XMM12";
                if (size == 2 ) return out << "XMM12";
                if (size == 1 ) return out << "XMM12";
            break;
            case ZMM13:
                if (size == 64) return out << "ZMM13";
                if (size == 32) return out << "YMM13";
                if (size == 16) return out << "XMM13";
                if (size == 8 ) return out << "XMM13";
                if (size == 4 ) return out << "XMM13";
                if (size == 2 ) return out << "XMM13";
                if (size == 1 ) return out << "XMM13";
            break;
            case ZMM14:
                if (size == 64) return out << "ZMM14";
                if (size == 32) return out << "YMM14";
                if (size == 16) return out << "XMM14";
                if (size == 8 ) return out << "XMM14";
                if (size == 4 ) return out << "XMM14";
                if (size == 2 ) return out << "XMM14";
                if (size == 1 ) return out << "XMM14";
            break;
            case ZMM15:
                if (size == 64) return out << "ZMM15";
                if (size == 32) return out << "YMM15";
                if (size == 16) return out << "XMM15";
                if (size == 8 ) return out << "XMM15";
                if (size == 4 ) return out << "XMM15";
                if (size == 2 ) return out << "XMM15";
                if (size == 1 ) return out << "XMM15";
            break;
            case ZMM16:
                if (size == 64) return out << "ZMM16";
                if (size == 32) return out << "YMM16";
                if (size == 16) return out << "XMM16";
                if (size == 8 ) return out << "XMM16";
                if (size == 4 ) return out << "XMM16";
                if (size == 2 ) return out << "XMM16";
                if (size == 1 ) return out << "XMM16";
            break;
            case ZMM17:
                if (size == 64) return out << "ZMM17";
                if (size == 32) return out << "YMM17";
                if (size == 16) return out << "XMM17";
                if (size == 8 ) return out << "XMM17";
                if (size == 4 ) return out << "XMM17";
                if (size == 2 ) return out << "XMM17";
                if (size == 1 ) return out << "XMM17";
            break;
            case ZMM18:
                if (size == 64) return out << "ZMM18";
                if (size == 32) return out << "YMM18";
                if (size == 16) return out << "XMM18";
                if (size == 8 ) return out << "XMM18";
                if (size == 4 ) return out << "XMM18";
                if (size == 2 ) return out << "XMM18";
                if (size == 1 ) return out << "XMM18";
            break;
            case ZMM19:
                if (size == 64) return out << "ZMM19";
                if (size == 32) return out << "YMM19";
                if (size == 16) return out << "XMM19";
                if (size == 8 ) return out << "XMM19";
                if (size == 4 ) return out << "XMM19";
                if (size == 2 ) return out << "XMM19";
                if (size == 1 ) return out << "XMM19";
            break;
            case ZMM20:
                if (size == 64) return out << "ZMM20";
                if (size == 32) return out << "YMM20";
                if (size == 16) return out << "XMM20";
                if (size == 8 ) return out << "XMM20";
                if (size == 4 ) return out << "XMM20";
                if (size == 2 ) return out << "XMM20";
                if (size == 1 ) return out << "XMM20";
            break;
            case ZMM21:
                if (size == 64) return out << "ZMM21";
                if (size == 32) return out << "YMM21";
                if (size == 16) return out << "XMM21";
                if (size == 8 ) return out << "XMM21";
                if (size == 4 ) return out << "XMM21";
                if (size == 2 ) return out << "XMM21";
                if (size == 1 ) return out << "XMM21";
            break;
            case ZMM22:
                if (size == 64) return out << "ZMM22";
                if (size == 32) return out << "YMM22";
                if (size == 16) return out << "XMM22";
                if (size == 8 ) return out << "XMM22";
                if (size == 4 ) return out << "XMM22";
                if (size == 2 ) return out << "XMM22";
                if (size == 1 ) return out << "XMM22";
            break;
            case ZMM23:
                if (size == 64) return out << "ZMM23";
                if (size == 32) return out << "YMM23";
                if (size == 16) return out << "XMM23";
                if (size == 8 ) return out << "XMM23";
                if (size == 4 ) return out << "XMM23";
                if (size == 2 ) return out << "XMM23";
                if (size == 1 ) return out << "XMM23";
            break;
            case ZMM24:
                if (size == 64) return out << "ZMM24";
                if (size == 32) return out << "YMM24";
                if (size == 16) return out << "XMM24";
                if (size == 8 ) return out << "XMM24";
                if (size == 4 ) return out << "XMM24";
                if (size == 2 ) return out << "XMM24";
                if (size == 1 ) return out << "XMM24";
            break;
            case ZMM25:
                if (size == 64) return out << "ZMM25";
                if (size == 32) return out << "YMM25";
                if (size == 16) return out << "XMM25";
                if (size == 8 ) return out << "XMM25";
                if (size == 4 ) return out << "XMM25";
                if (size == 2 ) return out << "XMM25";
                if (size == 1 ) return out << "XMM25";
            break;
            case ZMM26:
                if (size == 64) return out << "ZMM26";
                if (size == 32) return out << "YMM26";
                if (size == 16) return out << "XMM26";
                if (size == 8 ) return out << "XMM26";
                if (size == 4 ) return out << "XMM26";
                if (size == 2 ) return out << "XMM26";
                if (size == 1 ) return out << "XMM26";
            break;
            case ZMM27:
                if (size == 64) return out << "ZMM27";
                if (size == 32) return out << "YMM27";
                if (size == 16) return out << "XMM27";
                if (size == 8 ) return out << "XMM27";
                if (size == 4 ) return out << "XMM27";
                if (size == 2 ) return out << "XMM27";
                if (size == 1 ) return out << "XMM27";
            break;
            case ZMM28:
                if (size == 64) return out << "ZMM28";
                if (size == 32) return out << "YMM28";
                if (size == 16) return out << "XMM28";
                if (size == 8 ) return out << "XMM28";
                if (size == 4 ) return out << "XMM28";
                if (size == 2 ) return out << "XMM28";
                if (size == 1 ) return out << "XMM28";
            break;
            case ZMM29:
                if (size == 64) return out << "ZMM29";
                if (size == 32) return out << "YMM29";
                if (size == 16) return out << "XMM29";
                if (size == 8 ) return out << "XMM29";
                if (size == 4 ) return out << "XMM29";
                if (size == 2 ) return out << "XMM29";
                if (size == 1 ) return out << "XMM29";
            break;
            case ZMM30:
                if (size == 64) return out << "ZMM30";
                if (size == 32) return out << "YMM30";
                if (size == 16) return out << "XMM30";
                if (size == 8 ) return out << "XMM30";
                if (size == 4 ) return out << "XMM30";
                if (size == 2 ) return out << "XMM30";
                if (size == 1 ) return out << "XMM30";
            break;
            case ZMM31:
                if (size == 64) return out << "ZMM31";
                if (size == 32) return out << "YMM31";
                if (size == 16) return out << "XMM31";
                if (size == 8 ) return out << "XMM31";
                if (size == 4 ) return out << "XMM31";
                if (size == 2 ) return out << "XMM31";
                if (size == 1 ) return out << "XMM31";
            break;
            case R16:
                if (size == 8) return out << "r16";
                if (size == 4) return out << "r16d";
                if (size == 2) return out << "r16w";
                if (size == 1) return out << "r16b";
            break;
            case R17:
                if (size == 8) return out << "r17";
                if (size == 4) return out << "r17d";
                if (size == 2) return out << "r17w";
                if (size == 1) return out << "r17b";
            break;
            case R18:
                if (size == 8) return out << "r18";
                if (size == 4) return out << "r18d";
                if (size == 2) return out << "r18w";
                if (size == 1) return out << "r18b";
            break;
            case R19:
                if (size == 8) return out << "r19";
                if (size == 4) return out << "r19d";
                if (size == 2) return out << "r19w";
                if (size == 1) return out << "r19b";
            break;
            case R20:
                if (size == 8) return out << "r20";
                if (size == 4) return out << "r20d";
                if (size == 2) return out << "r20w";
                if (size == 1) return out << "r20b";
            break;
            case R21:
                if (size == 8) return out << "r21";
                if (size == 4) return out << "r21d";
                if (size == 2) return out << "r21w";
                if (size == 1) return out << "r21b";
            break;
            case R22:
                if (size == 8) return out << "r22";
                if (size == 4) return out << "r22d";
                if (size == 2) return out << "r22w";
                if (size == 1) return out << "r22b";
            break;
            case R23:
                if (size == 8) return out << "r23";
                if (size == 4) return out << "r23d";
                if (size == 2) return out << "r23w";
                if (size == 1) return out << "r23b";
            break;
            case R24:
                if (size == 8) return out << "r24";
                if (size == 4) return out << "r24d";
                if (size == 2) return out << "r24w";
                if (size == 1) return out << "r24b";
            break;
            case R25:
                if (size == 8) return out << "r25";
                if (size == 4) return out << "r25d";
                if (size == 2) return out << "r25w";
                if (size == 1) return out << "r25b";
            break;
            case R26:
                if (size == 8) return out << "r26";
                if (size == 4) return out << "r26d";
                if (size == 2) return out << "r26w";
                if (size == 1) return out << "r26b";
            break;
            case R27:
                if (size == 8) return out << "r27";
                if (size == 4) return out << "r27d";
                if (size == 2) return out << "r27w";
                if (size == 1) return out << "r27b";
            break;
            case R28:
                if (size == 8) return out << "r28";
                if (size == 4) return out << "r28d";
                if (size == 2) return out << "r28w";
                if (size == 1) return out << "r28b";
            break;
            case R29:
                if (size == 8) return out << "r29";
                if (size == 4) return out << "r29d";
                if (size == 2) return out << "r29w";
                if (size == 1) return out << "r29b";
            break;
            case R30:
                if (size == 8) return out << "r30";
                if (size == 4) return out << "r30d";
                if (size == 2) return out << "r30w";
                if (size == 1) return out << "r30b";
            break;
            case R31:
                if (size == 8) return out << "r31";
                if (size == 4) return out << "r31d";
                if (size == 2) return out << "r31w";
                if (size == 1) return out << "r31b";
            break;
            default:
                CodeGeneratorError("Internal: unhandled register in printing!");
        }
        return out;
    }

    void PrintOperand(const AsmOperand& op, std::ostream& out) {
        switch (op.op) {
        case Location::Literal:
            out << "$" << op.value.u64;
            return;
        case Location::String:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Label:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Call:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Register :
            if (op.useAddress) out << "(";
            out << "%";
            PrintRegisterName(op.value.u64, op.size, out);
            if (op.useAddress) out << ")";
            return;
        case Location::Memory:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Stack:
            out << op.value.offset << "(%rbp)";
            return;
        case Location::Offset :
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        default:
            CodeGeneratorError("Internal: unhandled operand in printing!");
        }
    }

    char GASPrefix(const AsmOperand& op) {
        switch (op.size) {
            case 8: return 'q';
            case 4: return 'd';
            case 2: return 'w';
            case 1: return 'b';
            default: CodeGeneratorError("Internal: invalid GAS prefix size!");
        }
        return 0;
    }

    std::string GetMnemonic(const AsmInstruction& ins) {
        switch (ins.code) {
        case InstructionCode::mov:
            return "mov" + GASPrefix(ins.op1);
        case InstructionCode::movss:
            return "";
        case InstructionCode::vmovss:
            return "";
        case InstructionCode::movsd:
            return "";
        case InstructionCode::vmovsd:
            return "";
        case InstructionCode::add:
            return "";
        case InstructionCode::addsd:
            return "";
        case InstructionCode::vaddsd:
            return "";
        case InstructionCode::addss:
            return "";
        case InstructionCode::vaddss:
            return "";
        case InstructionCode::and_op:
            return "";
        case InstructionCode::call:
            return "";
        case InstructionCode::cmp:
            return "";
        case InstructionCode::comiss:
            return "";
        case InstructionCode::vcomiss:
            return "";
        case InstructionCode::comisd:
            return "";
        case InstructionCode::vcomisd:
            return "";
        case InstructionCode::cvtsd2si:
            return "";
        case InstructionCode::cvtsd2ss:
            return "";
        case InstructionCode::vcvtsd2ss:
            return "";
        case InstructionCode::cvtsi2sd:
            return "";
        case InstructionCode::vcvtsi2sd:
            return "";
        case InstructionCode::cvtsi2ss:
            return "";
        case InstructionCode::vcvtsi2ss:
            return "";
        case InstructionCode::cvtss2sd:
            return "";
        case InstructionCode::vcvtss2sd:
            return "";
        case InstructionCode::cvtss2si:
            return "";
        case InstructionCode::vcvtss2si:
            return "";
        case InstructionCode::cvttsd2si:
            return "";
        case InstructionCode::vcvttsd2si:
            return "";
        case InstructionCode::cvttss2si:
            return "";
        case InstructionCode::vcvttss2si:
            return "";
        case InstructionCode::div:
            return "";
        case InstructionCode::divsd:
            return "";
        case InstructionCode::vdivsd:
            return "";
        case InstructionCode::divss:
            return "";
        case InstructionCode::vdivss:
            return "";
        case InstructionCode::idiv:
            return "";
        case InstructionCode::imul:
            return "";
        case InstructionCode::op_int:
            return "";
        case InstructionCode::int0:
            return "";
        case InstructionCode::int1:
            return "";
        case InstructionCode::int3:
            return "";
        case InstructionCode::ja:
            return "";
        case InstructionCode::jae:
            return "";
        case InstructionCode::jb:
            return "";
        case InstructionCode::jbe:
            return "";
        case InstructionCode::jc:
            return "";
        case InstructionCode::je:
            return "";
        case InstructionCode::jz:
            return "";
        case InstructionCode::jg:
            return "";
        case InstructionCode::jge:
            return "";
        case InstructionCode::jl:
            return "";
        case InstructionCode::jle:
            return "";
        case InstructionCode::jna:
            return "";
        case InstructionCode::jnae:
            return "";
        case InstructionCode::jnb:
            return "";
        case InstructionCode::jnbe:
            return "";
        case InstructionCode::jnc:
            return "";
        case InstructionCode::jne:
            return "";
        case InstructionCode::jng:
            return "";
        case InstructionCode::jnge:
            return "";
        case InstructionCode::jnl:
            return "";
        case InstructionCode::jnle:
            return "";
        case InstructionCode::jno:
            return "";
        case InstructionCode::jnp:
            return "";
        case InstructionCode::jns:
            return "";
        case InstructionCode::jnz:
            return "";
        case InstructionCode::jo:
            return "";
        case InstructionCode::jp:
            return "";
        case InstructionCode::jpe:
            return "";
        case InstructionCode::jpo:
            return "";
        case InstructionCode::js:
            return "";
        case InstructionCode::jmp:
            return "";
        case InstructionCode::lea:
            return "";
        case InstructionCode::maxsd:
            return "";
        case InstructionCode::vmaxsd:
            return "";
        case InstructionCode::maxss:
            return "";
        case InstructionCode::vmaxss:
            return "";
        case InstructionCode::minsd:
            return "";
        case InstructionCode::vminsd:
            return "";
        case InstructionCode::minss:
            return "";
        case InstructionCode::vminss:
            return "";
        case InstructionCode::movsx:
            return "";
        case InstructionCode::movsxd:
            return "";
        case InstructionCode::movzx:
            return "";
        case InstructionCode::mul:
            return "";
        case InstructionCode::mulsd:
            return "";
        case InstructionCode::vmulsd:
            return "";
        case InstructionCode::mulss:
            return "";
        case InstructionCode::vmulss:
            return "";
        case InstructionCode::neg:
            return "";
        case InstructionCode::nop:
            return "";
        case InstructionCode::op_not:
            return "";
        case InstructionCode::op_or:
            return "";
        case InstructionCode::pop:
            return "";
        case InstructionCode::sal:
            return "";
        case InstructionCode::sar:
            return "";
        case InstructionCode::sub:
            return "";
        case InstructionCode::subsd:
            return "";
        case InstructionCode::vsubsd:
            return "";
        case InstructionCode::subss:
            return "";
        case InstructionCode::vsubss:
            return "";
        case InstructionCode::syscall:
            return "";
        case InstructionCode::ret:
            return "ret";
        case InstructionCode::push:
            return "";
        case InstructionCode::op_xor:
            return "";
        case InstructionCode::label:
            return "";
            default:
                CodeGeneratorError("Internal: invalid GAS instruction!");
        }
        return "";
    }

    void PrintInstruction(AsmInstruction& ins, std::ofstream& out) {
        auto menomnic = GetMnemonic(ins);
        out << menomnic;
        if (ins.op1.op == Location::None) {out << "\n"; return;}
        if (menomnic.size() < Options::instructionSpace) {
            out << std::string(Options::instructionSpace - menomnic.size(), ' ');
        }
        else {
            out << " ";
        }
        PrintOperand(ins.op1, out);

        if (ins.op2.op != Location::None) {
            out << ", ";
            PrintOperand(ins.op2, out);
        }

        if (ins.op3.op != Location::None) {
            out << ", ";
            PrintOperand(ins.op3, out);
        }

        if (ins.op4.op != Location::None) {
            out << ", ";
            PrintOperand(ins.op4, out);
        }

        out << "\n";

    }

    void PrintInstructions(std::vector <AsmInstruction>& instructions, std::ofstream& out) {
        if (Options::assemblyFlavor == Options::AssemblyFlavor::GAS) {
            for (auto& n : instructions) {
                PrintInstruction(n, out);
            }
        }
        else CodeGeneratorError("Invalid assembly type!");
    }

}