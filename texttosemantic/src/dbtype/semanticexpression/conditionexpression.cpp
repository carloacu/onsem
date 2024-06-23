#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>

namespace onsem {

ConditionExpression::ConditionExpression(bool pIsAlwaysActive,
                                         bool pconditionPointsToAffirmations,
                                         UniqueSemanticExpression&& pConditionExp,
                                         UniqueSemanticExpression&& pThenExp)
    : SemanticExpression(SemanticExpressionType::CONDITION)
    , isAlwaysActive(pIsAlwaysActive)
    , conditionPointsToAffirmations(pconditionPointsToAffirmations)
    , conditionExp(std::move(pConditionExp))
    , conditionStr()
    , thenExp(std::move(pThenExp))
    , thenStr()
    , elseExp()
    , elseStr() {}


void ConditionExpression::assertEltsEqual(const ConditionExpression& pOther) const {
    assert(isAlwaysActive == pOther.isAlwaysActive);
    assert(conditionPointsToAffirmations == pOther.conditionPointsToAffirmations);
    conditionExp->assertEqual(*pOther.conditionExp);
    assert(conditionStr == pOther.conditionStr);
    thenExp->assertEqual(*pOther.thenExp);
    assert(thenStr == pOther.thenStr);
    _assertSemExpOptsEqual(elseExp, pOther.elseExp);
    assert(elseStr == pOther.elseStr);
}

}    // End of namespace onsem
