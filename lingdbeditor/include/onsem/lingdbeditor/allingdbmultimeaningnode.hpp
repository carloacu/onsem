#ifndef ALLINGDBMULTIMEANINGSNODE_H
#define ALLINGDBMULTIMEANINGSNODE_H

#include <vector>
#include <list>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>


namespace onsem
{
template<typename T>
struct ForwardPtrList;
class ALLingdbMeaning;
class ALLingdbWordForms;
class ALLingdbDynamicTrieNode;



struct ALLingdbNodeLinkedMeaning
{
  ALLingdbNodeLinkedMeaning
  (char pDirection,
   ALLingdbMeaning* pdMeaning)
    : direction(pDirection),
      meaning(pdMeaning)
  {
  }

  void init
  (char pDirection,
   ALLingdbMeaning* pdMeaning)
  {
    direction = pDirection;
    meaning = pdMeaning;
  }


  char direction; // type is LinkedMeaningDirection
  ALLingdbMeaning* meaning;

  static void getPointers
  (std::vector<const void*>& pRes, void* pVar)
  {
    pRes.emplace_back(&reinterpret_cast<ALLingdbNodeLinkedMeaning*>
                   (pVar)->meaning);
  }
};




/// A node of the dynamic trie that store the words.
class ALLingdbMultiMeaningsNode
{
public:

  ALLingdbMeaning* getRootMeaning() const
  {
    return fRootMeaning;
  }

  const ForwardPtrList<ALLingdbNodeLinkedMeaning>* getLinkedMeanings() const
  {
    return fLinkedMeanings;
  }

  bool isStrEqualToListOfLemmes
  (const std::string& pStr,
   std::size_t pBegin) const;


  static void getPointers
  (std::vector<const void*>& pRes, void* pVar);


private:
  ALLingdbMeaning* fRootMeaning;
  ForwardPtrList<ALLingdbNodeLinkedMeaning>* fLinkedMeanings;


private:
  friend class ALLingdbDynamicTrieNode;


  ALLingdbMultiMeaningsNode(ALLingdbMeaning* pRootMeaning);

  void xInit
  (ALCompositePoolAllocator& pAlloc,
   ALLingdbMeaning* pRootMeaning,
   std::list<std::pair<ALLingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings);
};


} // End of namespace onsem


#endif // ALLINGDBMULTIMEANINGSNODE_H
