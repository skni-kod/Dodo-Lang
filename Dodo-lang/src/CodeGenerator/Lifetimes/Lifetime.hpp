#ifndef LIFETIME_HPP
#define LIFETIME_HPP

#include "Bytecode.hpp"

// calculates lifetime information for all local, global and temporary variables
void CalculateLifetimes(Context& context);

#endif //LIFETIME_HPP
