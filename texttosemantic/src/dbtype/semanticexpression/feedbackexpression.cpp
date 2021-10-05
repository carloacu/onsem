#include <onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp>

namespace onsem
{


FeedbackExpression::FeedbackExpression
(UniqueSemanticExpression&& pFeedbackExp,
 UniqueSemanticExpression&& pConcernedExp)
  : SemanticExpression(SemanticExpressionType::FEEDBACK),
    feedbackExp(std::move(pFeedbackExp)),
    concernedExp(std::move(pConcernedExp))
{
}

void FeedbackExpression::assertEltsEqual(const FeedbackExpression& pOther) const
{
  feedbackExp->assertEqual(*pOther.feedbackExp);
  concernedExp->assertEqual(*pOther.concernedExp);
}


} // End of namespace onsem
