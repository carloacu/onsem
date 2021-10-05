#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_SEMANTICWORD_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_SEMANTICWORD_HPP

#include <string>
#include <map>
#include <assert.h>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include "../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API SemanticWord
{
  SemanticWord() = default;

  SemanticWord(SemanticLanguageEnum pLanguage,
               const std::string& pLemma,
               PartOfSpeech pPartOfSpeech)
    : language(pLanguage),
      lemma(pLemma),
      partOfSpeech(pPartOfSpeech)
  {
  }

  void setContent(SemanticLanguageEnum pLanguage,
                  const std::string& pLemma,
                  PartOfSpeech pPartOfSpeech)
  {
    language = pLanguage;
    lemma = pLemma;
    partOfSpeech = pPartOfSpeech;
  }

  void clear()
  {
    language = SemanticLanguageEnum::UNKNOWN;
    lemma.clear();
    partOfSpeech = PartOfSpeech::UNKNOWN;
  }

  bool isEmpty() const
  {
    return language == SemanticLanguageEnum::UNKNOWN &&
        lemma.empty() &&
        partOfSpeech == PartOfSpeech::UNKNOWN;
  }

  bool isReflexive() const
  {
    return language == SemanticLanguageEnum::FRENCH &&
        partOfSpeech == PartOfSpeech::VERB &&
        lemma.size() > 3 &&
        lemma.compare(lemma.size() - 3, 3, "~se") == 0;
  }

  // ex: "laver~se" -> "laver"
  // ! isReflexive() should return true, otherwise the result of this function is undefined
  SemanticWord getRootFormFromReflexive() const
  {
    assert(lemma.size() > 3);
    return SemanticWord(language, lemma.substr(0, lemma.size() - 3), partOfSpeech);
  }

  bool operator<(const SemanticWord& pOther) const;
  bool operator==(const SemanticWord& pOther) const;
  bool operator!=(const SemanticWord& pOther) const;

  SemanticLanguageEnum language{SemanticLanguageEnum::UNKNOWN};
  std::string lemma{};
  PartOfSpeech partOfSpeech{PartOfSpeech::UNKNOWN};
};


inline bool SemanticWord::operator<
(const SemanticWord& pOther) const
{
  if (lemma != pOther.lemma)
    return lemma < pOther.lemma;
  if (partOfSpeech != pOther.partOfSpeech)
    return partOfSpeech < pOther.partOfSpeech;
  return language < pOther.language;
}

inline bool SemanticWord::operator==
(const SemanticWord& pOther) const
{
  return lemma == pOther.lemma &&
      partOfSpeech == pOther.partOfSpeech &&
      language == pOther.language;
}

inline bool SemanticWord::operator!=
(const SemanticWord& pOther) const
{
  return !(*this == pOther);
}


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_SEMANTICWORD_HPP
