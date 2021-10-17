#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>

namespace onsem
{

LingdbWordForms::LingdbWordForms()
  : fMeaning(nullptr),
    fFlexions(nullptr),
    fFrequency(4)
{
}


void LingdbWordForms::xInit
(LingdbMeaning* pMeaning)
{
  fMeaning = pMeaning;
  fFlexions = nullptr;
  fFrequency = 4;
  fMeaning->xAddAPtrToThisMeaning();
}


void LingdbWordForms::xAddFlexions
(CompositePoolAllocator& pFPAlloc,
 PartOfSpeech pGram,
 const std::vector<std::string>& pFlexions)
{
  if (pFlexions.empty())
  {
    return;
  }
  if (fFlexions == nullptr)
  {
    fFlexions = pFPAlloc.allocate<LingdbFlexions>(1);
    fFlexions->xInit(pFPAlloc, pGram, pFlexions, this);
  }
  else
  {
    fFlexions->xAddNewFlexions(pFPAlloc, pGram, pFlexions,
                               this);
  }
}


void LingdbWordForms::copyFlexions
(LinguisticIntermediaryDatabase& pLingDatabase,
 const LingdbFlexions* pReferenceFlexions)
{
  if (fFlexions == nullptr && pReferenceFlexions != nullptr)
  {
    fFlexions = pReferenceFlexions->xClone(pLingDatabase.xGetFPAlloc());
  }
}



void LingdbWordForms::xDeallocate
(CompositePoolAllocator& pFPAlloc)
{
  if (pFPAlloc.isAllocated(fMeaning) &&
      fMeaning->xRemoveAPtrToThisMeaning() == 0)
  {
    fMeaning->xDeallocate(pFPAlloc);
  }

  if (fFlexions != nullptr)
  {
    fFlexions->xDeallocate(pFPAlloc);
  }
  pFPAlloc.deallocate<LingdbWordForms>(this);
}



} // End of namespace onsem
