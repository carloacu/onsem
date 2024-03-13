#include "extractsubchunkinsidelinkedtokens.hpp"
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem {
namespace linguistics {

void extractSubChunkInsideLinkedTokens(std::list<ChunkLink>& pFirstChildren, SemanticLanguageEnum pLanguage) {
    for (auto& currChunkLk : pFirstChildren) {
        auto& chunk = *currChunkLk.chunk;
        if (chunkTypeIsVerbal(chunk.type)) {
            TokIt tokenIt = chunk.tokRange.getItBegin();
            // Is head of word group
            if (tokenIt->linkedTokens.size() > 1) {
                auto itNextLinkedToken = *(++tokenIt->linkedTokens.begin());
                TokIt itEnd = chunk.tokRange.getItEnd();
                tokenIt = getNextToken(tokenIt, itEnd);

                // skip adverbs
                while (tokenIt != itEnd && tokenIt != itNextLinkedToken) {
                    if (tokenIt->getPartOfSpeech() == PartOfSpeech::ADVERB)
                        tokenIt = getNextToken(tokenIt, itEnd);
                    break;
                }

                auto itBeginOfNominalGroup = tokenIt;
                // extract nominal group inside
                while (tokenIt != itEnd && tokenIt != itNextLinkedToken) {
                    tokenIt = getNextToken(tokenIt, itEnd);
                }

                if (tokenIt != itBeginOfNominalGroup && tokenIt != itEnd) {
                    std::list<ChunkLink> endOfVerbalChunk;
                    moveEndOfAChunk(chunk, tokenIt, ChunkLinkType::SIMPLE, chunk.type, endOfVerbalChunk, pLanguage);

                    moveEndOfAChunk(chunk,
                                    itBeginOfNominalGroup,
                                    ChunkLinkType::DIRECTOBJECT,
                                    ChunkType::NOMINAL_CHUNK,
                                    chunk.children,
                                    pLanguage);
                    chunk.children.splice(chunk.children.end(), endOfVerbalChunk);
                }
            }
        }
    }
}

}    // End of namespace linguistics
}    // End of namespace onsem
