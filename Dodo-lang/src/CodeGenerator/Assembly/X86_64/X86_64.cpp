#include "X86_64.hpp"
#include "X86_64Enums.hpp"

#include <GenerateCode.hpp>

#include "ErrorHandling.hpp"

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

    void AddMoveWithRegisterNeededBetweenCheck(Context& context, std::vector <AsmInstruction>& moves, AsmOperand s, AsmOperand t, InstructionCode firstMove = mov, InstructionCode actualMove = mov) {
        if (s.anyOf(Location::Sta, Location::Mem, Location::Off) and t.anyOf(Location::Sta, Location::Mem, Location::Off)) {
            auto reg = context.getFreeRegister(s.type, s.size);
            moves.emplace_back(firstMove, reg, s);
            moves.emplace_back(actualMove, t, reg);
        }
        else if (s.anyOf(Location::Sta, Location::Mem, Location::Off, Location::Literal, Location::reg) and t.is(Location::reg)
            or s.anyOf(Location::reg, Location::Literal) and t.anyOf(Location::Sta, Location::Mem, Location::Off)){
            moves.emplace_back(actualMove, t, s);
        }
        else Error("Unimplemented type of move!");
    }

    void AddConversionsToMoveInternal(MoveInfo& move, Context& context, std::vector<AsmInstruction>& moves, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters, bool setContent) {

        // assumes every move contains known locations only, since we need to know which one exactly to move
        // so those can be: registers, stack offsets, literals, labels and addresses

        // easier to access
        auto& s = move.source;
        auto& t = move.target;

        if (s == t) {
            if (contentToSet.op == Location::reg or contentToSet.op == Location::sta or contentToSet.op == Location::mem) context.getContentRef(t) = context.getContent(contentToSet);
            else if (s.op == Location::imm and t.op == Location::imm) return;
            else if (t.op == Location::off) return;
            else if (setContent) context.getContentRef(t) = contentToSet;

            return;
        }

        // Here it is rewritten to be based on types and not where stiff is moved from

        if (s.useAddress and not t.useAddress) {
            Error("Internal: address to non address not implemented!");
        }
        else if (t.useAddress and not s.useAddress) {
            Error("Internal: non address to address not implemented!");
        }
        else if (not t.useAddress and not s.useAddress) {
            if (s.type == Type::address) {
                // addresses can be only moved if not used, so that's simple
                // TODO: make this possible for things like printing addresses
                if (t.type != Type::address) Error("Internal: address to non address move!");
                //if ((s.size != 8 and not (s.op == Location::imm and ((s.size = 8)))) or t.size != 8) Error("Internal: wrong address operand size!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) Error("Internal: invalid register for address storage!");

                // now actually moving it
                if (s.op == Location::imm) {
                    if (t.op == Location::reg or t.op == Location::sta) moves.emplace_back(mov, t, s);
                    else Error("Internal: Invalid address immediate target!");
                }
                else if (s.op == Location::sta or s.op == Location::off) {
                    if (t.op == Location::reg) moves.emplace_back(mov, t, s);
                    else if (t.op == Location::sta or t.op == Location::off) {
                        auto reg = context.getFreeRegister(s.type, s.size);
                        moves.emplace_back(mov, reg, s);
                        moves.emplace_back(mov, t, reg);
                    }
                    else Error("Internal: Invalid address stack target!");
                }
                else if (s.op == Location::reg) {
                    if (t.op == Location::reg or t.op == Location::sta or t.op == Location::off) moves.emplace_back(mov, t, s);
                    else Error("Internal: Invalid address register target!");
                }
                else if (s.op == Location::mem) {
                    if (t.op == Location::reg or t.op == Location::sta or t.op == Location::off) moves.emplace_back(mov, t, s);
                    else Error("Internal: Invalid address memory target!");
                }
                else if (s.op == Location::String) {
                    if (t.op == Location::reg or t.op == Location::sta) moves.emplace_back(mov, t, s);
                    else Error("Internal: Invalid address string target!");
                }
                else Error("Internal: invalid address source!");
            }
            else if (t.type == Type::address) {
                if (s.anyOf(Location::imm, Location::reg))
                    AddMoveWithRegisterNeededBetweenCheck(context, moves, s, s.copyTo(t.op, t.value));
                else if (s.op == Location::Sta or s.op == Location::off) {
                    AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t);
                }
                else Error("Internal: unimplemented value to address move!");
            }
            else if (s.type != Type::floatingPoint and t.type == Type::unsignedInteger) {
                // in case of unsigned target we treat the integer source as an unsigned too
                s.type = Type::unsignedInteger;
                if (s.op == Location::reg and not IsIntegerOperationRegister(s.value.reg)) Error("Internal: non integer register source!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) Error("Internal: non integer register target!");
                if (s.size == t.size) AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t);
                else if (s.size > t.size) {
                    s.size = t.size;
                    AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t);
                }
                else {
                    if (s.size != 4 or t.size != 8)  AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t, mov, movzx);
                    else {
                        // 4 -> 8 byte is a special case where movzx does not apply and we need to split it
                        AddMoveWithRegisterNeededBetweenCheck(context, moves, AsmOperand(Location::imm, Type::unsignedInteger, false, 8, 0), t);
                        AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t, mov, movzx);
                    }
                }
            }
            else if (s.type != Type::floatingPoint and t.type == Type::signedInteger) {
                // in case of signed target we treat the integer source as a signed too
                s.type = Type::signedInteger;
                if (s.op == Location::reg and not IsIntegerOperationRegister(s.value.reg)) Error("Internal: non integer register source!");
                if (t.op == Location::reg and not IsIntegerOperationRegister(t.value.reg)) Error("Internal: non integer register target!");
                if (s.size >= t.size) {
                    s.size = t.size;
                    AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t);
                }
                else
                    AddMoveWithRegisterNeededBetweenCheck(context, moves, s, t, mov, movsx);
            }
            else if (s.type != Type::floatingPoint and t.type == Type::floatingPoint) {
                // converting to floating point can only start from a 32-bit or 64-bit integer register or memory location
                if (    (s.op != Location::reg or not IsIntegerOperationRegister(s.value.reg) or s.size < 4)
                    and ((s.op != Location::sta or s.op != Location::mem) or s.size < 4)) Error("Internal: invalid source for conversion into float!");
                if (not IsFloatOperationRegister(t)) Error("Internal: non SSE/AVX register as float from int target!");

                if (t.size == 4) moves.emplace_back(cvtsi2ss, t, s);
                else moves.emplace_back(cvtsi2sd, t, s);
            }
            else if (s.type == Type::floatingPoint and t.type != Type::floatingPoint) {
                if (not IsFloatOperationRegister(s) and (s.op != Location::sta or s.size < 4)) Error("Internal: invalid source for float into int!");
                if (not IsIntegerOperationRegister(t) or t.size < 4) Error("Internal: invalid target for float into int!");

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
                        else Error("Internal: invalid target in float move!");
                    }
                    else if (s.op == Location::sta or s.op == Location::mem) {
                        if (IsFloatOperationRegister(t)) {
                            if (s.size == 4) moves.emplace_back(movss, t, s);
                            else moves.emplace_back(movsd, t, s);
                        }
                        else Error("Internal: invalid target in float move!");
                    }
                    else if (s.op == Location::imm) {
                        if (t.op == Location::sta) {
                            moves.emplace_back(mov, t, s);
                        }
                        else Error("Internal: invalid target in float move!");
                    }
                }
                else {
                    if (not IsFloatOperationRegister(t) and ((s.op != Location::sta and s.op != Location::mem) or s.size < 4)) Error("Internal: invalid source for float size conversion!");
                    if (not IsFloatOperationRegister(t)) Error("Internal: invalid target for float size conversion!");
                    if (s.size == 4) moves.emplace_back(cvtss2sd, t, s);
                    else moves.emplace_back(cvtsd2ss, t, s);
                }
            }
        }
        else Error("Internal: address to address move!");

        if (setContent)
            context.getContentRef(t) = contentToSet;
    }

    void AddConversionsToMove(MoveInfo& move, Context& context, std::vector<AsmInstruction>& moves, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters, bool setContent, bool intendedComplexSingleMove) {
        if (move.source.op != Location::sta)
            return AddConversionsToMoveInternal(move, context, moves, contentToSet, forbiddenRegisters, setContent);

        if (not intendedComplexSingleMove) {
            auto var = context.getContent(move.source).object(context);
            if (not var.meta.isReference and not var.meta.pointerLevel and var.type->members.size() > 1) {
                // if it's a complex type value move it's going to be more annoying
                if ((move.source.op != Location::Sta and move.source.op != Location::Off) or (move.target.op != Location::Sta and move.target.op != Location::Off))
                    Unimplemented();

                auto size = var.type->typeSize;
                auto alignment = var.type->typeAlignment;
                auto sourceOffset = move.source.op == Location::Sta ? move.source.value.offset : move.source.value.regOff.offset;
                auto targetOffset = move.target.op == Location::Sta ? move.target.value.offset : move.target.value.regOff.offset;
                for (auto current = 0; current < size; current += alignment) {
                    OperandValue regOff;
                    regOff.regOff.addressRegister = move.source.op == Location::Sta ? RBP : move.source.value.regOff.addressRegister;
                    regOff.regOff.offset = int32_t(sourceOffset) + current;
                    move.source = AsmOperand(Location::off, move.target.type, false, uint8_t(alignment), regOff);
                    regOff.regOff.addressRegister = move.target.op == Location::Sta ? RBP : move.target.value.regOff.addressRegister;
                    regOff.regOff.offset = int32_t(targetOffset) + current;
                    move.target = AsmOperand(Location::off, move.target.type, false, uint8_t(alignment), regOff);
                    AddConversionsToMoveInternal(move, context, moves, contentToSet, forbiddenRegisters, false);
                }
                if (setContent) {
                    if (move.target.value.regOff.addressRegister == RBP)
                        context.getContentRefAtOffset(targetOffset) = contentToSet;
                    else
                        Warning("Did not set content to non-stack location, as it is unknown!");
                }
                return;
            }
        }
        AddConversionsToMoveInternal(move, context, moves, contentToSet, forbiddenRegisters, setContent);
    }

    ArgumentLocation::ArgumentLocation(bool passStructAddress, uint16_t argNumber, uint32_t size, bool isSplit,
       int32_t memberOffset):
       passStructAddress(passStructAddress), argumentNumber(argNumber), size(size), isSplit(isSplit), memberOffset(memberOffset) {}


    struct ArgumentContext {
        std::vector <uint8_t> integerRegisters;
        std::vector <uint8_t> floatRegisters;
        std::stack <ArgumentLocation> stack{};
        std::vector <ArgumentLocation>& result;
        int32_t& offset;

        ArgumentContext(std::vector <uint8_t> integers, std::vector <uint8_t> floats, std::vector <ArgumentLocation>& result, int32_t& offset) :
            integerRegisters(std::move(integers)), floatRegisters(std::move(floats)), result(result), offset(offset) {}

        void addStack(ArgumentLocation toAdd) {
            toAdd.isStack = true;
            stack.emplace(toAdd);
        }

        void addInteger(ArgumentLocation&& toAdd) {
            if (integerRegisters.empty())
                return addStack(toAdd);

            toAdd.regNumber = integerRegisters.back();
            integerRegisters.pop_back();
            result.emplace_back(toAdd);
        }

        void addFloat(ArgumentLocation&& toAdd) {
            if (floatRegisters.empty())
                return addStack(toAdd);

            toAdd.regNumber = floatRegisters.back();
            floatRegisters.pop_back();
            result.emplace_back(toAdd);
        }

        void addStackToResult() {
            while (not stack.empty()) {
                offset += stack.top().size;
                stack.top().offset = -offset;
                result.emplace_back(std::move(stack.top()));
                stack.pop();

                if (offset % 8 == 0)
                    continue;;

                offset = (offset / 8 + 1) * 8;
            }
            offset *= -1;
        }
    };

    void GetArgumentLocationsLinux(std::vector<TypeInfo>& parameters, std::vector <ArgumentLocation>& result, int32_t& offset, bool addRaxForSyscall = false) {
        ArgumentContext context =
            addRaxForSyscall ?
            ArgumentContext{{R9, R8, RCX, RDX, RSI, RDI, RAX}, {XMM7, XMM6, XMM5, XMM4, XMM3, XMM2, XMM1, XMM0}, result, offset}
          : ArgumentContext{{R9, R8, RCX, RDX, RSI, RDI}, {XMM7, XMM6, XMM5, XMM4, XMM3, XMM2, XMM1, XMM0}, result, offset};

        uint16_t argNumber = 0;
        for (const auto& n : parameters) {

            // first off all pointers are passed simply
            if (n.isReference or n.pointerLevel)
                context.addInteger({false, argNumber, 8});
            else if (n.type->isPrimitive) {
                if (n.type->isPrimitive != Type::floatingPoint)
                    context.addInteger({false, argNumber, uint32_t(n.type->typeSize)});
                else
                    context.addFloat({false, argNumber, uint32_t(n.type->typeSize)});
            }
            // now it's time for complex types
            // TODO: somehow implement XMM register passing with float-only types, might be rarely used thankfully
            else if (n.type->typeSize <= 8)
                context.addInteger({false, argNumber, uint32_t(n.type->typeSize > 4 ? 8 : n.type->typeSize > 2 ? 4 : n.type->typeSize > 1 ? 2 : 1)});
            else if (n.type->typeSize <= 16) {
                // here we need to split it into 2 different arguments, yay!
                context.addInteger({false, argNumber, 8, true});
                context.addInteger({false, argNumber, uint32_t(n.type->typeSize > 12 ? 8 : n.type->typeSize > 10 ? 4 : n.type->typeSize > 9 ? 2 : 1), true, 8});
            }
            else {
                // here we get a pointer and are done with it
                context.addInteger({true, argNumber, 8});
            }

            argNumber++;
        }

        context.addStackToResult();
    }

    std::pair<std::vector<ArgumentLocation>, int32_t> GetFunctionMethodArgumentLocations(ParserFunctionMethod& target) {
        std::vector <ArgumentLocation> result;
        int32_t stackOffset = 0;

        std::vector <TypeInfo> types;
        types.reserve(target.parameters.size() + target.isMethod and not target.isOperator);
        if (target.isMethod and not target.isOperator)
            types.emplace_back(target.parentType, TypeMeta(0, true, true));
        for (auto& n : target.parameters)
            types.emplace_back(n.typeObject, n.typeMeta());

        if (Options::targetSystem == "LINUX")
            GetArgumentLocationsLinux(types, result, stackOffset);
        else
            Error("Non-linux argument place preparation not implemented!");

        return {result, stackOffset};
    }

    // for syscalls only
    std::pair<std::vector <ArgumentLocation>, int32_t> GetFunctionMethodArgumentLocations(std::vector<Bytecode*>& target) {
        std::vector <ArgumentLocation> result;
        int32_t stackOffset = 0;

        std::vector <TypeInfo> types{};
        types.reserve(target.size());
        for (auto& n : target)
            types.emplace_back(n->opType, n->opMeta);

        if (Options::targetSystem == "LINUX")
            GetArgumentLocationsLinux(types, result, stackOffset, true);
        else
            Error("Non-linux argument place preparation not implemented!");

        return {result, stackOffset};
    }

}
