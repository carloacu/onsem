#ifndef ONSEM_COMPILERMODEL_LINGDBMEANINGTOWORDS_HPP
#define ONSEM_COMPILERMODEL_LINGDBMEANINGTOWORDS_HPP

#include <vector>
#include <map>
#include <stddef.h>


namespace onsem
{
class LingdbMeaning;
class LinguisticIntermediaryDatabase;
class LingdbDynamicTrieNode;
class LingdbFlexions;
struct VerbConjugaison;
struct NounAdjConjugaison;

class LingdbMeaningToWords
{
public:

  void findWordsConjugaisons
  (std::map<const LingdbMeaning*, VerbConjugaison>& pWordConjugaison,
   std::map<const LingdbMeaning*, NounAdjConjugaison>& pNounConjugaison,
   const LinguisticIntermediaryDatabase& pLingDatabase) const;

};

} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_LINGDBMEANINGTOWORDS_HPP
