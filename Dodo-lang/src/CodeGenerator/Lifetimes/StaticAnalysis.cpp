#include "StaticAnalysis.hpp"

#include <iostream>

#include "BytecodeInternal.hpp"
#include "ErrorHandling.hpp"

void RunStaticAnalysis(Context& context) {

    // first off let's get the old codes out
    std::vector old{std::move(context.codes)};
    context.codes = {};
    context.codes.reserve(old.size());
    context.activeLevels = {0};
    uint64_t levelCounter = 0;

    if (Options::informationLevel >= Options::InformationLevel::full)
        std::cout << "INFO L3: Running static analysis...\nINFO L3: Bytecodes after processing:\n";

    auto CallDestructors = [] (Context& context, std::size_t index, bool careAboutExpiration = true) {
        for (std::size_t n = 0; n < context.temporaries.size(); n++) {
            auto& current = context.temporaries[n];
            if (not current.meta.isPointer() and (not careAboutExpiration or current.lastUse == index) and current.isDestructible) {
                current.isDestructible = false;
                CallDestructor(context, {Location::var, {VariableLocation(VariableLocation::Type::Temporary, 0, n)}, current.content.literalType, current.content.size}, false);
            }
        }
        for (std::size_t l = 0; l < context.localVariables.size(); l++)
            for (std::size_t n = 0; n < context.localVariables[l].size(); n++) {
                auto& current = context.localVariables[l][n];
                if (not current.meta.isPointer() and (not careAboutExpiration or current.lastUse == index) and current.isDestructible) {
                    current.isDestructible = false;
                    CallDestructor(context, {Location::var, {VariableLocation(VariableLocation::Type::Local, l, n)}, current.content.literalType, current.content.size}, false);
                }
            }
    };

    // TODO: add complete content tracking

    std::size_t printIndex = 0;
    std::size_t destructorIndex = 0;
    Bytecode* firstArgument = nullptr;
    std::vector<Bytecode> argumentsToSet{};

    for (std::size_t n = 0; n < old.size(); n++) {
        bool skipDestructing = false;
        bool dontAddOldCode = false;

        auto& current = old[n];
        auto op1 = current.op1();
        auto op2 = current.op2();
        auto op3 = current.op3();

        switch (current.type) {

        case Bytecode::BeginScope:
            context.activeLevels.push_back(++levelCounter);
            break;

        case Bytecode::EndScope:
            context.activeLevels.pop_back();
            break;

        case Bytecode::Define: {
            // TODO: add current callable to context
            // any actual value is assigned only when using arguments, in other cases it just reserves space
            if (current.op3Location == Location::Argument) {
                auto& obj = context.getVariableObject(current.op1());

                // non-pointers are assumed to be normal initialised objects and need to be destroyed later
                if (not current.opMeta.isPointer())
                    obj.isDestructible = true;

                auto& content = context.getVariableObject(op1);
                obj.content = content.content;
                obj.contentMeta = {};
            }
            break;
        }

        case Bytecode::Argument:
            skipDestructing = true;
            dontAddOldCode = true;

            if (op3.location == Location::var) {
                // if we pass the variable by value we skip its destruction later
                // if it is still used later we need to copy it via a copy constructor, if there is none throw error
                auto& arg = context.getVariableObject(op3);
                if (not arg.meta.isPointer()) {
                    if (arg.lastUse > n and not arg.type->defaultRAII) {
                        dontAddOldCode = true;

                        // first off reserve the space for the new variable
                        auto temp = context.insertTemporary(arg.type, arg.meta);

                        Bytecode code{};
                        code.type = Bytecode::Define;
                        code.op1(temp);
                        code.AssignType(arg.type, arg.meta);
                        context.codes.push_back(code);

                        // now construct it via the copy constructor
                        bool found = false;
                        for (auto& method : arg.type->methods)
                            if (method.isConstructor
                                and method.parameters.size() == 1
                                and method.parameters[0].typeObject == arg.type
                                and method.parameters[0].typeMeta() == TypeMeta(0, false, true)) {
                                // we found a copy constructor
                                found = true;

                                std::vector<ParserTreeValue> values{};
                                std::vector<TypeInfo> arguments = {{arg.type, {0, false, true}}};
                                code = {};
                                ParserTreeValue value;
                                AddCallIfMatches(context, &method, values, value, arguments, code, GetAddress(context, temp, {arg.type, TypeMeta(0, false, true)}), false, GetAddress(context, op3, {arg.type, TypeMeta(0, false, true)}));
                                context.codes.push_back(code);
                                }
                        if (not found)
                            Error("Type: " + arg.type->typeName + " does not contain a valid copy constructor!");

                        // and now finally use it as an argument
                        code = current;
                        code.op3(temp);

                        argumentsToSet.push_back(code);
                    }
                    else
                        argumentsToSet.push_back(current);

                    DebugError(not arg.type->defaultRAII and not arg.isDestructible, "Argument uninitialized, needs to be changed when content is tracked");
                    arg.isDestructible = false;
                }
                else
                    argumentsToSet.push_back(current);
            }
            else
                argumentsToSet.push_back(current);

            if (current.op1Value.ui == 1)
                firstArgument = &current;
            break;

        case Bytecode::Syscall:
        case Bytecode::Function:
            skipDestructing = true;
            for (auto& arg : argumentsToSet)
                context.codes.push_back(arg);
            argumentsToSet.clear();
            break;


        case Bytecode::AssignTo: {
            auto& target = context.getVariableObject(current.op1());

            if (not target.meta.pointerLevel and not target.meta.isReference) {
                if (target.isDestructible)
                    CallDestructor(context, current.op1());
                target.isDestructible = true;
            }

            // if the assignment target is a pointer then there's no concern about RAII
            if (op2.location == Location::Variable){
                auto& source = context.getVariableObject(op2);

                if (not target.meta.pointerLevel and not target.meta.isReference) {
                    if (source.lastUse > n)
                        Error("Unimplemented value copy assignment!");
                    source.isDestructible = false;
                }


                target.content = source.content;
                target.contentMeta = source.contentMeta;


            }
            else {
                target.content = op2;
                target.contentMeta = {};
            }

            if (current.op3Location == Location::var) {
                auto& result = context.getVariableObject(current.op3());
                if (result.lastUse > n) {
                    result.content = target.content;
                    result.contentMeta = target.contentMeta;
                }
                else current.op3({});
            }

            break;
        }

        // TODO: make this one in any way functional
        case Bytecode::AssignAt: {
            auto& target = context.getVariableObject(current.op1());

            if (target.meta.pointerLevel + target.meta.isReference == 1) {
                // in cases of assigning a value we need to call the destructor, but only if it's not a null pointer...
                // TODO: add null pointer check

            }

            // now we need to assign the content at the target to be that of source
            if (op2.location == Location::var) {
                auto& source = context.getVariableObject(op2);
                target.content = source.content;
            }
            else {
                target.content = op2;
            }


            break;
        }

        case Bytecode::Return: {
            if (op1.location == Location::var) {
                auto& obj = context.getVariableObject(op1);
                if (obj.isDestructible)
                    obj.isDestructible = false;
            }
            // now we need to destruct every destructible variable
            CallDestructors(context, n, false);
            break;
        }

        case Bytecode::Method: {
            skipDestructing = true;
            if (op1.value.function->isConstructor) {
                auto& obj = context.getVariableObject(firstArgument->op3());
                if (obj.isDestructible)
                    CallDestructor(context, op2);
                obj.isDestructible = true;

                // TODO: set the content of the argument pointer to be destructible or something
            }
            for (auto& arg : argumentsToSet)
                context.codes.push_back(arg);
            argumentsToSet.clear();

            break;
        }

        default:
            break;
        }

        while (destructorIndex < n and not skipDestructing) {
            destructorIndex++;
            // checking things that end their lifetimes at the given index
            CallDestructors(context, destructorIndex);
        }

        if (not dontAddOldCode)
            context.codes.emplace_back(current);

        if (Options::informationLevel >= Options::InformationLevel::full)
            for (; printIndex < context.codes.size(); printIndex++)
                std::cout << "INFO L3: (" << std::to_string(printIndex) << "): " << context.codes[printIndex];
    }

    // TODO: add return at end check when current callable is added in context
    //if (context.codes.empty() or context.codes.back().type != Bytecode::Return) {
    //    Bytecode code{};
    //    code.type = Bytecode::Return;
    //    context.codes.push_back(code);
    //}

    if (Options::informationLevel >= Options::InformationLevel::full)
        std::cout << "INFO L3: Static analysis complete!\nINFO L3: Processing to assembly:\n";

}
