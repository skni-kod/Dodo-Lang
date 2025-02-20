#include "TypeObject.hpp"
#include <ostream>

std::ostream& operator<<(std::ostream& out, const TypeObjectMember& member) {
    return out << "INFO L3: \t" << member.memberType->typeName << " " << member.memberName << "\n";
}

std::ostream& operator<<(std::ostream& out, const TypeObject& type) {
    if (type.isPrimitive) {
        out << "INFO L3: Primitive type " << type.typeName << ", representing a";
        switch (type.primitiveType) {
            case Primitive::floatingPoint:
                out << " floating point value sized ";
            break;
            case Primitive::unsignedInteger:
                out << "n unsigned integer sized ";
            break;
            case Primitive::signedInteger:
                out << "n integer sized ";
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