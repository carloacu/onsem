#include "allingdbanimationtag.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/lingdbeditor/allingdbstring.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>


namespace onsem
{

void PonderatedMeaning::init(ALLingdbMeaning* pMeaning,
          char pRelation)
{
  meaning = pMeaning;
  relation = pRelation;
}



ALLingdbAnimationsTag::ALLingdbAnimationsTag()
  : fTag(nullptr),
    fMeanings(nullptr),
    fLinksToConcept(nullptr)
{
}


void ALLingdbAnimationsTag::xInit
(ALCompositePoolAllocator& pFPAlloc,
 const std::string& pTagName)
{
  fTag = pFPAlloc.allocate<ALLingdbString>(1);
  fTag->xInit(pFPAlloc, pTagName);
  fMeanings = nullptr;
  fLinksToConcept = nullptr;
}


void ALLingdbAnimationsTag::xDeallocate
(ALCompositePoolAllocator& pFPAlloc)
{
  pFPAlloc.deallocate<ALLingdbString>(fTag);
  if (fLinksToConcept != nullptr)
  {
    fLinksToConcept->clear(pFPAlloc);
    fLinksToConcept = nullptr;
  }
}

void ALLingdbAnimationsTag::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbAnimationsTag*>
                 (pVar)->fTag);
  pRes.emplace_back(&reinterpret_cast<ALLingdbAnimationsTag*>
                 (pVar)->fMeanings);
  pRes.emplace_back(&reinterpret_cast<ALLingdbAnimationsTag*>
                 (pVar)->fLinksToConcept);
}


void ALLingdbAnimationsTag::addConcept
(ALCompositePoolAllocator& pAlloc,
 ALLingdbConcept* pConcept,
 char pMinValue)
{
  ALLingdbLinkToAConcept* newLinkToConcept = pAlloc.allocate<ALLingdbLinkToAConcept>(1);
  newLinkToConcept->xInit(pConcept, pMinValue);

  ForwardPtrList<ALLingdbLinkToAConcept>* newLkToConceptList =
      pAlloc.allocate<ForwardPtrList<ALLingdbLinkToAConcept> >(1);
  newLkToConceptList->init(newLinkToConcept);
  newLkToConceptList->next = fLinksToConcept;
  fLinksToConcept = newLkToConceptList;
}


void ALLingdbAnimationsTag::addMeaning
(ALCompositePoolAllocator& pAlloc,
 ALLingdbMeaning* pMeaning,
 char pRelation)
{
  PonderatedMeaning* pondMeaning = pAlloc.allocate<PonderatedMeaning>(1);
  pondMeaning->init(pMeaning, pRelation);

  ForwardPtrList<PonderatedMeaning>* newMeaningsList = pAlloc.allocate<ForwardPtrList<PonderatedMeaning> >(1);
  newMeaningsList->init(pondMeaning);
  newMeaningsList->next = fMeanings;
  fMeanings = newMeaningsList;
}




} // End of namespace onsem
