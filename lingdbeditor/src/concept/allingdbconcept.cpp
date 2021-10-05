#include "allingdbconcept.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/lingdbeditor/allingdbstring.hpp>
#include <onsem/lingdbeditor/allingdbtypes.hpp>

namespace onsem
{

ALLingdbConcept::ALLingdbConcept()
  : fName(nullptr),
    fOppositeConcepts(nullptr),
    fNearlyEqualConcepts(nullptr),
    fAutoFill(true)
{
}


void ALLingdbConcept::xInit
(ALCompositePoolAllocator& pAlloc,
 const std::string& pName,
 bool pAutoFill)
{
  fName = pAlloc.allocate<ALLingdbString>(1);
  fName->xInit(pAlloc, pName);
  fOppositeConcepts = nullptr;
  fNearlyEqualConcepts = nullptr;
  fAutoFill = pAutoFill;
}


void ALLingdbConcept::xDeallocate
(ALCompositePoolAllocator& pAlloc)
{
  fName->xDeallocate(pAlloc);
  fOppositeConcepts->clearWithoutDesallocateElts(pAlloc);
  fNearlyEqualConcepts->clearWithoutDesallocateElts(pAlloc);
  pAlloc.deallocate<ALLingdbConcept>(this);
}

void ALLingdbConcept::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbConcept*>
                 (pVar)->fName);
  pRes.emplace_back(&reinterpret_cast<ALLingdbConcept*>
                 (pVar)->fOppositeConcepts);
  pRes.emplace_back(&reinterpret_cast<ALLingdbConcept*>
                 (pVar)->fNearlyEqualConcepts);
}



void ALLingdbConcept::addAnOppositeConcept
(ALCompositePoolAllocator& pAlloc,
 ALLingdbConcept* pOppositeConcept)
{
  ForwardPtrList<ALLingdbConcept>* newOppCpt =
      pAlloc.allocate<ForwardPtrList<ALLingdbConcept> >(1);
  newOppCpt->init(pOppositeConcept);
  newOppCpt->next = nullptr;

  if (fOppositeConcepts == nullptr)
    fOppositeConcepts = newOppCpt;
  else
    fOppositeConcepts->back()->next = newOppCpt;
}


void ALLingdbConcept::addANearlyEqualConcept
(ALCompositePoolAllocator& pAlloc,
 ALLingdbConcept* pNearlyEqualConcept)
{
  ForwardPtrList<ALLingdbConcept>* newEquCpt =
      pAlloc.allocate<ForwardPtrList<ALLingdbConcept> >(1);
  newEquCpt->init(pNearlyEqualConcept);
  newEquCpt->next = nullptr;

  if (fNearlyEqualConcepts == nullptr)
    fNearlyEqualConcepts = newEquCpt;
  else
    fNearlyEqualConcepts->back()->next = newEquCpt;
}


bool ALLingdbConcept::conceptNameFinishWithAStar
(const std::string& pConceptName)
{
  return pConceptName.size() > 2 &&
      pConceptName.compare(pConceptName.size() - 2, 2, "_*") == 0;
}


} // End of namespace onsem
