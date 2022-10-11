#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLANGUAGEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLANGUAGEGROUNDING_HPP

#include "semanticgrounding.hpp"
#include <string>
#include <map>
#include <set>
#include <list>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include "../../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API SemanticLanguageGrounding : public SemanticGrounding
{
  SemanticLanguageGrounding
  (SemanticLanguageEnum pLanguage)
    : SemanticGrounding(SemanticGroundingType::LANGUAGE),
      language(pLanguage)
  {
  }

  const SemanticLanguageGrounding& getLanguageGrounding() const override { return *this; }
  SemanticLanguageGrounding& getLanguageGrounding() override { return *this; }
  const SemanticLanguageGrounding* getLanguageGroundingPtr() const override { return this; }
  SemanticLanguageGrounding* getLanguageGroundingPtr() override { return this; }

  bool operator==(const SemanticLanguageGrounding& pOther) const;
  bool isEqual(const SemanticLanguageGrounding& pOther) const;

  SemanticLanguageEnum language;
};






inline bool SemanticLanguageGrounding::operator==(const SemanticLanguageGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticLanguageGrounding::isEqual(const SemanticLanguageGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      language == pOther.language;
}


inline static void getAllLanguageTypes
(std::list<SemanticLanguageEnum>& pLanguageTypes)
{
  for (const auto& currLang : semanticLanguageEnum_allValues)
    if (currLang != SemanticLanguageEnum::UNKNOWN)
      pLanguageTypes.emplace_back(currLang);
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLANGUAGEGROUNDING_HPP
