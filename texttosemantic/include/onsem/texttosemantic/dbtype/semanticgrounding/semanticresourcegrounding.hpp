#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRESOURCEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRESOURCEGROUNDING_HPP

#include <list>
#include <memory>
#include "semanticgrouding.hpp"
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include "../../api.hpp"

namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API SemanticResource
{
  SemanticResource(const std::string& pLabel,
                   SemanticLanguageEnum pLanguage,
                   const std::string& pValue)
    : label(pLabel),
      language(pLanguage),
      value(pValue)
  {
  }

  SemanticResource(const SemanticResource& pOther)
    : label(pOther.label),
      language(pOther.language),
      value(pOther.value)
  {
  }

  bool operator==(const SemanticResource& pOther) const
  { return value == pOther.value && label == pOther.label && language == pOther.language; }

  bool operator<(const SemanticResource& pOther) const
  {
    if (value != pOther.value)
      return value < pOther.value;
    if (label != pOther.label)
      return label < pOther.label;
    return language < pOther.language;
  }

  std::string toStr() const
  {
    if (language != SemanticLanguageEnum::UNKNOWN)
      return label + "=#" + semanticLanguageEnum_toStr(language) + "#" + value;
    return label + "=" + value;
  }

  std::string toRadixMapStr() const
  {
    return value + "<" + label + semanticLanguageEnum_toStr(language);
  }

  std::string label;
  SemanticLanguageEnum language;
  std::string value;
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticResourceGrounding : public SemanticGrounding
{
  SemanticResourceGrounding(const std::string& pLabel,
                            SemanticLanguageEnum pLanguage,
                            const std::string& pValue)
    : SemanticGrounding(SemanticGroudingType::RESOURCE),
      resource(pLabel, pLanguage, pValue)
  {
    concepts["resource_*"] = 4;
  }

  SemanticResourceGrounding(const SemanticResourceGrounding& pOther)
    : SemanticGrounding(pOther),
      resource(pOther.resource)
  {
  }

  const SemanticResourceGrounding& getResourceGrounding() const override { return *this; }
  SemanticResourceGrounding& getResourceGrounding() override { return *this; }
  const SemanticResourceGrounding* getResourceGroundingPtr() const override { return this; }
  SemanticResourceGrounding* getResourceGroundingPtr() override { return this; }

  bool operator==(const SemanticResourceGrounding& pOther) const { return this->isEqual(pOther); }
  bool isEqual(const SemanticResourceGrounding& pOther) const
  { return _isMotherClassEqual(pOther) && resource == pOther.resource; }

  SemanticResource resource;
};


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRESOURCEGROUNDING_HPP
