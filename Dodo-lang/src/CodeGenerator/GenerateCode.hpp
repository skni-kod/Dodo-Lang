#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>
#include "ParserVariables.hpp"

#define EXPRESSION_SIGN "="

inline const FunctionInstruction* currentlyGeneratedInstruction = nullptr;


std::ostream& operator<<(std::ostream& out, const VariableType& type);

class __CodeGeneratorException : public std::exception {
public:
    const char* what();
};

void CodeGeneratorError(std::string message);

void GenerateCode();

void Warning(std::string message);

struct StringData {
    std::string* ptr = nullptr;
    bool isMutable = false;
    StringData(std::string* ptr, bool isMutable);
};

inline std::vector <StringData> Strings;

#endif //DODO_LANG_GENERATE_CODE_HPP
