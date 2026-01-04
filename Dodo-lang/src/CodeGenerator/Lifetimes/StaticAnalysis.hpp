#ifndef STATIC_ANALYSIS_HPP
#define STATIC_ANALYSIS_HPP
#include "Bytecode.hpp"

/// <summary>
/// Runs syntax analysis on context and generates new bytecodes
/// </summary>
/// <param name="context">Initially produced context</param>
void RunStaticAnalysis(Context& context);

/// <summary>
/// Removes non-call instructions that produce unused results
/// </summary>
/// <param name="context">Context containing unused instructions</param>
void RemoveUnusedInstructions(Context& context);

/// <summary>
/// Applies destructors to variables used in instructions by tracking their usage
/// </summary>
/// <param name="context">Context containing the code to control</param>
void ApplyRAII(Context& context);

#endif //STATIC_ANALYSIS_HPP
