#ifndef ONSEM_COMMON_ENUM_LINGUISTICCONDITION_HPP
#define ONSEM_COMMON_ENUM_LINGUISTICCONDITION_HPP

#include <string>
#include <map>
#include <assert.h>

namespace onsem
{


#define ONSEM_LINGUISTICCONDITION_TABLE                                                  \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYBEGINOFCONCEPT, "followedByBeginOfConcept")          \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYCONCEPTORHYPONYM, "followedByConceptOrHyponym")      \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYDATE, "followedByDate")                              \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYDEFINITENOUN, "followedByDefiniteNoun")              \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYHOUR, "followedByHour")                              \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYINFINITIVEVERB, "followedByInfinitiveVerb")          \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYOCCURRENCERANK, "followedByOccurenceRank")           \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYPARTOFSPEECH, "followedByPartOfSpeech")              \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYRESOURCE, "followedByResource")                      \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYSUBORDINATE, "followedBySubordinate")                \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYUNDEFINEDREF, "followedByUndefinedRef")              \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYQUANTITYEVERYTHING, "followedByQuantityEverything")  \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYVOWEL, "followedByVowel")                            \
  ADD_LINGUISTICCONDITION(FOLLOWEDBYONEOFPREFIXES, "followedByOneOfPrefixes")            \
  ADD_LINGUISTICCONDITION(LOCATIONEN_FR, "locationEn_fr")


#define ADD_LINGUISTICCONDITION(a, b) a,
enum class LinguisticCondition
{
  ONSEM_LINGUISTICCONDITION_TABLE
};
#undef ADD_LINGUISTICCONDITION



#define ADD_LINGUISTICCONDITION(a, b) {LinguisticCondition::a, b},
static const std::map<LinguisticCondition, std::string> _linguisticCondition_toStr = {
  ONSEM_LINGUISTICCONDITION_TABLE
};
#undef ADD_LINGUISTICCONDITION


#define ADD_LINGUISTICCONDITION(a, b) {b, LinguisticCondition::a},
static const std::map<std::string, LinguisticCondition> _linguisticCondition_fromStr = {
  ONSEM_LINGUISTICCONDITION_TABLE
};
#undef ADD_LINGUISTICCONDITION
#undef ONSEM_LINGUISTICCONDITION_TABLE


static inline std::string linguisticCondition_toStr(LinguisticCondition pVal)
{
  return _linguisticCondition_toStr.find(pVal)->second;
}


static inline bool linguisticCondition_fromStr(LinguisticCondition& pCondition,
                                               const std::string& pStr)
{
  auto it = _linguisticCondition_fromStr.find(pStr);
  if (it != _linguisticCondition_fromStr.end())
  {
    pCondition = it->second;
    return true;
  }
  assert(false);
  return false;
}


} // End of namespace onsem

#endif // ONSEM_COMMON_ENUM_LINGUISTICCONDITION_HPP
