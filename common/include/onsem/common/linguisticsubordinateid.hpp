#ifndef ONSEM_COMMON_LINGUISTICSUBORDINATEID_HPP
#define ONSEM_COMMON_LINGUISTICSUBORDINATEID_HPP

#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/enum/semanticrelativedurationtype.hpp>
#include <onsem/common/enum/semanticrelativelocationtype.hpp>
#include <onsem/common/enum/semanticrelativetimetype.hpp>


namespace onsem
{
namespace linguistics
{

static const char noRelativeSubordinate = -1;


struct LinguisticSubordinateId
{
  ChunkLinkType chunkLinkType{ChunkLinkType::SIMPLE};
  char relativeSubodinate{noRelativeSubordinate};

  bool operator<(const LinguisticSubordinateId& pOther) const
  {
    if (chunkLinkType != pOther.chunkLinkType)
      return chunkLinkType < pOther.chunkLinkType;
    return relativeSubodinate < pOther.relativeSubodinate;
  }
};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_COMMON_LINGUISTICSUBORDINATEID_HPP
