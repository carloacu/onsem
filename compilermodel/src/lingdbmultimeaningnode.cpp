#include <onsem/compilermodel/lingdbmultimeaningnode.hpp>
#include <string>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>


namespace onsem
{

void LingdbMultiMeaningsNode::getPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbMultiMeaningsNode*>
                 (pVar)->fRootMeaning);
  pRes.emplace_back(&reinterpret_cast<LingdbMultiMeaningsNode*>
                 (pVar)->fLinkedMeanings);
}


LingdbMultiMeaningsNode::LingdbMultiMeaningsNode
(LingdbMeaning* pRootMeaning)
  : fRootMeaning(pRootMeaning),
    fLinkedMeanings(nullptr)
{
  assert(pRootMeaning != nullptr);
}


void LingdbMultiMeaningsNode::xInit
(CompositePoolAllocator& pAlloc,
 LingdbMeaning* pRootMeaning,
 std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings)
{
  assert(pRootMeaning != nullptr);
  fRootMeaning = pRootMeaning;
  fLinkedMeanings = nullptr;
  assert(!pLinkedMeanings.empty());

  std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >::iterator
      itLinkMean = pLinkedMeanings.end();
  do
  {
    --itLinkMean;
    LingdbNodeLinkedMeaning* newLinkedMeaning = pAlloc.allocate<LingdbNodeLinkedMeaning>(1);
    newLinkedMeaning->init(static_cast<char>(itLinkMean->second),
                           itLinkMean->first);

    ForwardPtrList<LingdbNodeLinkedMeaning>* newLinkedMeaningList =
        pAlloc.allocate<ForwardPtrList<LingdbNodeLinkedMeaning> >(1);
    newLinkedMeaningList->init(newLinkedMeaning);
    newLinkedMeaningList->next = fLinkedMeanings;
    fLinkedMeanings = newLinkedMeaningList;
  }
  while (itLinkMean != pLinkedMeanings.begin());
}



bool LingdbMultiMeaningsNode::isStrEqualToListOfLemmes
(const std::string& pStr,
 std::size_t pBegin) const
{
  ForwardPtrList<LingdbNodeLinkedMeaning>* lksMean = fLinkedMeanings;
  std::size_t currPos = pStr.find_first_of('~', pBegin);
  while (currPos != std::string::npos &&
         currPos > pBegin)
  {
    if (lksMean == nullptr ||
        lksMean->elt->meaning->getLemma()->getWord()
        != pStr.substr(pBegin, currPos - pBegin))
    {
      return false;
    }

    lksMean = lksMean->next;
    pBegin = currPos + 1;
    currPos = pStr.find_first_of('~', pBegin);
  }

  if (lksMean == nullptr ||
      lksMean->elt->meaning->getLemma()->getWord()
      != pStr.substr(pBegin, pStr.size() - pBegin))
  {
    return false;
  }

  return lksMean->next == nullptr;
}




} // End of namespace onsem
