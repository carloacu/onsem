#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICMETAGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICMETAGROUNDING_HPP

#include <memory>
#include "semanticgrounding.hpp"
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API SemanticMetaGrounding : public SemanticGrounding {
    SemanticMetaGrounding(SemanticGroundingType pRefToType, int pIdParam, const std::string& pAttibuteName = "",
                          const std::string& pAttibuteValue = "")
        : SemanticGrounding(SemanticGroundingType::META)
        , refToType(pRefToType)
        , paramId(pIdParam)
        , attibuteName(pAttibuteName)
        , attibuteValue(pAttibuteValue) {}

    const SemanticMetaGrounding& getMetaGrounding() const override { return *this; }
    SemanticMetaGrounding& getMetaGrounding() override { return *this; }
    const SemanticMetaGrounding* getMetaGroundingPtr() const override { return this; }
    SemanticMetaGrounding* getMetaGroundingPtr() override { return this; }

    bool operator==(const SemanticMetaGrounding& pOther) const;
    bool isEqual(const SemanticMetaGrounding& pOther) const;

    static bool parseParameter(std::string& pAttributeName,
                               std::string& pAttributeValue,
                               const std::string& pStr);

    static std::unique_ptr<SemanticMetaGrounding> makeMetaGroundingFromStr(const std::string& pStr);


    static bool isTheBeginOfAParamOld(const std::string& pStr);
    static bool parseParameterOld(int& pParamId,
                                  std::string& pLabel,
                                  std::string& pAttributeName,
                                  const std::string& pStr);
    static std::unique_ptr<SemanticMetaGrounding> makeMetaGroundingFromStrOld(const std::string& pStr);


    static bool groundingTypeFromStr(SemanticGroundingType& pRefToType, const std::string& pStr);

    static const char firstCharOfStrOld;
    static const char firstCharOfStr;
    static const int returnId;

    SemanticGroundingType refToType;
    int paramId;
    std::string attibuteName;
    std::string attibuteValue;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICMETAGROUNDING_HPP
