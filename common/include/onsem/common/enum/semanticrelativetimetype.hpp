#ifndef  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVETIMETYPE_HPP
#define  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVETIMETYPE_HPP

#include <string>
#include <map>
#include <vector>
#include <assert.h>

namespace onsem
{

#define SEMANTIC_RELATIVETIMETYPE_TABLE                        \
  ADD_SEMANTIC_RELATIVETIMETYPE(AFTER, "after")                \
  ADD_SEMANTIC_RELATIVETIMETYPE(BEFORE, "before")              \
  ADD_SEMANTIC_RELATIVETIMETYPE(DELAYEDSTART, "delayedStart")  \
  ADD_SEMANTIC_RELATIVETIMETYPE(DURING, "during")              \
  ADD_SEMANTIC_RELATIVETIMETYPE(JUSTAFTER, "justAfter")        \
  ADD_SEMANTIC_RELATIVETIMETYPE(JUSTBEFORE, "justBefore")      \
  ADD_SEMANTIC_RELATIVETIMETYPE(SINCE, "since")


#define ADD_SEMANTIC_RELATIVETIMETYPE(a, b) a,
enum class SemanticRelativeTimeType : char
{
  SEMANTIC_RELATIVETIMETYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVETIMETYPE




#define ADD_SEMANTIC_RELATIVETIMETYPE(a, b) b,
static const std::vector<std::string> _semanticRelativeTimeType_toStr = {
  SEMANTIC_RELATIVETIMETYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVETIMETYPE

#define ADD_SEMANTIC_RELATIVETIMETYPE(a, b) {b, SemanticRelativeTimeType::a},
static const std::map<std::string, SemanticRelativeTimeType> _semanticRelativeTimeType_fromStr = {
  SEMANTIC_RELATIVETIMETYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVETIMETYPE

#define ADD_SEMANTIC_RELATIVETIMETYPE(a, b) SemanticRelativeTimeType::a,
static const std::vector<SemanticRelativeTimeType> semanticRelativeTimeType_allValues = {
  SEMANTIC_RELATIVETIMETYPE_TABLE
};
#undef ADD_SEMANTIC_RELATIVETIMETYPE

#define ADD_SEMANTIC_RELATIVETIMETYPE(a, b) 1 +
static const std::size_t semanticRelativeTimeType_size =
  SEMANTIC_RELATIVETIMETYPE_TABLE
0;
#undef ADD_SEMANTIC_RELATIVETIMETYPE
#undef SEMANTIC_RELATIVETIMETYPE_TABLE


static inline char semanticRelativeTimeType_toChar(SemanticRelativeTimeType pRelTimeType)
{
  return static_cast<char>(pRelTimeType);
}

static inline SemanticRelativeTimeType semanticRelativeTimeType_fromChar(unsigned char pRelTimeType)
{
  return static_cast<SemanticRelativeTimeType>(pRelTimeType);
}


static inline std::string semanticRelativeTimeType_toStr(SemanticRelativeTimeType pRelTimeType)
{
  return _semanticRelativeTimeType_toStr[semanticRelativeTimeType_toChar(pRelTimeType)];
}

static inline SemanticRelativeTimeType semanticRelativeTimeType_fromStr
(const std::string& pRelTimeTypeStr)
{
  auto itOfConvertion = _semanticRelativeTimeType_fromStr.find(pRelTimeTypeStr);
  if (itOfConvertion != _semanticRelativeTimeType_fromStr.end())
    return itOfConvertion->second;
  assert(false);
  return SemanticRelativeTimeType::AFTER;
}

static inline SemanticRelativeTimeType semanticRelativeTimeType_fromChar(char pRelTimeTypeChar)
{
  return static_cast<SemanticRelativeTimeType>(pRelTimeTypeChar);
}


static inline bool semanticRelativeTimeType_fromStr_ifFound
(SemanticRelativeTimeType& pRelativeTime,
 const std::string& pRelTimeTypeStr)
{
  auto itOfConvertion = _semanticRelativeTimeType_fromStr.find(pRelTimeTypeStr);
  if (itOfConvertion != _semanticRelativeTimeType_fromStr.end())
  {
    pRelativeTime = itOfConvertion->second;
    return true;
  }
  return false;
}

} // End of namespace onsem

#endif //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVETIMETYPE_HPP
