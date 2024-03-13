#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGER_HPP

#include "../synthesizertypes.hpp"

namespace onsem {

class SynthesizerChunksMerger {
public:
    virtual ~SynthesizerChunksMerger() {}

    virtual void formulateNominalGroup(std::list<WordToSynthesize>& pOut, OutNominalGroup& pOutSentence) const = 0;

    virtual void formulateSentence(std::list<WordToSynthesize>& pOut, OutSentence& pOutSentence) const = 0;

protected:
    SynthesizerChunksMerger() {}

    static void _writeDurationLocationAndTimeInGoodOrder(std::list<WordToSynthesize>& pOut, OutSentence& pOutSentence);

    static void _filterForInSentenceContext(std::list<WordToSynthesize>& pOut, const OutSentence& pOutSentence);
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGER_HPP
