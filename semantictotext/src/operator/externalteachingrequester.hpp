#ifndef ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_EXTERNALTEACHINGREQUESTER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_EXTERNALTEACHINGREQUESTER_HPP

#include <memory>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>

namespace onsem
{
struct SemanticMemoryBlockViewer;
namespace linguistics
{
struct LinguisticDatabase;
}

namespace privateImplem
{

std::unique_ptr<SemanticExpression> reactToAnExternalTeachingRequest(const SemanticMemoryBlockViewer& pMemViewer,
                                                                      const GroundedExpression& pGrdExp,
                                                                      const SemanticStatementGrounding& pStatGrd,
                                                                      const std::string& pAuthorId,
                                                                      const linguistics::LinguisticDatabase& pLingDb);

mystd::unique_propagate_const<UniqueSemanticExpression> externalTeachingRequester(const SemanticExpression& pSemExp,
                                                                    const SemanticMemoryBlockViewer& pMemViewer,
                                                                    const linguistics::LinguisticDatabase& pLingDb,
                                                                    const std::string& pAuthorUserId);

} // End of namespace privateImplem
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_OPERATOR_EXTERNALTEACHINGREQUESTER_HPP
