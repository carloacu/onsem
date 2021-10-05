#include <onsem/lingdbeditor/allingdbmultimeaningnode.hpp>
#include <string>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>


namespace onsem
{

void ALLingdbMultiMeaningsNode::getPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbMultiMeaningsNode*>
                 (pVar)->fRootMeaning);
  pRes.emplace_back(&reinterpret_cast<ALLingdbMultiMeaningsNode*>
                 (pVar)->fLinkedMeanings);
}


ALLingdbMultiMeaningsNode::ALLingdbMultiMeaningsNode
(ALLingdbMeaning* pRootMeaning)
  : fRootMeaning(pRootMeaning),
    fLinkedMeanings(nullptr)
{
  assert(pRootMeaning != nullptr);
}


void ALLingdbMultiMeaningsNode::xInit
(ALCompositePoolAllocator& pAlloc,
 ALLingdbMeaning* pRootMeaning,
 std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings)
{
  assert(pRootMeaning != nullptr);
  fRootMeaning = pRootMeaning;
  fLinkedMeanings = nullptr;
  assert(!pLinkedMeanings.empty());

  std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >::iterator
      itLinkMean = pLinkedMeanings.end();
  do
  {
    --itLinkMean;
    ALLingdbNodeLinkedMeaning* newLinkedMeaning = pAlloc.allocate<ALLingdbNodeLinkedMeaning>(1);
    newLinkedMeaning->init(static_cast<char>(itLinkMean->second),
                           itLinkMean->first);

    ForwardPtrList<ALLingdbNodeLinkedMeaning>* newLinkedMeaningList =
        pAlloc.allocate<ForwardPtrList<ALLingdbNodeLinkedMeaning> >(1);
    newLinkedMeaningList->init(newLinkedMeaning);
    newLinkedMeaningList->next = fLinkedMeanings;
    fLinkedMeanings = newLinkedMeaningList;
  }
  while (itLinkMean != pLinkedMeanings.begin());
}



bool ALLingdbMultiMeaningsNode::isStrEqualToListOfLemmes
(const std::string& pStr,
 std::size_t pBegin) const
{
  ForwardPtrList<ALLingdbNodeLinkedMeaning>* lksMean = fLinkedMeanings;
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
