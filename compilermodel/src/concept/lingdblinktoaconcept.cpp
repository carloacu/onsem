#include "lingdblinktoaconcept.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include "lingdbconcept.hpp"
#include <onsem/compilermodel/lingdbstring.hpp>

namespace onsem
{

LingdbLinkToAConcept::LingdbLinkToAConcept()
  : fConcept(nullptr),
    fRelatedToConcept(0)
{
}

void LingdbLinkToAConcept::xInit
(const LingdbConcept* pConcept,
 char pRelatedToConcept)
{
  fConcept = pConcept;
  fRelatedToConcept = pRelatedToConcept;
}

bool LingdbLinkToAConcept::operator<
(const LingdbLinkToAConcept& pOther) const
{
  if (fRelatedToConcept != pOther.fRelatedToConcept)
  {
    if (abs(fRelatedToConcept) < abs(pOther.fRelatedToConcept))
    {
      return true;
    }
    else if (abs(fRelatedToConcept) > abs(pOther.fRelatedToConcept))
    {
      return false;
    }
    return fRelatedToConcept < pOther.fRelatedToConcept;
  }
  return fConcept->getName()->toStr() > pOther.fConcept->getName()->toStr();
}




void LingdbLinkToAConcept::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbLinkToAConcept*>
                 (pVar)->fConcept);
}


} // End of namespace onsem
