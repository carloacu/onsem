#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_MEMORYMODIFIER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_MEMORYMODIFIER_HPP

#include <onsem/common/enum/grammaticaltype.hpp>
#include "sentenceslinks.hpp"

namespace onsem
{


namespace MemoryModifier
{

template <bool IS_MODIFIABLE>
void semExpSetIntersectionInPlace
(SentenceLinks<IS_MODIFIABLE>& pSet1,
 const SentenceLinks<IS_MODIFIABLE>& pSet2,
 GrammaticalType pExceptThoseWhoDoesntHaveThisChild = GrammaticalType::UNKNOWN);


} // End of namespace MemoryModifier

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_MEMORYMODIFIER_HPP
