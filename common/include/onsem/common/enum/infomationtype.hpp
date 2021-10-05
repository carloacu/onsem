#ifndef ONSEM_COMMON_TYPES_ENUM_INFORMATIONTYPE_HPP
#define ONSEM_COMMON_TYPES_ENUM_INFORMATIONTYPE_HPP

#include <map>
#include <string>
#include <assert.h>

namespace onsem
{


#define ONSEM_SEMANTIC_INFORMATIONTYPE_TABLE                         \
  ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(ASSERTION, "assertion")         \
  ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(INFORMATION, "information")     \
  ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(FALLBACK, "fallback")


#define ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(a, b) a,
enum class InformationType {
  ONSEM_SEMANTIC_INFORMATIONTYPE_TABLE
};
#undef ADD_ONSEM_SEMANTIC_INFORMATIONTYPE


#define ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(a, b) {InformationType::a, b},
static const std::map<InformationType, std::string> _informationType_toStr = {
  ONSEM_SEMANTIC_INFORMATIONTYPE_TABLE
};
#undef ADD_ONSEM_SEMANTIC_INFORMATIONTYPE

#define ADD_ONSEM_SEMANTIC_INFORMATIONTYPE(a, b) {b, InformationType::a},
static const std::map<std::string, InformationType> _informationType_fromStr = {
  ONSEM_SEMANTIC_INFORMATIONTYPE_TABLE
};
#undef ADD_ONSEM_SEMANTIC_INFORMATIONTYPE
#undef ONSEM_SEMANTIC_INFORMATIONTYPE_TABLE



static inline std::string informationType_toStr
(InformationType pInformationType)
{
  return _informationType_toStr.find(pInformationType)->second;
}

static inline InformationType informationType_fromStr
(const std::string& pInformationTypeStr)
{
  auto it = _informationType_fromStr.find(pInformationTypeStr);
  if (it != _informationType_fromStr.end())
    return it->second;
  assert(false);
  return InformationType::INFORMATION;
}



} // End of namespace onsem


#endif // ONSEM_COMMON_TYPES_ENUM_INFORMATIONTYPE_HPP
