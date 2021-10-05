#ifndef ALLINGDBMEANINGTOWORDS_H
#define ALLINGDBMEANINGTOWORDS_H

#include <vector>
#include <map>
#include <stddef.h>


namespace onsem
{
class ALLingdbMeaning;
class LinguisticIntermediaryDatabase;
class ALLingdbDynamicTrieNode;
class ALLingdbFlexions;
struct VerbConjugaison;
struct NounAdjConjugaison;

class ALLingdbMeaningToWords
{
public:

  void findWordsConjugaisons
  (std::map<const ALLingdbMeaning*, VerbConjugaison>& pWordConjugaison,
   std::map<const ALLingdbMeaning*, NounAdjConjugaison>& pNounConjugaison,
   const LinguisticIntermediaryDatabase& pLingDatabase) const;

};

} // End of namespace onsem


#endif // ALLINGDBMEANINGTOWORDS_H
