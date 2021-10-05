#include "allingdblinktoaconcept.hpp"
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include "allingdbconcept.hpp"
#include <onsem/lingdbeditor/allingdbstring.hpp>

namespace onsem
{

ALLingdbLinkToAConcept::ALLingdbLinkToAConcept()
  : fConcept(nullptr),
    fRelatedToConcept(0)
{
}

void ALLingdbLinkToAConcept::xInit
(const ALLingdbConcept* pConcept,
 char pRelatedToConcept)
{
  fConcept = pConcept;
  fRelatedToConcept = pRelatedToConcept;
}

bool ALLingdbLinkToAConcept::operator<
(const ALLingdbLinkToAConcept& pOther) const
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




void ALLingdbLinkToAConcept::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbLinkToAConcept*>
                 (pVar)->fConcept);
}


} // End of namespace onsem
