#include "taggertypes.hpp"


namespace onsem
{
namespace linguistics
{


TaggerTokenCheck::TaggerTokenCheck
(PartOfSpeech pPartOfSpeech,
 FinderConstraint pDealOnlyWithTheMostProbableGram,
 CompatibilityCheck pCheckCompatibilityWithNeighborhood,
 ActionIfLinked pActionIfLinked,
 ActionIfNotLinked pActionIfNotLinked,
 LinkedValue pLinkedValue)
  : iGramMatcher([pPartOfSpeech](const InflectedWord& pIGram) -> bool { return pIGram.word.partOfSpeech == pPartOfSpeech; }),
    actionIfLinked(pActionIfLinked),
    actionIfNotLinked(pActionIfNotLinked),
    gramCheckerStrategy(pDealOnlyWithTheMostProbableGram),
    checkCompatibilityWithNeighborhood(pCheckCompatibilityWithNeighborhood),
    linkedValue(pLinkedValue)
{
}

TaggerTokenCheck::TaggerTokenCheck
(std::function<bool (const InflectedWord&)> pIGramMatcher,
 FinderConstraint pDealOnlyWithTheMostProbableGram,
 CompatibilityCheck pCheckCompatibilityWithNeighborhood,
 ActionIfLinked pActionIfLinked,
 ActionIfNotLinked pActionIfNotLinked,
 LinkedValue pLinkedValue)
  : iGramMatcher(pIGramMatcher),
    actionIfLinked(pActionIfLinked),
    actionIfNotLinked(pActionIfNotLinked),
    gramCheckerStrategy(pDealOnlyWithTheMostProbableGram),
    checkCompatibilityWithNeighborhood(pCheckCompatibilityWithNeighborhood),
    linkedValue(pLinkedValue)
{
}


TaggerListOfTokenChecks::TaggerListOfTokenChecks
(CanBeEmpty pCanBeEmpty,
 CanHaveMany pCanHaveMany,
 int pPriority,
 LinkedValue pDefaultLinkValue)
  : elts(),
    canBeEmpty(pCanBeEmpty),
    canHaveMany(pCanHaveMany),
    priority(pPriority),
    defaultLinkValue(pDefaultLinkValue)
{
}

bool TaggerListOfTokenChecks::canBeAPunctuation() const
{
  static const InflectedWord punctuation
      (PartOfSpeech::PUNCTUATION,
       mystd::make_unique<EmptyInflections>());

  for (const auto& currElt : elts)
    if (currElt.iGramMatcher(punctuation))
      return true;
  return false;
}


} // End of namespace linguistics
} // End of namespace onsem
