#ifndef LIFETIME_HPP
#define LIFETIME_HPP

#include "Bytecode.hpp"

// calculates lifetime information for all local, global and temporary variables
void AnalyseAndOptimize(Context& context);

#endif //LIFETIME_HPP
