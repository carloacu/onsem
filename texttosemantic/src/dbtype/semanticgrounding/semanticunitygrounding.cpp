#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <assert.h>
#include <iostream>

namespace onsem {

void SemanticUnityGrounding::setValue(SemanticAngleUnity pAngleUnity) {
    typeOfUnity = TypeOfUnity::ANGLE;
    value = semanticAngleUnity_toChar(pAngleUnity);
}

void SemanticUnityGrounding::setValue(SemanticLengthUnity pLengthUnity) {
    typeOfUnity = TypeOfUnity::LENGTH;
    value = semanticLengthUnity_toChar(pLengthUnity);
}

void SemanticUnityGrounding::setValue(SemanticTimeUnity pTimeUnity) {
    typeOfUnity = TypeOfUnity::TIME;
    value = semanticTimeUnity_toChar(pTimeUnity);
}

void SemanticUnityGrounding::setValue(TypeOfUnity pTypeOfUnity, const std::string& pValueStr) {
    switch (pTypeOfUnity) {
        case TypeOfUnity::ANGLE: setValue(semanticAngleUnity_fromStr(pValueStr)); break;
        case TypeOfUnity::LENGTH: setValue(semanticLengthUnity_fromStr(pValueStr)); break;
        case TypeOfUnity::PERCENTAGE: typeOfUnity = pTypeOfUnity; break;
        case TypeOfUnity::TIME: setValue(semanticTimeUnity_fromStr(pValueStr)); break;
    }
}

SemanticAngleUnity SemanticUnityGrounding::getAngleUnity() const {
    if (typeOfUnity != TypeOfUnity::ANGLE) {
        assert(false);
        std::cerr << "getAngleUnity() called with a wrong unity" << std::endl;
        return SemanticAngleUnity::DEGREE;
    }
    return semanticAngleUnity_fromChar(value);
}

SemanticLengthUnity SemanticUnityGrounding::getLengthUnity() const {
    if (typeOfUnity != TypeOfUnity::LENGTH) {
        assert(false);
        std::cerr << "getLengthUnity() called with a wrong unity" << std::endl;
        return SemanticLengthUnity::CENTIMETER;
    }
    return semanticLengthUnity_fromChar(value);
}

SemanticTimeUnity SemanticUnityGrounding::getTimeUnity() const {
    if (typeOfUnity != TypeOfUnity::TIME) {
        assert(false);
        std::cerr << "getTimeUnity() called with a wrong unity" << std::endl;
        return SemanticTimeUnity::DAY;
    }
    return semanticTimeUnity_fromChar(value);
}

std::string SemanticUnityGrounding::getValueStr() const {
    switch (typeOfUnity) {
        case TypeOfUnity::ANGLE: return semanticAngleUnity_toStr(semanticAngleUnity_fromChar(value));
        case TypeOfUnity::LENGTH: return semanticLengthUnity_toStr(semanticLengthUnity_fromChar(value));
        case TypeOfUnity::PERCENTAGE: return "percentage";
        case TypeOfUnity::TIME: return semanticTimeUnity_toStr(semanticTimeUnity_fromChar(value));
    }
    assert(false);
    return "";
}

std::string SemanticUnityGrounding::getValueConcept() const {
    switch (typeOfUnity) {
        case TypeOfUnity::ANGLE: return semanticAngleUnity_toConcept(semanticAngleUnity_fromChar(value));
        case TypeOfUnity::LENGTH: return semanticLengthUnity_toConcept(semanticLengthUnity_fromChar(value));
        case TypeOfUnity::PERCENTAGE: return "percentage";
        case TypeOfUnity::TIME: return semanticTimeUnity_toConcept(semanticTimeUnity_fromChar(value));
    }
    assert(false);
    return "";
}

}    // End of namespace onsem
