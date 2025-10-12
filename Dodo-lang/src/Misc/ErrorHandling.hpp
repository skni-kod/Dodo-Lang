#ifndef DODO_LANG_ERRORHANDLING_HPP
#define DODO_LANG_ERRORHANDLING_HPP

#include <exception>
#include <string>
#include "Options.hpp"

/// <summary>
/// A standardised error handling exception class
/// </summary>
class CompilerException : std::exception {
public:
    /// <summary>
    /// Returns error message
    /// </summary>
    /// <returns>Message with what went wrong</returns>
    const char* what();

    /// <summary>
    /// Creates a new instance of exception to be thrown
    /// </summary>
    /// <param name="message">Message body to be displayed, surrounded by other information depending on compilation stage</param>
    explicit CompilerException(std::string message);
private:
    const char* message{};
};

/// <summary>
/// A macro that shortens error throws for standard use
/// </summary>
/// <param name="message">Message to show in error</param>
#define Error(message) throw CompilerException(message)

/// <summary>
/// A macro that shortens error throws for use when something is unimplemented
/// </summary>
#define Unimplemented() throw CompilerException("Feature unimplemented")

#ifdef EXTENDED_CHECKS
/// <summary>
/// A macro that shortens error throws for use in debug mode, does nothing when building in release
/// As such things that slow down the compilation but are convenient can be used here without any problems for checks
/// </summary>
/// <param name="condition">Condition that needs to be true to trigger the error</param>
/// <param name="message">Message to show in error</param>
#define DebugError(condition, message) if (condition) throw CompilerException("(Extended debug error) " + std::string(message))
#else
/// <summary>
/// A macro that shortens error throws for use in debug mode, does nothing when building in release
/// As such things that slow down the compilation but are convenient can be used here without any problems for checks
/// </summary>
/// <param name="condition">Condition that needs to be true to trigger the error</param>
/// <param name="message">Message to show in error</param>
#define DebugError(condition, message)
#endif

namespace CompilationStage {
    /// <summary>
    /// Stage of compilation for use in errors presented to the user
    /// </summary>
    enum Type {
        setup, lexing, parsing, bytecode, assembly, output
    };
}

/// <summary>
/// Sets the current compilation stage for the needs of user error presentation
/// </summary>
/// <param name="stage">Current stage</param>
void SetCompilationStage(CompilationStage::Type stage);

#endif //DODO_LANG_ERRORHANDLING_HPP
