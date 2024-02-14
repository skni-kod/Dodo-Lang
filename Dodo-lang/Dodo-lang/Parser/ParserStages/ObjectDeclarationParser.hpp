#ifndef DODO_LANG_OBJECT_PARSER_HPP
#define DODO_LANG_OBJECT_PARSER_HPP

#include "../../lexicalToken.h"
#include "../Generator.tpp"

void ParseObjectDeclarations(Generator<const LexicalToken*>& generator);

#endif //DODO_LANG_OBJECT_PARSER_HPP