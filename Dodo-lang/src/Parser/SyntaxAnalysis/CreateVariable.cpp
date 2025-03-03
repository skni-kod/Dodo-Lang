#include "AnalysisInternal.hpp"

void UpdateGlobalVariables() {
    for (auto& n : globalVariablesOLD.map) {
        if (not parserTypes.isKey(n.second.typeOrName)) {
            ParserError("Invalid typename \"" + n.second.typeOrName + "\" in global variable named \"" + n.first + "\"!");
        }
        auto& type = parserTypes[n.second.typeOrName];
        n.second.type.type = type.type;
        n.second.type.size = type.size;
        n.second.typeOrName = VariableType(type).getPrefix() + n.first;
    }
}

