#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_MISC_COREFERENCE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_MISC_COREFERENCE_HPP

#include <map>
#include <string>
#include <vector>
#include <assert.h>

namespace onsem
{

#define ONSEM_SEMANTIC_COREFERENCEDIRECTION_TABLE                                                   \
  ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(UNKNOWN, "unknown")                                       \
  ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(BEFORE, "before")                                         \
  ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(AFTER, "after")                                           \
  ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(PARENT, "parent")                                         \
  ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(ANNOTATION_SPECIFICATIONS, "annotation_specifications")



#define ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(a, b) a,
enum class CoreferenceDirectionEnum : char
{
  ONSEM_SEMANTIC_COREFERENCEDIRECTION_TABLE
};
#undef ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION


#define ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(a, b) b,
static const std::vector<std::string> _coreferenceDirectionEnum_toStr = {
  ONSEM_SEMANTIC_COREFERENCEDIRECTION_TABLE
};
#undef ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION

#define ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION(a, b) {b, CoreferenceDirectionEnum::a},
static const std::map<std::string, CoreferenceDirectionEnum> _coreferenceDirectionEnum_fromStr = {
  ONSEM_SEMANTIC_COREFERENCEDIRECTION_TABLE
};
#undef ADD_ONSEM_SEMANTIC_COREFERENCEDIRECTION
#undef ONSEM_SEMANTIC_COREFERENCEDIRECTION_TABLE


static inline char coreferenceDirectionEnum_toChar(CoreferenceDirectionEnum pCoreference)
{
  return static_cast<char>(pCoreference);
}

static inline CoreferenceDirectionEnum coreferenceDirectionEnum_fromChar(unsigned char pCoreference)
{
  return static_cast<CoreferenceDirectionEnum>(pCoreference);
}

static inline std::string coreferenceDirectionEnum_toStr
(CoreferenceDirectionEnum pCoreference)
{
  return _coreferenceDirectionEnum_toStr[coreferenceDirectionEnum_toChar(pCoreference)];
}

static inline CoreferenceDirectionEnum coreferenceDirectionEnum_fromStr
(const std::string& pCoreferenceStr)
{
  auto it = _coreferenceDirectionEnum_fromStr.find(pCoreferenceStr);
  if (it != _coreferenceDirectionEnum_fromStr.end())
    return it->second;
  assert(false);
  return CoreferenceDirectionEnum::UNKNOWN;
}



struct Coreference
{
  Coreference(CoreferenceDirectionEnum pDirection = CoreferenceDirectionEnum::BEFORE)
    : _direction(pDirection)
  {
  }

  bool operator==(const Coreference& pOther) const
  { return _direction == pOther._direction; }
  CoreferenceDirectionEnum getDirection() const { return _direction; }


private:
  CoreferenceDirectionEnum _direction;
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_MISC_COREFERENCE_HPP
