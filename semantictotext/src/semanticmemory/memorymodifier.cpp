#include "memorymodifier.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp>

namespace onsem
{

namespace MemoryModifier
{

template <typename MEMSENTENCES>
void _tryToRemoveAndAdvance
(std::map<intSemId, MEMSENTENCES*>& pSet1,
 typename std::map<intSemId, MEMSENTENCES*>::iterator pItSet1,
 GrammaticalType pExceptThoseWhoDoesntHaveThisChild)
{
  if (pExceptThoseWhoDoesntHaveThisChild == GrammaticalType::UNKNOWN)
    pSet1.erase(pItSet1);
  else
  {
    auto& sentWeWantToRemove = *pItSet1->second;
    auto itChild = sentWeWantToRemove.grdExp.children.find(pExceptThoseWhoDoesntHaveThisChild);
    if (itChild != sentWeWantToRemove.grdExp.children.end())
      pSet1.erase(pItSet1);
  }
}


template <bool IS_MODIFIABLE>
void semExpSetIntersectionInPlace
(SentenceLinks<IS_MODIFIABLE>& pSet1,
 const SentenceLinks<IS_MODIFIABLE>& pSet2,
 GrammaticalType pExceptThoseWhoDoesntHaveThisChild)
{
  for (auto it1 = pSet1.dynamicLinks.begin(); it1 != pSet1.dynamicLinks.end(); )
  {
    if (pSet2.dynamicLinks.count(it1->first) == 0)
      _tryToRemoveAndAdvance(pSet1.dynamicLinks, it1++,
                             pExceptThoseWhoDoesntHaveThisChild);
    else
      ++it1;
  }
}


template void semExpSetIntersectionInPlace(SentenceLinks<true>& pSet1,
                                           const SentenceLinks<true>& pSet2,
                                           GrammaticalType pExceptThoseWhoDoesntHaveThisChild);

template void semExpSetIntersectionInPlace(SentenceLinks<false>& pSet1,
                                           const SentenceLinks<false>& pSet2,
                                           GrammaticalType pExceptThoseWhoDoesntHaveThisChild);

} // End of namespace MemoryModifier

} // End of namespace onsem
