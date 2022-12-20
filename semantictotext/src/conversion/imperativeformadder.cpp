#include "imperativeformadder.hpp"
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
namespace imperativeFormAdder
{


void addFormForMandatoryGrdExps(UniqueSemanticExpression& pSemExp,
                                const std::string& pUserId)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroundingType::STATEMENT)
    {
      const SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();
      const GroundedExpression* objectGrdExpPtr = SemExpGetter::getGrdExpToDo(grdExp, statGrd, pUserId);
      if (objectGrdExpPtr != nullptr)
      {
        pSemExp = SemExpCreator::getImperativeAssociateFrom(*objectGrdExpPtr);
        addFormForMandatoryGrdExps(pSemExp, pUserId);
      }
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      addFormForMandatoryGrdExps(currElt, pUserId);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    addFormForMandatoryGrdExps(pSemExp->getIntExp().interpretedExp, pUserId);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    addFormForMandatoryGrdExps(pSemExp->getFdkExp().concernedExp, pUserId);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    addFormForMandatoryGrdExps(pSemExp->getAnnExp().semExp, pUserId);
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
      addFormForMandatoryGrdExps(metadataExp.semExp, agentPtr->userId);
    else
      addFormForMandatoryGrdExps(metadataExp.semExp, pUserId);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp->getCondExp();
    addFormForMandatoryGrdExps(condExp.thenExp, pUserId);
    if (condExp.elseExp)
      addFormForMandatoryGrdExps(*condExp.elseExp, pUserId);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}




} // End of namespace imperativeFormAdder
} // End of namespace onsem
