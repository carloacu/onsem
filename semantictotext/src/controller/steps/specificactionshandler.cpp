#include "specificactionshandler.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/common/utility/random.hpp>
#include "../../type/referencesfiller.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../../utility/semexpcreator.hpp"
#include "../semexpcontroller.hpp"


namespace onsem
{
namespace specificActionsHandler
{

bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             const SemanticStatementGrounding& pGrdExpStatement)
{
  for (const auto& verbCpt : pGrdExpStatement.concepts)
  {
    if (verbCpt.first == "verb_action_show" || verbCpt.first == "verb_action_say")
    {
      auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
      if (itObject != pGrdExp.children.end())
      {
        const GroundedExpression* ojectGrdExp = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (ojectGrdExp != nullptr &&
            (verbCpt.first == "verb_action_show" ||
             SemExpGetter::getReferenceTypeFromGrd(ojectGrdExp->grounding()) == SemanticReferenceType::DEFINITE))
        {
          SemControllerWorkingStruct subWorkStruct(pWorkStruct);
          if (subWorkStruct.askForNewRecursion())
          {
            subWorkStruct.reactOperator = SemanticOperatorEnum::SHOW;
            controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, *ojectGrdExp, {},
                                              *ojectGrdExp);

            static const SemanticRequests objectRequest(SemanticRequestType::OBJECT);
            static const SemanticRequests subjectRequest(SemanticRequestType::SUBJECT);
            std::list<const AnswerExp*> answers;
            subWorkStruct.getAnswersForRequest(answers, objectRequest);
            subWorkStruct.getAnswersForRequest(answers, subjectRequest);
            if (answers.empty())
              return false;
            auto itAnswerToShow = Random::advanceIterator(answers);
            const AnswerExp& allAnsw_knowAndGrdExp = **itAnswerToShow;
            pWorkStruct.addAnswer
                (ContextualAnnotation::ANSWER, allAnsw_knowAndGrdExp.getGrdExp().clone(),
                 ReferencesFiller(allAnsw_knowAndGrdExp.relatedContextAxioms));
            return true;
          }
        }
      }
    }
    else if (verbCpt.first == "verb_action_remove")
    {
      auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
      if (itObject != pGrdExp.children.end())
      {
        const GroundedExpression* ojectGrdExpPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (ojectGrdExpPtr != nullptr)
        {
          const SemanticGenericGrounding* genericGrdPtr =
              ojectGrdExpPtr->grounding().getGenericGroundingPtr();
          if (genericGrdPtr != nullptr &&
              (genericGrdPtr->quantity.type == SemanticQuantityType::MAXNUMBER ||
               genericGrdPtr->quantity.type == SemanticQuantityType::EVERYTHING) &&
              genericGrdPtr->concepts.find("condition") != genericGrdPtr->concepts.end())
          {
            pWorkStruct.addAnswerWithoutReferences
                (ContextualAnnotation::REMOVEALLCONDITIONS,
                 SemExpCreator::okIRemoveAllConditions());
            return true;
          }
        }
      }
    }
  }
  return false;
}


void process_forShowOperator(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const GroundedExpression& pGrdExp,
                             const SemanticStatementGrounding& pGrdExpStatement)
{
  if (pGrdExpStatement.concepts.find("verb_action_show") != pGrdExpStatement.concepts.end())
  {
    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExp.children.end())
    {
      const GroundedExpression* ojectGrdExp = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
      if (ojectGrdExp != nullptr)
      {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        if (subWorkStruct.askForNewRecursion())
        {
          subWorkStruct.reactOperator = SemanticOperatorEnum::SHOW;
          controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, *ojectGrdExp, {},
                                            *ojectGrdExp);
          pWorkStruct.addAnswers(subWorkStruct);
        }
      }
    }
  }
}


} // End of namespace specificActionsHandler
} // End of namespace onsem
