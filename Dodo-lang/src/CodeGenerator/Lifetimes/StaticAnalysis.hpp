#ifndef STATIC_ANALYSIS_HPP
#define STATIC_ANALYSIS_HPP
#include "Bytecode.hpp"

/// <summary>
/// Runs syntax analysis on context and generates new bytecodes
/// </summary>
/// <param name="context">Initially produced context</param>
void RunStaticAnalysis(Context& context);

#endif //STATIC_ANALYSIS_HPP
