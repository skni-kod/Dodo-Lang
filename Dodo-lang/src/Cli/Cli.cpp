#include <string>
#include <unordered_map>
#include <iostream>
#include <print>

#include "Options.hpp"
#include "Cli.hpp"



bool isNextExecutableName = false;
bool cliError = false;

void CLIHandlerO(std::string& arg) {
    isNextExecutableName = true;
}

void CLIHandlerhH(std::string& arg) {
    Options::helpOption = true;
}

void CLIHandlerR(std::string& arg) {
    // TODO: add optimization enabling here
}

void CLIHandlerD(std::string& arg) {
    // TODO: add optimization disabling here
}

void CLIHandlerL1(std::string& arg) {
    Options::informationLevel = Options::InformationLevel::minimal;
}

void CLIHandlerL2(std::string& arg) {
    Options::informationLevel = Options::InformationLevel::general;
}

void CLIHandlerL3(std::string& arg) {
    Options::informationLevel = Options::InformationLevel::full;
}

void CLIHandlerHelp(std::string& arg) {
    Options::helpOption = true;
}

void CLIHandlerTarget(std::string& arg) {
    if (arg.empty()) cliError = true;
    // system will be handled via macros
    else Options::targetSystem = arg;
}

void CLIHandlerPlatform(std::string& arg) {
    if (arg.empty()) cliError = true;
    else if (arg == "x86_64") {
        Options::targetArchitecture = Options::TargetArchitecture::x86_64;
        Options::addressSize = 8;
    }
    else {
        std::print("Unsupported platform!\n");
        cliError = true;
    }
}

void CLIHandlerStdLibDirectory(std::string& arg) {
    if (arg.empty()) cliError = true;
    else Options::stdlibDirectory = arg;
}

void CLIHandlerImport(std::string& arg) {
    if (arg.empty()) cliError = true;
    else Options::importDirectories.emplace_back(arg);
}

void CLIHandlerExtensions(std::string& arg) {
    if (arg.empty()) cliError = true;
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
             if (arg == "x86_64_v1" or arg == "AMD64_v1") Options::architectureVersion = Options::AMD64_v1;
        else if (arg == "x86_64_v2" or arg == "AMD64_v2") Options::architectureVersion = Options::AMD64_v2;
        else if (arg == "x86_64_v3" or arg == "AMD64_v3") Options::architectureVersion = Options::AMD64_v3;
        else if (arg == "x86_64_v4" or arg == "AMD64_v4") Options::architectureVersion = Options::AMD64_v4;
        else cliError = true;
    }
    else {
        std::print("Non x86-64 extensions not supported!");
    }
}

std::unordered_map <std::string, void (*)(std::string&)> CLIHandlers = {
    {"o", &CLIHandlerO},
    {"O", &CLIHandlerO},
    {"r", &CLIHandlerR},
    {"R", &CLIHandlerR},
    {"d", &CLIHandlerD},
    {"D", &CLIHandlerD},
    {"h", &CLIHandlerhH},
    {"H", &CLIHandlerhH},
    {"l1", &CLIHandlerL1},
    {"L1", &CLIHandlerL1},
    {"l2", &CLIHandlerL2},
    {"L2", &CLIHandlerL2},
    {"l3", &CLIHandlerL3},
    {"L3", &CLIHandlerL3},
    {"help", &CLIHandlerHelp},
    {"HELP", &CLIHandlerHelp},
    {"import", &CLIHandlerImport},
    {"IMPORT", &CLIHandlerImport},
    {"target", &CLIHandlerTarget},
    {"TARGET", &CLIHandlerTarget},
    {"platform", &CLIHandlerPlatform},
    {"PLATFORM", &CLIHandlerPlatform},
    {"stdlibdirectory", &CLIHandlerStdLibDirectory},
    {"STDLIBDIRECTORY", &CLIHandlerStdLibDirectory},
    {"extensions", &CLIHandlerExtensions},
    {"EXTENSIONS", &CLIHandlerExtensions}
};


bool ApplyCommandLineArguments(int argc, char** argv) {

    fs::path firstPath = {};
    for (uint64_t n = 1; n < argc; n++) {
        std::string current = {argv[n]};

        if (not current.empty()) {
            if (current.front() == '-') {
                std::string name = current.substr(1);;
                std::string argument;

                if (current.contains('=')) {
                    argument = name.substr(name.find_first_of('=') + 1);
                }

                if (not CLIHandlers.contains(name)) {
                    std::print("Invalid argument passed at position: {}!\n", n + 1);
                    return false;
                }

                CLIHandlers[name](argument);

                if (cliError) {
                    std::print("Malformed argument passed at position: {}!\n", n + 1);
                    return false;
                }
            }
            else {
                // it's an input file
                if (isNextExecutableName) {
                    isNextExecutableName = false;
                    Options::outputName = current;
                    continue;
                }
                if (Options::inputFiles.empty()) {
                    firstPath = absolute(fs::path(current)).parent_path();
                }
                Options::inputFiles.emplace(fs::path(current).filename());
            }
        }
    }

    if (Options::inputFiles.empty()) {
        std::cout << "No input files were passed!";
        return false;
    }

    if (Options::importDirectories.empty()) {
        Options::importDirectories.push_back(firstPath);
        std::cout << "Since no import directory was passed explicitly, project directory was automatically set to: " << Options::importDirectories.back() <<
            ", which is the location of first passed input file. Remember to set them if using external imports!\n";
    }

    if (Options::stdlibDirectory == "") {
        std::cout << "Standard library directory is not set!\n";
        return false;
    }
    Options::importDirectories.push_back(Options::stdlibDirectory);
    Options::inputFiles.emplace("_BaseImports.dodo");

    // it can realistically use quite a bit of memory
    CLIHandlers.clear();
    if (Options::architectureVersion == Options::ArchitectureVersion::None) Options::architectureVersion = Options::ArchitectureVersion::AMD64_v1;

    return true;
}
