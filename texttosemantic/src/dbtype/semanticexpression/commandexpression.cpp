#include <onsem/texttosemantic/dbtype/semanticexpression/commandexpression.hpp>

namespace onsem
{


CommandExpression::CommandExpression(UniqueSemanticExpression&& pSemExp)
  : SemanticExpression(SemanticExpressionType::COMMAND),
    semExp(std::move(pSemExp)),
    description()
{
}


bool CommandExpression::operator==(const CommandExpression& pOther) const
{
  return isEqual(pOther);
}

bool CommandExpression::isEqual(const CommandExpression& pOther) const
{
  return semExp == pOther.semExp &&
      description == pOther.description;
}

void CommandExpression::assertEltsEqual(const CommandExpression& pOther) const
{
  semExp->assertEqual(*pOther.semExp);
  _assertSemExpOptsEqual(description, pOther.description);
}

std::unique_ptr<CommandExpression> CommandExpression::clone
(const IndexToSubNameToParameterValue* pParams,
 bool pRemoveRecentContextInterpretations,
 const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const
{
  auto res = std::make_unique<CommandExpression>
      (semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
  if (description)
    res->description.emplace((*description)->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
  return res;
}

} // End of namespace onsem
