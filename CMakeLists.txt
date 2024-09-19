cmake_minimum_required(VERSION 3.27)
project(Dodo_Lang)

set(CMAKE_CXX_STANDARD 23)

include_directories(Dodo-lang/src)
include_directories(Dodo-lang/src/CodeGenerator)
include_directories(Dodo-lang/src/Parser)
include_directories(Dodo-lang/src/Parser/CreateTree)
include_directories(Dodo-lang/src/Parser/SyntaxAnalysis)

add_executable(Dodo_Lang
        Dodo-lang/src/Parser/Generator.tpp
        Dodo-lang/src/Parser/MapWrapper.tpp
        Dodo-lang/src/Parser/Parser.cpp
        Dodo-lang/src/Parser/Parser.hpp
        Dodo-lang/src/Parser/ParserVariables.cpp
        Dodo-lang/src/Parser/ParserVariables.hpp
        Dodo-lang/src/Flags.hpp
        Dodo-lang/src/lexical_analysis.cpp
        Dodo-lang/src/LexicalAnalysis.hpp
        Dodo-lang/src/LexicalToken.cpp
        Dodo-lang/src/LexicalToken.hpp
        Dodo-lang/src/main.cpp
        Dodo-lang/src/Parser/SyntaxAnalysis/SyntaxAnalysis.hpp
        Dodo-lang/src/Parser/SyntaxAnalysis/AnalysisRunner.cpp
        Dodo-lang/src/Parser/SyntaxAnalysis/AnalysisInternal.hpp
        Dodo-lang/src/Parser/SyntaxAnalysis/CreateType.cpp
        Dodo-lang/src/Parser/SyntaxAnalysis/CreateFunction.cpp
        Dodo-lang/src/Parser/SyntaxAnalysis/CreateInstruction.cpp
        Dodo-lang/src/CodeGenerator/GenerateCode.hpp
        Dodo-lang/src/Parser/SyntaxAnalysis/MathParser.cpp
        Dodo-lang/src/CodeGenerator/GenerateCode.cpp
        Dodo-lang/src/CodeGenerator/GenerateCodeInternal.hpp
        Dodo-lang/src/CodeGenerator/MemoryStructure.hpp
        Dodo-lang/src/CodeGenerator/MemoryStructure.cpp
        Dodo-lang/src/CodeGenerator/FunctionStepOne.cpp
        Dodo-lang/src/CodeGenerator/Bytecode.hpp
        Dodo-lang/src/CodeGenerator/Bytecode.cpp
)

# uncomment when project works well
#if(MSVC)
#    target_compile_options(Dodo_Lang PRIVATE /W4 /WX)
#else()
#    target_compile_options(Dodo_Lang PRIVATE -Wall -Wextra -Wpedantic -Werror)
#endif()

