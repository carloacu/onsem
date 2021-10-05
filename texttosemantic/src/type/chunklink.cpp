#include <onsem/texttosemantic/type/chunklink.hpp>


namespace onsem
{
namespace linguistics
{


ChunkLink::ChunkLink
(const ChunkLinkType& pType,
 const Chunk& pChild)
  : tokRange(pChild.tokRange.getTokList()),
    type(pType),
    chunk(std::make_shared<Chunk>(pChild))
{
}


ChunkLink::ChunkLink
(const ChunkLinkType& pType,
 const std::shared_ptr<Chunk>& pChunk)
  : tokRange(pChunk->tokRange.getTokList()),
    type(pType),
    chunk(pChunk)
{
}


ChunkLink::ChunkLink
(std::vector<Token>& pTokList,
 const ChunkLinkType& pType,
 const std::shared_ptr<Chunk>& pChunk,
 const TokIt pItBegin,
 const TokIt pItEnd)
  : tokRange(pTokList, pItBegin, pItEnd),
    type(pType),
    chunk(pChunk)
{
}


ChunkLink::ChunkLink
(const TokenRange& pTokRange,
 const ChunkLinkType& pType,
 const std::shared_ptr<Chunk>& pChunk)
  : tokRange(pTokRange),
    type(pType),
    chunk(pChunk)
{
}


} // End of namespace linguistics
} // End of namespace onsem
