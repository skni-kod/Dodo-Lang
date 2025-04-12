#include "X86_64.hpp"

namespace x86_64 {
    void MoveConverted(AsmOperand s, AsmOperand t, BytecodeContext& context, Processor& proc) {

    }

    std::vector <AsmInstruction> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc) {

        // assumes every move contains known locations only, since we need to know which one exactly to move
        // so those can be: registers, stack offsets, literals, labels and addresses

        // easier to access
        auto& s = move.source;
        auto& t = move.target;

        std::vector <AsmInstruction> moves;
        
        // if it can be a direct operation it can be skipped
        // hopefully I considered all the cases and didn't make if completely broken
        if (s.size == t.size and s.type == t.type
            and (
                (not s.useAddress and s.op == Location::reg and not t.useAddress and (t.op == Location::reg or t.op == Location::sta or t.op == Location::mem))
                or
                (not s.useAddress and s.op == Location::sta and not t.useAddress and t.op == Location::reg)
                or
                (not s.useAddress and s.op == Location::imm and not t.useAddress and (t.op == Location::reg or t.op == Location::sta or t.op == Location::mem))
                or
                (not s.useAddress and s.op == Location::mem and not t.useAddress and (t.op == Location::reg or t.op == Location::sta))
            )) return { AsmInstruction(mov, t, s)};
        if (s.type == Type::address and t.type != Type::address
            and (
                (not s.useAddress and s.op == Location::reg and t.useAddress and t.op == Location::reg)
                or
                (s.useAddress and s.op == Location::reg and not t.useAddress and t.op == Location::reg)
                or
                (not s.useAddress and s.op == Location::imm and t.useAddress and t.op == Location::reg)
            )) return { AsmInstruction(mov, t, s)};

        // immediate values can be moved to stack registers and arbitrary address
        if (s.op == Location::imm) {
            if (t.op == Location::reg) {
                if (     t.useAddress
                    and ((t.value.reg >= RAX and t.value.reg <= R15)
                    or   (t.value.reg >= R16 and t.value.reg <= R31))) {
                    // there is an address in the target register, so the literal needs to be stored there
                    // thankfully it can be done directly though
                    moves.emplace_back(mov, t, s);
                    // TODO: implement a way to change values here
                }
                else if (t.op == Location::off or t.op == Location::mem) {
                    // moving to a given offset in memory or address given label
                    moves.emplace_back(mov, t, s);
                    // TODO: implement a way to change values here
                }
                else if (t.useAddress) CodeGeneratorError("Internal: invalid x86-64 address register!");
                else if (t.value.reg >= XMM0 and t.value.reg <= XMM1) {
                    // xmm registers cannot get values directly from immediate and cannot be used as address storage
                    // first the value needs to be moved to memory
                    // then from memory into xmm
                    if (t.size == 4) {
                        // single precision
                        auto temp = proc.tempStack(4);
                        moves.emplace_back(mov, temp, s);
                        moves.emplace_back(movss, t, temp);
                        proc.registers[t.value.reg].content = s;
                    }
                    else if (t.size == 8) {
                        // double precision
                        auto temp = proc.tempStack(8);
                        moves.emplace_back(mov, temp, s);
                        moves.emplace_back(movsd, t, temp);
                        proc.registers[t.value.reg].content = s;
                    }
                    else CodeGeneratorError("Half precision floats not supported in x86-64!");

                }
                else if ((t.value.reg >= RAX and t.value.reg <= RDX)
                    or   (t.value.reg >= R8  and t.value.reg <= R15)
                    or   (t.value.reg >= R16 and t.value.reg <= R31)) {
                    // these registers can be directly moved to
                    moves.emplace_back(mov, t, s);
                    proc.registers[t.value.reg].content = s;
                }
                else CodeGeneratorError("Internal: invalid register for conversions!");
            }
            else if (t.op == Location::sta) {
                // a very simple case
                moves.emplace_back(mov, t, s);
                proc.get(t).sta->content = s;
            }
            else CodeGeneratorError("Internal: invalid imm -> somewhere move!");
        }
        // TODO: could stack be treated the same as general memory?
        else if (s.op == Location::sta) {
            // things from stack can be moved directly only to registers, however there can be type conversions involved
            if (t.op == Location::reg and not t.useAddress) {
                // moving to registers should be simple, though the damn conversions might happen

                // TODO: add temporary register getting functions if needed
                if (s.type == Type::address) {
                    if (t.value.reg > RDX and (t.value.reg < R8 or t.value.reg > R15) and t.value.reg < R16) CodeGeneratorError("Internal: address move in invalid register!");
                    // addresses are always of fixed size and cannot be moved to non-address locations
                    if (t.type != Type::address) CodeGeneratorError("Internal: cannot move address to non address without using it!");
                    moves.emplace_back(mov, t, s);
                }
                else if ((s.type == Type::unsignedInteger or s.type == Type::signedInteger) and t.type == Type::unsignedInteger) {
                    if (t.value.reg > RDX and (t.value.reg < R8 or t.value.reg > R15) and t.value.reg < R16) CodeGeneratorError("Internal: integer conversion in invalid register!");
                    // if the target is bigger, move with 0 extension
                    // if the target is smaller move only part of the value
                    // if they are the same do a normal move
                    s.type = Type::unsignedInteger;
                    if (s.size == t.size) moves.emplace_back(mov, t, s);
                    else {
                        if (s.size == 1) moves.emplace_back(movzx, t, s);
                        else if (s.size == 2) {
                            if (t.size > 2) moves.emplace_back(movzx, t, s);
                            else {
                                s.size = 1;
                                moves.emplace_back(mov, t, s);
                            }
                        }
                        else if (s.size == 4) {
                            if (t.size == 8) {
                                // that is a special case where x86 didn't include movzx
                                // we need to move a zero in first and then the value to get the correct one
                                AsmOperand zero;
                                zero.op = Location::Literal;
                                zero.type = s.type;
                                zero.size = t.size;
                                zero.value.u64 = 0;
                                moves.emplace_back(mov, t, zero);
                                moves.emplace_back(mov, t, s);
                            }
                            else {
                                s.size = t.size;
                                moves.emplace_back(mov, t, s);
                            }
                        }
                        else {
                            s.size = t.size;
                            moves.emplace_back(mov, t, s);
                        }
                    }
                }
                else if ((s.type == Type::signedInteger or s.type == Type::unsignedInteger) and t.type == Type::signedInteger) {
                    if (t.value.reg > RDX and (t.value.reg < R8 or t.value.reg > R15) and t.value.reg < R16) CodeGeneratorError("Internal: integer conversion in invalid register!");
                    // in that case we treat things as if we started with a signed and convert it accordingly
                    s.type = Type::signedInteger;
                    if (s.size == t.size) moves.emplace_back(mov, t, s);
                    else if (s.size == 1) moves.emplace_back(movsx, t, s);
                    else if (s.size == 2) {
                        if (t.size > 2) moves.emplace_back(movsx, t, s);
                        else {
                            s.size = t.size;
                            moves.emplace_back(mov, t, s);
                        }
                    }
                    else if (s.size == 4) {
                        if (t.size > 4) moves.emplace_back(movsxd, t, s);
                        else {
                            s.size = t.size;
                            moves.emplace_back(mov, t, s);
                        }
                    }
                    else {
                        s.size = t.size;
                        moves.emplace_back(mov, t, s);
                    }
                }
                else if (s.type != Type::floatingPoint and t.type == Type::floatingPoint) {
                    if (t.value.reg < XMM0 or t.value.reg > XMM31) CodeGeneratorError("Internal: float conversion in non XMM register!");
                    // converting from non-floats to floats is a much more complex thing
                    // first off the source needs to be 32 or 64 bits
                    // TODO: add moving into a temp register here
                    if (s.size < 4) CodeGeneratorError("Internal: operand to small to convert to float!");
                    if (t.size == 4) {
                        // converting to single precision float
                        moves.emplace_back(cvtsi2ss, t, s);
                    }
                    else moves.emplace_back(cvtsi2sd, t, s);
                }
                else if (s.type == Type::floatingPoint and t.type != Type::floatingPoint) {
                    if (t.value.reg > RDX and (t.value.reg < R8 or t.value.reg > R15) and t.value.reg < R16) CodeGeneratorError("Internal: float to conversion in invalid target register!");
                    if (s.op != Location::sta and (s.op != Location::reg or s.value.reg < XMM0 or s.value.reg > XMM31)) CodeGeneratorError("Internal: float to integer conversion in invalid source location!");
                    if (t.size < 4) CodeGeneratorError("Internal: too small target for float to integer conversion!");
                    // convert from float to signed integer, yes even for unsigned targets
                    if (s.size == 4) {
                        moves.emplace_back(cvtss2si, t, s);
                    }
                    else moves.emplace_back(cvtsd2si, t, s);
                }
                else if (s.type == t.type and s.type == Type::floatingPoint) {
                    if (t.value.reg < XMM0 or t.value.reg > XMM31) CodeGeneratorError("Internal: float conversion in non XMM register!");
                    if (s.op != Location::sta and (s.op != Location::reg or s.value.reg < XMM0 or s.value.reg > XMM31)) CodeGeneratorError("Internal: float conversion in invalid source location!");
                    if (s.size == t.size) {
                        if (s.size == 4) moves.emplace_back(movss, t, s);
                        else moves.emplace_back(movsd, t, s);
                    }
                    // target is a double
                    else if (s.size == 4) moves.emplace_back(cvtss2sd, t, s);
                    // source is a double
                    else moves.emplace_back(cvtsd2ss, t, s);
                }
                else CodeGeneratorError("Internal: unhanded type conversion in x86-64!");
                proc.get(t).reg->content = s;
            }
            else if (t.op == Location::sta) {
                CodeGeneratorError("Internal: memory to memory moves not implemented!");
            }
            else CodeGeneratorError("Internal: invalid sta -> somewhere move!");
        }
        else CodeGeneratorError("Internal: unimplemented conversion!");

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

            // TODO: add "eightbytes" for structures
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