#ifndef LEXINGINTERNAL_HPP
#define LEXINGINTERNAL_HPP

#include "Lexing.hpp"
#include "unordered_map"

inline std::unordered_map keywordsAndOperators {
    std::pair<std::string, LexerToken> ("primitive",        LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Primitive), 0)),
    std::pair<std::string, LexerToken> ("SIGNED_INTEGER",   LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::TypeSI), 0)),
    std::pair<std::string, LexerToken> ("UNSIGNED_INTEGER", LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::TypeUI), 0)),
    std::pair<std::string, LexerToken> ("FLOATING_POINT",   LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::TypeFP), 0)),
    std::pair<std::string, LexerToken> ("type",             LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Type), 0)),
    std::pair<std::string, LexerToken> ("void",             LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Void), 0)),
    std::pair<std::string, LexerToken> ("operator",         LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Operator), 0)),
    std::pair<std::string, LexerToken> ("return",           LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Return), 0)),
    std::pair<std::string, LexerToken> ("import",           LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Import), 0)),
    std::pair<std::string, LexerToken> ("extern",           LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Extern), 0)),
    std::pair<std::string, LexerToken> ("syscall",          LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Syscall), 0)),
    std::pair<std::string, LexerToken> ("public",           LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Public), 0)),
    std::pair<std::string, LexerToken> ("private",          LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Private), 0)),
    std::pair<std::string, LexerToken> ("protected",        LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Protected), 0)),
    std::pair<std::string, LexerToken> ("let",              LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Let), 0)),
    std::pair<std::string, LexerToken> ("mut",              LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Mut), 0)),
    std::pair<std::string, LexerToken> ("const",            LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Const), 0)),
    std::pair<std::string, LexerToken> (";",                LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::End), 0)),
    std::pair<std::string, LexerToken> ("break",            LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Break), 0)),
    std::pair<std::string, LexerToken> ("continue",         LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Continue), 0)),
    std::pair<std::string, LexerToken> ("switch",           LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Switch), 0)),
    std::pair<std::string, LexerToken> ("if",               LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::If), 0)),
    std::pair<std::string, LexerToken> ("while",            LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::While), 0)),
    std::pair<std::string, LexerToken> ("for",              LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::For), 0)),
    std::pair<std::string, LexerToken> ("else",             LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Else), 0)),
    std::pair<std::string, LexerToken> ("case",             LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Case), 0)),
    std::pair<std::string, LexerToken> ("do",               LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Do), 0)),
    std::pair<std::string, LexerToken> (":",                LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::After), 0)),
    std::pair<std::string, LexerToken> (",",                LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Comma), 0)),
    std::pair<std::string, LexerToken> (".",                LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Dot), 0)),
    std::pair<std::string, LexerToken> ("::",               LexerToken(Token::Keyword,  static_cast <uint64_t> (Keyword::Member), 0)),
    std::pair<std::string, LexerToken> ("&",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Address), 0)),
    std::pair<std::string, LexerToken> ("$",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Cast), 0)),
    std::pair<std::string, LexerToken> ("cast",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Cast), 0)),
    std::pair<std::string, LexerToken> ("=",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Assign), 0)),
    std::pair<std::string, LexerToken> ("assign",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Assign), 0, true)),
    std::pair<std::string, LexerToken> ("+",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Add), 0)),
    std::pair<std::string, LexerToken> ("add",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Add), 0, true)),
    std::pair<std::string, LexerToken> ("-",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Subtract), 0)),
    std::pair<std::string, LexerToken> ("subtract",         LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Subtract), 0, true)),
    std::pair<std::string, LexerToken> ("*",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Multiply), 0)),
    std::pair<std::string, LexerToken> ("multiply",         LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Multiply), 0, true)),
    std::pair<std::string, LexerToken> ("/",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Divide), 0)),
    std::pair<std::string, LexerToken> ("divide",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Divide), 0, true)),
    std::pair<std::string, LexerToken> ("power",            LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Power), 0, true)),
    std::pair<std::string, LexerToken> ("^^",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Power), 0)),
    std::pair<std::string, LexerToken> ("modulo",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Modulo), 0, true)),
    std::pair<std::string, LexerToken> ("%",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Modulo), 0)),
    std::pair<std::string, LexerToken> ("#",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Macro), 0)),
    std::pair<std::string, LexerToken> ("not",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Not), 0, true)),
    std::pair<std::string, LexerToken> ("!",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Not), 0)),
    std::pair<std::string, LexerToken> ("bnot",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNot), 0, true)),
    std::pair<std::string, LexerToken> (".!",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNot), 0)),
    std::pair<std::string, LexerToken> ("or",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Or), 0, true)),
    std::pair<std::string, LexerToken> ("|",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Or), 0)),
    std::pair<std::string, LexerToken> ("bor",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinOr), 0, true)),
    std::pair<std::string, LexerToken> (".|",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinOr), 0)),
    std::pair<std::string, LexerToken> ("nand",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NAnd), 0, true)),
    std::pair<std::string, LexerToken> ("!&",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NAnd), 0, true)),
    std::pair<std::string, LexerToken> ("bnand",            LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNAnd), 0, true)),
    std::pair<std::string, LexerToken> (".!&",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNAnd), 0, true)),
    std::pair<std::string, LexerToken> ("and",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::And), 0, true)),
    std::pair<std::string, LexerToken> ("&",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::And), 0)),
    std::pair<std::string, LexerToken> ("band",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinAnd), 0, true)),
    std::pair<std::string, LexerToken> (".&",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinAnd), 0)),
    std::pair<std::string, LexerToken> ("nor",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NOr), 0)),
    std::pair<std::string, LexerToken> ("!|",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NOr), 0)),
    std::pair<std::string, LexerToken> ("binnor",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNOr), 0, true)),
    std::pair<std::string, LexerToken> (".!|",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNOr), 0)),
    std::pair<std::string, LexerToken> ("xor",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::XOr), 0, true)),
    std::pair<std::string, LexerToken> ("^",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::XOr), 0)),
    std::pair<std::string, LexerToken> ("bxor",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinXOr), 0, true)),
    std::pair<std::string, LexerToken> (".^",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinXOr), 0)),
    std::pair<std::string, LexerToken> ("imply",            LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Imply), 0, true)),
    std::pair<std::string, LexerToken> ("=>",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Imply), 0)),
    std::pair<std::string, LexerToken> ("bimply",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinImply), 0, true)),
    std::pair<std::string, LexerToken> (".=>",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinImply), 0)),
    std::pair<std::string, LexerToken> ("nimply",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NImply), 0, true)),
    std::pair<std::string, LexerToken> ("!=>",              LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NImply), 0)),
    std::pair<std::string, LexerToken> ("bnimply",          LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNImply), 0, true)),
    std::pair<std::string, LexerToken> (".!=>",             LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BinNImply), 0)),
    std::pair<std::string, LexerToken> ("shift_right",      LexerToken(Token::Operator, static_cast <uint64_t>(Operator::ShiftRight), 0, true)),
    std::pair<std::string, LexerToken> (">>",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::ShiftRight), 0)),
    std::pair<std::string, LexerToken> ("shift_left",       LexerToken(Token::Operator, static_cast <uint64_t>(Operator::ShiftLeft), 0, true)),
    std::pair<std::string, LexerToken> ("<<",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::ShiftLeft), 0)),
    std::pair<std::string, LexerToken> ("lesser",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Lesser), 0, true)),
    std::pair<std::string, LexerToken> ("<",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Lesser), 0)),
    std::pair<std::string, LexerToken> ("greater",          LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Greater), 0, true)),
    std::pair<std::string, LexerToken> (">",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Greater), 0)),
    std::pair<std::string, LexerToken> ("equals",           LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Equals), 0, true)),
    std::pair<std::string, LexerToken> ("==",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Equals), 0)),
    std::pair<std::string, LexerToken> ("lesser_equal",     LexerToken(Token::Operator, static_cast <uint64_t>(Operator::LesserEqual), 0, true)),
    std::pair<std::string, LexerToken> ("<=",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::LesserEqual), 0)),
    std::pair<std::string, LexerToken> ("greater_equal",    LexerToken(Token::Operator, static_cast <uint64_t>(Operator::GreaterEqual), 0, true)),
    std::pair<std::string, LexerToken> (">=",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::GreaterEqual), 0)),
    std::pair<std::string, LexerToken> ("not_equal",        LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NotEqual), 0, true)),
    std::pair<std::string, LexerToken> ("!=",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::NotEqual), 0)),
    std::pair<std::string, LexerToken> ("(",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BracketOpen), 0)),
    std::pair<std::string, LexerToken> (")",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BracketClose), 0)),
    std::pair<std::string, LexerToken> ("()",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Bracket), 0)),
    std::pair<std::string, LexerToken> ("{",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BraceOpen), 0)),
    std::pair<std::string, LexerToken> ("}",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::BraceClose), 0)),
    std::pair<std::string, LexerToken> ("{}",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Brace), 0)),
    std::pair<std::string, LexerToken> ("[",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::IndexOpen), 0)),
    std::pair<std::string, LexerToken> ("]",                LexerToken(Token::Operator, static_cast <uint64_t>(Operator::IndexClose), 0)),
    std::pair<std::string, LexerToken> ("[]",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Index), 0)),
    std::pair<std::string, LexerToken> ("++",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Increment), 0)),
    std::pair<std::string, LexerToken> ("--",               LexerToken(Token::Operator, static_cast <uint64_t>(Operator::Decrement), 0))};

#endif //LEXINGINTERNAL_HPP
