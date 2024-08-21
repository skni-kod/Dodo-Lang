#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>

class CodeException : public std::exception {
public:
    const char* what();
};

void CodeError(std::string message);

inline std::string TargetArchitecture;
inline std::string TargetSystem;

void GenerateCode();


#endif //DODO_LANG_GENERATE_CODE_HPP
