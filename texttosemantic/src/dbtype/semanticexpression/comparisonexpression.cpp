#include <onsem/texttosemantic/dbtype/semanticexpression/comparisonexpression.hpp>

namespace onsem
{


ComparisonExpression::ComparisonExpression
(ComparisonOperator pComparisonOperator,
 UniqueSemanticExpression&& pLeftOperandExp)
  : SemanticExpression(SemanticExpressionType::COMPARISON),
    op(pComparisonOperator),
    tense(SemanticVerbTense::PRESENT),
    request(SemanticRequestType::NOTHING),
    whatIsComparedExp(),
    leftOperandExp(std::move(pLeftOperandExp)),
    rightOperandExp()
{
}


void ComparisonExpression::assertEltsEqual(const ComparisonExpression& pOther) const
{
  assert(op == pOther.op);
  assert(tense == pOther.tense);
  assert(request == pOther.request);
  _assertSemExpOptsEqual(whatIsComparedExp, pOther.whatIsComparedExp);
  leftOperandExp->assertEqual(*pOther.leftOperandExp);
  _assertSemExpOptsEqual(rightOperandExp, pOther.rightOperandExp);
}


} // End of namespace onsem
