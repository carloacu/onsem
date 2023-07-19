#include "linguisticsynthesizergrounding.hpp"
#include <sstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../merger/synthesizerchunksmerger.hpp"
#include "../tool/synthesizergetter.hpp"
#include "../linguisticsynthesizerprivate.hpp"

namespace onsem
{
namespace
{
void _printValue(std::stringstream& pSs,
                 SemanticGenderType& pGlobalGender,
                 const SemanticFloat& pValue,
                 const std::string& pConceptName,
                 const linguistics::SynthesizerDictionary& pStatSynthDico,
                 SemanticLanguageEnum pLanguage,
                 GrammaticalType pParentGrammaticalType)
{
  const auto& meaning = pStatSynthDico.conceptToMeaning(pConceptName);
  if (!meaning.isEmpty())
  {
    if (pParentGrammaticalType == GrammaticalType::INTERVAL && pLanguage == SemanticLanguageEnum::FRENCH)
      pSs << "les ";
    if (pParentGrammaticalType != GrammaticalType::INTERVAL || pValue != 1)
      pSs << pValue.toStr(pLanguage) << " ";

    bool forceInPlural = pParentGrammaticalType == GrammaticalType::INTERVAL && pLanguage == SemanticLanguageEnum::FRENCH;
    std::string word;
    SemanticNumberType number = forceInPlural ?
          SemanticNumberType::PLURAL : (pValue > 1 ? SemanticNumberType::PLURAL : SemanticNumberType::SINGULAR);
    SemanticGenderType gender = SemanticGenderType::UNKNOWN;
    pStatSynthDico.getNounForm(word, meaning, gender, number);
    pSs << word;
    if (pGlobalGender != SemanticGenderType::MASCULINE)
      pGlobalGender = gender;
  }
}

void _printAngleValue(std::stringstream& pSs,
                      const SemanticFloat& pValue,
                      SemanticAngleUnity pAngle,
                      const linguistics::SynthesizerDictionary& pStatSynthDico,
                      SemanticLanguageEnum pLanguage)
{
  SemanticGenderType globalGender = SemanticGenderType::UNKNOWN;
  GrammaticalType parentGrammaticalType = GrammaticalType::UNKNOWN;
  _printValue(pSs, globalGender, pValue, semanticAngleUnity_toConcept(pAngle),
              pStatSynthDico, pLanguage, parentGrammaticalType);
}

void _printLengthValue(std::stringstream& pSs,
                       const SemanticFloat& pValue,
                       SemanticLengthUnity pLength,
                       const linguistics::SynthesizerDictionary& pStatSynthDico,
                       SemanticLanguageEnum pLanguage)
{
  SemanticGenderType globalGender = SemanticGenderType::UNKNOWN;
  GrammaticalType parentGrammaticalType = GrammaticalType::UNKNOWN;
  _printValue(pSs, globalGender, pValue, semanticLengthUnity_toConcept(pLength),
              pStatSynthDico, pLanguage, parentGrammaticalType);
}

void _printTimeValue(std::stringstream& pSs,
                     SemanticGenderType& pGlobalGender,
                     const SemanticFloat& pValue,
                     SemanticTimeUnity pTime,
                     const linguistics::SynthesizerDictionary& pStatSynthDico,
                     SemanticLanguageEnum pLanguage,
                     GrammaticalType pParentGrammaticalType)
{
  _printValue(pSs, pGlobalGender, pValue, semanticTimeUnity_toConcept(pTime),
              pStatSynthDico, pLanguage, pParentGrammaticalType);
}
}


void Linguisticsynthesizergrounding::writeListSeparators
(std::list<WordToSynthesize>& pOut,
 ListExpressionType pListType,
 bool pBeforeLastElt) const
{
  if (pBeforeLastElt) // if we are before the last element
  {
    switch (pListType)
    {
    case ListExpressionType::AND:
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                _language == SemanticLanguageEnum::FRENCH ? "et" : "and");
      break;
    case ListExpressionType::OR:
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                _language == SemanticLanguageEnum::FRENCH ? "ou" : "or");
      break;
    case ListExpressionType::THEN:
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                _language == SemanticLanguageEnum::FRENCH ? "et puis" : "and then");
      break;
    case ListExpressionType::THEN_REVERSED:
      _strToOut(pOut, PartOfSpeech::CONJUNCTIVE,
                _language == SemanticLanguageEnum::FRENCH ? "et avant" : "and before");
      break;
    default:
      break;
    };
  }
  else
  {
    auto listSeparator = [&]
    {
      if (pListType == ListExpressionType::THEN)
        return _language == SemanticLanguageEnum::FRENCH ? ", puis" : ", then";
      if (pListType == ListExpressionType::THEN_REVERSED)
        return _language == SemanticLanguageEnum::FRENCH ? ", avant" : ", before";
      return ",";
    }();

    pOut.emplace_back(SemanticWord(_language, listSeparator, PartOfSpeech::LINKBETWEENWORDS),
                      InflectionToSynthesize(listSeparator, false, true, alwaysTrue));
  }
}


void Linguisticsynthesizergrounding::modifyContextForAGrounding
(SynthesizerWordContext& pWordContext,
 linguistics::InflectedWord& pOutInfoGram,
 const SynthesizerConfiguration& pConf,
 const SemanticGrounding& pGrounding,
 SynthesizerCurrentContextType pContextType,
 LinguisticVerbTense pVerbTense) const
{
  switch (pGrounding.type)
  {
  case SemanticGroundingType::GENERIC:
    _modifyContextForAGenGrounding(pWordContext, pOutInfoGram, pConf,
                                   pGrounding.getGenericGrounding());
    break;
  case SemanticGroundingType::CONCEPTUAL:
  {
    const SemanticConceptualGrounding& cptGrd = pGrounding.getConceptualGrounding();
    const auto& specLingDb = pConf.lingDb.langToSpec[_language];
    LinguisticMeaning lingMeaning;
    if (ConceptSet::haveAConcept(cptGrd.concepts, "tolink_1p"))
    {
      pOutInfoGram.word.language = _language;
      if (pContextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT)
        pOutInfoGram.word.partOfSpeech = PartOfSpeech::PRONOUN_SUBJECT;
      else
        pOutInfoGram.word.partOfSpeech = PartOfSpeech::PRONOUN;
      pOutInfoGram.word.lemma = _usRelativePersonToStr(pContextType, pVerbTense);
    }
    else
    {
      synthGetter::fillLingMeaningFromConcepts(lingMeaning, cptGrd.concepts, specLingDb.synthDico);
      specLingDb.lingDico.getInfoGram(pOutInfoGram, lingMeaning);
    }
    break;
  }
  case SemanticGroundingType::AGENT:
  {
    const SemanticAgentGrounding& agentGrd = pGrounding.getAgentGrounding();
    pWordContext.gender = pConf.memBlock.getGender(agentGrd.userId);
    break;
  }
  case SemanticGroundingType::NAME:
  {
    const SemanticNameGrounding& nameGrd = pGrounding.getNameGrounding();
    std::set<SemanticGenderType> meaningPossGenders;
    pWordContext.gender = synthGetter::getGender(pWordContext.gender, meaningPossGenders,
                                                 nameGrd.nameInfos.possibleGenders);
    pOutInfoGram.word.language = _language;
    pOutInfoGram.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
    bool firstIteration = true;
    for (const auto& currName : nameGrd.nameInfos.names)
    {
      if (firstIteration)
      {
        firstIteration = false;
        pOutInfoGram.word.lemma = currName;
      }
      else
      {
        pOutInfoGram.word.lemma += " " + currName;
      }
    }
    break;
  }
  case SemanticGroundingType::STATEMENT:
  {
    pOutInfoGram.word.partOfSpeech = PartOfSpeech::VERB;
    break;
  }
  default:
    break;
  }
  if (pOutInfoGram.word.isEmpty() &&
      ConceptSet::haveAConceptThatBeginWith(pGrounding.concepts, "rank_"))
    rankConceptToInlfWord(pOutInfoGram, pGrounding.concepts);
}


void Linguisticsynthesizergrounding::writeGroundingIntroduction
(OutSemExp& pBeforeOut,
 const OutSemExp& pOut,
 const linguistics::InflectedWord& pOutInfoGram,
 SynthesizerCurrentContext& pContext,
 SynthesizerConfiguration& pConf,
 const SemanticGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp) const
{
  switch (pGrounding.type)
  {
  case SemanticGroundingType::GENERIC:
  {
    if (pContext.contextType != SynthesizerCurrentContextType::SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB)
      writePreposition(pBeforeOut.out, &pOut, pOutInfoGram, pContext, pConf, pGrounding, pHoldingGrdExp);
    const auto& genGrd = pGrounding.getGenericGrounding();
    beforeGenGroundingTranslation(pBeforeOut, pOutInfoGram, pContext, pConf, genGrd, pHoldingGrdExp);
    break;
  }
  case SemanticGroundingType::RELATIVELOCATION:
  {
    writeReLocationType(pBeforeOut.out, pGrounding.getRelLocationGrounding().locationType);
    break;
  }
  case SemanticGroundingType::RELATIVETIME:
  {
    _writeReTimeType(pBeforeOut.out, pGrounding.getRelTimeGrounding().timeType, pContext.verbTense);
    break;
  }
  case SemanticGroundingType::RELATIVEDURATION:
  {
    _writeReDurationType(pBeforeOut.out, pGrounding.getRelDurationGrounding().durationType, pHoldingGrdExp);
    break;
  }
  default:
  {
    if (!ConceptSet::haveAConcept(pGrounding.concepts, "tolink_1p"))
      writePreposition(pBeforeOut.out, &pOut, pOutInfoGram, pContext, pConf, pGrounding, pHoldingGrdExp);
    break;
  }
  }
}


void Linguisticsynthesizergrounding::writeGrounding
(OutSemExp& pOutSemExp,
 const linguistics::InflectedWord& pOutInfoGram,
 SynthesizerCurrentContext& pContext,
 SynthesizerConfiguration& pConf,
 const SemanticGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp,
 const SemanticRequests& pRequests) const
{
  switch (pGrounding.type)
  {
  case SemanticGroundingType::GENERIC:
  {
    const auto& genGrd = pGrounding.getGenericGrounding();
    genGroundingTranslation(pOutSemExp, pOutInfoGram, pContext, pConf.lingDb, genGrd);
    break;
  }
  case SemanticGroundingType::STATEMENT:
  {
    const SemanticStatementGrounding& statementGrd =
        pGrounding.getStatementGrounding();
    OutSentence outSentence;
    linguistics::InflectedWord verbInfoGram;
    getIGramOfAStatementMeaning(verbInfoGram, statementGrd, pConf);
    statGroundingTranslation(outSentence, pConf, statementGrd, verbInfoGram,
                             pHoldingGrdExp, pContext, nullptr);
    getChunksMerger().formulateSentence(pOutSemExp.out, outSentence);
    pOutSemExp.partOfSpeech = PartOfSpeech::VERB;
    break;
  }
  case SemanticGroundingType::AGENT:
  {
    agentGroundingTranslation(pOutSemExp, pConf,
                              pGrounding.getAgentGrounding(),
                              pContext, pRequests);
    break;
  }
  case SemanticGroundingType::ANGLE:
  {
    const auto& synthDico = pConf.lingDb.langToSpec[_language].synthDico;
    angleTranslation(pOutSemExp.out, synthDico,
                     pGrounding.getAngleGrounding().angle, true);
    break;
  }
  case SemanticGroundingType::TIME:
  {
    timeGroundingTranslation(pOutSemExp.out, pConf.lingDb, pGrounding.getTimeGrounding());
    break;
  }
  case SemanticGroundingType::TEXT:
  {
    textGroundingTranslation(pOutSemExp.out, pGrounding.getTextGrounding(),
                             pContext);
    break;
  }
  case SemanticGroundingType::DURATION:
  {
    const auto& synthDico = pConf.lingDb.langToSpec[_language].synthDico;
    durationTranslation(pOutSemExp.out, synthDico,
                        pGrounding.getDurationGrounding().duration,
                        true, pContext.grammaticalTypeFromParent);
    break;
  }
  case SemanticGroundingType::LANGUAGE:
  {
    langGroundingTranslation(pOutSemExp.out, pGrounding.getLanguageGrounding());
    break;
  }
  case SemanticGroundingType::RESOURCE:
  {
    resourceGroundingTranslation(pOutSemExp.out, pGrounding.getResourceGrounding());
    break;
  }
  case SemanticGroundingType::LENGTH:
  {
    const auto& synthDico = pConf.lingDb.langToSpec[_language].synthDico;
    lengthTranslation(pOutSemExp.out, synthDico,
                      pGrounding.getLengthGrounding().length, true);
    break;
  }
  case SemanticGroundingType::META:
  {
    metaGroundingTranslation(pOutSemExp.out, pGrounding.getMetaGrounding());
    break;
  }
  case SemanticGroundingType::UNITY:
  {
    const auto& synthDico = pConf.lingDb.langToSpec[_language].synthDico;
    unityGroundingTranslation(pOutSemExp.out, pGrounding.getUnityGrounding(), synthDico,
                              pContext.grammaticalTypeFromParent, pRequests);
    break;
  }
  case SemanticGroundingType::NAME:
  {
    if (!pOutInfoGram.word.lemma.empty())
      _strToOut(pOutSemExp.out, pOutInfoGram.word.partOfSpeech, pOutInfoGram.word.lemma);
    break;
  }
  case SemanticGroundingType::PERCENTAGE:
  {
    const auto& grd = pGrounding.getPercentageGrounding();
    std::stringstream ss;
    ss << grd.value.toStr(_language);
    const auto& synthDico = pConf.lingDb.langToSpec[_language].synthDico;
    const auto& meaning = synthDico.statDb.conceptToMeaning("percent");
    if (!meaning.isEmpty())
    {
      std::string word;
      SemanticNumberType number = grd.value > 1 ? SemanticNumberType::PLURAL : SemanticNumberType::SINGULAR;
      SemanticGenderType gender = SemanticGenderType::UNKNOWN;
      synthDico.statDb.getNounForm(word, meaning, gender, number);
      ss << " " << word;
    }
    const std::string percentagePrinted = ss.str();
    if (!percentagePrinted.empty())
      _strToOut(pOutSemExp.out, PartOfSpeech::NOUN, percentagePrinted);
    break;
  }
  case SemanticGroundingType::CONCEPTUAL:
  {
    if (ConceptSet::haveAConcept(pGrounding.concepts, "reflexive"))
    {
      if (pContext.currSentence != nullptr)
        writeReflexiveObject(*pContext.currSentence, pOutSemExp.out, pContext.currSentence->subject.relativePerson,
                             pContext.currSentence->subject.gender, pContext.verbContextOpt);
      else
        _writeReflexivePronoun(pOutSemExp.out, pContext.wordContext.relativePerson, pContext.wordContext.gender);
    }
    else
    {
      if (pOutInfoGram.infos.linkedMeanings)
        _writeSubMeanings(pOutSemExp.out, pConf.lingDb, *pOutInfoGram.infos.linkedMeanings, pContext, true);

      writeSemWord(pOutSemExp.out, pOutSemExp, pOutSemExp.out,
                   pOutInfoGram.word, pConf.lingDb, pContext);

      if (pOutInfoGram.infos.linkedMeanings)
        _writeSubMeanings(pOutSemExp.out, pConf.lingDb, *pOutInfoGram.infos.linkedMeanings, pContext, false);
    }
    break;
  }
  case SemanticGroundingType::RELATIVELOCATION:
  case SemanticGroundingType::RELATIVETIME:
  case SemanticGroundingType::RELATIVEDURATION:
    break;
  }
}



void Linguisticsynthesizergrounding::langGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const SemanticLanguageGrounding& pGrounding) const
{
  _strToOutIfNotEmpty(pOut, PartOfSpeech::ADJECTIVE,
                     sayLanguageInLanguage(pGrounding.language, _language));
}


void Linguisticsynthesizergrounding::textGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const SemanticTextGrounding& pGrounding,
 const SynthesizerCurrentContext& pContext) const
{
  if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB &&
      (pGrounding.text.find("<br />") != std::string::npos || pGrounding.text.find('\n') != std::string::npos))
  {
    std::stringstream ss;
    ss << ":\n" << pGrounding.text;
    _strToOut(pOut, PartOfSpeech::UNKNOWN, ss.str());
    pOut.back().inflections.front().canHavePunctionAfter = false;
    return;
  }
  if (pGrounding.hasQuotationMark || pGrounding.text.empty())
  {
    _strToOut(pOut, PartOfSpeech::UNKNOWN, "\"" + pGrounding.text + "\"");
  }
  else
  {
    _strToOut(pOut, PartOfSpeech::UNKNOWN, pGrounding.text);
    InflectionToSynthesize& inflToSynt = pOut.back().inflections.front();
    inflToSynt.canHavePunctionAfter = false;
  }
}


void Linguisticsynthesizergrounding::nameInfosTranslation
(std::list<WordToSynthesize>& pOut,
 const NameInfos& pNameInfos) const
{
  for (const auto& currName : pNameInfos.names)
    _strToOut(pOut, PartOfSpeech::PROPER_NOUN, currName);
}

void Linguisticsynthesizergrounding::resourceGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const SemanticResourceGrounding& pGrounding) const
{
  _strToOut(pOut, PartOfSpeech::BOOKMARK, pGrounding.resource.value);
  InflectionToSynthesize& inflToSynt = pOut.back().inflections.front();
  inflToSynt.canHavePunctionAfter = false;
  inflToSynt.fromResourcePtr = &pGrounding;
}

void Linguisticsynthesizergrounding::unityGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const SemanticUnityGrounding& pGrounding,
 const linguistics::SynthesizerDictionary& pStatSynthDico,
 GrammaticalType pGrammTypeFromParent,
 const SemanticRequests& pRequests) const
{
  const auto& meaning = pStatSynthDico.conceptToMeaning(pGrounding.getValueConcept());
  if (!meaning.isEmpty())
  {
    SemanticNumberType number = SemanticNumberType::SINGULAR;
    if (pGrammTypeFromParent == GrammaticalType::UNITY ||
        pGrammTypeFromParent == GrammaticalType::DURATION ||
        pRequests.has(SemanticRequestType::QUANTITY))
    {
      number = pGrounding.typeOfUnity == TypeOfUnity::PERCENTAGE ?
            SemanticNumberType::SINGULAR : SemanticNumberType::PLURAL;
    }

    std::string word;
    SemanticGenderType gender = SemanticGenderType::UNKNOWN;
    pStatSynthDico.getNounForm(word, meaning, gender, number);
    _strToOut(pOut, PartOfSpeech::NOUN, word);
  }
}

void Linguisticsynthesizergrounding::metaGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const SemanticMetaGrounding& pGrounding) const
{
  if (pGrounding.refToType == SemanticGroundingType::AGENT)
  {
    if (_language == SemanticLanguageEnum::FRENCH)
      _strToOut(pOut, PartOfSpeech::PRONOUN, "quelqu'un");
    else
      _strToOut(pOut, PartOfSpeech::PRONOUN, "somebody");
  }
  else
  {
    if (_language == SemanticLanguageEnum::FRENCH)
      _strToOut(pOut, PartOfSpeech::PRONOUN, "quelque chose");
    else
      _strToOut(pOut, PartOfSpeech::PRONOUN, "something");
  }
}


void Linguisticsynthesizergrounding::statGroundingTranslation
(OutSentence& pOutSentence,
 const SynthesizerConfiguration& pConf,
 const SemanticStatementGrounding& pStatementGrd,
 const linguistics::InflectedWord& pVerbInfoGram,
 const GroundedExpression& pHoldingGrdExp,
 const SynthesizerCurrentContext& pVerbContext,
 const UniqueSemanticExpression* pSubjectPtr) const
{
  SynthesizerCurrentContext verbContext = pVerbContext;
  // write the auxiliary / verb goal
  if (_writeVerbGoal(pOutSentence, verbContext, pStatementGrd,
                     pVerbInfoGram, pConf, pSubjectPtr))
  {
    if (!verbContext.isPassive)
    {
      if (verbContext.verbTense == LinguisticVerbTense::PRETERIT_CONTINUOUS)
        verbContext.verbTense = LinguisticVerbTense::PRESENT_PARTICIPLE;
      else
      verbContext.verbTense = _language == SemanticLanguageEnum::ENGLISH ?
            LinguisticVerbTense::PRESENT_IMPERATIVE : LinguisticVerbTense::INFINITIVE;
    }
    verbContext.isPositive = true;
    verbContext.requests.clear();
  }
  _statGroundingTranslation(pOutSentence, pConf.lingDb, pVerbInfoGram,
                            pHoldingGrdExp, verbContext);
}


void Linguisticsynthesizergrounding::_statGroundingTranslation
(OutSentence& pOutSentence,
 const linguistics::LinguisticDatabase& pLingDb,
 const linguistics::InflectedWord& pOutInfoGram,
 const GroundedExpression& pHoldingGrdExp,
 const SynthesizerCurrentContext& pContext) const
{
  // write sub meanings before main word
  if (pOutInfoGram.infos.linkedMeanings &&
      pHoldingGrdExp.children.count(GrammaticalType::RECEIVER) == 0)
      _writeSubMeanings(pOutSentence.receiverBeforeVerb.out, pLingDb,
                      *pOutInfoGram.infos.linkedMeanings, pContext, true);

  // write the verb
  writeVerbalSemWord(pOutSentence.aux.out, pOutSentence.verb,
                     pOutSentence.negation2.out, pOutInfoGram.word, pLingDb, pContext);

  // write sub meanings after main word
  if (pOutInfoGram.infos.linkedMeanings)
    _writeSubMeanings(pOutSentence.verb2.out, pLingDb, *pOutInfoGram.infos.linkedMeanings, pContext, false);
}


void Linguisticsynthesizergrounding::agentGroundingTranslation
(OutSemExp& pOutSemExp,
 SynthesizerConfiguration& pConf,
 const SemanticAgentGrounding& pGrounding,
 const SynthesizerCurrentContext& pContext,
 const SemanticRequests& pRequests) const
{
  RelativePerson relPerson =
      agentTypeToRelativePerson(pGrounding, pConf, !pContext.isPartOfANameAssignement);
  if (relPerson == RelativePerson::THIRD_SING &&
      pGrounding.isSpecificUser())
  {
    if (pContext.isPartOfANameAssignement &&
        pConf.canDoAnotherRecurssiveCall())
    {
      auto subGrdExpPtr = pConf.memBlock.getEquivalentGrdExpPtr(pGrounding.userId, pConf.lingDb);
      if (subGrdExpPtr)
      {
        SemanticRequests requests;
        SemanticExpression const* lastSubject = nullptr;
        auto& subGrdExp = subGrdExpPtr->getGrdExp();
        _lingSynth.writeGrdExp(pOutSemExp, requests, &lastSubject, subGrdExp, pConf, pContext);
        return;
      }
    }
    pOutSemExp.partOfSpeech = PartOfSpeech::PROPER_NOUN;
    if (pGrounding.nameInfos)
    {
      nameInfosTranslation(pOutSemExp.out, *pGrounding.nameInfos);
    }
    else
    {
      auto name = pConf.memBlock.getName(pGrounding.userId);
      if (!name.empty())
      {
        _strToOutIfNotEmpty(pOutSemExp.out,
                            PartOfSpeech::PROPER_NOUN,
                            name);
      }
      else
      {
        name = pGrounding.userId;
        mystd::replace_all(name, "-", "_");
        _strToOut(pOutSemExp.out, PartOfSpeech::PROPER_NOUN, "@" + name);
      }
    }
    return;
  }
  pOutSemExp.partOfSpeech =
      writeRelativePerson(pOutSemExp.out, relPerson, SemanticReferenceType::INDEFINITE, false,
                          SemanticEntityType::HUMAN, SemanticQuantity(),
                          pContext, pRequests);
}


void Linguisticsynthesizergrounding::_modifyContextForAGenGrounding
(SynthesizerWordContext& pWordContext,
 linguistics::InflectedWord& pOutInfoGram,
 const SynthesizerConfiguration& pConf,
 const SemanticGenericGrounding& pGrounding) const
{
  pWordContext.referenceType = pGrounding.referenceType;
  // get the out infoGram
  if (!_groundingAttributesToWord(pOutInfoGram, pGrounding) &&
      !synthGetter::getInflWordFromWordAndConcepts(pOutInfoGram, pGrounding.word, pGrounding.concepts, pConf.lingDb, _language) &&
      !pGrounding.word.lemma.empty() &&
      beginWithUppercase(pGrounding.word.lemma))
  {
    pOutInfoGram.word.partOfSpeech = PartOfSpeech::PROPER_NOUN;
  }

  if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::NOUN ||
      pOutInfoGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN ||
      pOutInfoGram.word.partOfSpeech == PartOfSpeech::UNKNOWN ||
      pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE ||
      pOutInfoGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
  {
    if (pGrounding.quantity.type == SemanticQuantityType::EVERYTHING)
    {
      if (_language == SemanticLanguageEnum::FRENCH)
        pWordContext.referenceType = SemanticReferenceType::DEFINITE;
    }
    else if (pGrounding.quantity.type == SemanticQuantityType::EVERYTHING ||
             (pGrounding.quantity.type == SemanticQuantityType::NUMBER &&
              pGrounding.quantity.nb > 1) ||
             (pGrounding.quantity.type == SemanticQuantityType::MOREOREQUALTHANNUMBER &&
              pGrounding.quantity.nb >= 2))
    {
      pWordContext.number = SemanticNumberType::PLURAL;
    }
    if (pGrounding.quantity.type == SemanticQuantityType::NUMBER &&
        pGrounding.quantity.nb == 1)
    {
      pWordContext.number = SemanticNumberType::SINGULAR;
    }
    else if (_language == SemanticLanguageEnum::FRENCH &&
             pWordContext.number != SemanticNumberType::UNKNOWN &&
             pGrounding.entityType == SemanticEntityType::MODIFIER)
    {
    }
    else if (pGrounding.quantity.isPlural() ||
             (_language == SemanticLanguageEnum::FRENCH &&
              pGrounding.quantity.type == SemanticQuantityType::EVERYTHING))
    {
      pWordContext.number = SemanticNumberType::PLURAL;
    }
    else
    {
      pWordContext.number = SemanticNumberType::SINGULAR;
    }
    if (pWordContext.number == SemanticNumberType::PLURAL)
      pWordContext.relativePerson = RelativePerson::THIRD_PLUR;
    else
      pWordContext.relativePerson = RelativePerson::THIRD_SING;
    std::set<SemanticGenderType> meaningPossGenders;
    if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::NOUN ||
        pOutInfoGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN ||
        pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
    {
      LinguisticMeaning nounLingMeaning;
      SemExpGetter::wordToAMeaning(nounLingMeaning, pOutInfoGram.word, _language,
                                   pConf.lingDb);
      SemanticLanguageEnum meaningLanguage = SemanticLanguageEnum::UNKNOWN;
      if (nounLingMeaning.getLanguageIfNotEmpty(meaningLanguage))
        pConf.lingDb.langToSpec[meaningLanguage].synthDico.
            getNounGenders(meaningPossGenders, nounLingMeaning, pWordContext.number);
    }
    pWordContext.gender = synthGetter::getGender(pWordContext.gender, meaningPossGenders,
                                                 pGrounding.possibleGenders);
  }
}

bool Linguisticsynthesizergrounding::_needToWriteDeterminer
(const linguistics::InflectedWord& pOutInfoGram,
 const SynthesizerCurrentContext& pContext,
 const SemanticGenericGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp,
 const linguistics::LinguisticDatabase& pLingDb,
 bool pOwnerWrittenBefore) const
{
  if (pGrounding.entityType == SemanticEntityType::MODIFIER &&
      (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN || pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE))
    return false;
  if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADVERB &&
      pGrounding.coreference)
    return false;
  if (pGrounding.entityType == SemanticEntityType::NUMBER &&
      pGrounding.quantity.type == SemanticQuantityType::NUMBER)
      return pGrounding.referenceType != SemanticReferenceType::UNDEFINED;
  if (pContext.grammaticalTypeFromParent == GrammaticalType::REPETITION ||
      ((pContext.grammaticalTypeFromParent == GrammaticalType::OBJECT ||
        pContext.grammaticalTypeFromParent == GrammaticalType::LOCATION ||
        pContext.grammaticalTypeFromParent == GrammaticalType::UNKNOWN ||
        pContext.isParentARelativeGrounding) &&
       pGrounding.quantity.type == SemanticQuantityType::NUMBER &&
       pGrounding.quantity.nb > 1 &&
       pGrounding.referenceType != SemanticReferenceType::DEFINITE) ||
      pOwnerWrittenBefore)
    return false;
  if (SemExpGetter::getRank(pHoldingGrdExp) != 0)
    return true;
  if (pGrounding.entityType != SemanticEntityType::NUMBER &&
      !pGrounding.quantity.isEqualToZero() &&
      !(_language == SemanticLanguageEnum::ENGLISH && pGrounding.referenceType == SemanticReferenceType::UNDEFINED) &&
      !(_language == SemanticLanguageEnum::ENGLISH && pGrounding.quantity.type == SemanticQuantityType::EVERYTHING) &&
      !(_language == SemanticLanguageEnum::ENGLISH && pGrounding.quantity.type == SemanticQuantityType::MAXNUMBER) &&
      pOutInfoGram.word.partOfSpeech != PartOfSpeech::DETERMINER &&
      pOutInfoGram.word.partOfSpeech != PartOfSpeech::VERB &&
      SemExpGetter::hasASpecificWord(pGrounding))
  {
    if (!pGrounding.coreference &&
        pGrounding.referenceType == SemanticReferenceType::DEFINITE)
    {
      switch (_language)
      {
      case SemanticLanguageEnum::ENGLISH:
      {
        if (!pContext.requests.has(SemanticRequestType::ACTION) &&
            ConceptSet::haveAConceptThatBeginWithAnyOf(pGrounding.concepts, {"concrete_food_", "gas_", "liquid_", "sport_", "meal_", "number_", "time_day_", "url_"}))
          return false;
        auto isUncountable = SemExpGetter::isUncountableFromGrd(pGrounding, pLingDb);
        return !isUncountable || !*isUncountable;
      }
      case SemanticLanguageEnum::FRENCH:
      {
        return !ConceptSet::haveAConceptThatBeginWithAnyOf(pGrounding.concepts, {"url_"});
      }
      default:
        return false;
      }
    }
    if (ConceptSet::haveAConcept(pOutInfoGram.infos.concepts, "reference_indefinite"))
        return false;
    return partOfSpeech_isNominal(pOutInfoGram.word.partOfSpeech) ||
        pGrounding.referenceType != SemanticReferenceType::UNDEFINED;
  }
  return false;
}

void Linguisticsynthesizergrounding::writePreposition
(std::list<WordToSynthesize>& pOut,
 const OutSemExp* pSubordinateOutPtr,
 const linguistics::InflectedWord& pInflectedVerb,
 const SynthesizerCurrentContext& pContext,
 const SynthesizerConfiguration& pConf,
 const SemanticGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp) const
{
  if (_language == SemanticLanguageEnum::FRENCH &&
      pSubordinateOutPtr != nullptr &&
      pSubordinateOutPtr->partOfSpeech == PartOfSpeech::PRONOUN &&
      !pSubordinateOutPtr->out.empty())
  {
    const auto& frontOut = pSubordinateOutPtr->out.front();
    if (!frontOut.inflections.empty())
    {
      const auto& frontOutStr = frontOut.inflections.front().str;
      if (!frontOutStr.empty() && frontOutStr[0] == '-')
        return;
    }
  }

  const std::list<WordToSynthesize>* subordinateWords = pSubordinateOutPtr != nullptr ? &pSubordinateOutPtr->out : nullptr;
  mystd::optional<SemanticWord> introWord;
  if (!_getIntroductingWordStoredInTheGrdExp(introWord, pHoldingGrdExp, pConf))
  {
    auto chkLinkTypeOpt = linguistics::grammaticalTypeToChunkType(pContext.grammaticalTypeFromParent);
    if (chkLinkTypeOpt)
    {
      const auto& frameDict = pConf.lingDb.langToSpec[_language].getSemFrameDict();
      if (pContext.contextType != SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB &&
          pContext.verbContextOpt)
      {
        if ((*chkLinkTypeOpt != linguistics::ChunkLinkType::DIRECTOBJECT ||
             pGrounding.type != SemanticGroundingType::STATEMENT || pGrounding.getStatementGrounding().requests.empty()))
          frameDict.getIntroWord(introWord, *chkLinkTypeOpt,
                                 pContext.verbContextOpt->statGrd.concepts,
                                 pContext.verbContextOpt->verbInflWord,
                                 &pHoldingGrdExp, subordinateWords, pContext.wordContext.number,
                                 pContext.wordContext.gender, pInflectedVerb);
      }
      else
      {
        frameDict.getIntroWordWithoutVerb(introWord, *chkLinkTypeOpt,
                                          &pHoldingGrdExp, subordinateWords, pContext.wordContext.number,
                                          pContext.wordContext.gender, pInflectedVerb);
      }
    }
  }
  if (introWord)
    _printIntroWord(pOut, *introWord, pContext);
}


void Linguisticsynthesizergrounding::writePrepositionWithoutContext
(std::list<WordToSynthesize>& pOut,
 const SynthesizerCurrentContext& pContext,
 const SynthesizerConfiguration& pConf) const
{
  auto chkLinkTypeOpt = linguistics::grammaticalTypeToChunkType(pContext.grammaticalTypeFromParent);
  if (chkLinkTypeOpt)
  {
    mystd::optional<SemanticWord> introWord;
    pConf.lingDb.langToSpec[_language].getSemFrameDict().getIntroWordWithoutConditions(introWord, *chkLinkTypeOpt);
    if (introWord)
      _printIntroWord(pOut, *introWord, pContext);
  }
}


void Linguisticsynthesizergrounding::_printIntroWord(std::list<WordToSynthesize>& pOut,
                                                     const SemanticWord& introWord,
                                                     const SynthesizerCurrentContext& pContext) const
{
  if (introWord.language == SemanticLanguageEnum::FRENCH)
  {
    if (introWord.lemma == "de")
    {
      _strWithApostropheToOut(pOut, introWord.partOfSpeech, "d'", "de");
      return;
    }
    if (introWord.lemma == "au")
    {
      if (pContext.wordContext.number == SemanticNumberType::PLURAL)
      {
        _strToOut(pOut, introWord.partOfSpeech, "aux");
        return;
      }
    }
    else if (introWord.lemma.size() > 3)
    {
      if (introWord.lemma.compare(introWord.lemma.size() - 3, 3, " de") == 0)
      {
        _strToOut(pOut, introWord.partOfSpeech, introWord.lemma.substr(0, introWord.lemma.size() - 3));
        _strWithApostropheToOut(pOut, introWord.partOfSpeech, "d'", "de");
        return;
      }
      if (introWord.lemma.compare(introWord.lemma.size() - 3, 3, "que") == 0)
      {
        _strWithApostropheToOut(pOut, introWord.partOfSpeech, introWord.lemma.substr(0, introWord.lemma.size() - 1) + "'", introWord.lemma);
        return;
      }
    }
  }
  _strToOut(pOut, introWord.partOfSpeech, introWord.lemma);
}


void Linguisticsynthesizergrounding::beforeGenGroundingTranslation
(OutSemExp& pBeforeOut,
 const linguistics::InflectedWord& pOutInfoGram,
 const SynthesizerCurrentContext& pContext,
 const SynthesizerConfiguration& pConf,
 const SemanticGenericGrounding& pGrounding,
 const GroundedExpression& pHoldingGrdExp) const
{
  // write "all" word if needed
  if (SemExpGetter::hasASpecificWord(pGrounding) &&
      pOutInfoGram.word.partOfSpeech != PartOfSpeech::DETERMINER &&
      pOutInfoGram.word.partOfSpeech != PartOfSpeech::INTERJECTION)
  {
    if (pGrounding.quantity.type == SemanticQuantityType::MAXNUMBER)
      _getAllWord(pBeforeOut.out, pContext);
    else if (pGrounding.quantity.type == SemanticQuantityType::EVERYTHING)
      _getEveryWord(pBeforeOut.out, pContext);
  }

  // add determiner before the noun
  if (_needToWriteDeterminer(pOutInfoGram, pContext, pGrounding, pHoldingGrdExp, pConf.lingDb, pContext.ownerWrittenBefore))
    _getDeterminer(pBeforeOut.out, pGrounding, pHoldingGrdExp, pContext);

  // write number before the noun
  if (pGrounding.quantity.type == SemanticQuantityType::NUMBER)
  {
    if (pGrounding.quantity.nb > 1 || pGrounding.entityType == SemanticEntityType::NUMBER)
    {
      bool numberWritten = false;
      if (pGrounding.quantity.nb == 1)
      {
        auto itSpecifier = pHoldingGrdExp.children.find(GrammaticalType::SPECIFIER);
        if (itSpecifier != pHoldingGrdExp.children.end())
        {
          if (_language == SemanticLanguageEnum::FRENCH)
          {
            if (synthGetter::getGenderFromSemExp(*itSpecifier->second, pConf, pContext,
                                                 *this) == SemanticGenderType::FEMININE)
              _strToOut(pBeforeOut.out, PartOfSpeech::DETERMINER, "une");
            else
              _strToOut(pBeforeOut.out, PartOfSpeech::DETERMINER, "un");
          }
          else
          {
            _strToOut(pBeforeOut.out, PartOfSpeech::DETERMINER, "one");
          }
          numberWritten = true;
        }
      }
      if (!numberWritten)
      {
        std::stringstream ssNumber;
        ssNumber << pGrounding.quantity.nb.toStr(_language);
        _strToOut(pBeforeOut.out, PartOfSpeech::DETERMINER, ssNumber.str());
      }
    }
    else if (pGrounding.quantity.nb == 0 && pGrounding.entityType != SemanticEntityType::NUMBER &&
             !pOutInfoGram.word.lemma.empty())
    {
      if (_language == SemanticLanguageEnum::FRENCH)
      {
        if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
        {
          if (pGrounding.entityType == SemanticEntityType::HUMAN)
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "personne");
          else
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "rien");
          _strToOut(pBeforeOut.out, PartOfSpeech::PREPOSITION, "de");
        }
        else
        {
          if (pOutInfoGram.word.lemma == "particulier")
          {
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "rien");
            _strToOut(pBeforeOut.out, PartOfSpeech::PREPOSITION, "en");
          }
          else if (pContext.wordContext.gender == SemanticGenderType::FEMININE)
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "aucune");
          else
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "aucun");
        }
      }
      else
      {
        if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB &&
            pContext.verbTense != LinguisticVerbTense::INFINITIVE)
        {
          if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
          {
            if (pGrounding.entityType == SemanticEntityType::HUMAN)
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "anybody in");
            else
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "anything");
          }
          else
            _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "any");
        }
        else
        {
          if (pOutInfoGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE)
          {
            if (pGrounding.entityType == SemanticEntityType::HUMAN)
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "nobody in");
            else
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "nothing");
          }
          else
          {
            if (pOutInfoGram.word.lemma == "particular")
            {
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "nothing");
              _strToOut(pBeforeOut.out, PartOfSpeech::PREPOSITION, "in");
            }
            else
              _strToOut(pBeforeOut.out, PartOfSpeech::ADVERB, "no");
          }
        }
      }
    }
  }
}


void Linguisticsynthesizergrounding::genGroundingTranslation
(OutSemExp& pOutSemExp,
 const linguistics::InflectedWord& pOutInfoGram,
 SynthesizerCurrentContext& pContext,
 const linguistics::LinguisticDatabase& pLingDb,
 const SemanticGenericGrounding& pGrounding) const
{
  if (!pGrounding.polarity)
  {
    if (_language == SemanticLanguageEnum::FRENCH)
      _strToOut(pOutSemExp.out, PartOfSpeech::ADVERB, "pas");
    else
      _strToOut(pOutSemExp.out, PartOfSpeech::ADVERB, "not");
  }

  if (pGrounding.entityType != SemanticEntityType::NUMBER)
  {
    // write sub meanings before main word
    if (pOutInfoGram.infos.linkedMeanings)
      _writeSubMeanings(pOutSemExp.out, pLingDb, *pOutInfoGram.infos.linkedMeanings, pContext, true);

    // write the main word
    _writeGenGrounding(pOutSemExp, pOutInfoGram, pLingDb, pGrounding, pContext);

    // write sub meanings after main word
    if (pOutInfoGram.infos.linkedMeanings)
      _writeSubMeanings(pOutSemExp.out, pLingDb, *pOutInfoGram.infos.linkedMeanings, pContext, false);
  }
}


void Linguisticsynthesizergrounding::getIGramOfAStatementMeaning
(linguistics::InflectedWord& pIGram,
 const SemanticStatementGrounding& pStatGrounding,
 const SynthesizerConfiguration& pConf) const
{
  getIGramOfAWord(pIGram, pStatGrounding.word, pStatGrounding.concepts, pConf);
}


void Linguisticsynthesizergrounding::getIGramOfAWord
(linguistics::InflectedWord& pIGram,
 const SemanticWord& pWord,
 const std::map<std::string, char>& pConcepts,
 const SynthesizerConfiguration& pConf) const
{
  StaticLinguisticMeaning lingMeaning(_language,
                                      SemExpGetter::wordToMeaningId
                                      (pWord, _language, pConf.lingDb));
  const auto& specLingDb = pConf.lingDb.langToSpec[lingMeaning.language];
  if (lingMeaning.isEmpty())
  {
    char currentPriority = 0;
    for (const auto& currCpt : pConcepts)
    {
      if (currCpt.second <= currentPriority)
        continue;
      auto newLingMeaning = specLingDb.synthDico.statDb.conceptToMeaning(currCpt.first);

      if (newLingMeaning.isEmpty() &&
          currCpt.first == "verb_generality")
      {
        if (_language == SemanticLanguageEnum::FRENCH)
          newLingMeaning = specLingDb.synthDico.statDb.conceptToMeaning("verb_have");
        else
          newLingMeaning = specLingDb.synthDico.statDb.conceptToMeaning("verb_equal_be");
      }

      if (!newLingMeaning.isEmpty())
      {
        currentPriority = currCpt.second;
        lingMeaning = newLingMeaning;
      }
    }
  }
  if (!lingMeaning.isEmpty())
  {
    specLingDb.lingDico.statDb.getInfoGram(pIGram, lingMeaning);
  }
}


bool Linguisticsynthesizergrounding::angleTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pStatSynthDico,
 const SemanticAngle& pAngle,
 bool) const
{
  GroundingAnglePrettyPrintStruct anglePrint(pAngle);
  std::stringstream ss;
  if (anglePrint.degree)
  {
    _printAngleValue(ss, *anglePrint.degree, SemanticAngleUnity::DEGREE, pStatSynthDico, _language);
  }
  if (anglePrint.radian)
  {
    if (!ss.str().empty())
      ss << " ";
    _printAngleValue(ss, *anglePrint.radian, SemanticAngleUnity::RADIAN, pStatSynthDico, _language);
  }

  const std::string timePrinted = ss.str();
  if (!timePrinted.empty())
  {
    _strToOut(pOut, PartOfSpeech::NOUN, timePrinted);
    return true;
  }
  return false;
}


bool Linguisticsynthesizergrounding::lengthTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pStatSynthDico,
 const SemanticLength& pLength,
 bool pPrintPrecisely) const
{
  GroundingLengthPrettyPrintStruct lengthPrint(pLength);
  std::stringstream ss;
  bool finishedToPrint = false;
  if (lengthPrint.kilometer)
  {
    _printLengthValue(ss, *lengthPrint.kilometer, SemanticLengthUnity::KILOMETER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.hectometer)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.hectometer, SemanticLengthUnity::HECTOMETER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.decameter)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.decameter, SemanticLengthUnity::DECAMETER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.meter)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.meter, SemanticLengthUnity::METER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.decimeter)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.decimeter, SemanticLengthUnity::DECIMETER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.centimeter)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.centimeter, SemanticLengthUnity::CENTIMETER, pStatSynthDico, _language);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && lengthPrint.millimeter)
  {
    if (!ss.str().empty())
      ss << " ";
    _printLengthValue(ss, *lengthPrint.millimeter, SemanticLengthUnity::MILLIMETER, pStatSynthDico, _language);
  }

  const std::string timePrinted = ss.str();
  if (!timePrinted.empty())
  {
    _strToOut(pOut, PartOfSpeech::NOUN, timePrinted);
    return true;
  }
  return false;
}


bool Linguisticsynthesizergrounding::durationTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::SynthesizerDictionary& pStatSynthDico,
 const SemanticDuration& pDuration,
 bool pPrintPrecisely,
 GrammaticalType pParentGrammaticalType) const
{
  GroundingDurationPrettyPrintStruct durationPrint(pDuration);
  SemanticGenderType globalGender = SemanticGenderType::UNKNOWN;
  std::stringstream ss;
  bool finishedToPrint = false;
  if (durationPrint.day)
  {
    _printTimeValue(ss, globalGender, *durationPrint.day, SemanticTimeUnity::DAY,
                    pStatSynthDico, _language, pParentGrammaticalType);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (durationPrint.hour)
  {
    _printTimeValue(ss, globalGender, *durationPrint.hour, SemanticTimeUnity::HOUR,
                    pStatSynthDico, _language, pParentGrammaticalType);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && durationPrint.minute)
  {
    if (!ss.str().empty())
      ss << " ";
    _printTimeValue(ss, globalGender, *durationPrint.minute, SemanticTimeUnity::MINUTE,
                    pStatSynthDico, _language, pParentGrammaticalType);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && durationPrint.second)
  {
    if (!ss.str().empty())
      ss << " ";
    _printTimeValue(ss, globalGender, *durationPrint.second, SemanticTimeUnity::SECOND,
                    pStatSynthDico, _language, pParentGrammaticalType);
    if (!pPrintPrecisely)
      finishedToPrint = true;
  }
  if (!finishedToPrint && durationPrint.millisecond)
  {
    if (!ss.str().empty())
      ss << " ";
    _printTimeValue(ss, globalGender, *durationPrint.millisecond, SemanticTimeUnity::MILLISECOND,
                    pStatSynthDico, _language, pParentGrammaticalType);
  }

  std::string timePrinted = ss.str();
  if (!timePrinted.empty())
  {
    if (pParentGrammaticalType == GrammaticalType::INTERVAL)
    {
      if (_language == SemanticLanguageEnum::FRENCH)
        _strToOut(pOut, PartOfSpeech::DETERMINER,
                  globalGender == SemanticGenderType::FEMININE ? "toutes" : "tous");
      else
        _strToOut(pOut, PartOfSpeech::DETERMINER, "every");
    }
    _strToOut(pOut, PartOfSpeech::NOUN, timePrinted);
    return true;
  }
  return false;
}





void Linguisticsynthesizergrounding::timeGroundingTranslation
(std::list<WordToSynthesize>& pOut,
 const linguistics::LinguisticDatabase& pLingDb,
 const SemanticTimeGrounding& pGrounding) const
{
  SemanticTimeGrounding refTimeGrd;
  refTimeGrd.equalToNow();

  const auto& synthDico = pLingDb.langToSpec[_language].synthDico;
  LinguisticMeaning partOfDayMeaningToSay;
  assert(partOfDayMeaningToSay.isEmpty());
  std::string partOfDayConceptToSay;
  if (pGrounding.tryToConvertToADayConcept(partOfDayConceptToSay, refTimeGrd))
    partOfDayMeaningToSay = synthDico.conceptToMeaning(partOfDayConceptToSay);

  std::string dateConceptToSay;
  bool dateWritten = false;
  if (pGrounding.tryToConvertToATimeConcept(dateConceptToSay, refTimeGrd))
  {
    if (dateConceptToSay == "time_relativeDay_today" &&
        partOfDayMeaningToSay.isEmpty())
    {
      SemanticTimeUnity timePrecision = pGrounding.timeOfDay.precision();
      if (timePrecision > SemanticTimeUnity::DAY)
      {
        SemanticDuration timeBeginDiff = refTimeGrd.timeOfDay - pGrounding.timeOfDay;
        SemanticDuration timeDiff = timeBeginDiff - pGrounding.length;
        if (!timeDiff.isPositive())
          timeDiff = std::move(timeBeginDiff);
        SemanticDuration timeDiffAbs = SemanticDuration::abs(timeDiff);
        SemanticDuration threeHours;
        threeHours.add(SemanticTimeUnity::HOUR, 4);
        if (timeDiffAbs < threeHours)
        {
          std::list<WordToSynthesize> durationOut;
          if (durationTranslation(durationOut, synthDico, timeDiff, false, GrammaticalType::UNKNOWN))
          {
            if (timeDiff.isPositive())
              _writeDurationAgo(durationOut);
            else
              _writeDurationIn(durationOut);
            pOut.splice(pOut.end(), durationOut);
            return;
          }
        }
      }
    }

    const auto meaning = synthDico.conceptToMeaning(dateConceptToSay);
    if (!meaning.isEmpty())
    {
      if (partOfDayMeaningToSay.isEmpty())
      {
        _strToOut(pOut, PartOfSpeech::ADVERB, synthDico.getLemma(meaning, false));
        if (dateConceptToSay == "time_relative_now")
          return;
        dateWritten = true;
      }
      else
      {
        if (dateConceptToSay == "time_relativeDay_today")
        {
          static const SemanticNumberType numberSingular = SemanticNumberType::SINGULAR;
          std::set<SemanticGenderType> meaningPossGenders;
          synthDico.getNounGenders(meaningPossGenders,
                                   partOfDayMeaningToSay,
                                   numberSingular);
          SemanticGenderType detGender = meaningPossGenders.empty() ?
                SemanticGenderType::NEUTRAL : *meaningPossGenders.begin();
          _getDeterminerThatReferToRecentContext(pOut, numberSingular, detGender);
        }
        else
          _strToOut(pOut, PartOfSpeech::ADJECTIVE, synthDico.getLemma(meaning, false));
        _strToOut(pOut, PartOfSpeech::NOUN,
                  synthDico.getLemma(partOfDayMeaningToSay, false));
        return;
      }
    }
  }

  if (!dateWritten)
    dateWritten = _dateTranslation(pOut, synthDico, pGrounding.date);
  _dayHourTranslation(pOut, synthDico, pGrounding.timeOfDay, dateWritten);
}



RelativePerson Linguisticsynthesizergrounding::agentTypeToRelativePerson
(const SemanticAgentGrounding& pGrounding,
 const SynthesizerConfiguration& pConf,
 bool pCanReplaceByEquivalent) const
{
  std::string grdUserId = pGrounding.userId == SemanticAgentGrounding::userNotIdentified ?
        SemanticAgentGrounding::currentUser : pGrounding.userId;
  if (grdUserId == pConf.authorId ||
      (pCanReplaceByEquivalent && pConf.memBlock.areSameUserConst(grdUserId, pConf.authorId)))
    return RelativePerson::FIRST_SING;
  if (grdUserId == pConf.receiverId ||
      (pCanReplaceByEquivalent && pConf.memBlock.areSameUserConst(grdUserId, pConf.receiverId)))
  {
    if (_language == SemanticLanguageEnum::FRENCH && pConf.textProcessingContext.vouvoiement)
      return RelativePerson::SECOND_PLUR;
    return RelativePerson::SECOND_SING;
  }
  return RelativePerson::THIRD_SING;
}


void Linguisticsynthesizergrounding::_writeSubMeanings
(std::list<WordToSynthesize>& pOut,
 const linguistics::LinguisticDatabase& pLingDb,
 const linguistics::LingWordsGroup& pMeaningsGroup,
 const SynthesizerCurrentContext& pContext,
 bool pBeforeVerb) const
{
  for (const auto& currLkMean : pMeaningsGroup.linkedMeanings)
  {
    bool befOrAfter = _doWeHaveToPutSubMeaningBeforeOrAfterTheWord(pContext, currLkMean.second);
    if (pBeforeVerb == befOrAfter)
    {
      const SemanticWord& semWord = *currLkMean.first;
      SynthesizerCurrentContext subContext = pContext;
      if (semWord.partOfSpeech == PartOfSpeech::VERB)
      {
        subContext.verbGoal = VerbGoalEnum::NOTIFICATION;
        subContext.hasASubject = false;
        subContext.verbTense = LinguisticVerbTense::INFINITIVE;
      }

      OutSemExp outSemExp;
      writeSemWord(outSemExp.out, outSemExp, outSemExp.out, semWord, pLingDb, subContext);
      pOut.splice(pOut.end(), outSemExp.out);
    }
  }
}



void Linguisticsynthesizergrounding::_writeGenGrounding
(OutSemExp& pOutSemExp,
 const linguistics::InflectedWord& pInfoGram,
 const linguistics::LinguisticDatabase& pLingDb,
 const SemanticGenericGrounding& pGrounding,
 SynthesizerCurrentContext& pContext) const
{
  if (!pInfoGram.word.lemma.empty())
    writeSemWord(pOutSemExp.out, pOutSemExp, pOutSemExp.out, pInfoGram.word, pLingDb, pContext);
  else
  {
    pOutSemExp.partOfSpeech =
        writeRelativePerson(pOutSemExp.out, pContext.wordContext.relativePerson, pGrounding.referenceType,
                            static_cast<bool>(pGrounding.coreference),
                            pGrounding.entityType, pGrounding.quantity,
                            pContext, pContext.requests);
  }
}




bool Linguisticsynthesizergrounding::_writeVerbalLinguisticMeaning
(std::list<WordToSynthesize>& pBeforeOut,
 OutSemExp& pOutSemExp,
 std::list<WordToSynthesize>& pAfterOut,
 const StaticLinguisticMeaning& pLingMeaning,
 const linguistics::LinguisticDatabase& pLingDb,
 const SynthesizerCurrentContext& pContext,
 bool pTryToWriteInSubordianteForm) const
{
  const auto& specLingDb = pLingDb.langToSpec[pLingMeaning.language];
  const auto& statSynthDico = specLingDb.synthDico.statDb;
  std::list<WordToSynthesize> verbForm;
  if (pContext.verbTense == LinguisticVerbTense::PRESENT_IMPERATIVE)
  {
    const auto& statSynthDico = specLingDb.synthDico.statDb;
    statSynthDico.getImperativeVerbForm(verbForm, pLingMeaning,
                                        pContext.wordContext.relativePerson, pContext.isPositive);
  }
  else if (pTryToWriteInSubordianteForm &&
           _language == SemanticLanguageEnum::FRENCH &&
           pContext.verbTense != LinguisticVerbTense::INFINITIVE)
  {
    _strToOut(pOutSemExp.out, PartOfSpeech::PRONOUN, "qui");
    std::string negationStr;
    bool isAGatheringMeaning = specLingDb.lingDico.statDb.isAGatheringMeaning(pLingMeaning);
    statSynthDico.getVerbForm
        (verbForm, negationStr, pLingMeaning, pContext.wordContext.relativePerson, pContext.wordContext.gender,
         LinguisticVerbTense::PRESENT_INDICATIVE, pContext.verbGoal, pContext.isPositive, pContext.hasASubject,
         isAGatheringMeaning, pContext.isPassive, pContext.isASubordinateWithoutPreposition, pContext.requests.has(SemanticRequestType::YESORNO));
  }
  else
  {
    std::string negationStr;
    bool isAGatheringMeaning = specLingDb.lingDico.statDb.isAGatheringMeaning(pLingMeaning);
    statSynthDico.getVerbForm
        (verbForm, negationStr, pLingMeaning, pContext.wordContext.relativePerson, pContext.wordContext.gender,
         pContext.verbTense, pContext.verbGoal, pContext.isPositive, pContext.hasASubject, isAGatheringMeaning,
         pContext.isPassive, pContext.isASubordinateWithoutPreposition, pContext.requests.has(SemanticRequestType::YESORNO));
    _strToOutIfNotEmpty(pAfterOut, PartOfSpeech::ADVERB, negationStr);
  }

  bool verbFormFound = !verbForm.empty();
  if (verbForm.size() > 1)
  {
    auto itBegin = verbForm.begin();
    if (itBegin->word.partOfSpeech != PartOfSpeech::UNKNOWN)
      pBeforeOut.splice(pBeforeOut.end(), verbForm, itBegin);
  }
  while (!verbForm.empty())
    pOutSemExp.out.splice(pOutSemExp.out.end(), verbForm, verbForm.begin());
  return verbFormFound;
}


void Linguisticsynthesizergrounding::writeVerbalSemWord
(std::list<WordToSynthesize>& pBeforeOut,
 OutSemExp& pOutSemExp,
 std::list<WordToSynthesize>& pAfterOut,
 const SemanticWord& pWord,
 const linguistics::LinguisticDatabase& pLingDb,
 const SynthesizerCurrentContext& pContext) const
{
  pOutSemExp.partOfSpeech = pWord.partOfSpeech;
  pOutSemExp.gender = pContext.wordContext.gender;
  pOutSemExp.relativePerson = pContext.wordContext.relativePerson;
  if (pWord.partOfSpeech == PartOfSpeech::VERB ||
      pWord.partOfSpeech == PartOfSpeech::AUX)
  {
    StaticLinguisticMeaning lingMeaning;
    SemExpGetter::wordToAStaticMeaning(lingMeaning, pWord, _language, pLingDb);
    if (!lingMeaning.isEmpty() &&
        _writeVerbalLinguisticMeaning(pBeforeOut, pOutSemExp, pAfterOut, lingMeaning,
                                      pLingDb, pContext, false))
      return;
  }
  if (!pWord.lemma.empty())
    _strToOut(pOutSemExp.out, pWord.partOfSpeech, _getRootLemma(pWord.lemma));
}


void Linguisticsynthesizergrounding::writeSemWord
(std::list<WordToSynthesize>& pBeforeOut,
 OutSemExp& pOutSemExp,
 std::list<WordToSynthesize>& pAfterOut,
 const SemanticWord& pWord,
 const linguistics::LinguisticDatabase& pLingDb,
 SynthesizerCurrentContext& pContext) const
{
  pOutSemExp.partOfSpeech = pWord.partOfSpeech;
  pOutSemExp.gender = pContext.wordContext.gender;
  pOutSemExp.relativePerson = pContext.wordContext.relativePerson;
  switch (pWord.partOfSpeech)
  {
  case PartOfSpeech::NOUN:
  {
    std::string nounStr;
    std::set<WordContextualInfos> contextualInfos;
    pLingDb.getInflectedNoun(nounStr, contextualInfos, _language, pWord,
                             pContext.wordContext.gender, pContext.wordContext.number);
    if (_strToOutIfNotEmpty(pOutSemExp.out, pWord.partOfSpeech, nounStr, &contextualInfos))
      return;
    break;
  }
  case PartOfSpeech::ADJECTIVE:
  {
    StaticLinguisticMeaning lingMeaning;
    SemExpGetter::wordToAStaticMeaning(lingMeaning, pWord, _language, pLingDb);
    if (!lingMeaning.isEmpty())
    {
      const auto& specLingDb = pLingDb.langToSpec[lingMeaning.language];
      const auto& statSynthDico = specLingDb.synthDico.statDb;
      std::string adjStr;
      statSynthDico.getAdjForm
          (adjStr, lingMeaning, pContext.wordContext.gender, pContext.wordContext.number, pContext.compPolarity);
      if (_strToOutIfNotEmpty(pAfterOut, pWord.partOfSpeech, adjStr))
        return;
    }
    break;
  }
  case PartOfSpeech::PRONOUN_COMPLEMENT:
  {
    StaticLinguisticMeaning lingMeaning;
    SemExpGetter::wordToAStaticMeaning(lingMeaning, pWord, _language, pLingDb);
    if (!lingMeaning.isEmpty())
    {
      const auto& specLingDb = pLingDb.langToSpec[lingMeaning.language];
      _getPronounComplement(pOutSemExp.out, lingMeaning, specLingDb, pContext);
      return;
    }
    break;
  }
  case PartOfSpeech::VERB:
  case PartOfSpeech::AUX:
  {
    StaticLinguisticMeaning lingMeaning;
    SemExpGetter::wordToAStaticMeaning(lingMeaning, pWord, _language, pLingDb);
    if (!lingMeaning.isEmpty() &&
        _writeVerbalLinguisticMeaning(pBeforeOut, pOutSemExp, pAfterOut, lingMeaning,
                                      pLingDb, pContext, true))
      return;
    break;
  }
  default:
    break;
  }

  if (!pWord.lemma.empty())
    _strToOut(pOutSemExp.out, pWord.partOfSpeech, _getRootLemma(pWord.lemma));
}


std::string Linguisticsynthesizergrounding::_getRootLemma
(const std::string& pLemma)
{
  std::size_t pos = pLemma.find('~');
  if (pos != std::string::npos)
    return pLemma.substr(0, pos);
  return pLemma;
}


void Linguisticsynthesizergrounding::_getPronounComplement
(std::list<WordToSynthesize>& pOut,
 const StaticLinguisticMeaning& pMeaning,
 const linguistics::SpecificLinguisticDatabase& pSpecLingDb,
 const SynthesizerCurrentContext&) const
{
  _strToOut(pOut, PartOfSpeech::PRONOUN_COMPLEMENT,
            pSpecLingDb.synthDico.statDb.getLemma(pMeaning, false));
}


bool Linguisticsynthesizergrounding::_getIntroductingWordStoredInTheGrdExp(mystd::optional<SemanticWord>& pIntroWord,
                                                                           const GroundedExpression& pGrdExp,
                                                                           const SynthesizerConfiguration& pConf) const
{
  auto itIntroductingWord = pGrdExp.children.find(GrammaticalType::INTRODUCTING_WORD);
  if (itIntroductingWord != pGrdExp.children.end())
  {
    auto* introWordGrdExpPtr = itIntroductingWord->second->getGrdExpPtr_SkipWrapperPtrs();
    if (introWordGrdExpPtr != nullptr)
    {
      const SemanticGrounding& grd = introWordGrdExpPtr->grounding();
      auto* genGrdPtr = grd.getGenericGroundingPtr();
      if (genGrdPtr != nullptr &&
          genGrdPtr->word.language != _language &&
          genGrdPtr->word.language != SemanticLanguageEnum::UNKNOWN)
        return false;

      linguistics::InflectedWord outInfoGram;
      SynthesizerWordContext wordContext;
      modifyContextForAGrounding(wordContext, outInfoGram, pConf, grd,
                                 SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC,
                                 LinguisticVerbTense::INFINITIVE);
      if (!outInfoGram.word.isEmpty())
        pIntroWord = outInfoGram.word;
      return true;
    }
  }
  return false;
}

} // End of namespace onsem
