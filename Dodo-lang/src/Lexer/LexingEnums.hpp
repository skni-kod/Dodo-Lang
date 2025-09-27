#ifndef LEXINGENUMS_HPP
#define LEXINGENUMS_HPP

namespace Keyword {
    enum KeywordType {
        None, Primitive, TypeSI, TypeUI, TypeFP, Type, Void, Operator, Return, Import, End, After,
        Extern, Syscall, Public, Private, Protected, Let, Mut, Const, Comma, Dot, Member,
        Break, Continue, Switch, If, While, Else, Case, Do, For
    };
}

struct LexerToken;

namespace Token {
    enum Type {
        Identifier, Operator, Number, String, Keyword, Unknown
    };
}

namespace Operator {

    enum Type {
        // enum order affects order of operations
        // lower code means being before the one with higher
        Convert, Address, Dereference,
        Not, BinNot,
        // first kinda like PEMDAS
        Increment, Decrement, Power, Multiply, Divide, Modulo, Add, Subtract,
        ShiftRight, ShiftLeft,
        // boolean order similar to C/C++ but expanded for more things
        BinNAnd, BinAnd,
        BinXOr,
        BinNOr, BinOr,
        BinImply, BinNImply,
        // assign, compare and logical last so that they are evaluated after everything
        Lesser, Greater, Equals, LesserEqual, GreaterEqual, NotEqual, And, NAnd, XOr, Or, NOr, Imply, NImply, Assign,
        // these are special cases that do not use order of operations
        Macro, BracketOpen, BracketClose, Bracket,
        BraceOpen, BraceClose, Brace, IndexOpen, IndexClose, Index, Destructor, Constructor, None

    };
}

#endif //LEXINGENUMS_HPP
