#include "managecondition.hpp"
#include <onsem/common/utility/optional.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../semexpcontroller.hpp"
#include "../../utility/semexpcreator.hpp"
#include "semanticmemorylinker.hpp"

namespace onsem
{


bool _applyConditionAccordingToHisTrueness(SemControllerWorkingStruct& pWorkStruct,
                                           SemanticMemoryBlockViewer& pMemViewer,
                                           const ConditionSpecification& pCondSpec,
                                           TruenessValue pTruenessOfCondition)
{
  switch (pTruenessOfCondition)
  {
  case TruenessValue::VAL_TRUE:
  {
    controller::applyOperatorOnSemExp(pWorkStruct, pMemViewer, pCondSpec.thenExp);
    return true;
  }
  case TruenessValue::VAL_FALSE:
  {
    if (pCondSpec.elseExpPtr != nullptr)
    {
      controller::applyOperatorOnSemExp(pWorkStruct, pMemViewer, *pCondSpec.elseExpPtr);
      return true;
    }
    return false;
  }
  case TruenessValue::UNKNOWN:
    return false;
  }
  assert(false);
  return false;
}


void manageCondition(SemControllerWorkingStruct& pWorkStruct,
                     SemanticMemoryBlockViewer& pMemViewer,
                     const ConditionExpression& pCondExp)
{
  const SemanticExpression* elseExpPtr = nullptr;
  if (pCondExp.elseExp)
    elseExpPtr = &**pCondExp.elseExp;
  ConditionSpecification condSpec(pCondExp.isAlwaysActive,
                                  pCondExp.conditionPointsToAffirmations,
                                  *pCondExp.conditionExp,
                                  *pCondExp.thenExp,
                                  elseExpPtr);

  if (pWorkStruct.reactOperator == SemanticOperatorEnum::RESOLVECOMMAND)
  {
    auto thenCommand = memoryOperation::resolveCommandFromMemBlock(condSpec.thenExp,
                                                                   pMemViewer.constView,
                                                                   pMemViewer.currentUserId,
                                                                   pWorkStruct.lingDb);
    if (thenCommand)
      pWorkStruct.addAnswerWithoutReferences
          (ContextualAnnotation::BEHAVIOR,
           std::make_unique<ConditionExpression>
           (condSpec.isAlwaysActive, condSpec.conditionShouldBeInformed,
            condSpec.conditionExp.clone(), std::move(*thenCommand)));
  }
  else if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
           pWorkStruct.reactOperator == SemanticOperatorEnum::REACTFROMTRIGGER ||
           pWorkStruct.reactOperator == SemanticOperatorEnum::INFORM ||
           pWorkStruct.reactOperator == SemanticOperatorEnum::TEACHCONDITION)
  {
    // don't even try to learn a condition if the knowledge come from us
    if (pWorkStruct.expHandleInMemory == nullptr)
      return;
    {
      const SemanticAgentGrounding* authorPtr = SemExpGetter::extractAuthor(*pWorkStruct.expHandleInMemory->semExp);
      if (authorPtr != nullptr &&
          authorPtr->userId == SemanticAgentGrounding::me)
        return;
    }

    if (pWorkStruct.reactionOptions.canAnswerWithATrigger &&
        (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT ||
         pWorkStruct.reactOperator == SemanticOperatorEnum::REACTFROMTRIGGER) &&
        semanticMemoryLinker::addTriggerCondExp(pWorkStruct, pMemViewer, pCondExp))
      return;

    if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACTFROMTRIGGER)
      return;

    TruenessValue truenessOfCondition = TruenessValue::UNKNOWN;
    SemanticExpressionCategory thenCategory = memoryOperation::categorize(condSpec.thenExp);
    SemanticExpressionCategory elseCategory = condSpec.elseExpPtr != nullptr ?
        memoryOperation::categorize(*condSpec.elseExpPtr) : SemanticExpressionCategory::NOMINALGROUP;
    mystd::optional<bool> thenCommandCanBeDone;
    mystd::optional<bool> elseCommandCanBeDone;

    if (thenCategory == SemanticExpressionCategory::COMMAND)
    {
      auto* condGrdExpPtr = condSpec.conditionExp.getGrdExpPtr_SkipWrapperPtrs();
      if (condGrdExpPtr != nullptr)
      {
        auto* timeGrdPtr = condGrdExpPtr->grounding().getTimeGroundingPtr();
        if (timeGrdPtr != nullptr)
        {
          const auto& nowTime = SemanticTimeGrounding::now();
          if (timeGrdPtr != nullptr &&
              *timeGrdPtr <= nowTime &&
              timeGrdPtr->isEqualMoreOrLess10Seconds(nowTime))
          {
            controller::applyOperatorOnSemExp(pWorkStruct, pMemViewer, condSpec.thenExp);
            return;
          }
        }
      }

      truenessOfCondition = controller::operator_check_semExp(condSpec.conditionExp,
                                                              pMemViewer.constView,
                                                              pMemViewer.currentUserId,
                                                              pWorkStruct.lingDb);

      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        subWorkStruct.reactOperator = SemanticOperatorEnum::RESOLVECOMMAND;
        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, condSpec.thenExp);
        thenCommandCanBeDone.emplace
            (controller::compAnswerToContextualAnnotation(*subWorkStruct.compositeSemAnswers) == ContextualAnnotation::BEHAVIOR);
      }
    }

    if ((!thenCommandCanBeDone || *thenCommandCanBeDone) &&
        condSpec.elseExpPtr != nullptr &&
        elseCategory == SemanticExpressionCategory::COMMAND)
    {
      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        subWorkStruct.reactOperator = SemanticOperatorEnum::RESOLVECOMMAND;
        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *condSpec.elseExpPtr);
        elseCommandCanBeDone.emplace
            (controller::compAnswerToContextualAnnotation(*subWorkStruct.compositeSemAnswers) == ContextualAnnotation::BEHAVIOR);
      }
    }

    if (thenCommandCanBeDone || elseCommandCanBeDone)
    {
      if (thenCommandCanBeDone && !*thenCommandCanBeDone)
        controller::applyOperatorOnSemExp(pWorkStruct, pMemViewer, condSpec.thenExp);
      else if (elseCommandCanBeDone && !*elseCommandCanBeDone)
        controller::applyOperatorOnSemExp(pWorkStruct, pMemViewer, *condSpec.elseExpPtr);
      else
      {
        mystd::optional<ConditionResult> optCondRes;
        optCondRes.emplace(ConditionResult(condSpec, thenCategory, elseCategory));
        pWorkStruct.addConditionalAnswer
            (ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE,
             SemExpCreator::sayWeWillDoIt(condSpec), optCondRes);
      }
    }
    else if (thenCategory == SemanticExpressionCategory::AFFIRMATION)
    {
      mystd::optional<ConditionResult> optCondRes;
      optCondRes.emplace(ConditionResult(condSpec, thenCategory, elseCategory));
      if (condSpec.conditionShouldBeInformed)
      {
        pWorkStruct.addConditionalAnswer
            (ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE,
             SemExpCreator::sayOk(), optCondRes);
      }
      else
      {
        pWorkStruct.addConditionalAnswer
            (ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE,
             SemExpCreator::confirmInfoCondition(condSpec), optCondRes);
      }
    }

    _applyConditionAccordingToHisTrueness(pWorkStruct, pMemViewer, condSpec, truenessOfCondition);
  }
}



} // End of namespace onsem
