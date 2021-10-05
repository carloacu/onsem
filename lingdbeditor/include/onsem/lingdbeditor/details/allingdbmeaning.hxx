#ifndef ALLINGDBMEANING_HXX
#define ALLINGDBMEANING_HXX

#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <assert.h>

namespace onsem
{


inline ALLingdbMeaning::ALLingdbMeaning
()
  : fPartOfSpeech(0),
    fLemma(nullptr),
    fPtrCount(0),
    fLinkToConcepts(nullptr),
    fContextInfos(nullptr)
{
}


inline void ALLingdbMeaning::xInit
(ALLingdbDynamicTrieNode* pLemme, char pGram)
{
  fPartOfSpeech = pGram;
  assert(pLemme != nullptr);
  fLemma = pLemme;
  fPtrCount = 0;
  fLinkToConcepts = nullptr;
  fContextInfos = nullptr;
}


inline void ALLingdbMeaning::xAddAPtrToThisMeaning()
{
  ++fPtrCount;
}

inline unsigned int ALLingdbMeaning::xRemoveAPtrToThisMeaning()
{
  assert(fPtrCount > 0);
  return --fPtrCount;
}



inline const ForwardPtrList<ALLingdbLinkToAConcept>* ALLingdbMeaning::getLinkToConcepts
() const
{
  return fLinkToConcepts;
}


inline ALLingdbDynamicTrieNode* ALLingdbMeaning::getLemma
() const
{
  return fLemma;
}


inline PartOfSpeech ALLingdbMeaning::getPartOfSpeech() const
{
  return static_cast<PartOfSpeech>(fPartOfSpeech);
}

inline const ForwardPtrList<char>* ALLingdbMeaning::getContextInfos
() const
{
  return fContextInfos;
}



inline void ALLingdbMeaning::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbMeaning*>
                 (pVar)->fLemma);
  pRes.emplace_back(&reinterpret_cast<ALLingdbMeaning*>
                 (pVar)->fLinkToConcepts);
  pRes.emplace_back(&reinterpret_cast<ALLingdbMeaning*>
                 (pVar)->fContextInfos);
}


} // End of namespace onsem




#endif // ALLINGDBMEANING_HXX
