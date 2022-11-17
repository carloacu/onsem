#include "sentenceslinks.hpp"
#include "semanticmemoryblockbinaryreader.hpp"


namespace onsem
{

template <bool IS_MODIFIABLE>
void RelationsThatMatch<IS_MODIFIABLE>::addMemSentStatic(const unsigned char* pStaticMemorySentencePtr,
                                                         const linguistics::LinguisticDatabase& pLingDb)
{
  auto elt = std::make_unique<AnswerElementStatic>(pStaticMemorySentencePtr, pLingDb);
  if (filterPtr == nullptr ||
      filterPtr->filterConditionStatic(*elt))
    res.staticLinks[SemanticMemoryBlockBinaryReader::memorySentenceToId(pStaticMemorySentencePtr)] =
        std::move(elt);
}


template struct RelationsThatMatch<true>;
template struct RelationsThatMatch<false>;


template <bool IS_MODIFIABLE>
MemoryLinksAccessor<IS_MODIFIABLE> RequestToMemoryLinks<IS_MODIFIABLE>::getMemoryLinksAccessors(SemanticRequestType pRequest)
{
  auto itGramToSemExp = d.find(pRequest);
  auto* linksForARequestPtr = itGramToSemExp != d.end() ? &itGramToSemExp->second : nullptr;
  return {linksForARequestPtr, SemanticMemoryBlockBinaryReader::getLinksForARequest(c, pRequest)};
}



MemoryLinksAccessor<false> RequestToMemoryLinks<false>::getMemoryLinksAccessors(SemanticRequestType pRequest)
{
  auto itGramToSemExp = d.find(pRequest);
  auto* linksForARequestPtr = itGramToSemExp != d.end() ? &itGramToSemExp->second : nullptr;
  return {linksForARequestPtr, SemanticMemoryBlockBinaryReader::getLinksForARequest(c, pRequest)};
}

template struct RequestToMemoryLinks<true>;




} // End of namespace onsem

