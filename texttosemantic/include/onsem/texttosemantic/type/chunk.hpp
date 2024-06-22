#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_CHUNK_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_CHUNK_HPP

#include <list>
#include <vector>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "enum/lingverbform.hpp"
#include "enum/chunktype.hpp"
#include "../api.hpp"

namespace onsem {
namespace linguistics {
struct ChunkLink;

/**
 * @brief A chunk of the syntactic graph.
 * (chunk = node of the syntactic graph = words group)
 */
struct ONSEM_TEXTTOSEMANTIC_API Chunk {
    Chunk(const TokenRange& pTokRange, ChunkType pType);

    const SemanticWord& getHeadWord() const;
    PartOfSpeech getHeadPartOfSpeech() const;
    const WordAssociatedInfos& getHeadAssInfos() const;
    const std::map<std::string, char>& getHeadConcepts() const;

    TokenRange getTokRangeWrappingChildren() const;

    TokenRange tokRange;
    /**
     * @brief The head of the chunk.
     * This the word of the chunk that will be used to check compatibilities
     * between chunks.
     */
    TokIt head;
    bool hasOnlyOneReference;
    /// If the chunk has a positive form.
    bool positive;
    LingVerbForm form;
    SemanticRequests requests;
    bool requestCanBeObject;
    mystd::optional<TokenPos> requestWordTokenPosOpt;
    bool isPassive;
    /// The type of the chunk.
    ChunkType type;
    /// The sub-chunks.
    std::list<ChunkLink> children;
    mystd::optional<const SemanticWord*> introductingWordToSaveForSynthesis;

private:
    void _increaseTokRangeWrappingChildren(TokenRange& pTokenRange) const;
};

struct ONSEM_TEXTTOSEMANTIC_API ChunkAndTokIt {
    ChunkAndTokIt(Chunk& pChunk)
        : chunk(pChunk)
        , tokIt(pChunk.tokRange.getItBegin()) {}

    TokIt getItBegin() const { return chunk.tokRange.getItBegin(); }
    TokIt getItEnd() const { return chunk.tokRange.getItEnd(); }
    bool atBegin() const { return tokIt == chunk.tokRange.getItBegin(); }
    bool atEnd() const { return tokIt == chunk.tokRange.getItEnd(); }

    Chunk& chunk;
    TokIt tokIt;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_CHUNK_HPP
