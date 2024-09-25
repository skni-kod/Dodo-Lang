#include "TheGenerator.hpp"
#include "GenerateCode.hpp"

uint8_t GetOperandType(const std::string& operand) {
    if (operand.front() == '%') {
        return Operator::reg;
    }
    if (operand.front() == '-' or (operand.front() >= '0' and operand.front() <= '9')) {
        return Operator::imm;
    }
    if (operand.front() == 'i' or operand.front() == 'u' or operand.front() == 'f') {
        return Operator::var;
    }
    CodeGeneratorError("Invalid operator passed to instruction generator!");
    return Operator::none;
}

std::vector<uint8_t> GetOperandTypes(const InstructionRequirements& req) {
    std::vector<uint8_t> types;

    if (req.op1.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op1));
    if (req.op2.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op2));
    if (req.op3.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op3));
    if (req.op4.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op4));

    return types;

}

const OpCombination& ChooseOpCombination(const InstructionRequirements& req, const std::vector<uint8_t>& operands) {
    std::vector <const OpCombination*> validOnes;

    for (auto& n : req.combinations) {
        // check everything about the possibilities to only leave the ones usable in this instance

        // first the amount of operands
        uint8_t operandAmount = 0;
        if (n.type1 != Operator::none) {
            if (n.type2 != Operator::none) {
                if (n.type3 != Operator::none) {
                    if (n.type4 != Operator::none) {
                        operandAmount = 4;
                    }
                    else {
                        operandAmount = 3;
                    }
                }
                else {
                    operandAmount = 2;
                }
            }
            else {
                operandAmount = 1;
            }
        }

        if (operands.size() != operandAmount) {
            continue;
        }

        // when size is confirmed types need to be checked
        // TODO: add check between every type of operand
    }
    return {};
}

void InsertValue(std::string target, std::string source) {

}

void GenerateInstruction(InstructionRequirements req) {


    const OpCombination& combination = ChooseOpCombination(req);


}