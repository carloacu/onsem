#include "answeridontknow.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include "../utility/semexpcreator.hpp"


namespace onsem
{
namespace privateImplem
{
namespace
{
mystd::unique_propagate_const<UniqueSemanticExpression> _answerIDontKnowFromGrdExp(const GroundedExpression& pGrdExp,
                                                                     bool pForQuestions,
                                                                     bool pForActions)
{
  const auto* statementGrdPtr = pGrdExp->getStatementGroundingPtr();
  if (statementGrdPtr != nullptr)
  {
    const auto& statementGrd = *statementGrdPtr;
    SemanticRequestType requestType = statementGrd.requests.firstOrNothing();
    if (requestType != SemanticRequestType::NOTHING)
    {
      if (requestType == SemanticRequestType::ACTION)
      {
        if (pForActions)
          return mystd::unique_propagate_const<UniqueSemanticExpression>(SemExpCreator::sayThatTheRobotCannotDoIt(pGrdExp));
      }
      else if (pForQuestions)
        return mystd::unique_propagate_const<UniqueSemanticExpression>(SemExpCreator::sayThatWeDontKnowTheAnswer(pGrdExp));
    }
  }
  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}
}


mystd::unique_propagate_const<UniqueSemanticExpression> answerIDontKnow(const SemanticExpression& pSemExp,
                                                          bool pForQuestions,
                                                          bool pForActions)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
    return _answerIDontKnowFromGrdExp(pSemExp.getGrdExp(), pForQuestions, pForActions);
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    auto resListExp = mystd::make_unique<ListExpression>();
    for (const auto& currElt : listExp.elts)
    {
      auto subRes = answerIDontKnow(*currElt, pForQuestions, pForActions);
      if (subRes)
        resListExp->elts.emplace_back(std::move(*subRes));
    }
    if (resListExp->elts.size() == 1)
      return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(resListExp->elts.front()));
    else if (resListExp->elts.size() > 1)
      return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(resListExp));
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    const SetOfFormsExpression& setOfFormsExp = pSemExp.getSetOfFormsExp();
    const GroundedExpression* originalGrdExpForm = SemExpGetter::getOriginalGrdExpForm(setOfFormsExp);
    if (originalGrdExpForm != nullptr)
      return _answerIDontKnowFromGrdExp(*originalGrdExpForm, pForQuestions, pForActions);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
    return answerIDontKnow(*pSemExp.getIntExp().interpretedExp, pForQuestions, pForActions);
  case SemanticExpressionType::FEEDBACK:
    return answerIDontKnow(*pSemExp.getFdkExp().concernedExp, pForQuestions, pForActions);
  case SemanticExpressionType::ANNOTATED:
    return answerIDontKnow(*pSemExp.getAnnExp().semExp, pForQuestions, pForActions);
  case SemanticExpressionType::METADATA:
    return answerIDontKnow(*pSemExp.getMetadataExp().semExp, pForQuestions, pForActions);
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return answerIDontKnow(pSemExp.getFSynthExp().getSemExp(), pForQuestions, pForActions);
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
    break;
  }

  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}


} // End of namespace privateImplem
} // End of namespace onsem
