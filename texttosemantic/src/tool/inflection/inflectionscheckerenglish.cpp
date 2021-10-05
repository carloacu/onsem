#include "inflectionscheckerenglish.hpp"
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>


namespace onsem
{
namespace linguistics
{

InflectionsCheckerEnglish::InflectionsCheckerEnglish(const LinguisticDictionary& pLingDic)
  : InflectionsCheckerVirtual(pLingDic)
{
}


bool InflectionsCheckerEnglish::isAuxVerbCompatibles
(const InflectedWord& pIGramAux,
 const InflectedWord& pIGramVerb) const
{
  const VerbalInflections* auxInfls = pIGramAux.inflections().getVerbalIPtr();
  const VerbalInflections* verbInfls = pIGramVerb.inflections().getVerbalIPtr();

  if (pIGramAux.word == _lingDic.getBeAux().word)
  {
    if (auxInfls != nullptr)
    {
      for (const auto& currAuxInfl : auxInfls->inflections)
      {
        if (currAuxInfl.tense == LinguisticVerbTense::IMPERFECT_INDICATIVE)
        {
          if (verbInfls != nullptr)
          {
            for (const auto& currVerbInfl : verbInfls->inflections)
            {
              if (currVerbInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE ||
                  (currVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE &&
                   relativePersonsAreWeaklyEqual(currAuxInfl.person, currVerbInfl.person)))
              {
                return true;
              }
            }
          }
          return false;
        }
        else if (currAuxInfl.tense == LinguisticVerbTense::INFINITIVE)
        {
          for (const auto& currVerbInfl : verbInfls->inflections)
          {
            if (currVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE &&
                relativePersonsAreWeaklyEqual(currAuxInfl.person, currVerbInfl.person))
            {
              return true;
            }
          }
          return false;
        }
        else if (currAuxInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE)
        {
          return false;
        }
      }
    }
    return InflectionsChecker::verbIsAtPastParticiple(pIGramVerb) ||
        InflectionsChecker::verbIsAtPresentParticiple(pIGramVerb);
  }
  else if (pIGramAux.word == _lingDic.getHaveAux().word)
  {
    return InflectionsChecker::verbIsAtPastParticiple(pIGramVerb);
  }

  if (verbInfls != nullptr)
  {
    for (const auto& currVerbInfl : verbInfls->inflections)
    {
      if (!verbCanHaveAnAuxiliary(currVerbInfl))
      {
        continue;
      }
      if (auxInfls != nullptr)
      {
        for (const auto& currAuxInfl : auxInfls->inflections)
        {
          // check gender
          if (currAuxInfl.tense != LinguisticVerbTense::PRESENT_PARTICIPLE &&
              (currAuxInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE ||
               gendersAreWeaklyEqual(currAuxInfl.gender, currVerbInfl.gender)))
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}


bool InflectionsCheckerEnglish::isVerbAdjCompatibles
(const InflectedWord& pIGramVerb,
 const InflectedWord&) const
{
  return !InflectionsChecker::verbIsAtPresentParticiple(pIGramVerb);
}


bool InflectionsCheckerEnglish::areDetCompatibles(const InflectedWord& pInflDet1,
                                                  const InflectedWord& pInflDet2) const
{
  const auto& inflections1 = pInflDet1.inflections();
  const auto& inflections2 = pInflDet2.inflections();
  if (inflections1.type != InflectionType::NOMINAL ||
      inflections2.type != InflectionType::NOMINAL)
    return true;
  const NominalInflections& verbInfls1 = inflections1.getNominalI();
  const NominalInflections& verbInfls2 = inflections2.getNominalI();
  if (verbInfls1.empty() || verbInfls2.empty())
    return true;
  for (const auto& currInfl1 : verbInfls1.inflections)
    for (const auto& currInfl2 : verbInfls2.inflections)
      if (gendersAreWeaklyEqual(currInfl1.gender, currInfl2.gender) &&
          numbersAreWeaklyEqual(currInfl1.number, currInfl2.number))
        return true;
  return ConceptSet::haveAConceptThatBeginWith(pInflDet1.infos.concepts, "number_");
}


bool InflectionsCheckerEnglish::areDetNounCompatibles(const InflectedWord& pInfWord1,
                                                     const InflectedWord& pInfWord2) const
{
  const auto& inflections1 = pInfWord1.inflections();
  const auto& inflections2 = pInfWord2.inflections();
  if (inflections1.type != InflectionType::NOMINAL ||
      inflections2.type != InflectionType::NOMINAL)
    return true;
  const NominalInflections& verbInfls1 = inflections1.getNominalI();
  const NominalInflections& verbInfls2 = inflections2.getNominalI();
  if (verbInfls1.empty() || verbInfls2.empty())
    return true;
  for (const auto& currInfl1 : verbInfls1.inflections)
    for (const auto& currInfl2 : verbInfls2.inflections)
      if (numbersAreWeaklyEqual(currInfl1.number, currInfl2.number))
        return true;
  return ConceptSet::haveAConceptThatBeginWith(pInfWord1.infos.concepts, "number_");
}

bool InflectionsCheckerEnglish::isDetProperNounCompatibles(const InflectedWord&,
                                                           const InflectedWord& pInflProperNoun) const
{
  return !ConceptSet::haveAConceptThatBeginWith(pInflProperNoun.infos.concepts, "agent_");
}


bool InflectionsCheckerEnglish::areNounDetCompatibles
(const InflectedWord& pNounInflWord,
 const InflectedWord& pDetInflWord) const
{
  return !ConceptSet::haveAConceptThatBeginWith(pNounInflWord.infos.concepts, "time_month_") ||
      !ConceptSet::haveAConceptThatBeginWithAnyOf(pDetInflWord.infos.concepts, {"number_", "rank_"});
}

bool InflectionsCheckerEnglish::areNounNounCompatibles(const InflectedWord& pNounInflWord1,
                                                       const InflectedWord& pNounInflWord2) const
{
  return ConceptSet::haveAConceptThatBeginWith(pNounInflWord2.infos.concepts, "time_") &&
      !ConceptSet::haveAConceptThatBeginWith(pNounInflWord1.infos.concepts, "time_");
}


} // End of namespace linguistics
} // End of namespace onsem
