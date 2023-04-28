#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <sstream>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/inflections.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include "inflection/inflectionscheckerenglish.hpp"
#include "inflection/inflectionscheckerfrench.hpp"

namespace onsem
{
namespace linguistics
{
namespace
{

SemanticVerbTense _verbTimeTense(VerbGoalEnum& pVerbGoal,
                                 bool pRootIsAVerb,
                                 const VerbalInflection& pVerbInfl)
{
  switch (pVerbInfl.tense)
  {
  case LinguisticVerbTense::IMPERFECT_INDICATIVE:
  case LinguisticVerbTense::IMPERFECT_SUBJONCTIVE:
  case LinguisticVerbTense::SIMPLE_PAST_INDICATIVE:
  {
    return SemanticVerbTense::PAST;
  }
  case LinguisticVerbTense::PRETERIT_CONTINUOUS:
  case LinguisticVerbTense::PAST_PARTICIPLE:
  {
    return SemanticVerbTense::PUNCTUALPAST;
  }
  case LinguisticVerbTense::PRESENT_INDICATIVE:
  case LinguisticVerbTense::PRESENT_CONTINUOUS:
  case LinguisticVerbTense::PRESENT_SUBJONCTIVE:
  case LinguisticVerbTense::PRESENT_IMPERATIVE:
  {
    return SemanticVerbTense::PRESENT;
  }
  case LinguisticVerbTense::PRESENT_PARTICIPLE:
  {
    return pRootIsAVerb ? SemanticVerbTense::UNKNOWN : SemanticVerbTense::PRESENT;
  }
  case LinguisticVerbTense::PRESENT_CONDITIONAL:
  {
    pVerbGoal = VerbGoalEnum::CONDITIONAL;
    return SemanticVerbTense::PRESENT;
  }
  case LinguisticVerbTense::FUTURE_INDICATIVE:
  {
    return SemanticVerbTense::FUTURE;
  }
  case LinguisticVerbTense::INFINITIVE:
    return SemanticVerbTense::UNKNOWN;
  }
  return SemanticVerbTense::UNKNOWN;
}

bool _isPrepVerbCompatibles(const Inflections& pVerbInflections)
{
  if (pVerbInflections.type == InflectionType::VERBAL)
  {
    const VerbalInflections& verbInfls = pVerbInflections.getVerbalI();
    for (const auto& currVerbInfl : verbInfls.inflections)
      if (currVerbInfl.tense == LinguisticVerbTense::INFINITIVE ||
          currVerbInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE)
        return true;
  }
  return false;
}

bool _isVerbPronounComplementCompatibles(const Inflections& pVerbInfls)
{
  bool res = false;
  if (pVerbInfls.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : pVerbInfls.getVerbalI().inflections)
    {
      if (currVerbInfl.tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
      {
        res = true;
        continue;
      }
    }
  }
  return res;
}

bool _canInterjectionBeAtTheEndOfASentence(const InflectedWord& pInflWordInterjection)
{
  return ConceptSet::haveAConcept(pInflWordInterjection.infos.concepts, "forExample");
}


bool _isAdjAdjCompatibles(const Inflections& pAdjInfls1,
                          const Inflections& pAdjInfls2)
{
  if (pAdjInfls1.type != InflectionType::ADJECTIVAL ||
      pAdjInfls2.type != InflectionType::ADJECTIVAL)
    return true;
  const AdjectivalInflections& adjInfls1 = pAdjInfls1.getAdjectivalI();
  const AdjectivalInflections& adjInfls2 = pAdjInfls2.getAdjectivalI();
  if (adjInfls1.inflections.empty() || adjInfls2.inflections.empty())
    return true;
  for (const auto& currAdjInfl1 : adjInfls1.inflections)
    for (const auto& currAdjInfl2 : adjInfls2.inflections)
      if (gendersAreWeaklyEqual(currAdjInfl1.gender, currAdjInfl2.gender) &&
          numbersAreWeaklyEqual(currAdjInfl1.number, currAdjInfl2.number))
        return true;
  return false;
}

bool _verbSetAt3ePers(const VerbalInflection& pVerbInfl)
{
  return pVerbInfl.person == RelativePerson::THIRD_SING || pVerbInfl.person == RelativePerson::THIRD_PLUR;
}

bool _conjVerbCanBeAt3ePers(const VerbalInflection& pVerbInfl)
{
  return pVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE ||
      pVerbInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE ||
      _verbSetAt3ePers(pVerbInfl);
}

bool _isNounVerbCompatibles(const Inflections& pNomInflections,
                            const Inflections& pVerbInflections)
{
  if (pVerbInflections.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : pVerbInflections.getVerbalI().inflections)
    {
      if (currVerbInfl.tense == LinguisticVerbTense::INFINITIVE ||
          currVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE)
      {
        return true;
      }
      if (pNomInflections.type == InflectionType::NOMINAL)
      {
        for (const auto& currNomInfl : pNomInflections.getNominalI().inflections)
        {
          if ((_conjVerbCanBeAt3ePers(currVerbInfl) &&
               numbersAreWeaklyEqual(currVerbInfl.number(), currNomInfl.number)) &&
              (currVerbInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE ||
               gendersAreWeaklyEqual(currVerbInfl.gender, currNomInfl.gender)))
          {
            return true;
          }
        }
      }
      else if (_conjVerbCanBeAt3ePers(currVerbInfl))
        return true;
    }
  }
  return false;
}

bool _isPronounNounCompatibles(const InflectedWord& pIGramPron,
                               const LinguisticDictionary& pLingDic)
{
  return pLingDic.semWordToRequest(pIGramPron.word) == SemanticRequestType::OBJECT;
}

bool _isPronounAdjCompatibles(const InflectedWord& pIGramPron,
                              const LinguisticDictionary& pLingDic)
{
  return pLingDic.semWordToRequest(pIGramPron.word) == SemanticRequestType::OBJECT;
}

bool _isPronounSubjectNounCompatibles(const Inflections& pPronInfls,
                                      const Inflections& pNomInfls)
{
  const PronominalInflections* pronInfls = pPronInfls.getPronominalIPtr();
  if (pronInfls == nullptr || pronInfls->inflections.empty())
    return true;
  else
  {
    for (const auto& currPronInfl : pronInfls->inflections)
    {
      if (currPronInfl.personWithoutNumber != RelativePersonWithoutNumber::UNKNOWN)
        continue;
      if (pNomInfls.type == InflectionType::NOMINAL)
      for (const auto& currNomInlf : pNomInfls.getNominalI().inflections)
        if (gendersAreWeaklyEqual(currPronInfl.gender, currNomInlf.gender) &&
            numbersAreWeaklyEqual(currPronInfl.number, currNomInlf.number))
          return true;
    }
  }
  return false;
}

bool _isPronounSubjectAdjCompatibles(const Inflections& pPronInfls,
                                     const Inflections& pAdjInfls)
{
  const PronominalInflections* pronInfls = pPronInfls.getPronominalIPtr();
  if (pronInfls == nullptr || pronInfls->inflections.empty())
    return true;
  else
  {
    for (const auto& currPronInfl : pronInfls->inflections)
    {
      if (currPronInfl.personWithoutNumber != RelativePersonWithoutNumber::UNKNOWN)
        continue;
      if (pAdjInfls.type == InflectionType::ADJECTIVAL)
      for (const auto& currNomInlf : pAdjInfls.getAdjectivalI().inflections)
        if (gendersAreWeaklyEqual(currPronInfl.gender, currNomInlf.gender) &&
            numbersAreWeaklyEqual(currPronInfl.number, currNomInlf.number))
          return true;
    }
  }
  return false;
}

bool _verbCanHaveASubject(const VerbalInflection& pVerbInfl)
{
  return pVerbInfl.tense != LinguisticVerbTense::INFINITIVE &&
      pVerbInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE &&
      pVerbInfl.tense != LinguisticVerbTense::PRESENT_PARTICIPLE &&
      pVerbInfl.tense != LinguisticVerbTense::PRESENT_IMPERATIVE;
}


bool _isPronounVerbCompatibles(const InflectedWord& pIGramPron,
                               const InflectedWord& pIGramVerb)
{
  const Inflections& verbInfls = pIGramVerb.inflections();
  if (verbInfls.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : verbInfls.getVerbalI().inflections)
    {
      if (currVerbInfl.tense == LinguisticVerbTense::INFINITIVE)
        return true;
      if (!_verbCanHaveASubject(currVerbInfl))
        continue;
      const PronominalInflections* pronInflsPtr = pIGramPron.inflections().getPronominalIPtr();
      if ((pronInflsPtr == nullptr || pronInflsPtr->inflections.empty()) &&
          _conjVerbCanBeAt3ePers(currVerbInfl))
        return true;
      if (pronInflsPtr != nullptr)
        for (const auto& currPronInfl : pronInflsPtr->inflections)
          if (relativePersonsAreWeaklyEqual(currVerbInfl.person, currPronInfl.person()))
            return true;
    }
  }
  return false;
}

bool _isPronounSubjectVerbCompatibles(const InflectedWord& pIGramPron,
                                      const InflectedWord& pIGramVerb)
{
  const Inflections& verbInfls = pIGramVerb.inflections();
  if (verbInfls.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : verbInfls.getVerbalI().inflections)
    {
      if (!_verbCanHaveASubject(currVerbInfl))
        continue;
      const PronominalInflections* pronInflsPtr = pIGramPron.inflections().getPronominalIPtr();
      if ((pronInflsPtr == nullptr || pronInflsPtr->inflections.empty()) &&
          _conjVerbCanBeAt3ePers(currVerbInfl))
        return true;
      if (pronInflsPtr != nullptr)
        for (const auto& currPronInfl : pronInflsPtr->inflections)
          if (relativePersonsAreWeaklyEqual(currVerbInfl.person, currPronInfl.person()))
            return true;
    }
  }
  return false;
}

bool _pronounCanReferToA3ePers(const PronominalInflection& pPronInfl)
{
  return pPronInfl.personWithoutNumber == RelativePersonWithoutNumber::THIRD ||
      pPronInfl.personWithoutNumber == RelativePersonWithoutNumber::UNKNOWN;
}


}



InflectionsChecker::InflectionsChecker
(const SpecificLinguisticDatabase& pSpecLingDb)
  : fBinDico(pSpecLingDb.lingDico),
    _impl()
{
  if (pSpecLingDb.language == SemanticLanguageEnum::FRENCH)
    _impl = std::make_unique<InflectionsCheckerFrench>(pSpecLingDb.lingDico);
  else
    _impl = std::make_unique<InflectionsCheckerEnglish>(pSpecLingDb.lingDico);
}

InflectionsChecker::~InflectionsChecker()
{
}


bool InflectionsChecker::areCompatibles
(const InflectedWord& pIGram1,
 const InflectedWord& pIGram2) const
{
  switch (pIGram1.word.partOfSpeech)
  {
  case PartOfSpeech::PREPOSITION:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::VERB:
    {
      return _isPrepVerbCompatibles(pIGram2.inflections());
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::NOUN:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::NOUN:
    {
      return _impl->areNounNounCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::DETERMINER:
    {
      return _impl->areNounDetCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::ADJECTIVE:
    {
      return _impl->isNounAdjCompatibles(pIGram1, pIGram2.inflections());
    }
    case PartOfSpeech::VERB:
    {
      return _isNounVerbCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    case PartOfSpeech::PRONOUN:
    case PartOfSpeech::PRONOUN_SUBJECT:
    {
      return _isPronounSubjectNounCompatibles(pIGram2.inflections(), pIGram1.inflections());
    }
    case PartOfSpeech::INTERJECTION:
    {
      return _canInterjectionBeAtTheEndOfASentence(pIGram2);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::DETERMINER:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::ADJECTIVE:
    {
      return _impl->isDetAdjCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::DETERMINER:
    {
      return _impl->areDetCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::NOUN:
    {
      return _impl->areDetNounCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::PRONOUN:
    case PartOfSpeech::PRONOUN_SUBJECT:
    {
      return _isPronounSubjectNounCompatibles(pIGram2.inflections(), pIGram1.inflections());
    }
    case PartOfSpeech::PROPER_NOUN:
    {
      return _impl->isDetProperNounCompatibles(pIGram1, pIGram2);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::ADJECTIVE:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::NOUN:
    {
      return _impl->isAdjNounCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::ADJECTIVE:
    {
      return _isAdjAdjCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    case PartOfSpeech::DETERMINER:
    {
      return _impl->isNounAdjCompatibles(pIGram2, pIGram1.inflections());
    }
    case PartOfSpeech::VERB:
    {
      return _impl->isVerbAdjCompatibles(pIGram2, pIGram1);
    }
    case PartOfSpeech::PRONOUN:
    case PartOfSpeech::PRONOUN_SUBJECT:
    {
      return _isPronounSubjectAdjCompatibles(pIGram2.inflections(), pIGram1.inflections());
    }
    case PartOfSpeech::PROPER_NOUN:
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::VERB:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::ADJECTIVE:
    {
      return _impl->isVerbAdjCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::NOUN:
    case PartOfSpeech::UNKNOWN:
    {
      return _isNounVerbCompatibles(pIGram2.inflections(), pIGram1.inflections());
    }
    case PartOfSpeech::VERB:
    {
      return _impl->_isVerbVerbCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    case PartOfSpeech::PRONOUN:
    case PartOfSpeech::PRONOUN_SUBJECT:
    {
      return _isPronounVerbCompatibles(pIGram2, pIGram1);
    }
    case PartOfSpeech::PRONOUN_COMPLEMENT:
    {
      return _isVerbPronounComplementCompatibles(pIGram1.inflections());
    }
    case PartOfSpeech::PREPOSITION:
    {
      return true;
    }
    case PartOfSpeech::AUX:
    {
      return false;
    }
    case PartOfSpeech::INTERJECTION:
    {
      return ConceptSet::haveAConcept(pIGram1.infos.concepts, "verb_action_say") ||
          ConceptSet::haveAConcept(pIGram2.infos.concepts, "please");
    }
    case PartOfSpeech::SUBORDINATING_CONJONCTION:
    {
      return _impl->isVerbSubConjonction(pIGram1, pIGram2);
    }
    default:
    {
      return true;
    }
    }
  }


  case PartOfSpeech::AUX:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::VERB:
    {
      return _impl->isAuxVerbCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::PRONOUN:
    case PartOfSpeech::PRONOUN_SUBJECT:
    {
      return _isPronounVerbCompatibles(pIGram2, pIGram1);
    }
    case PartOfSpeech::NOUN:
    case PartOfSpeech::UNKNOWN:
    {
      return _isNounVerbCompatibles(pIGram2.inflections(), pIGram1.inflections());
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::INTERJECTION:
  {
    return _impl->isIntjInflCompatibles(pIGram1, pIGram2);
  }

  case PartOfSpeech::PROPER_NOUN:
  case PartOfSpeech::UNKNOWN:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::AUX:
    case PartOfSpeech::VERB:
    {
      return _isNounVerbCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    default:
    {
      return true;
    }
    }
  }



  case PartOfSpeech::PRONOUN:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::NOUN:
    case PartOfSpeech::DETERMINER:
    {
      return _isPronounNounCompatibles(pIGram1, fBinDico);
    }
    case PartOfSpeech::ADJECTIVE:
    {
      return _isPronounAdjCompatibles(pIGram1, fBinDico);
    }
    case PartOfSpeech::VERB:
    {
      return _isPronounVerbCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::PRONOUN_COMPLEMENT:
    {
      return _impl->isPronounPronounComplementCompatibles(pIGram1);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::PRONOUN_COMPLEMENT:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::DETERMINER:
    {
      return _impl->isPronounCompDetCompatibles(pIGram1);
    }
    case PartOfSpeech::ADVERB:
    {
      return _impl->isPronounComplAdverbCompatibles(pIGram1, pIGram2);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::PRONOUN_SUBJECT:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::NOUN:
    case PartOfSpeech::DETERMINER:
    {
      return _isPronounSubjectNounCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    case PartOfSpeech::ADJECTIVE:
    {
      return _isPronounSubjectAdjCompatibles(pIGram1.inflections(), pIGram2.inflections());
    }
    case PartOfSpeech::VERB:
    {
      return _isPronounSubjectVerbCompatibles(pIGram1, pIGram2);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::PUNCTUATION:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::VERB:
    {
      return !verbIsOnlyAtPastParticiple(pIGram2);
    }
    case PartOfSpeech::SUBORDINATING_CONJONCTION:
    {
      return pIGram2.infos.hasContextualInfo(WordContextualInfos::SENTENCECANBEGINWITH);
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::PARTITIVE:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::AUX:
    case PartOfSpeech::INTERJECTION:
    case PartOfSpeech::VERB:
    {
      return false;
    }
    default:
    {
      return true;
    }
    }
  }

  case PartOfSpeech::ADVERB:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::INTERJECTION:
    {
      return _impl->isAdvIntjCompatibles(pIGram1, pIGram2);
    }
    case PartOfSpeech::NOUN:
    case PartOfSpeech::PROPER_NOUN:
    case PartOfSpeech::UNKNOWN:
    {
      return !pIGram1.infos.hasContextualInfo(WordContextualInfos::CANNOTBEBEFORENOUN);
    }
    default:
    {
      return true;
    }
    }
  }


  default:
  {
    return true;
  }
  }
}


void InflectionsChecker::unionOfSameVerbTenses
(VerbalInflections& pResVerInfls,
 const Inflections& pInflections1,
 const Inflections& pInflections2) const
{
  if (pInflections1.type == InflectionType::VERBAL &&
      pInflections2.type == InflectionType::VERBAL)
  {
    const VerbalInflections& verbInfl1 = pInflections1.getVerbalI();
    const VerbalInflections& verbInfl2 = pInflections2.getVerbalI();
    pResVerInfls = verbInfl1;
    for (const auto& currVerbInfl2 : verbInfl2.inflections)
    {
      bool flexionExists = false;
      for (const auto& currVerbInfl1 : verbInfl1.inflections)
      {
        if (currVerbInfl2.tense == currVerbInfl1.tense)
        {
          flexionExists = true;
          break;
        }
      }
      if (!flexionExists)
        pResVerInfls.inflections.emplace_back(currVerbInfl2);
    }
  }
}

void InflectionsChecker::intersectionOfSameVerbTenses
(VerbalInflections& pExistingVerbInfls,
 const VerbalInflections& pNewVerbInfls) const
{
  for (auto it = pExistingVerbInfls.inflections.begin();
       it != pExistingVerbInfls.inflections.end(); )
  {
    bool flexionExists = false;
    for (const auto& currNewFlexion : pNewVerbInfls.inflections)
    {
      if (it->tense == currNewFlexion.tense)
      {
        flexionExists = true;
        break;
      }
    }
    if (flexionExists)
      ++it;
    else
      it = pExistingVerbInfls.inflections.erase(it);
  }
}


bool InflectionsChecker::verbIsOnlyAtPastParticiple
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
  {
    const VerbalInflections& verbInfls = infls.getVerbalI();
    for (const auto& currInfl : verbInfls.inflections)
      if (currInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE)
        return false;
    return !verbInfls.inflections.empty();
  }
  return false;
}


bool InflectionsChecker::verbIsOnlyAtPresentOrPastParticiple
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
  {
    const VerbalInflections& verbInfls = infls.getVerbalI();
    for (const auto& currInfl : verbInfls.inflections)
      if (currInfl.tense != LinguisticVerbTense::PRESENT_PARTICIPLE &&
          currInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE)
        return false;
    return !verbInfls.inflections.empty();
  }
  return false;
}


bool InflectionsChecker::verbIsAtPastParticiple
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE)
        return true;
  return false;
}

bool InflectionsChecker::verbCanBeAtImperative
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
        return true;
  return false;
}

bool InflectionsChecker::verbCanBeAtThridOfSingularExceptImperative
(const InflectedWord& pInflWord)
{
  const Inflections& infls = pInflWord.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
    {
      if (currInfl.tense != LinguisticVerbTense::PRESENT_IMPERATIVE &&
          currInfl.person == RelativePerson::THIRD_SING)
        return true;
    }
  return false;
}

bool InflectionsChecker::verbIsAtPresentIndicative
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::PRESENT_INDICATIVE)
        return true;
  return false;
}

bool InflectionsChecker::verbIsAtPresentParticiple
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    return verbalInflAreAtPresentParticiple(infls.getVerbalI());
  return false;
}

bool InflectionsChecker::verbalInflAreAtPresentParticiple
(const VerbalInflections& pVerbalInfls)
{
  for (const auto& currInfl : pVerbalInfls.inflections)
    if (currInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE)
      return true;
  return false;
}

bool InflectionsChecker::verbRemoveAllInflectionsThatAreNotAtPastParticiple
(InflectedWord& pInflWord)
{
  bool res = false;
  Inflections& infls = pInflWord.inflections();
  if (infls.type == InflectionType::VERBAL)
  {
    auto& verbalInfls = infls.getVerbalI().inflections;
    for (auto itInfl = verbalInfls.begin(); itInfl != verbalInfls.end(); )
    {
      if (itInfl->tense != LinguisticVerbTense::PAST_PARTICIPLE)
      {
        itInfl = verbalInfls.erase(itInfl);
        res = true;
      }
      else
      {
        ++itInfl;
      }
    }
  }
  return res;
}

bool InflectionsChecker::verbIsAtInfinitive
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::INFINITIVE)
        return true;
  return false;
}

bool InflectionsChecker::verbIsConjugated
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense != LinguisticVerbTense::INFINITIVE)
        return true;
  return false;
}


bool InflectionsChecker::isuncountable(const WordAssociatedInfos& pWordWithInfos)
{
  return ConceptSet::haveAConceptIncompatibleWithSomethingCountable(pWordWithInfos.concepts) ||
      pWordWithInfos.hasContextualInfo(WordContextualInfos::UNCOUNTABLE);
}

bool InflectionsChecker::verbCanBeSingular
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.number() == SemanticNumberType::SINGULAR)
        return true;
  return false;
}


bool InflectionsChecker::verbCanBePlural
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.number() == SemanticNumberType::PLURAL)
        return true;
  return false;
}

bool InflectionsChecker::verbIsConjAtPerson
(const InflectedWord& pInflWord,
 RelativePerson pPerson) const
{
  const Inflections& infls = pInflWord.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.person == pPerson)
        return true;
  return false;
}

bool InflectionsChecker::verbCanHaveAnAuxiliary
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (_impl->verbCanHaveAnAuxiliary(currInfl))
        return true;
  return false;
}

bool InflectionsChecker::verbHasToHaveAnAuxiliary
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
  {
    const VerbalInflections& verbInfls = infls.getVerbalI();
    for (const auto& currInfl : verbInfls.inflections)
      if (currInfl.tense != LinguisticVerbTense::PAST_PARTICIPLE)
        return false;
    return !verbInfls.inflections.empty();
  }
  return false;
}


void InflectionsChecker::initGenderSetFromIGram
(std::set<SemanticGenderType>& pPossibleGenders,
 const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  switch (infls.type)
  {
  case InflectionType::NOMINAL:
  {
    for (const auto& currNomInfl : infls.getNominalI().inflections)
      if (currNomInfl.gender != SemanticGenderType::UNKNOWN)
        pPossibleGenders.insert(currNomInfl.gender);
    if (pPossibleGenders.size() > 1)
      pPossibleGenders.clear();
    break;
  }
  case InflectionType::PRONOMINAL:
    for (const auto& currPronInfl : infls.getPronominalI().inflections)
      if (currPronInfl.gender != SemanticGenderType::UNKNOWN)
        pPossibleGenders.insert(currPronInfl.gender);
    break;
  default:
    break;
  }
}


bool InflectionsChecker::verbCantHaveSubject
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (_verbCanHaveASubject(currInfl))
        return true;
  return false;
}


bool InflectionsChecker::verbCanHaveNoSubject(const InflectedWord& pInflWord)
{
  const Inflections& infls = pInflWord.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::PRESENT_IMPERATIVE ||
          currInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE ||
          currInfl.tense == LinguisticVerbTense::INFINITIVE)
        return true;
  return false;
}


RelativePerson InflectionsChecker::imperativeVerbToRelativePerson
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::VERBAL)
    for (const auto& currInfl : infls.getVerbalI().inflections)
      if (currInfl.tense == LinguisticVerbTense::PRESENT_IMPERATIVE)
      {
        if (currInfl.person == RelativePerson::UNKNOWN)
          return RelativePerson::SECOND_SING;
        return currInfl.person;
      }
  return RelativePerson::SECOND_SING;
}


bool InflectionsChecker::pronounSetAtSamePers
(const InflectedWord& pIGram1,
 const InflectedWord& pIGram2) const
{  
  const Inflections& infls1 = pIGram1.inflections();
  const Inflections& infls2 = pIGram2.inflections();
  if (infls1.type == InflectionType::PRONOMINAL && infls2.type == InflectionType::PRONOMINAL)
  {
    const PronominalInflections pronInfls1 = infls1.getPronominalI();
    const PronominalInflections pronInfls2 = infls2.getPronominalI();
    return pronInfls1.inflections.size() == 1 &&
        pronInfls2.inflections.size() == 1 &&
        pronInfls1.inflections.front().person() == pronInfls2.inflections.front().person();
  }
  return false;
}


bool InflectionsChecker::pronounCanBePlural
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
    for (const auto& currInfl : infls.getPronominalI().inflections)
      if (currInfl.number == SemanticNumberType::PLURAL)
        return true;
  return false;
}

void InflectionsChecker::pronounRemovePluralPossibility
(InflectedWord& pIGram) const
{
  Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
  {
    auto& pronInfls = infls.getPronominalI();
    for (auto itInfl = pronInfls.inflections.begin();
         itInfl != pronInfls.inflections.end(); )
    {
      if (itInfl->number == SemanticNumberType::PLURAL)
        itInfl = pronInfls.inflections.erase(itInfl);
      else
        ++itInfl;
    }
  }
}

void InflectionsChecker::pronounRemoveSingularPossibility
(InflectedWord& pIGram) const
{
  Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
  {
    auto& pronInfls = infls.getPronominalI();
    for (auto itInfl = pronInfls.inflections.begin();
         itInfl != pronInfls.inflections.end(); )
    {
      if (itInfl->number == SemanticNumberType::SINGULAR)
        itInfl = pronInfls.inflections.erase(itInfl);
      else
        ++itInfl;
    }
  }
}

bool InflectionsChecker::nounCanBePlural
(const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::NOMINAL)
    for (const auto& currInfl : infls.getNominalI().inflections)
      if (currInfl.number == SemanticNumberType::PLURAL)
        return true;
  return false;
}


void InflectionsChecker::getNounNumberAndGender(SemanticNumberType& pNumber,
                                                SemanticGenderType& pGender,
                                                const InflectedWord& pIGram)
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::NOMINAL)
  {
    for (const auto& currInfl : infls.getNominalI().inflections)
    {
      pNumber = currInfl.number;
      pGender = currInfl.gender;
      return;
    }
  }
}


bool InflectionsChecker::adjCanBeComparative
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::ADJECTIVAL)
    for (const auto& currInfl : infls.getAdjectivalI().inflections)
      if (currInfl.comparisonType == ComparisonType::COMPARATIVE)
        return true;
  return false;
}



bool InflectionsChecker::pronounSetAt3ePers
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
    for (const auto& currInfl : infls.getPronominalI().inflections)
      if (currInfl.personWithoutNumber == RelativePersonWithoutNumber::THIRD)
        return true;
  return false;
}

bool InflectionsChecker::pronounCanReferToA3ePers
(const InflectedWord& pIGram) const
{
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
  {
    PronominalInflections pronInfls = infls.getPronominalI();
    for (const auto& currInfl : pronInfls.inflections)
      if (_pronounCanReferToA3ePers(currInfl))
        return true;
    return pronInfls.inflections.empty();
  }
  return true;
}



RelativePerson InflectionsChecker::pronounGetPerson
(const InflectedWord& pIGram) const
{
  bool canBeSingular = false;
  bool canBePlural = false;
  const Inflections& infls = pIGram.inflections();
  if (infls.type == InflectionType::PRONOMINAL)
  {
    for (const auto& currInfl : infls.getPronominalI().inflections)
    {
      if (currInfl.personWithoutNumber != RelativePersonWithoutNumber::UNKNOWN)
        return currInfl.person();
      if (currInfl.number == SemanticNumberType::PLURAL)
        canBePlural = true;
      else if (currInfl.number == SemanticNumberType::SINGULAR)
        canBeSingular = true;
    }
  }
  if (!canBeSingular && canBePlural)
    return RelativePerson::THIRD_PLUR;
  return RelativePerson::THIRD_SING;
}




bool InflectionsChecker::areVerbAndPronComplCanBeLinked
(const Inflections& pPronComplInflections,
 const Inflections& pVerbInflections) const
{
  if (pVerbInflections.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : pVerbInflections.getVerbalI().inflections)
    {
      if (currVerbInfl.tense == LinguisticVerbTense::PRESENT_PARTICIPLE)
        continue;
      const PronominalInflections* pronInflsPtr = pPronComplInflections.getPronominalIPtr();
      if ((pronInflsPtr == nullptr || pronInflsPtr->inflections.empty()) &&
          _conjVerbCanBeAt3ePers(currVerbInfl))
        return true;
      if (pronInflsPtr != nullptr)
      {
        for (const auto& currPronInfl : pronInflsPtr->inflections)
        {
          if (currVerbInfl.tense == LinguisticVerbTense::PAST_PARTICIPLE)
          {
            if (_pronounCanReferToA3ePers(currPronInfl))
              return true;
          }
          else if (relativePersonsAreWeaklyEqual(currVerbInfl.person, currPronInfl.person()))
            return true;
        }
      }
    }
  }
  return false;
}


bool InflectionsChecker::canBeAssociatedInAList
(const InflectedWord& pIGram1,
 const InflectedWord& pIGram2) const
{
  if (pIGram1.word.partOfSpeech == pIGram2.word.partOfSpeech ||
      pIGram1.word.partOfSpeech == PartOfSpeech::UNKNOWN ||
      pIGram2.word.partOfSpeech == PartOfSpeech::UNKNOWN)
  {
    return true;
  }

  switch (pIGram1.word.partOfSpeech)
  {
  case PartOfSpeech::PRONOUN:
  {
    return pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT ||
        pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT ||
        (pIGram2.word.partOfSpeech == PartOfSpeech::NOUN && !fBinDico.hasContextualInfo(WordContextualInfos::POSSESSIVE, pIGram1.word));
  }
  case PartOfSpeech::PRONOUN_COMPLEMENT:
  {
    return pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN ||
        pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT;
  }
  case PartOfSpeech::PRONOUN_SUBJECT:
  {
    return pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN ||
        pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT;
  }
  case PartOfSpeech::NOUN:
  {
    return (pIGram2.word.partOfSpeech == PartOfSpeech::PRONOUN && !fBinDico.hasContextualInfo(WordContextualInfos::POSSESSIVE, pIGram2.word)) ||
        (pIGram2.word.partOfSpeech == PartOfSpeech::PROPER_NOUN && !nounCanBePlural(pIGram1));
  }
  case PartOfSpeech::PROPER_NOUN:
  {
    return pIGram2.word.partOfSpeech == PartOfSpeech::NOUN && !nounCanBePlural(pIGram2);
  }
  case PartOfSpeech::ADJECTIVE:
  {
    switch (pIGram2.word.partOfSpeech)
    {
    case PartOfSpeech::VERB:
    {
      return verbIsAtPresentParticiple(pIGram2);
    }
    default:
    {
      return false;
    }
    }
  }
  default:
  {
    return false;
  }
  }

  return false;
}


void InflectionsChecker::verbTenseAndGoalFromInflections(SemanticVerbTense& pRes,
                                                         VerbGoalEnum& pVerbGoal,
                                                         const Inflections& pInflections,
                                                         bool pRootIsAVerb) const
{
  if (pInflections.type == InflectionType::VERBAL)
  {
    for (const auto& currVerbInfl : pInflections.getVerbalI().inflections)
    {
      SemanticVerbTense newTimeTense = _verbTimeTense(pVerbGoal, pRootIsAVerb, currVerbInfl);
      if (newTimeTense != SemanticVerbTense::UNKNOWN)
      {
        pRes = newTimeTense;
        if (pRes == SemanticVerbTense::PRESENT)
          break;
      }
    }
  }
}

} // End of namespace linguistics
} // End of namespace onsem
