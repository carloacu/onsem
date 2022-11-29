#include <onsem/semantictotext/triggers.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "controller/semexpcontroller.hpp"
#include "conversion/conditionsadder.hpp"
#include "utility/utility.hpp"


namespace onsem
{
namespace triggers
{

void add(UniqueSemanticExpression pTriggerSemExp,
         UniqueSemanticExpression pAnswerSemExp,
         SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb)
{
  conditionsAdder::addConditonsForSomeTimedGrdExp(pTriggerSemExp);

  memoryOperation::resolveAgentAccordingToTheContext(pTriggerSemExp, pSemanticMemory, pLingDb);
  converter::splitEquivalentQuestions(pTriggerSemExp, pLingDb);
  auto expForMem = pSemanticMemory.memBloc.addRootSemExp(std::move(pTriggerSemExp), pLingDb);
  expForMem->outputToAnswerIfTriggerHasMatched.emplace(std::move(pAnswerSemExp));
  expForMem->addTriggerLinks(InformationType::ASSERTION, *expForMem->semExp, pLingDb);
}


std::shared_ptr<ExpressionWithLinks> match(
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
                                               SemanticOperatorEnum::REACTFROMTRIGGER,
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



} // End of namespace triggers
} // End of namespace onsem
