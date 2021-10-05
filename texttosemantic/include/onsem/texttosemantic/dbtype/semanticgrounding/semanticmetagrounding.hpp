#ifndef SEMEXPMODEL_SEMANTICMETAGROUNDING_H
#define SEMEXPMODEL_SEMANTICMETAGROUNDING_H

#include <memory>
#include "semanticgrouding.hpp"
#include "../../api.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticMetaGrounding : public SemanticGrounding
{
  SemanticMetaGrounding(SemanticGroudingType pRefToType,
                        int pIdParam,
                        const std::string& pAttibuteName = "")
    : SemanticGrounding(SemanticGroudingType::META),
      refToType(pRefToType),
      paramId(pIdParam),
      attibuteName(pAttibuteName)
  {
  }

  const SemanticMetaGrounding& getMetaGrounding() const override { return *this; }
  SemanticMetaGrounding& getMetaGrounding() override { return *this; }
  const SemanticMetaGrounding* getMetaGroundingPtr() const override { return this; }
  SemanticMetaGrounding* getMetaGroundingPtr() override { return this; }

  bool operator==(const SemanticMetaGrounding& pOther) const;
  bool isEqual(const SemanticMetaGrounding& pOther) const;

  static bool isTheBeginOfAParam(const std::string& pStr);

  static bool parseParameter(int& pParamId,
                             std::string& pLabel,
                             std::string& pAttributeName,
                             const std::string& pStr);

  static std::unique_ptr<SemanticMetaGrounding> makeMetaGroundingFromStr
  (const std::string& pStr);

  static bool groundingTypeFromStr(SemanticGroudingType& pRefToType,
                                  const std::string& pStr);

  static const char firstCharOfStr;
  static const int returnId;

  SemanticGroudingType refToType;
  int paramId;
  std::string attibuteName;
};



} // End of namespace onsem


#endif // SEMEXPMODEL_SEMANTICMETAGROUNDING_H
