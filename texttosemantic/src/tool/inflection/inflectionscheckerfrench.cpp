#include "inflectionscheckerfrench.hpp"
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/inflection/adjectivalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>


namespace onsem
{
namespace linguistics
{

InflectionsCheckerFrench::InflectionsCheckerFrench(const LinguisticDictionary& pLingDic)
  : InflectionsCheckerVirtual(pLingDic)
{
}


bool InflectionsCheckerFrench::isAuxVerbCompatibles(const InflectedWord& pIGramAux,
                                                    const InflectedWord& pIGramVerb) const
{
  if (InflectionsChecker::verbCanBeAtImperative(pIGramAux))
    return false;
  const Inflections& verbInfls = pIGramVerb.inflections();
  if (verbInfls.type == InflectionType::VERBAL)
    for (const auto& currVerbInfl : verbInfls.getVerbalI().inflections)
      if (verbCanHaveAnAuxiliary(currVerbInfl))
        return true;
  return false;
}


bool InflectionsCheckerFrench::_isVerbVerbCompatibles(const Inflections& pVerbInfl1,
                                                      const Inflections& pVerbInfl2) const
{
  if (pVerbInfl1.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl1 : pVerbInfl1.getVerbalI().inflections)
    {
      switch (currVerbInfl1.tense)
      {
      case LinguisticVerbTense::INFINITIVE:
      case LinguisticVerbTense::PAST_PARTICIPLE:
        return true;
      default:
      {
        if (currVerbInfl1.tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
          for (const auto& currVerbInfl2 : pVerbInfl2.getVerbalI().inflections)
            if (currVerbInfl2.tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
              return true;

        if (pVerbInfl2.type == InflectionType::VERBAL)
          for (const auto& currVerbInfl2 : pVerbInfl2.getVerbalI().inflections)
            if (currVerbInfl2.tense == LinguisticVerbTense::INFINITIVE ||
                currVerbInfl2.tense == LinguisticVerbTense::PRESENT_PARTICIPLE ||
                currVerbInfl2.tense == LinguisticVerbTense::PAST_PARTICIPLE)
              return true;
        break;
      }
      }
    }
  }
  return false;
}


bool InflectionsCheckerFrench::isDetAdjCompatibles(const InflectedWord& pIGramDet,
                                                   const InflectedWord& pIGramAdj) const
{
  if (ConceptSet::haveAConceptThatBeginWith(pIGramDet.infos.concepts, "number_"))
    return true;
  return pIGramAdj.infos.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN) &&
      isNounAdjCompatibles(pIGramDet, pIGramAdj.inflections());
}


bool InflectionsCheckerFrench::isVerbSubConjonction(const InflectedWord& pInflVerb,
                                                    const InflectedWord& pInflSubConj) const
{
  return pInflVerb.word.lemma != "être" ||
      pInflSubConj.word.lemma != "qui";
}


bool InflectionsCheckerFrench::isAdjNounCompatibles(const InflectedWord& pIGramAdj,
                                                    const InflectedWord& pIGramNoun) const
{
  return pIGramAdj.infos.hasContextualInfo(WordContextualInfos::CANBEBEFORENOUN) &&
      isNounAdjCompatibles(pIGramNoun, pIGramAdj.inflections());
}


bool InflectionsCheckerFrench::isVerbAdjCompatibles(const InflectedWord& pIGramVerb,
                                                    const InflectedWord& pIGramAdj) const
{
  const VerbalInflections* verbInfls = pIGramVerb.inflections().getVerbalIPtr();
  const AdjectivalInflections* adjInfls = pIGramAdj.inflections().getAdjectivalIPtr();

  if (verbInfls == nullptr || verbInfls->inflections.empty() ||
      adjInfls == nullptr || adjInfls->inflections.empty())
    return true;
  else
  {
    for (const auto& currVerbInfl : verbInfls->inflections)
    {
      if (currVerbInfl.tense == LinguisticVerbTense::INFINITIVE ||
          currVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE)
        return true;
      for (const auto& currAdjInfl : adjInfls->inflections)
      {
        if (numbersAreWeaklyEqual(currVerbInfl.number(), currAdjInfl.number) &&
            (currVerbInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE ||
             gendersAreWeaklyEqual(currVerbInfl.gender, currAdjInfl.gender)))
        {
          return true;
        }
      }
    }
  }
  return false;
}


bool InflectionsCheckerFrench::areDetCompatibles(const InflectedWord& pInflDet1,
                                                 const InflectedWord& pInflDet2) const
{
  return pInflDet1.infos.hasContextualInfo(WordContextualInfos::CANBEBEFOREDETERMINER) ||
      ConceptSet::haveAConceptThatBeginWith(pInflDet1.infos.concepts, "quantity_") ||
      (ConceptSet::haveAConceptThatBeginWith(pInflDet2.infos.concepts, "number_") &&
       !ConceptSet::haveAConcept(pInflDet2.infos.concepts, "number_1"));
}


bool InflectionsCheckerFrench::areDetNounCompatibles(const InflectedWord& pDeInfWord,
                                                     const InflectedWord& pNounInfWord) const
{
  const auto& detInflections = pDeInfWord.inflections();
  const auto& nounInflections = pNounInfWord.inflections();
  if (detInflections.type != InflectionType::NOMINAL ||
      nounInflections.type != InflectionType::NOMINAL)
    return true;
  bool checkGenders = !pDeInfWord.infos.hasContextualInfo(WordContextualInfos::POSSESSIVE);
  const NominalInflections& verbInfls1 = detInflections.getNominalI();
  const NominalInflections& verbInfls2 = nounInflections.getNominalI();
  if (verbInfls1.empty() || verbInfls2.empty())
    return true;
  for (const auto& currInfl1 : verbInfls1.inflections)
    for (const auto& currInfl2 : verbInfls2.inflections)
      if ((!checkGenders || gendersAreWeaklyEqual(currInfl1.gender, currInfl2.gender)) &&
          numbersAreWeaklyEqual(currInfl1.number, currInfl2.number))
        return true;
  return false;
}


bool InflectionsCheckerFrench::isDetProperNounCompatibles(const InflectedWord&,
                                                          const InflectedWord& pInflProperNoun) const
{
  return pInflProperNoun.word.lemma != "de" && pInflProperNoun.word.lemma != "du" &&
      (!ConceptSet::haveAConceptThatBeginWith(pInflProperNoun.infos.concepts, "agent_") ||
       pInflProperNoun.infos.contextualInfos.count(WordContextualInfos::BEGINSWITHUPPERCHARACTER) > 0);
}

bool InflectionsCheckerFrench::areNounNounCompatibles(const InflectedWord& pNounInflWord1,
                                                      const InflectedWord&) const
{
  return !ConceptSet::haveAConceptThatBeginWith(pNounInflWord1.infos.concepts, "number_");
}


bool InflectionsCheckerFrench::isPronounPronounComplementCompatibles(const InflectedWord& pInflPronoun) const
{
  return pInflPronoun.word.lemma != "qu'est-ce que";
}


} // End of namespace linguistics
} // End of namespace onsem
