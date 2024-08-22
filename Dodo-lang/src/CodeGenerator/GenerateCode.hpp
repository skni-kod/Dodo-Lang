#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>
#include <fstream>

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
    bool nonDefault() const;
    std::string registerBySize(uint16_t size) const;
};

char AddInstructionPostfix(uint16_t size);
std::string AddRegisterA(uint16_t size);

void CodeError(std::string message);

inline std::string TargetArchitecture;
inline std::string TargetSystem;

void GenerateCode();

std::string ConvertSizeInRegister(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, RegisterNames registers = {}, bool returnToOriginal = false);

std::string GenerateFunctionCall(std::ofstream& out, uint64_t stackOffset, uint64_t& stackPointerOffset,
                                 const std::string& functionName, uint16_t outputSize, RegisterNames outputLocation = {});

#endif //DODO_LANG_GENERATE_CODE_HPP
