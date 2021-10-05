#ifndef ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_COMPLETEWITHCONTEXT_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_COMPLETEWITHCONTEXT_HPP

#include <onsem/common/enum/grammaticaltype.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticExpression;
struct UniqueSemanticExpression;
struct SemanticMemoryBlock;


void completeWithContext(UniqueSemanticExpression& pSemExp,
                         GrammaticalType pGramParentLink,
                         const SemanticExpression& pContextSemExp,
                         bool pSameAuthor,
                         const SemanticExpression* pAuthorPtr,
                         const SemanticMemoryBlock& pMemBlock,
                         const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_INTERPRETATION_COMPLETEWITHCONTEXT_HPP
