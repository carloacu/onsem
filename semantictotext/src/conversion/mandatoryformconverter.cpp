#include "mandatoryformconverter.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativedurationgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include "simplesentencesplitter.hpp"
#include "../utility/semexpcreator.hpp"

namespace onsem
{
namespace mandatoryFormConverter
{


void process(UniqueSemanticExpression& pSemExp,
             const std::string& pUserId)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroundingType::STATEMENT)
    {
      SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();
      if (statGrd.requests.has(SemanticRequestType::ACTION))
      {
        statGrd.requests.erase(SemanticRequestType::ACTION);
        statGrd.verbGoal = VerbGoalEnum::MANDATORY;
      }
      else
      {
        const GroundedExpression* objectGrdExpPtr = SemExpGetter::getGrdExpToDo(grdExp, statGrd, pUserId);
        if (objectGrdExpPtr != nullptr)
          pSemExp = SemExpCreator::getMandatoryForm(*objectGrdExpPtr);
      }
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      process(currElt, pUserId);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    process(pSemExp->getIntExp().interpretedExp, pUserId);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    process(pSemExp->getFdkExp().concernedExp, pUserId);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    process(pSemExp->getAnnExp().semExp, pUserId);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pSemExp->getMetadataExp();
    const SemanticAgentGrounding* agentPtr = nullptr;
    auto* authorSemExpPtr = metadataExp.getAuthorSemExpPtr();
    if (authorSemExpPtr != nullptr)
      agentPtr = SemExpGetter::extractAgentGrdPtr(*authorSemExpPtr);
    if (agentPtr != nullptr)
      process(metadataExp.semExp, agentPtr->userId);
    else
      process(metadataExp.semExp, pUserId);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp->getCondExp();
    process(condExp.thenExp, pUserId);
    if (condExp.elseExp)
      process(*condExp.elseExp, pUserId);
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    auto& setOfFormsExp = pSemExp->getSetOfFormsExp();
    for (auto& currPrioToFrom : setOfFormsExp.prioToForms)
      for (auto& currFrom : currPrioToFrom.second)
        process(currFrom->exp, pUserId);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
    break;
  }
}


} // End of namespace mandatoryFormConverter
} // End of namespace onsem
