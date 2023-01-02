#include "linguisticsynthesizergroundingfrench.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include "../tool/synthesizeradder.hpp"

namespace onsem
{

LinguisticsynthesizergroundingFrench::LinguisticsynthesizergroundingFrench(const LinguisticSynthesizerPrivate& pLingSynth)
  : Linguisticsynthesizergrounding(SemanticLanguageEnum::FRENCH, pLingSynth),
    _chunksMerger()
{
}


bool LinguisticsynthesizergroundingFrench::_dateTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pDicoSynth,
 const SemanticDate& pDate) const
{
  if (pDate.month)
  {
    if (pDate.day)
    {
      _strToOut(pOut, PartOfSpeech::DETERMINER, "le", WordToSynthesizeTag::DATE);
      {
        std::stringstream ss;
        ss << *pDate.day;
        _strToOut(pOut, PartOfSpeech::NOUN, ss.str(), WordToSynthesizeTag::DATE);
      }
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
  return pDate.month || pDate.year;
}


std::string LinguisticsynthesizergroundingFrench::_usRelativePersonToStr
(SynthesizerCurrentContextType,
 LinguisticVerbTense) const
{
  return "nous";
}


bool LinguisticsynthesizergroundingFrench::_groundingAttributesToWord(linguistics::InflectedWord& pOutInfoGram,
                                                                      const SemanticGenericGrounding& pGrounding) const
{
  if (pGrounding.coreference &&
      pGrounding.referenceType == SemanticReferenceType::DEFINITE &&
      ConceptSet::haveAConcept(pGrounding.concepts, "location"))
  {
    pOutInfoGram.word.setContent(_language, "là", PartOfSpeech::ADVERB);
    return true;
  }
  return false;
}


PartOfSpeech LinguisticsynthesizergroundingFrench::writeRelativePerson
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticReferenceType pReferenceType,
 bool pHasToBeCompletedFromContext,
 SemanticEntityType pAgentType,
 const SemanticQuantity& pQuantity,
 const SynthesizerCurrentContext& pContext,
 const SemanticRequests& pRequests) const
{
  if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB ||
      pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB)
    return _writeRelativePersonObject(pOut, pRelativePerson, pContext.wordContext.gender,
                                      pReferenceType, pHasToBeCompletedFromContext,
                                      pAgentType, pContext.contextType, pRequests);

  switch (pRelativePerson)
  {
  case RelativePerson::FIRST_SING:
  {
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT)
    {
      _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "j'", "je");
      return PartOfSpeech::PRONOUN_SUBJECT;
    }
    if (pRequests.has(SemanticRequestType::ACTION))
    {
      pOut.emplace_back(WordToSynthesize(SemanticWord(_language, "moi", PartOfSpeech::PRONOUN),
                                         InflectionToSynthesize("-moi", false, true, alwaysTrue)));
      return PartOfSpeech::PRONOUN;
    }
    _strToOut(pOut, PartOfSpeech::PRONOUN, "moi");
    return PartOfSpeech::PRONOUN;
  }
  case RelativePerson::SECOND_SING:
  {
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "tu");
      return PartOfSpeech::PRONOUN_SUBJECT;
    }
    _strToOut(pOut, PartOfSpeech::PRONOUN, "toi");
    return PartOfSpeech::PRONOUN;
  }
  case RelativePerson::FIRST_PLUR:
  {
    _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "nous");
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  case RelativePerson::SECOND_PLUR:
  {
    _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "vous");
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  case RelativePerson::THIRD_PLUR:
  {
    if (pQuantity.type == SemanticQuantityType::MAXNUMBER)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN,
                pContext.wordContext.gender == SemanticGenderType::FEMININE ? "toutes" : "tous");
      return PartOfSpeech::PRONOUN;
    }
    else if (pQuantity.type == SemanticQuantityType::EVERYTHING)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN,
                pAgentType == SemanticEntityType::HUMAN ?
                  "tout le monde" : "tout");
      return PartOfSpeech::PRONOUN;
    }
    if (pContext.wordContext.gender == SemanticGenderType::FEMININE)
      _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "elles");
    else
      _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "ils");
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  case RelativePerson::THIRD_SING:
  default:
  {
    if (pQuantity.type == SemanticQuantityType::EVERYTHING)
    {
      if (pHasToBeCompletedFromContext)
        _strToOut(pOut, PartOfSpeech::PRONOUN, "tous");
      else
        _strToOut(pOut, PartOfSpeech::PRONOUN,
                  pAgentType == SemanticEntityType::HUMAN ?
                    "tout le monde" : "tout");
      return PartOfSpeech::PRONOUN;
    }
    else if (pQuantity.type == SemanticQuantityType::ANYTHING)
    {
      if (pAgentType == SemanticEntityType::HUMAN)
        _strToOut(pOut, PartOfSpeech::PRONOUN, "n'importe qui");
      else
        _strToOut(pOut, PartOfSpeech::PRONOUN, "n'importe quoi");
      return PartOfSpeech::PRONOUN;
    }
    else if (pQuantity.type == SemanticQuantityType::MAXNUMBER)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN,
                pContext.wordContext.gender == SemanticGenderType::FEMININE ? "toutes" : "tous");
      return PartOfSpeech::PRONOUN;
    }
    else if (pReferenceType == SemanticReferenceType::INDEFINITE)
    {
      if (pAgentType == SemanticEntityType::HUMAN)
      {
        if (pQuantity.type == SemanticQuantityType::NUMBER &&
            pQuantity.nb == 0)
          _strToOut(pOut, PartOfSpeech::PRONOUN, "personne");
        else
          _strToOut(pOut, PartOfSpeech::PRONOUN, "quelqu'un");
      }
      else
      {
        if (pQuantity.type == SemanticQuantityType::NUMBER &&
            pQuantity.nb == 0)
          _strToOut(pOut, PartOfSpeech::PRONOUN, "rien");
        else
          _strToOut(pOut, PartOfSpeech::PRONOUN, "quelque chose");
      }
      return PartOfSpeech::PRONOUN;
    }
    else if (pRequests.has(SemanticRequestType::ACTION))
    {
      if (pAgentType == SemanticEntityType::HUMAN)
        pOut.emplace_back(WordToSynthesize(SemanticWord(_language, "lui", PartOfSpeech::PRONOUN),
                                           InflectionToSynthesize("-lui", false, true, alwaysTrue)));
      else
        pOut.emplace_back(WordToSynthesize(SemanticWord(_language, "le", PartOfSpeech::PRONOUN),
                                           InflectionToSynthesize("-le", false, true, alwaysTrue)));
    }
    else if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB ||
             pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTAFTERVERB ||
             pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_CONDITION ||
             pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_TIME ||
             pContext.grammaticalTypeFromParent == GrammaticalType::WAY)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN,
               pAgentType == SemanticEntityType::HUMAN ?
                 "lui" : "cela");
      return PartOfSpeech::PRONOUN;
    }
    else
    {
      if (pAgentType == SemanticEntityType::HUMAN ||
          pAgentType == SemanticEntityType::AGENTORTHING)
      {
        switch (pContext.wordContext.gender)
        {
        case SemanticGenderType::MASCULINE:
          _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "il");
          break;
        case SemanticGenderType::FEMININE:
          _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "elle");
          break;
        default:
          _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "c'", "ce");
          break;
        }
        return PartOfSpeech::PRONOUN_SUBJECT;
      }
      if (pContext.currSentence != nullptr &&
          ((!pContext.currSentence->verb.out.empty() &&
            pContext.currSentence->verb.out.front().word.lemma == "être") ||
           (!pContext.currSentence->aux.out.empty() &&
            pContext.currSentence->aux.out.front().word.lemma == "être")))
      {
        _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "c'", "ce");
        return PartOfSpeech::PRONOUN_SUBJECT;
      }
      _strToOut(pOut, PartOfSpeech::PRONOUN, "ça");
      return PartOfSpeech::PRONOUN;
    }
    break;
  }
  }
  return PartOfSpeech::PRONOUN;
}


void LinguisticsynthesizergroundingFrench::rankConceptToInlfWord(
    linguistics::InflectedWord& pInflWord,
    const std::map<std::string, char>& pConcepts) const
{
  std::string numberStr;
  if (ConceptSet::rankConceptToNumberStr(numberStr, pConcepts))
  {
    pInflWord.word.lemma = numberStr + "e";
    pInflWord.word.partOfSpeech = PartOfSpeech::ADJECTIVE;
    pInflWord.word.language = SemanticLanguageEnum::FRENCH;
  }
}


PartOfSpeech LinguisticsynthesizergroundingFrench::_writeRelativePersonObject
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticGenderType pGender,
 SemanticReferenceType pReferenceType,
 bool pHasToBeCompletedFromContext,
 SemanticEntityType pAgentType,
 SynthesizerCurrentContextType pContextType,
 const SemanticRequests& pRequests) const
{
  switch (pRelativePerson)
  {
  case RelativePerson::FIRST_SING:
  {
    _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN, "m'", "me");
    return PartOfSpeech::PRONOUN;
  }
  case RelativePerson::SECOND_SING:
  {
    _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN, "t'", "te");
    return PartOfSpeech::PRONOUN;
  }
  case RelativePerson::FIRST_PLUR:
  {
    _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "nous");
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  case RelativePerson::SECOND_PLUR:
  {
    _strToOut(pOut, PartOfSpeech::PRONOUN_SUBJECT, "vous");
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  case RelativePerson::THIRD_PLUR:
  {
    if (pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB &&
        pAgentType == SemanticEntityType::HUMAN)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN, "leur");
      return PartOfSpeech::PRONOUN;
    }
    if (pRequests.has(SemanticRequestType::ACTION))
    {
      pOut.emplace_back(WordToSynthesize(SemanticWord(_language, "les", PartOfSpeech::PRONOUN),
                                         InflectionToSynthesize("-les", false, true, alwaysTrue)));
      return PartOfSpeech::PRONOUN;
    }
    _strToOut(pOut, PartOfSpeech::PRONOUN, "les");
    return PartOfSpeech::PRONOUN;
  }
  case RelativePerson::THIRD_SING:
  default:
  {
    if (pReferenceType == SemanticReferenceType::INDEFINITE &&
        pHasToBeCompletedFromContext)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN, "en");
    }
    else if (pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB &&
             pAgentType == SemanticEntityType::HUMAN)
    {
      _strToOut(pOut, PartOfSpeech::PRONOUN, "lui");
    }
    else if (pRequests.has(SemanticRequestType::ACTION))
    {
      pOut.emplace_back(WordToSynthesize(SemanticWord(_language, "le", PartOfSpeech::PRONOUN),
                                         InflectionToSynthesize("-le", false, true, alwaysTrue)));
    }
    else if (pGender == SemanticGenderType::FEMININE)
    {
      _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN, "l'", "la");
    }
    else
    {
      _strWithApostropheToOut(pOut, PartOfSpeech::PRONOUN, "l'", "le");
    }
    return PartOfSpeech::PRONOUN;
  }
  }
  return PartOfSpeech::PRONOUN;
}



void LinguisticsynthesizergroundingFrench::writeReflexiveObject(OutSentence& pOutSentence,
                                                                std::list<WordToSynthesize>& pOut,
                                                                RelativePerson pSubjectRelativePerson,
                                                                SemanticGenderType pGender,
                                                                const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt) const
{
  if (pVerbContextOpt)
  {
    if (linguistics::canBeAFrenchReflexiveVerb(pVerbContextOpt->verbInflWord.infos))
    {
      _getSeWord(pOutSentence.objectBeforeVerb.out, pSubjectRelativePerson);
    }
    else
    {
      SemanticRequests requests;
      _writeRelativePersonObject(pOutSentence.objectBeforeVerb.out, pSubjectRelativePerson,
                                 pGender, SemanticReferenceType::DEFINITE, false, SemanticEntityType::UNKNOWN,
                                 SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB, requests);
    }
  }
  else // TODO: maybe delete this else
    _writeReflexivePronoun(pOut, pSubjectRelativePerson, pGender);
}


bool LinguisticsynthesizergroundingFrench::_writeVerbGoal
(OutSentence& pOutSentence,
 SynthesizerCurrentContext& pVerbContext,
 const SemanticStatementGrounding& pStatementGrd,
 const linguistics::InflectedWord&,
 const SynthesizerConfiguration& pConf,
 const UniqueSemanticExpression* pSubjectPtr) const
{
  switch (pStatementGrd.verbGoal)
  {
  case VerbGoalEnum::ABILITY:
  case VerbGoalEnum::ADVICE:
  case VerbGoalEnum::MANDATORY:
  {
    auto canGenGrd = std::make_unique<GroundedExpression>
        ([&pStatementGrd]
    {
      const std::string newStatementConcept = pStatementGrd.verbGoal == VerbGoalEnum::ABILITY ?
            "verb_can" : "verb_haveto";
      auto canStatementGrd = std::make_unique<SemanticStatementGrounding>();
      canStatementGrd->concepts[newStatementConcept] = 4;
      return canStatementGrd;
    }());
    if (pStatementGrd.verbGoal == VerbGoalEnum::ADVICE)
      pVerbContext.verbGoal = VerbGoalEnum::CONDITIONAL;
    const auto& statGrd = (*canGenGrd)->getStatementGrounding();
    OutSentence subOutSentence;
    linguistics::InflectedWord verbInfoGram;
    if (pStatementGrd.verbGoal == VerbGoalEnum::MANDATORY && SemExpGetter::hasGenericConcept(pSubjectPtr))
      verbInfoGram.word = SemanticWord(SemanticLanguageEnum::FRENCH, "falloir", PartOfSpeech::VERB);
    else
      getIGramOfAStatementMeaning(verbInfoGram, statGrd, pConf);
    SynthesizerCurrentContext subContext(pVerbContext);
    subContext.isPassive = false;
    if (subContext.verbGoal == VerbGoalEnum::ABILITY)
      subContext.verbGoal = VerbGoalEnum::NOTIFICATION;
    statGroundingTranslation(subOutSentence, pConf, statGrd,
                             verbInfoGram, *canGenGrd, subContext, nullptr);
    _chunksMerger.formulateSentence(pOutSentence.verbGoal.out, subOutSentence);
    return true;
  }
  case VerbGoalEnum::POSSIBILITY:
  {
    _strToOut(pOutSentence.verbGoal.out, PartOfSpeech::AUX, "peut-être");
    return true;
  }
  case VerbGoalEnum::CONDITIONAL:
  case VerbGoalEnum::NOTIFICATION:
    break;
  };
  return false;
}


void LinguisticsynthesizergroundingFrench::_writeDurationAgo
(std::list<WordToSynthesize>& pOut) const
{
  const std::string word = "il y a";
  pOut.emplace_front(SemanticWord(SemanticLanguageEnum::FRENCH, word, PartOfSpeech::ADVERB),
                     InflectionToSynthesize(word, true, true, alwaysTrue));
}

void LinguisticsynthesizergroundingFrench::_writeDurationIn(std::list<WordToSynthesize>& pOut) const
{
  const std::string word = "dans";
  pOut.emplace_front(SemanticWord(SemanticLanguageEnum::FRENCH, word, PartOfSpeech::PREPOSITION),
                     InflectionToSynthesize(word, true, true, alwaysTrue));
}


bool LinguisticsynthesizergroundingFrench::_dayHourTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pStatSynthDico,
 const SemanticDuration& pDuration,
 bool pDateWritten) const
{
  GroundingDurationPrettyPrintStruct durationPrint(pDuration);
  std::stringstream ss;
  if (durationPrint.hour)
  {
    if (pDateWritten)
      ss << "à ";
    ss << durationPrint.hour->toStr(_language);
    const auto& meaning = pStatSynthDico.conceptToMeaning("duration_hour");
    if (!meaning.isEmpty())
    {
      std::string hourWord;
      SemanticGenderType gender = SemanticGenderType::UNKNOWN;
      SemanticNumberType number = *durationPrint.hour > 1 ? SemanticNumberType::PLURAL : SemanticNumberType::SINGULAR;
      pStatSynthDico.getNounForm(hourWord, meaning, gender, number);
      ss << " " << hourWord;
    }
  }
  if (durationPrint.minute)
  {
    if (durationPrint.hour)
      ss << " ";
    ss << durationPrint.minute->toStr(_language);
  }

  const std::string timePrinted = ss.str();
  if (!timePrinted.empty())
  {
    _strToOut(pOut, PartOfSpeech::NOUN, timePrinted);
    return true;
  }
  return false;
}

void LinguisticsynthesizergroundingFrench::writeReLocationType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeLocationType pLocationType) const
{
  static const PartOfSpeech perp = PartOfSpeech::PREPOSITION;
  switch (pLocationType)
  {
  case SemanticRelativeLocationType::L_ABOVE:
    _strWithApostropheToOut(pOut, perp, "au dessus d'", "au dessus de");
    return;
  case SemanticRelativeLocationType::L_BEHIND:
    _strToOut(pOut, perp, "derrère");
    return;
  case SemanticRelativeLocationType::L_BELOW:
    _strWithApostropheToOut(pOut, perp, "en dessous d'", "en dessous de");
    return;
  case SemanticRelativeLocationType::L_CLOSE:
    _strWithApostropheToOut(pOut, perp, "près d'", "près de");
    return;
  case SemanticRelativeLocationType::L_EAST:
    _strToOut(pOut, perp, "est");
    return;
  case SemanticRelativeLocationType::L_INFRONTOF:
    _strToOut(pOut, perp, "devant");
    return;
  case SemanticRelativeLocationType::L_INSIDE:
    _strToOut(pOut, perp, "dans");
    return;
  case SemanticRelativeLocationType::L_FAR:
    _strWithApostropheToOut(pOut, perp, "loin d'", "loin de");
    return;
  case SemanticRelativeLocationType::L_HIGH:
    _strToOut(pOut, perp, "haut");
    return;
  case SemanticRelativeLocationType::L_LEFT:
    _strToOut(pOut, perp, "gauche");
    return;
  case SemanticRelativeLocationType::L_LOW:
    _strToOut(pOut, perp, "bas");
    return;
  case SemanticRelativeLocationType::L_NORTH:
    _strToOut(pOut, perp, "nord");
    return;
  case SemanticRelativeLocationType::L_ON:
    _strToOut(pOut, perp, "sur");
    return;
  case SemanticRelativeLocationType::L_OUTSIDE:
    _strToOut(pOut, perp, "extérieur");
    return;
  case SemanticRelativeLocationType::L_RIGHT:
    _strToOut(pOut, perp, "droit");
    return;
  case SemanticRelativeLocationType::L_SOUTH:
    _strToOut(pOut, perp, "sud");
    return;
  case SemanticRelativeLocationType::L_UNDER:
    _strToOut(pOut, perp, "sous");
    return;
  case SemanticRelativeLocationType::L_WEST:
    _strToOut(pOut, perp, "ouest");
    return;
  }
  assert(false);
}


void LinguisticsynthesizergroundingFrench::_writeReTimeType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeTimeType pTimeType,
 LinguisticVerbTense pVerbTense) const
{
  static const PartOfSpeech perp = PartOfSpeech::PREPOSITION;
  switch (pTimeType)
  {
  case SemanticRelativeTimeType::AFTER:
    _strToOut(pOut, perp, "après");
    return;
  case SemanticRelativeTimeType::BEFORE:
    _strToOut(pOut, perp, "avant");
    return;
  case SemanticRelativeTimeType::JUSTAFTER:
    _strToOut(pOut, perp, "juste après");
    return;
  case SemanticRelativeTimeType::JUSTBEFORE:
    _strToOut(pOut, perp, "juste avant");
    return;
  case SemanticRelativeTimeType::SINCE:
  {
    if (pVerbTense == LinguisticVerbTense::FUTURE_INDICATIVE)
      _strToOut(pOut, perp, "dès");
    else
      _strToOut(pOut, perp, "depuis");
    return;
  }
  }
  assert(false);
}


void LinguisticsynthesizergroundingFrench::_writeReDurationType
(std::list<WordToSynthesize>& pOut,
 SemanticRelativeDurationType pDurationType,
 const GroundedExpression& pHoldingGrdExp) const
{
  static const PartOfSpeech perp = PartOfSpeech::PREPOSITION;
  switch (pDurationType)
  {
  case SemanticRelativeDurationType::DELAYEDSTART:
  {
    _strToOut(pOut, perp, "dans");
    return;
  }
  case SemanticRelativeDurationType::UNTIL:
  {
    auto itSpec = pHoldingGrdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpec != pHoldingGrdExp.children.end() &&
        SemExpGetter::semExphasAStatementGrd(itSpec->second.getSemExp()))
      _strWithApostropheToOut(pOut, perp, "jusqu'à ce qu'", "jusqu'à ce que");
    else
      _strToOut(pOut, perp, "jusqu'à");
    return;
  }
  }
  assert(false);
}


void LinguisticsynthesizergroundingFrench::_getSeWord
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson) const
{
  static const PartOfSpeech pronComp = PartOfSpeech::PRONOUN_COMPLEMENT;
  switch (pRelativePerson)
  {
  case RelativePerson::FIRST_SING:
    _strWithApostropheToOut(pOut, pronComp, "m'", "me");
    break;
  case RelativePerson::SECOND_SING:
    _strWithApostropheToOut(pOut, pronComp, "t'", "te");
    break;
  case RelativePerson::FIRST_PLUR:
    _strToOut(pOut, pronComp, "nous");
    break;
  case RelativePerson::SECOND_PLUR:
    _strToOut(pOut, pronComp, "vous");
    break;
  case RelativePerson::THIRD_SING:
  case RelativePerson::THIRD_PLUR:
  default:
    _strWithApostropheToOut(pOut, pronComp, "s'", "se");
    break;
  }
}


void LinguisticsynthesizergroundingFrench::_getPronounComplement
(std::list<WordToSynthesize>& pOut,
 const StaticLinguisticMeaning& pMeaning,
 const linguistics::SpecificLinguisticDatabase& pSpecLingDb,
 const SynthesizerCurrentContext& pContext) const
{
  static const PartOfSpeech pronComp = PartOfSpeech::PRONOUN_COMPLEMENT;
  if (pMeaning == pSpecLingDb.lingDico.statDb.getLingMeaning("se", pronComp, false))
  {
    if (pContext.isPositive && pContext.verbTense == LinguisticVerbTense::PRESENT_IMPERATIVE)
    {
      if (pContext.wordContext.relativePerson == RelativePerson::FIRST_PLUR)
        pOut.emplace_back(SemanticWord(_language, "nous", pronComp),
                          InflectionToSynthesize("-nous", false, true, alwaysTrue));
      else if (pContext.wordContext.relativePerson == RelativePerson::SECOND_PLUR)
        pOut.emplace_back(SemanticWord(_language, "vous", pronComp),
                          InflectionToSynthesize("-vous", false, true, alwaysTrue));
      else
        pOut.emplace_back(SemanticWord(_language, "toi", pronComp),
                          InflectionToSynthesize("-toi", false, true, alwaysTrue));
    }
    else
    {
      _getSeWord(pOut, pContext.wordContext.relativePerson);
    }
  }
  else
  {
    _strToOut(pOut, pronComp, pSpecLingDb.synthDico.statDb.getLemma(pMeaning, false));
  }
}


void LinguisticsynthesizergroundingFrench::_getAllWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerCurrentContext& pContext) const
{
  _strToOut(pOut, PartOfSpeech::DETERMINER, pContext.wordContext.gender == SemanticGenderType::FEMININE ? "toutes" : "tous");
}

void LinguisticsynthesizergroundingFrench::_getEveryWord
(std::list<WordToSynthesize>& pOut,
 const SynthesizerCurrentContext& pContext) const
{
  _getAllWord(pOut, pContext);
}


void LinguisticsynthesizergroundingFrench::_getDeterminerThatReferToRecentContext
(std::list<WordToSynthesize>& pOut,
 SemanticNumberType pNumber,
 SemanticGenderType pGender) const
{
  static const PartOfSpeech det = PartOfSpeech::DETERMINER;
  if (pNumber == SemanticNumberType::SINGULAR)
  {
    if (pGender == SemanticGenderType::FEMININE)
    {
      _strToOut(pOut, det, "cette");
    }
    else
    {
      pOut.emplace_back(SemanticWord(_language, "ce", det),
                        InflectionToSynthesize("cet", true, true, ifNextCharIsAVowel));
      pOut.back().inflections.emplace_back("ce", true, true, alwaysTrue);
    }
  }
  else
  {
    _strToOut(pOut, det, "ces");
  }
}


void LinguisticsynthesizergroundingFrench::_getDeterminer
(std::list<WordToSynthesize>& pOut,
 const SemanticGenericGrounding& pGrounding,
 const GroundedExpression& pGrdExp,
 const SynthesizerCurrentContext& pContext) const
{
  static const PartOfSpeech det = PartOfSpeech::DETERMINER;
  if (pGrounding.coreference)
  {
    _getDeterminerThatReferToRecentContext(pOut, pContext.wordContext.number,
                                           pContext.wordContext.gender);
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::MANY)
  {
    _strWithApostropheToOut(pOut, det, "beaucoup d'", "beaucoup de");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::SOME)
  {
    if (pContext.wordContext.gender == SemanticGenderType::FEMININE)
      _strToOut(pOut, det, "certaines");
    else
      _strToOut(pOut, det, "certains");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::MORE)
  {
    _strWithApostropheToOut(pOut, det, "plus d'", "plus de");
  }
  else if (pGrounding.quantity.subjectiveValue == SemanticSubjectiveQuantity::LESS)
  {
    _strWithApostropheToOut(pOut, det, "moins d'", "moins de");
  }
  else if (pContext.wordContext.referenceType == SemanticReferenceType::INDEFINITE)
  {
    if (pContext.wordContext.number == SemanticNumberType::PLURAL)
    {
      if (pGrounding.quantity.type == SemanticQuantityType::NUMBER)
      {
        _strWithApostropheToOut(pOut, det, "d'", "de");
      }
      else if (pGrounding.quantity.type == SemanticQuantityType::MOREOREQUALTHANNUMBER)
      {
        _strToOut(pOut, det, "des");
      }
      else
      {
        if (pContext.isPositive)
          _strToOut(pOut, det, "des");
        else
          _strWithApostropheToOut(pOut, det, "d'", "de");
      }
    }
    else
      _strToOut(pOut, det, pContext.wordContext.gender == SemanticGenderType::FEMININE ?
                  "une" : "un");
  }
  else if (pContext.wordContext.referenceType == SemanticReferenceType::UNDEFINED)
  {
    if ((pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB ||
         ((pContext.requests.has(SemanticRequestType::QUANTITY) || pContext.requests.has(SemanticRequestType::LENGTH) ||
           pContext.requests.has(SemanticRequestType::DURATION)) &&
          pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB)) &&
        pContext.grammaticalTypeFromParent != GrammaticalType::IN_CASE_OF &&
        pGrounding.entityType != SemanticEntityType::MODIFIER &&
        pGrounding.quantity.type != SemanticQuantityType::NUMBER &&
        !SemExpGetter::hasAChildModifier(pGrdExp) &&
        pContext.grammaticalTypeFromParent != GrammaticalType::LOCATION &&
        (pOut.empty() || pOut.front().word.lemma != "de"))
    {
      if (pContext.wordContext.number == SemanticNumberType::PLURAL &&
          !pContext.requests.has(SemanticRequestType::QUANTITY) &&
          !pContext.requests.has(semanticRequestType_fromSemGram(pContext.grammaticalTypeFromParent)))
        _strToOut(pOut, det, "plusieurs");
      else
        _strWithApostropheToOut(pOut, det, "d'", "de");
    }
  }
  else if (pContext.wordContext.referenceType == SemanticReferenceType::DEFINITE)
  {
    if (pGrounding.entityType != SemanticEntityType::MODIFIER &&
        pGrounding.quantity.type != SemanticQuantityType::NUMBER &&
        pGrounding.quantity.type != SemanticQuantityType::MOREOREQUALTHANNUMBER &&
        pGrounding.quantity.type != SemanticQuantityType::MAXNUMBER &&
        pContext.contextType != SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC &&
        pContext.verbContextOpt &&
        pContext.verbContextOpt->verbInflWord.infos.hasContextualInfo(WordContextualInfos::FR_VERBFOLLOWEDBYWORDDE))
    {
      _strWithApostropheToOut(pOut, det, "d'", "de");
    }
    if (pContext.wordContext.number == SemanticNumberType::PLURAL)
    {
      _strToOut(pOut, det, "les");
    }
    else
    {
      if (pContext.wordContext.gender == SemanticGenderType::FEMININE)
        _strWithApostropheToOut(pOut, det, "l'", "la");
      else if (pOut.empty() || pOut.back().word.lemma != "au")
        _strWithApostropheToOut(pOut, det, "l'", "le");
    }
  }
}


void LinguisticsynthesizergroundingFrench::_writeReflexivePronoun
(std::list<WordToSynthesize>& pOut,
 RelativePerson pRelativePerson,
 SemanticGenderType pGender) const
{
  switch (pRelativePerson)
  {
  case RelativePerson::FIRST_SING:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "moi-même");
    break;
  case RelativePerson::SECOND_SING:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "toi-même");
    break;
  case RelativePerson::THIRD_SING:
    _strToOut(pOut, PartOfSpeech::PRONOUN,
             pGender == SemanticGenderType::FEMININE ? "elle-même" : "lui-même");
    break;
  case RelativePerson::FIRST_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "nous-mêmes");
    break;
  case RelativePerson::SECOND_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN, "vous-mêmes");
    break;
  case RelativePerson::THIRD_PLUR:
    _strToOut(pOut, PartOfSpeech::PRONOUN,
             pGender == SemanticGenderType::FEMININE ? "elles-mêmes" : "eux-mêmes");
    break;
  case RelativePerson::UNKNOWN:
    _strToOut(pOut, PartOfSpeech::PRONOUN,
             pGender == SemanticGenderType::FEMININE ? "elle-même" : "lui-même");
    break;
  }
}


bool LinguisticsynthesizergroundingFrench::_doWeHaveToPutSubMeaningBeforeOrAfterTheWord
(const SynthesizerCurrentContext& pContext,
 LinkedMeaningDirection pDirection) const
{
  switch (pDirection)
  {
  case LinkedMeaningDirection::FORWARD:
    return false;
  case LinkedMeaningDirection::BACKWARD:
    return true;
  case LinkedMeaningDirection::BOTH:
    return !pContext.isPositive || pContext.verbTense != LinguisticVerbTense::PRESENT_IMPERATIVE;
  }
  return false;
}



} // End of namespace onsem
