#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include "controller/steps/semanticmemorylinker.hpp"
#include "controller/semexpcontroller.hpp"
#include "conversion/conditionsadder.hpp"
#include "operator/answeridontknow.hpp"
#include "operator/externalteachingrequester.hpp"
#include "operator/semanticcategorizer.hpp"
#include "type/semanticdetailledanswer.hpp"
#include "semanticmemory/memorymodifier.hpp"
#include "interpretation/addagentinterpretation.hpp"
#include "interpretation/completewithcontext.hpp"
#include "semanticmemory/semanticmemoryblockviewer.hpp"
#include "utility/semexpcreator.hpp"
#include "utility/utility.hpp"


namespace onsem
{
namespace memoryOperation
{

namespace
{

// TODO: solve hacks in this function
bool _hasObjectExecptCoreference(const GroundedExpression& pGrdExp)
{
  if (SemExpGetter::hasChild(pGrdExp, GrammaticalType::SPECIFIER))
    return true;
  auto itObjectChild = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObjectChild != pGrdExp.children.end())
  {
    const SemanticExpression& objectSepExp = *itObjectChild->second;
    const GroundedExpression* objectGrdExpPtr = objectSepExp.getGrdExpPtr_SkipWrapperPtrs();
    if (objectGrdExpPtr != nullptr)
    {
      const SemanticGenericGrounding* objectGenGrdPtr = objectGrdExpPtr->grounding().getGenericGroundingPtr();
      if (objectGenGrdPtr != nullptr && objectGenGrdPtr->coreference)
        return false;
    }
    return true;
  }
  return false;
}

}




void _cloneGrdExpList(std::vector<std::unique_ptr<GroundedExpression>>& pAnswers,
                      const std::list<const GroundedExpression*>& pGrdExpList)
{
  static const std::set<GrammaticalType> introWordGrammType = {GrammaticalType::INTRODUCTING_WORD};
  pAnswers.resize(pGrdExpList.size());
  std::size_t i = 0;
  for (const auto& currGrdExp : pGrdExpList)
  {
    pAnswers[i] = currGrdExp->clone(nullptr, false, nullptr, &introWordGrammType);
    ++i;
  }
}



mystd::unique_propagate_const<UniqueSemanticExpression> answer
(UniqueSemanticExpression pSemExp,
 bool pCanAnswerIDontKnow,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb)
{
  converter::splitPossibilitiesOfQuestions(pSemExp, pLingDb);
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  auto currUserId = pSemanticMemory.getCurrUserId();
  const auto* externalFallback = pSemanticMemory.getExternalFallback();
  const auto& semExp = *pSemExp;
  controller::applyOperatorOnSemExpConstMem(compositeSemAnswers, semExp,
                                            SemanticOperatorEnum::ANSWER, InformationType::INFORMATION,
                                            pSemanticMemory.memBloc, currUserId, &pSemanticMemory.proativeSpecifications,
                                            externalFallback, &pSemanticMemory.callbackToSentencesCanBeAnswered,
                                            nullptr, pLingDb, nullptr, &pCanAnswerIDontKnow);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  if (compositeSemAnswers)
    controller::compAnswerToSemExp(res, *compositeSemAnswers);
  return res;
}

mystd::unique_propagate_const<UniqueSemanticExpression> answerIDontKnow(const SemanticExpression& pSemExp)
{
  return privateImplem::answerIDontKnow(pSemExp, true, false);
}

mystd::unique_propagate_const<UniqueSemanticExpression> answerICannotDo(const SemanticExpression& pSemExp)
{
  return privateImplem::answerIDontKnow(pSemExp, false, true);
}


mystd::unique_propagate_const<UniqueSemanticExpression> notKnowing(const SemanticExpression& pSemExp)
{
  return privateImplem::answerIDontKnow(pSemExp, true, true);
}


mystd::unique_propagate_const<UniqueSemanticExpression> sayFeedback
(const SemanticExpression& pSemExp,
 SemanticTypeOfFeedback pTypeOfFeedBack,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  controller::applyOperatorOnSemExpConstMem(compositeSemAnswers, pSemExp,
                                            SemanticOperatorEnum::FEEDBACK,
                                            InformationType::INFORMATION,
                                            pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(), &pSemanticMemory.proativeSpecifications,
                                            pSemanticMemory.getExternalFallback(), &pSemanticMemory.callbackToSentencesCanBeAnswered,
                                            nullptr, pLingDb, &pTypeOfFeedBack);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  if (compositeSemAnswers)
    controller::compAnswerToSemExp(res, *compositeSemAnswers);
  return res;
}


SemanticExpressionCategory categorize(const SemanticExpression& pSemExp)
{
  return privateImplem::categorize(pSemExp);
}


TruenessValue check(const SemanticExpression& pSemExp,
                    const SemanticMemoryBlock& pMemBlock,
                    const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  controller::applyOperatorOnSemExpConstMem(compositeSemAnswers, pSemExp,
                                            SemanticOperatorEnum::CHECK, InformationType::INFORMATION,
                                            pMemBlock, SemanticAgentGrounding::userNotIdentified,
                                            nullptr, nullptr, nullptr, nullptr, pLingDb);
  if (compositeSemAnswers)
    return compositeSemAnswers->getAgreementValue();
  return TruenessValue::UNKNOWN;
}



SemanticEngagementValue _extractEngagementWordConcept(const SemanticExpression& pSemExp,
                                                      const SemanticWord* pWordToSkip)
{
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const auto& grd = grdExpPtr->grounding();
    if (ConceptSet::haveAConceptThatBeginWith(grd.concepts, "engagement_"))
    {
      const SemanticGenericGrounding* genGrdPtr = grd.getGenericGroundingPtr();
      if (pWordToSkip != nullptr && genGrdPtr != nullptr && genGrdPtr->word == *pWordToSkip)
        return SemanticEngagementValue::UNKNWON;
      if (grd.concepts.count("engagement_engage") > 0)
        return SemanticEngagementValue::ENGAGE;
      return SemanticEngagementValue::DISENGAGE_GOODBYE;
    }
  }
  return SemanticEngagementValue::UNKNWON;
}

SemanticEngagementValue extractEngagement(const SemanticExpression& pSemExp)
{
  SemanticWord byeWord(SemanticLanguageEnum::ENGLISH, "bye", PartOfSpeech::INTERJECTION);

  // extract engagement on interjections
  {
    SemanticEngagementValue res = _extractEngagementWordConcept(pSemExp, &byeWord);
    if (res != SemanticEngagementValue::UNKNWON)
      return res;
  }

  const FeedbackExpression* fdkExpPtr = pSemExp.getFdkExpPtr_SkipWrapperPtrs();
  if (fdkExpPtr != nullptr)
  {
    SemanticEngagementValue res = _extractEngagementWordConcept(*fdkExpPtr->feedbackExp, &byeWord);
    if (res != SemanticEngagementValue::UNKNWON)
    {
      const GroundedExpression* concernedExpPtr =
          fdkExpPtr->concernedExp->getGrdExpPtr_SkipWrapperPtrs();
      if (concernedExpPtr != nullptr)
      {
        const SemanticGrounding& concernedGrd = concernedExpPtr->grounding();
        const auto* concernedGenGrdPtr = concernedGrd.getGenericGroundingPtr();
        if (concernedGenGrdPtr != nullptr)
        {
          if (concernedGenGrdPtr->word.lemma == "robot")
            return res;
        }
        else
        {
          const auto* concernedNameGrdPtr = concernedGrd.getNameGroundingPtr();
          if (concernedNameGrdPtr != nullptr)
          {
            if (concernedNameGrdPtr->nameInfos.names.size() == 1 &&
                concernedNameGrdPtr->nameInfos.names.front() == "Buddy")
              return res;
          }
        }
      }
    }
  }

  // extract engagement on sentences
  const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    // filter verb haveto/need in french
    const SemanticStatementGrounding* statementGrdPtr =
        grdExpPtr->grounding().getStatementGroundingPtr();
    if (statementGrdPtr != nullptr &&
        statementGrdPtr->verbTense == SemanticVerbTense::PRESENT)
    {
      // filter user has the subject
      const GroundedExpression* subjectGrdExpPtr =
          SemExpGetter::getGrdExpChild(*grdExpPtr, GrammaticalType::SUBJECT);
      if (subjectGrdExpPtr != nullptr)
      {
        const SemanticAgentGrounding* subjectAgentGrdPtr =
            subjectGrdExpPtr->grounding().getAgentGroundingPtr();
        if (subjectAgentGrdPtr != nullptr &&
            subjectAgentGrdPtr->userId == SemanticAgentGrounding::currentUser)
        {
          const GroundedExpression* actionGrdExpPtr = nullptr;
          if (statementGrdPtr->verbGoal == VerbGoalEnum::MANDATORY)
            actionGrdExpPtr = grdExpPtr;
          else if (ConceptSet::haveAConcept(statementGrdPtr->concepts, "verb_need"))
            actionGrdExpPtr = SemExpGetter::getGrdExpChild(*grdExpPtr, GrammaticalType::OBJECT);

          // filter object go/leave
          if (actionGrdExpPtr != nullptr)
          {
            const GroundedExpression& actionGrdExp = *actionGrdExpPtr;
            const auto* actionStatementGrdPtr = actionGrdExp->getStatementGroundingPtr();
            if (actionStatementGrdPtr != nullptr &&
                ConceptSet::haveAnyOfConcepts(actionStatementGrdPtr->concepts, {"verb_action_go", "verb_action_leave"}) &&
                (!_hasObjectExecptCoreference(actionGrdExp) ||
                 SemExpGetter::isNow(actionGrdExp)))
              return SemanticEngagementValue::DISENGAGE_NEEDTOGO;
          }
        }
      }
    }
  }

  return SemanticEngagementValue::UNKNWON;
}


bool isASubpart(const SemanticExpression& pInputSemExp,
                const SemanticExpression& pSemExpToFind,
                const SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb)
{
  const ListExpression* listExp = pInputSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExp != nullptr &&
      (listExp->listType == ListExpressionType::UNRELATED ||
       listExp->listType == ListExpressionType::AND))
    for (auto& currElt : listExp->elts)
      if (isASubpart(*currElt, pSemExpToFind, pSemanticMemory, pLingDb))
        return true;

  ImbricationType semExpsImbrications =
      SemExpComparator::getSemExpsImbrications(pInputSemExp, pSemExpToFind, pSemanticMemory.memBloc, pLingDb, nullptr);
  if (semExpsImbrications == ImbricationType::EQUALS ||
      semExpsImbrications == ImbricationType::MORE_DETAILED ||
      semExpsImbrications == ImbricationType::HYPONYM)
    return true;

  // try to match with a feedback
  const GroundedExpression* grdExptrToMatch = pSemExpToFind.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExptrToMatch != nullptr)
  {
    const SemanticGenericGrounding* genGrdPtrToMatch = grdExptrToMatch->grounding().getGenericGroundingPtr();
    if (genGrdPtrToMatch != nullptr &&
        genGrdPtrToMatch->word.partOfSpeech == PartOfSpeech::INTERJECTION)
    {
      const FeedbackExpression* inputFdkExpPtr = pInputSemExp.getFdkExpPtr_SkipWrapperPtrs();
      if (inputFdkExpPtr != nullptr)
      {
        ImbricationType feedbackImbrications =
            SemExpComparator::getSemExpsImbrications(*inputFdkExpPtr->feedbackExp, pSemExpToFind, pSemanticMemory.memBloc, pLingDb, nullptr);
        if (feedbackImbrications == ImbricationType::EQUALS ||
            feedbackImbrications == ImbricationType::MORE_DETAILED ||
            feedbackImbrications == ImbricationType::HYPONYM)
          return true;
      }
    }
  }

  return pSemExpToFind.isEmpty();
}


void get(std::vector<std::unique_ptr<GroundedExpression>>& pAnswers,
         UniqueSemanticExpression pSemExp,
         const SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb)
{
  converter::splitPossibilitiesOfQuestions(pSemExp, pLingDb);
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  auto currUserId = pSemanticMemory.getCurrUserId();
  const auto* externalFallback = pSemanticMemory.getExternalFallback();
  const auto& semExp = *pSemExp;
  controller::applyOperatorOnSemExpConstMem(compositeSemAnswers, semExp,
                                            SemanticOperatorEnum::GET,
                                            InformationType::INFORMATION,
                                            pSemanticMemory.memBloc, currUserId,
                                            &pSemanticMemory.proativeSpecifications, externalFallback,
                                            &pSemanticMemory.callbackToSentencesCanBeAnswered, nullptr, pLingDb);

  if (compositeSemAnswers)
  {
    std::list<const GroundedExpression*> grdExpAnswers;
    CompositeSemAnswer::getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
    if (!grdExpAnswers.empty())
      _cloneGrdExpList(pAnswers, grdExpAnswers);
  }
}


void howYouKnow(std::vector<std::unique_ptr<GroundedExpression>>& pAnswers,
                const SemanticExpression& pSemExp,
                const SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  controller::applyOperatorOnSemExpConstMem(compositeSemAnswers, pSemExp,
                                            SemanticOperatorEnum::HOWYOUKNOW,
                                            InformationType::INFORMATION,
                                            pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(),
                                            &pSemanticMemory.proativeSpecifications, pSemanticMemory.getExternalFallback(),
                                            &pSemanticMemory.callbackToSentencesCanBeAnswered, nullptr, pLingDb);
  if (compositeSemAnswers)
  {
    std::list<const GroundedExpression*> grdExpAnswers;
    CompositeSemAnswer::getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
    if (!grdExpAnswers.empty())
      _cloneGrdExpList(pAnswers, grdExpAnswers);
  }
}



mystd::unique_propagate_const<UniqueSemanticExpression> generateDefintionFromAnOldOrder(const SemanticExpression& pOrderSemExp,
                                                                          const GroundedExpression& pNewLabel,
                                                                          const SemanticMemory& pSemanticMemory,
                                                                          const linguistics::LinguisticDatabase& pLingDb)
{
  auto copyGrdExpAtImperative = [](const GroundedExpression& pGrdExp,
                                     const SemanticStatementGrounding& pStatGrd)
  {
    auto res = std::make_unique<GroundedExpression>
        ([&pStatGrd]
    {
      auto resStat = std::make_unique<SemanticStatementGrounding>(pStatGrd);
      resStat->requests.clear();
      resStat->verbTense = SemanticVerbTense::UNKNOWN;
      return resStat;
    }());
    for (const auto& currChild : pGrdExp.children)
      if (currChild.first != GrammaticalType::SUBJECT)
        res->children.emplace(currChild.first, currChild.second->clone(nullptr, true));
    return res;
  };

  const GroundedExpression* grdExpPtr = pOrderSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const GroundedExpression& grdExp = *grdExpPtr;
    auto* statGrdPtr = grdExp->getStatementGroundingPtr();
    if (statGrdPtr != nullptr)
    {
      auto& statGrd = *statGrdPtr;
      if (statGrd.requests.types.size() == 1 &&
          statGrd.requests.first() == SemanticRequestType::ACTION)
      {
        auto subActionAtInfinitive = copyGrdExpAtImperative(grdExp, statGrd);

        // if subLabel == newLabel, we replace the subLabel by his definition
        if (SemExpComparator::grdExpsAreEqual(*subActionAtInfinitive, pNewLabel, pSemanticMemory.memBloc, pLingDb))
        {
          std::vector<std::unique_ptr<GroundedExpression>> subDefinition;
          get(subDefinition, std::move(subActionAtInfinitive), pSemanticMemory, pLingDb);
          if (!subDefinition.empty())
            return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(subDefinition.front()));
          return mystd::unique_propagate_const<UniqueSemanticExpression>();
        }
        return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(subActionAtInfinitive));
      }
      return mystd::unique_propagate_const<UniqueSemanticExpression>();
    }
    if (grdExp->getResourceGroundingPtr() != nullptr)
      return mystd::unique_propagate_const<UniqueSemanticExpression>(grdExp.clone(nullptr, true));
    return mystd::unique_propagate_const<UniqueSemanticExpression>();
  }

  const ListExpression* listExpPtr = pOrderSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExpPtr != nullptr)
  {
    auto res = std::make_unique<ListExpression>(listExpPtr->listType);
    for (const auto& currElt : listExpPtr->elts)
    {
      auto subRes = generateDefintionFromAnOldOrder(*currElt, pNewLabel, pSemanticMemory, pLingDb);
      if (!subRes)
        return mystd::unique_propagate_const<UniqueSemanticExpression>();
      res->elts.emplace_back(std::move(*subRes));
    }
    if (!res->elts.empty())
      return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(res));
  }
  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}



void mergeWithContext(UniqueSemanticExpression& pSemExp,
                      const SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  auto* lastText = pSemanticMemory.memBloc.getLastSemExpFromContext();
  if (lastText != nullptr)
  {
    const SemanticExpression* currAuthorPtr = SemExpGetter::extractAuthorSemExp(*pSemExp);
    const SemanticExpression* contextAuthorPtr = SemExpGetter::extractAuthorSemExp(*lastText);
    bool sameAuthor = false;
    if (currAuthorPtr != nullptr && contextAuthorPtr != nullptr)
      sameAuthor = SemExpComparator::semExpsAreEqualFromMemBlock(*currAuthorPtr, *contextAuthorPtr,
                                                                 pSemanticMemory.memBloc, pLingDb, nullptr);

    if (!sameAuthor && currAuthorPtr != nullptr)
    {
      const SemanticExpression& currAuthor = *currAuthorPtr;
      const GroundedExpression* actionLabelPtr = nullptr;
      const SemanticStatementGrounding* equalityStatementPtr = nullptr;
      if (SemExpGetter::isItAnActionLabeling(actionLabelPtr, equalityStatementPtr, *pSemExp))
      {
        assert(actionLabelPtr != nullptr);
        const GroundedExpression& actionLabel = *actionLabelPtr;
        auto* beforeLastTextPtr = pSemanticMemory.memBloc.getBeforeLastSemExpOfAnAuthor(currAuthor, pLingDb);
        if (beforeLastTextPtr != nullptr)
        {
          auto& beforeLastText = *beforeLastTextPtr;
          auto defSemExp = generateDefintionFromAnOldOrder(beforeLastText, actionLabel, pSemanticMemory, pLingDb);
          if (defSemExp)
            pSemExp = std::make_unique<InterpretationExpression>
                (InterpretationSource::RECENTCONTEXT,
                 SemExpCreator::formulateActionDefinition(actionLabel, *equalityStatementPtr, std::move(*defSemExp)),
                 std::move(pSemExp));
        }
      }
    }
    completeWithContext(pSemExp, GrammaticalType::UNKNOWN, *lastText, sameAuthor, currAuthorPtr, pSemanticMemory.memBloc, pLingDb);
  }
  controller::applyOperatorResolveAgentAccordingToTheContext(pSemExp, pSemanticMemory, pLingDb);
}


void resolveAgentAccordingToTheContext(UniqueSemanticExpression& pSemExp,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb)
{
  controller::applyOperatorResolveAgentAccordingToTheContext(pSemExp, pSemanticMemory, pLingDb);
}

void addAgentInterpretations(UniqueSemanticExpression& pSemExp,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb)
{
  agentInterpretations::addAgentInterpretations(pSemExp, pSemanticMemory, pLingDb);
}



void track(SemanticMemory& pSemanticMemory,
           UniqueSemanticExpression pSemExp,
           std::shared_ptr<SemanticTracker>& pSemTracker,
           const linguistics::LinguisticDatabase& pLingDb)
{
  pSemanticMemory.memBloc.addTrackerSemExp(std::move(pSemExp), pSemTracker, pLingDb);
}


void untrack(SemanticMemoryBlock& pMemBlock,
             std::shared_ptr<SemanticTracker>& pSemTracker,
             const linguistics::LinguisticDatabase& pLingDb,
             std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  pMemBlock.removeTrackerSemExp(pSemTracker, pLingDb, pAxiomToConditionCurrentStatePtr);
}



std::shared_ptr<ExpressionWithLinks> _informMetaMemory
(UniqueSemanticExpression pSemExp,
 SemanticMemoryBlock& pMemBloc,
 SemanticMemory& pSemanticMemory,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
 const linguistics::LinguisticDatabase& pLingDb,
 InformationType pInformationType,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr)
{
  conditionsAdder::addConditonsForSomeTimedGrdExp(pSemExp);
  auto newExpForMem = pMemBloc.addRootSemExp(std::move(pSemExp), pLingDb, pLinkedInfosPtr);
  ExpressionWithLinks& newExpForMemRef = *newExpForMem;

  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  controller::applyOperatorOnExpHandleInMemory(compositeSemAnswers, newExpForMemRef,
                                               SemanticOperatorEnum::INFORM, pInformationType,
                                               pSemanticMemory, pAxiomToConditionCurrentStatePtr,
                                               pLingDb, nullptr);

  if (compositeSemAnswers)
  {
    controller::linkConditionalReactions(compositeSemAnswers->semAnswers, newExpForMemRef,
                                         pSemanticMemory, pLingDb, pInformationType);
    controller::sendActionProposalIfNecessary(*compositeSemAnswers, pSemanticMemory.memBloc);
  }
  return newExpForMem;
}


void notifyPunctually(const SemanticExpression& pSemExp,
                      InformationType pInformationType,
                      SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compositeSemAnswers;
  controller::applyOperatorOnSemExp(compositeSemAnswers, pSemExp,
                                    SemanticOperatorEnum::INFORM, pInformationType,
                                    pSemanticMemory, pLingDb);
  if (compositeSemAnswers)
  {
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    controller::compAnswerToSemExp(reaction, *compositeSemAnswers);
    if (reaction)
      pSemanticMemory.memBloc.actionProposalSignal(*reaction);
  }
}

std::shared_ptr<ExpressionWithLinks> inform
(UniqueSemanticExpression pSemExp,
 SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  return _informMetaMemory(std::move(pSemExp), pSemanticMemory.memBloc,
                           pSemanticMemory, pAxiomToConditionCurrentStatePtr,
                           pLingDb, InformationType::INFORMATION, pLinkedInfosPtr);
}


std::shared_ptr<ExpressionWithLinks> informAxiom
(UniqueSemanticExpression pSemExp,
 SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr)
{
  return _informMetaMemory(std::move(pSemExp), pSemanticMemory.memBloc,
                           pSemanticMemory, pAxiomToConditionCurrentStatePtr,
                           pLingDb, InformationType::ASSERTION, pLinkedInfosPtr);
}


void _informAxioms(const std::vector<std::string>& pInfomations,
                   SemanticLanguageEnum pLanguage,
                   SemanticMemory& pSemanticMemory,
                   const linguistics::LinguisticDatabase& pLingDb)
{
  TextProcessingContext textProcContext(SemanticAgentGrounding::me,
                                        SemanticAgentGrounding::userNotIdentified,
                                        pLanguage);
  for (const auto& currInfo : pInfomations)
    informAxiom(converter::textToSemExp(currInfo, textProcContext, pLingDb),
                      pSemanticMemory, pLingDb);
}


void learnSayCommand(SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb)
{
  _informAxioms({"to say \\p_meta=0\\ means \\p_meta=0\\",
                 "to ask \\p_meta=0_#dontanswer\\ means \\p_meta=0_#dontanswer\\",
                 "to repeat means to say the last thing that I said"},
                SemanticLanguageEnum::ENGLISH, pSemanticMemory, pLingDb);
}

void allowToInformTheUserHowToTeach(SemanticMemory& pSemanticMemory)
{
  pSemanticMemory.proativeSpecifications.informTheUserHowToTeachMe = true;
}


void defaultKnowledge(SemanticMemory& pSemanticMemory,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  learnSayCommand(pSemanticMemory, pLingDb);
  _informAxioms({"je vais bien"},
                      SemanticLanguageEnum::FRENCH, pSemanticMemory, pLingDb);
  allowToInformTheUserHowToTeach(pSemanticMemory);

  TextProcessingContext textProcContext(SemanticAgentGrounding::me,
                                        SemanticAgentGrounding::userNotIdentified,
                                        SemanticLanguageEnum::ENGLISH);
  auto helloSemExp = converter::textToSemExp("hello", textProcContext, pLingDb);
  triggers::add(helloSemExp->clone(), helloSemExp->clone(), pSemanticMemory, pLingDb);
  auto byeSemExp = converter::textToSemExp("bye-bye", textProcContext, pLingDb);
  triggers::add(byeSemExp->clone(), byeSemExp->clone(), pSemanticMemory, pLingDb);
}


std::shared_ptr<ExpressionWithLinks> addFallback
(UniqueSemanticExpression pSemExp,
 SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb,
 const mystd::radix_map_str<std::string>* pLinkedInfosPtr)
{
  pSemanticMemory.memBloc.ensureFallbacksBlock();
  return _informMetaMemory(std::move(pSemExp), *pSemanticMemory.memBloc.getFallbacksBlockPtr(),
                           pSemanticMemory, nullptr, pLingDb, InformationType::FALLBACK,
                           pLinkedInfosPtr);
}



mystd::unique_propagate_const<UniqueSemanticExpression> resolveCommandFromMemBlock(
    const SemanticExpression& pSemExp,
    const SemanticMemoryBlock& pMemblock,
    const std::string& pCurrentUserId,
    const linguistics::LinguisticDatabase& pLingDb)
{
  UniqueSemanticExpression clonedSemExp = pSemExp.clone();
  conditionsAdder::addConditonsForSomeTimedGrdExp(clonedSemExp);

  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  controller::applyOperatorOnSemExpConstMem(
        compSemAnswers, *clonedSemExp,
        SemanticOperatorEnum::RESOLVECOMMAND, InformationType::INFORMATION,
        pMemblock, pCurrentUserId, nullptr, nullptr, nullptr, nullptr, pLingDb);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  if (compSemAnswers)
    controller::compAnswerToSemExp(res, *compSemAnswers);
  return res;
}


mystd::unique_propagate_const<UniqueSemanticExpression> resolveCommand(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  return resolveCommandFromMemBlock(pSemExp, pSemanticMemory.memBloc,
                                    pSemanticMemory.getCurrUserId(), pLingDb);
}



mystd::unique_propagate_const<UniqueSemanticExpression> executeBehaviorFromMemBlock(
    const SemanticExpression& pSemExp,
    const SemanticMemoryBlock& pMemblock,
    const std::string& pCurrentUserId,
    const linguistics::LinguisticDatabase& pLingDb)
{
  UniqueSemanticExpression clonedSemExp = pSemExp.clone();
  conditionsAdder::addConditonsForSomeTimedGrdExp(clonedSemExp);

  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  controller::applyOperatorOnSemExpConstMem(
        compSemAnswers, *clonedSemExp,
        SemanticOperatorEnum::EXECUTEBEHAVIOR, InformationType::INFORMATION,
        pMemblock, pCurrentUserId, nullptr, nullptr, nullptr, nullptr, pLingDb);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  if (compSemAnswers)
    controller::compAnswerToSemExp(res, *compSemAnswers);
  return res;
}


mystd::unique_propagate_const<UniqueSemanticExpression> executeBehavior(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  return executeBehaviorFromMemBlock(pSemExp, pSemanticMemory.memBloc,
                                    pSemanticMemory.getCurrUserId(), pLingDb);
}

mystd::unique_propagate_const<UniqueSemanticExpression> executeFromTrigger(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  UniqueSemanticExpression clonedSemExp = pSemExp.clone();
  conditionsAdder::addConditonsForSomeTimedGrdExp(clonedSemExp);

  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  controller::applyOperatorOnSemExpConstMem(
        compSemAnswers, *clonedSemExp,
        SemanticOperatorEnum::EXECUTEFROMTRIGGER, InformationType::INFORMATION,
        pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(),
        nullptr, nullptr, nullptr, nullptr, pLingDb);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;
  if (compSemAnswers)
    controller::compAnswerToSemExp(res, *compSemAnswers);
  return res;
}


mystd::unique_propagate_const<UniqueSemanticExpression> execute(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  UniqueSemanticExpression clonedSemExp = pSemExp.clone();
  conditionsAdder::addConditonsForSomeTimedGrdExp(clonedSemExp);
  mystd::unique_propagate_const<UniqueSemanticExpression> res;

  std::map<const SentenceWithLinks*, TruenessValue> axiomToConditionCurrentState;
  {
    std::unique_ptr<CompositeSemAnswer> compSemAnswers;
    controller::applyOperatorOnSemExpConstMem(
          compSemAnswers, *clonedSemExp,
          SemanticOperatorEnum::EXECUTEBEHAVIOR, InformationType::INFORMATION,
          pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(),
          nullptr, nullptr, nullptr, nullptr, pLingDb);
    if (compSemAnswers)
      controller::compAnswerToSemExp(res, *compSemAnswers);
    if (res)
      return res;
  }

  {
    std::unique_ptr<CompositeSemAnswer> compSemAnswers;
    controller::applyOperatorOnSemExpConstMem(
          compSemAnswers, *clonedSemExp,
          SemanticOperatorEnum::EXECUTEFROMTRIGGER, InformationType::INFORMATION,
          pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(),
          nullptr, nullptr, nullptr, nullptr, pLingDb);
    if (compSemAnswers)
      controller::compAnswerToSemExp(res, *compSemAnswers);
    return res;
  }
}


mystd::unique_propagate_const<UniqueSemanticExpression> externalRequester(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticMemoryBlockViewer memBloc(nullptr, pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId());
  return privateImplem::externalTeachingRequester(pSemExp, memBloc, pLingDb,
                                                  pSemanticMemory.getCurrUserId());
}


void pingTime(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
              SemanticMemory& pSemanticMemory,
              const SemanticDuration& pNowTimeDuration,
              const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  controller::notifyCurrentTime(compSemAnswers, pSemanticMemory, nullptr,
                                pNowTimeDuration, pLingDb);

  if (compSemAnswers)
  {
    utility::keepOnlyLastFeedback(*compSemAnswers);
    controller::compAnswerToSemExp(pReaction, *compSemAnswers);
  }
}



std::shared_ptr<ExpressionWithLinks> react(
    mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
    SemanticMemory& pSemanticMemory,
    UniqueSemanticExpression pSemExp,
    const linguistics::LinguisticDatabase& pLingDb,
    const ReactionOptions* pReactionOptions)
{
  converter::splitPossibilitiesOfQuestions(pSemExp, pLingDb);
  conditionsAdder::addConditonsForSomeTimedGrdExp(pSemExp);

  static const InformationType informationType = InformationType::INFORMATION;
  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  auto expForMem = pSemanticMemory.memBloc.addRootSemExp(std::move(pSemExp), pLingDb);
  ExpressionWithLinks& expForMemRef = *expForMem;
  controller::applyOperatorOnExpHandleInMemory(compSemAnswers, expForMemRef,
                                               SemanticOperatorEnum::REACT,
                                               informationType, pSemanticMemory, nullptr, pLingDb,
                                               pReactionOptions);

  if (compSemAnswers)
  {
    controller::linkConditionalReactions(compSemAnswers->semAnswers, expForMemRef,
                                         pSemanticMemory, pLingDb, informationType);
    utility::keepOnlyLastFeedback(*compSemAnswers);
    controller::compAnswerToSemExp(pReaction, *compSemAnswers);
  }
  return expForMem;
}


std::shared_ptr<ExpressionWithLinks> teach
(mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
 SemanticMemory& pSemanticMemory,
 UniqueSemanticExpression pSemExp,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticActionOperatorEnum pActionOperator)
{
  conditionsAdder::addConditonsForSomeTimedGrdExp(pSemExp);
  static const InformationType informationType = InformationType::INFORMATION;
  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  auto expForMem = pSemanticMemory.memBloc.addRootSemExp(std::move(pSemExp), pLingDb);
  ExpressionWithLinks& expForMemRef = *expForMem;
  SemanticOperatorEnum opEnum = SemanticOperatorEnum::TEACHINFORMATION;
  if (pActionOperator == SemanticActionOperatorEnum::BEHAVIOR)
    opEnum = SemanticOperatorEnum::TEACHBEHAVIOR;
  else if (pActionOperator == SemanticActionOperatorEnum::CONDITION)
    opEnum = SemanticOperatorEnum::TEACHCONDITION;
  controller::applyOperatorOnExpHandleInMemory(compSemAnswers, expForMemRef, opEnum,
                                                 informationType, pSemanticMemory, nullptr,
                                                 pLingDb, nullptr);

  if (compSemAnswers && !compSemAnswers->semAnswers.empty())
  {
    controller::linkConditionalReactions(compSemAnswers->semAnswers, expForMemRef,
                                         pSemanticMemory, pLingDb, informationType);
    controller::compAnswerToSemExp(pReaction, *compSemAnswers);
  }
  return expForMem;
}


void show(std::vector<std::unique_ptr<GroundedExpression>>& pAnswers,
          const SemanticExpression& pSemExp,
          const SemanticMemory& pSemanticMemory,
          const linguistics::LinguisticDatabase& pLingDb)
{
  std::unique_ptr<CompositeSemAnswer> compSemAnswers;
  controller::applyOperatorOnSemExpConstMem(compSemAnswers, pSemExp,
                                            SemanticOperatorEnum::SHOW,
                                            InformationType::INFORMATION,
                                            pSemanticMemory.memBloc, pSemanticMemory.getCurrUserId(),
                                            &pSemanticMemory.proativeSpecifications, pSemanticMemory.getExternalFallback(),
                                            &pSemanticMemory.callbackToSentencesCanBeAnswered, nullptr, pLingDb);

  if (compSemAnswers)
  {
    std::list<const GroundedExpression*> grdExpAnswers;
    CompositeSemAnswer::getGrdExps(grdExpAnswers, compSemAnswers->semAnswers);
    if (!grdExpAnswers.empty())
      _cloneGrdExpList(pAnswers, grdExpAnswers);
  }
}



} // End of namespace memoryOperation
} // End of namespace onsem
