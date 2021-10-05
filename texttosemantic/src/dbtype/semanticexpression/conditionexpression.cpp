#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>

namespace onsem
{


ConditionExpression::ConditionExpression
(bool pIsAlwaysActive,
 bool pconditionPointsToAffirmations,
 UniqueSemanticExpression&& pConditionExp,
 UniqueSemanticExpression&& pThenExp)
  : SemanticExpression(SemanticExpressionType::CONDITION),
    isAlwaysActive(pIsAlwaysActive),
    conditionPointsToAffirmations(pconditionPointsToAffirmations),
    conditionExp(std::move(pConditionExp)),
    thenExp(std::move(pThenExp)),
    elseExp()
{
}


void ConditionExpression::assertEltsEqual(const ConditionExpression& pOther) const
{
  assert(isAlwaysActive == pOther.isAlwaysActive);
  assert(conditionPointsToAffirmations == pOther.conditionPointsToAffirmations);
  conditionExp->assertEqual(*pOther.conditionExp);
  thenExp->assertEqual(*pOther.thenExp);
  _assertSemExpOptsEqual(elseExp, pOther.elseExp);
}


} // End of namespace onsem
