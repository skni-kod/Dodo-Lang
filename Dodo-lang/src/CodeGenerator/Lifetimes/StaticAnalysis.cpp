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

    std::size_t printIndex = 0;


    for (std::size_t n = 0; n < old.size(); n++) {
        auto& current = old[n];
        auto op1 = current.op1();
        auto op2 = current.op2();
        auto op3 = current.op3();

        auto DestroyUnused = [&context, n]() {
            for (std::size_t d = 0; d < context.temporaries.size(); d++)
                if (context.temporaries[d].isDestructible and context.temporaries[d].lastUse <= n and not context.temporaries[d].meta.isPointer()) {
                    CallDestructor(context, BytecodeOperand(Location::Var, VariableLocation(VariableLocation::Temporary, 0, d), Type::none, 0));
                    context.temporaries[d].isDestructible = false;
                }

            for (std::size_t l = 0; l < context.localVariables.size(); l++)
                for (std::size_t v = 0; v < context.localVariables[l].size(); v++)
                    if (context.localVariables[l][v].isDestructible and context.localVariables[l][v].lastUse <= n and not context.localVariables[l][v].meta.isPointer()) {
                        CallDestructor(context, BytecodeOperand(Location::Var, VariableLocation(VariableLocation::Local, l, v), Type::none, 0));
                        context.localVariables[l][v].isDestructible = false;
                    }
        };

        auto DestroyAll = [&context]() {
            for (std::size_t d = 0; d < context.temporaries.size(); d++)
                if (context.temporaries[d].isDestructible and not context.temporaries[d].meta.isPointer()) {
                    CallDestructor(context, BytecodeOperand(Location::Var, VariableLocation(VariableLocation::Temporary, 0, d), Type::none, 0));
                    context.temporaries[d].isDestructible = false;
                }

            for (std::size_t l = 0; l < context.localVariables.size(); l++)
                for (std::size_t v = 0; v < context.localVariables[l].size(); v++)
                    if (context.localVariables[l][v].isDestructible and not context.localVariables[l][v].meta.isPointer()) {
                        CallDestructor(context, BytecodeOperand(Location::Var, VariableLocation(VariableLocation::Local, l, v), Type::none, 0));
                        context.localVariables[l][v].isDestructible = false;
                    }
        };

        auto IsUnused = [&context](BytecodeOperand op) mutable {
            switch (op.location) {
            case Location::Variable:
                return context.getVariableObject(op).uses == 0;
            default:
                Unimplemented();
            }
        };

        auto AssignValueFull = [&context, &n](BytecodeOperand op, BytecodeOperand value, TypeMeta meta, bool isDestructible) mutable {
            switch (op.location) {
            case Location::Variable: {
                auto& obj = context.getVariableObject(op);

                if (obj.isDestructible and not obj.meta.isPointer())
                    CallDestructor(context, op);

                obj.content = value;
                if (not meta.isPointer() and value.location == Location::Variable and value.value.ui != op.value.ui) {
                    auto& val = context.getVariableObject(value);
                    if (val.lastUse > n)
                        obj.content = CopyConstruct(context, value);
                    else if (val.lastUse <= n)
                        val.isDestructible = false;
                }

                obj.contentMeta = meta;
                obj.isDestructible = isDestructible;


                break;
            }
            default:
                Unimplemented();
            }
        };

        // this one calculates the meta and destructibility itself
        auto AssignValue = [&context, &AssignValueFull](BytecodeOperand op, BytecodeOperand value, int8_t pointerChange = 0, bool assumeDestructible = false) mutable {
            switch (value.location) {
            case Location::Variable: {
                auto& obj = context.getVariableObject(value);
                auto meta = obj.contentMeta;
                meta.pointerLevel += pointerChange;
                // TODO: diagnose why temporary values in print i32 could overwrite content of number
                BytecodeOperand content;
                if (obj.content.location == Location::Unknown or
                    (value.value.variable.type == VariableLocation::Local
                        and obj.content.location == Location::Variable
                        and obj.content.value.variable.type == VariableLocation::Temporary))
                    content = value;
                else
                    content = obj.content;
                AssignValueFull(op, content, meta, obj.isDestructible or assumeDestructible);
                break;
            }
            case Location::Literal:
                //// TODO: should literals be destructible?
                //AssignValueFull(op, value, {}, true);
                //break;

            case Location::String:
                //// TODO: string literals cannot be destroyed, right?
                //AssignValueFull(op, value, {1, false, false}, false);
                //break;

            case Location::Unknown:
                AssignValueFull(op, op, context.getVariableObject(op).meta, true);
                break;

            default:
                Unimplemented();
            }
        };

        auto ReplaceResult = [&old, &n](BytecodeOperand replacee, BytecodeOperand value) {
            if (replacee.location != Location::Variable or replacee.value.variable.type != VariableLocation::Temporary)
                Error("Invalid result to replace!");
            for (std::size_t m = n; m < old.size(); m++) {
                auto& current = old[m];
                auto op1 = current.op1();
                // it's an union so this comparison can be used too
                if (op1.location == Location::Variable and op1.value.ui == replacee.value.ui)
                    current.op1(value);
                auto op2 = current.op2();
                if (op2.location == Location::Variable and op2.value.ui == replacee.value.ui)
                    current.op2(value);
                auto op3 = current.op3();
                if (op3.location == Location::Variable and op3.value.ui == replacee.value.ui)
                    current.op3(value);
            }
        };

        switch (current.type) {

        case Bytecode::Define:
            context.codes.emplace_back(current);
            break;

        case Bytecode::AssignAt: {
            auto obj = context.getVariableObject(op1);
            if (obj.content.location == Location::Variable and obj.contentMeta.pointerLevel == 1) {
                current.type = Bytecode::AssignTo;
                current.op1(obj.content);
                op1 = obj.content;
            }
            else {
                if (IsUnused(op3))
                    current.op3({});
                context.codes.emplace_back(current);
                break;
            }
        }

        case Bytecode::AssignTo:
            // when value is assigned from an argument, it's always assumed to be constructed and has a value of itself
            if (current.op2Location == Location::Argument)
                AssignValueFull(op1, op1, {}, true);
            else
                AssignValue(op1, op2);
            ReplaceResult(op3, op1);
            context.codes.emplace_back(current);
            break;

        case Bytecode::Address:
            AssignValue(op3, op1, +1);
            context.codes.emplace_back(current);
            break;

        case Bytecode::Dereference:
            if (op1.location == Location::Variable) {
                auto obj = context.getVariableObject(op1);
                if (obj.content.location == Location::Variable and obj.contentMeta.pointerLevel == 1) {
                    // since this creates a temporary we can replace it with the actual variable
                    ReplaceResult(op3, obj.content);
                    break;
                }
            }

            context.codes.emplace_back(current);
        break;

        case Bytecode::Return:
            if (op1.location == Location::Variable) {
                auto& obj = context.getVariableObject(op1);
                if (not obj.isDestructible)
                    Error("Cannot return an uninitialised variable!");
                obj.isDestructible = false;
            }
            DestroyAll();

            context.codes.emplace_back(current);
            break;

        case Bytecode::Argument:
            context.codes.emplace_back(current);
            break;

        // TODO: is this one even used?
        case Bytecode::Save:
            if (current.op1Location == Location::Argument)
                AssignValueFull(op3, op3, {}, true);
            else
                AssignValue(op3, op1);
            context.codes.emplace_back(current);
            break;


        case Bytecode::Function:
        case Bytecode::Method:
        case Bytecode::Syscall:
            if (op3.location != Location::Unknown) {
                AssignValue(op3, op3);
            }
            context.codes.emplace_back(current);
            break;

        // TODO: what about brace lists?
        case Bytecode::BraceListElement:

        case Bytecode::If:
        case Bytecode::LoopLabel:
        case Bytecode::Label:
        case Bytecode::Jump:
        case Bytecode::Break:
        case Bytecode::Continue:
            context.codes.emplace_back(current);
            break;

        case Bytecode::ToReference:
        case Bytecode::GetIndexValue:
        case Bytecode::GetIndexAddress:
        case Bytecode::Member:
        case Bytecode::Convert:
        case Bytecode::Power:
        case Bytecode::Multiply:
        case Bytecode::Divide:
        case Bytecode::Modulo:
        case Bytecode::Add:
        case Bytecode::Subtract:
        case Bytecode::ShiftRight:
        case Bytecode::ShiftLeft:
        case Bytecode::NAnd:
        case Bytecode::BinNAnd:
        case Bytecode::And:
        case Bytecode::BinAnd:
        case Bytecode::XOr:
        case Bytecode::BinXOr:
        case Bytecode::NOr:
        case Bytecode::BinNOr:
        case Bytecode::Or:
        case Bytecode::BinOr:
        case Bytecode::NImply:
        case Bytecode::Imply:
        case Bytecode::BinNImply:
        case Bytecode::BinImply:
        case Bytecode::Lesser:
        case Bytecode::Greater:
        case Bytecode::Equals:
        case Bytecode::LesserEqual:
        case Bytecode::GreaterEqual:
        case Bytecode::NotEqual:
        case Bytecode::Not:
        case Bytecode::BinNot:
            AssignValue(op3, op3, 0, true);
            context.codes.emplace_back(current);
            break;

        case Bytecode::BeginScope:
            context.activeLevels.push_back(++levelCounter);
            context.codes.emplace_back(current);
            break;

        case Bytecode::EndScope:
            context.activeLevels.pop_back();
            context.codes.emplace_back(current);
            break;

        case Bytecode::None:
        default:
            Unimplemented();
        }

        DestroyUnused();

        if (Options::informationLevel >= Options::InformationLevel::full)
            for (; printIndex < context.codes.size(); printIndex++)
                std::cout << "INFO L3: (" << std::to_string(printIndex) << "): " << context.codes[printIndex];
    }
}

void RemoveUnusedInstructions(Context& context) {

    if (Options::informationLevel >= Options::InformationLevel::full)
        std::cout << "INFO L3: Removing unused instructions...\n";

    for (int64_t n = context.codes.size() - 1; n >= 0; n--) {
        auto& current = context.codes[n];
        auto op1 = current.op1();
        auto op2 = current.op2();
        auto op3 = current.op3();

        auto RemoveUse = [&context, &n](BytecodeOperand op) mutable {
            if (op.location == Location::Variable) {
                auto& obj = context.getVariableObject(op);
                if (obj.uses > 0)
                    obj.uses--;
            }
        };

        switch (current.type) {

        default:
            break;

        case Bytecode::Address:
        case Bytecode::Convert:
        case Bytecode::Dereference:
        case Bytecode::ToReference:
        case Bytecode::Member:
        case Bytecode::Save:
        case Bytecode::GetIndexValue:
        case Bytecode::GetIndexAddress:
        case Bytecode::Power:
        case Bytecode::Multiply:
        case Bytecode::Divide:
        case Bytecode::Modulo:
        case Bytecode::Add:
        case Bytecode::Subtract:
        case Bytecode::ShiftRight:
        case Bytecode::ShiftLeft:
        case Bytecode::NAnd:
        case Bytecode::BinNAnd:
        case Bytecode::And:
        case Bytecode::BinAnd:
        case Bytecode::XOr:
        case Bytecode::BinXOr:
        case Bytecode::NOr:
        case Bytecode::BinNOr:
        case Bytecode::Or:
        case Bytecode::BinOr:
        case Bytecode::NImply:
        case Bytecode::Imply:
        case Bytecode::BinNImply:
        case Bytecode::BinImply:
        case Bytecode::Lesser:
        case Bytecode::Greater:
        case Bytecode::Equals:
        case Bytecode::LesserEqual:
        case Bytecode::GreaterEqual:
        case Bytecode::NotEqual:
        case Bytecode::Not:
        case Bytecode::BinNot:
            if (op3.location == Location::Variable and context.getVariableObject(op3).uses == 0) {
                RemoveUse(op1);
                RemoveUse(op2);
                context.codes.erase(context.codes.begin() + n);
            }
            break;

            // removing assignments to the same variable
        case Bytecode::AssignTo:
            if (op1.location == op2.location and op1.value.ui == op2.value.ui
                and op1.location == op3.location and op1.value.ui == op3.value.ui)
                context.codes.erase(context.codes.begin() + n);
            break;
        }



    }

    if (Options::informationLevel >= Options::InformationLevel::full) {
        std::cout << "INFO L3: Codes after removal:\n";
        for (std::size_t n = 0; n < context.codes.size(); n++)
            std::cout << "INFO L3: (" << std::to_string(n) << "): " << context.codes[n];
    }
}
