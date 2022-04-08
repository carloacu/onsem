#include "linguisticsynthesizergroundingenglish.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include "../tool/synthesizeradder.hpp"
#include "../tool/synthesizergetter.hpp"

namespace onsem
{

LinguisticsynthesizergroundingEnglish::LinguisticsynthesizergroundingEnglish(const LinguisticSynthesizerPrivate& pLingSynth)
  : Linguisticsynthesizergrounding(SemanticLanguageEnum::ENGLISH, pLingSynth),
    _chunksMerger()
{
}


bool LinguisticsynthesizergroundingEnglish::_dateTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pDicoSynth,
 const SemanticDate& pDate) const
{
  if (!pDate.month && !pDate.year)
    return false;

  if (pDate.month)
  {
    if (pDate.day)
    {
      const auto& meaning = pDicoSynth.conceptToMeaning(monthConceptStr_fromMonthId(*pDate.month));
      if (!meaning.isEmpty())
      {
        std::map<std::string, char> monthConcept{{"time_month_*", 4}};
        _strToOutCptsMove(pOut, PartOfSpeech::NOUN, pDicoSynth.getLemma(meaning, false),
                          std::move(monthConcept), WordToSynthesizeTag::DATE);
      }
      {
        std::stringstream ss;
        ss << *pDate.day;
        _strToOut(pOut, PartOfSpeech::NOUN, ss.str(), WordToSynthesizeTag::DATE);
      }
      if (pDate.year)
      {
        pOut.emplace_back(SemanticWord(_language, ",", PartOfSpeech::LINKBETWEENWORDS),
                          InflectionToSynthesize(",", false, true, alwaysTrue));
        std::stringstream ss;
        ss << *pDate.year;
        _strToOut(pOut, PartOfSpeech::NOUN, ss.str(), WordToSynthesizeTag::DATE);
      }
      return true;
    }

    const auto& meaning = pDicoSynth.conceptToMeaning(monthConceptStr_fromMonthId(*pDate.month));
    if (!meaning.isEmpty())
      _strToOut(pOut, PartOfSpeech::NOUN, pDicoSynth.getLemma(meaning, false), WordToSynthesizeTag::DATE);
  }
  if (pDate.year)
  {
    std::stringstream ss;
    ss << *pDate.year;
    _strToOut(pOut, PartOfSpeech::NOUN, ss.str(), WordToSynthesizeTag::DATE);
  }
  return true;
}


PartOfSpeech LinguisticsynthesizergroundingEnglish::writeRelativePerson
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticReferenceType pReferenceType,
 bool,
 SemanticEntityType pAgentType,
 const SemanticQuantity& pQuantity,
 const SynthesizerCurrentContext& pContext,
 const SemanticRequests&) const
{
  return _writeRelativePerson(pOut, pRelativePerson, pContext.wordContext.gender,
                              pReferenceType, pAgentType,
                              pQuantity, pContext.contextType, pContext.verbTense,
                              pContext.isPositive);
}


std::string LinguisticsynthesizergroundingEnglish::_usRelativePersonToStr
(SynthesizerCurrentContextType pContextType,
 LinguisticVerbTense pVerbTense) const
{
  return pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT &&
      pVerbTense != LinguisticVerbTense::INFINITIVE ?
        "we" : "us";
}


void LinguisticsynthesizergroundingEnglish::rankConceptToInlfWord(
    linguistics::InflectedWord& pInflWord,
    const std::map<std::string, char>& pConcepts) const
{
  std::string numberStr;
  if (ConceptSet::rankConceptToNumberStr(numberStr, pConcepts))
  {
    pInflWord.word.lemma = numberStr + "th";
    pInflWord.word.partOfSpeech = PartOfSpeech::ADJECTIVE;
    pInflWord.word.language = SemanticLanguageEnum::ENGLISH;
  }
}


bool LinguisticsynthesizergroundingEnglish::_groundingAttributesToWord(linguistics::InflectedWord& pOutInfoGram,
                                                                      const SemanticGenericGrounding& pGrounding) const
{
  if (pGrounding.coreference &&
      pGrounding.referenceType == SemanticReferenceType::DEFINITE &&
      ConceptSet::haveAConcept(pGrounding.concepts, "location"))
  {
    pOutInfoGram.word.setContent(_language, "there", PartOfSpeech::ADVERB);
    return true;
  }
  return false;
}


PartOfSpeech LinguisticsynthesizergroundingEnglish::_writeRelativePerson
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticGenderType pGender,
 SemanticReferenceType pReferenceType,
 SemanticEntityType pAgentType,
 const SemanticQuantity& pQuantity,
 SynthesizerCurrentContextType pContextType,
 LinguisticVerbTense pVerbTense,
 bool pIsPosisitve) const
{
  _strToOutIfNotEmpty(pOut, PartOfSpeech::PRONOUN, [&]() -> std::string
  {
    switch (pRelativePerson)
    {
    case RelativePerson::FIRST_SING:
    {
      return pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT &&
          pVerbTense != LinguisticVerbTense::INFINITIVE ?
            "I" : "me";
    }
    case RelativePerson::SECOND_SING:
    {
      return "you";
    }
    case RelativePerson::FIRST_PLUR:
    {
      return _usRelativePersonToStr(pContextType, pVerbTense);
    }
    case RelativePerson::THIRD_PLUR:
    {
      if (pQuantity.type == SemanticQuantityType::MAXNUMBER)
        return "all";
      return pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT &&
          pVerbTense != LinguisticVerbTense::INFINITIVE ?
            "they" : "them";
    }
    case RelativePerson::THIRD_SING:
    default:
    {
      if (pQuantity.type == SemanticQuantityType::NUMBER &&
          pQuantity.nb == 0)
      {
        if (pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB &&
            pVerbTense != LinguisticVerbTense::INFINITIVE)
        {
          if (pAgentType == SemanticEntityType::HUMAN)
            return pIsPosisitve ? "somebody" : "anybody";
          return pIsPosisitve ? "something" : "anything";
        }
        return pAgentType == SemanticEntityType::HUMAN ? "nobody" : "nothing";
      }
      if (pQuantity.type == SemanticQuantityType::EVERYTHING)
      {
        return pAgentType == SemanticEntityType::HUMAN ?
              "everybody" :"everything";
      }
      if (pQuantity.type == SemanticQuantityType::ANYTHING)
      {
        return pAgentType == SemanticEntityType::HUMAN ?
              "anybody" :"anything";
      }
      if (pQuantity.type == SemanticQuantityType::MAXNUMBER)
      {
        return "all";
      }
      if (pReferenceType == SemanticReferenceType::INDEFINITE)
      {
        return pAgentType == SemanticEntityType::HUMAN ?
              "somebody" : "something";
      }
      else if (pContextType != SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT)
      {
        if (pAgentType == SemanticEntityType::HUMAN)
        {
          switch (pGender)
          {
          case SemanticGenderType::FEMININE:
            return "her";
          case SemanticGenderType::MASCULINE:
            return "him";
          case SemanticGenderType::NEUTRAL:
          case SemanticGenderType::UNKNOWN:
            return "it";
          }
        }
        return "that";
      }
      else
      {
        if (pAgentType == SemanticEntityType::HUMAN ||
            pAgentType == SemanticEntityType::AGENTORTHING)
        {
          if (pGender == SemanticGenderType::FEMININE)
            return "she";
          if (pGender == SemanticGenderType::MASCULINE)
            return "he";
        }
        return "it";
      }
      break;
    }
    }
    return "";
  }());
  return PartOfSpeech::PRONOUN;
}


void LinguisticsynthesizergroundingEnglish::writeReflexiveObject(OutSentence&,
                                                                 std::list<WordToSynthesize>& pOut,
                                                                 RelativePerson pSubjectRelativePerson,
                                                                 SemanticGenderType pGender,
                                                                 const mystd::optional<SynthesizerVerbContext>&) const
{
  _writeReflexivePronoun(pOut, pSubjectRelativePerson, pGender);
}


bool LinguisticsynthesizergroundingEnglish::_writeVerbGoal
(OutSentence& pOutSentence,
 SynthesizerCurrentContext& pVerbContext,
 const SemanticStatementGrounding& pStatementGrd,
 const linguistics::InflectedWord& pOutInfoGram,
 const SynthesizerConfiguration& pConf,
 const UniqueSemanticExpression* pSubjectPtr) const
{
  static const PartOfSpeech auxPOF = PartOfSpeech::AUX;

  switch (pStatementGrd.verbGoal)
  {
  case VerbGoalEnum::ABILITY:
  {
    if (pVerbContext.verbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE)
      _strToOut(pOutSentence.aux.out, auxPOF,
                pVerbContext.isPositive ? "could" : "could not");
    else
      _strToOut(pOutSentence.aux.out, auxPOF,
                pVerbContext.isPositive ? "can" : "can't");
    return true;
  }
  case VerbGoalEnum::ADVICE:
  {
    _strToOut(pOutSentence.aux.out, auxPOF,
             pVerbContext.isPositive ? "should" : "shouldn't");
    return true;
  }
  case VerbGoalEnum::CONDITIONAL:
  {
    _strToOut(pOutSentence.aux.out, auxPOF,
             pVerbContext.isPositive ? "would" : "won't");
    return true;
  }
  case VerbGoalEnum::MANDATORY:
  {
    if (pStatementGrd.verbGoal == VerbGoalEnum::MANDATORY && SemExpGetter::hasGenericConcept(pSubjectPtr) &&
        pVerbContext.requests.empty())
      _strToOut(pOutSentence.aux.out, auxPOF,
                pVerbContext.isPositive ? "have to" : "have not to");
    else
      _strToOut(pOutSentence.aux.out, auxPOF,
                pVerbContext.isPositive ? "must" : "must not");
    return true;
  }
  case VerbGoalEnum::POSSIBILITY:
  {
    _strToOut(pOutSentence.aux.out, auxPOF,
             pVerbContext.isPositive ? "might" : "might not");
    return true;
  }
  case VerbGoalEnum::NOTIFICATION:
    break;
  }

  if (pSubjectPtr != nullptr &&
      pOutSentence.verbGoal.out.empty() &&
      !pOutSentence.requests.empty() &&
      !pOutSentence.requests.has(SemanticRequestType::ACTION) &&
      !(pOutSentence.equVerb && ConceptSet::haveAConcept(pStatementGrd.concepts, "verb_equal_be")) &&
      pOutSentence.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC)
  {
    if (pVerbContext.verbTense == LinguisticVerbTense::FUTURE_INDICATIVE)
    {
      _strToOut(pOutSentence.aux.out,  auxPOF, "will");
    }
    else
    {
      LinguisticMeaning lingMeaning;
      SemExpGetter::wordToAMeaning(lingMeaning, pOutInfoGram.word, _language, pConf.lingDb);
      SemanticLanguageEnum meaningLanguage = SemanticLanguageEnum::UNKNOWN;
      if (lingMeaning.getLanguageIfNotEmpty(meaningLanguage))
      {
        const auto& specLingDb = pConf.lingDb.langToSpec[meaningLanguage];
        if (specLingDb.lingDico.hasContextualInfo(WordContextualInfos::BEISTHEAUXILIARY, lingMeaning) ||
            pVerbContext.isPassive)
        {
          return false;
        }
        else
        {

          if (isAPastVerbTense(pVerbContext.verbTense))
          {
            if (pVerbContext.verbTense == LinguisticVerbTense::PRETERIT_CONTINUOUS)
            {
              if (pVerbContext.wordContext.relativePerson == RelativePerson::FIRST_SING ||
                  pVerbContext.wordContext.relativePerson == RelativePerson::THIRD_SING)
                _strToOut(pOutSentence.aux.out,  auxPOF,
                          pVerbContext.isPositive ? "was" : "was not");
              else
                _strToOut(pOutSentence.aux.out,  auxPOF,
                          pVerbContext.isPositive ? "were" : "were not");
            }
            else
            {
              pOutSentence.aux.out.emplace_back(SemanticWord(_language, "do", auxPOF),
                                                InflectionToSynthesize(pVerbContext.isPositive ? "did" : "didn't", true, true, alwaysTrue));
            }
          }
          else
          {
            std::string inflWordStr;
            if (pVerbContext.wordContext.relativePerson == RelativePerson::THIRD_SING)
              inflWordStr = pVerbContext.isPositive ? "does" : "doesn't";
            else
              inflWordStr = pVerbContext.isPositive ? "do" : "don't";
            pOutSentence.aux.out.emplace_back(SemanticWord(_language, "do", auxPOF),
                                              InflectionToSynthesize(inflWordStr, true, true, alwaysTrue));
          }
        }
      }
    }
    return true;
  }
  return false;
}


void LinguisticsynthesizergroundingEnglish::_writeDurationAgo
(std::list<WordToSynthesize>& pOut) const
{
  _strToOut(pOut, PartOfSpeech::ADVERB, "ago");
}

void LinguisticsynthesizergroundingEnglish::_writeDurationIn(std::list<WordToSynthesize>& pOut) const
{
  const std::string word = "in";
  pOut.emplace_front(SemanticWord(SemanticLanguageEnum::FRENCH, word, PartOfSpeech::PREPOSITION),
                     InflectionToSynthesize(word, true, true, alwaysTrue));
}


bool LinguisticsynthesizergroundingEnglish::_dayHourTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary&,
 const SemanticDuration& pDuration,
 bool pDateWritten) const
{
  GroundingDurationPrettyPrintStruct durationPrint(pDuration);
  bool amOrPm = durationPrint.hour <= 12;
  std::stringstream ss;
  if (durationPrint.hour != -1)
  {
    if (pDateWritten)
      ss << "at ";
    if (amOrPm)
      ss << durationPrint.hour;
    else
      ss << durationPrint.hour - 12;
  }
  if (durationPrint.minute != -1)
  {
    if (durationPrint.hour != -1)
      ss << ":";
    ss << durationPrint.minute;
  }
  if (durationPrint.hour != -1)
  {
    if (amOrPm)
      ss << " am";
    else
      ss << " pm";
  }
  const std::string timePrinted = ss.str();
  if (!timePrinted.empty())
  {
    _strToOut(pOut, PartOfSpeech::NOUN, timePrinted);
    return true;
  }
  return false;
}


void LinguisticsynthesizergroundingEnglish::writeReLocationType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeLocationType pLocationType) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, [&]()
  {
    switch (pLocationType)
    {
    case SemanticRelativeLocationType::L_ABOVE:
      return "above";
    case SemanticRelativeLocationType::L_BEHIND:
      return "behind";
    case SemanticRelativeLocationType::L_BELOW:
      return "below";
    case SemanticRelativeLocationType::L_CLOSE:
      return "close to";
    case SemanticRelativeLocationType::L_EAST:
      return "east";
    case SemanticRelativeLocationType::L_INFRONTOF:
      return "in front of";
    case SemanticRelativeLocationType::L_INSIDE:
      return "in";
    case SemanticRelativeLocationType::L_FAR:
      return "far away from";
    case SemanticRelativeLocationType::L_HIGH:
      return "high";
    case SemanticRelativeLocationType::L_LEFT:
      return "left";
    case SemanticRelativeLocationType::L_LOW:
      return "low";
    case SemanticRelativeLocationType::L_NORTH:
      return "north";
    case SemanticRelativeLocationType::L_ON:
      return "on";
    case SemanticRelativeLocationType::L_OUTSIDE:
      return "outside";
    case SemanticRelativeLocationType::L_RIGHT:
      return "right";
    case SemanticRelativeLocationType::L_SOUTH:
      return "south";
    case SemanticRelativeLocationType::L_UNDER:
      return "under";
    case SemanticRelativeLocationType::L_WEST:
      return "west";
    }
    assert(false);
    return "";
  }());
}


void LinguisticsynthesizergroundingEnglish::_writeReTimeType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeTimeType pTimeType,
 LinguisticVerbTense pVerbTense) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, [&]()
  {
    switch (pTimeType)
    {
    case SemanticRelativeTimeType::AFTER:
      return "after";
    case SemanticRelativeTimeType::BEFORE:
      return "before";
    case SemanticRelativeTimeType::JUSTAFTER:
      return "just after";
    case SemanticRelativeTimeType::JUSTBEFORE:
      return "just before";
    case SemanticRelativeTimeType::SINCE:
    {
      if (pVerbTense == LinguisticVerbTense::FUTURE_INDICATIVE)
        return "from";
      return "since";
    }
    }
    assert(false);
    return "";
  }());
}

void LinguisticsynthesizergroundingEnglish::_writeReDurationType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeDurationType pDurationType,
 const GroundedExpression&) const
{
  _strToOut(pOut, PartOfSpeech::PREPOSITION, [&]()
  {
    switch (pDurationType)
    {
    case SemanticRelativeDurationType::DELAYEDSTART:
      return "in";
    case SemanticRelativeDurationType::UNTIL:
      return "until";
    }
    assert(false);
    return "";
  }());
}


void LinguisticsynthesizergroundingEnglish::_getAllWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerCurrentContext&) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, "all");
}

void LinguisticsynthesizergroundingEnglish::_getEveryWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerCurrentContext&) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, "every");
}

void LinguisticsynthesizergroundingEnglish::_getDeterminerThatReferToRecentContext
(std::list<WordToSynthesize>& pOut,
 SemanticNumberType,
 SemanticGenderType) const
{
  _strToOut(pOut, PartOfSpeech::PRONOUN, "this");
}

void LinguisticsynthesizergroundingEnglish::_getDeterminer
(std::list<WordToSynthesize>& pOut,
 const SemanticGenericGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp,
 const SynthesizerCurrentContext& pContext) const
{
  if (pGrounding.coreference)
  {
    _getDeterminerThatReferToRecentContext(pOut, pContext.wordContext.number,
                                           pContext.wordContext.gender);
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::MANY)
  {
    _strToOut(pOut, PartOfSpeech::DETERMINER, "many");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::SOME)
  {
    _strToOut(pOut, PartOfSpeech::DETERMINER, "some");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::MORE)
  {
    _strToOut(pOut, PartOfSpeech::DETERMINER, "more");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::LESS)
  {
    _strToOut(pOut, PartOfSpeech::DETERMINER, "less");
  }
  else if (pGrounding.referenceType == SemanticReferenceType::INDEFINITE)
  {
    if (pContext.wordContext.number == SemanticNumberType::PLURAL)
    {
      if (!pContext.isPositive &&
          !pGrounding.quantity.isPlural())
        _strToOut(pOut, PartOfSpeech::ADJECTIVE, "any");
    }
    else
    {
      pOut.emplace_back(SemanticWord(_language, "a", PartOfSpeech::DETERMINER),
                        InflectionToSynthesize("an", true, true, ifNextCharIsAVowel));
      pOut.back().inflections.emplace_back("a", true, true, alwaysTrue);
    }
  }
  else if (pGrounding.referenceType == SemanticReferenceType::DEFINITE)
  {
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_TIME &&
        pGrounding.word.partOfSpeech == PartOfSpeech::NOUN &&
        pHoldingGrdExp.children.empty())
      _strToOut(pOut, PartOfSpeech::PREPOSITION, "at");
    else
      _strToOut(pOut, PartOfSpeech::DETERMINER, "the");
  }
}


void LinguisticsynthesizergroundingEnglish::_writeReflexivePronoun
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticGenderType pGender) const
{
  switch (pRelativePerson)
  {
  case RelativePerson::FIRST_SING:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "myself");
    break;
  case RelativePerson::SECOND_SING:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "yourself");
    break;
  case RelativePerson::THIRD_SING:
  {
    switch (pGender)
    {
    case SemanticGenderType::FEMININE:
      _strToOut(pOut,  PartOfSpeech::PRONOUN, "herself");
      return;
    case SemanticGenderType::MASCULINE:
      _strToOut(pOut,  PartOfSpeech::PRONOUN, "himself");
      return;
    case SemanticGenderType::NEUTRAL:
    case SemanticGenderType::UNKNOWN:
      _strToOut(pOut, PartOfSpeech::PRONOUN, "itself");
      break;
    }
    break;
  }
  case RelativePerson::FIRST_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "ourselves");
    break;
  case RelativePerson::SECOND_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "yourselves");
    break;
  case RelativePerson::THIRD_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "themselves");
    break;
  case RelativePerson::UNKNOWN:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "itself");
    break;
  }
}


} // End of namespace onsem
