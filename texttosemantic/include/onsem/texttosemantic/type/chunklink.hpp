#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_CHUNKLINK_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_CHUNKLINK_HPP

#include <memory>
#include <onsem/common/enum/chunklinktype.hpp>
#include "chunk.hpp"
#include "../api.hpp"

namespace onsem {
namespace linguistics {

/// A link to a chunk.
struct ONSEM_TEXTTOSEMANTIC_API ChunkLink {
    ChunkLink(const ChunkLinkType& pType, const Chunk& pChild);

    ChunkLink(const ChunkLinkType& pType, const std::shared_ptr<Chunk>& pChunk);

    ChunkLink(std::vector<Token>& pTokList,
              const ChunkLinkType& pType,
              const std::shared_ptr<Chunk>& pChunk,
              const TokIt pItBegin,
              const TokIt pItEnd);

    ChunkLink(const TokenRange& pTokRange, const ChunkLinkType& pType, const std::shared_ptr<Chunk>& pChunk);

    TokenRange tokRange;
    /// The type of the link.
    ChunkLinkType type;
    /// The chunk pointed by a link.
    std::shared_ptr<Chunk> chunk;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_CHUNKLINK_HPP
