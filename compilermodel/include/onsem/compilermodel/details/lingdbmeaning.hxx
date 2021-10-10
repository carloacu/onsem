#ifndef ONSEM_COMPILERMODEL_LINGDBMEANING_HXX
#define ONSEM_COMPILERMODEL_LINGDBMEANING_HXX

#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <assert.h>

namespace onsem
{


inline LingdbMeaning::LingdbMeaning
()
  : fPartOfSpeech(0),
    fLemma(nullptr),
    fPtrCount(0),
    fLinkToConcepts(nullptr),
    fContextInfos(nullptr)
{
}


inline void LingdbMeaning::xInit
(LingdbDynamicTrieNode* pLemme, char pGram)
{
  fPartOfSpeech = pGram;
  assert(pLemme != nullptr);
  fLemma = pLemme;
  fPtrCount = 0;
  fLinkToConcepts = nullptr;
  fContextInfos = nullptr;
}


inline void LingdbMeaning::xAddAPtrToThisMeaning()
{
  ++fPtrCount;
}

inline unsigned int LingdbMeaning::xRemoveAPtrToThisMeaning()
{
  assert(fPtrCount > 0);
  return --fPtrCount;
}



inline const ForwardPtrList<LingdbLinkToAConcept>* LingdbMeaning::getLinkToConcepts
() const
{
  return fLinkToConcepts;
}


inline LingdbDynamicTrieNode* LingdbMeaning::getLemma
() const
{
  return fLemma;
}


inline PartOfSpeech LingdbMeaning::getPartOfSpeech() const
{
  return static_cast<PartOfSpeech>(fPartOfSpeech);
}

inline const ForwardPtrList<char>* LingdbMeaning::getContextInfos
() const
{
  return fContextInfos;
}



inline void LingdbMeaning::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbMeaning*>
                 (pVar)->fLemma);
  pRes.emplace_back(&reinterpret_cast<LingdbMeaning*>
                 (pVar)->fLinkToConcepts);
  pRes.emplace_back(&reinterpret_cast<LingdbMeaning*>
                 (pVar)->fContextInfos);
}


} // End of namespace onsem




#endif // ONSEM_COMPILERMODEL_LINGDBMEANING_HXX
