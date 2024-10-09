#ifndef DODO_LANG_GENERATE_CODE_INTERNAL_HPP
#define DODO_LANG_GENERATE_CODE_INTERNAL_HPP

#include "ParserVariables.hpp"

void GenerateFunctionStepOne(const ParserFunction& function);

void GenerateFunctionStepTwo(const ParserFunction& function);

inline bool doAddLeave = false;

void AddFunctionHeaders();

inline const std::string* lastFunctionName = nullptr;

#endif //DODO_LANG_GENERATE_CODE_INTERNAL_HPP
