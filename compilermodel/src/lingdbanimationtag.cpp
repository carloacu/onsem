#include "lingdbanimationtag.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>


namespace onsem
{

void PonderatedMeaning::init(LingdbMeaning* pMeaning,
          char pRelation)
{
  meaning = pMeaning;
  relation = pRelation;
}



LingdbAnimationsTag::LingdbAnimationsTag()
  : fTag(nullptr),
    fMeanings(nullptr),
    fLinksToConcept(nullptr)
{
}


void LingdbAnimationsTag::xInit
(CompositePoolAllocator& pFPAlloc,
 const std::string& pTagName)
{
  fTag = pFPAlloc.allocate<LingdbString>(1);
  fTag->xInit(pFPAlloc, pTagName);
  fMeanings = nullptr;
  fLinksToConcept = nullptr;
}


void LingdbAnimationsTag::xDeallocate
(CompositePoolAllocator& pFPAlloc)
{
  pFPAlloc.deallocate<LingdbString>(fTag);
  if (fLinksToConcept != nullptr)
  {
    fLinksToConcept->clear(pFPAlloc);
    fLinksToConcept = nullptr;
  }
}

void LingdbAnimationsTag::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbAnimationsTag*>
                 (pVar)->fTag);
  pRes.emplace_back(&reinterpret_cast<LingdbAnimationsTag*>
                 (pVar)->fMeanings);
  pRes.emplace_back(&reinterpret_cast<LingdbAnimationsTag*>
                 (pVar)->fLinksToConcept);
}


void LingdbAnimationsTag::addConcept
(CompositePoolAllocator& pAlloc,
 LingdbConcept* pConcept,
 char pMinValue)
{
  LingdbLinkToAConcept* newLinkToConcept = pAlloc.allocate<LingdbLinkToAConcept>(1);
  newLinkToConcept->xInit(pConcept, pMinValue);

  ForwardPtrList<LingdbLinkToAConcept>* newLkToConceptList =
      pAlloc.allocate<ForwardPtrList<LingdbLinkToAConcept> >(1);
  newLkToConceptList->init(newLinkToConcept);
  newLkToConceptList->next = fLinksToConcept;
  fLinksToConcept = newLkToConceptList;
}


void LingdbAnimationsTag::addMeaning
(CompositePoolAllocator& pAlloc,
 LingdbMeaning* pMeaning,
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
