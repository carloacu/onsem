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
    auto knowGrdExPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (knowGrdExPtr == nullptr)
      return false;

    const SemanticStatementGrounding* knowStatGrdPtr = knowGrdExPtr->grounding().getStatementGroundingPtr();
    if (knowStatGrdPtr == nullptr ||
        knowStatGrdPtr->verbTense != SemanticVerbTense::UNKNOWN ||
        knowStatGrdPtr->concepts.count("mentalState_know") == 0)
      return false;

    auto itKnowObject = knowGrdExPtr->children.find(GrammaticalType::OBJECT);
    if (itKnowObject != knowGrdExPtr->children.end())
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

  for (const auto& currCpt : statGrdPtr->concepts)
  {
    if (currCpt.first == "verb_want")
      return _processWantSentences(pWorkStruct, pMemViewer, pGrdExp);
    if (pWorkStruct.proativeSpecificationsPtr != nullptr &&
        pWorkStruct.proativeSpecificationsPtr->canLearnANewAxiomaticnAction &&
        currCpt.first == "verb_action_teach")
      return _processTeachSentences(pWorkStruct, pMemViewer, pGrdExp, *statGrdPtr);
  }

  return false;
}


} // End of namespace answerToSpecificAssertions
} // End of namespace onsem
