#ifndef DODO_LANG_OBJECT_PARSER_HPP
#define DODO_LANG_OBJECT_PARSER_HPP

#include "../../LexicalToken.hpp"
#include "../Generator.tpp"

void ParseObjectDeclarations(Generator<const LexicalToken*>& generator);
void ParseObjectMethodMemberDefinitions(Generator<const LexicalToken*>& generator);

#endif //DODO_LANG_OBJECT_PARSER_HPP