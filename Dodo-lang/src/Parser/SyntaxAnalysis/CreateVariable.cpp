#include "AnalysisInternal.hpp"

void CheckGlobalVariables() {
    for (auto& n : globalVariables) {
        if (not types.contains(n.second.typeName())) {
            ParserError("Invalid typename \"" + n.second.typeName() + "\" in global variable named \"" + n.first + "\"!");
        }
        n.second.typeObject = &types[n.second.typeName()];
    }
}

