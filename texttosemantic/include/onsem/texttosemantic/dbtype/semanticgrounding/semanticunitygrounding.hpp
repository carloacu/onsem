#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP

#include <list>
#include <string>
#include "../../api.hpp"
#include <onsem/texttosemantic/dbtype/misc/typeofunity.hpp>
#include "semanticgrounding.hpp"
#include "semanticanglegrounding.hpp"
#include "semanticlengthgrounding.hpp"
#include "semanticdurationgrounding.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API SemanticUnityGrounding : public SemanticGrounding {
    SemanticUnityGrounding(SemanticAngleUnity pAngleUnity)
        : SemanticGrounding(SemanticGroundingType::UNITY)
        , typeOfUnity(TypeOfUnity::ANGLE)
        , value(semanticAngleUnity_toChar(pAngleUnity)) {}
    SemanticUnityGrounding(SemanticLengthUnity pLengthUnity)
        : SemanticGrounding(SemanticGroundingType::UNITY)
        , typeOfUnity(TypeOfUnity::LENGTH)
        , value(semanticLengthUnity_toChar(pLengthUnity)) {}
    SemanticUnityGrounding(SemanticTimeUnity pTimeUnity)
        : SemanticGrounding(SemanticGroundingType::UNITY)
        , typeOfUnity(TypeOfUnity::TIME)
        , value(semanticTimeUnity_toChar(pTimeUnity)) {}
    SemanticUnityGrounding(TypeOfUnity pTypeOfUnity, const std::string& pValueStr)
        : SemanticGrounding(SemanticGroundingType::UNITY)
        , typeOfUnity(TypeOfUnity::TIME)
        , value(0) {
        setValue(pTypeOfUnity, pValueStr);
    }

    const SemanticUnityGrounding& getUnityGrounding() const override { return *this; }
    SemanticUnityGrounding& getUnityGrounding() override { return *this; }
    const SemanticUnityGrounding* getUnityGroundingPtr() const override { return this; }
    SemanticUnityGrounding* getUnityGroundingPtr() override { return this; }

    bool operator==(const SemanticUnityGrounding& pOther) const;
    bool isEqual(const SemanticUnityGrounding& pOther) const;

    void setValue(SemanticAngleUnity pAngleUnity);
    void setValue(SemanticLengthUnity pLengthUnity);
    void setValue(SemanticTimeUnity pTimeUnity);
    void setValue(TypeOfUnity pTypeOfUnity, const std::string& pValueStr);

    SemanticAngleUnity getAngleUnity() const;
    SemanticLengthUnity getLengthUnity() const;
    SemanticTimeUnity getTimeUnity() const;
    std::string getValueStr() const;
    std::string getValueConcept() const;

    TypeOfUnity typeOfUnity;
    unsigned char value;
};

inline bool SemanticUnityGrounding::operator==(const SemanticUnityGrounding& pOther) const {
    return this->isEqual(pOther);
}

inline bool SemanticUnityGrounding::isEqual(const SemanticUnityGrounding& pOther) const {
    return _isMotherClassEqual(pOther) && typeOfUnity == pOther.typeOfUnity && value == pOther.value;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP
