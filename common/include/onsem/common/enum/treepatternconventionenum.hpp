#ifndef ONSEM_COMMON_ENUM_TREEPATTERNCONVENTIONENUM_HPP
#define ONSEM_COMMON_ENUM_TREEPATTERNCONVENTIONENUM_HPP

#include <string>

namespace onsem
{


#define TREEPATTERN_ENUM_TABLE                             \
  ADD_TREEPATTERN_ENUMVAL(TREEPATTERN_INTEXT, "inText")    \
  ADD_TREEPATTERN_ENUMVAL(TREEPATTERN_OUTTEXT, "outText")  \
  ADD_TREEPATTERN_ENUMVAL(TREEPATTERN_MIND, "mind")


#define ADD_TREEPATTERN_ENUMVAL(a, b) a,
enum TreePatternConventionEnum {
  TREEPATTERN_ENUM_TABLE
  TREEPATTERN_ENUM_TABLE_ENDFORNOCOMPILWARNING
};
#undef ADD_TREEPATTERN_ENUMVAL


#define ADD_TREEPATTERN_ENUMVAL(a, b) b,
static const std::string treePatternConventionEnum_toStr[] = {
  TREEPATTERN_ENUM_TABLE
};
#undef ADD_TREEPATTERN_ENUMVAL


static inline TreePatternConventionEnum treePatternConventionEnum_fromStr
(const std::string& pValStr)
{
  for (std::size_t i = 0;
       i < TREEPATTERN_ENUM_TABLE_ENDFORNOCOMPILWARNING; ++i)
  {
    if (treePatternConventionEnum_toStr[i] == pValStr)
    {
      return TreePatternConventionEnum(i);
    }
  }
  return TREEPATTERN_ENUM_TABLE_ENDFORNOCOMPILWARNING;
}


} // End of namespace onsem

#endif // ONSEM_COMMON_ENUM_TREEPATTERNCONVENTIONENUM_HPP
