#ifndef  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVEDURATIONTYPE_HPP
#define  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVEDURATIONTYPE_HPP

#include <string>
#include <map>
#include <vector>
#include <assert.h>

namespace onsem
{

#define SEMANTIC_RELATIVEDURATIONTYPE_TABLE                         \
  ADD_SEMANTIC_RELATIVEDURATIONTYPE(DELAYEDSTART, "delayedStart")   \
  ADD_SEMANTIC_RELATIVEDURATIONTYPE(UNTIL, "until")


#define ADD_SEMANTIC_RELATIVEDURATIONTYPE(a, b) a,
enum class SemanticRelativeDurationType : char
{
  SEMANTIC_RELATIVEDURATIONTYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVEDURATIONTYPE




#define ADD_SEMANTIC_RELATIVEDURATIONTYPE(a, b) b,
static const std::vector<std::string> _semanticRelativeDurationType_toStr = {
  SEMANTIC_RELATIVEDURATIONTYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVEDURATIONTYPE

#define ADD_SEMANTIC_RELATIVEDURATIONTYPE(a, b) {b, SemanticRelativeDurationType::a},
static const std::map<std::string, SemanticRelativeDurationType> _semanticRelativeDurationType_fromStr = {
  SEMANTIC_RELATIVEDURATIONTYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVEDURATIONTYPE


static inline char semanticRelativeDurationType_toChar(SemanticRelativeDurationType pRelDurationType)
{
  return static_cast<char>(pRelDurationType);
}

static inline SemanticRelativeDurationType semanticRelativeDurationType_fromChar(unsigned char pRelDurationType)
{
  return static_cast<SemanticRelativeDurationType>(pRelDurationType);
}


static inline std::string semanticRelativeDurationType_toStr(SemanticRelativeDurationType pRelDurationType)
{
  return _semanticRelativeDurationType_toStr[semanticRelativeDurationType_toChar(pRelDurationType)];
}


static inline SemanticRelativeDurationType semanticRelativeDurationType_fromStr
(const std::string& pRelDurationTypeStr)
{
  auto itOfConvertion = _semanticRelativeDurationType_fromStr.find(pRelDurationTypeStr);
  if (itOfConvertion != _semanticRelativeDurationType_fromStr.end())
    return itOfConvertion->second;
  assert(false);
  return SemanticRelativeDurationType::UNTIL;
}

static inline SemanticRelativeDurationType semanticRelativeDurationType_fromChar(char pRelDurationTypeChar)
{
  return static_cast<SemanticRelativeDurationType>(pRelDurationTypeChar);
}


static inline bool semanticRelativeDurationType_fromStr_ifFound
(SemanticRelativeDurationType& pRelativeDuration,
 const std::string& pRelDurationTypeStr)
{
  auto itOfConvertion = _semanticRelativeDurationType_fromStr.find(pRelDurationTypeStr);
  if (itOfConvertion != _semanticRelativeDurationType_fromStr.end())
  {
    pRelativeDuration = itOfConvertion->second;
    return true;
  }
  return false;
}


} // End of namespace onsem

#endif //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVEDURATIONTYPE_HPP
