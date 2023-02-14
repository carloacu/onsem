#ifndef ADDCOMPOSEDWORDS_H
#define ADDCOMPOSEDWORDS_H

#include <set>
#include <QDomElement>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include "metawiki/patternrecognizer.hpp"


namespace onsem
{

struct Wikitionary_ComposedWord
{
  explicit Wikitionary_ComposedWord
  (PartOfSpeech pNewGram)
    : newGram(pNewGram),
      rootSubMeaning(nullptr),
      subMeanings()
  {
  }

  explicit Wikitionary_ComposedWord
  (const Wikitionary_ComposedWord& pOther)
    : newGram(pOther.newGram),
      rootSubMeaning(pOther.rootSubMeaning),
      subMeanings(pOther.subMeanings)
  {
  }


  bool operator<(const Wikitionary_ComposedWord& pOther) const
  {
    if (rootSubMeaning != pOther.rootSubMeaning)
    {
      return rootSubMeaning < pOther.rootSubMeaning;
    }
    if (newGram != pOther.newGram)
    {
      return newGram < pOther.newGram;
    }
    if (subMeanings.size() != pOther.subMeanings.size())
    {
      return subMeanings.size() < pOther.subMeanings.size();
    }

    for (auto it = subMeanings.begin(), itOther = pOther.subMeanings.begin();
         it != subMeanings.end(); ++it, ++itOther)
    {
      if (it->first != itOther->first)
      {
        return it->first < itOther->first;
      }
      if (it->second != itOther->second)
      {
        return it->second < itOther->second;
      }
    }
    return false;
  }

  PartOfSpeech newGram;
  LingdbMeaning* rootSubMeaning;
  std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection>> subMeanings;

private:
  Wikitionary_ComposedWord& operator=
  (const Wikitionary_ComposedWord& pOther);
};



class AddComposedWords
{
public:
  explicit AddComposedWords
  (const PatternRecognizer& pPatternRecognizer);

  void extractDatasFromFile
  (std::set<Wikitionary_ComposedWord>& pNewComposedWords,
   std::ifstream& pWikionaryFile,
   const LinguisticIntermediaryDatabase& pInLingDatabase,
   SemanticLanguageEnum pLangEnum) const;

  static void writeNewComposedWords
  (const std::set<Wikitionary_ComposedWord>& pNewComposedWords,
   const std::string& poutFile);


private:
  const PatternRecognizer& fPatternReco;


  static void xFillMeaningAttriabutes
  (QDomElement& pMeaningElt,
   LingdbMeaning* pMeaning);

  static void xTryToAddPronominalFrenchVerb
  (std::set<Wikitionary_ComposedWord>& pNewComposedWords,
   const LinguisticIntermediaryDatabase& pInLingDatabase,
   const std::string& pRootVerb);

};

} // End of namespace onsem



#endif // ADDCOMPOSEDWORDS_H
