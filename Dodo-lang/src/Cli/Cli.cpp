#include "Cli.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <print>

#include "Options.hpp"

bool ApplyCommandLineArguments(int argc, char** argv) {

    bool isNextExecutableName = false;

    // TODO: add a awy not to include stdlib headers by default

    for (uint64_t n = 1; n < argc; n++) {
        std::string current = {argv[n]};

        if (not current.empty()) {
            if (current.front() == '-') {
                // it's an argument
                switch (current.size()) {
                    case 1:
                        std::print("Invalid argument passed at position: {}!", n + 1);
                        return false;
                    case 2:
                        switch (current[1]) {
                            case 'o':
                                isNextExecutableName = true;
                                break;
                            case 'h':
                                Options::helpOption = true;
                            return true;
                            case 'd':
                                Optimizations::skipUselessMoves = false;
                                Optimizations::mergeThreeOperandInstruction = false;
                                Optimizations::skipDoubleJumps = false;
                                Optimizations::skipUnusedVariables = false;
                                Optimizations::swapExpressionOperands = false;
                                Optimizations::checkPotentialUselessStores = false;
                                Optimizations::optimizeBytecode = false;
                                Optimizations::replaceKnownValueVariables = false;
                                // TODO: change this
                                Optimizations::groupVariableInstances = true;
                            break;
                            default:
                                std::print("Invalid argument passed at position: {}!", n + 1);
                            return false;
                        }
                    break;
                    case 3:
                        if (current[1] == 'l') {
                            switch (current[2]) {
                                case '1':
                                    Options::informationLevel = Options::InformationLevel::minimal;
                                    break;
                                case '2':
                                    Options::informationLevel = Options::InformationLevel::general;
                                break;
                                case '3':
                                    Options::informationLevel = Options::InformationLevel::full;
                                break;
                                default:
                                    std::print("Invalid argument passed at position: {}!", n + 1);
                                return false;
                            }
                        }
                        else {
                            std::print("Invalid argument passed at position: {}!", n + 1);
                            return false;
                        }
                    break;
                    default:
                        if (current == "-help") {
                            Options::helpOption = true;
                            return true;
                        }
                        if (current.starts_with("-target=")) {
                            Options::targetSystem = current.substr(8);
                        }
                        else if (current.starts_with("-platform=")) {
                            if (current == "-platform=x86_64") {
                                Options::targetArchitecture = Options::TargetArchitecture::x86_64;
                                Options::addressSize = 8;
                            }
                            else {
                                std::print("Unsupported platform at position: {}!", n + 1);
                                return false;
                            }
                        }
                        else if (current.starts_with("-stdlibDirectory=")) {
                            Options::stdlibDirectory = current.substr(17);
                        }
                        else if (current.starts_with("-import=")) {
                            Options::importDirectories.emplace_back(current.substr(8));
                        }
                        else {
                            std::print("Invalid argument passed at position: {}!", n + 1);
                            return false;
                        }
                }
            }
            else {
                // it's an input file
                if (isNextExecutableName) {
                    isNextExecutableName = false;
                    Options::outputName = current;
                    continue;
                }
                Options::inputFiles.emplace(fs::path(current).filename());
            }
        }
    }

    if (Options::inputFiles.empty()) {
        std::print("No input files were passed!");
        return false;
    }

    if (Options::importDirectories.empty()) {
        Options::importDirectories.push_back(absolute(Options::inputFiles.front()).parent_path());
        std::cout << "Warning: As it was not defined, project directory was automatically set to: " << Options::importDirectories.back() << ". Remember to set it if using external imports!\n";

    }

    if (Options::stdlibDirectory == "") {
        std::print("Standard library directory is not set!\n");
        return false;
    }
    Options::importDirectories.push_back(Options::stdlibDirectory);
    Options::inputFiles.emplace("_BaseImports.dodo");

    return true;
}
