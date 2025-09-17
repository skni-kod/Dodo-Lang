#include <GenerateCode.hpp>

#include "X86_64.hpp"
#include "X86_64Enums.hpp"

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
                if (size == 64) return out << "zmm0";
                if (size == 32) return out << "ymm0";
                if (size == 16) return out << "xmm0";
                if (size == 8 ) return out << "xmm0";
                if (size == 4 ) return out << "xmm0";
                if (size == 2 ) return out << "xmm0";
                if (size == 1 ) return out << "xmm0";
            break;
            case ZMM1:
                if (size == 64) return out << "zmm1";
                if (size == 32) return out << "ymm1";
                if (size == 16) return out << "xmm1";
                if (size == 8 ) return out << "xmm1";
                if (size == 4 ) return out << "xmm1";
                if (size == 2 ) return out << "xmm1";
                if (size == 1 ) return out << "xmm1";
            break;
            case ZMM2:
                if (size == 64) return out << "zmm2";
                if (size == 32) return out << "ymm2";
                if (size == 16) return out << "xmm2";
                if (size == 8 ) return out << "xmm2";
                if (size == 4 ) return out << "xmm2";
                if (size == 2 ) return out << "xmm2";
                if (size == 1 ) return out << "xmm2";
            break;
            case ZMM3:
                if (size == 64) return out << "zmm3";
                if (size == 32) return out << "ymm3";
                if (size == 16) return out << "xmm3";
                if (size == 8 ) return out << "xmm3";
                if (size == 4 ) return out << "xmm3";
                if (size == 2 ) return out << "xmm3";
                if (size == 1 ) return out << "xmm3";
            break;
            case ZMM4:
                if (size == 64) return out << "zmm4";
                if (size == 32) return out << "ymm4";
                if (size == 16) return out << "xmm4";
                if (size == 8 ) return out << "xmm4";
                if (size == 4 ) return out << "xmm4";
                if (size == 2 ) return out << "xmm4";
                if (size == 1 ) return out << "xmm4";
            break;
            case ZMM5:
                if (size == 64) return out << "zmm5";
                if (size == 32) return out << "ymm5";
                if (size == 16) return out << "xmm5";
                if (size == 8 ) return out << "xmm5";
                if (size == 4 ) return out << "xmm5";
                if (size == 2 ) return out << "xmm5";
                if (size == 1 ) return out << "xmm5";
            break;
            case ZMM6:
                if (size == 64) return out << "zmm6";
                if (size == 32) return out << "ymm6";
                if (size == 16) return out << "xmm6";
                if (size == 8 ) return out << "xmm6";
                if (size == 4 ) return out << "xmm6";
                if (size == 2 ) return out << "xmm6";
                if (size == 1 ) return out << "xmm6";
            break;
            case ZMM7:
                if (size == 64) return out << "zmm7";
                if (size == 32) return out << "ymm7";
                if (size == 16) return out << "xmm7";
                if (size == 8 ) return out << "xmm7";
                if (size == 4 ) return out << "xmm7";
                if (size == 2 ) return out << "xmm7";
                if (size == 1 ) return out << "xmm7";
            break;
            case ZMM8:
                if (size == 64) return out << "zmm8";
                if (size == 32) return out << "ymm8";
                if (size == 16) return out << "xmm8";
                if (size == 8 ) return out << "xmm8";
                if (size == 4 ) return out << "xmm8";
                if (size == 2 ) return out << "xmm8";
                if (size == 1 ) return out << "xmm8";
            break;
            case ZMM9:
                if (size == 64) return out << "zmm9";
                if (size == 32) return out << "ymm9";
                if (size == 16) return out << "xmm9";
                if (size == 8 ) return out << "xmm9";
                if (size == 4 ) return out << "xmm9";
                if (size == 2 ) return out << "xmm9";
                if (size == 1 ) return out << "xmm9";
            break;
            case ZMM10:
                if (size == 64) return out << "zmm10";
                if (size == 32) return out << "ymm10";
                if (size == 16) return out << "xmm10";
                if (size == 8 ) return out << "xmm10";
                if (size == 4 ) return out << "xmm10";
                if (size == 2 ) return out << "xmm10";
                if (size == 1 ) return out << "xmm10";
            break;
            case ZMM11:
                if (size == 64) return out << "zmm11";
                if (size == 32) return out << "ymm11";
                if (size == 16) return out << "xmm11";
                if (size == 8 ) return out << "xmm11";
                if (size == 4 ) return out << "xmm11";
                if (size == 2 ) return out << "xmm11";
                if (size == 1 ) return out << "xmm11";
            break;
            case ZMM12:
                if (size == 64) return out << "zmm12";
                if (size == 32) return out << "ymm12";
                if (size == 16) return out << "xmm12";
                if (size == 8 ) return out << "xmm12";
                if (size == 4 ) return out << "xmm12";
                if (size == 2 ) return out << "xmm12";
                if (size == 1 ) return out << "xmm12";
            break;
            case ZMM13:
                if (size == 64) return out << "zmm13";
                if (size == 32) return out << "ymm13";
                if (size == 16) return out << "xmm13";
                if (size == 8 ) return out << "xmm13";
                if (size == 4 ) return out << "xmm13";
                if (size == 2 ) return out << "xmm13";
                if (size == 1 ) return out << "xmm13";
            break;
            case ZMM14:
                if (size == 64) return out << "zmm14";
                if (size == 32) return out << "ymm14";
                if (size == 16) return out << "xmm14";
                if (size == 8 ) return out << "xmm14";
                if (size == 4 ) return out << "xmm14";
                if (size == 2 ) return out << "xmm14";
                if (size == 1 ) return out << "xmm14";
            break;
            case ZMM15:
                if (size == 64) return out << "zmm15";
                if (size == 32) return out << "ymm15";
                if (size == 16) return out << "xmm15";
                if (size == 8 ) return out << "xmm15";
                if (size == 4 ) return out << "xmm15";
                if (size == 2 ) return out << "xmm15";
                if (size == 1 ) return out << "xmm15";
            break;
            case ZMM16:
                if (size == 64) return out << "zmm16";
                if (size == 32) return out << "ymm16";
                if (size == 16) return out << "xmm16";
                if (size == 8 ) return out << "xmm16";
                if (size == 4 ) return out << "xmm16";
                if (size == 2 ) return out << "xmm16";
                if (size == 1 ) return out << "xmm16";
            break;
            case ZMM17:
                if (size == 64) return out << "zmm17";
                if (size == 32) return out << "ymm17";
                if (size == 16) return out << "xmm17";
                if (size == 8 ) return out << "xmm17";
                if (size == 4 ) return out << "xmm17";
                if (size == 2 ) return out << "xmm17";
                if (size == 1 ) return out << "xmm17";
            break;
            case ZMM18:
                if (size == 64) return out << "zmm18";
                if (size == 32) return out << "ymm18";
                if (size == 16) return out << "xmm18";
                if (size == 8 ) return out << "xmm18";
                if (size == 4 ) return out << "xmm18";
                if (size == 2 ) return out << "xmm18";
                if (size == 1 ) return out << "xmm18";
            break;
            case ZMM19:
                if (size == 64) return out << "zmm19";
                if (size == 32) return out << "ymm19";
                if (size == 16) return out << "xmm19";
                if (size == 8 ) return out << "xmm19";
                if (size == 4 ) return out << "xmm19";
                if (size == 2 ) return out << "xmm19";
                if (size == 1 ) return out << "xmm19";
            break;
            case ZMM20:
                if (size == 64) return out << "zmm20";
                if (size == 32) return out << "ymm20";
                if (size == 16) return out << "xmm20";
                if (size == 8 ) return out << "xmm20";
                if (size == 4 ) return out << "xmm20";
                if (size == 2 ) return out << "xmm20";
                if (size == 1 ) return out << "xmm20";
            break;
            case ZMM21:
                if (size == 64) return out << "zmm21";
                if (size == 32) return out << "ymm21";
                if (size == 16) return out << "xmm21";
                if (size == 8 ) return out << "xmm21";
                if (size == 4 ) return out << "xmm21";
                if (size == 2 ) return out << "xmm21";
                if (size == 1 ) return out << "xmm21";
            break;
            case ZMM22:
                if (size == 64) return out << "zmm22";
                if (size == 32) return out << "ymm22";
                if (size == 16) return out << "xmm22";
                if (size == 8 ) return out << "xmm22";
                if (size == 4 ) return out << "xmm22";
                if (size == 2 ) return out << "xmm22";
                if (size == 1 ) return out << "xmm22";
            break;
            case ZMM23:
                if (size == 64) return out << "zmm23";
                if (size == 32) return out << "ymm23";
                if (size == 16) return out << "xmm23";
                if (size == 8 ) return out << "xmm23";
                if (size == 4 ) return out << "xmm23";
                if (size == 2 ) return out << "xmm23";
                if (size == 1 ) return out << "xmm23";
            break;
            case ZMM24:
                if (size == 64) return out << "zmm24";
                if (size == 32) return out << "ymm24";
                if (size == 16) return out << "xmm24";
                if (size == 8 ) return out << "xmm24";
                if (size == 4 ) return out << "xmm24";
                if (size == 2 ) return out << "xmm24";
                if (size == 1 ) return out << "xmm24";
            break;
            case ZMM25:
                if (size == 64) return out << "zmm25";
                if (size == 32) return out << "ymm25";
                if (size == 16) return out << "xmm25";
                if (size == 8 ) return out << "xmm25";
                if (size == 4 ) return out << "xmm25";
                if (size == 2 ) return out << "xmm25";
                if (size == 1 ) return out << "xmm25";
            break;
            case ZMM26:
                if (size == 64) return out << "zmm26";
                if (size == 32) return out << "ymm26";
                if (size == 16) return out << "xmm26";
                if (size == 8 ) return out << "xmm26";
                if (size == 4 ) return out << "xmm26";
                if (size == 2 ) return out << "xmm26";
                if (size == 1 ) return out << "xmm26";
            break;
            case ZMM27:
                if (size == 64) return out << "zmm27";
                if (size == 32) return out << "ymm27";
                if (size == 16) return out << "xmm27";
                if (size == 8 ) return out << "xmm27";
                if (size == 4 ) return out << "xmm27";
                if (size == 2 ) return out << "xmm27";
                if (size == 1 ) return out << "xmm27";
            break;
            case ZMM28:
                if (size == 64) return out << "zmm28";
                if (size == 32) return out << "ymm28";
                if (size == 16) return out << "xmm28";
                if (size == 8 ) return out << "xmm28";
                if (size == 4 ) return out << "xmm28";
                if (size == 2 ) return out << "xmm28";
                if (size == 1 ) return out << "xmm28";
            break;
            case ZMM29:
                if (size == 64) return out << "zmm29";
                if (size == 32) return out << "ymm29";
                if (size == 16) return out << "xmm29";
                if (size == 8 ) return out << "xmm29";
                if (size == 4 ) return out << "xmm29";
                if (size == 2 ) return out << "xmm29";
                if (size == 1 ) return out << "xmm29";
            break;
            case ZMM30:
                if (size == 64) return out << "zmm30";
                if (size == 32) return out << "ymm30";
                if (size == 16) return out << "xmm30";
                if (size == 8 ) return out << "xmm30";
                if (size == 4 ) return out << "xmm30";
                if (size == 2 ) return out << "xmm30";
                if (size == 1 ) return out << "xmm30";
            break;
            case ZMM31:
                if (size == 64) return out << "zmm31";
                if (size == 32) return out << "ymm31";
                if (size == 16) return out << "xmm31";
                if (size == 8 ) return out << "xmm31";
                if (size == 4 ) return out << "xmm31";
                if (size == 2 ) return out << "xmm31";
                if (size == 1 ) return out << "xmm31";
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
        CodeGeneratorError("Internal: invalid register size in printing!");
        return out;
    }

    void PrintOperand(const AsmOperand& op, std::ostream& out) {
        switch (op.op) {
        case Location::Literal:
            out << "$" << std::to_string(op.value.u64);
            return;
        case Location::String:
            out << "$LS" << std::to_string(op.value.u64);
            return;
        case Location::Label:
            if (op.labelType == AsmOperand::function) {
                out << op.value.function->getFullName();
                return;
            }
            if (op.labelType == AsmOperand::jump) {
                out << "LC" << std::to_string(op.value.ui);
                return;
            }
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Call:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Register :
            if (op.useAddress)
                out << "(";
            out << "%";
            PrintRegisterName(op.value.u64, op.size, out);
            if (op.useAddress)
                out << ")";
            return;
        case Location::Memory:
            CodeGeneratorError("Internal: unimplemented operand print!");
            return;
        case Location::Stack:
            out << std::to_string(op.value.offset) << "(%rbp)";
            return;
        case Location::Offset:
            if (not op.value.regOff.isPrefixLabel) {
                if (op.value.regOff.offset != 0) out << std::to_string(op.value.regOff.offset);
            }
            else CodeGeneratorError("Label register offsets not supported!");
            if (op.value.regOff.addressRegister != NO_REGISTER_IN_OFFSET) {
                out << "(%";
                PrintRegisterName(op.value.regOff.addressRegister, Options::addressSize, out);
            }
            else out << "(";
            if (op.value.regOff.indexRegister != NO_REGISTER_IN_OFFSET) {
                out << ", %";
                PrintRegisterName(op.value.regOff.indexRegister, Options::addressSize, out);
            }
            if (op.value.regOff.indexScale != 0) out << ", " << std::to_string(op.value.regOff.indexScale);
                out << ")";
            return;
        default:
            CodeGeneratorError("Internal: unhandled operand in printing!");
        }
    }

    std::string GASPrefix(const AsmOperand& op) {
        switch (op.size) {
            case 8: return "q";
            case 4: return "l";
            case 2: return "w";
            case 1: return "b";
            default: CodeGeneratorError("Internal: invalid GAS prefix size!");
        }
        return "";
    }

    std::string GetMnemonic(const AsmInstruction& ins) {
        switch (ins.code) {
        case InstructionCode::mov:
            return "mov" + GASPrefix(ins.op1);
        case InstructionCode::movss:
            return "movss";
        case InstructionCode::vmovss:
            return "vmovss";
        case InstructionCode::movsd:
            return "movsd";
        case InstructionCode::vmovsd:
            return "vmovsd";
        case InstructionCode::add:
            return "add" + GASPrefix(ins.op1);
        case InstructionCode::addsd:
            return "addsd";
        case InstructionCode::vaddsd:
            return "";
        case InstructionCode::addss:
            return "addss";
        case InstructionCode::vaddss:
            return "";
        case InstructionCode::and_op:
            return "";
        case InstructionCode::call:
            return "call";
        case InstructionCode::cmp:
            return "cmp" + GASPrefix(ins.op1);
        case InstructionCode::comiss:
            return "comiss";
        case InstructionCode::vcomiss:
            return "";
        case InstructionCode::comisd:
            return "comisd";
        case InstructionCode::vcomisd:
            return "";
        case InstructionCode::cvtsd2si:
            return "cvtsd2si";
        case InstructionCode::cvtsd2ss:
            return "cvtsd2ss";
        case InstructionCode::vcvtsd2ss:
            return "";
        case InstructionCode::cvtsi2sd:
            if (ins.op2.op == Location::sta) return "cvtsi2sd" + GASPrefix(ins.op2);
            return "cvtsi2sd";
        case InstructionCode::vcvtsi2sd:
            return "";
        case InstructionCode::cvtsi2ss:
            if (ins.op2.op == Location::sta) return "cvtsi2ss" + GASPrefix(ins.op2);
            return "cvtsi2ss";
        case InstructionCode::vcvtsi2ss:
            return "";
        case InstructionCode::cvtss2sd:
            return "cvtss2sd";
        case InstructionCode::vcvtss2sd:
            return "";
        case InstructionCode::cvtss2si:
            return "cvtss2si";
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
            return "div" + GASPrefix(ins.op1);
        case InstructionCode::divsd:
            return "";
        case InstructionCode::vdivsd:
            return "";
        case InstructionCode::divss:
            return "";
        case InstructionCode::vdivss:
            return "";
        case InstructionCode::idiv:
            return "idiv" + GASPrefix(ins.op1);
        case InstructionCode::imul:
            return "imul" + GASPrefix(ins.op1);
        case InstructionCode::op_int:
            return "";
        case InstructionCode::int0:
            return "";
        case InstructionCode::int1:
            return "";
        case InstructionCode::int3:
            return "";
        case InstructionCode::ja:
            return "ja";
        case InstructionCode::jae:
            return "jae";
        case InstructionCode::jb:
            return "jb";
        case InstructionCode::jbe:
            return "jbe";
        case InstructionCode::jc:
            return "jc";
        case InstructionCode::je:
            return "je";
        case InstructionCode::jz:
            return "jz";
        case InstructionCode::jg:
            return "jg";
        case InstructionCode::jge:
            return "jge";
        case InstructionCode::jl:
            return "jl";
        case InstructionCode::jle:
            return "jle";
        case InstructionCode::jna:
            return "jna";
        case InstructionCode::jnae:
            return "jnae";
        case InstructionCode::jnb:
            return "jnb";
        case InstructionCode::jnbe:
            return "jnbe";
        case InstructionCode::jnc:
            return "jnc";
        case InstructionCode::jne:
            return "jne";
        case InstructionCode::jng:
            return "jng";
        case InstructionCode::jnge:
            return "jnge";
        case InstructionCode::jnl:
            return "jnl";
        case InstructionCode::jnle:
            return "jnle";
        case InstructionCode::jno:
            return "jno";
        case InstructionCode::jnp:
            return "jnp";
        case InstructionCode::jns:
            return "jns";
        case InstructionCode::jnz:
            return "jnz";
        case InstructionCode::jo:
            return "jo";
        case InstructionCode::jp:
            return "jp";
        case InstructionCode::jpe:
            return "jpe";
        case InstructionCode::jpo:
            return "jpo";
        case InstructionCode::js:
            return "js";
        case InstructionCode::jmp:
            return "jmp";
        case InstructionCode::lea:
            return "lea" + GASPrefix(ins.op1);
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
            return "movs" + GASPrefix(ins.op2)  + GASPrefix(ins.op1);
        case InstructionCode::movsxd:
            return "";
        case InstructionCode::movzx:
            return "movz" + GASPrefix(ins.op2)  + GASPrefix(ins.op1);
        case InstructionCode::mul:
            return "mul" + GASPrefix(ins.op1);
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
            return "pop" + GASPrefix(ins.op1);
        case InstructionCode::sal:
            return "";
        case InstructionCode::sar:
            return "";
        case InstructionCode::sub:
            return "sub" + GASPrefix(ins.op1);
        case InstructionCode::subsd:
            return "subsd";
        case InstructionCode::vsubsd:
            return "";
        case InstructionCode::subss:
            return "subss";
        case InstructionCode::vsubss:
            return "";
        case InstructionCode::syscall:
            return "syscall";
        case InstructionCode::ret:
            return "ret";
        case InstructionCode::push:
            return "push" + GASPrefix(ins.op1);
        case InstructionCode::op_xor:
            return "";
        case InstructionCode::seta:
            return "seta";
        case InstructionCode::setae:
            return "setae";
        case InstructionCode::setb:
            return "setb";
        case InstructionCode::setbe:
            return "setbe";
        case InstructionCode::setc:
            return "setc";
        case InstructionCode::sete:
            return "sete";
        case InstructionCode::setz:
            return "setz";
        case InstructionCode::setg:
            return "setg";
        case InstructionCode::setge:
            return "setge";
        case InstructionCode::setl:
            return "setl";
        case InstructionCode::setle:
            return "setle";
        case InstructionCode::setna:
            return "setna";
        case InstructionCode::setnae:
            return "setnae";
        case InstructionCode::setnb:
            return "setnb";
        case InstructionCode::setnbe:
            return "setnbe";
        case InstructionCode::setnc:
            return "setnc";
        case InstructionCode::setne:
            return "setne";
        case InstructionCode::setng:
            return "setng";
        case InstructionCode::setnge:
            return "setnge";
        case InstructionCode::setnl:
            return "setnl";
        case InstructionCode::setnle:
            return "setnle";
        case InstructionCode::setno:
            return "setno";
        case InstructionCode::setnp:
            return "setnp";
        case InstructionCode::setns:
            return "setns";
        case InstructionCode::setnz:
            return "setnz";
        case InstructionCode::seto:
            return "seto";
        case InstructionCode::setp:
            return "setp";
        case InstructionCode::setpe:
            return "setpe";
        case InstructionCode::setpo:
            return "setpo";
        case InstructionCode::sets:
            return "sets";
        case InstructionCode::leave:
            return "leave";
        case InstructionCode::cbw:
            return "cbw";
        case InstructionCode::cwd:
            return "cwd";
        case InstructionCode::cdq:
            return "cdq";
        case InstructionCode::cqo:
            return "cqo";
        case InstructionCode::label:
            return "";
            default:
                CodeGeneratorError("Internal: invalid GAS instruction!");
        }
        return "";
    }

    void PrintInstruction(AsmInstruction& ins, std::ostream& out) {
        if (ins.code == label) {
            PrintOperand(ins.op1, out);
            out << ":\n";
            return;
        }
        auto mnemonic = GetMnemonic(ins);
         out << std::string(Options::functionIndentation, ' ');
        out << mnemonic;
        if (ins.op1.op == Location::None) {out << "\n"; return;}
        if (mnemonic.size() < Options::instructionSpace) {
            out << std::string(Options::instructionSpace - mnemonic.size(), ' ');
        }
        else {
            out << " ";
        }

        if (ins.op2.op != Location::None) {
            PrintOperand(ins.op2, out);
            out << ", ";
        }

        PrintOperand(ins.op1, out);

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

    void PrintInstruction(std::ostream& out, AsmInstruction ins) {
        if (ins.code == label) {
            PrintOperand(ins.op1, out);
            out << ":\n";
            return;
        }
        auto mnemonic = GetMnemonic(ins);
        out << std::string(Options::functionIndentation, ' ');
        out << mnemonic;
        if (ins.op1.op == Location::None) {out << "\n"; return;}
        if (mnemonic.size() < Options::instructionSpace) {
            out << std::string(Options::instructionSpace - mnemonic.size(), ' ');
        }
        else {
            out << " ";
        }

        if (ins.op2.op != Location::None) {
            PrintOperand(ins.op2, out);
            out << ", ";
        }

        PrintOperand(ins.op1, out);

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

    void PrintInstructions(std::vector <AsmInstruction>& instructions, std::ostream& out, int32_t maxOffset) {
        if (Options::assemblyFlavor == Options::AssemblyFlavor::GAS) {
            // stack stuff
            if (maxOffset % 16 != 0) maxOffset = (maxOffset / 16 - 1) * 16;
            PrintInstruction(out, AsmInstruction(push,  AsmOperand(Location::reg, Type::address, false, 8, RBP)));
            PrintInstruction(out, AsmInstruction(mov,  AsmOperand(Location::reg, Type::address, false, 8, RBP), AsmOperand(Location::reg, Type::address, false, 8, RSP)));
            if (maxOffset != 0) {
                PrintInstruction(out, AsmInstruction(sub,  AsmOperand(Location::reg, Type::address, false, 8, RSP), AsmOperand(Location::imm, Type::address, false, 8, -maxOffset)));
            }

            for (auto& n : instructions) {
                if (n.code == ret) {
                    if (maxOffset != 0) {
                        PrintInstruction(out, AsmInstruction(leave));
                    }
                    else PrintInstruction(out, AsmInstruction(pop,  AsmOperand(Location::reg, Type::address, false, 8, RBP)));
                }
                PrintInstruction(n, out);
            }
        }
        else CodeGeneratorError("Invalid assembly type!");
    }

}