#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_QUESTIONWORDS_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_QUESTIONWORDS_HPP

#include <list>
#include <onsem/common/enum/semanticrequesttype.hpp>


namespace onsem
{
namespace linguistics
{
struct ChunkLinkWorkingZone;
struct SpecificLinguisticDatabase;
struct Chunk;
struct ChunkLink;

namespace questionWords
{

void extractQuestionWordsInsideAVerbGroup(std::list<ChunkLink>& pSyntTree,
                                          std::list<ChunkLink>::iterator pVerbChunkLink,
                                          const SpecificLinguisticDatabase& pSpecLingDb);

void addQuestionWords(ChunkLinkWorkingZone& pWorkingZone,
                      Chunk* pFirstVerbChunk,
                      Chunk* pSecondVerbChunk,
                      const SpecificLinguisticDatabase& pSpecLingDb);



} // End of namespace questionWords
} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_QUESTIONWORDS_HPP
