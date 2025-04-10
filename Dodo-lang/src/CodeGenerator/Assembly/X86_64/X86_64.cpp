#include "X86_64.hpp"

namespace x86_64 {
    std::vector <MoveInfo> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc) {

        // assumes every move contains known locations only, since we need to know which one exactly to move
        // so those can be: registers, stack offsets, literals, labels and addresses

        // easier to access
        auto& s = move.source;
        auto& t = move.target;
        
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
            )) return {move};
        if (s.type == Type::address and t.type != Type::address
            and (
                (not s.useAddress and s.op == Location::reg and t.useAddress and t.op == Location::reg)
                or
                (s.useAddress and s.op == Location::reg and not t.useAddress and t.op == Location::reg)
                or
                (not s.useAddress and s.op == Location::imm and t.useAddress and t.op == Location::reg)
            )) return {move};

        CodeGeneratorError("Internal: unimplemented conversion!");
        return {};
    }
}