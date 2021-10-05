#ifndef LINGUISTICANALYZER_ALTAGGERTYPES_H
#define LINGUISTICANALYZER_ALTAGGERTYPES_H

#include <functional>
#include <list>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>


namespace onsem
{
namespace linguistics
{


enum class FinderConstraint
{
  HAS,
  FIRST_ELT,
  ONLY_ONE_ELT
};


enum class CompatibilityCheck
{
  IS_COMPATIBLE,
  ISNT_COMPATIBLE,
  DONT_CARE
};

enum class LinkedValue
{
  LINKED,
  NOT_LINKED,
  UNKNOWN
};


enum class ActionIfLinked
{
  DEL_ALL_OTHERS,
  DEL_ALL_EXPECT_AUX,
  DEL_LESSER_PROBA,
  DEL_THIS_POSSIBILITY,
  PUT_ON_TOP,
  PUT_ON_BOTTOM,
  NOTHING
};

enum class ActionIfNotLinked
{
  REMOVE,
  NOTHING
};

/// Allows rules to be skipped if YES.
enum class CanBeEmpty
{
  YES,
  NO
};

/// Allows to retry rules that had matched if YES.
enum class CanHaveMany
{
  YES,
  NO
};

/// A simplified visit function specification, using a custom matcher or the kind of part of speech.
struct TaggerTokenCheck
{
  TaggerTokenCheck() = default;

  TaggerTokenCheck(PartOfSpeech pPartOfSpeech,
                   FinderConstraint pGramCheckerStrategy = FinderConstraint::HAS,
                   CompatibilityCheck pCheckCompatibilityWithNeighborhood = CompatibilityCheck::DONT_CARE,
                   ActionIfLinked pActionIfLinked = ActionIfLinked::NOTHING,
                   ActionIfNotLinked pActionIfNotLinked = ActionIfNotLinked::NOTHING,
                   LinkedValue pLinkedValue = LinkedValue::LINKED);

  TaggerTokenCheck(std::function<bool (const InflectedWord&)> pIGramMatcher,
                   FinderConstraint pGramCheckerStrategy = FinderConstraint::HAS,
                   CompatibilityCheck pCheckCompatibilityWithNeighborhood = CompatibilityCheck::DONT_CARE,
                   ActionIfLinked pActionIfLinked = ActionIfLinked::NOTHING,
                   ActionIfNotLinked pActionIfNotLinked = ActionIfNotLinked::NOTHING,
                   LinkedValue pLinkedValue = LinkedValue::LINKED);

  std::function<bool (const InflectedWord&)> iGramMatcher;
  ActionIfLinked actionIfLinked{ActionIfLinked::NOTHING};
  ActionIfNotLinked actionIfNotLinked{ActionIfNotLinked::NOTHING};
  FinderConstraint gramCheckerStrategy{FinderConstraint::HAS};
  CompatibilityCheck checkCompatibilityWithNeighborhood{CompatibilityCheck::DONT_CARE};

  /// Link value to set if linked.
  LinkedValue linkedValue;
};


struct TaggerListOfTokenChecks
{
  TaggerListOfTokenChecks(CanBeEmpty pCanBeEmpty = CanBeEmpty::NO,
                          CanHaveMany pCanHaveMany = CanHaveMany::NO,
                          int pPriority = 10,
                          LinkedValue pDefaultLinkValue = LinkedValue::NOT_LINKED);

  bool canBeAPunctuation() const;

  std::list<TaggerTokenCheck> elts;
  CanBeEmpty canBeEmpty;
  CanHaveMany canHaveMany;
  int priority;
  LinkedValue defaultLinkValue;
};


struct AIGramContext
{
  std::list<TaggerListOfTokenChecks> before{};
  std::list<TaggerListOfTokenChecks> after{};
};


struct TaggerPattern
{
  TaggerPattern(TaggerTokenCheck&& pRootGram)
    : rootGram(pRootGram),
      possibilities()
  {
  }

  TaggerTokenCheck rootGram;
  std::list<AIGramContext> possibilities;
};



} // End of namespace linguistics
} // End of namespace onsem


#endif // !LINGUISTICANALYZER_ALTAGGERTYPES_H
