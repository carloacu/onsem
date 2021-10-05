#include <onsem/texttosemantic/dbtype/semanticexpression/fixedsynthesisexpression.hpp>

namespace onsem
{


FixedSynthesisExpression::FixedSynthesisExpression
(UniqueSemanticExpression&& pSemExp)
  : SemanticExpression(SemanticExpressionType::FIXEDSYNTHESIS),
    langToSynthesis(),
    _semExp(std::move(pSemExp))
{
}


void FixedSynthesisExpression::assertEltsEqual(const FixedSynthesisExpression& pOther) const
{
  assert(langToSynthesis == pOther.langToSynthesis);
  _semExp->assertEqual(*pOther._semExp);
}


std::unique_ptr<FixedSynthesisExpression> FixedSynthesisExpression::clone
(const IndexToSubNameToParameterValue* pParams,
 bool pRemoveRecentContextInterpretations,
 const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const
{
  auto res = mystd::make_unique<FixedSynthesisExpression>
      (_semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
  res->langToSynthesis = langToSynthesis;
  return res;
}


} // End of namespace onsem
