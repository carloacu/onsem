#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGERENGLISH_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGERENGLISH_HPP

#include "synthesizerchunksmerger.hpp"

namespace onsem {

class SynthesizerChunksMergerEnglish : public SynthesizerChunksMerger {
public:
    virtual void formulateNominalGroup(std::list<WordToSynthesize>& pOut, OutNominalGroup& pOutSentence) const;

    virtual void formulateSentence(std::list<WordToSynthesize>& pOut, OutSentence& pOutSentence) const;

private:
    void _formulation_objectQuestionWithSubjetAfterTheVerb(std::list<WordToSynthesize>& pOut,
                                                           OutSentence& pOutSentence) const;

    void _formulation_objectQuestionWithSubjetBeforeTheVerb(std::list<WordToSynthesize>& pOut,
                                                            OutSentence& pOutSentence) const;

    void _formulation_default(std::list<WordToSynthesize>& pOut, OutSentence& pOutSentence) const;

    void _writEndOfSentence(std::list<WordToSynthesize>& pOut, OutSentence& pOutSentence) const;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_MERGER_SYNTHESIZERCUNKSMERGERENGLISH_HPP
