#include "sentenceslinks.hpp"
#include "semanticmemoryblockbinaryreader.hpp"


namespace onsem
{

template <bool IS_MODIFIABLE>
void RelationsThatMatch<IS_MODIFIABLE>::addMemSentStatic(const unsigned char* pStaticMemorySentencePtr,
                                                         const linguistics::LinguisticDatabase& pLingDb)
{
  auto elt = mystd::make_unique<AnswerElementStatic>(pStaticMemorySentencePtr, pLingDb);
  if (filterPtr == nullptr ||
      filterPtr->filterConditionStatic(*elt))
    res.staticLinks[SemanticMemoryBlockBinaryReader::memorySentenceToId(pStaticMemorySentencePtr)] =
        std::move(elt);
}


template struct RelationsThatMatch<true>;
template struct RelationsThatMatch<false>;

} // End of namespace onsem

