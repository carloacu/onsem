#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_VERBALCHUNKER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_VERBALCHUNKER_HPP

#include <vector>
#include <list>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>

namespace onsem {
namespace linguistics {
struct Token;
struct TokensTree;
class AlgorithmSetForALanguage;
struct SpecificLinguisticDatabase;
struct ChunkLinkWorkingZone;
struct ChunkLink;

class VerbalChunker {
public:
    explicit VerbalChunker(const AlgorithmSetForALanguage& pConfiguration);

    void process(TokensTree& pTokensTree, std::list<ChunkLink>& pFirstChildren) const;

private:
    const AlgorithmSetForALanguage& fConf;
    const SpecificLinguisticDatabase& fSpecLingDb;

    void xFindVerbsAndAuxiliaries(std::list<TokIt>& pConjVerbs, std::vector<Token>& pTokens) const;

    void xDelimitVerbChunk(TokIt& pItVerbBeginGroup,
                           TokIt& pItVerbEndGroup,
                           TokIt pItVerb,
                           const TokenRange& pTokRangeContext) const;

    void xSplitChunks(ChunkLinkWorkingZone& pWorkingZone,
                      const std::vector<PartOfSpeech>& pChunckSeps,
                      ChunkType pChunkType) const;

    static TokIt xUntilNextConjVerb(std::list<TokIt>::const_iterator pCurrVerb,
                                    const std::list<TokIt>& pVerbList,
                                    TokIt pLastToken);

    bool xIsPositif(const Chunk& pChunk) const;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_VERBALCHUNKER_HPP
