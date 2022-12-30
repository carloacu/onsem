#include "semanticmemorylinker.hpp"
#include <iostream>
#include <onsem/common/binary/enummapreader.hpp>
#include <onsem/common/utility/container.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include "../../semanticmemory/semanticannotation.hpp"
#include "../../semanticmemory/semanticmemoryblockbinaryreader.hpp"
#include "../../semanticmemory/semanticmemoryblockprivate.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../utility/semexpcreator.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../../type/answerexp.hpp"
#include "../semexpcontroller.hpp"
#include "answerfromdatastoredinsidethequestion.hpp"
#include "howyouthatanswer.hpp"
#include "managechoice.hpp"
#include "semanticmemorygetter.hpp"
#include "answertospecificquestions.hpp"
#include "unknowninfosgetter.hpp"
#include "specificactionshandler.hpp"
#include "../../tool/semexpsimilaritycoef.hpp"

namespace onsem
{
namespace semanticMemoryLinker
{

namespace
{
const SemanticTriggerAxiomId _emptyAxiomId;


template<typename GRDEXP, typename GENGRD>
void _incrementNotDefiniteQuantitiesFromGrdExps(GRDEXP& pGrdExpRes,
                                                const GroundedExpression& pGrdExp,
                                                const SemanticMemoryBlock& pMemBlock,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                std::map<GENGRD*, std::map<const GroundedExpression*, int>>& pAlreadyCountedElts);

template<typename SEMEXP, typename GENGRD>
void _incrementQuantities(SEMEXP& pSemExpRes,
                          const SemanticExpression& pSemExp,
                          const SemanticMemoryBlock& pMemBlock,
                          const linguistics::LinguisticDatabase& pLingDb,
                          std::map<GENGRD*, std::map<const GroundedExpression*, int>>& pAlreadyCountedElts)
{
  auto* grdExpResPtr = pSemExpRes.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpResPtr != nullptr)
  {
    const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
      _incrementNotDefiniteQuantitiesFromGrdExps(*grdExpResPtr, *grdExpPtr,
                                                 pMemBlock, pLingDb, pAlreadyCountedElts);
  }
}

template<typename GRDEXP, typename GENGRD>
void _incrementNotDefiniteQuantitiesFromGrdExps(GRDEXP& pGrdExpRes,
                                                const GroundedExpression& pGrdExp,
                                                const SemanticMemoryBlock& pMemBlock,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                std::map<GENGRD*, std::map<const GroundedExpression*, int>>& pAlreadyCountedElts)
{
  auto* genGrdResPtr = pGrdExpRes->getGenericGroundingPtr();
  if (genGrdResPtr != nullptr &&
      genGrdResPtr->referenceType != SemanticReferenceType::DEFINITE &&
      genGrdResPtr->quantity.type == SemanticQuantityType::NUMBER)
  {
    auto nb = SemExpGetter::getNumberOfElementsFromGrdExp(pGrdExp);
    if (nb)
    {
      auto itForCurrGenGrd = pAlreadyCountedElts.find(genGrdResPtr);
      if (itForCurrGenGrd != pAlreadyCountedElts.end())
      {
        for (auto& currAlreadyCountedElt : itForCurrGenGrd->second)
        {
          SemExpComparator::ComparisonExceptions compExceptions;
          compExceptions.quantity = true;
          auto imbrication = SemExpComparator::getGrdExpsImbrications(pGrdExp,
                                                                      *currAlreadyCountedElt.first,
                                                                      pMemBlock, pLingDb, &compExceptions);
          if (imbrication == ImbricationType::EQUALS ||
              imbrication == ImbricationType::LESS_DETAILED ||
              imbrication == ImbricationType::HYPERNYM)
          {
            break;
          }
          else
          {
            itForCurrGenGrd->second.emplace(&pGrdExp, *nb);
          }
        }
      }
      else
      {
        pAlreadyCountedElts[genGrdResPtr].emplace(&pGrdExp, *nb);
      }
    }
  }

  if (!pGrdExpRes.children.empty() &&
      SemExpComparator::grdsHaveSamePolarity(*pGrdExpRes, *pGrdExp, pLingDb.conceptSet))
  {
    for (auto& currEltRes : pGrdExpRes.children)
    {
      auto itChild = pGrdExp.children.find(currEltRes.first);
      if (itChild != pGrdExp.children.end())
        _incrementQuantities(*currEltRes.second, *itChild->second, pMemBlock, pLingDb, pAlreadyCountedElts);
    }
  }
}

bool _doAnActionLinkedToACondition(SemControllerWorkingStruct& pWorkStruct,
                                   SemanticMemoryBlockViewer& pMemViewer,
                                   const SemanticExpression& pSemExpToDo)
{
  SemControllerWorkingStruct subWorkStruct(pWorkStruct);
  if (subWorkStruct.askForNewRecursion())
  {
    subWorkStruct.reactOperator = SemanticOperatorEnum::RESOLVECOMMAND;
    controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, pSemExpToDo);
    pWorkStruct.addAnswers(subWorkStruct);
    return true;
  }
  return false;
}

template<bool IS_MODIFIABLE>
bool _doActions(SemControllerWorkingStruct& pWorkStruct,
                SemanticMemoryBlockViewer& pMemViewer,
                const GroundedExpression* pGrdExpPtr,
                const SentenceLinks<IS_MODIFIABLE>& pSentsWithAction)
{
  auto memSentenceIsTrue = [&](const GroundedExpWithLinks& pMemSent)
  {
    if (pWorkStruct.expHandleInMemory != nullptr ||
        pWorkStruct.reactOperator == SemanticOperatorEnum::UNINFORM)
    {
      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;
        controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, pMemSent.grdExp, {}, pMemSent.grdExp);
        return subWorkStruct.agreementTypeOfTheAnswer();
      }
    }
    else
    {
      if (pGrdExpPtr == nullptr)
        return TruenessValue::VAL_TRUE;
      ImbricationType imbr =
          SemExpComparator::getGrdExpsImbrications(*pGrdExpPtr, pMemSent.grdExp, pMemViewer.constView, pWorkStruct.lingDb, nullptr);
      if (imbr == ImbricationType::EQUALS ||
          imbr == ImbricationType::ISCONTAINED ||
          imbr == ImbricationType::MORE_DETAILED ||
          imbr == ImbricationType::HYPONYM)
        return TruenessValue::VAL_TRUE;
      return TruenessValue::VAL_FALSE;
    }
    return TruenessValue::UNKNOWN;
  };

  bool res = false;
  for (auto& currSent : pSentsWithAction.dynamicLinks)
  {
    const GroundedExpWithLinks& memSent = *currSent.second;
    const SentenceWithLinks& contextAxiom = memSent.getContextAxiom();
    const auto& semTrackerOpt = contextAxiom.semTracker;
    if (semTrackerOpt)
    {
      SemanticTracker& semTracker = **semTrackerOpt;
      const auto* requestsPtr = SemExpGetter::getRequestList(memSent.grdExp);
      if (requestsPtr != nullptr &&
          !requestsPtr->empty() &&
          !requestsPtr->has(SemanticRequestType::YESORNO) &&
          pGrdExpPtr != nullptr &&
          pGrdExpPtr->children.count(semanticRequestType_toSemGram(requestsPtr->firstOrNothing())) > 0)
      {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        if (subWorkStruct.askForNewRecursion())
        {
          subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
          controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, memSent.grdExp, {}, memSent.grdExp);
          std::list<std::unique_ptr<SemAnswer>> detailledAnswers;
          controller::convertToDetalledAnswer(detailledAnswers, subWorkStruct);
          std::list<const GroundedExpression*> grdExpAnswers;
          CompositeSemAnswer::getGrdExps(grdExpAnswers, detailledAnswers);
          if (grdExpAnswers.empty())
            semTracker.val(UniqueSemanticExpression());
          else
            semTracker.val(SemExpModifier::grdExpsToUniqueSemExp(grdExpAnswers));
        }
        continue;
      }
    }
    {
      const SemanticExpression* semExpToDo = contextAxiom.semExpToDo;
      const SemanticExpression* semExpToDoElse = contextAxiom.semExpToDoElse;
      assert(semTrackerOpt || semExpToDo != nullptr || semExpToDoElse != nullptr);
      TruenessValue newTruenessValue = memSentenceIsTrue(memSent);
      if (pWorkStruct.axiomToConditionCurrentStatePtr != nullptr)
      {
        TruenessValue* conditionCurrentState = nullptr;
        auto itCA = pWorkStruct.axiomToConditionCurrentStatePtr->find(&contextAxiom);
        if (itCA != pWorkStruct.axiomToConditionCurrentStatePtr->end())
          conditionCurrentState = &itCA->second;
        else {
          conditionCurrentState = &(*pWorkStruct.axiomToConditionCurrentStatePtr)[&contextAxiom];
          *conditionCurrentState = TruenessValue::UNKNOWN;
        }
        if (*conditionCurrentState == newTruenessValue)
          continue;
        *conditionCurrentState = newTruenessValue;
      }

      switch (newTruenessValue)
      {
      case TruenessValue::VAL_TRUE:
      {
        if (semTrackerOpt)
        {
          SemanticTracker& semTracker = **semTrackerOpt;
          const auto* requestsPtr = SemExpGetter::getRequestList(memSent.grdExp);
          if (requestsPtr != nullptr)
          {
            if (requestsPtr->has(SemanticRequestType::YESORNO))
              semTracker.val(SemExpCreator::sayTrue());
            continue;
          }
          semTracker.val(UniqueSemanticExpression());
        }
        else if (semExpToDo != nullptr)
          res = _doAnActionLinkedToACondition(pWorkStruct, pMemViewer, *semExpToDo) || res;
        break;
      }
      case TruenessValue::VAL_FALSE:
      {
        if (semTrackerOpt)
        {
          SemanticTracker& semTracker = **semTrackerOpt;
          const auto* requestsPtr = SemExpGetter::getRequestList(memSent.grdExp);
          if (requestsPtr != nullptr &&
              requestsPtr->has(SemanticRequestType::YESORNO))
            semTracker.val(SemExpCreator::sayFalse());
        }
        else if (semExpToDoElse != nullptr)
          res = _doAnActionLinkedToACondition(pWorkStruct, pMemViewer, *semExpToDoElse) || res;
        break;
      }
      default:
        break;
      }
    }
  }
  return res;
}


bool _checkGlobalCondition(const SemControllerWorkingStruct& pWorkStruct,
                           const SemanticMemoryBlockViewer& pMemViewer,
                           const GroundedExpWithLinks& pCurrMemSent)
{
  const SentenceWithLinks& contextAxiom = pCurrMemSent.getContextAxiom();

  if (contextAxiom.memorySentences.elts.size() > 1 &&
      contextAxiom.memorySentences.and_or)
  {
    bool res = true;
    for (const GroundedExpWithLinks& currElt : contextAxiom.memorySentences.elts)
    {
      if (&currElt == &pCurrMemSent ||
          currElt.isANoun() ||
          !currElt.isAConditionToSatisfy())
        continue;

      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;
        SemanticMemoryBlockViewer subMemViewer(pMemViewer);
        controller::applyOperatorOnGrdExp(subWorkStruct, subMemViewer, currElt.grdExp, {}, currElt.grdExp);
        if (subWorkStruct.agreementTypeOfTheAnswer() != TruenessValue::VAL_TRUE)
        {
          res = false;
          break;
        }
      }
      else
      {
        res = false;
        break;
      }
    }
    return res;
  }
  return true;
}


bool _areTimeInformationIncompatable(const SemControllerWorkingStruct& pWorkStruct,
                                     const SemanticMemoryBlockViewer& pMemViewer,
                                     const GroundedExpression& pMemGrdExp,
                                     const GroundedExpression* pFromGrdExpQuestion,
                                     SemanticRequestType pFormRequest)
{
  if (pFormRequest != SemanticRequestType::TIME &&
      pFromGrdExpQuestion != nullptr &&
      pFromGrdExpQuestion->children.count(GrammaticalType::TIME) == 0)
  {
    const SemanticStatementGrounding* questionStatGrdPtr = (*pFromGrdExpQuestion)->getStatementGroundingPtr();
    if (questionStatGrdPtr != nullptr &&
        questionStatGrdPtr->verbTense == SemanticVerbTense::PRESENT)
    {
      auto itTimeMemGrdExp = pMemGrdExp.children.find(GrammaticalType::TIME);
      if (itTimeMemGrdExp != pMemGrdExp.children.end())
      {
        auto* timeGrdExpPtr = itTimeMemGrdExp->second->getGrdExpPtr_SkipWrapperPtrs();
        if (timeGrdExpPtr == nullptr ||
            timeGrdExpPtr->grounding().type == SemanticGroundingType::STATEMENT)
        {
          SemControllerWorkingStruct subWorkStruct(pWorkStruct);
          if (subWorkStruct.askForNewRecursion())
          {
            subWorkStruct.reactOperator = SemanticOperatorEnum::CHECK;
            SemanticMemoryBlockViewer subMemViewer(pMemViewer);
            controller::applyOperatorOnSemExp(subWorkStruct, subMemViewer, *itTimeMemGrdExp->second);
            if (subWorkStruct.agreementTypeOfTheAnswer() != TruenessValue::VAL_TRUE)
              return true;
          }
        }
      }
    }
  }
  return false;
}


template <typename ANSWERTYPE>
void _filterWithOccurenceRank(std::map<semIdAbs, ANSWERTYPE>& pRes,
                              const SemanticExpression& pOccurenceRankFilter)
{
  int rankNb = SemExpGetter::getRank(pOccurenceRankFilter);
  if (rankNb != 0)
  {
    int resSize = static_cast<int>(pRes.size());
    int resIndex = resSize;
    if (rankNb > 0)
    {
      resIndex = rankNb - 1;
    }
    else if (rankNb < 0)
    {
      int ranFromEnd = -rankNb;
      if (ranFromEnd <= resSize)
        resIndex = resSize - ranFromEnd;
    }

    if (resIndex < resSize)
    {
      auto resItToKeep = pRes.begin();
      std::advance(resItToKeep, resIndex);
      if (resItToKeep != pRes.begin())
        pRes.erase(pRes.begin(), resItToKeep);
      auto itNext = resItToKeep;
      ++itNext;
      if (itNext != pRes.end())
        pRes.erase(itNext, pRes.end());
    }
    else
    {
      pRes.clear();
    }
  }
}

template <typename MEM_SENTENCE>
void _filterGlobalConditionImpossibilities(std::map<intSemId, MEM_SENTENCE*>& pRes,
                                           const SemControllerWorkingStruct& pWorkStruct,
                                           const SemanticMemoryBlockViewer& pMemViewer)
{
  for (auto itMemSent = pRes.begin(); itMemSent != pRes.end(); )
  {
    auto& currMemSent = *itMemSent->second;
    if (_checkGlobalCondition(pWorkStruct, pMemViewer, currMemSent))
      ++itMemSent;
    else
      itMemSent = pRes.erase(itMemSent);
  }
}


template <typename MEM_SENTENCE_PTR>
void _filterTimeImpossibilities(std::map<intSemId, MEM_SENTENCE_PTR>& pRes,
                                const SemControllerWorkingStruct& pWorkStruct,
                                const SemanticMemoryBlockViewer& pMemViewer,
                                const GroundedExpression* pFromGrdExpQuestion = nullptr,
                                SemanticRequestType pFormRequest = SemanticRequestType::NOTHING)
{
  for (auto itMemSent = pRes.begin(); itMemSent != pRes.end(); )
  {
    auto& currMemSent = *itMemSent->second;
    if (!_areTimeInformationIncompatable(pWorkStruct, pMemViewer, currMemSent.getGrdExpRef(), pFromGrdExpQuestion, pFormRequest))
      ++itMemSent;
    else
      itMemSent = pRes.erase(itMemSent);
  }
}

template <bool IS_MODIFIABLE>
void _getResultFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRes,
                          RequestToMemoryLinksVirtual<IS_MODIFIABLE>& pReqToList,
                          const SemControllerWorkingStruct& pWorkStruct,
                          MemoryBlockPrivateAccessorPtr<IS_MODIFIABLE>& pMemBlockPrivatePtr,
                          SemanticMemoryBlockViewer& pMemViewer,
                          const RequestLinks& pReqLinks,
                          bool pIsATrigger,
                          semanticMemoryGetter::RequestContext pRequestContext,
                          const GroundedExpression* pFromGrdExpQuestion = nullptr,
                          SemanticRequestType pFormRequest = SemanticRequestType::NOTHING,
                          bool pCheckTimeRequest = true,
                          bool pConsiderCoreferences = false)
{
  semanticMemoryGetter::getResultFromMemory
      (pRes, pReqToList, pReqLinks, pRequestContext, pMemBlockPrivatePtr, pIsATrigger, pWorkStruct.lingDb,
       pCheckTimeRequest, pConsiderCoreferences);
  _filterGlobalConditionImpossibilities(pRes.res.dynamicLinks, pWorkStruct, pMemViewer);
  _filterTimeImpossibilities(pRes.res.dynamicLinks, pWorkStruct, pMemViewer, pFromGrdExpQuestion, pFormRequest);
  _filterTimeImpossibilities(pRes.res.staticLinks, pWorkStruct, pMemViewer, pFromGrdExpQuestion, pFormRequest);
}


bool _addTriggerAnswer(SemControllerWorkingStruct& pWorkStruct,
                       bool& pAnAnswerHasBeenAdded,
                       const ExpressionWithLinks& pExp,
                       const SemanticMemoryBlockViewer& pMemViewer,
                       ContextualAnnotation pContAnnotation)
{
  assert(pExp.outputToAnswerIfTriggerHasMatched);
  const auto& outSemExp = pExp.outputToAnswerIfTriggerHasMatched->getSemExp();
  if (!pWorkStruct.canBeANewAnswer(outSemExp))
    return false;
  UniqueSemanticExpression res = outSemExp.clone();
  std::list<std::string> references;
  SemExpGetter::extractReferences(references, *pExp.semExp);
  simplifier::processFromMemBlock(res, pMemViewer.constView, pWorkStruct.lingDb);
  pWorkStruct.addAnswer(pContAnnotation, std::move(res), ReferencesFiller(references));
  pAnAnswerHasBeenAdded = true;
  return pWorkStruct.isFinished();
}


bool _addTriggerThatMatchTheMost(SemControllerWorkingStruct& pWorkStruct,
                                 bool& pAnAnswerHasBeenAdded,
                                 const std::set<const ExpressionWithLinks*>& pSemExpWrapperPtrs,
                                 const SemanticMemoryBlockViewer& pMemViewer,
                                 ContextualAnnotation pContAnnotation,
                                 const SemanticExpression& pInputSemExp)
{
  std::size_t pSemExpWrapperPtrsSize = pSemExpWrapperPtrs.size();
  if (pSemExpWrapperPtrsSize == 1)
  {
    const auto& exp = **pSemExpWrapperPtrs.begin();
    return _addTriggerAnswer(pWorkStruct, pAnAnswerHasBeenAdded, exp,
                             pMemViewer, pContAnnotation);
  }

  if (pSemExpWrapperPtrsSize > 1)
  {
    std::map<int, std::map<intSemId, const ExpressionWithLinks*>> similarityCoef;
    for (const auto& currRel : pSemExpWrapperPtrs)
      similarityCoef[getSimilarityCoef(pInputSemExp, *currRel->semExp)].emplace(currRel->getIdOfFirstSentence(), currRel);

    bool res = false;
    for (auto itExp = similarityCoef.rbegin(); itExp != similarityCoef.rend(); ++itExp)
    {
      for (auto& currExpHandleInMemory : itExp->second)
      {
        if (_addTriggerAnswer(pWorkStruct, pAnAnswerHasBeenAdded, *currExpHandleInMemory.second,
                              pMemViewer, pContAnnotation))
        {
          if (!pWorkStruct.reactionOptions.canAnswerWithAllTheTriggers)
            return true;
          res = true;
        }
      }
    }
    return res;
  }
  return false;
}


void _matchAnyTrigger
(std::set<const ExpressionWithLinks*>& pSemExpWrapperPtrs,
 SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 RequestToMemoryLinksVirtual<false>& pReqToGrdExps,
 const RequestLinks& pReqLinks,
 const GroundedExpression& pInputGrdExp)
{
  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RelationsThatMatch<false> idsToSentences;
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory(idsToSentences, pReqToGrdExps, pWorkStruct, memBlockPrivateAccessor,
                       pMemViewer, pReqLinks, true, semanticMemoryGetter::RequestContext::SENTENCE,
                       nullptr, SemanticRequestType::NOTHING, true, true);

  std::map<SemExpComparator::ComparisonErrorsCoef, std::set<const ExpressionWithLinks*>> nbOfErrorsToLowPrioritySemExpWrapperPtrs;
  for (const auto& currRel : idsToSentences.res.dynamicLinks)
  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    const GroundedExpWithLinks& memSent = *currRel.second;
    if (SemExpComparator::grdExpsAreEqual(pInputGrdExp, memSent.grdExp, pMemViewer.constView,
                                          pWorkStruct.lingDb, &pWorkStruct.comparisonExceptions, &comparisonErrorReporting))
    {
      pSemExpWrapperPtrs.insert(&memSent.getContextAxiom().getSemExpWrappedForMemory());
    }
    else
    {
      bool canBeAtLowPriority = true;
      for (const auto& currChildrenError : comparisonErrorReporting.childrenThatAreNotEqual)
      {
        if (currChildrenError.first != GrammaticalType::RECEIVER &&
            currChildrenError.first != GrammaticalType::SPECIFIER &&
            currChildrenError.first != GrammaticalType::OTHER_THAN)
        {
          // If it a anything trigger we accept the matching
          bool isAnythingTrigger = false;
          for (const auto& currComparisonErrors : currChildrenError.second)
          {
            for (const auto& currTriggerChild : currComparisonErrors.second.child2Ptr.elts)
            {
              if (SemExpGetter::isAnythingFromSemExp(*currTriggerChild))
              {
                isAnythingTrigger = true;
                break;
              }
            }
            if (isAnythingTrigger)
              break;
          }
          if (!isAnythingTrigger)
            canBeAtLowPriority = false;
          break;
        }
      }
      if (canBeAtLowPriority)
        nbOfErrorsToLowPrioritySemExpWrapperPtrs[comparisonErrorReporting.getErrorCoef()].insert(
              &memSent.getContextAxiom().getSemExpWrappedForMemory());
    }
  }
  if (pSemExpWrapperPtrs.empty() &&
      !nbOfErrorsToLowPrioritySemExpWrapperPtrs.empty())
    pSemExpWrapperPtrs = std::move(nbOfErrorsToLowPrioritySemExpWrapperPtrs.begin()->second);
}


bool _isAValidAnswerForTheQuestionFilter(const GroundedExpression& pGrdExp,
                                         const GroundedExpression* pQuestMetaGrdExp,
                                         SemanticRequestType pRequest,
                                         const ConceptSet& pConceptSet,
                                         const linguistics::LinguisticDatabase& pLingDb)
{
  if (pQuestMetaGrdExp == nullptr)
    return true;
  if (pRequest == SemanticRequestType::QUANTITY)
  {
    if (SemExpGetter::getNumberOfElements(pGrdExp))
      return SemExpComparator::isAnInstanceOf(pGrdExp, *pQuestMetaGrdExp, pLingDb) ||
          SemExpComparator::areGrdExpEqualsExceptForTheQuantity(pGrdExp, *pQuestMetaGrdExp, pConceptSet);
    return false;
  }
  return SemExpComparator::isAnInstanceOf(pGrdExp, *pQuestMetaGrdExp, pLingDb);
}


void _matchTriggerSentences
(std::set<const ExpressionWithLinks*>& pSemExpWrapperPtrs,
 SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 SemanticExpressionCategory pExpCategory,
 const SemanticTriggerAxiomId& pAxiomId,
 const GroundedExpression& pInputGrdExp)
{
  const auto* awLinks = pMemViewer.getConstViewPrivate().getSentenceTriggersLinks(pExpCategory, pAxiomId);
  if (awLinks != nullptr)
  {
    RequestToMemoryLinks<false> linksForVerb(awLinks->getLinks(pReqLinks.tense, pReqLinks.verbGoal).reqToGrdExps, nullptr);
    _matchAnyTrigger(pSemExpWrapperPtrs, pWorkStruct, pMemViewer,
                     linksForVerb, pReqLinks, pInputGrdExp);
    if (!pSemExpWrapperPtrs.empty())
      return;
  }
  if (pMemViewer.constView.subBlockPtr != nullptr)
  {
    SemanticMemoryBlockViewer subMemView(nullptr, *pMemViewer.constView.subBlockPtr, pMemViewer.currentUserId);
    _matchTriggerSentences(pSemExpWrapperPtrs, pWorkStruct, subMemView, pReqLinks,
                           pExpCategory, pAxiomId, pInputGrdExp);
  }
}


void _matchNominalGroupTrigger
(std::set<const ExpressionWithLinks*>& pSemExpWrapperPtrs,
 SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 const SemanticTriggerAxiomId& pAxiomId,
 const GroundedExpression& pInputGrdExp)
{
  const auto* recoLinks = pMemViewer.getConstViewPrivate().getNominalGroupsTriggersLinks(pAxiomId);
  if (recoLinks != nullptr)
  {
    RecommendationMemoryLinks<false> reqToLinks(*recoLinks);
    _matchAnyTrigger(pSemExpWrapperPtrs, pWorkStruct, pMemViewer,
                     reqToLinks, pReqLinks, pInputGrdExp);
    if (!pSemExpWrapperPtrs.empty())
      return;
  }
  if (pMemViewer.constView.subBlockPtr != nullptr)
  {
    SemanticMemoryBlockViewer subMemView(nullptr, *pMemViewer.constView.subBlockPtr, pMemViewer.currentUserId);
    _matchNominalGroupTrigger(pSemExpWrapperPtrs, pWorkStruct, subMemView,
                              pReqLinks, pAxiomId, pInputGrdExp);
  }
}



void _matchGrdExpTrigger
(std::set<const ExpressionWithLinks*>& pSemExpWrapperPtrs,
 SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 SemanticExpressionCategory pExpCategory,
 const SemanticTriggerAxiomId& pAxiomId,
 const GroundedExpression& pInputGrdExp)
{
  if (pExpCategory == SemanticExpressionCategory::NOMINALGROUP)
    _matchNominalGroupTrigger(pSemExpWrapperPtrs, pWorkStruct, pMemViewer,
                             pReqLinks, pAxiomId, pInputGrdExp);
  else
    _matchTriggerSentences(pSemExpWrapperPtrs, pWorkStruct, pMemViewer,
                           pReqLinks, pExpCategory, pAxiomId, pInputGrdExp);
}

bool _addTriggerGrdExps
(SemControllerWorkingStruct& pWorkStruct,
 bool& pAnAnswerHasBeenAdded,
 SemanticMemoryBlockViewer& pMemViewer,
 const std::list<const GroundedExpression*>& pGrdExpPtrs,
 const SemanticExpression& pSemExp,
 const std::function<SemanticTriggerAxiomId(std::size_t)>& pGetAxiomIdFromId)
{
  std::set<const ExpressionWithLinks*> semExpWrapperPtrs;
  std::size_t i = 0;
  for (const auto* currGrdExpPtr : pGrdExpPtrs)
  {
    const auto& currGrdExp = *currGrdExpPtr;
    SemanticExpressionCategory semExpCategory = SemanticExpressionCategory::NOMINALGROUP;
    const auto* statGrdPtr = currGrdExp->getStatementGroundingPtr();
    if (statGrdPtr != nullptr)
    {
      if (statGrdPtr->requests.empty())
        semExpCategory = SemanticExpressionCategory::AFFIRMATION;
      else if (statGrdPtr->requests.firstOrNothing() == SemanticRequestType::ACTION)
        semExpCategory = SemanticExpressionCategory::COMMAND;
      else
        semExpCategory = SemanticExpressionCategory::QUESTION;
    }

    RequestLinks reqLinks;
    getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, currGrdExp, false);
    SemanticTriggerAxiomId axiomId = pGetAxiomIdFromId(i++);
    std::set<const ExpressionWithLinks*> subSemExpWrapperPtrs;
    _matchGrdExpTrigger(subSemExpWrapperPtrs, pWorkStruct, pMemViewer, reqLinks,
                        semExpCategory, axiomId, currGrdExp);
    if (subSemExpWrapperPtrs.empty())
      return false;
    // merge with the gathering set
    if (semExpWrapperPtrs.empty())
    {
      semExpWrapperPtrs = std::move(subSemExpWrapperPtrs);
    }
    else
    {
      auto itSemExpWrapperPtrs = semExpWrapperPtrs.begin();
      while (itSemExpWrapperPtrs != semExpWrapperPtrs.end())
      {
        if (subSemExpWrapperPtrs.count(*itSemExpWrapperPtrs) == 0)
          itSemExpWrapperPtrs = semExpWrapperPtrs.erase(itSemExpWrapperPtrs);
        else
          ++itSemExpWrapperPtrs;
      }
      if (semExpWrapperPtrs.empty())
        return false;
    }
  }
  return _addTriggerThatMatchTheMost(pWorkStruct, pAnAnswerHasBeenAdded, semExpWrapperPtrs, pMemViewer,
                                     ContextualAnnotation::ANSWER, pSemExp);
}

bool _handleActionRelations(SentenceLinks<false>& pIdsToSentences,
                            SemControllerWorkingStruct& pWorkStruct,
                            SemanticMemoryBlockViewer& pMemViewer,
                            const GroundedExpression& pGrdExp)
{
  GrdKnowToUnlinked incompleteRelations;
  if (unknownInfosGetter::splitCompeleteIncompleteOfActions(pWorkStruct, pMemViewer, incompleteRelations,
                                                            pIdsToSentences, pGrdExp))
    return true;

  SemanticRequestType askForThisRequestType = SemanticRequestType::NOTHING;
  if (!incompleteRelations.empty() &&
      unknownInfosGetter::getRequestToAskForPrecision(askForThisRequestType, incompleteRelations))
  {
    pWorkStruct.addQuestion(SemExpCreator::askForPrecision(pGrdExp, askForThisRequestType));
    return true;
  }
  return false;
}


bool _getActionFromAMemblock(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const RequestLinks& pReqLinks,
                             const GroundedExpression& pGrdExp)
{
  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RequestToMemoryLinks<false> reqToGrdExps(memBlockPrivateViewer.sentWithInfActionLinks.reqToGrdExps, nullptr);
  RelationsThatMatch<false> idsToSentences;
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory(idsToSentences, reqToGrdExps, pWorkStruct, memBlockPrivateAccessor, pMemViewer,
                       pReqLinks, false, semanticMemoryGetter::RequestContext::COMMAND,
                       nullptr, SemanticRequestType::NOTHING, false);

  if (!idsToSentences.empty() &&
      _handleActionRelations(idsToSentences.res, pWorkStruct, pMemViewer, pGrdExp))
    return true;

  if (pMemViewer.constView.subBlockPtr != nullptr)
  {
    SemanticMemoryBlockViewer subMemViewer(nullptr, *pMemViewer.constView.subBlockPtr, pMemViewer.currentUserId);
    return _getActionFromAMemblock(pWorkStruct, subMemViewer, pReqLinks, pGrdExp);
  }
  return false;
}

const SemanticExpression* _getActionDefFromAMemblock(
    SemControllerWorkingStruct& pWorkStruct,
    SemanticMemoryBlockViewer& pMemViewer,
    const RequestLinks& pReqLinks,
    const GroundedExpression& pGrdExp)
{
  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RequestToMemoryLinks<false> reqToGrdExps(memBlockPrivateViewer.sentWithInfActionLinks.reqToGrdExps, nullptr);
  RelationsThatMatch<false> idsToSentences;
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory(idsToSentences, reqToGrdExps, pWorkStruct, memBlockPrivateAccessor, pMemViewer,
                       pReqLinks, false, semanticMemoryGetter::RequestContext::COMMAND,
                       nullptr, SemanticRequestType::NOTHING, false);

  if (!idsToSentences.res.dynamicLinks.empty())
  {
    auto itFirstDynLinks = --idsToSentences.res.dynamicLinks.end();
    return itFirstDynLinks->second->getContextAxiom().infCommandToDo;
  }

  if (pMemViewer.constView.subBlockPtr != nullptr)
  {
    SemanticMemoryBlockViewer subMemViewer(nullptr, *pMemViewer.constView.subBlockPtr, pMemViewer.currentUserId);
    return _getActionDefFromAMemblock(pWorkStruct, subMemViewer, pReqLinks, pGrdExp);
  }
  return nullptr;
}

}


void RequestLinks::eraseChild(SemanticRequestType pRequestType)
{
  _semExpLinks.erase(pRequestType);
  for (auto it = _semExpLinksSorted.begin(); it != _semExpLinksSorted.end(); ++it)
  {
    if (it->first == pRequestType)
    {
      _semExpLinksSorted.erase(it);
      return;
    }
  }
}

enum class LinkPosition
{
  FIRST,
  SECOND,
  THIRD,
  BEFORELAST,
  LAST
};

void RequestLinks::fillSortedSemExps()
{
  _semExpLinksSorted.clear();

  std::map<LinkPosition, std::set<SemanticRequestType>> orderedRequest;
  for (const auto& currElt : _semExpLinks)
  {
    LinkPosition position = LinkPosition::FIRST;
    if (!currElt.second.semExps.empty())
    {
      const auto& currSemExp = currElt.second.semExps.front();
      std::list<const GroundedExpression*> grdExpPtr;
      currSemExp->getGrdExpPtrs_SkipWrapperLists(grdExpPtr);

      for (const auto& currGrdExpPtr : grdExpPtr)
      {
        if (currGrdExpPtr->grounding().type == SemanticGroundingType::RELATIVETIME)
        {
          position = LinkPosition::LAST;
          break;
        }
        if (SemExpGetter::isAnything(*currGrdExpPtr))
        {
          position = LinkPosition::BEFORELAST;
        }

        auto* genGrdPtr = currGrdExpPtr->grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr && genGrdPtr->word.isEmpty())
          position = LinkPosition::THIRD;
      }

      if (position == LinkPosition::FIRST)
      {
        if (currElt.first == SemanticRequestType::ACTION)
          position = LinkPosition::SECOND;
      }
      orderedRequest[position].insert(currElt.first);
    }
    else
    {
      orderedRequest[LinkPosition::SECOND].insert(currElt.first);
    }
  }

  for (const auto& currPos : orderedRequest)
  {
    for (const auto& currReq : currPos.second)
    {
      auto it = _semExpLinks.find(currReq);
      if (it != _semExpLinks.end())
        _semExpLinksSorted.emplace_back(it->first, &it->second);
    }
  }
}

void RequestLinks::clear()
{
  _semExpLinks.clear();
  _semExpLinksSorted.clear();
}

SubRequestLinks& RequestLinks::_addChildWithoutSortedContainer(SemanticRequestType pRequestType)
{
  return _semExpLinks[pRequestType];
}


void getLinksOfAGrdExp(RequestLinks& pReqLinks,
                       SemControllerWorkingStruct& pWorkStruct,
                       SemanticMemoryBlockViewer& pMemViewer,
                       const GroundedExpression& pGrdExp,
                       bool pAddSubordinateLinks,
                       bool pIsAnAction)
{
  const SemanticStatementGrounding* startPtr = pGrdExp->getStatementGroundingPtr();
  if (startPtr != nullptr)
  {
    if (startPtr->word.language != SemanticLanguageEnum::UNKNOWN)
      pReqLinks.language = startPtr->word.language;
    pReqLinks.tense = startPtr->verbTense;
    pReqLinks.verbGoal = startPtr->verbGoal;
    pReqLinks.isEquVerb = ConceptSet::haveAConceptThatBeginWith(startPtr->concepts, "verb_equal_");
    pReqLinks.gramTypeOfTheAnswer = semanticRequestType_toSemGram(startPtr->requests.firstOrNothing());
    pReqLinks._addChildWithoutSortedContainer(SemanticRequestType::ACTION).semExps.push_back(&pGrdExp);

    // iterate over all the sub semantic exressions
    for (const auto& currChild : pGrdExp.children)
    {
      GrammaticalType grammType = currChild.first;
      SemanticRequestType reqType = SemExpGetter::convertSemGramToRequestType(grammType);
      if (reqType != SemanticRequestType::NOTHING)
      {
        // never consider sentence with parameters
        const GroundedExpression* childGrdExpPtr = currChild.second->getGrdExpPtr();
        if (childGrdExpPtr != nullptr)
        {
          const auto& childGrdExp = *childGrdExpPtr;
          auto* childGenGrdPtr = childGrdExp->getGenericGroundingPtr();
          if (childGenGrdPtr != nullptr)
          {
            if (childGenGrdPtr->coreference &&
                childGenGrdPtr->coreference->getDirection() == CoreferenceDirectionEnum::PARENT)
              continue;
          }
          else if (SemExpGetter::grdExpIsAnEmptyStatementGrd(childGrdExp))
          {
            continue;
          }
          else
          {
            if (pIsAnAction && reqType == SemanticRequestType::TIME)
            {
              auto* childTimeGrdPtr = childGrdExp->getTimeGroundingPtr();
              if (childTimeGrdPtr != nullptr)
              {
                SemanticTimeGrounding refTimeGrd;
                refTimeGrd.equalToNow();
                if (childTimeGrdPtr->date.isTheSameDay(refTimeGrd.date))
                  continue;
              }
            }
          }

          auto& subLinks = pReqLinks._addChildWithoutSortedContainer(reqType);
          if (pAddSubordinateLinks)
          {
            auto itSpecifier = childGrdExp.children.find(GrammaticalType::SPECIFIER);
            if (itSpecifier != childGrdExp.children.end())
            {
              std::list<const GroundedExpression*> specifierGrdExpPtrs;
              itSpecifier->second->getGrdExpPtrs_SkipWrapperLists(specifierGrdExpPtrs);
              for (const auto& currSpecGrdExpPtr : specifierGrdExpPtrs)
              {
                auto* specStatGrdPtr = currSpecGrdExpPtr->grounding().getStatementGroundingPtr();
                if (specStatGrdPtr != nullptr)
                {
                  GrammaticalType subGrammType = SemExpGetter::childGrammaticalTypeOfParentCoreference(*currSpecGrdExpPtr);
                  if (subGrammType != GrammaticalType::UNKNOWN)
                  {
                    subLinks.crossedLinks.subReqListsToFilter.emplace_back();
                    auto& subReqList = subLinks.crossedLinks.subReqListsToFilter.back();
                    getLinksOfAGrdExp(subReqList, pWorkStruct, pMemViewer, *currSpecGrdExpPtr, pAddSubordinateLinks);
                    subReqList.gramTypeOfTheAnswer = subGrammType;
                    subLinks.crossedLinks.semExpsWithSpecificFilter.insert(currSpecGrdExpPtr);
                  }
                }
              }
            }
          }

          switch (childGrdExp->type)
          {
          case SemanticGroundingType::META:
          {
            pReqLinks.clear();
            return;
          }
          case SemanticGroundingType::GENERIC:
          {
            auto& childGenGrd = childGrdExp->getGenericGrounding();
            if (pAddSubordinateLinks &&
                !pReqLinks.isEquVerb && childGenGrd.referenceType != SemanticReferenceType::DEFINITE &&
                !ConceptSet::haveAConcept(childGenGrd.concepts, "agent") && // Big optimization for who questions and it's not needed because the fact that something is an agent cannot be found dynamically
                (!childGenGrd.concepts.empty() || !childGenGrd.word.isEmpty()))
            {
              subLinks.crossedLinks.subReqListsToAdd.emplace_back();
              auto& subReqList = subLinks.crossedLinks.subReqListsToAdd.back();
              subReqList.tense = SemanticVerbTense::PRESENT;
              subReqList.isEquVerb = true;
              subReqList.gramTypeOfTheAnswer = GrammaticalType::SUBJECT;
              subReqList._addChildWithoutSortedContainer(SemanticRequestType::ACTION).concepts.emplace("verb_equal_be", 4);
              auto& subObjLinks = subReqList._addChildWithoutSortedContainer(SemanticRequestType::OBJECT);
              subObjLinks.semExps.emplace_back(&*currChild.second);
              subObjLinks.crossedLinks.semExpsWithSpecificFilter = subLinks.crossedLinks.semExpsWithSpecificFilter;
              subReqList.fillSortedSemExps();
            }
            break;
          }
          case SemanticGroundingType::STATEMENT:
          {
            const auto& childStatGrd = childGrdExp->getStatementGrounding();
            // add the corresponding time slots of the time child
            if (reqType == SemanticRequestType::TIME &&
                pReqLinks.tense == SemanticVerbTense::PUNCTUALPAST &&
                childStatGrd.verbTense == SemanticVerbTense::PUNCTUALPAST)
            {
              SemControllerWorkingStruct subWorkStruct(pWorkStruct);
              if (subWorkStruct.askForNewRecursion())
              {
                subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
                std::list<const GroundedExpression*> otherGrdExps;
                SemanticRequests request;
                request.set(reqType);
                satisfyAQuestion(subWorkStruct, pMemViewer, childGrdExp, otherGrdExps,
                                 childGrdExp, request);
                std::list<std::unique_ptr<SemAnswer>> semAnswers;
                controller::convertToDetalledAnswer(semAnswers, subWorkStruct);
                std::list<const GroundedExpression*> grdExpsAnswers;
                CompositeSemAnswer::getGrdExps(grdExpsAnswers, semAnswers);

                SubRequestLinks& linksToFill =
                    pReqLinks._addChildWithoutSortedContainer(reqType);
                for (const auto& currGrdExpAnswer : grdExpsAnswers)
                  if ((*currGrdExpAnswer)->getTimeGroundingPtr() != nullptr)
                    linksToFill.semExps.emplace_back(currGrdExpAnswer);
              }
            }
            break;
          }
          default:
            break;
          };
        }
        auto& subLinks = pReqLinks._addChildWithoutSortedContainer(reqType);
        subLinks.semExps.push_back(&*currChild.second);
      }
      else if (grammType == GrammaticalType::OCCURRENCE_RANK)
      {
        pReqLinks.occurenceRankFilter = &*currChild.second;
      }
    }
    pReqLinks.fillSortedSemExps();
    return;
  }
  auto& subLinks = pReqLinks._addChildWithoutSortedContainer(SemanticRequestType::NOTHING);
  subLinks.semExps.push_back(&pGrdExp);
  pReqLinks.fillSortedSemExps();
}


void getInformationsLinkedToCondition(std::set<const GroundedExpWithLinks*>& pNewInformations,
                                      SemControllerWorkingStruct& pWorkStruct,
                                      SemanticMemoryBlockViewer& pMemViewer,
                                      const RequestLinks& pReqLinks)
{
  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RelationsThatMatch<false> idsToSentences;
  RequestToMemoryLinks<false> reqToLinks(memBlockPrivateViewer.getLinks(SemanticTypeOfLinks::CONDITION_INFORMATION, pReqLinks.tense, pReqLinks.verbGoal));
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory(idsToSentences, reqToLinks, pWorkStruct, memBlockPrivateAccessor, pMemViewer,
                       pReqLinks, false, semanticMemoryGetter::RequestContext::SENTENCE);
  for (const auto& currRes : idsToSentences.res.dynamicLinks)
  {
    auto& currMemSent = *currRes.second;
    for (auto& otherMemSent : currMemSent.getContextAxiom().memorySentences.elts)
    {
      if (!otherMemSent.isAConditionToSatisfy())
      {
        assert(&currMemSent != &otherMemSent);
        pNewInformations.insert(&otherMemSent);
      }
    }
  }
}


void getNowConditions(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const SemanticDuration& pNowTimeDuration,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  auto* viewPrivatePtr = pMemViewer.getViewPrivatePtr();
  if (viewPrivatePtr == nullptr)
    return;

  RequestToMemoryLinks<true> reqToGrdExps(viewPrivatePtr->getLinks(SemanticTypeOfLinks::SENT_WITH_ACTION,
                                                                   SemanticVerbTense::UNKNOWN, VerbGoalEnum::NOTIFICATION));
  RelationsThatMatch<true> idsToSentences;
  static const SentenceLinks<true> emptyLinks;
  auto& memBlockPrivate = pMemViewer.getConstViewPrivate();
  semanticMemoryGetter::getResultMatchingNowTimeFromMemory(idsToSentences, emptyLinks, reqToGrdExps, pNowTimeDuration,
                                                           memBlockPrivate, false, pLingDb, false);
  _doActions(pWorkStruct, pMemViewer, nullptr, idsToSentences.res);
  // Unlink the time condition
  for (auto& currSent : idsToSentences.res.dynamicLinks)
    currSent.second->setEnabled(false);
}


const SemanticExpression* getActionComposition(SemControllerWorkingStruct& pWorkStruct,
                                               SemanticMemoryBlockViewer& pMemViewer,
                                               const GroundedExpression& pGrdExp)
{
  semanticMemoryLinker::RequestLinks reqLinks;
  getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);

  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RequestToMemoryLinks<false> reqToGrdExps(memBlockPrivateViewer.sentWithInfActionLinks.reqToGrdExps, nullptr);
  RelationsThatMatch<false> idToSentences;
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory
      (idToSentences, reqToGrdExps, pWorkStruct, memBlockPrivateAccessor, pMemViewer, reqLinks,
       false, semanticMemoryGetter::RequestContext::COMMAND, nullptr, SemanticRequestType::NOTHING, false);
  if (idToSentences.res.dynamicLinks.empty())
    return nullptr;
  return idToSentences.res.dynamicLinks.begin()->second->getContextAxiom().infCommandToDo;
}


bool checkForConditionsLinkedToStatement(SemControllerWorkingStruct& pWorkStruct,
                                         SemanticMemoryBlockViewer& pMemViewer,
                                         const RequestLinks& pReqLinks,
                                         const GroundedExpression& pGrdExp)
{
  auto& memBlockPrivateViewer = pMemViewer.getConstViewPrivate();
  RequestToMemoryLinks<false> reqToGrdExps(memBlockPrivateViewer.getLinks(SemanticTypeOfLinks::SENT_WITH_ACTION, pReqLinks.tense, pReqLinks.verbGoal));
  RelationsThatMatch<false> idToSentences;
  MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&memBlockPrivateViewer);
  _getResultFromMemory
      (idToSentences, reqToGrdExps, pWorkStruct, memBlockPrivateAccessor, pMemViewer,
       pReqLinks, false, semanticMemoryGetter::RequestContext::SENTENCE_TO_CONDITION);
  return _doActions(pWorkStruct, pMemViewer, &pGrdExp, idToSentences.res);
}


void disableActionsLinkedToASentence(SemanticMemory& pSemanticMemory,
                                     const GroundedExpression& pGrdExp,
                                     const linguistics::LinguisticDatabase& pLingDb)
{
  SemControllerWorkingStruct workStruct
      (InformationType::INFORMATION, nullptr,
       SemanticLanguageEnum::UNKNOWN, nullptr,
       SemanticOperatorEnum::REACT, &pSemanticMemory.proativeSpecifications,
       pSemanticMemory.getExternalFallback(), &pSemanticMemory.callbackToSentencesCanBeAnswered,
       nullptr, pLingDb);
  SemanticMemoryBlockViewer memViewer(&pSemanticMemory.memBloc, pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId());
  RequestLinks reqLinks;
  getLinksOfAGrdExp(reqLinks, workStruct, memViewer, pGrdExp);

  auto* memViewPrivatePtr = memViewer.getViewPrivatePtr();
  if (memViewPrivatePtr != nullptr)
  {
    RelationsThatMatch<true> idsToSentences;
    RequestToMemoryLinks<true> reqToLinks(memViewPrivatePtr->getLinks(SemanticTypeOfLinks::SENT_WITH_ACTION, reqLinks.tense, reqLinks.verbGoal));
    MemoryBlockPrivateAccessorPtr<true> memBlockPrivateAccessor(memViewPrivatePtr);
    _getResultFromMemory(idsToSentences, reqToLinks, workStruct, memBlockPrivateAccessor, memViewer, reqLinks,
                         false, semanticMemoryGetter::RequestContext::SENTENCE_TO_CONDITION);
    for (const auto& sent : idsToSentences.res.dynamicLinks)
     sent.second->setEnabled(false);
    return;
  }
  assert(false);
}


bool addTriggerSentencesAnswer
(SemControllerWorkingStruct& pWorkStruct,
 bool& pAnAnswerHasBeenAdded,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 SemanticExpressionCategory pExpCategory,
 const SemanticTriggerAxiomId& pAxiomId,
 const GroundedExpression& pInputGrdExp,
 ContextualAnnotation pContAnnotation)
{
  std::set<const ExpressionWithLinks*> semExpWrapperPtrs;
  _matchGrdExpTrigger(semExpWrapperPtrs, pWorkStruct, pMemViewer,
                      pReqLinks, pExpCategory, pAxiomId, pInputGrdExp);
  return _addTriggerThatMatchTheMost(pWorkStruct, pAnAnswerHasBeenAdded, semExpWrapperPtrs,
                                     pMemViewer, pContAnnotation, pInputGrdExp);
}


bool satisfyAnAction(SemControllerWorkingStruct& pWorkStruct,
                     SemanticMemoryBlockViewer& pMemViewer,
                     const GroundedExpression& pGrdExp,
                     const SemanticStatementGrounding& pGrdExpStatement)
{
  // get links of the current sentence
  RequestLinks reqLinks;
  getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false, true);

  bool anAnswerHasBeenAdded = false;
  if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT &&
      addTriggerSentencesAnswer(pWorkStruct, anAnswerHasBeenAdded, pMemViewer, reqLinks,
                                SemanticExpressionCategory::COMMAND, _emptyAxiomId, pGrdExp,
                                ContextualAnnotation::BEHAVIOR))
    return true;

  if (!pGrdExpStatement.polarity)
  {
    pWorkStruct.addAnswerWithoutReferences
        (ContextualAnnotation::BEHAVIORNOTFOUND,
         SemExpCreator::confirmInformation(pGrdExp));
    return true;
  }

  if (specificActionsHandler::process(pWorkStruct, pMemViewer, pGrdExp, pGrdExpStatement))
    return true;

  // get a semantic expression that is linked to an action
  // (Action learned from sentences: "to write is to blabla")
  reqLinks.eraseChild(SemanticRequestType::SUBJECT);
  return _getActionFromAMemblock(pWorkStruct, pMemViewer, reqLinks, pGrdExp) ||
      anAnswerHasBeenAdded;
}


const SemanticExpression* getActionActionDefinition(SemControllerWorkingStruct& pWorkStruct,
                                                    SemanticMemoryBlockViewer& pMemViewer,
                                                    const GroundedExpression& pGrdExp)
{
  RequestLinks reqLinks;
  getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp, false);
  return _getActionDefFromAMemblock(pWorkStruct, pMemViewer, reqLinks, pGrdExp);
}


template<typename TUSEMEXP>
void _addGrdExpsFromASemExp(std::map<semIdAbs, std::list<AnswerExp>>& pSemExpsContainer,
                            const semIdAbs& pId,
                            TUSEMEXP& pSemExpToAdd,
                            const RelatedContextAxiom& pRelatedContextAxioms,
                            const SemanticExpression* pEqualityOfTheMemoryAnswer,
                            std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer,
                            const GroundedExpression* pQuestMetaGrdExp,
                            const SemControllerWorkingStruct& pWorkStruct,
                            const SemanticMemoryBlock& pMemBlock,
                            SemanticRequestType pRequest)
{
  switch (pSemExpToAdd->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    auto& grdExp = pSemExpToAdd->getGrdExp();

    // don't answer with an english text/url if the question is in wrong language
    switch (grdExp->type)
    {
    case SemanticGroundingType::TEXT:
    {
      auto questFromLanguage = pWorkStruct.fromLanguage;
      const SemanticTextGrounding& textGrdToAdd = grdExp->getTextGrounding();
      if (textGrdToAdd.forLanguage != SemanticLanguageEnum::UNKNOWN &&
          questFromLanguage != SemanticLanguageEnum::UNKNOWN &&
          textGrdToAdd.forLanguage != questFromLanguage)
      {
        return;
      }
      break;
    }
    case SemanticGroundingType::RESOURCE:
    {
      auto questFromLanguage = pWorkStruct.fromLanguage;
      const SemanticResourceGrounding& resourceGrdToAdd = grdExp->getResourceGrounding();
      if (resourceGrdToAdd.resource.language != SemanticLanguageEnum::UNKNOWN &&
          questFromLanguage != SemanticLanguageEnum::UNKNOWN &&
          resourceGrdToAdd.resource.language != questFromLanguage)
      {
        return;
      }
      break;
    }
    default:
      break;
    }

    if (_isAValidAnswerForTheQuestionFilter(grdExp, pQuestMetaGrdExp, pRequest, pWorkStruct.lingDb.conceptSet, pWorkStruct.lingDb) ||
        (grdExp.children.count(GrammaticalType::SUB_CONCEPT) > 0 &&
         pWorkStruct.lingDb.conceptSet.areConceptsCompatibles(grdExp->concepts, pQuestMetaGrdExp->grounding().concepts) == true))
    {
      auto grdExpContainer = makeGrdExpContainer(pSemExpToAdd);
      pSemExpsContainer[pId].emplace_back(pRelatedContextAxioms, std::move(grdExpContainer), pEqualityOfTheMemoryAnswer,
                                          pAnnotationsOfTheAnswer);
    }
    else
    {
      const auto itEquChild = grdExp.children.find(GrammaticalType::SPECIFIER);
      if (itEquChild != grdExp.children.end())
      {
        _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(itEquChild->second),
                                         pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                         pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
      }
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pSemExpToAdd->getListExp();
    for (auto& elt : listExp.elts)
      _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(elt),
                                       pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                       pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pSemExpToAdd->getIntExp();
    _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(intExp.interpretedExp),
                                     pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                     pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkExp = pSemExpToAdd->getFdkExp();
    _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(fdkExp.concernedExp),
                                     pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                     pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pSemExpToAdd->getAnnExp();
    _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(annExp.semExp),
                                     pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                     pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pSemExpToAdd->getMetadataExp();
    _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(metadataExp.semExp),
                                     pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                     pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    std::list<simplifier::ConditionSolvedResult<TUSEMEXP>> solvedConditons;
    simplifier::solveConditions<TUSEMEXP>(solvedConditons, pSemExpToAdd, pMemBlock,
                                                          pWorkStruct.lingDb);
    for (const auto& currSolvedCond : solvedConditons)
    {
      auto& condExp = currSolvedCond.rootSemExp->getCondExp();
      if (currSolvedCond.truenessValue == TruenessValue::VAL_TRUE)
        _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(condExp.thenExp),
                                         pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                         pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
      else if (currSolvedCond.truenessValue == TruenessValue::VAL_FALSE &&
               condExp.elseExp)
        _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(*condExp.elseExp),
                                         pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                         pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    }
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    auto& fSynthExp = pSemExpToAdd->getFSynthExp();
    _addGrdExpsFromASemExp<TUSEMEXP>(pSemExpsContainer, pId, pSemExpToAdd.wrapInContainer(fSynthExp.getUSemExp()),
                                     pRelatedContextAxioms, pEqualityOfTheMemoryAnswer, pAnnotationsOfTheAnswer,
                                     pQuestMetaGrdExp, pWorkStruct, pMemBlock, pRequest);
    break;
  }
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}


void _extractAnnotationsFromAnswer(const SemanticExpression*& pEqualityOfTheMemoryAnswer,
                                   std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer,
                                   SemanticRequestType pFormRequest,
                                   const GroundedExpression& pGrdExp,
                                   const SemanticAnnotations& pAnnotations,
                                   const std::vector<GrammaticalType>& pGramTypes)
{
  for (const auto& currAnsGrdExpChild : pGrdExp.children)
  {
    GrammaticalType childAnswerGramType = currAnsGrdExpChild.first;
    if (childAnswerGramType == GrammaticalType::SUBJECT)
      continue;

    bool shouldContinue = false;
    for (auto& currGram : pGramTypes)
    {
      if (currGram == childAnswerGramType)
      {
        shouldContinue = true;
        break;
      }
    }
    if (shouldContinue)
      continue;

    if (childAnswerGramType == GrammaticalType::SPECIFIER)
    {
      pEqualityOfTheMemoryAnswer = [pFormRequest, &currAnsGrdExpChild]() -> const SemanticExpression*
      {
        if (pFormRequest != SemanticRequestType::OBJECT &&
            pFormRequest != SemanticRequestType::MANNER)
          return nullptr;
        return &*currAnsGrdExpChild.second;
      }();
      if (pEqualityOfTheMemoryAnswer != nullptr)
        continue;
    }
    pAnnotationsOfTheAnswer.emplace(childAnswerGramType, &*currAnsGrdExpChild.second);
  }
  pAnnotations.iterateOverAllValues([&](GrammaticalType pGrammaticalType,
                                    const SemanticExpression& pSemExp)
  {
    pAnnotationsOfTheAnswer.emplace(pGrammaticalType, &pSemExp);
  });
}


bool _keepGroundingsWithSamePolarity(std::list<AnswerExp>& pAllAnswers,
                                     bool pRemoveIfInformationAlreadyExist,
                                     std::map<semIdAbs, std::list<AnswerExp>>& pSemExpsWithSamePolarity,
                                     std::map<semIdAbs, std::list<AnswerExp>>& pSemExpsWithOtherPolarity,
                                     SemanticRequestType pRequest,
                                     const SemanticMemoryBlock& pMemBlock,
                                     const linguistics::LinguisticDatabase& pLingDb)
{
  // if a grounding has both same plarity and a different polarity,
  // we keep the more recent

  for (auto& samPolList : pSemExpsWithSamePolarity)
  {
    std::set<const AnswerExp*> answersToCancel;

    // if a contrary info is more recent we cancel the information
    for (const auto& othPolList : pSemExpsWithOtherPolarity)
      if (samPolList.first < othPolList.first) // more recent check
        for (const auto& othPol : othPolList.second)
          for (const auto& samPol : samPolList.second)
            if (answerExpAreEqual(othPol, samPol, pMemBlock, pLingDb))
              answersToCancel.insert(&samPol);

    for (auto samPolIt = samPolList.second.begin();
         samPolIt != samPolList.second.end(); )
    {
      auto nextIt = samPolIt;
      ++nextIt;
      if (answersToCancel.count(&*samPolIt) == 0)
      {
        bool alreadyExistInAllAnswers = false;
        if (pRemoveIfInformationAlreadyExist)
        {
          for (const auto& answ : pAllAnswers)
          {
            if (answerExpAreEqual(*samPolIt, answ, pMemBlock, pLingDb))
            {
              alreadyExistInAllAnswers = true;
              break;
            }
          }
        }
        if (!alreadyExistInAllAnswers)
        {
          pAllAnswers.splice(pAllAnswers.end(), samPolList.second, samPolIt);
        }
      }
      samPolIt = nextIt;
    }
  }

  if (pAllAnswers.empty())
  {
    for (auto& othPolList : pSemExpsWithOtherPolarity)
    {
      for (auto othPolIt = othPolList.second.begin();
           othPolIt != othPolList.second.end(); ++othPolIt)
      {
        if (SemExpComparator::grdHaveNbSetToZero(*othPolIt->getGrdExp()))
        {
          pAllAnswers.splice(pAllAnswers.end(), othPolList.second, othPolIt);
          return true;
        }
      }
    }
  }
  return !pAllAnswers.empty() ||
      (!pSemExpsWithOtherPolarity.empty() && pRequest == SemanticRequestType::YESORNO);
}


void _genericFilterSemExpThatCanAnswer(std::map<SemanticRequestType, AllAnswerElts>& pAllAnswers,
                                       std::map<semIdAbs, std::unique_ptr<AnswerElement>>& pRelations,
                                       const GroundedExpression& pFromGrdExpQuestion,
                                       SemanticRequestType pFormRequest,
                                       SemanticRequestType pDefaultRequType,
                                       const SemControllerWorkingStruct& pWorkStruct,
                                       const SemanticMemoryBlock& pMemBlock)
{
  std::map<semIdAbs, std::list<AnswerExp>> semExpsWithSamePolarity;
  std::map<semIdAbs, std::list<AnswerExp>> semExpsWithOtherPolarity;
  // iterate over all related semantic expressions
  for (auto& relSemExp : pRelations)
  {
    AnswerElement& ansElt = *relSemExp.second;
    auto grammaticalTypes = SemExpGetter::requestToGrammaticalTypes(pFormRequest);

    const SemanticExpression* equalityOfTheMemoryAnswer = nullptr;
    std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;
    const auto& answGrdExp = ansElt.getGrdExpRef();
    _extractAnnotationsFromAnswer(equalityOfTheMemoryAnswer, annotationsOfTheAnswer, pFormRequest,
                                  answGrdExp, ansElt.getAnnotations(), grammaticalTypes);

    if (grammaticalTypes.empty())
    {
      bool hasSamePolarity = SemExpComparator::haveSamePolarity
          (answGrdExp, pFromGrdExpQuestion, pWorkStruct.lingDb.conceptSet);
      auto answGrdExpContainer = ansElt.getGrdExpContainer();
      auto& semExpsContainer = hasSamePolarity ? semExpsWithSamePolarity : semExpsWithOtherPolarity;
      semExpsContainer[relSemExp.first].emplace_back(ansElt.relatedContextAxioms,
                                                     std::move(answGrdExpContainer), nullptr,
                                                     annotationsOfTheAnswer);
    }
    else
    {
      for (const auto& currGramType : grammaticalTypes)
      {
        bool hasSamePolarity = false;
        auto childSemExpPtr = ansElt.getSemExpForGrammaticalType
            (currGramType, &pFromGrdExpQuestion, &pWorkStruct.lingDb, &hasSamePolarity);
        if (childSemExpPtr)
        {
          const GroundedExpression* questMetaGrdExp = nullptr;
          const auto itQuestionMetaDesc = pFromGrdExpQuestion.children.find(currGramType);
          if (itQuestionMetaDesc != pFromGrdExpQuestion.children.end())
            questMetaGrdExp = itQuestionMetaDesc->second->getGrdExpPtr_SkipWrapperPtrs();

          auto& semExpsContainer = hasSamePolarity ? semExpsWithSamePolarity : semExpsWithOtherPolarity;
          auto* uSemExp = dynamic_cast<UniqueSemanticExpression*>(&*childSemExpPtr);
          if (uSemExp != nullptr)
          {
            _addGrdExpsFromASemExp(semExpsContainer, relSemExp.first, *uSemExp, ansElt.relatedContextAxioms,
                                   equalityOfTheMemoryAnswer, annotationsOfTheAnswer, questMetaGrdExp,
                                   pWorkStruct, pMemBlock, pFormRequest);
          }
          else
          {
            const auto* semExpRef = dynamic_cast<ReferenceOfSemanticExpressionContainer*>(&*childSemExpPtr);
            if (semExpRef != nullptr)
              _addGrdExpsFromASemExp(semExpsContainer, relSemExp.first, *semExpRef, ansElt.relatedContextAxioms,
                                     equalityOfTheMemoryAnswer, annotationsOfTheAnswer, questMetaGrdExp,
                                     pWorkStruct, pMemBlock, pFormRequest);
          }
        }
      }
    }
  }

  std::list<AnswerExp>& answersForDefaultRequ = pAllAnswers[pDefaultRequType].answersFromMemory;
  bool removeIfInformationAlreadyExist = pDefaultRequType != SemanticRequestType::TIMES;
  if (!_keepGroundingsWithSamePolarity(answersForDefaultRequ,
                                       removeIfInformationAlreadyExist,
                                       semExpsWithSamePolarity, semExpsWithOtherPolarity,
                                       pDefaultRequType, pMemBlock, pWorkStruct.lingDb))
  {
    pAllAnswers.erase(pDefaultRequType);
  }
}


void _addCauseResult(std::map<SemanticRequestType, AllAnswerElts>& pAllAnswers,
                     AnswerElement& pAnswerElement)
{
  for (const GroundedExpWithLinks& currSent : pAnswerElement.getMemorySentences())
  {
    if (currSent.isAConditionToSatisfy())
    {
      auto grdExpRef = std::make_unique<GroundedExpressionRef>(currSent.grdExp);
      std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;
      pAllAnswers[SemanticRequestType::CAUSE].answersFromMemory.emplace_back
          (currSent.getContextAxiom(), std::move(grdExpRef), nullptr,
           annotationsOfTheAnswer);
    }
  }
}


void _replaceAnswersByNumberOfInstances(std::map<SemanticRequestType, AllAnswerElts>& pAllAnswers,
                                        const SemanticUnityGrounding* pUnityGrdPtr)
{
  std::unique_ptr<SemanticGrounding> answerGrdPtr;
  auto _addElts = [&](const SemanticExpression& pSemExp)
  {
    auto quantityGrd = SemExpGetter::extractQuantity(pSemExp, pUnityGrdPtr);
    if (quantityGrd)
    {
      if (answerGrdPtr)
        answerGrdPtr = SemExpGetter::mergeQuantities(*answerGrdPtr, std::move(quantityGrd));
      else
        answerGrdPtr = std::move(quantityGrd);
    }
  };

  for (const auto& currAnswers : pAllAnswers)
  {
    for (const auto& currAnswerFromMemory : currAnswers.second.answersFromMemory)
      _addElts(currAnswerFromMemory.getGrdExp());
    for (const auto& currAnswerGenerated : currAnswers.second.answersGenerated)
      _addElts(*currAnswerGenerated.genSemExp);
  }

  if (answerGrdPtr)
  {
    AllAnswerElts quantityAnswer;
    quantityAnswer.answersGenerated.emplace_back(std::make_unique<GroundedExpression>(std::move(answerGrdPtr)));
    pAllAnswers.clear();
    pAllAnswers.emplace(SemanticRequestType::QUANTITY, std::move(quantityAnswer));
  }
  else
  {
    pAllAnswers.clear();
  }
}


bool matchAffirmationTrigger
(SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 const GroundedExpression& pInputGrdExp)
{
  bool anAnswerHasBeenAdded = false;
  bool res = addTriggerSentencesAnswer(pWorkStruct, anAnswerHasBeenAdded, pMemViewer, pReqLinks,
                                       SemanticExpressionCategory::AFFIRMATION, _emptyAxiomId, pInputGrdExp,
                                       ContextualAnnotation::ANSWER);

  if (!res && pWorkStruct.callbackToSentencesCanBeAnsweredPtr != nullptr)
  {
    for (const auto& currCallbackToSents : *pWorkStruct.callbackToSentencesCanBeAnsweredPtr)
    {
      const auto& memBlock = currCallbackToSents->memBlockForTriggers;
      SemanticMemoryBlockViewer subMemViewer(nullptr, memBlock, SemanticAgentGrounding::userNotIdentified);
      auto& subMemBlockViewer = subMemViewer.getConstViewPrivate();
      RequestToMemoryLinks<false> awLinks(subMemBlockViewer.getLinks(SemanticTypeOfLinks::ANSWER, pReqLinks.tense, pReqLinks.verbGoal));
      RelationsThatMatch<false> idsToSentences;
      MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&subMemBlockViewer);
      SemControllerWorkingStruct subWorkStruct(pWorkStruct.informationType, pWorkStruct.authorSemExp,
                                               pWorkStruct.fromLanguage, pWorkStruct.expHandleInMemory,
                                               pWorkStruct.reactOperator, nullptr, nullptr, nullptr, nullptr,
                                               pWorkStruct.lingDb);
      _getResultFromMemory(idsToSentences, awLinks, subWorkStruct, memBlockPrivateAccessor, subMemViewer,
                           pReqLinks, true, semanticMemoryGetter::RequestContext::SENTENCE);

      for (const auto& currRel : idsToSentences.res.dynamicLinks)
      {
        auto params = std::make_unique<IndexToSubNameToParameterValue>();
        std::unique_ptr<InteractionContextContainer> subIntContext;
        unknownInfosGetter::checkIfMatchAndGetParams(*params, subIntContext, nullptr, *currRel.second,
                                                     pInputGrdExp, pWorkStruct, subMemViewer);

        SemanticLanguageEnum searchLangType = pWorkStruct.fromLanguage;
        if (searchLangType == SemanticLanguageEnum::UNKNOWN)
          searchLangType = SemanticLanguageEnum::ENGLISH;
        try
        {
          auto answer = currCallbackToSents->getAnswer(*params, searchLangType);
          if (answer)
          {
            pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWER, std::move(*answer));
            res = true;
          }
        }
        catch (const std::exception& e)
        {
          std::cerr << "Exception caught when calling an external trigger method. The error was: " << e.what() << std::endl;
        }
      }
    }
  }

  return res;
}


bool addTriggerListExp
(SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const ListExpression& pListExp)
{
  std::list<const GroundedExpression*> grdExpPtrs;
  pListExp.getGrdExpPtrs_SkipWrapperLists(grdExpPtrs, false);

  std::size_t nbListOfElts = grdExpPtrs.size();
  auto getAxiomIdFromId = [&nbListOfElts, &pListExp](std::size_t pId)
  {
    return SemanticTriggerAxiomId(nbListOfElts, pId, pListExp.listType);
  };
  bool anAnswerHasBeenAdded = false;
  _addTriggerGrdExps(pWorkStruct, anAnswerHasBeenAdded, pMemViewer, grdExpPtrs, pListExp, getAxiomIdFromId);
  return anAnswerHasBeenAdded;
}


bool addTriggerCondExp
(SemControllerWorkingStruct& pWorkStruct,
 SemanticMemoryBlockViewer& pMemViewer,
 const ConditionExpression& pCondExp)
{
  std::list<const GroundedExpression*> grdExpPtrs;
  pCondExp.toListOfGrdExpPtrs(grdExpPtrs, false);
  std::size_t nbListOfElts = grdExpPtrs.size();
  auto getAxiomIdFromId = [&nbListOfElts](std::size_t pId)
  {
    return SemanticTriggerAxiomId(nbListOfElts, pId, SemanticExpressionType::CONDITION);
  };
  bool anAnswerHasBeenAdded = false;
  _addTriggerGrdExps(pWorkStruct, anAnswerHasBeenAdded, pMemViewer, grdExpPtrs, pCondExp, getAxiomIdFromId);
  return anAnswerHasBeenAdded;
}

void _fillStaticLinksIfNecessary(std::map<intMemBlockId, MemorySentencesInResult>& pMemSentencesInResult,
                                 std::map<intSemId, std::unique_ptr<AnswerElementStatic>>& pStaticLinks,
                                 const std::shared_ptr<SemanticMemoryBlockBinaryReader>& pSubBinaryBlockPtr)
{
  if (!pStaticLinks.empty() && pSubBinaryBlockPtr)
    pMemSentencesInResult[pSubBinaryBlockPtr->getId()].idToStaticSentences = std::move(pStaticLinks);
}


void _getSentencesFromMemBlockRecursively
(std::map<intMemBlockId, MemorySentencesInResult>& pMemSentencesInResult,
 SemanticMemoryBlockViewer& pMemViewer,
 const RequestLinks& pReqLinks,
 const linguistics::LinguisticDatabase& pLingDb,
 bool pCheckTimeRequest)
{
  bool considerCoreferences = false;
  auto& memBlockViewer = pMemViewer.getConstViewPrivate();
  auto* memViewPrivatePtr = pMemViewer.getViewPrivatePtr();
  if (memViewPrivatePtr != nullptr)
  {
    RelationsThatMatch<true> idToSentences;
    auto awLinks = memViewPrivatePtr->getLinks(SemanticTypeOfLinks::ANSWER, pReqLinks.tense, pReqLinks.verbGoal);
    MemoryBlockPrivateAccessorPtr<true> memViewPrivateAccessor(memViewPrivatePtr);
    semanticMemoryGetter::getResultFromMemory(idToSentences, awLinks, pReqLinks,
                                              semanticMemoryGetter::RequestContext::SENTENCE,
                                              memViewPrivateAccessor, false, pLingDb,
                                              pCheckTimeRequest, considerCoreferences);
    if (!idToSentences.res.dynamicLinks.empty())
    {
      intMemBlockId memBlockId = memBlockViewer.getId();
      pMemSentencesInResult[memBlockId].idsToDynamicSentences = std::move(idToSentences.res.dynamicLinks);
    }
    _fillStaticLinksIfNecessary(pMemSentencesInResult, idToSentences.res.staticLinks, memBlockViewer.subBinaryBlockPtr);
  }
  else
  {
    RelationsThatMatch<false> idToSentences;
    auto awLinks = memBlockViewer.getLinks(SemanticTypeOfLinks::ANSWER, pReqLinks.tense, pReqLinks.verbGoal);
    MemoryBlockPrivateAccessorPtr<false> memViewPrivateAccessor(&memBlockViewer);
    semanticMemoryGetter::getResultFromMemory(idToSentences, awLinks, pReqLinks,
                                              semanticMemoryGetter::RequestContext::SENTENCE,
                                              memViewPrivateAccessor, false, pLingDb,
                                              pCheckTimeRequest, considerCoreferences);
    if (!idToSentences.res.dynamicLinks.empty())
    {
      intMemBlockId memBlockId = memBlockViewer.getId();
      pMemSentencesInResult[memBlockId].idToConstDynamicSentences = std::move(idToSentences.res.dynamicLinks);
    }
    _fillStaticLinksIfNecessary(pMemSentencesInResult, idToSentences.res.staticLinks, memBlockViewer.subBinaryBlockPtr);
  }

  if (pMemViewer.constView.subBlockPtr != nullptr)
  {
    SemanticMemoryBlockViewer subMemViewer(nullptr, *pMemViewer.constView.subBlockPtr, pMemViewer.currentUserId);
    _getSentencesFromMemBlockRecursively(pMemSentencesInResult, subMemViewer, pReqLinks,
                                         pLingDb, pCheckTimeRequest);
  }
}


void _getRelationsOfLinks
(std::map<SemanticRequestType, AllAnswerElts>& pUserAnswers,
 SemanticMemoryBlockViewer& pMemViewer,
 const SemControllerWorkingStruct& pWorkStruct,
 const RequestLinks& pReqLinks,
 const GroundedExpression& pGrdExp,
 SemanticRequestType pRequestType,
 SemanticRequestType pOriginalQuestRequestType)
{
  std::map<intMemBlockId, MemorySentencesInResult> memSentencesInResult;
  // get informations
  _getSentencesFromMemBlockRecursively(memSentencesInResult, pMemViewer,
                                       pReqLinks, pWorkStruct.lingDb, true);

  // get fallbacks
  if (memSentencesInResult.empty())
  {
    auto* memViewPtr = pMemViewer.getViewPtr();
    auto* fallbacksBlockPtr = memViewPtr != nullptr ? memViewPtr->getFallbacksBlockPtr() : nullptr;
    auto* constFallbacksBlockPtr = pMemViewer.constView.getFallbacksBlockPtr();
    if (constFallbacksBlockPtr != nullptr)
    {
      SemanticMemoryBlockViewer fallbackSubMemViewer(fallbacksBlockPtr, *constFallbacksBlockPtr, pMemViewer.currentUserId);
      _getSentencesFromMemBlockRecursively(memSentencesInResult, fallbackSubMemViewer,
                                           pReqLinks, pWorkStruct.lingDb, false);
    }
  }

  // Convert to new answer structure
  std::map<semIdAbs, std::unique_ptr<AnswerElement>> answElts;
  const std::size_t maxSizeOfAnswers = 50;
  for (auto itMemMemBlock = memSentencesInResult.begin(); itMemMemBlock != memSentencesInResult.end(); )
  {
    auto& memSents = itMemMemBlock->second;
    _filterGlobalConditionImpossibilities(memSents.idsToDynamicSentences, pWorkStruct, pMemViewer);
    _filterGlobalConditionImpossibilities(memSents.idToConstDynamicSentences, pWorkStruct, pMemViewer);
    _filterTimeImpossibilities(memSents.idsToDynamicSentences, pWorkStruct, pMemViewer, &pGrdExp, pRequestType);
    _filterTimeImpossibilities(memSents.idToConstDynamicSentences, pWorkStruct, pMemViewer, &pGrdExp, pRequestType);
    _filterTimeImpossibilities(memSents.idToStaticSentences, pWorkStruct, pMemViewer, &pGrdExp, pRequestType);
    if (memSents.empty())
      itMemMemBlock = memSentencesInResult.erase(itMemMemBlock);
    else
    {
      // keep only the 50 first answers
      keepOnlyTheFirstElements(memSents.idsToDynamicSentences, maxSizeOfAnswers);
      keepOnlyTheFirstElements(memSents.idToConstDynamicSentences, maxSizeOfAnswers);
      keepOnlyTheFirstElements(memSents.idToStaticSentences, maxSizeOfAnswers);
      const intMemBlockId& blockId = itMemMemBlock->first;
      for (auto& currMemSent : memSents.idsToDynamicSentences)
      {
        auto newElt = std::make_unique<AnswerElementDynamic>(currMemSent.second);
        newElt->relatedContextAxioms.elts.emplace_back(&currMemSent.second->getContextAxiom());
        answElts[semIdAbs(blockId, currMemSent.first)] = std::move(newElt);
      }
      for (auto& currMemSent : memSents.idToConstDynamicSentences)
      {
        auto newElt = std::make_unique<AnswerElementDynamic>(currMemSent.second);
        newElt->relatedContextAxioms.constElts.emplace_back(&currMemSent.second->getContextAxiom());
        answElts[semIdAbs(blockId, currMemSent.first)] = std::move(newElt);
      }
      for (auto& currElt : memSents.idToStaticSentences)
        answElts[semIdAbs(blockId, currElt.first)] = std::move(currElt.second);
      ++itMemMemBlock;
    }
  }
  if (pReqLinks.occurenceRankFilter != nullptr)
    _filterWithOccurenceRank(answElts, *pReqLinks.occurenceRankFilter);
  // keep only the 50 first answers
  keepOnlyTheFirstElements(answElts, maxSizeOfAnswers);

  if (answElts.empty())
    return;

  switch (pRequestType)
  {
  case SemanticRequestType::YESORNO:
  {
    RelatedContextAxiom relatedContextAxioms;

    // remove negative sentence in memory that are not equal to the input
    for (auto itAnsw = answElts.begin(); itAnsw != answElts.end(); )
    {
      const auto& currAnwGrdExp = itAnsw->second->getGrdExpRef();
      if (!SemExpGetter::isGrdExpPositive(currAnwGrdExp))
      {
        SemExpComparator::ComparisonExceptions compException;
        compException.request = true;
        compException.polarity = true;
        ImbricationType imbr = SemExpComparator::getGrdExpsImbrications(currAnwGrdExp, pGrdExp, pMemViewer.constView,
                                                                        pWorkStruct.lingDb, &compException);
        if (imbr != ImbricationType::EQUALS && imbr != ImbricationType::OPPOSES &&
            imbr != ImbricationType::LESS_DETAILED && imbr != ImbricationType::HYPERNYM)
        {
          itAnsw = answElts.erase(itAnsw);
          continue;
        }
      }
      ++itAnsw;
    }
    if (answElts.empty())
      return;

    for (auto& currAnsw : answElts)
      relatedContextAxioms.add(currAnsw.second->relatedContextAxioms);

    AnswerElement& moreRecentAnsElt = *(--answElts.end())->second;
    const GroundedExpression& memGrdExp = moreRecentAnsElt.getGrdExpRef();
    const SemanticExpression* equalityOfTheMemoryAnswer = nullptr;
    std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;
    static const std::vector<GrammaticalType> emptyGramTypes;
    _extractAnnotationsFromAnswer(equalityOfTheMemoryAnswer, annotationsOfTheAnswer, pRequestType,
                                  memGrdExp, moreRecentAnsElt.getAnnotations(), emptyGramTypes);

    bool samePolarity =
        SemExpComparator::haveSamePolarity(pGrdExp, memGrdExp,
                                           pWorkStruct.lingDb.conceptSet);
    if (samePolarity)
    {
      // genGrd from input -> grd exp of memory -> quantity of the grd exp from memory
      std::map<const SemanticGenericGrounding*, std::map<const GroundedExpression*, int>> alreadyCountedElts;
      for (auto itAnsw = answElts.rbegin(); itAnsw != answElts.rend(); ++itAnsw)
        _incrementNotDefiniteQuantitiesFromGrdExps(pGrdExp, itAnsw->second->getGrdExpRef(),
                                                   pMemViewer.constView, pWorkStruct.lingDb, alreadyCountedElts);

      bool sameQuantity = true;
      for (const auto& currElt : alreadyCountedElts)
      {
        int quantityFromMemory = 0;
        for (const auto& currQuantFromMem : currElt.second)
          quantityFromMemory += currQuantFromMem.second;
        if (quantityFromMemory != currElt.first->quantity.nb)
        {
          sameQuantity = false;
          break;
        }
      }

      if (!sameQuantity)
      {
        if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER ||
            pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
        {
          auto inputWithNewQuantities = pGrdExp.clone(nullptr, true);
          std::map<SemanticGenericGrounding*, std::map<const GroundedExpression*, int>> alreadyCountedElts;
          for (auto itAnsw = answElts.rbegin(); itAnsw != answElts.rend(); ++itAnsw)
            _incrementNotDefiniteQuantitiesFromGrdExps(*inputWithNewQuantities,
                                                       itAnsw->second->getGrdExpRef(),
                                                       pMemViewer.constView, pWorkStruct.lingDb, alreadyCountedElts);
          for (auto& currElt : alreadyCountedElts)
          {
            int quantityFromMemory = 0;
            for (const auto& currQuantFromMem : currElt.second)
              quantityFromMemory += currQuantFromMem.second;
            currElt.first->quantity.setNumber(quantityFromMemory);
          }

          pUserAnswers[pRequestType].answersGenerated.emplace_back(
                SemExpCreator::generateYesOrNoAnswerFromMemory(std::move(inputWithNewQuantities), false, annotationsOfTheAnswer), &relatedContextAxioms);
          return;
        }
        samePolarity = false;
      }
    }

    if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER ||
        pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
    {
      pUserAnswers[pRequestType].answersGenerated.emplace_back(
            SemExpCreator::generateYesOrNoAnswerFromQuestion(pGrdExp, samePolarity, annotationsOfTheAnswer), &relatedContextAxioms);
      return;
    }
    pUserAnswers[pRequestType].answersGenerated.emplace_back(
          SemExpCreator::sayYesOrNo(samePolarity), &relatedContextAxioms);
    return;
  }
  case SemanticRequestType::CAUSE:
  {
    AnswerElement& moreRecentAnsElt = *(--answElts.end())->second;
    _addCauseResult(pUserAnswers, moreRecentAnsElt);
    break;
  }
  case SemanticRequestType::QUANTITY:
  {
    _genericFilterSemExpThatCanAnswer(pUserAnswers, answElts, pGrdExp, pRequestType,
                                      pOriginalQuestRequestType, pWorkStruct, pMemViewer.constView);

    const SemanticUnityGrounding* unityGrdPtr = nullptr;
    auto itUnityChild = pGrdExp.children.find(GrammaticalType::UNITY);
    if (itUnityChild != pGrdExp.children.end())
    {
      auto* unityGrdExpPtr = itUnityChild->second->getGrdExpPtr_SkipWrapperPtrs();
      if (unityGrdExpPtr != nullptr)
        unityGrdPtr = unityGrdExpPtr->grounding().getUnityGroundingPtr();
    }
    _replaceAnswersByNumberOfInstances(pUserAnswers, unityGrdPtr);
    return;
  }
  default:
    break;
  }
  _genericFilterSemExpThatCanAnswer(pUserAnswers, answElts, pGrdExp, pRequestType,
                                    pOriginalQuestRequestType, pWorkStruct, pMemViewer.constView);
}



void checkNominalGrdExp
(SemControllerWorkingStruct& pWorkStruct,
 const GroundedExpression& pGrdExp)
{
  TruenessValue agrementRes = semExpAgreementDetector::getAgreementValue(pGrdExp);
  if (agrementRes != TruenessValue::UNKNOWN)
  {
    pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&agrementRes]
    {
      auto leafAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
      leafAnsw->answerElts[SemanticRequestType::YESORNO].answersGenerated.emplace_back
          (SemExpCreator::sayYesOrNo(agrementRes == TruenessValue::VAL_TRUE));
      return leafAnsw;
    }());
  }
}


// ex:
// H: I am 6
// H: Am I 7 ?
// R: No, you are 6 <= This answer is generated by this function because the pattern "I am + <age>" is unique
bool _answerToYesOrNoQuestionIfTheSemExpCorrespondToUniquePatternThatAlreadyHaveAValue(SemControllerWorkingStruct& pWorkStruct,
                                                                                       SemanticMemoryBlockViewer& pMemViewer,
                                                                                       const GroundedExpression& pGrdExp)
{
  mystd::unique_propagate_const<UniqueSemanticExpression> childThatShouldBeUnique;
  GrammaticalType grammTypeOfChildThatShouldBeUnique =
      pWorkStruct.lingDb.treeConverter.getChildThatShouldBeUnique(childThatShouldBeUnique, pGrdExp);
  if (grammTypeOfChildThatShouldBeUnique != GrammaticalType::UNKNOWN)
  {
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion())
    {
      subWorkStruct.reactOperator = SemanticOperatorEnum::GET;

      auto subQuestionGrdExpPtr = pGrdExp.clone();
      {
        GroundedExpression& subQuestionGrdExp = *subQuestionGrdExpPtr;
        SemanticRequests subQuestionRequests;
        subQuestionRequests.set(SemExpGetter::convertSemGramToRequestType(grammTypeOfChildThatShouldBeUnique));
        SemExpModifier::swapRequests(subQuestionGrdExp, subQuestionRequests);
        if (childThatShouldBeUnique && !(*childThatShouldBeUnique)->isEmpty())
          SemExpModifier::setChild(subQuestionGrdExp, grammTypeOfChildThatShouldBeUnique, std::move(*childThatShouldBeUnique));
        else
          SemExpModifier::removeChild(subQuestionGrdExp, grammTypeOfChildThatShouldBeUnique);
      }

      UniqueSemanticExpression subQuestionSemExp(std::move(subQuestionGrdExpPtr));
      converter::splitPossibilitiesOfQuestions(subQuestionSemExp, pWorkStruct.lingDb);
      controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *subQuestionSemExp);
      if (!subWorkStruct.compositeSemAnswers->semAnswers.empty())
      {
        std::list<std::unique_ptr<SemAnswer>> detailledAnswers;
        RelatedContextAxiom relatedContextAxioms;
        subWorkStruct.compositeSemAnswers->getSourceContextAxiom(relatedContextAxioms);

        controller::convertToDetalledAnswer(detailledAnswers, subWorkStruct);
        std::list<const GroundedExpression*> grdExpAnswers;
        CompositeSemAnswer::getGrdExps(grdExpAnswers, detailledAnswers);

        if (grdExpAnswers.size() == 1)
        {
          auto itSubordinateThatShouldBeUnique = pGrdExp.children.find(grammTypeOfChildThatShouldBeUnique);
          if (itSubordinateThatShouldBeUnique != pGrdExp.children.end())
          {
            if (SemExpComparator::semExpsAreEqualFromMemBlock(*itSubordinateThatShouldBeUnique->second,
                                                              *grdExpAnswers.front(), pMemViewer.constView, pWorkStruct.lingDb, nullptr))
            {
              if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
              {
                std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;
                pWorkStruct.addAnswer
                    (ContextualAnnotation::ANSWER,
                     SemExpCreator::generateYesOrNoAnswerFromQuestion(pGrdExp, true, annotationsOfTheAnswer),
                     ReferencesFiller(relatedContextAxioms));
              }
              else
              {
                //subSemAnswer.getSourceContextAxiom(relatedContextAxioms);
                auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                AllAnswerElts& allAnswElts = newAnsw->answerElts[SemanticRequestType::YESORNO];
                allAnswElts.answersGenerated.emplace_back(SemExpCreator::sayYesOrNo(true), &relatedContextAxioms);
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
              }
              return true;
            }
          }
        }


        if (!grdExpAnswers.empty())
        {
          if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT)
          {
            auto answerGrdExp = pGrdExp.clone();
            SemExpModifier::clearRequestList(*answerGrdExp);
            SemExpModifier::setChild(*answerGrdExp, grammTypeOfChildThatShouldBeUnique,
                                     grdExpAnswers.front()->clone());
            pWorkStruct.addAnswer(ContextualAnnotation::ANSWER,
                                  SemExpCreator::generateYesOrNoAnswer(std::move(answerGrdExp), false),
                                  ReferencesFiller(relatedContextAxioms));
          }
          else
          {

            auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
            AllAnswerElts& allAnswElts = newAnsw->answerElts[SemanticRequestType::YESORNO];
            allAnswElts.answersGenerated.emplace_back(SemExpCreator::sayYesOrNo(false), &relatedContextAxioms);
            pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
          }
          return true;
        }
      }
    }
  }
  return false;
}



bool satisfyAQuestion(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const GroundedExpression& pGrdExp,
                      const std::list<const GroundedExpression*>& pOtherGrdExps,
                      const GroundedExpression& pOriginalGrdExp,
                      const SemanticRequests& pRequests)
{
  // get the triggers
  bool anAnswerHasBeenAdded = false;
  if (pWorkStruct.reactionOptions.canAnswerWithATrigger &&
      (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
       pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER))
  {
    // TODO: get the links if the there is some triggers to optimize!
    RequestLinks reqLinksOfOriginalGrdExp;
    getLinksOfAGrdExp(reqLinksOfOriginalGrdExp, pWorkStruct, pMemViewer, pOriginalGrdExp, false);
    if (addTriggerSentencesAnswer(pWorkStruct, anAnswerHasBeenAdded, pMemViewer, reqLinksOfOriginalGrdExp,
                                  SemanticExpressionCategory::QUESTION, _emptyAxiomId,
                                  pOriginalGrdExp, ContextualAnnotation::ANSWER))
      return true;
  }

  for (const auto& currRequest : pRequests.types)
  {
    GrammaticalType grammTypeOfQuestion = semanticRequestType_toSemGram(currRequest);
    if (grammTypeOfQuestion != GrammaticalType::UNKNOWN)
    {
      auto itChildOfQuestion = pGrdExp.children.find(grammTypeOfQuestion);
      if (itChildOfQuestion != pGrdExp.children.end())
      {
        const ListExpression* listExpOfQuestionPtr =
            itChildOfQuestion->second->getListExpPtr_SkipWrapperPtrs();
        if (listExpOfQuestionPtr != nullptr &&
            listExpOfQuestionPtr->listType == ListExpressionType::OR)
          return manageChoice(pWorkStruct, pMemViewer, pGrdExp,
                              grammTypeOfQuestion, *listExpOfQuestionPtr) || anAnswerHasBeenAdded;
      }
    }
  }

  if (!pWorkStruct.canHaveAnotherTextualAnswer() ||
      answerToSpecificQuestions::process(pWorkStruct, pMemViewer, pGrdExp, pRequests.firstOrNothing()))
    return true;

  // store the links of the current sentence
  RequestLinks reqLinks;
  getLinksOfAGrdExp(reqLinks, pWorkStruct, pMemViewer, pGrdExp);

  auto newAnsw = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
  std::map<SemanticRequestType, AllAnswerElts>& allAnswers = newAnsw->answerElts;
  for (const auto& currRequest : pRequests.types)
  {
    _getRelationsOfLinks(allAnswers, pMemViewer, pWorkStruct,
                         reqLinks, pGrdExp, currRequest, currRequest);

    for (const auto& currOtherGrdExp : pOtherGrdExps)
    {
      const auto* requests = SemExpGetter::getRequestList(*currOtherGrdExp);
      if (requests != nullptr && !requests->empty())
      {
        RequestLinks subReqLinks;
        getLinksOfAGrdExp(subReqLinks, pWorkStruct, pMemViewer, *currOtherGrdExp);
        SemanticRequestType subRequestType = requests->first();
        _getRelationsOfLinks(allAnswers, pMemViewer, pWorkStruct,
                             subReqLinks, *currOtherGrdExp, subRequestType, currRequest);
      }
    }
  }

  if (pRequests.has(SemanticRequestType::YESORNO) &&
      allAnswers.empty() &&
      _answerToYesOrNoQuestionIfTheSemExpCorrespondToUniquePatternThatAlreadyHaveAValue(pWorkStruct, pMemViewer, pGrdExp))
    return true;

  if (pWorkStruct.reactionOptions.canAnswerWithExternalEngines &&
      pWorkStruct.callbackToSentencesCanBeAnsweredPtr != nullptr &&
      allAnswers.empty())
  {
    for (const auto& currCallbackToSents : *pWorkStruct.callbackToSentencesCanBeAnsweredPtr)
    {
      const auto& memBlock = currCallbackToSents->memBlock;
      SemanticMemoryBlockViewer subMemViewer(nullptr, memBlock, SemanticAgentGrounding::userNotIdentified);
      auto& subMemBlockViewer = subMemViewer.getConstViewPrivate();
      auto awLinks = subMemBlockViewer.getLinks(SemanticTypeOfLinks::ANSWER, reqLinks.tense, reqLinks.verbGoal);
      RelationsThatMatch<false> idsToSentences;
      MemoryBlockPrivateAccessorPtr<false> memBlockPrivateAccessor(&subMemBlockViewer);
      SemControllerWorkingStruct subWorkStruct(pWorkStruct.informationType, pWorkStruct.authorSemExp,
                                               pWorkStruct.fromLanguage, pWorkStruct.expHandleInMemory,
                                               pWorkStruct.reactOperator, nullptr, nullptr, nullptr, nullptr,
                                               pWorkStruct.lingDb);
      _getResultFromMemory(idsToSentences, awLinks, subWorkStruct, memBlockPrivateAccessor, subMemViewer,
                           reqLinks, false, semanticMemoryGetter::RequestContext::SENTENCE);
      for (const auto& currRel : idsToSentences.res.dynamicLinks)
      {
        auto params = std::make_unique<IndexToSubNameToParameterValue>();
        std::unique_ptr<InteractionContextContainer> subIntContext;
        unknownInfosGetter::checkIfMatchAndGetParams(*params, subIntContext, nullptr, *currRel.second,
                                                     pGrdExp, pWorkStruct, subMemViewer);

        SemanticLanguageEnum searchLangType = pWorkStruct.fromLanguage;
        if (searchLangType == SemanticLanguageEnum::UNKNOWN)
          searchLangType = SemanticLanguageEnum::ENGLISH;
        try
        {
          auto answer = currCallbackToSents->getAnswer(*params, searchLangType);
          if (answer)
          {
            RelatedContextAxiom relatedContextAxiom;
            relatedContextAxiom.constElts.emplace_back(&currRel.second->getContextAxiom());
            allAnswers[pRequests.firstOrNothing()].answersGenerated.emplace_back(std::move(*answer), &relatedContextAxiom);
          }
        }
        catch (const std::exception& e)
        {
          std::cerr << "Exception caught when calling an external answer method. The error was: " << e.what() << std::endl;
        }
      }
    }
  }

  // if the answer is not found in the memory, we try to answer from inside the question
  if (allAnswers.empty())
  {
    auto rootRequest = pRequests.firstOrNothing();
    answerFromDataStoredInsideTheQuestion::getAnswers(allAnswers, pGrdExp, rootRequest, pWorkStruct.lingDb);
    if (allAnswers.empty())
      for (const auto& currOtherGrdExp : pOtherGrdExps)
        if (allAnswers.empty())
          answerFromDataStoredInsideTheQuestion::getAnswers(allAnswers, *currOtherGrdExp, rootRequest, pWorkStruct.lingDb);
  }

  if (allAnswers.empty())
  {
    if (pRequests.has(SemanticRequestType::TIMES))
    {
      switch (pWorkStruct.reactOperator)
      {
      case SemanticOperatorEnum::ANSWER:
      case SemanticOperatorEnum::REACT:
      {
        pWorkStruct.addAnswerWithoutReferences
            (ContextualAnnotation::ANSWER,
             SemExpCreator::generateNumberOfTimesAnswer(pOriginalGrdExp, 0));
        return true;
      }
      default:
        return anAnswerHasBeenAdded;
      }
    }
    else
    {
      return anAnswerHasBeenAdded;
    }
  }
  else
  {
    switch (pWorkStruct.reactOperator)
    {
    case SemanticOperatorEnum::ANSWER:
    case SemanticOperatorEnum::REACT:
    {
      std::list<std::string> references;
      auto answerSemExpOpt = SemExpCreator::generateAnswer
          (allAnswers, references, pOriginalGrdExp, pRequests, pMemViewer.constView, pWorkStruct.lingDb);
      allAnswers.clear();

      if (answerSemExpOpt)
      {
        auto sentimentSpec = sentimentDetector::extractMainSentiment
           (pOriginalGrdExp, *pWorkStruct.author, pWorkStruct.lingDb.conceptSet);
        if (sentimentSpec && sentimentSpec->sentimentStrengh > 1) // 1 and not 0 because we don't want to react on too weak sentiments
          SemExpModifier::addNewSemExp(*answerSemExpOpt, SemExpCreator::sayThanks());
        pWorkStruct.addAnswer(ContextualAnnotation::ANSWER, std::move(*answerSemExpOpt),
                              ReferencesFiller(references));
        return true;
      }
      break;
    }
    case SemanticOperatorEnum::HOWYOUKNOW:
    {
      howYouThatAnswer::process(newAnsw);
      break;
    }
    case SemanticOperatorEnum::CHECK:
    case SemanticOperatorEnum::GET:
    case SemanticOperatorEnum::FEEDBACK:
    case SemanticOperatorEnum::FIND:
    case SemanticOperatorEnum::RESOLVECOMMAND:
    case SemanticOperatorEnum::EXECUTEBEHAVIOR:
    case SemanticOperatorEnum::EXECUTEFROMTRIGGER:
    case SemanticOperatorEnum::INFORM:
    case SemanticOperatorEnum::REACTFROMTRIGGER:
    case SemanticOperatorEnum::SHOW:
    case SemanticOperatorEnum::TEACHBEHAVIOR:
    case SemanticOperatorEnum::TEACHCONDITION:
    case SemanticOperatorEnum::TEACHINFORMATION:
    case SemanticOperatorEnum::UNINFORM:
      break;
    }
  }

  if (newAnsw)
    pWorkStruct.compositeSemAnswers->semAnswers.emplace_back(std::move(newAnsw));
  return true;
}


} // End of namespace semanticMemoryLinker
} // End of namespace onsem
