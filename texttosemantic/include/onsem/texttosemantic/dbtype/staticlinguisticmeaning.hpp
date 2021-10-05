#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_STATICLINGUISTICMEANING_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_STATICLINGUISTICMEANING_HPP

#include <onsem/common/linguisticmeaning_nomeaningid.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include "../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API StaticLinguisticMeaning
{
  StaticLinguisticMeaning(SemanticLanguageEnum pLanguage,
                          int pMeaningId);
  StaticLinguisticMeaning();

  void clear();
  bool isEmpty() const;

  bool operator==(const StaticLinguisticMeaning& pOther) const;
  bool operator<(const StaticLinguisticMeaning& pOther) const;

  SemanticLanguageEnum language;
  int32_t meaningId;
};





inline StaticLinguisticMeaning::StaticLinguisticMeaning
(SemanticLanguageEnum pLanguage,
 int pMeaningId)
  : language(pLanguage),
    meaningId(pMeaningId)
{
}

inline StaticLinguisticMeaning::StaticLinguisticMeaning()
  : language(SemanticLanguageEnum::UNKNOWN),
    meaningId(LinguisticMeaning_noMeaningId)
{
}

inline void StaticLinguisticMeaning::clear()
{
  language = SemanticLanguageEnum::UNKNOWN;
  meaningId = LinguisticMeaning_noMeaningId;
}

inline bool StaticLinguisticMeaning::isEmpty() const
{
  return meaningId == LinguisticMeaning_noMeaningId;
}


inline bool StaticLinguisticMeaning::operator==
(const StaticLinguisticMeaning& pOther) const
{
  return meaningId == pOther.meaningId &&
      language == pOther.language;
}

inline bool StaticLinguisticMeaning::operator<
(const StaticLinguisticMeaning& pOther) const
{
  if (language != pOther.language)
    return language < pOther.language;
  return meaningId < pOther.meaningId;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_STATICLINGUISTICMEANING_HPP
