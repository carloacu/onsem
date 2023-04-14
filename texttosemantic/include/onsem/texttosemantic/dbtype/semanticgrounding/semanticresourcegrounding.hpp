#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRESOURCEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICRESOURCEGROUNDING_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "semanticgrounding.hpp"
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include "../../api.hpp"
#include "../semanticexpression/semanticexpression.hpp"

namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API SemanticResource
{
  SemanticResource(const std::string& pLabel,
                   SemanticLanguageEnum pLanguage,
                   const std::string& pValue)
    : label(pLabel),
      language(pLanguage),
      value(pValue),
      parameterLabelsToQuestions()
  {
  }

  SemanticResource(const SemanticResource& pOther)
    : label(pOther.label),
      language(pOther.language),
      value(pOther.value),
      parameterLabelsToQuestions()
  {
    for (const auto& currParam : pOther.parameterLabelsToQuestions)
    {
      auto& newParam = parameterLabelsToQuestions[currParam.first];
      for (const auto& currSemExp : currParam.second)
        newParam.push_back(currSemExp->clone());
    }
  }

  bool operator==(const SemanticResource& pOther) const
  { return label == pOther.label && language == pOther.language && value == pOther.value &&
        parameterLabelsToQuestions == pOther.parameterLabelsToQuestions; }

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
  std::map<std::string, std::vector<UniqueSemanticExpression>> parameterLabelsToQuestions;
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticResourceGrounding : public SemanticGrounding
{
  SemanticResourceGrounding(const std::string& pLabel,
                            SemanticLanguageEnum pLanguage,
                            const std::string& pValue)
    : SemanticGrounding(SemanticGroundingType::RESOURCE),
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
