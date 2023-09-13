#include "synthesizerchunksmerger.hpp"


namespace onsem
{
namespace
{

void _addPrioritytoOutList(
    std::map<std::size_t, std::list<std::list<WordToSynthesize>*>>& pPriorities,
    OutSemExp& pOutSemExp)
{
  if (!pOutSemExp.out.empty())
    pPriorities[pOutSemExp.getPriority()].emplace_back(&pOutSemExp.out);
}

}


void SynthesizerChunksMerger::_writeDurationLocationAndTimeInGoodOrder(
    std::list<WordToSynthesize>& pOut,
    OutSentence& pOutSentence)
{
  std::map<std::size_t, std::list<std::list<WordToSynthesize>*>> priorities;
  _addPrioritytoOutList(priorities, pOutSentence.length);
  _addPrioritytoOutList(priorities, pOutSentence.duration);
  _addPrioritytoOutList(priorities, pOutSentence.time);
  _addPrioritytoOutList(priorities, pOutSentence.location);
  for (auto& currOuts : priorities)
    for (auto& currOut : currOuts.second)
      pOut.splice(pOut.end(), *currOut);

  pOut.splice(pOut.end(), pOutSentence.other.out);
}


void SynthesizerChunksMerger::_filterForInSentenceContext(std::list<WordToSynthesize>& pOut,
                                                          const OutSentence& pOutSentence)
{
  for (auto& currWordToSynthesize : pOut)
  {
    for (auto it = currWordToSynthesize.inflections.begin(); it != currWordToSynthesize.inflections.end(); )
    {
      if (it->contextCondition(&pOutSentence))
        ++it;
      else
        it = currWordToSynthesize.inflections.erase(it);
    }
  }
}

} // End of namespace onsem
