#ifndef ONSEM_TEXTTOSEMANTIC_SRC_STEPS_VERBALTONOMINALCHUNKSLINKER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_STEPS_VERBALTONOMINALCHUNKSLINKER_HPP

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

class VerbalToNominalChunksLinker {
public:
    explicit VerbalToNominalChunksLinker(const AlgorithmSetForALanguage& pConfiguration);

    void process(std::list<ChunkLink>& pFirstChildren) const;

    void extractEnglishSubjectOf(std::list<ChunkLink>& pFirstChildren) const;

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
    const SpecificLinguisticDatabase& fSpecLingDb;
    const LinguisticDictionary& fLingDico;

    void _extractQuestionWords(ChunkLinkWorkingZone& pWorkingZone,
                               Chunk* pFirstVerbChunk,
                               Chunk* pSecondVerbChunk,
                               Chunk* pSubSecondVerbChunk) const;

    void _tryAddDirectObjectToImperativeVerb(Chunk& pVerbRoot, ChunkLinkWorkingZone& pWorkingZone) const;

    void _linkObjectBeforeAnInterrogativeVerb(ChunkLinkWorkingZone& pWorkingZone,
                                              Chunk& pSecondVerbChunk,
                                              Chunk& pSubSecondVerbChunk) const;

    bool _linkAVerbGroupToHisCOD(ChunkLinkWorkingZone& pWorkingZone, Chunk* pVerbChunk) const;

    bool _linkAVerbChunkToHisSubject(Chunk& pSubVerbChunk, Chunk& pVerbChunk, ChunkLink& pPotentialSubject) const;

    void _splitAdverbBeforeNouns(ChunkLinkIter& pChunkLinkIter) const;

    void _constructASyntGraphBetween2VerbChunks(std::list<ChunkLink>& pRootList,
                                                std::list<ChunkLink>::iterator pItFirstVerbChunk,
                                                std::list<ChunkLink>::iterator pItSecondVerbChunk) const;

    bool _canLinkNextChunkAsTheSubject(ChunkLinkWorkingZone& pWorkingZone,
                                       const Chunk* firstVerbChunk,
                                       TokIt newFirstChunkHead) const;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_STEPS_VERBALTONOMINALCHUNKSLINKER_HPP
