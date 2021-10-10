#include "lingdbconcept.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>

namespace onsem
{

LingdbConcept::LingdbConcept()
  : fName(nullptr),
    fOppositeConcepts(nullptr),
    fNearlyEqualConcepts(nullptr),
    fAutoFill(true)
{
}


void LingdbConcept::xInit
(ALCompositePoolAllocator& pAlloc,
 const std::string& pName,
 bool pAutoFill)
{
  fName = pAlloc.allocate<LingdbString>(1);
  fName->xInit(pAlloc, pName);
  fOppositeConcepts = nullptr;
  fNearlyEqualConcepts = nullptr;
  fAutoFill = pAutoFill;
}


void LingdbConcept::xDeallocate
(ALCompositePoolAllocator& pAlloc)
{
  fName->xDeallocate(pAlloc);
  fOppositeConcepts->clearWithoutDesallocateElts(pAlloc);
  fNearlyEqualConcepts->clearWithoutDesallocateElts(pAlloc);
  pAlloc.deallocate<LingdbConcept>(this);
}

void LingdbConcept::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbConcept*>
                 (pVar)->fName);
  pRes.emplace_back(&reinterpret_cast<LingdbConcept*>
                 (pVar)->fOppositeConcepts);
  pRes.emplace_back(&reinterpret_cast<LingdbConcept*>
                 (pVar)->fNearlyEqualConcepts);
}



void LingdbConcept::addAnOppositeConcept
(ALCompositePoolAllocator& pAlloc,
 LingdbConcept* pOppositeConcept)
{
  ForwardPtrList<LingdbConcept>* newOppCpt =
      pAlloc.allocate<ForwardPtrList<LingdbConcept> >(1);
  newOppCpt->init(pOppositeConcept);
  newOppCpt->next = nullptr;

  if (fOppositeConcepts == nullptr)
    fOppositeConcepts = newOppCpt;
  else
    fOppositeConcepts->back()->next = newOppCpt;
}


void LingdbConcept::addANearlyEqualConcept
(ALCompositePoolAllocator& pAlloc,
 LingdbConcept* pNearlyEqualConcept)
{
  ForwardPtrList<LingdbConcept>* newEquCpt =
      pAlloc.allocate<ForwardPtrList<LingdbConcept> >(1);
  newEquCpt->init(pNearlyEqualConcept);
  newEquCpt->next = nullptr;

  if (fNearlyEqualConcepts == nullptr)
    fNearlyEqualConcepts = newEquCpt;
  else
    fNearlyEqualConcepts->back()->next = newEquCpt;
}


bool LingdbConcept::conceptNameFinishWithAStar
(const std::string& pConceptName)
{
  return pConceptName.size() > 2 &&
      pConceptName.compare(pConceptName.size() - 2, 2, "_*") == 0;
}


} // End of namespace onsem
