#include <onsem/lingdbeditor/allingdbwordforms.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/allingdbflexions.hpp>

namespace onsem
{

ALLingdbWordForms::ALLingdbWordForms()
  : fMeaning(nullptr),
    fFlexions(nullptr),
    fFrequency(4)
{
}


void ALLingdbWordForms::xInit
(ALLingdbMeaning* pMeaning)
{
  fMeaning = pMeaning;
  fFlexions = nullptr;
  fFrequency = 4;
  fMeaning->xAddAPtrToThisMeaning();
}


void ALLingdbWordForms::xAddFlexions
(ALCompositePoolAllocator& pFPAlloc,
 PartOfSpeech pGram,
 const std::vector<std::string>& pFlexions)
{
  if (pFlexions.empty())
  {
    return;
  }
  if (fFlexions == nullptr)
  {
    fFlexions = pFPAlloc.allocate<ALLingdbFlexions>(1);
    fFlexions->xInit(pFPAlloc, pGram, pFlexions, this);
  }
  else
  {
    fFlexions->xAddNewFlexions(pFPAlloc, pGram, pFlexions,
                               this);
  }
}


void ALLingdbWordForms::copyFlexions
(LinguisticIntermediaryDatabase& pLingDatabase,
 const ALLingdbFlexions* pReferenceFlexions)
{
  if (fFlexions == nullptr && pReferenceFlexions != nullptr)
  {
    fFlexions = pReferenceFlexions->xClone(pLingDatabase.xGetFPAlloc());
  }
}



void ALLingdbWordForms::xDeallocate
(ALCompositePoolAllocator& pFPAlloc)
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
  pFPAlloc.deallocate<ALLingdbWordForms>(this);
}



} // End of namespace onsem
