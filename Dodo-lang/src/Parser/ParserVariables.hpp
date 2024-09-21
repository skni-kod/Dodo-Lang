#ifndef DODO_LANG_PARSER_VARIABLES_HPP
#define DODO_LANG_PARSER_VARIABLES_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "MapWrapper.tpp"
#include "Parser.hpp"
#include "Generator.tpp"

#define INSERT_SUBTYPE_ENUM \
enum Subtype { \
    value, pointer, reference, none \
};

#define INSERT_CONDITION_ENUM \
enum Condition { \
    equals, notEquals, greater, greaterEqual, lesser, lesserEqual \
};

bool IsType(const std::string& token);
bool IsObject(const std::string& token);
// that is if it can be used for a variable
bool IsDeclarable(const std::string& token);

struct ParserType {
    enum Type {
        signedInteger, unsignedInteger, floatingPoint
    };
    std::string name;
    uint8_t type:2;                         // allowed values 0-2
    uint8_t size:6;                         // allowed values 1-8 (maybe more in future)
    ParserType(uint8_t type, uint8_t size); // assumes valid input
    ParserType(uint8_t type, uint8_t size, std::string name); // assumes valid input
    ParserType() = default;
};

struct FunctionArgument {
    INSERT_SUBTYPE_ENUM
    uint8_t type = Subtype::value;
    bool isMutable = false;
    std::string name;
    std::string typeName;
};

struct ParserValue {
    enum Node {
        constant, variable, operation, empty
    };
    enum Operation {
        // !!! keep this updated with Bytecode list, maybe do a macro
        addition, subtraction, multiplication, division, functionCall
    };
    enum Value {
        signedInteger, unsignedInteger, floatingPoint
    };
    uint8_t nodeType = 0;
    uint8_t operationType = 0;
    uint8_t valueType = 0;
    bool isNegative = false;
    std::unique_ptr<ParserValue> left = nullptr;
    std::unique_ptr<ParserValue> right = nullptr;
    std::unique_ptr<std::string> value = nullptr;
    // takes the input and prepares the value and it's classification
    void fillValue(std::string val);
};

struct DeclarationInstruction {
    std::string typeName;
    std::string name;
    ParserValue expression;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
    bool isMutable = false;
};

struct ValueChangeInstruction {
    std::string name;
    ParserValue expression;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
};

struct ReturnInstruction {
    ParserValue expression;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
};

// no return value catching
struct FunctionCallInstruction {
    std::string functionName;
    ParserValue arguments;
};

struct ParserCondition {
    INSERT_CONDITION_ENUM
    uint8_t type = 0;
    ParserValue left, right;
    void SetOperand(const std::string& value);
};

struct IfInstruction {
    ParserCondition condition;
};

struct WhileInstruction {
    ParserCondition condition;
};

struct DoWhileInstruction {
    ParserCondition condition;
};

struct ForLoopVariable {
    std::string typeName;
    std::string identifier;
    ParserValue value;
};

struct FunctionInstruction;

struct ForInstruction {
    std::vector <ForLoopVariable> variables;
    ParserCondition condition;
    std::vector<FunctionInstruction> instructions;
};

struct FunctionInstruction {
    enum Type {
        declaration, returnValue, valueChange, functionCall, ifStatement, whileStatement, doWhileStatement,
        forStatement, elseStatement, beginScope, endScope
    };
    union Variant {
        DeclarationInstruction* declarationInstruction = nullptr;
        ReturnInstruction* returnInstruction;
        ValueChangeInstruction* valueChangeInstruction;
        FunctionCallInstruction* functionCallInstruction;
        IfInstruction* ifInstruction;
        WhileInstruction* whileInstruction;
        DoWhileInstruction* doWhileInstruction;
        ForInstruction* forInstruction;
    }variant;
    uint8_t type = 0;
    ~FunctionInstruction();
    FunctionInstruction() = default;
    FunctionInstruction(const FunctionInstruction& F);
    FunctionInstruction(FunctionInstruction&& F) noexcept ;
    void DeleteAfterCopy();

};

struct ParserFunction {
    INSERT_SUBTYPE_ENUM
    std::string returnType;
    std::string name;
    uint8_t returnValueType:2 = 0;
    std::vector <FunctionArgument> arguments;
    std::vector <FunctionInstruction> instructions;
};

inline MapWrapper <std::string, ParserType> parserTypes;
inline MapWrapper <std::string, ParserFunction> parserFunctions;


#endif //DODO_LANG_PARSER_VARIABLES_HPP