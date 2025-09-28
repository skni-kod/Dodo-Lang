#include "TypeObject.hpp"

#include <GenerateCode.hpp>
#include <ostream>

#include "ErrorHandling.hpp"

TypeInfo::TypeInfo(uint8_t pointerLevel, bool isMutable, bool isReference, TypeObject* type) {
    this->pointerLevel = pointerLevel;
    this->isMutable = isMutable;
    this->isReference = isReference;
    this->type = type;
}

TypeInfo::TypeInfo(TypeMeta meta, TypeObject* type) : TypeMeta(meta) {
    this->type = type;
}

TypeInfo::TypeInfo(TypeObject* type, TypeMeta meta) : TypeInfo(meta, type) {}

TypeInfo::TypeInfo(const TypeInfo& old, int8_t pointerLevelDifference) {
    *this = TypeInfo(TypeMeta(old, pointerLevelDifference), old.type);
}

TypeMeta TypeInfo::meta() const {
    return static_cast<TypeMeta>(*this);
}

TypeInfo TypeInfo::noReference() const {
    TypeInfo t = *this;
    t.isReference = false;
    return t;
}
TypeInfo TypeInfo::reference() const {
    TypeInfo t = *this;
    t.isReference = true;
    return t;
}

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
            case Type::floatingPoint:
                out << " floating point value sized ";
            break;
            case Type::unsignedInteger:
                out << "n unsigned integer value sized ";
            break;
            case Type::signedInteger:
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


std::string& ParserMemberVariableParameter::name() const {
    return *definition[1].identifier;
}

std::string& ParserMemberVariableParameter::typeName() const {
    return *definition[0].identifier;
}

TypeMeta::TypeMeta(const uint8_t pointerLevel, const bool isMutable, const bool isReference) : pointerLevel(pointerLevel), isMutable(isMutable), isReference(isReference) {}

TypeMeta::TypeMeta(const TypeMeta& old, const int8_t pointerLevelDifference) {
    DebugError(pointerLevelDifference != -1 and pointerLevelDifference != +1, "Invalid pointer level difference!");

    if (pointerLevelDifference == -1 and old.pointerLevel == 0) {
        Error("Cannot dereference a value");
    }
    if (pointerLevelDifference == +1 and old.pointerLevel == 0x7F) {
        Error("Maximum pointer level reached!");
    }
    *this = TypeMeta(old.pointerLevel + pointerLevelDifference, old.isMutable, old.isReference);
}

TypeMeta TypeMeta::noReference() const {
    TypeMeta t = *this;
    t.isReference = false;
    return t;
}
TypeMeta TypeMeta::reference() const {
    TypeMeta t = *this;
    t.isReference = true;
    return t;
}

char Type::FirstCharacter(TypeEnum type) {
    switch (type) {
    case unsignedInteger: return 'u';
    case signedInteger: return 'i';
    case floatingPoint: return 'f';
    case address: return 'u';
    }
    Unimplemented();
}

uint8_t TypeMeta::variableSize(TypeObject& type) const {
    if (isReference or pointerLevel != 0 or not type.isPrimitive) return Options::addressSize;
    return type.typeSize;
}

bool TypeMeta::operator==(const TypeMeta& other) const {
    if (other.isReference != isReference) return false;
    if (other.pointerLevel != pointerLevel) return false;
    // TODO: what about mutability
    return true;
}

std::string ParserFunctionMethod::getFullName() const {
    std::string name;

    if (isConstructor) name = "constructor$";
    else if (isDestructor) name = "destructor$";
    else if (returnType.typeName != nullptr) name = *returnType.typeName;
    else name = "void";
    name += "$";
    name += (returnType.type.isMutable ? "m" : "") + std::string(returnType.type.pointerLevel, 'p') + (returnType.type.isReference ? "r" : "");
    if (this->name == nullptr) {
        // operator overload
        // TODO: change this to verbose instead of number
        name += "..operator_" + std::to_string(static_cast <uint64_t>(this->overloaded));
    }
    else {
        // normal function
        name += "." + *this->name;
    }
    for (auto& n : parameters) {
        name += "." + n.typeName() + "$";
        name += (n.typeMeta().isMutable ? "m" : "") + std::string(n.typeMeta().pointerLevel, 'p') + (n.typeMeta().isReference ? "r" : "");
    }
    return std::move(name);
}
