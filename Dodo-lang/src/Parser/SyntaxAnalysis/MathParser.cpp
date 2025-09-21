#include "AnalysisInternal.hpp"

bool IsLValue(std::vector <ParserTreeValue>& valueArray, const uint32_t start) {
    for (auto current = &valueArray[start];;) {
        if (current->isLValued) {
            if (current->next != 0) {
                current = &valueArray[current->next];
                continue;
            }
            return true;
        }
        return false;
    }
}

bool IsRightToLeftOrdered(const LexerToken* token) {
    if (token->type == Token::Operator) {
        switch (token->op) {
            case Operator::Increment:
            case Operator::Decrement:
            case Operator::Not:
            case Operator::BinNot:
            case Operator::Assign:
            case Operator::Address:
            case Operator::Dereference:
            case Operator::Cast:
                return true;
            default:
                break;
        }
    }
    return false;
}

// pass operators only, cheers
bool IsSplittableOperator(const LexerToken* token) {
    switch (token->op) {
        case Operator::Brace:
        case Operator::BraceOpen:
        case Operator::BraceClose:
        case Operator::Bracket:
        case Operator::BracketOpen:
        case Operator::BracketClose:
        case Operator::Index:
        case Operator::IndexOpen:
        case Operator::IndexClose:
        case Operator::Macro:
        case Operator::Address:
        case Operator::Dereference:
            return false;
        default:
            return true;
    }
}

uint32_t FindFirstClosed(const Operator::Type groupType, const std::pair<uint32_t, uint32_t>& range, const std::vector <LexerToken*>& tokens, uint32_t startLevel = 0) {
    for (uint32_t n = range.first; n < range.second; n++) {
        // opening
        if (tokens[n]->MatchOperator(static_cast <Operator::Type>(groupType - 2))) {
            startLevel++;
        }
        else if (tokens[n]->MatchOperator(static_cast <Operator::Type>(groupType - 1))) {
            if (startLevel == 0) {
                ParserError("Invalid bracket/brace/index sequence!");
            }
            if (startLevel == 1) {
                return n;
            }
            startLevel--;
        }
    }
    ParserError("Could not find closing bracket/brace/index!");
    return 0;
}

// takes a range where arguments should be and returns the number the first one is at
// assumes there is at least one argument
uint16_t ParserArgumentsStep(std::vector <ParserTreeValue>& valueArray, std::pair<uint32_t, uint32_t> range,
    std::vector <LexerToken*>& tokens, ParserOperation::Type operationType = ParserOperation::Argument);

ParserTreeValue ParseExpressionCast(std::vector <ParserTreeValue>& valueArray, std::pair<uint32_t, uint32_t> range, std::vector <LexerToken*>& tokens) {
    auto [start, end] = range;
    if (tokens[start]->type != Token::Identifier) ParserError("Expected a type identifier!");
    ParserTreeValue out;
    out.operation = ParserOperation::TypeIdentifier;
    out.identifier = tokens[start]->text;
    for (uint32_t n = start + 1; n < end; n++) {
        if (tokens[n]->MatchOperator(Operator::Multiply, Operator::Dereference)) out.typeMeta.pointerLevel++;
        else if (tokens[n]->MatchOperator(Operator::Address)) {
            if (n != end - 1) ParserError("Reference operator must be the last!");
            out.typeMeta.isReference = true;
        }
        else ParserError("Invalid type modifier!");
    }
    valueArray.push_back(out);
    return out;
}

ParserTreeValue ParseExpressionStep(std::vector <ParserTreeValue>& valueArray, std::pair<uint32_t, uint32_t> range,
    std::vector <LexerToken*>& tokens, ParserOperation::Type previousOperation = ParserOperation::None, const LexerToken* auxToken = nullptr) {

    auto [start, end] = range;

    uint32_t length = end - start;
    if (length == 0) {
        ParserError("Internal bug: Empty expression step!");
    }

    LexerToken* mostImportant = nullptr;
    uint32_t firstChosen = 0, lastChosen = 0;
    uint16_t bracketLevel = 0, braceLevel = 0, indexLevel = 0;

    // searching through the array to find the element with the highest split priority
    for (uint32_t n = start; n < end; n++) {
        if (tokens[n]->type == Token::Operator) {
            switch (tokens[n]->op) {
                case Operator::BraceOpen:
                    braceLevel++;
                continue;
                case Operator::BracketOpen:
                    bracketLevel++;
                continue;
                case Operator::IndexOpen:
                    indexLevel++;
                continue;
                case Operator::BraceClose:
                    if (braceLevel == 0) {
                        ParserError("Invalid braces!");
                    }
                braceLevel--;
                continue;
                case Operator::BracketClose:
                    if (bracketLevel == 0) {
                        ParserError("Invalid brackets!");
                    }
                bracketLevel--;
                continue;
                case Operator::IndexClose:
                    if (indexLevel == 0) {
                        ParserError("Invalid index brackets!");
                    }
                indexLevel--;
                continue;
                default:
                    break;
            }
        }
        if (braceLevel == 0 and bracketLevel == 0 and indexLevel == 0) {
            if (tokens[n]->type == Token::Operator and IsSplittableOperator(tokens[n])
            and (mostImportant == nullptr or mostImportant->op < tokens[n]->op)) {

                mostImportant = tokens[n];
                firstChosen = lastChosen = n;
            }
            else if (mostImportant != nullptr and *tokens[n] == *mostImportant) {
                lastChosen = n;
            }
        }
    }

    ParserTreeValue out;

    // now we have found (or not) the most important splitting operand and can construct something out of it
    if (mostImportant != nullptr and mostImportant->op != Operator::Address and mostImportant->op != Operator::Dereference
        and not tokens[end - 1]->MatchOperator(Operator::Increment, Operator::Decrement)
        and not tokens[start]->MatchOperator(Operator::Increment, Operator::Decrement)) {
        if (length < 3) {
            ParserError("Invalid expression!");
        }

        out.operation = ParserOperation::Operator;
        out.code = mostImportant->op;



        if (IsRightToLeftOrdered(mostImportant)) {
            valueArray.push_back(ParseExpressionStep(valueArray, {start, lastChosen}, tokens));
            out.left = valueArray.size() - 1;
            if (out.code == Operator::Cast) valueArray.push_back(ParseExpressionCast(valueArray, {lastChosen + 1, end}, tokens));
            else valueArray.push_back(ParseExpressionStep(valueArray, {lastChosen + 1, end}, tokens));
            out.right = valueArray.size() - 1;
        }
        else {
            valueArray.push_back(ParseExpressionStep(valueArray, {start, firstChosen}, tokens));
            out.left = valueArray.size() - 1;
            valueArray.push_back(ParseExpressionStep(valueArray, {firstChosen + 1, end}, tokens));
            out.right = valueArray.size() - 1;
        }
    }
    else {
        // that one is more complicated
        if (length == 1) {
            // that should be the simplest case, right?
            if (tokens[start]->type == Token::Identifier) {
                // a variable
                out.operation = ParserOperation::Variable;
                out.identifier = tokens[start]->text;
                out.isLValued = true;
            }
            else if (tokens[start]->type == Token::String) {
                // a string literal
                out.operation = ParserOperation::String;
                out.identifier = tokens[start]->text;
                ReEscapeCharacters(tokens[start]->text);
                AddString(tokens[start]->text);
            }
            else if (tokens[start]->type == Token::Number) {
                // a constant number
                out.operation = ParserOperation::Literal;
                out.literal = tokens[start];
            }
            else if (tokens[start]->type == Token::Keyword) {
                ParserError("Are keywords valid in expressions? Null maybe?");
            }
            else {
                ParserError("Invalid single token remained in expression!");
            }
        }
        else if (tokens[start]->MatchOperator(Operator::BracketOpen)) {
            auto closing = FindFirstClosed(Operator::Bracket, {start + 1, end}, tokens, 1);
            // bracket
            if (previousOperation == ParserOperation::Variable or previousOperation == ParserOperation::Member and auxToken != nullptr) {
                // function call
                out.operation = ParserOperation::Call;
                out.identifier = auxToken->text;
                // now parsing arguments...
                if (length >2) {
                    out.argument = ParserArgumentsStep(valueArray, {start + 1, closing}, tokens);
                }
                out.isLValued = true;
            }
            else if (auxToken != nullptr and previousOperation == ParserOperation::None) {
                // probably syscall
                if (not auxToken->MatchKeyword(Keyword::Syscall)) {
                    ParserError("Internal bug: Called syscall from non syscall expression!");
                }
                if (length < 3) {
                    ParserError("Syscall MUST have a number!");
                }
                out.operation = ParserOperation::Syscall;
                out.argument = ParserArgumentsStep(valueArray, {start + 1, closing}, tokens);
                if (valueArray[valueArray[out.argument].left].operation != ParserOperation::Literal or not valueArray[valueArray[out.argument].left].literal->MatchNumber(Type::unsignedInteger)) {
                    ParserError("First argument of syscall needs to be an unsigned constant!");
                }
                out.code = valueArray[valueArray[out.argument].left].literal->_unsigned;
            }
            else {
                // just a normal bracket operation
                out.operation = ParserOperation::Group;
                out.code = Operator::Bracket;
                valueArray.push_back(ParseExpressionStep(valueArray, {start + 1, closing}, tokens));
                out.value = valueArray.size() - 1;
                out.isLValued = true;
            }
            // there's something after it
            if (closing != end - 1) {
                valueArray.push_back(ParseExpressionStep(valueArray, {closing + 1, end}, tokens));
                out.next = valueArray.size() - 1;
            }
        }
        else if (tokens[start]->MatchOperator(Operator::BraceOpen)) {
            auto closing = FindFirstClosed(Operator::Brace, {start + 1, end}, tokens, 1);
            // brace
            out.operation = ParserOperation::Group;
            out.code = Operator::Brace;
            out.value = ParserArgumentsStep(valueArray, {start + 1, closing}, tokens, ParserOperation::ArrayElement);
            out.isLValued = true;
            if (closing != end - 1) {
                valueArray.push_back(ParseExpressionStep(valueArray, {closing + 1, end}, tokens));
                out.next = valueArray.size() - 1;
            }
        }
        else if (tokens[start]->MatchOperator(Operator::IndexOpen)) {
            auto closing = FindFirstClosed(Operator::Index, {start + 1, end}, tokens, 1);
            // index
            out.operation = ParserOperation::Group;
            out.code = Operator::Index;
            valueArray.push_back(ParseExpressionStep(valueArray, {start + 1, closing}, tokens));
            out.value = valueArray.size() - 1;
            out.isLValued = true;
            if (closing != end - 1) {
                valueArray.push_back(ParseExpressionStep(valueArray, {closing + 1, end}, tokens));
                out.next = valueArray.size() - 1;
            }
        }
        else if (tokens[start]->type == Token::Identifier and not tokens[end - 1]->MatchOperator(Operator::Increment, Operator::Decrement)) {
            // a variable with members or call
            out.operation = ParserOperation::Variable;
            out.identifier = tokens[start]->text;
            if (auto result = ParseExpressionStep(valueArray, {start + 1, end}, tokens, ParserOperation::Variable, tokens[start]); result.operation == ParserOperation::Call) {
                out = result;
            }
            else {
                valueArray.push_back(result);
                out.next = valueArray.size() - 1;
                out.isLValued = true;
            }
            out.next = valueArray.size() - 1;
            out.isLValued = true;

        }
        else if (tokens[start]->MatchKeyword(Keyword::Dot) and tokens[start + 1]->type == Token::Identifier) {
            out.operation = ParserOperation::Member;
            out.identifier = tokens[start + 1]->text;
            if (length > 2) {
                if (auto result = ParseExpressionStep(valueArray, {start + 2, end}, tokens, ParserOperation::Member, tokens[start + 1]); result.operation == ParserOperation::Call) {
                    out = result;
                }
                else {
                    valueArray.push_back(result);
                    out.next = valueArray.size() - 1;
                    out.isLValued = true;
                }
            }
            else {
                out.isLValued = true;
            }
        }
        else if (tokens[start]->MatchKeyword(Keyword::Syscall)) {
            if (length < 4) {
                ParserError("Invalid syscall!");
            }
            out = ParseExpressionStep(valueArray, {start + 1, end}, tokens, ParserOperation::None, tokens[start]);
        }
        else if (tokens[start]->MatchOperator(Operator::Dereference, Operator::Address, Operator::Increment, Operator::Decrement, Operator::Not, Operator::BinNot)) {
            out.operation = ParserOperation::SingleOperator;
            out.operatorType = tokens[start]->op;
            valueArray.push_back(ParseExpressionStep(valueArray, {start + 1, end}, tokens));
            out.prefix = valueArray.size() - 1;
            // TODO: lvalue at this point is probably  useless
            out.isLValued = true;
        }
        else if (tokens[end - 1]->MatchOperator(Operator::Increment, Operator::Decrement)) {
            out.operation = ParserOperation::SingleOperator;
            out.operatorType = tokens[end - 1]->op;
            valueArray.push_back(ParseExpressionStep(valueArray, {start, end - 1}, tokens));
            out.postfix = valueArray.size() - 1;
            // TODO: lvalue at this point is probably  useless
            out.isLValued = true;
        }
        else
            ParserError("Unsupported expression step!");
    }

    if (auxToken != nullptr and auxToken->MatchKeyword(Keyword::Syscall) and out.operation != ParserOperation::Syscall) {
        ParserError("Expected a syscall after syscall keyword!");
    }
    
    return out;
}

uint16_t ParserArgumentsStep(std::vector <ParserTreeValue>& valueArray, std::pair<uint32_t, uint32_t> range,
    std::vector <LexerToken*>& tokens, ParserOperation::Type operationType) {

    auto [start, end] = range;
    uint16_t bracketLevel = 0, braceLevel = 0, indexLevel = 0;
    uint16_t firstIndex = 0, lastIndex = 0, startIndex = start;

    // searching through the array to find the commas and parsing arguments in between
    for (uint32_t n = start; n < end; n++) {
        if (tokens[n]->type == Token::Operator) {
            switch (tokens[n]->op) {
                case Operator::BraceOpen:
                    braceLevel++;
                continue;
                case Operator::BracketOpen:
                    bracketLevel++;
                continue;
                case Operator::IndexOpen:
                    indexLevel++;
                continue;
                case Operator::BraceClose:
                    if (braceLevel == 0) {
                        ParserError("Invalid braces!");
                    }
                braceLevel--;
                //continue;
                break;
                case Operator::BracketClose:
                    if (bracketLevel == 0) {
                        ParserError("Invalid brackets!");
                    }
                bracketLevel--;
                //continue;
                break;
                case Operator::IndexClose:
                    if (indexLevel == 0) {
                        ParserError("Invalid index brackets!");
                    }
                indexLevel--;
                //continue;
                break;
                default:
                    break;
            }
        }
        if (braceLevel == 0 and bracketLevel == 0 and indexLevel == 0 and (tokens[n]->MatchKeyword(Keyword::Comma) or (n == end - 1 and n++))) {
            ParserTreeValue out;

            out.operation = operationType;
            out.identifier = tokens[start]->text;
            valueArray.push_back(ParseExpressionStep(valueArray, {startIndex, n}, tokens));
            startIndex = n + 1;
            out.left = valueArray.size() - 1;
            if (firstIndex == 0) {
                lastIndex = firstIndex = valueArray.size();
            }
            else {
                valueArray[lastIndex].argument = valueArray.size();
                lastIndex = valueArray.size();
            }
            valueArray.push_back(out);
        }
    }

    // now the argument that does not have a comma
    //ParserTreeValue out;

    //out.operation = ParserOperation::Argument;
    //out.identifier = tokens[start]->text;
    //valueArray.push_back(ParseExpressionStep(valueArray, {startIndex, end}, tokens));
    //out.value = valueArray.size() - 1;
    //out.isLValued = true;
    //if (firstIndex != 0) {
    //    valueArray[lastIndex].next = valueArray.size();
    //}
    //valueArray.push_back(out);


    return firstIndex;
}

bool IsExpressionEndToken(const LexerToken* token) {
    switch (token->type) {
        case Token::Keyword:
            switch (token->kw) {
                case Keyword::End:
                case Keyword::Comma:
                    return true;
                default:
                    return false;
            }
        case Token::Operator:
            switch (token->op) {
                case Operator::BraceClose:
                case Operator::BracketClose:
                case Operator::IndexClose:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

LexerToken* ParseExpression(Generator <LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <LexerToken*> tokens) {
    // first of all let's
    LexerToken* last = nullptr;
    int32_t bracketLevel = 0, braceLevel = 0, indexLevel = 0;
    if (tokens.empty()) {
        tokens.push_back(generator());
    }
    for (uint32_t n = 0; n < tokens.size(); n++) {

        if (tokens[n]->type == Token::Operator) {
            switch (tokens[n]->op) {
                case Operator::BraceOpen:
                    braceLevel++;
                break;
                case Operator::BracketOpen:
                    bracketLevel++;
                break;
                case Operator::IndexOpen:
                    indexLevel++;
                break;
                case Operator::BraceClose:
                    braceLevel--;
                    break;
                case Operator::BracketClose:
                    bracketLevel--;
                break;
                case Operator::IndexClose:
                    indexLevel--;
                break;
                default:
                    break;
            }
        }

        if ((bracketLevel == 0 and braceLevel == 0 and indexLevel == 0 and IsExpressionEndToken(tokens[n])
                and not tokens[n]->MatchOperator(Operator::BraceClose, Operator::BracketClose, Operator::IndexClose))
            or braceLevel == -1 or bracketLevel == -1 or indexLevel == -1) {
            if (n != tokens.size() - 1) {
                ParserError("Invalid expression!");
            }
            last = tokens.back();
            tokens.pop_back();
        }

        if (n == tokens.size() - 1) {
            tokens.push_back(generator());
        }
    }

    if (tokens.empty()) {
        ParserError("Empty expression!");
    }

    // finding dereference operators
    for (int32_t n = 0; n < tokens.size(); n++) {
        if (tokens[n]->MatchOperator(Operator::Multiply)) {
            if ((n >= 1 and tokens[n - 1]->type == Token::Operator and tokens.size() > n - 2) or n == 0) {
                tokens[n]->op = Operator::Dereference;
            }
        }
    }

    // detecting if it's a declaration, if it is then some structure must be built
    if (tokens.size() >= 3 and tokens[0]->MatchKeyword(Keyword::Let)) {
        uint32_t first = 0;

        if (tokens[1]->MatchKeyword(Keyword::Mut)) {
            first = 2;
        }
        else {
            first = 1;
        }

        if (tokens[first]->type != Token::Identifier) {
            ParserError("Expected an identifier after declaration!");
        }

        uint8_t pointerLevel = 0;
        bool isReference = false;
        uint32_t name = 0;
        for (uint32_t n = first + 1; n < tokens.size(); n++) {
            if (tokens[n]->MatchOperator(Operator::Multiply, Operator::Dereference)) {
                if (isReference) ParserError("Cannot make a pointer to reference!");
                pointerLevel++;
            }
            else if (tokens[n]->type == Token::Identifier) {
                name = n;
                break;
            }
            else if (tokens[n]->MatchOperator(Operator::Address)) {
                if (isReference) ParserError("Cannot create a double reference!");
                isReference = true;
            }
            else {
                break;
            }
        }

        if (name == 0) {
            ParserError("Expected name identifier after type!");
        }

        valueArray.reserve(valueArray.size() + tokens.size());

        const uint32_t startIndex = valueArray.size();
        ParserTreeValue declaration;
        declaration.operation = ParserOperation::Definition;
        declaration.typeMeta.pointerLevel = pointerLevel;
        declaration.typeMeta.isMutable = first == 2;
        declaration.typeMeta.isReference = isReference;
        declaration.identifier = tokens[first]->text;
        declaration.next = startIndex + 1;
        valueArray.emplace_back(declaration);

        ParserTreeValue var;
        var.operation = ParserOperation::Variable;
        var.isLValued = true;
        var.identifier = tokens[name]->text;
        valueArray.emplace_back(var);

        if (name != tokens.size() - 1) {
            if (not tokens[name + 1]->MatchOperator(Operator::Assign)) {
                ParserError("Invalid use of newly declared variable!");
            }
            valueArray[startIndex].value = valueArray.size();
            valueArray.emplace_back();
            // TODO: is this fragment needed?
            valueArray[valueArray[startIndex].value] = ParseExpressionStep(valueArray, {name + 2, tokens.size()}, tokens);
        }
        valueArray.shrink_to_fit();

        return last;
    }

    // now the entire expression bounds are done, and it can be parsed into a tree and compressed to size
    valueArray.reserve(valueArray.size() + tokens.size());
    valueArray.emplace_back();
    uint32_t backIndex = valueArray.size() - 1;
    valueArray[backIndex] = ParseExpressionStep(valueArray, {0, tokens.size()}, tokens);
    valueArray.shrink_to_fit();

    return last;
}