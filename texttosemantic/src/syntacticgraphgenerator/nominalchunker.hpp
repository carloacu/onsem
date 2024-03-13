#ifndef ONSEM_TEXTTOSEMANTIC_SRC_STEPS_NOMINALCHUNKER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_STEPS_NOMINALCHUNKER_HPP

#include <list>
#include <vector>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/listiter.hpp"

namespace onsem {
namespace linguistics {
class EntityRecognizer;
struct ChunkLinkWorkingZone;
class StaticLinguisticDictionary;
struct ChunkLink;
struct Chunk;

class NominalChunker {
public:
    explicit NominalChunker(const AlgorithmSetForALanguage& pConfiguration);

    void process(std::list<ChunkLink>& pFirstChildren) const;

private:
    struct TokPosInAChunk {
        TokPosInAChunk(TokIt pItTokenEmpty, std::list<ChunkLink>::iterator pItChkLkEmpty)
            : itTokenEmpty(pItTokenEmpty)
            , itToken(pItTokenEmpty)
            , itChkLkEmpty(pItChkLkEmpty)
            , itChkLk(pItChkLkEmpty) {}

        void clear() {
            itToken = itTokenEmpty;
            itChkLk = itChkLkEmpty;
        }

        bool isEmpty() const { return itToken == itTokenEmpty; }

        TokIt itTokenEmpty;
        TokIt itToken;
        std::list<ChunkLink>::iterator itChkLkEmpty;
        std::list<ChunkLink>::iterator itChkLk;
    };
    const AlgorithmSetForALanguage& fConf;
    const LinguisticDictionary& fLingDico;

    void xSplitAChunk(ChunkLinkIter& pItChunkLink) const;
    void xLinkPartitivesOfWorkingZone(ChunkLinkWorkingZone& pWorkingZone) const;
    void xLinkOwners(ChunkLinkWorkingZone& pWorkingZone) const;

    void xSplitPronounToken(ChunkLinkWorkingZone& pWorkingZone) const;

    void xSplitLastTokens(ChunkLinkWorkingZone& pWorkingZone, const std::vector<PartOfSpeech>& pSplitLastToken) const;

    void xProcessBetween2VerbChunks(std::list<ChunkLink>& pRootList,
                                    std::list<ChunkLink>::iterator pItFirstVerbChunk,
                                    std::list<ChunkLink>::iterator pItSecondVerbChunk) const;

    void xExtractEnglishSPossessive(std::list<ChunkLink>& pFirstChildren) const;

    void xEnglishSPossessiveAddAOwner(std::list<ChunkLink>& pFirstChildren,
                                      TokPosInAChunk& pItOwner,
                                      TokPosInAChunk& pItPoss,
                                      TokPosInAChunk& pItSPoss,
                                      TokPosInAChunk& pItMain) const;

    static void xEnglishSPossessiveClearContext(TokPosInAChunk& pItOwner,
                                                TokPosInAChunk& pItPoss,
                                                TokPosInAChunk& pItSPoss,
                                                TokPosInAChunk& pItMain);
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_STEPS_NOMINALCHUNKER_HPP
