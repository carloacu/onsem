#include "synthesizerchunksmerger.hpp"


namespace onsem
{


void SynthesizerChunksMerger::_writeDurationLocationAndTimeInGoodOrder(
    std::list<WordToSynthesize>& pOut,
    OutSentence& pOutSentence)
{
  std::map<std::size_t, std::list<std::list<WordToSynthesize>*>> priorities;
  priorities[pOutSentence.length.getPriority()]
      .emplace_back(&pOutSentence.length.out);
  priorities[pOutSentence.duration.getPriority()]
      .emplace_back(&pOutSentence.duration.out);
  priorities[pOutSentence.time.getPriority()]
      .emplace_back(&pOutSentence.time.out);
  priorities[pOutSentence.location.getPriority()]
      .emplace_back(&pOutSentence.location.out);
  for (auto& currOuts : priorities)
    for (auto& currOut : currOuts.second)
      pOut.splice(pOut.end(), *currOut);

  pOut.splice(pOut.end(), pOutSentence.other.out);
}


} // End of namespace onsem
