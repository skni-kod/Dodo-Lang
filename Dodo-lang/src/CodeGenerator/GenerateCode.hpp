#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>
#include <fstream>
#include "StackVector.hpp"
#include "ParserVariables.hpp"

class CodeException : public std::exception {
public:
    const char* what();
};

struct RegisterNames {
    std::string size1 = "%al";
    std::string size2 = "%ax";
    std::string size4 = "%eax";
    std::string size8 = "%rax";
    RegisterNames(std::string size1, std::string size2, std::string size4, std::string size8);
    RegisterNames() = default;
    [[nodiscard]] bool nonDefault() const;
    [[nodiscard]] std::string registerBySize(uint16_t size) const;
};

struct ValueType {
    uint8_t reg:1 = 1;
    uint8_t val:1 = 1;
    uint8_t sta:1 = 1;
    uint8_t hea:1 = 1;
    ValueType(uint8_t reg, uint8_t val, uint8_t sta, uint8_t hea);
};

char AddInstructionPostfix(uint16_t size);
std::string AddRegisterA(uint16_t size);
uint64_t GetTypeSize(const std::string& typeName);
uint64_t ConvertValue(const std::string& value, uint16_t bitSize);
uint64_t ConvertValue(const ParserValue& value, uint16_t bitSize);

void CodeError(std::string message);

inline std::string TargetArchitecture;
inline std::string TargetSystem;

void GenerateCode();

// converts value in register from to given size and type
std::string ConvertValueInRegister(std::ofstream& out, uint8_t originalSize, uint8_t targetSize,
                                   uint8_t inputType, uint8_t outputType,
                                   RegisterNames registers = {}, bool returnToOriginal = false);

std::string ConvertSizeFromStack(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, int64_t offset,
                                 uint8_t inputType, uint8_t outputType,
                                 bool mustBeReg = false, StackVector* stack = nullptr, bool mustUseGivenReg = false, RegisterNames registers = {});

std::string GenerateFunctionCall(std::ofstream& out, StackVector& variables,
                                 const std::string& functionName, uint16_t outputSize, uint8_t outputType, const ParserValue* arguments = nullptr, RegisterNames outputLocation = {});

std::string CalculateExpression(StackVector& variables, std::ofstream& out, uint16_t outputSize,
                                const ParserValue& expression, uint8_t outputType,
                                ValueType returnValueLocations = {1, 1, 1, 1}, RegisterNames registers = {});

#endif //DODO_LANG_GENERATE_CODE_HPP
