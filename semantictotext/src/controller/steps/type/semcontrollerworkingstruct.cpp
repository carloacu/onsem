#include "semcontrollerworkingstruct.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>
#include "../../../type/referencesfiller.hpp"
#include "../../../type/semanticdetailledanswer.hpp"


namespace onsem
{


bool SemControllerWorkingStruct::askForNewRecursion()
{
  return --nbRecurssiveCallsRemaining >= 0;
}



void SemControllerWorkingStruct::getAnswersForRequest
(std::list<const AnswerExp*>& pAnswers,
 const SemanticRequests& pRequests) const
{
  for (const auto& currSubDetAnswers : compositeSemAnswers->semAnswers)
  {
    const LeafSemAnswer* leafPtr = currSubDetAnswers->getLeafPtr();
    if (leafPtr != nullptr)
    {
      for (const auto& currRequest : pRequests.types)
      {
        auto itForReq = leafPtr->answerElts.find(currRequest);
        if (itForReq != leafPtr->answerElts.end())
          for (const auto& currAnsw : itForReq->second.answersFromMemory)
            pAnswers.push_back(&currAnsw);
      }
    }
  }
}


void SemControllerWorkingStruct::addAnswerWithoutReferences(
    ContextualAnnotation pType,
    UniqueSemanticExpression pReaction)
{
  contAnnotationOfPreviousAnswers = pType;
  compositeSemAnswers->semAnswers.emplace_back
      (std::make_unique<LeafSemAnswer>(pType, std::move(pReaction)));
}

void SemControllerWorkingStruct::addQuestion(
    UniqueSemanticExpression pReaction)
{
  addAnswerWithoutReferences(ContextualAnnotation::QUESTION, std::move(pReaction));
}

void SemControllerWorkingStruct::addAnswer(
    ContextualAnnotation pType,
    UniqueSemanticExpression pReaction,
    const ReferencesFiller& pReferencesFiller)
{
  contAnnotationOfPreviousAnswers = pType;
  pReferencesFiller.addReferences(pReaction);
  compositeSemAnswers->semAnswers.emplace_back
      (std::make_unique<LeafSemAnswer>(pType, std::move(pReaction)));
}

void SemControllerWorkingStruct::addConditionForAUserAnswer(
    ContextualAnnotation pType,
    UniqueSemanticExpression pReaction,
    ConditionForAUser pConditionForAUser)
{
  contAnnotationOfPreviousAnswers = pType;
  compositeSemAnswers->semAnswers.emplace_back
      (std::make_unique<LeafSemAnswer>(pType, std::move(pReaction), std::move(pConditionForAUser)));
}

void SemControllerWorkingStruct::addConditionalAnswer(ContextualAnnotation pType,
                                                      UniqueSemanticExpression pReaction,
                                                      const mystd::optional<ConditionResult>& pCondition)
{
  contAnnotationOfPreviousAnswers = pType;
  compositeSemAnswers->semAnswers.emplace_back
      (std::make_unique<LeafSemAnswer>(pType, std::move(pReaction), pCondition));
}

void SemControllerWorkingStruct::addAnswers(
    SemControllerWorkingStruct& pOther)
{
  if (pOther.reactionOptions.reactWithTextAndResource)
  {
    while (pOther.compositeSemAnswers->semAnswers.size() > 1)
    {
      SemControllerWorkingStruct subOther(pOther);
      assert(subOther.compositeSemAnswers->semAnswers.empty());
      subOther.compositeSemAnswers->semAnswers.splice(subOther.compositeSemAnswers->semAnswers.end(),
                                                      pOther.compositeSemAnswers->semAnswers,
                                                      pOther.compositeSemAnswers->semAnswers.begin());
      if (_canBeANewAnswer(subOther))
        addAnswers(ListExpressionType::UNRELATED, subOther);
    }
    if (_canBeANewAnswer(pOther))
      addAnswers(ListExpressionType::UNRELATED, pOther);
  }
  else
    addAnswers(ListExpressionType::UNRELATED, pOther);
}


void SemControllerWorkingStruct::addAnswers(
    ListExpressionType pListExpType,
    SemControllerWorkingStruct& pOther)
{
  if (!pOther.haveAnAnswer())
    return;
  contAnnotationOfPreviousAnswers = pOther.contAnnotationOfPreviousAnswers;

  if (compositeSemAnswers->semAnswers.empty())
  {
    compositeSemAnswers = std::move(pOther.compositeSemAnswers);
    return;
  }

  if (compositeSemAnswers->listType == pListExpType &&
      pOther.compositeSemAnswers->listType == pListExpType)
  {
    compositeSemAnswers->semAnswers.splice(compositeSemAnswers->semAnswers.end(),
                                           pOther.compositeSemAnswers->semAnswers);
    return;
  }

  if (compositeSemAnswers->semAnswers.size() == 1 &&
      (pOther.compositeSemAnswers->semAnswers.size() <= 1 ||
       pOther.compositeSemAnswers->listType == pListExpType))
  {
    compositeSemAnswers->listType = pListExpType;
    compositeSemAnswers->semAnswers.splice(compositeSemAnswers->semAnswers.end(),
                                           pOther.compositeSemAnswers->semAnswers);
    return;
  }

  if (pOther.compositeSemAnswers->semAnswers.size() == 1 &&
      (compositeSemAnswers->semAnswers.size() <= 1 ||
       compositeSemAnswers->listType == pListExpType))
  {
    compositeSemAnswers->listType = pListExpType;
    compositeSemAnswers->semAnswers.splice(compositeSemAnswers->semAnswers.end(),
                                           pOther.compositeSemAnswers->semAnswers);
    return;
  }

  if (compositeSemAnswers->semAnswers.size() == 1)
  {
    compositeSemAnswers->listType = pListExpType;
    compositeSemAnswers->semAnswers.emplace_back
        (std::move(pOther.compositeSemAnswers));
    return;
  }

  if (pOther.compositeSemAnswers->semAnswers.size() == 1)
  {
    auto compSemExp = std::make_unique<CompositeSemAnswer>(pListExpType);
    compSemExp->semAnswers.emplace_back(std::move(compositeSemAnswers));
    compSemExp->semAnswers.splice(compSemExp->semAnswers.end(),
                                  pOther.compositeSemAnswers->semAnswers);
    compositeSemAnswers = std::move(compSemExp);
    return;
  }

  auto compSemExp = std::make_unique<CompositeSemAnswer>(pListExpType);
  compSemExp->semAnswers.emplace_back
      (std::move(compositeSemAnswers));
  compSemExp->semAnswers.emplace_back
      (std::move(pOther.compositeSemAnswers));
  compositeSemAnswers = std::move(compSemExp);
}


bool SemControllerWorkingStruct::haveAnAnswer() const
{
  return !compositeSemAnswers->semAnswers.empty();
}


bool SemControllerWorkingStruct::isFinished() const
{
  if (reactionOptions.reactWithTextAndResource)
  {
    bool hasTextualAnswer = false;
    bool hasResourceAnswer = false;
    std::list<const GroundedExpression*> grdExpAnswers;
    compositeSemAnswers->getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
    for (const auto& currGrdExpAnswer : grdExpAnswers)
    {
      if (SemExpGetter::isAResourceFromGrdExp(*currGrdExpAnswer))
      {
        if (hasTextualAnswer)
          return true;
        hasResourceAnswer = true;
      }
      else
      {
        if (hasResourceAnswer ||
            SemExpGetter::isGrdExpAQuestion(*currGrdExpAnswer))
          return true;
        hasTextualAnswer = true;
      }
    }
    return false;
  }
  return haveAnAnswer();
}


bool SemControllerWorkingStruct::canHaveAnotherTextualAnswer() const
{
  if (reactionOptions.reactWithTextAndResource)
  {
    std::list<const GroundedExpression*> grdExpAnswers;
    compositeSemAnswers->getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
    for (const auto& currGrdExpAnswer : grdExpAnswers)
      if (!SemExpGetter::isAResourceFromGrdExp(*currGrdExpAnswer))
        return false;
    return true;
  }
  return !haveAnAnswer();
}


bool SemControllerWorkingStruct::canBeANewAnswer(const SemanticExpression& pSemExp) const
{
  if (reactionOptions.reactWithTextAndResource)
  {
    const bool isInputAResource = SemExpGetter::isAResource(pSemExp);
    return _canBeANewAnswer(isInputAResource);
  }
  return true;
}

bool SemControllerWorkingStruct::_canBeANewAnswer(const SemControllerWorkingStruct& pOther) const
{
  if (reactionOptions.reactWithTextAndResource)
  {
    const bool isInputAResource = pOther._isAResource();
    return _canBeANewAnswer(isInputAResource);
  }
  return true;
}

bool SemControllerWorkingStruct::_isAResource() const
{
  std::list<const GroundedExpression*> grdExpAnswers;
  compositeSemAnswers->getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
  for (const auto& currGrdExpAnswer : grdExpAnswers)
    if (SemExpGetter::isAResourceFromGrdExp(*currGrdExpAnswer))
      return true;
  return false;
}

bool SemControllerWorkingStruct::_canBeANewAnswer(bool pIsInputAResource) const
{
  std::list<const GroundedExpression*> grdExpAnswers;
  compositeSemAnswers->getGrdExps(grdExpAnswers, compositeSemAnswers->semAnswers);
  for (const auto& currGrdExpAnswer : grdExpAnswers)
    if (SemExpGetter::isAResourceFromGrdExp(*currGrdExpAnswer) == pIsInputAResource)
      return false;
  return true;
}


void SemControllerWorkingStruct::getSourceContextAxiom(RelatedContextAxiom& pRes) const
{
  if (compositeSemAnswers)
    compositeSemAnswers->getSourceContextAxiom(pRes);
}

TruenessValue SemControllerWorkingStruct::agreementTypeOfTheAnswer()
{
  if (compositeSemAnswers)
    return compositeSemAnswers->getAgreementValue();
  return TruenessValue::UNKNOWN;
}

std::string SemControllerWorkingStruct::getAuthorUserId() const
{
  if (author != nullptr)
    return author->userId;
  return "";
}


SemControllerWorkingStruct::SemControllerWorkingStruct
(InformationType pInformationType,
 const SemanticExpression* pAuthorSemExpPtr,
 SemanticLanguageEnum pFromLanguage,
 ExpressionWithLinks* pMemKnowledge,
 SemanticOperatorEnum pReactOperator,
 const ProativeSpecifications* pProativeSpecificationsPtr,
 const ExternalFallback* pExternalFallbackPtr,
 const std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback> >* pCallbackToSentencesCanBeAnsweredPtr,
 std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr,
 const linguistics::LinguisticDatabase& pLingDb)
  : isAtRoot(true),
    informationType(pInformationType),
    authorSemExp(pAuthorSemExpPtr),
    originalSemExpPtr(nullptr),
    author(pAuthorSemExpPtr != nullptr ? SemExpGetter::extractAgentGrdPtr(*pAuthorSemExpPtr) : nullptr),
    fromLanguage(pFromLanguage),
    expHandleInMemory(pMemKnowledge),
    annotatedExps(),
    reactOperator(pReactOperator),
    typeOfFeedback(),
    proativeSpecificationsPtr(pProativeSpecificationsPtr),
    externalFallbackPtr(pExternalFallbackPtr),
    callbackToSentencesCanBeAnsweredPtr(pCallbackToSentencesCanBeAnsweredPtr),
    axiomToConditionCurrentStatePtr(pAxiomToConditionCurrentStatePtr),
    lingDb(pLingDb),
    comparisonExceptions(),
    nbRecurssiveCallsRemaining(10),
    contAnnotationOfPreviousAnswers(),
    reactionOptions(),
    compositeSemAnswers(std::make_unique<CompositeSemAnswer>(ListExpressionType::UNRELATED))
{
  comparisonExceptions.interpretations = true; // We do not consider the interpretations from InterpretationExpression
}



SemControllerWorkingStruct::SemControllerWorkingStruct
(const SemControllerWorkingStruct& pOther)
  : isAtRoot(false),
    informationType(pOther.informationType),
    authorSemExp(pOther.authorSemExp),
    originalSemExpPtr(nullptr),
    author(pOther.author),
    fromLanguage(pOther.fromLanguage),
    expHandleInMemory(pOther.expHandleInMemory),
    annotatedExps(pOther.annotatedExps),
    reactOperator(pOther.reactOperator),
    typeOfFeedback(pOther.typeOfFeedback),
    proativeSpecificationsPtr(pOther.proativeSpecificationsPtr),
    externalFallbackPtr(pOther.externalFallbackPtr),
    callbackToSentencesCanBeAnsweredPtr(pOther.callbackToSentencesCanBeAnsweredPtr),
    axiomToConditionCurrentStatePtr(pOther.axiomToConditionCurrentStatePtr),
    lingDb(pOther.lingDb),
    comparisonExceptions(pOther.comparisonExceptions),
    nbRecurssiveCallsRemaining(pOther.nbRecurssiveCallsRemaining),
    contAnnotationOfPreviousAnswers(pOther.contAnnotationOfPreviousAnswers),
    reactionOptions(pOther.reactionOptions),
    compositeSemAnswers(std::make_unique<CompositeSemAnswer>(ListExpressionType::UNRELATED))
{
}


} // End of namespace onsem

