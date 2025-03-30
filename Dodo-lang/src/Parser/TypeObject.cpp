#include "TypeObject.hpp"

#include <GenerateCode.hpp>
#include <ostream>

std::ostream& operator<<(std::ostream& out, const ParserMemberVariableParameter& variable) {
    return out << "INFO L3: \t" << (variable.typeMeta().isMutable ? "mut " : "")
    << variable.typeObject->typeName
    << (variable.typeMeta().pointerLevel < 4 ? std::string(variable.typeMeta().pointerLevel, '*') : "*(" + std::to_string(variable.typeMeta().pointerLevel) + ")")
    << " " << variable.name() << "\n";
}

std::ostream& operator<<(std::ostream& out, const TypeObject& type) {
    if (type.isPrimitive) {
        out << "INFO L3: Primitive type " << type.typeName << ", representing a";
        switch (type.primitiveType) {
            case Primitive::floatingPoint:
                out << " floating point value sized ";
            break;
            case Primitive::unsignedInteger:
                out << "n unsigned integer value sized ";
            break;
            case Primitive::signedInteger:
                out << "n integer value sized ";
            break;
            default:
                break;
        }
        return out << type.typeSize << " bytes\n";
    }
    out << "INFO L3: Type " << type.typeName << " containing members:\n";
    for (auto& n : type.members) {
        out << n;
    }
    return out << "INFO L3: It's total size is: " << type.typeSize << " byte(s), aligned to: " << type.typeAlignment << " byte(s)\n";
}

const TypeMeta& ParserMemberVariableParameter::typeMeta() const {
    return definition[0].typeMeta;
}


const std::string& ParserMemberVariableParameter::name() const {
    return *definition[1].identifier;
}

const std::string& ParserMemberVariableParameter::typeName() const {
    return *definition[0].identifier;
}

TypeMeta::TypeMeta(const TypeMeta& old, const int8_t amountToChange) {
    if (amountToChange < 0 and old.pointerLevel < -amountToChange) CodeGeneratorError("Cannot get put address into a non pointer!");
    pointerLevel = old.pointerLevel + amountToChange;
    isMutable = old.isMutable;
}
