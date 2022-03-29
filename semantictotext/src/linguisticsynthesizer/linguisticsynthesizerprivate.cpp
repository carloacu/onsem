#include "linguisticsynthesizerprivate.hpp"
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticsynthesizerdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "grounding/linguisticsynthesizergrounding.hpp"
#include "merger/synthesizerchunksmerger.hpp"
#include "conversion/tokenstostringconverter.hpp"
#include "synthesizertypes.hpp"
#include "linguisticsynthesizerfrench.hpp"
#include "linguisticsynthesizerenglish.hpp"
#include "tool/synthesizergetter.hpp"


namespace onsem
{
std::map<SemanticLanguageEnum, std::unique_ptr<LinguisticSynthesizerPrivate>> LinguisticSynthesizerPrivate::_instances;

namespace
{
void _invertRelativePerson(RelativePerson& pRelPerson)
{
  if (pRelPerson == RelativePerson::FIRST_SING)
  {
    pRelPerson = RelativePerson::SECOND_SING;
  }
  else if (pRelPerson == RelativePerson::SECOND_SING)
  {
    pRelPerson = RelativePerson::FIRST_SING;
  }
}

}


void LinguisticSynthesizerPrivate::_initInstances()
{
  if (_instances.empty())
  {
    for (const auto& currLangEnum : semanticLanguageEnum_allValues)
    {
      if (currLangEnum == SemanticLanguageEnum::FRENCH)
        _instances.emplace(currLangEnum, mystd::make_unique<LinguisticSynthesizerFrench>());
      else
        _instances.emplace(currLangEnum, mystd::make_unique<LinguisticSynthesizerEnglish>());
    }
  }
}

const LinguisticSynthesizerPrivate& LinguisticSynthesizerPrivate::getInstance(SemanticLanguageEnum pLangueType)
{
  auto itInstance = _instances.find(pLangueType);
  if (itInstance != _instances.end())
  {
    return *itInstance->second;
  }
  _initInstances();
  return *_instances.find(pLangueType)->second;
}



void LinguisticSynthesizerPrivate::getTranslationToNaturalLanguage
(std::list<std::unique_ptr<SynthesizerResult>>& pNaturalLanguageResult,
 const SemanticExpression& pSemExp,
 bool pOneLinePerSentence,
 const SemanticMemoryBlock& pMemBlock,
 const std::string& pCurrentUserId,
 const TextProcessingContext& pTextProcContext,
 const linguistics::LinguisticDatabase& pLingDb) const
{
  SemanticLanguageEnum outLanguage = getSyntGrounding().getOutLanguage();
  if (outLanguage == SemanticLanguageEnum::UNKNOWN)
  {
    return;
  }
  SynthesizerConfiguration privateConf(pMemBlock, pOneLinePerSentence, pCurrentUserId, pTextProcContext, pLingDb);
  // first list -> succestion of sentence elements
  // second list -> different possibilities
  OutSemExp outBlocs;
  SemanticExpression const* lastSubject = nullptr;

  {
    privateConf.semExp = &pSemExp;
    SemanticRequests requests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    SynthesizerCurrentContext rootContext;
    rootContext.isAtRoot = true;
    _writeSemExp(semExpSyntTypes, outBlocs, requests, &lastSubject, pSemExp, privateConf,
                 rootContext);
    _addPunctuation(outBlocs.out, semExpSyntTypes, requests, pOneLinePerSentence);
  }
  getTokenToStrConverter().writeOutputText(pNaturalLanguageResult, outBlocs.out, pOneLinePerSentence);
}


void LinguisticSynthesizerPrivate:: _addPunctuation
(std::list<WordToSynthesize>& pOut,
 const std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
 const SemanticRequests& pRequests,
 bool pOneLinePerSentence) const
{
  if (pOut.empty() ||
      (!pOut.back().inflections.empty() && !pOut.back().inflections.front().canHavePunctionAfter))
    return;

  SemExpTypeEnumFormSynt semExpSyntLastType = SEMEXPTYPEENUMFORSYNT_NOTHING;
  if (!pSemExpSyntTypes.empty())
    semExpSyntLastType = pSemExpSyntTypes.back();
  if (semExpSyntLastType == SEMEXPTYPEENUMFORSYNT_SENTENCE)
  {
    bool haveASpaceBeforeInterrogativeOrExclamativeMark = _language == SemanticLanguageEnum::FRENCH;
    if (pRequests.empty())
    {
      InflectionToSynthesize infls(".", false, true, alwaysTrue);
      infls.canHavePunctionAfter = false;
      pOut.emplace_back(SemanticWord(_language, ".", PartOfSpeech::PUNCTUATION), std::move(infls));
    }
    else if (pRequests.has(SemanticRequestType::ACTION))
    {
      InflectionToSynthesize infls("!", haveASpaceBeforeInterrogativeOrExclamativeMark, true, alwaysTrue);
      infls.canHavePunctionAfter = false;
      pOut.emplace_back(SemanticWord(_language, "!", PartOfSpeech::PUNCTUATION), std::move(infls));
    }
    else // it's a question
    {
      InflectionToSynthesize infls("?", haveASpaceBeforeInterrogativeOrExclamativeMark, true, alwaysTrue);
      infls.canHavePunctionAfter = false;
      pOut.emplace_back(SemanticWord(_language, "?", PartOfSpeech::PUNCTUATION), std::move(infls));
    }
  }
  else if (semExpSyntLastType == SEMEXPTYPEENUMFORSYNT_WORD)
  {
    InflectionToSynthesize infls("", false, true, isNextIsNotAPunctuation);
    if (!pOneLinePerSentence)
    {
      infls.str = ".";
      infls.canHavePunctionAfter = false;
    }
    pOut.emplace_back(SemanticWord(_language, infls.str, PartOfSpeech::PUNCTUATION), std::move(infls));
  }
}



void LinguisticSynthesizerPrivate::_writeSemExp
(std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
 OutSemExp& pOutSemExp,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const SemanticExpression& pSemExp,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    pSemExpSyntTypes.emplace_back(writeGrdExp(pOutSemExp, pRequests, pLastSubject, grdExp, pConf, pContext));
    return;
  }
  case SemanticExpressionType::CONDITION:
  {
    const ConditionExpression& condExp = pSemExp.getCondExp();
    pSemExpSyntTypes.emplace_back(_writeCondExp(pOutSemExp, pRequests, pLastSubject, condExp, pConf, pContext));
    return;
  }
  case SemanticExpressionType::COMPARISON:
  {
    const ComparisonExpression& compExp = pSemExp.getCompExp();
    pSemExpSyntTypes.emplace_back(_writeComparisonExp(pOutSemExp.out, pRequests, pLastSubject, compExp, pConf, pContext));
    return;
  }
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    _writeListExp(pSemExpSyntTypes, pOutSemExp, pConf, listExp, pContext, pLastSubject);
    return;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const InterpretationExpression& intExp = pSemExp.getIntExp();
    _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, *intExp.interpretedExp, pConf, pContext);
    return;
  }
  case SemanticExpressionType::METADATA:
  {
    const MetadataExpression& metadataExp = pSemExp.getMetadataExp();
    _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, *metadataExp.semExp, pConf, pContext);
    return;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const FeedbackExpression& fdkExp = pSemExp.getFdkExp();
    const SemanticExpression& feedbackExpChild = *fdkExp.feedbackExp;
    const SemanticExpression& concernedExpChild = *fdkExp.concernedExp;
    OutSemExp interjectionOut;
    std::list<SemExpTypeEnumFormSynt> intjTypeOfSemExps;
    _writeSemExp(intjTypeOfSemExps, interjectionOut, pRequests, pLastSubject, feedbackExpChild, pConf,
                 SynthesizerCurrentContext());
    OutSemExp mainSemExpOut;
    _writeSemExp(pSemExpSyntTypes, mainSemExpOut, pRequests, pLastSubject, concernedExpChild,
                 pConf, pContext);
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC &&
        !pSemExpSyntTypes.empty() &&
        (pSemExpSyntTypes.size() > 1 || pSemExpSyntTypes.back() == SEMEXPTYPEENUMFORSYNT_SENTENCE))
      interjectionOut.out.emplace_back(SemanticWord(_language, ",", PartOfSpeech::LINKBETWEENWORDS),
                                       InflectionToSynthesize(",", false, true, alwaysTrue));
    pOutSemExp.out.splice(pOutSemExp.out.end(), interjectionOut.out);
    pOutSemExp.out.splice(pOutSemExp.out.end(), mainSemExpOut.out);
    return;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pSemExp.getAnnExp();
    const SemanticExpression& semExpChild = *annExp.semExp;
    if (annExp.synthesizeAnnotations)
    {
      SynthesizerCurrentContext grdContext(pContext);
      OutNominalGroup outNominalGroup;
      for (const auto& currAnnotation : annExp.annotations)
        _writeNominalChild(outNominalGroup, pRequests, pLastSubject, pConf,
                           currAnnotation.first, *currAnnotation.second,
                           grdContext, nullptr, nullptr);
      _getChunksMerger().formulateNominalGroup(pOutSemExp.out, outNominalGroup);
     }
    else if (_handleSpecificLanguageSynthesis(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject,
                                              annExp.annotations, semExpChild, pConf, pContext))
      return;
    _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, semExpChild, pConf, pContext);
    return;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const SetOfFormsExpression& setOfFormsExp = pSemExp.getSetOfFormsExp();
    UniqueSemanticExpression* originalFormExp = setOfFormsExp.getOriginalForm();
    if (originalFormExp != nullptr)
    {
      _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, **originalFormExp, pConf, pContext);
      return;
    }
    return;
  }
  case SemanticExpressionType::COMMAND:
  {
    const CommandExpression& cmdExp = pSemExp.getCmdExp();
    _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, *cmdExp.semExp, pConf, pContext);
    return;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const FixedSynthesisExpression& fSynthExp = pSemExp.getFSynthExp();
    auto itLangToSynth = fSynthExp.langToSynthesis.find(_language);
    if (itLangToSynth != fSynthExp.langToSynthesis.end())
      synthTool::strToOut(pOutSemExp.out, PartOfSpeech::UNKNOWN, itLangToSynth->second, _language);
    else
      _writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject, fSynthExp.getSemExp(), pConf, pContext);
    return;
  }
  }

  assert(false);
}



SemExpTypeEnumFormSynt LinguisticSynthesizerPrivate::_writeComparisonExp
(std::list<WordToSynthesize>& pOut,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const ComparisonExpression& pCompExp,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
  const UniqueSemanticExpression* objectPtr = pCompExp.rightOperandExp ?
        &*pCompExp.rightOperandExp : nullptr;
  if (pCompExp.request == SemanticRequestType::NOTHING)
    pRequests.clear();
  else
    pRequests.set(pCompExp.request);
  OutSentence outSentence;
  const UniqueSemanticExpression* childToPutBeforeSubject = nullptr;
  _startAQuestion(pOut, outSentence.subject, pRequests, true, SemanticVerbTense::UNKNOWN, SemanticStatementGrounding(),
                  &pCompExp.leftOperandExp, objectPtr, false,
                  ObjectPosition::AFTERVERB, childToPutBeforeSubject, pConf,
                  pContext.contextType, pContext, true);

  outSentence.requests = pRequests;
  outSentence.equVerb = true;
  outSentence.contextType = pContext.contextType;

  SynthesizerCurrentContext subjectContext;
  subjectContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT;
  subjectContext.wordContext.relativePerson = _semExpToRelativePerson(*pCompExp.leftOperandExp, pConf,
                                                                      pContext.isPartOfANameAssignement);
  _writeSubjectContext(outSentence,
                       pLastSubject, pCompExp.leftOperandExp,
                       subjectContext, pConf, pRequests);

  {
    SynthesizerCurrentContext verbContext;
    verbContext.verbTense = _semanticVerbTenseToLinguisticVerbTense(pCompExp.tense, pContext.contextType, pContext.rootStatement, pRequests);
    verbContext.wordContext.relativePerson = subjectContext.wordContext.relativePerson;
    const SemanticWord& beWord = pConf.lingDb.langToSpec[_language].lingDico.getBeVerb().word;
    grdSynth.writeVerbalSemWord(outSentence.aux.out, outSentence.verb, outSentence.negation2.out,
                                beWord, pConf.lingDb, verbContext);
  }

  if (pCompExp.whatIsComparedExp)
  {
    SynthesizerCurrentContext objectContext;
    objectContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
    objectContext.compPolarity = pCompExp.op;
    SemanticRequests subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outSentence.objectAfterVerb, subRequests, pLastSubject,
                 **pCompExp.whatIsComparedExp, pConf, objectContext);
  }
  else
  {
    if (_language == SemanticLanguageEnum::FRENCH)
    {
      subjectContext.wordContext.gender = synthGetter::getGenderFromSemExp(*pCompExp.leftOperandExp, pConf, pContext, grdSynth);
      subjectContext.wordContext.number = relativePerson_toNumberType(subjectContext.wordContext.relativePerson);
    }
    _getComparisonWord(outSentence.objectAfterVerb.out,
                       pCompExp.op, subjectContext.wordContext.gender,
                       subjectContext.wordContext.number);
  }

  if (pCompExp.rightOperandExp)
  {
    _getThanWord(outSentence.objectAfterVerb.out, pCompExp.whatIsComparedExp);
    SynthesizerCurrentContext objectContext;
    objectContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
    SemanticRequests subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outSentence.objectAfterVerb, subRequests, pLastSubject,
                 **pCompExp.rightOperandExp, pConf, objectContext);
  }

  _getChunksMerger().formulateSentence(pOut, outSentence);
  return SEMEXPTYPEENUMFORSYNT_SENTENCE;
}


void LinguisticSynthesizerPrivate::_writeListExp
(std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
 OutSemExp& pOutSemExp,
 SynthesizerConfiguration& pConf,
 const ListExpression& pListExp,
 const SynthesizerCurrentContext& pContext,
 SemanticExpression const** pLastSubject) const
{
  std::size_t nbElts = pListExp.elts.size();
  std::size_t currId = 0;
  for (const auto& currElt : pListExp.elts)
  {
    ++currId;
    if (pListExp.listType != ListExpressionType::UNRELATED && currId > 1)
      getSyntGrounding().writeListSeparators(pOutSemExp.out, pListExp.listType, currId == nbElts);
    if (currId == nbElts && SemExpGetter::isACoreference(*currElt, CoreferenceDirectionEnum::AFTER))
    {
      pSemExpSyntTypes.emplace_back(SEMEXPTYPEENUMFORSYNT_NOTHING);
      continue;
    }
    else if (currId == 1 && nbElts == 2 && SemExpGetter::isACoreference(*currElt, CoreferenceDirectionEnum::BEFORE))
    {
      pSemExpSyntTypes.emplace_back(SEMEXPTYPEENUMFORSYNT_NOTHING);
      continue;
    }
    SemanticRequests requests;

    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT)
    {
      SynthesizerCurrentContext subContext(pContext);
      subContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
      _writeSemExp(pSemExpSyntTypes, pOutSemExp, requests, pLastSubject, *currElt,  pConf, subContext);
    }
    else
    {
      _writeSemExp(pSemExpSyntTypes, pOutSemExp, requests, pLastSubject, *currElt,  pConf, pContext);
    }

    if (pContext.isAtRoot && pListExp.listType == ListExpressionType::UNRELATED)
      _addPunctuation(pOutSemExp.out, pSemExpSyntTypes, requests, pConf.oneLinePerSentence);
  }
}


void LinguisticSynthesizerPrivate::_startAQuestion
(std::list<WordToSynthesize>& pOut,
 OutSemExp& pSubjectOut,
 const SemanticRequests& pRequests,
 bool pEquVerb,
 SemanticVerbTense pVerbTense,
 const SemanticStatementGrounding& pStatGrd,
 const UniqueSemanticExpression* pSubjectPtr,
 const UniqueSemanticExpression* pObjectPtr,
 bool pIsPassive,
 ObjectPosition pObjectPosition,
 const UniqueSemanticExpression*& pChildToPutBeforeSubject,
 const SynthesizerConfiguration& pConf,
 SynthesizerCurrentContextType pHoldingContextType,
 const SynthesizerCurrentContext& pContext,
 bool pNeedToWriteTheVerb) const
{
  if (!pOut.empty() && pOut.back().word.partOfSpeech == PartOfSpeech::PREPOSITION)
    return;
  if ((pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB ||
       pHoldingContextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB) &&
      (pRequests.has(SemanticRequestType::YESORNO) ||
       (pRequests.has(SemanticRequestType::OBJECT) &&
        pObjectPtr != nullptr &&
        pObjectPtr->getSemExp().getListExpPtr_SkipWrapperPtrs() != nullptr)))
  {
    _getBeginOfYesOrNoSubordonate(pOut);
  }
  else
  {
    _getQuestionWord(pOut, pSubjectOut, pRequests, pEquVerb, pVerbTense, pStatGrd, pSubjectPtr, pObjectPtr,
                     pIsPassive, pObjectPosition, pChildToPutBeforeSubject, pConf, pHoldingContextType,
                     pContext, pNeedToWriteTheVerb);
  }
}



SemExpTypeEnumFormSynt LinguisticSynthesizerPrivate::writeGrdExp
(OutSemExp& pOutSemExp,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const GroundedExpression& pGrdExp,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  const SemanticStatementGrounding* statementGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statementGrdPtr != nullptr)
  {
    pOutSemExp.partOfSpeech = PartOfSpeech::VERB;
    const SemanticStatementGrounding& statementGrd = *statementGrdPtr;
    _writeSentenceGrdExp(pOutSemExp.out, pRequests, pLastSubject, pGrdExp, statementGrd,
                         pConf, pContext);
    return SEMEXPTYPEENUMFORSYNT_SENTENCE;
  }

  const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
  linguistics::InflectedWord outInfoGram;
  SynthesizerCurrentContext grdContext(pContext);
  grdSynth.modifyContextForAGrounding(grdContext.wordContext, outInfoGram, pConf,
                                      pGrdExp.grounding(), pContext.contextType,
                                      pContext.verbTense);
  _writeNominalGrdExp(pOutSemExp, pRequests, pLastSubject, pGrdExp,
                      outInfoGram, pConf, grdContext);
  if (pOutSemExp.containsOnly(PartOfSpeech::BOOKMARK))
    return SEMEXPTYPEENUMFORSYNT_NOTHING;
  return SEMEXPTYPEENUMFORSYNT_WORD;
}


bool LinguisticSynthesizerPrivate::_handleSpecificLanguageSynthesis
(std::list<SemExpTypeEnumFormSynt>& pSemExpSyntTypes,
 OutSemExp& pOutSemExp,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const std::map<GrammaticalType, UniqueSemanticExpression>& pAnnotations,
 const SemanticExpression& pSemExpToExecute,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  SemanticLanguageEnum subLanguage = SemExpGetter::getLanguage(pAnnotations);
  if (subLanguage != SemanticLanguageEnum::UNKNOWN)
  {
    const LinguisticSynthesizerPrivate& subSynth = getInstance(subLanguage);
    subSynth._writeSemExp(pSemExpSyntTypes, pOutSemExp, pRequests, pLastSubject,
                          pSemExpToExecute, pConf, pContext);
    return true;
  }
  return false;
}


void LinguisticSynthesizerPrivate::_writeSentenceGrdExp
(std::list<WordToSynthesize>& pOut,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const GroundedExpression& pGrdExp,
 const SemanticStatementGrounding& pStatementGrd,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  SyntSentWorkStruct sentWorkStruct(pGrdExp, pStatementGrd, pLastSubject);
  auto* requestsPtr = SemExpGetter::getRequestList(pGrdExp);
  if (requestsPtr != nullptr)
    pRequests = *requestsPtr;
  else
    pRequests.clear();
  auto originRequests = pRequests;

  sentWorkStruct.outs.verbTense = sentWorkStruct.statementGrd.verbTense;
  if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
  {
    if (pRequests.has(SemanticRequestType::YESORNO))
      pRequests.clear();
  }
  else if ((pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN ||
            pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE) &&
           pRequests.has(SemanticRequestType::OBJECT))
  {
    pRequests.clear();
  }

  const auto& grdSynth = getSyntGrounding();

  linguistics::InflectedWord inflectedVerb;
  grdSynth.getIGramOfAStatementMeaning(inflectedVerb, pStatementGrd, pConf);

  SynthesizerCurrentContext verbContext;
  if (_language == SemanticLanguageEnum::ENGLISH &&
      sentWorkStruct.outs.verbTense == SemanticVerbTense::PUNCTUALPRESENT &&
      (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_CONDITION ||
       pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_TIME ||
       (ConceptSet::haveAConceptThatBeginWithAnyOf(pStatementGrd.concepts, {/*"appearance_",*/ "mentalState_", "perception_", "emotion_"}) &&
        !ConceptSet::haveAConceptThatBeginWith(pStatementGrd.concepts, "verb_action"))))
    verbContext.verbTense = LinguisticVerbTense::PRESENT_INDICATIVE;
  else if (_language == SemanticLanguageEnum::ENGLISH &&
           pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBVERB)
  {
    verbContext.verbTense = LinguisticVerbTense::PRESENT_INDICATIVE;
  }
  else if (pRequests.has(SemanticRequestType::ACTION))
    verbContext.verbTense = LinguisticVerbTense::PRESENT_IMPERATIVE;
  else if (sentWorkStruct.statementGrd.verbTense == SemanticVerbTense::UNKNOWN &&
           pContext.grammaticalTypeFromParent == GrammaticalType::WAY)
    verbContext.verbTense = LinguisticVerbTense::PRESENT_PARTICIPLE;
  else
  {
    verbContext.verbTense = _semanticVerbTenseToLinguisticVerbTense(sentWorkStruct.outs.verbTense, pContext.contextType,
                                                                    pContext.rootStatement, pStatementGrd.requests, &inflectedVerb.infos);
  }
  verbContext.verbContextOpt.emplace(inflectedVerb, pStatementGrd);

  bool isAGeneralitySentence = false;
  sentWorkStruct.outs.isPassive = pStatementGrd.isPassive && *pStatementGrd.isPassive;
  bool isPassiveFromObject = false;
  if (pStatementGrd.word.lemma.empty() &&
      pStatementGrd.concepts.count("verb_generality") > 0)
  {
    isAGeneralitySentence = true;
  }
  else if (!sentWorkStruct.outs.isPassive)
  {
    bool isPassiveFromSubject = sentWorkStruct.outs.verbTense != SemanticVerbTense::UNKNOWN &&
        !pRequests.has(SemanticRequestType::SUBJECT) &&
        sentWorkStruct.subjectPtr == nullptr &&
        sentWorkStruct.objectPtr != nullptr &&
        !SemExpGetter::semExphasAStatementGrd(**sentWorkStruct.objectPtr);
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN ||
        pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE)
    {
      if (sentWorkStruct.subjectPtr == nullptr &&
          pRequests.has(SemanticRequestType::OBJECT))
        isPassiveFromSubject = true;
      if (sentWorkStruct.outs.verbTense == SemanticVerbTense::PUNCTUALPAST)
      {
        if (sentWorkStruct.subjectPtr != nullptr &&
            SemExpGetter::isACoreference(**sentWorkStruct.subjectPtr, CoreferenceDirectionEnum::PARENT))
          isPassiveFromSubject = true;
      }
      if (sentWorkStruct.objectPtr != nullptr &&
          SemExpGetter::isACoreference(**sentWorkStruct.objectPtr, CoreferenceDirectionEnum::PARENT))
        isPassiveFromObject = true;
    }
    sentWorkStruct.outs.isPassive = isPassiveFromSubject || isPassiveFromObject;
  }
  sentWorkStruct.outs.contextType = pContext.contextType;
  if (sentWorkStruct.outs.isPassive)
  {
    for (auto& currRequest : pRequests.types)
      SemExpModifier::modifyRequestIfAtPassiveForm(currRequest);
    if (pContext.isASubordinateWithoutPreposition)
    {
      if (isPassiveFromObject)
        sentWorkStruct.objectPtr = nullptr;
      else
        sentWorkStruct.subjectPtr = nullptr;
    }
    else
    {
      if (isPassiveFromObject)
      {
        auto* objectPtr = sentWorkStruct.objectPtr;
        sentWorkStruct.objectPtr = sentWorkStruct.subjectPtr;
        sentWorkStruct.subjectPtr = objectPtr;
      }
      else
      {
        auto* subjectPtr = sentWorkStruct.subjectPtr;
        sentWorkStruct.subjectPtr = sentWorkStruct.objectPtr;
        sentWorkStruct.objectPtr = subjectPtr;
      }
    }
    // A reflexive verb can be passive, so we replace by the root form
    if (inflectedVerb.word.isReflexive())
      grdSynth.getIGramOfAWord(inflectedVerb, inflectedVerb.word.getRootFormFromReflexive(), pStatementGrd.concepts, pConf);
  }

  sentWorkStruct.objectIsAnNoElement = sentWorkStruct.objectPtr != nullptr &&
      SemExpGetter::getNumberOfElements(**sentWorkStruct.objectPtr) == 0;

  ObjectPosition objectPosition = _getObjectPosition(sentWorkStruct, pStatementGrd, pRequests,
                                                     pConf, verbContext.verbTense);

  const UniqueSemanticExpression* childToPutBeforeSubject = nullptr;
  if (originRequests.has(SemanticRequestType::OBJECT))
  {
    if (objectPosition == ObjectPosition::BEFORESUBJECT)
      childToPutBeforeSubject = sentWorkStruct.objectPtr;
  }
  else
  {
    GrammaticalType reqGrammType = semanticRequestType_toSemGram(originRequests.firstOrNothing());
    auto itChild = pGrdExp.children.find(reqGrammType);
    if (itChild != pGrdExp.children.end() &&
        !SemExpGetter::isDefinite(*itChild->second))
      childToPutBeforeSubject = &itChild->second;
    }

  sentWorkStruct.outs.equVerb = ConceptSet::haveAConceptThatBeginWith(pStatementGrd.concepts, "verb_equal_");
  if (!inflectedVerb.word.isEmpty())
    grdSynth.writePreposition(pOut, nullptr, inflectedVerb, pContext, pConf, pStatementGrd, pGrdExp);

  bool needToWriteTheVerb = !(_language == SemanticLanguageEnum::FRENCH &&
                              pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC &&
                              originRequests.has(SemanticRequestType::OBJECT) &&
                              inflectedVerb.word.lemma == "être" &&
                              sentWorkStruct.objectPtr == nullptr &&
                              sentWorkStruct.subjectPtr != nullptr &&
                              !SemExpGetter::hasSpecifications(**sentWorkStruct.subjectPtr));

  if (!originRequests.empty())
  {
    _startAQuestion(pOut, sentWorkStruct.outs.subject, originRequests,
                    sentWorkStruct.outs.equVerb, sentWorkStruct.outs.verbTense, pStatementGrd,
                    sentWorkStruct.subjectPtr, sentWorkStruct.objectPtr, sentWorkStruct.outs.isPassive,
                    objectPosition, childToPutBeforeSubject, pConf, pContext.contextType,
                    verbContext, needToWriteTheVerb);
  }
  else if (pStatementGrd.coreference &&
           pGrdExp.children.empty())
  {
    SemanticRequests requests;
    grdSynth.writeRelativePerson(pOut, RelativePerson::THIRD_SING, SemanticReferenceType::DEFINITE,
                                 true, SemanticEntityType::UNKNOWN, SemanticQuantity(),
                                 pContext, requests);
    return;
  }

  sentWorkStruct.outs.requests = pRequests;
  if (!sentWorkStruct.outs.subject.empty() &&
      sentWorkStruct.subjectPtr != nullptr &&
      sentWorkStruct.objectPtr == nullptr)
  {
    if (!SemExpGetter::isACoreference(**sentWorkStruct.subjectPtr, CoreferenceDirectionEnum::UNKNOWN))
      sentWorkStruct.objectPtr = sentWorkStruct.subjectPtr;
    sentWorkStruct.subjectPtr = nullptr;
  }

  bool isANameAssignement = SemExpGetter::isANameAssignement(pGrdExp);
  RelativePerson subjectRelativePerson = RelativePerson::UNKNOWN;
  if (sentWorkStruct.subjectPtr != nullptr)
  {
    subjectRelativePerson = _semExpToRelativePerson(**sentWorkStruct.subjectPtr, pConf,
                                                    isANameAssignement);
    if (pRequests.has(SemanticRequestType::ACTION))
      _invertRelativePerson(subjectRelativePerson);
  }
  else if (isAGeneralitySentence)
  {
    if (_language == SemanticLanguageEnum::ENGLISH &&
        sentWorkStruct.objectPtr != nullptr)
    {
      subjectRelativePerson = _semExpToRelativePerson(**sentWorkStruct.objectPtr, pConf,
                                                      isANameAssignement);
    }
    else
    {
      subjectRelativePerson = RelativePerson::THIRD_SING;
    }
  }
  else if (pRequests.has(SemanticRequestType::SUBJECT))
  {
    subjectRelativePerson = RelativePerson::THIRD_SING;
  }
  else if (sentWorkStruct.statementGrd.verbTense == SemanticVerbTense::UNKNOWN)
  {
    subjectRelativePerson = pContext.wordContext.relativePerson;
  }
  else if (pContext.parentSubject != nullptr)
  {
    subjectRelativePerson = _semExpToRelativePerson(*pContext.parentSubject, pConf,
                                                    isANameAssignement);
  }

  const bool subjectIsAParentCoreference = sentWorkStruct.subjectPtr != nullptr &&
      SemExpGetter::isACoreference(**sentWorkStruct.subjectPtr, CoreferenceDirectionEnum::PARENT);
  const bool objectIsAParentCoreference = sentWorkStruct.objectPtr != nullptr &&
      SemExpGetter::isACoreference(**sentWorkStruct.objectPtr, CoreferenceDirectionEnum::PARENT);
  if (subjectIsAParentCoreference ||
      (sentWorkStruct.subjectPtr != nullptr &&
       pRequests.has(SemanticRequestType::SUBJECT) &&
       SemExpGetter::isWhoSemExp(**sentWorkStruct.subjectPtr)))
    sentWorkStruct.subjectPtr = nullptr;
  else if (objectIsAParentCoreference ||
           (sentWorkStruct.objectPtr != nullptr &&
            pRequests.has(SemanticRequestType::OBJECT) &&
            SemExpGetter::isWhoSemExp(**sentWorkStruct.objectPtr)))
    sentWorkStruct.objectPtr = nullptr;

  verbContext.isPositive = pStatementGrd.polarity;
  bool writeTheRepetitions = true;
  if (sentWorkStruct.objectIsAnNoElement)
    verbContext.isPositive = !verbContext.isPositive;
  if (SemExpGetter::getNumberOfRepetitions(pGrdExp.children) == 0)
  {
    writeTheRepetitions = false;
    verbContext.isPositive = !verbContext.isPositive;
  }
  // write the verb
  if (needToWriteTheVerb)
  {
    if (pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_SUBVERB &&
        pStatementGrd.verbGoal == VerbGoalEnum::NOTIFICATION)
      verbContext.verbGoal = pContext.verbGoal;
    else
      verbContext.verbGoal = pStatementGrd.verbGoal;
    verbContext.isASubordinateWithoutPreposition = pContext.isASubordinateWithoutPreposition;
    verbContext.requests = pRequests;
    verbContext.hasASubject = sentWorkStruct.subjectPtr != nullptr;
    verbContext.parentSubject = pContext.parentSubject;
    verbContext.rootSubject = pContext.rootSubject;
    verbContext.rootStatement = pContext.rootStatement;
    verbContext.isPassive = sentWorkStruct.outs.isPassive && !inflectedVerb.infos.hasContextualInfo(WordContextualInfos::PASSIVE);

    if (_language == SemanticLanguageEnum::ENGLISH &&
        sentWorkStruct.statementGrd.verbTense == SemanticVerbTense::UNKNOWN)
      verbContext.wordContext.relativePerson = RelativePerson::FIRST_SING;
    else
      verbContext.wordContext.relativePerson = subjectRelativePerson;

    if (_language == SemanticLanguageEnum::FRENCH &&
        (verbContext.verbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE ||
         verbContext.isPassive))
    {
      if (sentWorkStruct.subjectPtr != nullptr)
        verbContext.wordContext.gender = synthGetter::getGenderFromSemExp(sentWorkStruct.subjectPtr->getSemExp(),
                                                                          pConf, verbContext, grdSynth);
      else
        verbContext.wordContext.gender = pContext.wordContext.gender;
    }

    // small for say/tell before to have a better translator engine
    if (inflectedVerb.word == SemanticWord(SemanticLanguageEnum::ENGLISH, "say", PartOfSpeech::VERB) &&
        pGrdExp.children.count(GrammaticalType::RECEIVER) > 0)
      inflectedVerb.word.lemma = "tell";
    grdSynth.statGroundingTranslation(sentWorkStruct.outs, pConf, pStatementGrd, inflectedVerb,
                                      sentWorkStruct.grdExp, verbContext,
                                      sentWorkStruct.subjectPtr);

    // write the negations
    if (!verbContext.isPositive)
    {
      _getNegationsBeforeVerb(sentWorkStruct.outs.negation1.out);
      if (!sentWorkStruct.objectIsAnNoElement)
        _getNegationsAfterVerb(sentWorkStruct.outs.negation2.out);
    }
  }

  // write the subject
  if (!pRequests.has(SemanticRequestType::ACTION))
  {
    if (sentWorkStruct.subjectPtr != nullptr)
    {
      const UniqueSemanticExpression& subjectRef = *sentWorkStruct.subjectPtr;
      SynthesizerCurrentContext subjectContext; // TODO: = verbContext;
      subjectContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBJECT;
      subjectContext.wordContext.relativePerson = subjectRelativePerson;
      subjectContext.currSentence = &sentWorkStruct.outs;
      subjectContext.verbTense = verbContext.verbTense;
      subjectContext.isPartOfANameAssignement = isANameAssignement;
      _writeSubjectContext(sentWorkStruct.outs,
                           pLastSubject, subjectRef,
                           subjectContext, pConf, pRequests);
    }
    else if (isAGeneralitySentence)
    {
      _writeSubjectOfGeneralitySentence(sentWorkStruct.outs);
    }
  }

  // write the objects
  if (sentWorkStruct.objectPtr != nullptr)
  {
    const SemanticExpression& objectSemExp = **sentWorkStruct.objectPtr;
    if (!_writeReflexionIfNeeded(sentWorkStruct, sentWorkStruct.outs.objectAfterVerb,
                                sentWorkStruct.outs.subject.relativePerson, pStatementGrd,
                                inflectedVerb, pGrdExp, objectSemExp, pConf,
                                verbContext, verbContext.rootSubject, verbContext.verbContextOpt))
    {
      OutSemExp& objectOutSemExp = [&]() -> OutSemExp&
      {
        if (objectPosition == ObjectPosition::BEFOREVERB)
          return sentWorkStruct.outs.objectBeforeVerb;
        if (objectPosition == ObjectPosition::BEFORESUBJECT)
          return sentWorkStruct.outs.objectBeforeSubject;
        return sentWorkStruct.outs.objectAfterVerb;
      }();
      _writeObjects(objectOutSemExp, sentWorkStruct.outs, sentWorkStruct, verbContext,
                    pStatementGrd, objectPosition, objectSemExp, pConf, pRequests,
                    isANameAssignement, pLastSubject);
    }
  }

  for (const auto& currChild : sentWorkStruct.grdExp.children)
  {
    if (currChild.first == GrammaticalType::SUBJECT ||
        currChild.first == GrammaticalType::OBJECT)
      continue;

    if (!(_language == SemanticLanguageEnum::FRENCH && currChild.first == GrammaticalType::MANNER) &&
        originRequests.has(semanticRequestType_fromSemGram(currChild.first)) &&
        childToPutBeforeSubject != nullptr)
    {
      OutSemExp& outSE = sentWorkStruct.outs.objectBeforeSubject;
      SynthesizerCurrentContext recContext;
      recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB;
      recContext.requests = originRequests;
      recContext.grammaticalTypeFromParent = currChild.first;
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, outSE, pRequests, pLastSubject,
                   **childToPutBeforeSubject, pConf, recContext);
      continue;
    }

    SynthesizerCurrentContextType childContextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
    const SemanticExpression& currSemExpChild = *currChild.second;
    switch (currChild.first)
    {
    case GrammaticalType::STARTING_POINT:
    {
      OutSemExp& outSE = sentWorkStruct.outs.startingPoint;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::IN_CASE_OF:
    {
      OutSemExp& outSE = sentWorkStruct.outs.inCaseOf;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::DURATION:
    {
      OutSemExp& outSE = sentWorkStruct.outs.duration;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::LANGUAGE:
    {
      OutSemExp& outSE = sentWorkStruct.outs.receiverAfterVerb;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::LOCATION:
    {
      OutSemExp& outSE = sentWorkStruct.outs.location;
      _writeLocation(outSE, currSemExpChild, pConf, pRequests,
                     verbContext, &sentWorkStruct, &verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::MANNER:
    {
      OutSemExp* outSE = &sentWorkStruct.outs.manner;

      auto* grdExpChildPtr = currSemExpChild.getGrdExpPtr_SkipWrapperPtrs();
      if (grdExpChildPtr != nullptr)
      {
        auto& grdExpChild = *grdExpChildPtr;

        if (_language == SemanticLanguageEnum::FRENCH)
        {
          if (grdExpChild->type != SemanticGroudingType::STATEMENT &&
              ConceptSet::haveAConcept(grdExpChild->concepts, "manner_well"))
            outSE = &sentWorkStruct.outs.mannerBetweenAuxAndVerb;
        }
        else
        {
          outSE = &sentWorkStruct.outs.manner;
        }
      }
      _writeObjectAfterVerb(sentWorkStruct, *outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::OCCURRENCE_RANK:
    {
      OutSemExp& outSE = sentWorkStruct.outs.occurrenceRank;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::RECEIVER:
    {
      _writeReceiver(sentWorkStruct.outs.receiverBeforeVerb,
                     sentWorkStruct.outs.receiverAfterVerb,
                     currSemExpChild, pConf, verbContext.isPositive, pRequests,
                     verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::REPETITION:
    {
      if (writeTheRepetitions)
      {
        OutSemExp& outSE = sentWorkStruct.outs.other;
        _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                              inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                              childContextType, verbContext, pLastSubject);
      }
      break;
    }
    case GrammaticalType::SPECIFIER:
    {
      _writeEquality(sentWorkStruct, currSemExpChild, pConf, pRequests,
                     verbContext, pContext.contextType, pLastSubject);
      break;
    }
    case GrammaticalType::TIME:
    {
      OutSemExp* outSE = &sentWorkStruct.outs.time;
      const GroundedExpression* timeGrdExpPtr = currSemExpChild.getGrdExpPtr_SkipWrapperPtrs();
      if (timeGrdExpPtr != nullptr &&
          timeGrdExpPtr->grounding().type != SemanticGroudingType::STATEMENT &&
          originRequests.has(SemanticRequestType::TIME) &&
          childToPutBeforeSubject != nullptr)
      {
        outSE = &sentWorkStruct.outs.objectBeforeSubject;
      }

      SynthesizerCurrentContext timeContext(verbContext);
      timeContext.grammaticalTypeFromParent = GrammaticalType::TIME;
      timeContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_TIME;
      SemanticRequests subRequestType = pRequests;
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, *outSE, subRequestType, pLastSubject,
                   currSemExpChild, pConf, timeContext);
      break;
    }
    case GrammaticalType::TOPIC:
    {
      OutSemExp& outSE = sentWorkStruct.outs.other;
      if (_tryToWriteTopicBeforeVerb(sentWorkStruct.outs.receiverBeforeVerb.out, currSemExpChild))
        break;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
    }
    case GrammaticalType::ACCORDING_TO:
    case GrammaticalType::AGAINST:
    case GrammaticalType::BETWEEN:
    case GrammaticalType::CAUSE:
    case GrammaticalType::DESPITE_CONTRAINT:
    case GrammaticalType::MITIGATION:
    case GrammaticalType::PURPOSE:
    case GrammaticalType::SIMILARITY:
    case GrammaticalType::SUBORDINATE:
    case GrammaticalType::THANKS_TO:
    case GrammaticalType::TODO:
    case GrammaticalType::WAY:
    case GrammaticalType::WITH:
    case GrammaticalType::WITHOUT:
    {
      OutSemExp& outSE = sentWorkStruct.outs.other;
      _writeObjectAfterVerb(sentWorkStruct, outSE, currChild.first, pStatementGrd,
                            inflectedVerb, pGrdExp, currSemExpChild, pConf, pRequests,
                            childContextType, verbContext, pLastSubject);
      break;
     }
    case GrammaticalType::SUBJECT:
    case GrammaticalType::OBJECT:
    case GrammaticalType::OWNER:
    case GrammaticalType::REASONOF:
    case GrammaticalType::SUB_CONCEPT:
    case GrammaticalType::OTHER_THAN:
    case GrammaticalType::INTRODUCTING_WORD:
    case GrammaticalType::NOT_UNDERSTOOD:
    case GrammaticalType::UNKNOWN:
      break;
    }
  }

  _getChunksMerger().formulateSentence(pOut, sentWorkStruct.outs);
}



void LinguisticSynthesizerPrivate::_writeNominalGrdExp
(OutSemExp& pOutSemExp,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const GroundedExpression& pGrdExp,
 const linguistics::InflectedWord& pOutInfoGram,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
  SynthesizerCurrentContext grdContext(pContext);
  const SemanticGrounding& grounding = pGrdExp.grounding();
  OutNominalGroup outNominalGroup;

  auto genGrdPtr = grounding.getGenericGroundingPtr();
  if (genGrdPtr != nullptr && genGrdPtr->coreference.has_value())
  {
    auto ownerIt = pGrdExp.children.find(GrammaticalType::OWNER);
    if (ownerIt != pGrdExp.children.end())
    {
      auto ownerGrdExp = ownerIt->second->getGrdExpPtr_SkipWrapperPtrs();
      if (ownerGrdExp != nullptr)
      {
        RelativePerson ownerRelPerson = _grdExpToRelativePerson(*ownerGrdExp, pConf,
                                                                grdContext.isPartOfANameAssignement);
        _getPossessiveWord(outNominalGroup.noun.out, grdContext.wordContext, ownerRelPerson,
                           ownerGrdExp->grounding(), pConf, grdContext);
        _getChunksMerger().formulateNominalGroup(pOutSemExp.out, outNominalGroup);
        return;
      }
    }
  }

  grdSynth.writeGrounding(outNominalGroup.noun,
                          pOutInfoGram, grdContext, pConf, grounding, pGrdExp, pRequests);

  for (const auto& currChild : pGrdExp.children)
    _writeNominalChild(outNominalGroup, pRequests, pLastSubject, pConf,
                       currChild.first, *currChild.second,
                       grdContext, &pOutInfoGram, &pGrdExp);

  grdSynth.writeGroundingIntroduction(outNominalGroup.determiner, outNominalGroup.noun,
                                      pOutInfoGram, grdContext, pConf, grounding, pGrdExp);
  pOutSemExp.partOfSpeech = outNominalGroup.noun.partOfSpeech;
  pOutSemExp.gender = grdContext.wordContext.gender;
  pOutSemExp.relativePerson = grdContext.wordContext.relativePerson;

  // write the nominal group parts in the good order
  _getChunksMerger().formulateNominalGroup(pOutSemExp.out, outNominalGroup);
}


void LinguisticSynthesizerPrivate::_writeNominalChild(OutNominalGroup& outNominalGroup,
                                                      SemanticRequests& pRequests,
                                                      SemanticExpression const** pLastSubject,
                                                      SynthesizerConfiguration& pConf,
                                                      GrammaticalType pChildGrammaticalType,
                                                      const SemanticExpression& currSemExpChild,
                                                      SynthesizerCurrentContext& pContext,
                                                      const linguistics::InflectedWord* pInflWordPtr,
                                                      const GroundedExpression* pGrdExpPtr) const
{
  const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();

  switch (pChildGrammaticalType)
  {
  case GrammaticalType::BETWEEN:
  {
    _getBeginOfBetweenSubordonate(outNominalGroup.subordinate.out);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.subordinate, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  case GrammaticalType::CAUSE:
  {
    _getBeginOfCauseSubordonate(outNominalGroup.subordinate.out);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.subordinate, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  case GrammaticalType::LOCATION:
  {
    _writeLocation(outNominalGroup.location, currSemExpChild, pConf,
                   pRequests, pContext, nullptr, nullptr, pLastSubject);
    break;
  }
  case GrammaticalType::OBJECT:
  {
    _writeNounSubordinates(outNominalGroup.subordinate, currSemExpChild, pChildGrammaticalType,
                           pConf, pRequests, pGrdExpPtr, pLastSubject);
    break;
  }
  case GrammaticalType::TODO:
  {
    _writeNounSubordinates(outNominalGroup.subordinate, currSemExpChild, pChildGrammaticalType,
                           pConf, pRequests, pGrdExpPtr, pLastSubject);
    break;
  }
  case GrammaticalType::MITIGATION:
  {
    _getBeginOfMitigationChild(outNominalGroup.subordinate.out);
    _writeNounSubordinates(outNominalGroup.subordinate, currSemExpChild, pChildGrammaticalType,
                           pConf, pRequests, pGrdExpPtr, pLastSubject);
    break;
  }
  case GrammaticalType::OWNER:
  {
    const GroundedExpression* ownerGrdExpPtr = currSemExpChild.getGrdExpPtr_SkipWrapperPtrs();

    if (ownerGrdExpPtr != nullptr && ConceptSet::haveAConcept(ownerGrdExpPtr->grounding().concepts, "tolink_1p"))
    {
      _getPossessiveDeterminer(outNominalGroup.ownerBeforeMainWord.out, RelativePerson::FIRST_PLUR,
                               pContext.wordContext.gender, pContext.wordContext.number);
      pContext.ownerWrittenBefore = true;
      break;
    }

    if (ownerGrdExpPtr != nullptr)
    {
      const GroundedExpression& ownerGrdExp = *ownerGrdExpPtr;
      const auto& ownerGrd = ownerGrdExp.grounding();
      RelativePerson ownerRelPerson = _grdExpToRelativePerson(ownerGrdExp, pConf,
                                                              pContext.isPartOfANameAssignement);

      switch (ownerRelPerson)
      {
      case RelativePerson::FIRST_SING:
      case RelativePerson::SECOND_SING:
      case RelativePerson::FIRST_PLUR:
      case RelativePerson::SECOND_PLUR:
      {
        _getPossessiveDeterminer(outNominalGroup.ownerBeforeMainWord.out, ownerRelPerson,
                                 pContext.wordContext.gender, pContext.wordContext.number);
        pContext.ownerWrittenBefore = true;
        break;
      }
      case RelativePerson::THIRD_SING:
      case RelativePerson::THIRD_PLUR:
      {
        const SemanticGenericGrounding* ownerGenGrd = ownerGrd.getGenericGroundingPtr();
        if (ownerGenGrd != nullptr &&
            ownerGenGrd->coreference &&
            ownerGenGrd->word.lemma.empty())
        {
          _getPossessiveDeterminer(outNominalGroup.ownerBeforeMainWord.out, ownerRelPerson,
                                   pContext.wordContext.gender, pContext.wordContext.number);
          pContext.ownerWrittenBefore = true;
        }
        else if (pContext.rootSubject != nullptr &&
                 SemExpComparator::semExpsAreEqual(*pContext.rootSubject, ownerGrdExp, pConf.memBlock, pConf.lingDb))
        {
          auto gender = pContext.wordContext.gender;
          auto number = pContext.wordContext.number;
          if (_language == SemanticLanguageEnum::ENGLISH)
          {
            gender = synthGetter::getGenderFromGrounding(ownerGrd, pConf, pContext, grdSynth);
            number = SemExpGetter::getNumberFromGrd(ownerGrd);
          }
          _getPossessiveDeterminer(outNominalGroup.ownerBeforeMainWord.out, ownerRelPerson, gender, number);
          pContext.ownerWrittenBefore = true;
        }
        break;
      }
      case RelativePerson::UNKNOWN:
        break;
      }

      if (_language == SemanticLanguageEnum::ENGLISH &&
          !pContext.ownerWrittenBefore &&
          !ConceptSet::haveAConceptThatBeginWith(ownerGrd.concepts, "country_"))
      {
        SynthesizerCurrentContext ownerContext;
        ownerContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC;
        auto subRequests = pRequests;
        linguistics::InflectedWord outInfoGram;
        grdSynth.modifyContextForAGrounding(ownerContext.wordContext, outInfoGram, pConf, ownerGrdExp.grounding(),
                                            ownerContext.contextType, ownerContext.verbTense);
        _writeNominalGrdExp(outNominalGroup.ownerBeforeMainWord, subRequests, pLastSubject, ownerGrdExp,
                            outInfoGram, pConf, ownerContext);
        const std::string possessiveStr = synthGetter::doesOutFinishedWithAS(outNominalGroup.ownerBeforeMainWord.out) ? "'" : "'s";
        outNominalGroup.ownerBeforeMainWord.out.emplace_back(SemanticWord(_language, possessiveStr, PartOfSpeech::ADVERB),
                                                             InflectionToSynthesize(possessiveStr, false, true, alwaysTrue));
        pContext.ownerWrittenBefore = true;
      }

      if (outNominalGroup.ownerBeforeMainWord.empty() &&
          ownerRelPerson != RelativePerson::FIRST_SING &&
          ownerRelPerson != RelativePerson::SECOND_SING)
      {
        SynthesizerCurrentContext ownerContext;
        ownerContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN;
        auto subRequests = pRequests;

        linguistics::InflectedWord outInfoGram;
        grdSynth.modifyContextForAGrounding(ownerContext.wordContext, outInfoGram, pConf, ownerGrdExp.grounding(),
                                            ownerContext.contextType, ownerContext.verbTense);
        if (pGrdExpPtr != nullptr)
        {
          const SemanticGrounding& grounding = pGrdExpPtr->grounding();
          _getBeginOfSpecification(outNominalGroup.ownerAfterMainWord.out, ownerGrdExp, grounding,
                                   ownerContext.wordContext);
        }
        _writeNominalGrdExp(outNominalGroup.ownerAfterMainWord, subRequests, pLastSubject,
                            ownerGrdExp, outInfoGram, pConf, ownerContext);
      }
    }
    break;
  }
  case GrammaticalType::SPECIFIER:
  {
    if (pGrdExpPtr != nullptr)
    {
      bool aDetermantHasBeenWrote = !outNominalGroup.determiner.out.empty();
      _writeSomeModifiersOfAWord(outNominalGroup.modifiersBeforeDeterminer,
                                 outNominalGroup.modifiersBeforeNoun,
                                 outNominalGroup.modifiersAfterNoun,
                                 outNominalGroup.subordinate,
                                 currSemExpChild,
                                 pConf, pContext, pRequests,
                                 aDetermantHasBeenWrote,
                                 *pGrdExpPtr, pInflWordPtr, pLastSubject);
    }
    break;
  }
  case GrammaticalType::SUB_CONCEPT:
  {
    if (pInflWordPtr != nullptr)
    {
      auto subRequests = pRequests;
      auto& subConcepts = _subConceptBeforeOrAfter(pInflWordPtr->infos) ?
            outNominalGroup.subConceptsBeforeNoun : outNominalGroup.subConceptsAfterNoun;
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, subConcepts,
                   subRequests, pLastSubject, currSemExpChild,
                   pConf, SynthesizerCurrentContext());
    }
    break;
  }
  case GrammaticalType::TIME:
  {
    _getOfWord(outNominalGroup.time.out, pContext.wordContext);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.time, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  case GrammaticalType::OTHER_THAN:
  {
    const auto& specLingDb = pConf.lingDb.langToSpec[_language];
    LinguisticMeaning lingMeaning = specLingDb.synthDico.conceptToMeaning("reference_other");
    if (!lingMeaning.isEmpty())
    {
      linguistics::InflectedWord inflWord;
      specLingDb.lingDico.getInfoGram(inflWord, lingMeaning);
      OutSemExp& otherChildOutSemExp = outNominalGroup.modifiersBeforeNoun;
      grdSynth.writeSemWord(otherChildOutSemExp.out, otherChildOutSemExp, otherChildOutSemExp.out,
                            inflWord.word, pConf.lingDb, pContext);
    }
    break;
  }
  case GrammaticalType::WITH:
  {
    _getBeginOfWithChild(outNominalGroup.subordinate.out);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.subordinate, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  case GrammaticalType::WITHOUT:
  {
    _getBeginOfWithoutChild(outNominalGroup.subordinate.out);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.subordinate, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  case GrammaticalType::PURPOSE:
  {
    _getBeginOfForChild(outNominalGroup.purpose.out, currSemExpChild);
    auto subRequests = pRequests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, outNominalGroup.purpose, subRequests,
                 pLastSubject, currSemExpChild,
                 pConf, SynthesizerCurrentContext());
    break;
  }
  default:
    break;
  }
}


SemExpTypeEnumFormSynt LinguisticSynthesizerPrivate::_writeCondExp
(OutSemExp& pOutSemExp,
 SemanticRequests& pRequests,
 SemanticExpression const** pLastSubject,
 const ConditionExpression& pCondExp,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext) const
{
  const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
  grdSynth.writePrepositionWithoutContext(pOutSemExp.out, pContext, pConf);

  auto writeIntroOfConditionWord = [this, &pCondExp]
      (std::list<WordToSynthesize>& pOut)
  {
    const auto* condGrdExpPtr = pCondExp.conditionExp->getGrdExpPtr_SkipWrapperPtrs();
    if (condGrdExpPtr == nullptr ||
        (condGrdExpPtr->grounding().type != SemanticGroudingType::RELATIVEDURATION &&
         condGrdExpPtr->grounding().type != SemanticGroudingType::TIME))
    {
      if (pCondExp.conditionPointsToAffirmations)
      {
        _getBeginOfCauseSubordonate(pOut);
      }
      else if (pCondExp.isAlwaysActive)
      {
        _getWheneverCondition(pOut);
      }
      else
      {
        _getBeginOfYesOrNoSubordonate(pOut);
      }
    }
  };

  if (pCondExp.elseExp ||
      pContext.contextType == SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB)
  {
    writeIntroOfConditionWord(pOutSemExp.out);
    {
      auto subCondRequests = pRequests;
      SynthesizerCurrentContext condContext;
      condContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_CONDITION;
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, pOutSemExp, subCondRequests, pLastSubject, *pCondExp.conditionExp,
                   pConf, condContext);
    }

    if (pCondExp.isAlwaysActive)
    {
      pOutSemExp.out.emplace_back(SemanticWord(_language, ",", PartOfSpeech::LINKBETWEENWORDS),
                                  InflectionToSynthesize(",", false, true, alwaysTrue));
    }
    else
    {
      _getThenWord(pOutSemExp.out);
    }
    {
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, pOutSemExp, pRequests, pLastSubject, *pCondExp.thenExp,
                   pConf, SynthesizerCurrentContext());
    }
    if (pCondExp.elseExp)
    {
      _getElseWord(pOutSemExp.out);
      {
        SynthesizerCurrentContext elseContext;
        elseContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
        std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
        _writeSemExp(semExpSyntTypes, pOutSemExp, pRequests, pLastSubject, **pCondExp.elseExp,
                     pConf, elseContext);
      }
    }
  }
  else
  {
    {
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, pOutSemExp, pRequests, pLastSubject, *pCondExp.thenExp,
                   pConf, SynthesizerCurrentContext());
    }

    writeIntroOfConditionWord(pOutSemExp.out);
    {
      auto subThenRequests = pRequests;
      SynthesizerCurrentContext condContext;
      condContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_CONDITION;
      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, pOutSemExp, subThenRequests, pLastSubject, *pCondExp.conditionExp,
                   pConf, condContext);
    }
  }
  return SEMEXPTYPEENUMFORSYNT_SENTENCE;
}


void LinguisticSynthesizerPrivate::_writeObjects
(OutSemExp& pOutSemExp,
 OutSentence& pCurrSentence,
 const SyntSentWorkStruct& pSentWorkStruct,
 const SynthesizerCurrentContext& pVerbContext,
 const SemanticStatementGrounding& pStatementGrd,
 ObjectPosition pObjectPosition,
 const SemanticExpression& pObjectSemExp,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests,
 bool pIsANameAssignement,
 SemanticExpression const** pLastSubject) const
{
  const ListExpression* objectListExpPtr = pObjectSemExp.getListExpPtr_SkipWrapperPtrs();
  if (objectListExpPtr != nullptr)
  {
    std::size_t nbElts = objectListExpPtr->elts.size();
    std::size_t currId = 0;
    for (const auto& currElt : objectListExpPtr->elts)
    {
      ++currId;
      if (objectListExpPtr->listType != ListExpressionType::UNRELATED && currId > 1)
        getSyntGrounding().writeListSeparators(pOutSemExp.out, objectListExpPtr->listType, currId == nbElts);
      if (currId == nbElts && SemExpGetter::isACoreference(*currElt, CoreferenceDirectionEnum::AFTER))
        continue;
      else if (currId == 1 && nbElts == 2 && SemExpGetter::isACoreference(*currElt, CoreferenceDirectionEnum::BEFORE))
        continue;
      _writeObjects(pOutSemExp, pCurrSentence, pSentWorkStruct, pVerbContext, pStatementGrd,
                    pObjectPosition, *currElt, pConf, pRequests, pIsANameAssignement,
                    pLastSubject);
    }

    return;
  }

  const GroundedExpression* objectGrdExpPtr = pObjectSemExp.getGrdExpPtr_SkipWrapperPtrs();
  SemanticRequests subRequests = pRequests;
  if (pObjectPosition == ObjectPosition::BEFOREVERB)
  {
    if (_language == SemanticLanguageEnum::FRENCH &&
        pSentWorkStruct.objectIsAnNoElement &&
        pVerbContext.verbTense == LinguisticVerbTense::INFINITIVE)
    {
      _strToOut(pOutSemExp.out, PartOfSpeech::ADVERB, "rien");
    }
    else
    {
      SynthesizerCurrentContext objBefVerbContext;
      objBefVerbContext.grammaticalTypeFromParent = GrammaticalType::OBJECT;
      objBefVerbContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB;
      objBefVerbContext.wordContext.relativePerson = _semExpToRelativePerson(pObjectSemExp, pConf, pIsANameAssignement);
      objBefVerbContext.requests = pVerbContext.requests;

      std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
      _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
                   pObjectSemExp, pConf, objBefVerbContext);
    }
  }
  else if (pObjectPosition == ObjectPosition::BEFORESUBJECT)
  {
    SynthesizerCurrentContext objBefVerbContext;
    objBefVerbContext.grammaticalTypeFromParent = GrammaticalType::OBJECT;
    objBefVerbContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTBEFOREVERB;
    objBefVerbContext.wordContext.relativePerson = _semExpToRelativePerson(pObjectSemExp, pConf, pIsANameAssignement);
    objBefVerbContext.requests = pVerbContext.requests;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
                 pObjectSemExp, pConf, objBefVerbContext);
  }
  else
  {
    SynthesizerCurrentContext objAfterVerbContext;
    objAfterVerbContext.grammaticalTypeFromParent = GrammaticalType::OBJECT;
    if (pSentWorkStruct.subjectPtr != nullptr &&
        (pSentWorkStruct.outs.equVerb || SemExpGetter::semExphasAStatementGrd(pObjectSemExp)))
    {
      objAfterVerbContext.wordContext.gender = pSentWorkStruct.outs.subject.gender;
      objAfterVerbContext.wordContext.relativePerson = pSentWorkStruct.outs.subject.relativePerson;
      if (pSentWorkStruct.outs.subject.relativePerson >= RelativePerson::FIRST_PLUR &&
          pSentWorkStruct.outs.subject.relativePerson <= RelativePerson::THIRD_PLUR)
        objAfterVerbContext.wordContext.number = SemanticNumberType::PLURAL;
      else
        objAfterVerbContext.wordContext.number = SemanticNumberType::SINGULAR;
    }
    objAfterVerbContext.contextType = pSentWorkStruct.beginOfAVerbalForm() ?
          SYNTHESIZERCURRENTCONTEXTTYPE_SUBVERB : SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
    objAfterVerbContext.isPositive = pVerbContext.isPositive;
    objAfterVerbContext.verbTense = pVerbContext.verbTense;
    objAfterVerbContext.verbGoal = pVerbContext.verbGoal;
    if (pVerbContext.verbContextOpt)
      objAfterVerbContext.verbContextOpt.emplace(*pVerbContext.verbContextOpt);
    else
      objAfterVerbContext.verbContextOpt.reset();
    objAfterVerbContext.requests = pVerbContext.requests;
    objAfterVerbContext.isPartOfANameAssignement = pIsANameAssignement;
    objAfterVerbContext.currSentence = &pCurrSentence;
    objAfterVerbContext.rootSubject = pSentWorkStruct.subjectPtr != nullptr ?
          &**pSentWorkStruct.subjectPtr : pVerbContext.rootSubject;
    objAfterVerbContext.rootStatement = &pStatementGrd;
    if (objectGrdExpPtr != nullptr)
      _beginOfSubordonateIfNeeded(objAfterVerbContext.isASubordinateWithoutPreposition,
                                  pOutSemExp, pSentWorkStruct, GrammaticalType::OBJECT,
                                  *objectGrdExpPtr, pVerbContext.verbContextOpt, pConf);
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
                 pObjectSemExp, pConf, objAfterVerbContext);
  }
}


void LinguisticSynthesizerPrivate::_writeSubjectContext
(OutSentence& pOutSentence,
 SemanticExpression const** pLastSubject,
 const UniqueSemanticExpression& pSemExp,
 SynthesizerCurrentContext& pSubjectContext,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests) const
{
  const GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const GroundedExpression& grdExp = *grdExpPtr;
    const SemanticGrounding& grounding = grdExp.grounding();
    if (ConceptSet::haveAConcept(grounding.concepts, "generic"))
    {
      _writeGenericSubject(pOutSentence);
      return;
    }
  }

  bool useLastSubject = false;
  if (*pLastSubject != nullptr)
  {
    SemExpComparator::ComparisonExceptions compExceptions;
    SemExpGetter::getStatementSubordinates(compExceptions.semExps1ToSkip, **pLastSubject);
    SemExpGetter::getStatementSubordinates(compExceptions.semExps2ToSkip, *pSemExp);
    auto imbrication = SemExpComparator::getSemExpsImbrications(**pLastSubject, *pSemExp, pConf.memBlock, pConf.lingDb, &compExceptions);
    useLastSubject = imbrication == ImbricationType::EQUALS || imbrication == ImbricationType::MORE_DETAILED;
  }
  if (useLastSubject)
  {
    const auto& grdSynth = getSyntGrounding();
    pSubjectContext.wordContext.gender = synthGetter::getGenderFromSemExp(*pSemExp, pConf, pSubjectContext, grdSynth);
    SemanticEntityType entityType = SemExpGetter::getEntity(**pLastSubject);
    if (_language == SemanticLanguageEnum::FRENCH && entityType == SemanticEntityType::THING &&
        !SemExpGetter::isACoreference(**pLastSubject, CoreferenceDirectionEnum::UNKNOWN))
      entityType = SemanticEntityType::AGENTORTHING;
    pOutSentence.subject.gender = pSubjectContext.wordContext.gender;
    pOutSentence.subject.relativePerson = pSubjectContext.wordContext.relativePerson;
    pOutSentence.subject.partOfSpeech =
        grdSynth.writeRelativePerson(pOutSentence.subject.out, pSubjectContext.wordContext.relativePerson,
                                     SemanticReferenceType::DEFINITE, false, entityType,
                                     SemanticQuantity(), pSubjectContext, pRequests);
  }
  else
  {
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    auto subRequests = pRequests;
    _writeSemExp(semExpSyntTypes, pOutSentence.subject, subRequests, pLastSubject,
                 *pSemExp, pConf, pSubjectContext);
    *pLastSubject = &*pSemExp;

    if (_language == SemanticLanguageEnum::FRENCH &&
        grdExpPtr != nullptr)
    {
      const GroundedExpression& grdExp = *grdExpPtr;
      const SemanticGrounding& grounding = grdExp.grounding();
      const SemanticStatementGrounding* statGrdPtr = grounding.getStatementGroundingPtr();
      // add "ce" when the subject is an infinitive verb
      if (statGrdPtr != nullptr &&
          statGrdPtr->verbTense == SemanticVerbTense::UNKNOWN)
        _strWithApostropheToOut(pOutSentence.subject.out, PartOfSpeech::PRONOUN_SUBJECT, "c'", "ça");
    }
  }
}



RelativePerson LinguisticSynthesizerPrivate::_semExpToRelativePerson
(const SemanticExpression& pSemExp,
 const SynthesizerConfiguration& pConf,
 bool pIsANameAssignement) const
{
  const GroundedExpression* grdExp = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp != nullptr)
    return _grdExpToRelativePerson(*grdExp, pConf, pIsANameAssignement);

  const ListExpression* listExp = pSemExp.getListExpPtr();
  if (listExp != nullptr)
  {
    for (const auto& currElt : listExp->elts)
    {
      RelativePerson subPerson = _semExpToRelativePerson(*currElt, pConf, pIsANameAssignement);
      if (subPerson == RelativePerson::FIRST_SING)
        return RelativePerson::FIRST_PLUR;
      if (subPerson == RelativePerson::SECOND_SING)
        return RelativePerson::SECOND_PLUR;
    }
    return RelativePerson::THIRD_PLUR;
  }
  return RelativePerson::THIRD_SING;
}




void LinguisticSynthesizerPrivate::_writeReceiver
(OutSemExp& pOutSemExpBeforeVerb,
 OutSemExp& pOutSemExpAfterVerb,
 const SemanticExpression& pSemExp,
 SynthesizerConfiguration& pConf,
 bool pVerbIsAffirmative,
 const SemanticRequests& pRequests,
 const SynthesizerCurrentContext& pVerbContext,
 SemanticExpression const** pLastSubject) const
{
  std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
  if (_putReceiverBeforeVerb(pSemExp, pVerbIsAffirmative, pRequests, pConf))
  {
    SynthesizerCurrentContext recContext;
    recContext.grammaticalTypeFromParent = GrammaticalType::RECEIVER;
    recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTBEFOREVERB;
    auto subRequests = pRequests;
    _writeSemExp(semExpSyntTypes, pOutSemExpBeforeVerb, subRequests, pLastSubject,
                 pSemExp, pConf, recContext);
  }
  else
  {
    SynthesizerCurrentContext recContext(pVerbContext);
    recContext.grammaticalTypeFromParent = GrammaticalType::RECEIVER;
    recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_INDIRECTOBJECTAFTERVERB;
    auto subRequests = pRequests;
    _writeSemExp(semExpSyntTypes, pOutSemExpAfterVerb, subRequests, pLastSubject,
                 pSemExp, pConf, recContext);
  }
}


void _extractCommonBegin(std::list<WordToSynthesize>& pCommonBegin,
                         std::list<OutSemExp>& pSubOutSemExps)
{
  std::list<WordToSynthesize>* firstOutsPtr = nullptr;
  WordToSynthesize* endOfCommonBeginPtr = nullptr;
  for (auto it = pSubOutSemExps.begin(); it != pSubOutSemExps.end(); ++it)
  {
    auto& currOut = it->out;
    if (firstOutsPtr == nullptr)
    {
      firstOutsPtr = &currOut;
    }
    else
    {
      for (auto itCommon = firstOutsPtr->begin(), itCurr = currOut.begin(); itCommon != firstOutsPtr->end() && &*itCommon != endOfCommonBeginPtr; ++itCommon, ++itCurr)
      {
        if (itCurr == currOut.end() || itCurr->word.partOfSpeech == PartOfSpeech::DETERMINER || *itCommon != *itCurr)
        {
          endOfCommonBeginPtr = &*itCommon;
          break;
        }
      }
    }
  }

  if (firstOutsPtr != nullptr)
  {
    std::list<WordToSynthesize> commonBeginRes;
    for (auto itCommon = firstOutsPtr->begin(); itCommon != firstOutsPtr->end() && &*itCommon != endOfCommonBeginPtr; )
    {
      auto itCommonNext = itCommon;
      ++itCommonNext;
      commonBeginRes.splice(commonBeginRes.end(), *firstOutsPtr, itCommon);
      itCommon = itCommonNext;
    }
    bool firstIteration = true;
    for (auto& currOutSemExp : pSubOutSemExps)
    {
      if (firstIteration)
      {
        firstIteration = false;
      }
      else
      {
        for (std::size_t i = 0; i < commonBeginRes.size(); ++i)
          currOutSemExp.out.pop_front();
      }
    }
    pCommonBegin.splice(pCommonBegin.end(), commonBeginRes);
  }
}


void LinguisticSynthesizerPrivate::_writeLocation
(OutSemExp& pOutSemExp,
 const SemanticExpression& pSemExp,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests,
 const SynthesizerCurrentContext& pContext,
 const SyntSentWorkStruct* pSentWorkStructPtr,
 const SynthesizerCurrentContext* pVerbContextPtr,
 SemanticExpression const** pLastSubject) const
{
  const ListExpression* locationListExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (locationListExpPtr != nullptr)
  {
    std::list<OutSemExp> subOutSemExps;
    for (auto& currElt : locationListExpPtr->elts)
    {
      subOutSemExps.emplace_back();
      auto& subOutSemExp = subOutSemExps.back();
      _writeLocation(subOutSemExp, *currElt, pConf, pRequests, pContext,
                     pSentWorkStructPtr, pVerbContextPtr, pLastSubject);
    }
    _extractCommonBegin(pOutSemExp.out, subOutSemExps);

    std::size_t nbElts = subOutSemExps.size();
    std::size_t currId = 0;
    for (auto& currSubOutSemExp : subOutSemExps)
    {
      ++currId;
      if (locationListExpPtr->listType != ListExpressionType::UNRELATED && currId > 1)
        getSyntGrounding().writeListSeparators(pOutSemExp.out, locationListExpPtr->listType, currId == nbElts);
      pOutSemExp.out.splice(pOutSemExp.out.end(), currSubOutSemExp.out);
    }
    return;
  }


  if (pVerbContextPtr != nullptr)
  {
    SynthesizerCurrentContext recContext(*pVerbContextPtr);
    recContext.grammaticalTypeFromParent = GrammaticalType::LOCATION;
    recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
    std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
    auto subRequests = pRequests;
    _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
                 pSemExp, pConf, recContext);
    return;
  }

  SynthesizerCurrentContext recContext;
  recContext.grammaticalTypeFromParent = GrammaticalType::LOCATION;
  recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_OBJECTAFTERVERB;
  std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
  auto subRequests = pRequests;
  _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
               pSemExp, pConf, recContext);
}


bool LinguisticSynthesizerPrivate::_writeReflexionIfNeeded
(SyntSentWorkStruct& pSentWorkStruct,
 OutSemExp& pOutSemExp,
 RelativePerson pSubjectRelativePerson,
 const SemanticStatementGrounding& pStatementGrd,
 const linguistics::InflectedWord& pInflectedVerb,
 const GroundedExpression& pHoldingGrdExp,
 const SemanticExpression& pSemExp,
 const SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext,
 SemanticExpression const* pRootSubject,
 const mystd::optional<SynthesizerVerbContext>& pVerbContextOpt) const
{
  const auto& grdSynth = getSyntGrounding();
  SemExpComparator::ComparisonExceptions compExceptions;
  compExceptions.equivalentUserId = true;
  compExceptions.corefenceExcludeEquality = true;
  if (pSentWorkStruct.subjectPtr != nullptr)
  {
    if (SemExpComparator::semExpsAreEqualFromMemBlock(**pSentWorkStruct.subjectPtr, pSemExp, pConf.memBlock, pConf.lingDb, &compExceptions))
    {
      if (_language == SemanticLanguageEnum::ENGLISH)
        grdSynth.writePreposition(pOutSemExp.out, nullptr, pInflectedVerb, pContext, pConf, pStatementGrd, pHoldingGrdExp);
      grdSynth.writeReflexiveObject(pSentWorkStruct.outs, pOutSemExp.out, pSubjectRelativePerson, pSentWorkStruct.outs.subject.gender, pVerbContextOpt);
      return true;
    }
  }
  else if (pRootSubject != nullptr)
  {
    if (SemExpComparator::semExpsAreEqualFromMemBlock(*pRootSubject, pSemExp, pConf.memBlock, pConf.lingDb, &compExceptions))
    {
      pSubjectRelativePerson = _semExpToRelativePerson(*pRootSubject, pConf, false);
      SemanticGenderType gender = synthGetter::getGenderFromSemExp(*pRootSubject, pConf, pContext, grdSynth);
      if (_language == SemanticLanguageEnum::ENGLISH)
        grdSynth.writePreposition(pOutSemExp.out, nullptr, pInflectedVerb, pContext, pConf, pStatementGrd, pHoldingGrdExp);
      grdSynth.writeReflexiveObject(pSentWorkStruct.outs, pOutSemExp.out, pSubjectRelativePerson, gender, pVerbContextOpt);
      return true;
    }
  }
  return false;
}



void LinguisticSynthesizerPrivate::_writeObjectAfterVerb
(SyntSentWorkStruct& pSentWorkStruct,
 OutSemExp& pOutSemExp,
 GrammaticalType pGramType,
 const SemanticStatementGrounding& pStatementGrd,
 const linguistics::InflectedWord& pInflectedVerb,
 const GroundedExpression& pHoldingGrdExp,
 const SemanticExpression& pSemExp,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests,
 SynthesizerCurrentContextType pChildContextType,
 const SynthesizerCurrentContext& pVerbContext,
 SemanticExpression const** pLastSubject) const
{
  SynthesizerCurrentContext objectContext(pVerbContext);
  objectContext.grammaticalTypeFromParent = pGramType;
  objectContext.contextType = pChildContextType;

  if (_writeReflexionIfNeeded(pSentWorkStruct, pOutSemExp, pSentWorkStruct.outs.subject.relativePerson,
                              pStatementGrd, pInflectedVerb, pHoldingGrdExp, pSemExp, pConf,
                              objectContext, pVerbContext.rootSubject, objectContext.verbContextOpt))
    return;

  SemanticRequests subRequests = pRequests;
  std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
  _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
               pSemExp, pConf, objectContext);
}


void LinguisticSynthesizerPrivate::_writeEquality
(SyntSentWorkStruct& pSentWorkStruct,
 const SemanticExpression& pSemExp,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests,
 const SynthesizerCurrentContext& pContext,
 SynthesizerCurrentContextType pParentContextType,
 SemanticExpression const** pLastSubject) const
{
  const ListExpression* listModifierGrdExp = pSemExp.getListExpPtr();
  if (listModifierGrdExp != nullptr)
  {
    for (const auto& currElt : listModifierGrdExp->elts)
      _writeEquality(pSentWorkStruct, *currElt, pConf, pRequests, pContext, pParentContextType, pLastSubject);
    return;
  }

  const GroundedExpression* specGrdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (specGrdExpPtr != nullptr)
  {
    SynthesizerCurrentContext context;
    const GroundedExpression& specGrdExp = *specGrdExpPtr;
    const auto& specGrd = specGrdExp.grounding();
    if (specGrd.type == SemanticGroudingType::STATEMENT)
      return;
    const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
    linguistics::InflectedWord outInfoGram;
    SynthesizerCurrentContext grdContext(context);
    grdSynth.modifyContextForAGrounding(grdContext.wordContext, outInfoGram, pConf,
                                        specGrdExp.grounding(), context.contextType,
                                        context.verbTense);

    bool beforeOrAfterTheVerb = outInfoGram.infos.hasContextualInfo(WordContextualInfos::CANBEBEFOREVERB) &&
        (_language != SemanticLanguageEnum::FRENCH ||
        pParentContextType == SynthesizerCurrentContextType::SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN);
    OutSemExp& outSemExp = beforeOrAfterTheVerb ?
          pSentWorkStruct.outs.objectBeforeVerb : pSentWorkStruct.outs.equalityAfterVerb;
    auto subRequests = pRequests;
    _writeNominalGrdExp(outSemExp, subRequests, pLastSubject, specGrdExp,
                        outInfoGram, pConf, grdContext);
  }
}



void LinguisticSynthesizerPrivate::_writeNounSubordinates
(OutSemExp& pOutSemExp,
 const SemanticExpression& pSemExp,
 GrammaticalType pSemExpFatherGramType,
 SynthesizerConfiguration& pConf,
 const SemanticRequests& pRequests,
 const GroundedExpression* pGrdExpPtr,
 SemanticExpression const** pLastSubject) const
{
  const ListExpression* locationListExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
  if (locationListExpPtr != nullptr)
  {
    std::list<OutSemExp> subOutSemExps;
    for (const auto& currElt : locationListExpPtr->elts)
    {
      subOutSemExps.emplace_back();
      auto& subOutSemExp = subOutSemExps.back();
      _writeNounSubordinates(subOutSemExp, *currElt, pSemExpFatherGramType, pConf, pRequests, pGrdExpPtr, pLastSubject);
    }

    std::size_t nbElts = subOutSemExps.size();
    std::size_t currId = 0;
    for (auto& currSubOutSemExp : subOutSemExps)
    {
      ++currId;
      if (locationListExpPtr->listType != ListExpressionType::UNRELATED && currId > 1)
        getSyntGrounding().writeListSeparators(pOutSemExp.out, locationListExpPtr->listType, currId == nbElts);
      pOutSemExp.out.splice(pOutSemExp.out.end(), currSubOutSemExp.out);
    }
    return;
  }

  if (_language == SemanticLanguageEnum::FRENCH)
  {
    const auto* grdExpEltPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpEltPtr != nullptr)
    {
      const auto* statGrdEltPtr = grdExpEltPtr->grounding().getStatementGroundingPtr();
      if (statGrdEltPtr != nullptr &&
          statGrdEltPtr->verbTense == SemanticVerbTense::UNKNOWN)
      {
        if (pSemExpFatherGramType == GrammaticalType::TODO)
          _strToOut(pOutSemExp.out, PartOfSpeech::PREPOSITION, "à");
        else
          _strWithApostropheToOut(pOutSemExp.out, PartOfSpeech::PREPOSITION, "d'", "de");
      }
    }
  }
  SynthesizerCurrentContext recContext;
  recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN;
  if (pGrdExpPtr != nullptr && pGrdExpPtr->grounding().getGenericGroundingPtr() != nullptr &&
      pGrdExpPtr->grounding().getGenericGrounding().word.partOfSpeech == PartOfSpeech::ADJECTIVE)
    recContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERADJECTIVE;
  std::list<SemExpTypeEnumFormSynt> semExpSyntTypes;
  auto subRequests = pRequests;
  _writeSemExp(semExpSyntTypes, pOutSemExp, subRequests, pLastSubject,
               pSemExp, pConf, recContext);
}


void LinguisticSynthesizerPrivate::_writeSomeModifiersOfAWord
(OutSemExp& pOutBeforeDeterminer,
 OutSemExp& pOutBeforeMainWord,
 OutSemExp& pOutAfterMainWord,
 OutSemExp& pOutSubordinate,
 const SemanticExpression& pModifierSemExp,
 SynthesizerConfiguration& pConf,
 const SynthesizerCurrentContext& pContext,
 const SemanticRequests& pRequests,
 bool pADeterminantHasBeenWrote,
 const GroundedExpression& pMotherGrdExp,
 const linguistics::InflectedWord* pInflWordPtr,
 SemanticExpression const** pLastSubject) const
{
  const GroundedExpression* modifierGrdExpPtr = pModifierSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (modifierGrdExpPtr != nullptr)
  {
    const GroundedExpression& modifierGrdExp = *modifierGrdExpPtr;
    const Linguisticsynthesizergrounding& grdSynth = getSyntGrounding();
    linguistics::InflectedWord outInfoGram;
    const SemanticGrounding& modifierGrd = modifierGrdExp.grounding();
    SynthesizerCurrentContext grdContext(pContext);
    grdContext.isParentARelativeGrounding = semanticGroudingsType_isRelativeType(pMotherGrdExp.grounding().type);
    grdContext.grammaticalTypeFromParent = GrammaticalType::SPECIFIER;
    SynthesizerWordContext wordContext;
    grdSynth.modifyContextForAGrounding(wordContext, outInfoGram, pConf, modifierGrd,
                                        pContext.contextType, pContext.verbTense);

    if (pContext.grammaticalTypeFromParent == GrammaticalType::LOCATION)
      grdContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_GENERIC;
    else
      grdContext.contextType = SYNTHESIZERCURRENTCONTEXTTYPE_SUBORDINATEAFTERNOUN;
    grdContext.parentSubject = &pMotherGrdExp;

    OutSemExp outSemExp;
    SpecifierPosition specifierPosition =
        _getSpecifierPosition(outInfoGram, modifierGrd, pADeterminantHasBeenWrote, pInflWordPtr);
    if (specifierPosition == SpecifierPosition::AFTER &&
        _doWeHaveToWriteBeginOfSpecifier(grdContext, outInfoGram, modifierGrdExp, *pMotherGrdExp))
    {
      const SemanticRelativeTimeGrounding* relTimeGrdPtr = pMotherGrdExp->getRelTimeGroundingPtr();
      if (relTimeGrdPtr != nullptr)
        _getRelTimeFollowingPrep(outSemExp.out, *relTimeGrdPtr);
      else
        _getBeginOfSpecification(outSemExp.out, modifierGrdExp, pMotherGrdExp.grounding(), wordContext);
      grdContext.wordContext = wordContext;
    }
    auto subRequests = pRequests;
    writeGrdExp(outSemExp, subRequests, pLastSubject, modifierGrdExp,
                pConf, grdContext);

    switch (specifierPosition)
    {
    case SpecifierPosition::BEFORE_DETERMINER:
    {
      pOutBeforeDeterminer.moveContent(outSemExp);
      break;
    }
    case SpecifierPosition::BEFORE:
    {
      pOutBeforeMainWord.moveContent(outSemExp);
      break;
    }
    case SpecifierPosition::AFTER:
    {
      if (outSemExp.partOfSpeech == PartOfSpeech::VERB)
        pOutSubordinate.moveContent(outSemExp);
      else
        pOutAfterMainWord.moveContent(outSemExp);
      break;
    }
    }
    return;
  }

  const ListExpression* modifierListExp = pModifierSemExp.getListExpPtr();
  if (modifierListExp != nullptr)
  {
    std::list<OutSemExp> subOutEltsBeforeDeterminer;
    std::list<OutSemExp> subOutEltsBeforeMainWord;
    std::list<OutSemExp> subOutEltsAfterMainWord;
    std::list<OutSemExp> subOutEltsSubordinate;

    for (const auto& currElt : modifierListExp->elts)
    {
      subOutEltsBeforeDeterminer.emplace_back();
      subOutEltsBeforeMainWord.emplace_back();
      subOutEltsAfterMainWord.emplace_back();
      subOutEltsSubordinate.emplace_back();
      _writeSomeModifiersOfAWord(subOutEltsBeforeDeterminer.back(),
                                 subOutEltsBeforeMainWord.back(),
                                 subOutEltsAfterMainWord.back(),
                                 subOutEltsSubordinate.back(),
                                 *currElt, pConf, pContext, pRequests,
                                 pADeterminantHasBeenWrote, pMotherGrdExp,
                                 pInflWordPtr, pLastSubject);
      if (subOutEltsBeforeDeterminer.back().out.empty())
        subOutEltsBeforeDeterminer.pop_back();
      if (subOutEltsBeforeMainWord.back().out.empty())
        subOutEltsBeforeMainWord.pop_back();
      if (subOutEltsAfterMainWord.back().out.empty())
        subOutEltsAfterMainWord.pop_back();
      if (subOutEltsSubordinate.back().out.empty())
        subOutEltsSubordinate.pop_back();
    }

    if (modifierListExp->listType == ListExpressionType::UNRELATED)
    {
      for (auto& currOutElt : subOutEltsBeforeDeterminer)
        pOutBeforeDeterminer.out.splice(pOutBeforeDeterminer.out.end(), currOutElt.out);
      for (auto& currOutElt : subOutEltsBeforeMainWord)
        pOutBeforeMainWord.out.splice(pOutBeforeMainWord.out.end(), currOutElt.out);
      for (auto& currOutElt : subOutEltsAfterMainWord)
        pOutAfterMainWord.out.splice(pOutAfterMainWord.out.end(), currOutElt.out);
      for (auto& currOutElt : subOutEltsSubordinate)
        pOutSubordinate.out.splice(pOutSubordinate.out.end(), currOutElt.out);
    }
    else
    {
      _fillOutBlocsList(pOutBeforeDeterminer.out, subOutEltsBeforeDeterminer,
                        modifierListExp->listType);
      _fillOutBlocsList(pOutBeforeMainWord.out, subOutEltsBeforeMainWord,
                        modifierListExp->listType);
      _fillOutBlocsList(pOutAfterMainWord.out, subOutEltsAfterMainWord,
                        modifierListExp->listType);
      _fillOutBlocsList(pOutSubordinate.out, subOutEltsSubordinate,
                        modifierListExp->listType);
    }
    return;
  }
}


RelativePerson LinguisticSynthesizerPrivate::_grdExpToRelativePerson
(const GroundedExpression& pGrdExp,
 const SynthesizerConfiguration& pConf,
 bool pIsANameAssignement) const
{
  if (ConceptSet::haveAConcept(pGrdExp->concepts, "tolink_1p"))
    return RelativePerson::FIRST_PLUR;

  switch (pGrdExp.grounding().type)
  {
  case SemanticGroudingType::AGENT:
  {
    return getSyntGrounding().agentTypeToRelativePerson
        (pGrdExp->getAgentGrounding(), pConf, !pIsANameAssignement);
  }
  case SemanticGroudingType::GENERIC:
  {
    const SemanticGenericGrounding& genGrd = pGrdExp->getGenericGrounding();
    return (genGrd.entityType != SemanticEntityType::NUMBER && genGrd.quantity.isPlural()) ?
          RelativePerson::THIRD_PLUR : RelativePerson::THIRD_SING;
  }
  default:
  {
    return RelativePerson::THIRD_SING;
  }
  }
}


void LinguisticSynthesizerPrivate::_fillOutBlocsList
(std::list<WordToSynthesize>& pOut,
 std::list<OutSemExp>& pSubOutElts,
 ListExpressionType pListType) const
{
  std::size_t nbElts = pSubOutElts.size();
  std::size_t currId = 0;
  for (auto itOutElt = pSubOutElts.begin();
       itOutElt != pSubOutElts.end(); ++itOutElt)
  {
    ++currId;
    if (itOutElt != pSubOutElts.begin())
      getSyntGrounding().writeListSeparators(pOut, pListType, currId == nbElts);
    pOut.splice(pOut.end(), itOutElt->out);
  }
}


} // End of namespace onsem
