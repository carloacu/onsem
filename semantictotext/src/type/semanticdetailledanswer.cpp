#include "semanticdetailledanswer.hpp"
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>

namespace onsem
{

ConditionForAUser::ConditionForAUser(const std::string& pUser,
                                     std::unique_ptr<SemanticExpression> pThenSemExp)
  : user(pUser),
    thenSemExp(std::move(pThenSemExp))
{
}

ConditionForAUser::ConditionForAUser(ConditionForAUser&& pOther)
  : user(std::move(pOther.user)),
    thenSemExp(std::move(pOther.thenSemExp))
{
}

ConditionForAUser& ConditionForAUser::operator=(ConditionForAUser&& pOther)
{
  user = std::move(pOther.user);
  thenSemExp = std::move(pOther.thenSemExp);
  return *this;
}


void AllAnswerElts::getReferences(std::list<std::string>& pReferences) const
{
  for (const auto& currAnswer : answersFromMemory)
    currAnswer.relatedContextAxioms.getReferences(pReferences);
  for (const auto& currAnswer : answersGenerated)
    currAnswer.relatedContextAxioms.getReferences(pReferences);
}


LeafSemAnswer::LeafSemAnswer(ContextualAnnotation pType)
  : SemAnswer(),
    type(pType),
    reaction(),
    answerElts(),
    condition(),
    conditionForAUser(),
    interactionContextContainer()
{
}

LeafSemAnswer::LeafSemAnswer(ContextualAnnotation pType,
                             UniqueSemanticExpression pReaction)
  : SemAnswer(),
    type(pType),
    reaction(std::move(pReaction)),
    answerElts(),
    condition(),
    conditionForAUser(),
    interactionContextContainer()
{
}

LeafSemAnswer::LeafSemAnswer(ContextualAnnotation pType,
                             UniqueSemanticExpression pReaction,
                             const mystd::optional<ConditionResult>& pCondition)
  : SemAnswer(),
    type(pType),
    reaction(std::move(pReaction)),
    answerElts(),
    condition(pCondition),
    conditionForAUser(),
    interactionContextContainer()
{
}

LeafSemAnswer::LeafSemAnswer(ContextualAnnotation pType,
                             UniqueSemanticExpression pReaction,
                             ConditionForAUser pConditionForAUser)
  : SemAnswer(),
    type(pType),
    reaction(std::move(pReaction)),
    answerElts(),
    condition(),
    conditionForAUser(std::move(pConditionForAUser)),
    interactionContextContainer()
{
}



void LeafSemAnswer::getGroundedExps(std::list<const GroundedExpression*>& pGrdExps) const
{
  for (const auto& currAnsForARequ : answerElts)
  {
    const AllAnswerElts& answElts = currAnsForARequ.second;
    for (const auto& currKnoAndGrdExp : answElts.answersFromMemory)
      pGrdExps.push_back(&currKnoAndGrdExp.getGrdExp());
    for (const auto& currKnowAns : answElts.answersGenerated)
      currKnowAns.genSemExp->getGrdExpPtrs_SkipWrapperLists(pGrdExps);
  }

  if (reaction)
    (*reaction)->getGrdExpPtrs_SkipWrapperLists(pGrdExps);
}


void LeafSemAnswer::getSourceContextAxiom(RelatedContextAxiom& pRes) const
{
  for (const auto& currElt : answerElts)
  {
    const AllAnswerElts& answElts = currElt.second;
    for (const AnswerExp& currAnswExp : answElts.answersFromMemory)
      pRes.add(currAnswExp.relatedContextAxioms);
    for (const auto& currElt : answElts.answersGenerated)
      pRes.add(currElt.relatedContextAxioms);
  }
}

bool LeafSemAnswer::containsOnlyResourcesOrTexts() const
{
  for (const auto& currElt : answerElts)
  {
    const AllAnswerElts& answElts = currElt.second;
    for (const AnswerExp& currAnswExp : answElts.answersFromMemory)
      if (!SemExpGetter::isAResourceOrATextFromGrdExp(currAnswExp.getGrdExp()))
        return false;
    for (const auto& currElt : answElts.answersGenerated)
      if (!SemExpGetter::isAResourceOrAText(*currElt.genSemExp))
        return false;
  }
  return true;
}



bool LeafSemAnswer::isEmpty() const
{
  return answerElts.empty();
}


CompositeSemAnswer::CompositeSemAnswer(ListExpressionType pListType)
  : SemAnswer(),
    listType(pListType),
    semAnswers()
{
}


void CompositeSemAnswer::getGrdExps
(std::list<const GroundedExpression*>& pGrdExps,
 const std::list<std::unique_ptr<SemAnswer>>& pSemEnswers)
{
  for (const auto& currDetAns : pSemEnswers)
  {
    const LeafSemAnswer* leafPtr = currDetAns->getLeafPtr();
    if (leafPtr != nullptr)
      leafPtr->getGroundedExps(pGrdExps);
  }
}

std::unique_ptr<UniqueSemanticExpression> CompositeSemAnswer::convertToSemExp() const
{
  std::list<const GroundedExpression*> grdExpAnswers;
  getGrdExps(grdExpAnswers, semAnswers);
  if (!grdExpAnswers.empty())
  {
    auto itGrdExp = grdExpAnswers.begin();
    UniqueSemanticExpression res = (*itGrdExp)->clone();
    if (itGrdExp != grdExpAnswers.end())
    {
      ++itGrdExp;
      while (itGrdExp != grdExpAnswers.end())
      {
        SemExpModifier::addNewSemExp(res, (*itGrdExp)->clone());
        ++itGrdExp;
      }
    }
    return std::make_unique<UniqueSemanticExpression>(std::move(res));
  }
  return std::unique_ptr<UniqueSemanticExpression>();
}


void CompositeSemAnswer::getSourceContextAxiom(RelatedContextAxiom& pRes) const
{
  for (const auto& currElt : semAnswers)
    currElt->getSourceContextAxiom(pRes);
}

bool CompositeSemAnswer::containsOnlyResourcesOrTexts() const
{
  for (const auto& currElt : semAnswers)
    if (!currElt->containsOnlyResourcesOrTexts())
      return false;
  return true;
}

TruenessValue CompositeSemAnswer::getAgreementValue() const
{
  if (semAnswers.empty())
    return TruenessValue::UNKNOWN;
  const LeafSemAnswer* leafPtr = semAnswers.front()->getLeafPtr();
  if (leafPtr == nullptr)
    return TruenessValue::UNKNOWN;
  const auto& leafAnsw = *leafPtr;

  for (const auto& currAnswer : leafAnsw.answerElts)
  {
    const AllAnswerElts& answElts = currAnswer.second;
    if (!answElts.answersGenerated.empty())
      return semExpAgreementDetector::semExpToAgreementValue
          (*answElts.answersGenerated.front().genSemExp);
  }

  if (leafAnsw.reaction)
    return semExpAgreementDetector::semExpToAgreementValue(leafAnsw.reaction->getSemExp());
  return TruenessValue::UNKNOWN;
}

void CompositeSemAnswer::keepOnlyTheResourcesOrTexts()
{
  for (auto it = semAnswers.begin(); it != semAnswers.end(); )
  {
    if (!(*it)->containsOnlyResourcesOrTexts())
      it = semAnswers.erase(it);
    else
      ++it;
  }
}

std::unique_ptr<InteractionContextContainer> CompositeSemAnswer::getInteractionContextContainer()
{
  for (auto& currSemAswer : semAnswers)
  {
    auto* leafPtr = currSemAswer->getLeafPtr();
    if (leafPtr != nullptr && leafPtr->interactionContextContainer)
    {
      auto res = std::move(leafPtr->interactionContextContainer);
      leafPtr->interactionContextContainer.reset();
      return res;
    }
  }
  return {};
}



} // End of namespace onsem

