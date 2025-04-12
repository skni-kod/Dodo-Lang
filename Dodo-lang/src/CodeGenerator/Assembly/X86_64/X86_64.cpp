#include "X86_64.hpp"

#include <GenerateCode.hpp>

namespace x86_64 {
    bool IsIntegerOperationRegister(uint8_t number) {
        if (number <= RDI) return true;
        if (number >= R8 and number <= R15) return true;
        if (number >= R16) return true;
        return false;
    }

    bool IsIntegerOperationRegister(const AsmOperand op) {
        if (op.op != Location::reg) return false;
        if (op.value.reg <= RDI) return true;
        if (op.value.reg >= R8 and op.value.reg <= R15) return true;
        if (op.value.reg >= R16) return true;
        return false;
    }

    bool IsFloatOperationRegister(uint8_t number) {
        return number >= ZMM0 and number <= ZMM31;
    }

    bool IsFloatOperationRegister(const AsmOperand op) {
        return op.op == Location::reg and (op.value.reg >= ZMM0 and op.value.reg <= ZMM31);
    }

    std::vector <AsmInstruction> AddConversionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc) {

        // assumes every move contains known locations only, since we need to know which one exactly to move
        // so those can be: registers, stack offsets, literals, labels and addresses

        // easier to access
        auto& s = move.source;
        auto& t = move.target;

        std::vector <AsmInstruction> moves;

        if (s == t) return {};

        // Here it is rewritten to be based on types and not where stiff is moved from

        if (s.useAddress and not t.useAddress) {
            CodeGeneratorError("Internal: address to non address not implemented!");
        }
        else if (t.useAddress and not s.useAddress) {
            CodeGeneratorError("Internal: non address to address not implemented!");
        }
        else if (not t.useAddress and not s.useAddress) {
            if (s.type == Type::address) {
                // addresses can be only moved if not used, so that's simple
                // TODO: make this possible for things like printing addresses
                if (t.type != Type::address) CodeGeneratorError("Internal: address to non address move!");
                if (s.size != 8 or t.size != 8) CodeGeneratorError("Internal: wrong address operand size!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) CodeGeneratorError("Internal: invalid register for address storage!");

                // now actually moving it
                if (s.op == Location::imm) {
                    if (t.op == Location::reg or t.op == Location::sta) moves.emplace_back(mov, t, s);
                    else CodeGeneratorError("Internal: Invalid address immediate target!");
                }
                else if (s.op == Location::sta) {
                    if (t.op == Location::reg) moves.emplace_back(mov, t, s);
                    else if (t.op == Location::sta) CodeGeneratorError("Internal: Invalid address stack to stack!");
                    else CodeGeneratorError("Internal: Invalid address stack target!");
                }
                else if (s.op == Location::reg) {
                    if (t.op == Location::reg or t.op == Location::sta) moves.emplace_back(mov, t, s);
                    else CodeGeneratorError("Internal: Invalid address register target!");
                }
                else if (s.op == Location::mem) {
                    if (t.op == Location::reg or t.op == Location::sta) moves.emplace_back(mov, t, s);
                    else CodeGeneratorError("Internal: Invalid address memory target!");
                }
                else CodeGeneratorError("Internal: invalid address source!");
            }
            else if (s.type != Type::floatingPoint and t.type == Type::unsignedInteger) {
                // in case of unsigned target we treat the integer source as an unsigned too
                s.type = Type::unsignedInteger;
                if (s.op == t.op and s.op == Location::sta) CodeGeneratorError("Internal: stack to stack move unimplemented!");
                if (s.op == Location::reg and not IsIntegerOperationRegister(s.value.reg)) CodeGeneratorError("Internal: non integer register source!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) CodeGeneratorError("Internal: non integer register target!");
                if (s.op == Location::imm) moves.emplace_back(mov, t, s);
                else if ((s.op == Location::reg and t.op == Location::sta)
                    or (s.op == Location::sta and t.op == Location::reg)
                    or (s.op == Location::reg and t.op == Location::reg)) {
                    if (s.size == t.size) moves.emplace_back(mov, t, s);
                    else if (s.size > t.size) {
                        s.size = t.size;
                        moves.emplace_back(mov, t, s);
                    }
                    else {
                        if (s.size != 4 and t.size != 8)  moves.emplace_back(movzx, t, s);
                        else {
                            // 4 -> 8 byte is a special case where movzx does not apply and we need to split it
                            moves.emplace_back(movzx, t, AsmOperand(Location::imm, Type::unsignedInteger, false, 8, 0));
                            moves.emplace_back(movzx, t, s);
                        }
                    }
                }
                else CodeGeneratorError("Internal: unimplemented unsigned conversion!");
            }
            else if (s.type != Type::floatingPoint and t.type == Type::signedInteger) {
                // in case of signed target we treat the integer source as a signed too
                s.type = Type::signedInteger;
                if (s.op == t.op and s.op == Location::sta) CodeGeneratorError("Internal: stack to stack move unimplemented!");
                if (s.op == Location::reg and not IsIntegerOperationRegister(s.value.reg)) CodeGeneratorError("Internal: non integer register source!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) CodeGeneratorError("Internal: non integer register target!");
                if (s.op == Location::imm) moves.emplace_back(mov, t, s);
                else if ((s.op == Location::reg and t.op == Location::sta)
                    or (s.op == Location::sta and t.op == Location::reg)
                    or (s.op == Location::reg and t.op == Location::reg)) {
                    if (s.size == t.size) moves.emplace_back(mov, t, s);
                    else if (s.size > t.size) {
                        s.size = t.size;
                        moves.emplace_back(mov, t, s);
                    }
                    else moves.emplace_back(movsx, t, s);
                }
                else CodeGeneratorError("Internal: unimplemented signed conversion!");
            }
            else if (s.type != Type::floatingPoint and t.type == Type::floatingPoint) {
                // converting to floating point can only start from a 32-bit or 64-bit integer register or memory location
                if (    (s.op != Location::reg or not IsIntegerOperationRegister(s.value.reg) or s.size < 4)
                    and ((s.op != Location::sta or s.op != Location::mem) or s.size < 4)) CodeGeneratorError("Internal: invalid source for conversion into float!");
                if (not IsFloatOperationRegister(t)) CodeGeneratorError("Internal: non SSE/AVX register as float from int target!");

                if (t.size == 4) moves.emplace_back(cvtsi2ss, t, s);
                else moves.emplace_back(cvtsi2sd, t, s);
            }
            else if (s.type == Type::floatingPoint and t.type != Type::floatingPoint) {
                if (not IsFloatOperationRegister(s) and (s.op != Location::sta or s.size < 4)) CodeGeneratorError("Internal: invalid source for float into int!");
                if (not IsIntegerOperationRegister(t) or t.size < 4) CodeGeneratorError("Internal: invalid target for float into int!");

                if (s.size == 4) moves.emplace_back(cvtss2si, t, s);
                else moves.emplace_back(cvtsd2si, t, s);
            }
            else if (s.type == Type::floatingPoint and t.type == Type::floatingPoint) {
                if (s.size == t.size) {
                    if (IsFloatOperationRegister(s)) {
                        if (IsFloatOperationRegister(t) or (t.op == Location::sta or t.op == Location::mem)) {
                            if (s.size == 4) moves.emplace_back(movss, t, s);
                            else moves.emplace_back(movsd, t, s);
                        }
                        else CodeGeneratorError("Internal: invalid target in float move!");
                    }
                    else if (s.op == Location::sta or s.op == Location::mem) {
                        if (IsFloatOperationRegister(t)) {
                            if (s.size == 4) moves.emplace_back(movss, t, s);
                            else moves.emplace_back(movsd, t, s);
                        }
                        else CodeGeneratorError("Internal: invalid target in float move!");
                    }
                }
                else {
                    if (not IsFloatOperationRegister(t) and ((s.op != Location::sta and s.op != Location::mem) or s.size < 4)) CodeGeneratorError("Internal: invalid source for float size conversion!");
                    if (not IsFloatOperationRegister(t)) CodeGeneratorError("Internal: invalid target for float size conversion!");
                    if (s.size == 4) moves.emplace_back(cvtss2sd, t, s);
                    else moves.emplace_back(cvtsd2ss, t, s);
                }
            }
        }
        else CodeGeneratorError("Internal: address to address move!");
        return moves;
    }

    std::pair<std::vector <AsmOperand>, int32_t> GetFunctionMethodArgumentLocations(ParserFunctionMethod& target) {
        std::vector <AsmOperand> result;
        int32_t stackOffset = 0;


        // first off preparing the correct table of places
        if (Options::targetSystem == "LINUX") {
            std::array <uint8_t, 6> integersPointers = {RDI, RSI, RDX, RDX, R8, R9};
            std::array <uint8_t, 8> floats = {XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7};
            uint16_t intPointerCounter = 0, floatCounter = 0;

            // static chain pointer is probably not going to be used since it's for parent function in lambdas and such

            if (target.isMethod and not target.isOperator) {
                // add the "this" argument if it applies
                intPointerCounter++;
                result.emplace_back(Location::reg, Type::address, false, Options::addressSize, RDI);
            }

            // TODO: add "eightbytes" for structures, mostly for syscalls/c calls
            for (auto& n : target.parameters) {
                if (n.typeObject == nullptr) {
                    n.typeObject = &types[n.typeName()];
                }
                if (n.typeObject->isPrimitive and not n.typeMeta().isReference and not n.typeMeta().pointerLevel) {
                    // a primitive
                    auto primitiveType = n.typeObject->primitiveType;
                    auto typeSize = n.typeObject->typeSize;
                    if (floatCounter != 8 and primitiveType == Type::floatingPoint) {
                        // floats into SSE/AVX registers
                        result.emplace_back(Location::reg, Type::floatingPoint, false, typeSize, floats[floatCounter++]);
                    }
                    else if (intPointerCounter != 6 and primitiveType != Type::floatingPoint){
                        // integers into normal registers
                        result.emplace_back(Location::reg, primitiveType, false, typeSize, integersPointers[intPointerCounter++]);
                    }
                    else {
                        // move the value onto the stack

                        // ensure it's aligned properly and added
                        if (stackOffset % n.typeObject->typeAlignment) stackOffset = (stackOffset / n.typeObject->typeAlignment + 2) * n.typeObject->typeAlignment;
                        else stackOffset += n.typeObject->typeSize;

                        OperandValue offset;
                        offset.offset = stackOffset;
                        result.emplace_back(Location::sta, primitiveType, false, typeSize, offset);
                    }
                }
                else {
                    // it's an address
                    if (stackOffset % 8) stackOffset = (stackOffset / 8 + 2) * 8;
                    else stackOffset += 8;

                    OperandValue offset;
                    offset.offset = stackOffset;
                    auto typeSize = n.typeObject->typeSize;
                    result.emplace_back(Location::sta, Type::address, false, typeSize, offset);
                }
            }
            if (stackOffset % 16) {
                stackOffset = (stackOffset / 16 + 1) * 16;
            }
            for (auto& n : result) {
                if (n.op == Location::sta) {
                    n.value.offset = 16 + stackOffset - n.value.offset;
                }
            }
        }
        else if (Options::targetSystem == "WINDOWS") {
            CodeGeneratorError("Microsoft x86-64 calling convention is not supported, linux's is though!");
        }
        else CodeGeneratorError("x86-64 calling convention for: " + Options::targetSystem + "if undefined!");
        return {result, stackOffset};
    }
}
