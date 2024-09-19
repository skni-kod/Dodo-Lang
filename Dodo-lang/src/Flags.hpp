#ifndef DODO_LANG_FLAGS_HPP
#define DODO_LANG_FLAGS_HPP

#include <cstdint>

namespace options {
    // default values inside
    enum InformationLevel {
        // only general stages, time
        minimal,
        // stages, amounts of files, types, time
        general,
        // full information about the compilation process
        full
    };
    inline uint8_t informationLevel = InformationLevel::full;
    inline std::string targetArchitecture = "X86_64";
    inline std::string targetSystem = "LINUX";
    inline std::string commentPrefix = "# ";
}

#endif //DODO_LANG_FLAGS_HPP
