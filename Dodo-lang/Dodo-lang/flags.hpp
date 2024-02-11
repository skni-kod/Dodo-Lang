#ifndef DODO_LANG_FLAGS_HPP
#define DODO_LANG_FLAGS_HPP

#include <cstdint>

namespace flags {
    // default values inside
    enum informationLevel {
        // only general stages, time
        minimal,
        // stages, amounts of files, types, time
        general,
        // full information about the compilation process
        full
    };
    inline uint8_t informationLevel = full;
}

#endif //DODO_LANG_FLAGS_HPP
