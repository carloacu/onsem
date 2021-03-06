#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include <onsem/semantictotext/type/naturallanguageexpression.hpp>
#include "linguisticsynthesizer/linguisticsynthesizer.hpp"
#include "conversion/occurrencerankconverter.hpp"
#include "conversion/simplesentencesplitter.hpp"
#include "conversion/reasonofrefactor.hpp"
#include "interpretation/completewithcontext.hpp"
#include "linguisticsynthesizer/synthesizerresulttypes.hpp"
#include "utility/semexpcreator.hpp"


namespace onsem
{
namespace converter
{

namespace
{
const SemanticMemoryBlock _emptyMemBlock(0);

void _completeWithContextInternallyToASemExp(SemanticExpression& pSemExp,
                                             const linguistics::LinguisticDatabase& pLingDb,
                                             const SemanticExpression* pAuthorSemExpPtr)
{
  // complete with context between each sentence
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp.getGrdExp();
    auto itSubject = grdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != grdExp.children.end() &&
        SemExpGetter::doesSemExpCanBeCompletedWithContext(*itSubject->second))
      for (auto& currChild : grdExp.children)
        if (currChild.first != GrammaticalType::SUBJECT)
          for (auto& currElt : SemExpGetter::iterateOnListOfGrdExps(*currChild.second))
            for (auto& currEltChild : currElt->children)
              completeWithContext(currEltChild.second, currEltChild.first, *itSubject->second,
                                  true, pAuthorSemExpPtr, _emptyMemBlock, pLingDb);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp.getListExp();
    for (auto& currElt : listExp.elts)
      _completeWithContextInternallyToASemExp(*currElt, pLingDb, pAuthorSemExpPtr);

    auto itPrevElt = listExp.elts.begin();
    if (itPrevElt != listExp.elts.end())
    {
      auto it = itPrevElt;
      ++it;
      while (it != listExp.elts.end())
      {
        completeWithContext(*it, GrammaticalType::UNKNOWN, **itPrevElt, true, pAuthorSemExpPtr, _emptyMemBlock, pLingDb);
        itPrevElt = it;
        ++it;
      }
    }
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    ConditionExpression& condExp = pSemExp.getCondExp();
    _completeWithContextInternallyToASemExp(*condExp.conditionExp, pLingDb, pAuthorSemExpPtr);
    _completeWithContextInternallyToASemExp(*condExp.thenExp, pLingDb, pAuthorSemExpPtr);
    if (condExp.elseExp)
      _completeWithContextInternallyToASemExp(**condExp.elseExp, pLingDb, pAuthorSemExpPtr);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fbkExp = pSemExp.getFdkExp();
    _completeWithContextInternallyToASemExp(*fbkExp.concernedExp, pLingDb, pAuthorSemExpPtr);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = pSemExp.getIntExp();
    _completeWithContextInternallyToASemExp(*intExp.interpretedExp, pLingDb, pAuthorSemExpPtr);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pSemExp.getAnnExp();
    _completeWithContextInternallyToASemExp(*annExp.semExp, pLingDb, pAuthorSemExpPtr);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metaExp = pSemExp.getMetadataExp();
    pAuthorSemExpPtr = metaExp.getAuthorSemExpPtr();
    _completeWithContextInternallyToASemExp(*metaExp.semExp, pLingDb, pAuthorSemExpPtr);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}

UniqueSemanticExpression _syntGraphToSemExp(const linguistics::SyntacticGraph& pSyntGraph,
                                            const TextProcessingContext& pLocutionContext,
                                            const SemanticTimeGrounding& pNowTimeGrd,
                                            bool pDoWeSplitQuestions,
                                            std::list<std::list<SemLineToPrint> >* pDebugOutput,
                                            std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>())
{
  auto semExp  = linguistics::convertToSemExp(pSyntGraph, pLocutionContext, pNowTimeGrd,
                                              std::move(pAgentWeAreTalkingAbout));
  splitter::splitInVerySimpleSentences(semExp, pDoWeSplitQuestions);
  semanticReasonOfRefactor::process(semExp);
  pSyntGraph.langConfig.lingDb.treeConverter.refactorSemExp
      (semExp,
       TREEPATTERN_INTEXT, TREEPATTERN_MIND,
       pSyntGraph.langConfig.getLanguageType(), pDebugOutput);
  _completeWithContextInternallyToASemExp(*semExp, pSyntGraph.langConfig.lingDb, nullptr);
  return semExp;
}


void _naturalLanguageTextToSemanticWord(
    SemanticWord& pWord,
    const NaturalLanguageText& pNaturalLanguageText)
{
  pWord.lemma = pNaturalLanguageText.text;
  if (pNaturalLanguageText.type == NaturalLanguageTypeOfText::VERB)
    pWord.partOfSpeech = PartOfSpeech::VERB;
  else
    pWord.partOfSpeech = PartOfSpeech::NOUN;
  pWord.language = pNaturalLanguageText.language;
}

}


void splitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp,
                                   const linguistics::LinguisticDatabase& pLingDb,
                                   std::list<std::list<SemLineToPrint>>* pDebugOutput)
{
  semanticOccurrenceRankConverter::process(pSemExp);
  // TODO: do a refactor inside splitPossibilitiesOfQuestion to not make difference between languages
  pLingDb.treeConverter.splitPossibilitiesOfQuestions(pSemExp, SemanticLanguageEnum::UNKNOWN,
                                                      pDebugOutput);
}


void splitEquivalentQuestions(UniqueSemanticExpression& pSemExp,
                              const linguistics::LinguisticDatabase& pLingDb,
                              std::list<std::list<SemLineToPrint>>* pDebugOutput)
{
  pLingDb.treeConverter.splitEquivalentQuestions(pSemExp, SemanticLanguageEnum::UNKNOWN,
                                                 pDebugOutput);
}


void unsplitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pSemExp->getAnnExp();
    unsplitPossibilitiesOfQuestions(annExp.semExp);
    break;
  }
  case SemanticExpressionType::COMMAND:
  {
    auto& cmdExp = pSemExp->getCmdExp();
    unsplitPossibilitiesOfQuestions(cmdExp.semExp);
    break;
  }
  case SemanticExpressionType::COMPARISON:
  {
    auto& compExp = pSemExp->getCompExp();
    unsplitPossibilitiesOfQuestions(compExp.leftOperandExp);
    if (compExp.rightOperandExp)
      unsplitPossibilitiesOfQuestions(*compExp.rightOperandExp);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp->getCondExp();
    unsplitPossibilitiesOfQuestions(condExp.conditionExp);
    unsplitPossibilitiesOfQuestions(condExp.thenExp);
    if (condExp.elseExp)
      unsplitPossibilitiesOfQuestions(*condExp.elseExp);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkExp = pSemExp->getFdkExp();
    unsplitPossibilitiesOfQuestions(fdkExp.concernedExp);
    unsplitPossibilitiesOfQuestions(fdkExp.feedbackExp);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    auto& grdExp = pSemExp->getGrdExp();
    for (auto& currChild : grdExp.children)
      unsplitPossibilitiesOfQuestions(currChild.second);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pSemExp->getIntExp();
    unsplitPossibilitiesOfQuestions(intExp.interpretedExp);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      unsplitPossibilitiesOfQuestions(currElt);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pSemExp->getMetadataExp();
    unsplitPossibilitiesOfQuestions(metadataExp.semExp);
    if (metadataExp.source)
      unsplitPossibilitiesOfQuestions(*metadataExp.source);
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFromPtr = pSemExp->getSetOfFormsExp().getOriginalForm();
    if (originalFromPtr != nullptr)
    {
      pSemExp = std::move(*originalFromPtr);
      unsplitPossibilitiesOfQuestions(pSemExp);
    }
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
    break;
  }
}


UniqueSemanticExpression textToSemExp(const std::string& pText,
                                      const TextProcessingContext& pTextProcContext,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      bool pDoWeSplitQuestions,
                                      SemanticLanguageEnum* pExtractedLanguagePtr,
                                      std::unique_ptr<SemanticTimeGrounding>* pNowTimePtr,
                                      const std::list<std::string>* pReferencesPtr,
                                      std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout)
{
  SemanticLanguageEnum language = [&]
  {
    if (pTextProcContext.langType == SemanticLanguageEnum::UNKNOWN)
      return linguistics::getLanguage(pText, pLingDb);
    return pTextProcContext.langType;
  }();
  if (pExtractedLanguagePtr != nullptr)
    *pExtractedLanguagePtr = language;

  auto nowTimeGrd = SemanticTimeGrounding::nowInstance();
  const SemanticTimeGrounding& nowTimeRef = [&]
  {
    if (pNowTimePtr != nullptr)
    {
      *pNowTimePtr = std::move(nowTimeGrd);
      return **pNowTimePtr;
    }
    return *nowTimeGrd;
  }();
  linguistics::SyntacticGraph syntGraph(pLingDb, language);
  linguistics::tokenizationAndSyntacticalAnalysis(syntGraph, pText,
                                                  pTextProcContext.spellingMistakeTypesPossible,
                                                  pTextProcContext.cmdGrdExtractorPtr);
  auto resSemExp = _syntGraphToSemExp(syntGraph, pTextProcContext, nowTimeRef,
                                      pDoWeSplitQuestions, nullptr, std::move(pAgentWeAreTalkingAbout));

  if (pReferencesPtr != nullptr)
  {
    auto res = mystd::make_unique<MetadataExpression>
        (SemanticSourceEnum::UNKNOWN, UniqueSemanticExpression(), std::move(resSemExp));
      res->references = *pReferencesPtr;
    return std::move(res);
  }
  return resSemExp;
}


std::unique_ptr<MetadataExpression> wrapSemExpWithContextualInfos
(UniqueSemanticExpression pSemExp,
 const std::string& pText,
 const TextProcessingContext& pLocutionContext,
 SemanticSourceEnum pFrom,
 SemanticLanguageEnum pLanguage,
 std::unique_ptr<SemanticTimeGrounding> pNowTimeGrd,
 const std::list<std::string>* pReferencesPtr)
{
  assert(pNowTimeGrd);
  auto source = MetadataExpression::constructSourceFromSourceEnum(
        mystd::make_unique<SemanticAgentGrounding>(pLocutionContext.author),
        mystd::make_unique<SemanticAgentGrounding>(pLocutionContext.receiver),
        pFrom, std::move(pNowTimeGrd));

  IndexToSubNameToParameterValue params;
  params[0].emplace("",
                    mystd::make_unique<ReferenceOfSemanticExpressionContainer>(*pSemExp));
  static const std::set<SemanticExpressionType> expressionTypesToSkip{SemanticExpressionType::SETOFFORMS};
  source = source->clone(&params, true, &expressionTypesToSkip);
  if (SemExpGetter::doesSemExpContainsOnlyARequest(*pSemExp))
    SemExpModifier::replaceSayByAskToRobot(*source);

  auto res = mystd::make_unique<MetadataExpression>
      (pFrom, std::move(source), std::move(pSemExp));
  res->fromLanguage = pLanguage;
  res->fromText = pText;
  if (pReferencesPtr != nullptr)
    res->references = *pReferencesPtr;
  return res;
}


UniqueSemanticExpression textToContextualSemExp
(const std::string& pText,
 const TextProcessingContext& pLocutionContext,
 SemanticSourceEnum pFrom,
 const linguistics::LinguisticDatabase& pLingDb,
 const std::list<std::string>* pReferencesPtr,
 std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout)
{
  SemanticLanguageEnum language = SemanticLanguageEnum::UNKNOWN;
  std::unique_ptr<SemanticTimeGrounding> nowTimeGrd;
  auto semExp = textToSemExp(pText, pLocutionContext, pLingDb, false, &language, &nowTimeGrd,
                             nullptr, std::move(pAgentWeAreTalkingAbout));
  return wrapSemExpWithContextualInfos(std::move(semExp), pText, pLocutionContext, pFrom, language,
                                       std::move(nowTimeGrd), pReferencesPtr);
}


UniqueSemanticExpression naturalLanguageExpressionToSemanticExpression(const NaturalLanguageExpression& pNaturalLanguageExpression,
                                                                       const linguistics::LinguisticDatabase& pLingDb,
                                                                       const std::vector<std::string>& pResourceLabels)
{
  if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::EXPRESSION)
  {
    auto textProc = TextProcessingContext::getTextProcessingContextFromRobot(pNaturalLanguageExpression.word.language);
    if (!pResourceLabels.empty())
      textProc.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(pResourceLabels);
    return textToSemExp(pNaturalLanguageExpression.word.text, textProc, pLingDb);
  }

  auto res = mystd::make_unique<GroundedExpression>(
        [&]() -> std::unique_ptr<SemanticGrounding>
        {
          if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::VERB)
          {
            auto statGrd = mystd::make_unique<SemanticStatementGrounding>();
            _naturalLanguageTextToSemanticWord(statGrd->word, pNaturalLanguageExpression.word);
            statGrd->verbTense = pNaturalLanguageExpression.verbTense;
            statGrd->verbGoal = pNaturalLanguageExpression.verbGoal;
            statGrd->polarity = pNaturalLanguageExpression.polarity;
            pLingDb.langToSpec[pNaturalLanguageExpression.word.language].lingDico.getConcepts(statGrd->concepts, statGrd->word);
            return statGrd;
          }
          if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::AGENT)
          {
            return mystd::make_unique<SemanticAgentGrounding>(pNaturalLanguageExpression.word.text);
          }
          if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::QUOTE)
          {
            return mystd::make_unique<SemanticTextGrounding>(pNaturalLanguageExpression.word.text);
          }
          auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
          _naturalLanguageTextToSemanticWord(genGrd->word, pNaturalLanguageExpression.word);
          genGrd->quantity = pNaturalLanguageExpression.quantity;
          genGrd->referenceType = pNaturalLanguageExpression.reference;
          pLingDb.langToSpec[pNaturalLanguageExpression.word.language].lingDico.getConcepts(genGrd->concepts, genGrd->word);
          return genGrd;
        }());
  for (const auto& currChild : pNaturalLanguageExpression.children)
    res->children.emplace(currChild.first, naturalLanguageExpressionToSemanticExpression(currChild.second, pLingDb));
  return std::move(res);
}


UniqueSemanticExpression agentIdWithNameToSemExp(const std::string& pAgentId,
                                                 const std::vector<std::string>& pNames)
{
  // fill verb
  auto res = mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PRESENT;
    statementGrd->concepts.emplace(ConceptSet::conceptVerbEquality, 4);
    return statementGrd;
  }());

  // fill subject
  res->children.emplace(GrammaticalType::SUBJECT, mystd::make_unique<GroundedExpression>(
                          mystd::make_unique<SemanticAgentGrounding>(pAgentId)));

  // fill object
  res->children.emplace(GrammaticalType::OBJECT, mystd::make_unique<GroundedExpression>(
                          mystd::make_unique<SemanticNameGrounding>(pNames)));

  return std::move(res);
}



UniqueSemanticExpression syntGraphToSemExp(const linguistics::SyntacticGraph& pSyntGraph,
                                           const TextProcessingContext& pLocutionContext,
                                           std::list<std::list<SemLineToPrint> >* pDebugOutput)
{
  auto nowTimeGrd = SemanticTimeGrounding::nowInstance();
  return _syntGraphToSemExp(pSyntGraph, pLocutionContext, *nowTimeGrd, false, pDebugOutput);
}



void semExpToSentiments(std::list<std::unique_ptr<SentimentContext>>& pSentInfos,
                        const SemanticExpression& pSemExp,
                        const ConceptSet& pConceptSet)
{
  const SemanticAgentGrounding* authorPtr = SemExpGetter::extractAuthor(pSemExp);
  if (authorPtr != nullptr)
    sentimentDetector::semExpToSentimentInfos
        (pSentInfos, pSemExp, *authorPtr, pConceptSet);
}



void semExpToText(std::string& pResStr,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemoryBlock& pMemBlock,
                  const std::string& pCurrentUserId,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint> >* pDebugOutput)
{
  std::list<std::unique_ptr<SynthesizerResult>> res;
  synthesize(res, std::move(pSemExp), pOneLinePerSentence,
             pMemBlock, pCurrentUserId, pTextProcContext, pLingDb, pDebugOutput);
  for (const auto& currElt : res)
  {
    switch (currElt->type)
    {
    case SynthesizerResultEnum::TEXT:
    {
      const auto& syntText = *dynamic_cast<const SynthesizerText*>(&*currElt);
      pResStr += syntText.text;
      break;
    }
    case SynthesizerResultEnum::TASK:
    {
      const auto& syntTask = *dynamic_cast<const SynthesizerTask*>(&*currElt);
      pResStr += syntTask.resource.value;
      break;
    }
    }
  }
}


void semExpToText(std::string& pResStr,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint> >* pDebugOutput)
{
  semExpToText(pResStr, std::move(pSemExp), pTextProcContext, pOneLinePerSentence,
               pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(), pLingDb, pDebugOutput);
}




void getInfinitiveToTwoDifferentPossibleWayToAskForIt(UniqueSemanticExpression& pOut1,
                                                      UniqueSemanticExpression& pOut2,
                                                      UniqueSemanticExpression pUSemExp)
{
  {
    auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
     pOut1 = SemExpCreator::getImperativeAssociateFrom(*grdExpPtr);
  }

  {
    auto* grdExpPtr = pOut1->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
      pOut2 = SemExpCreator::iWantThatYou(SemanticAgentGrounding::currentUser, SemExpCreator::getIndicativeFromImperative(*grdExpPtr));
  }
}


UniqueSemanticExpression getFutureIndicativeAssociatedForm(UniqueSemanticExpression pUSemExp)
{
  auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    return SemExpCreator::getFutureIndicativeAssociatedForm(*grdExpPtr);

  auto* listExpPtr = pUSemExp->getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    auto& listExp = *listExpPtr;
    auto res = std::make_unique<ListExpression>(listExp.listType);
    for (auto& currElt : listExp.elts)
      res->elts.push_back(getFutureIndicativeAssociatedForm(std::move(currElt)));
    return std::move(res);
  }
  return UniqueSemanticExpression();
}



std::unique_ptr<UniqueSemanticExpression> imperativeToIWantThatYou(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
    if (statGrdPtr != nullptr)
    {
      auto& statGrd = *statGrdPtr;
      if (statGrd.requests.has(SemanticRequestType::ACTION))
        return std::make_unique<UniqueSemanticExpression>(
              SemExpCreator::iWantThatYou(SemanticAgentGrounding::currentUser, SemExpCreator::getIndicativeFromImperative(*grdExpPtr)));
    }
  }
  return {};
}


std::unique_ptr<UniqueSemanticExpression> imperativeToInfinitive(const SemanticExpression& pSemExp)
{
  auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
    if (statGrdPtr != nullptr)
    {
      auto& statGrd = *statGrdPtr;
      if (statGrd.requests.has(SemanticRequestType::ACTION))
        return std::make_unique<UniqueSemanticExpression>(SemExpCreator::getInfinitiveFromImperativeForm(*grdExpPtr));
    }
  }
  return {};
}


UniqueSemanticExpression constructTeachSemExp(
    UniqueSemanticExpression pInfitiveLabelSemExp,
    UniqueSemanticExpression pSemExpToDo)
{
  auto res = std::make_unique<GroundedExpression>();
  res->children.emplace(GrammaticalType::PURPOSE, std::move(pInfitiveLabelSemExp));
  res->children.emplace(GrammaticalType::OBJECT, std::move(pSemExpToDo));
  return std::move(res);
}


void addOtherTriggerFormulations(std::list<UniqueSemanticExpression>& pRes,
                                 const SemanticExpression& pSemExp)
{
  auto iWantThatYou = imperativeToIWantThatYou(pSemExp);
  if (iWantThatYou)
    pRes.emplace_back(std::move(*iWantThatYou));
  auto inf = imperativeToInfinitive(pSemExp);
  if (inf)
    pRes.emplace_back(std::move(*inf));
}


} // End of namespace converter
} // End of namespace onsem
