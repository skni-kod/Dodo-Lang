#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>

#define EXPRESSION_SIGN "="

class __CodeGeneratorException : public std::exception {
public:
    const char* what();
};

void CodeGeneratorError(std::string message);

void GenerateCode();

void Warning(std::string message);

#endif //DODO_LANG_GENERATE_CODE_HPP
