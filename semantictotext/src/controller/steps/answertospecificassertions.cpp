#include "answertospecificassertions.hpp"
#include <set>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../semexpcontroller.hpp"
#include "../../operator/externalteachingrequester.hpp"


namespace onsem
{
namespace answerToSpecificAssertions
{

namespace
{

bool _processWantSentences(SemControllerWorkingStruct& pWorkStruct,
                           SemanticMemoryBlockViewer& pMemViewer,
                           const GroundedExpression& pGrdExp)
{
  if (pWorkStruct.author == nullptr ||
      !SemExpGetter::agentIsTheSubject(pGrdExp, pWorkStruct.author->userId))
    return false;

  auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
  if (itObject != pGrdExp.children.end())
  {
    auto objGrdExPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (objGrdExPtr == nullptr)
      return false;

    const SemanticStatementGrounding* objStatGrdPtr = objGrdExPtr->grounding().getStatementGroundingPtr();
    if (objStatGrdPtr == nullptr)
      return false;

    if (objStatGrdPtr->concepts.count("mentalState_know") != 0)
    {
      if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT)
        return false;
      if (objStatGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
        return false;

      auto itKnowObject = objGrdExPtr->children.find(GrammaticalType::OBJECT);
      if (itKnowObject != objGrdExPtr->children.end())
      {
        auto questionGrdExPtr = itKnowObject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (questionGrdExPtr == nullptr)
          return false;

        const GroundedExpression& questionGrdExp = *questionGrdExPtr;
        const SemanticStatementGrounding* questionStatGrdPtr = questionGrdExp->getStatementGroundingPtr();
        if (questionStatGrdPtr != nullptr)
        {
          auto& questionRequests = questionStatGrdPtr->requests;
          if (questionRequests.has(SemanticRequestType::NOTHING) ||
              questionRequests.has(SemanticRequestType::ACTION))
            return false;
          SemControllerWorkingStruct subWorkStruct(pWorkStruct);
          if (subWorkStruct.askForNewRecursion())
          {
            controller::manageQuestion(subWorkStruct, pMemViewer, questionRequests,
                                       questionGrdExp, {}, questionGrdExp);
            pWorkStruct.addAnswers(subWorkStruct);
            return true;
          }
        }
      }
    }
    else if (SemExpGetter::agentIsTheSubject(*objGrdExPtr, SemanticAgentGrounding::me))
    {
      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        subWorkStruct.comparisonExceptions.request = true;
        controller::manageAction(subWorkStruct, pMemViewer, *objStatGrdPtr, *objGrdExPtr, *objGrdExPtr);
        pWorkStruct.addAnswers(subWorkStruct);
        return true;
      }
    }
  }
  return false;
}


bool _processTeachSentences(SemControllerWorkingStruct& pWorkStruct,
                            const SemanticMemoryBlockViewer& pMemViewer,
                            const GroundedExpression& pGrdExp,
                            const SemanticStatementGrounding& pStatGrd)
{
  auto res = privateImplem::reactToAnExternalTeachingRequest(pMemViewer, pGrdExp, pStatGrd, pWorkStruct.author->userId, pWorkStruct.lingDb);
  if (!res)
    return false;
  pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::EXTERNALTEACHINGREQUEST, std::move(res));
  return true;
}

}



bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp)
{
  const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statGrdPtr == nullptr)
    return false;
  auto& statGrd = *statGrdPtr;
  if (!statGrd.polarity)
    return false;

  for (const auto& currCpt : statGrd.concepts)
  {
    if (currCpt.first == "verb_want")
      return _processWantSentences(pWorkStruct, pMemViewer, pGrdExp);
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT &&
        pWorkStruct.proativeSpecificationsPtr != nullptr &&
        pWorkStruct.proativeSpecificationsPtr->canLearnANewAxiomaticnAction &&
        currCpt.first == "verb_action_teach")
      return _processTeachSentences(pWorkStruct, pMemViewer, pGrdExp, *statGrdPtr);
  }

  if (statGrd.verbGoal == VerbGoalEnum::MANDATORY)
  {
      SemControllerWorkingStruct subWorkStruct(pWorkStruct);
      if (subWorkStruct.askForNewRecursion())
      {
        controller::manageAction(subWorkStruct, pMemViewer, statGrd, pGrdExp, pGrdExp);
        pWorkStruct.addAnswers(subWorkStruct);
        return true;
      }
  }

  return false;
}


} // End of namespace answerToSpecificAssertions
} // End of namespace onsem
