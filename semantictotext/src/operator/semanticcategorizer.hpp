#ifndef ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_SEMANTICCATEGORIZER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_SEMANTICCATEGORIZER_HPP

#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>

namespace onsem
{
struct SemanticExpression;

namespace privateImplem
{

SemanticExpressionCategory categorizeGrdExp(const GroundedExpression& pGrdExp,
                                            const std::string& pAuthorUserId);

SemanticExpressionCategory categorize(const SemanticExpression& pSemExp);

} // End of namespace privateImplem
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_SEMANTICCATEGORIZER_HPP
