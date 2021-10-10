#ifndef ONSEM_COMPILERMODEL_LINGUISTICINTERMADIARYDATABASE_HXX
#define ONSEM_COMPILERMODEL_LINGUISTICINTERMADIARYDATABASE_HXX

#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>


namespace onsem
{

inline const ALCompositePoolAllocator& LinguisticIntermediaryDatabase::getFPAlloc
() const
{
  return fAlloc;
}

inline ALCompositePoolAllocator& LinguisticIntermediaryDatabase::xGetFPAlloc
()
{
  return fAlloc;
}


inline void LinguisticIntermediaryDatabase::save
(const std::string& pFilename)
{
  fAlloc.serialize(pFilename);
}

inline void LinguisticIntermediaryDatabase::clear
()
{
  fConceptNameToPtr.clear();
  fAlloc.clear();
  xInitDatabase();
}

inline void LinguisticIntermediaryDatabase::setSeparatorNeeded
(bool pSeparatorNeeded)
{
  fInfos->separatorNeeded = pSeparatorNeeded;
}

inline bool LinguisticIntermediaryDatabase::isSeparatorNeeded
() const
{
  return fInfos->separatorNeeded;
}

inline unsigned int LinguisticIntermediaryDatabase::getVersion
() const
{
  return fInfos->version;
}

inline void LinguisticIntermediaryDatabase::setVersion
(unsigned int pVersion)
{
  fInfos->version = pVersion;
}

inline const LingdbString* LinguisticIntermediaryDatabase::getLanguage
() const
{
  return fInfos->language;
}

inline void LinguisticIntermediaryDatabase::setLanguage
(const std::string& pLanguage)
{
  if (fInfos->language != nullptr)
  {
    fInfos->language->xDeallocate(fAlloc);
    fInfos->language = nullptr;
  }
  fInfos->language = fAlloc.allocate<LingdbString>(1);
  fInfos->language->xInit(fAlloc, pLanguage);
}


inline void LinguisticIntermediaryDatabase::prettyPrintMemory
(std::ostream& pOs) const
{
  pOs << getFPAlloc();
}

inline bool LinguisticIntermediaryDatabase::doesWordExist
(const std::string& word) const
{
  return getPointerToEndOfWord(word) != nullptr;
}

inline LingdbDynamicTrieNode* LinguisticIntermediaryDatabase::getRoot
() const
{
  return fRoot;
}




} // End of namespace onsem

#endif // ONSEM_COMPILERMODEL_LINGUISTICINTERMADIARYDATABASE_HXX
