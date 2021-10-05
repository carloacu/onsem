#ifndef ONSEM_COMMON_TYPES_ENUM_SEMANTICSOURCEENUM_HPP
#define ONSEM_COMMON_TYPES_ENUM_SEMANTICSOURCEENUM_HPP

#include <string>
#include <map>
#include <vector>


namespace onsem
{

#define ONSEM_SEMANTIC_SOURCEENUM_TABLE                              \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(ASR, "asr")                          \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(EVENT, "event")                      \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(WRITTENTEXT, "written_text")         \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(TTS, "tts")                          \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(SEMREACTION, "sem_reaction")         \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(METHODCALL, "call")                  \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(PROPERTY, "property")                \
  ADD_ONSEM_SEMANTIC_SOURCEENUM(UNKNOWN, "unknown")


#define ADD_ONSEM_SEMANTIC_SOURCEENUM(a, b) a,
enum class SemanticSourceEnum : char
{
  ONSEM_SEMANTIC_SOURCEENUM_TABLE
};
#undef ADD_ONSEM_SEMANTIC_SOURCEENUM


#define ADD_ONSEM_SEMANTIC_SOURCEENUM(a, b) b,
static const std::vector<std::string> _semanticSourceEnum_toStr = {
  ONSEM_SEMANTIC_SOURCEENUM_TABLE
};
#undef ADD_ONSEM_SEMANTIC_SOURCEENUM

#define ADD_ONSEM_SEMANTIC_SOURCEENUM(a, b) {b, SemanticSourceEnum::a},
static const std::map<std::string, SemanticSourceEnum> _semanticSourceEnum_fromStr = {
  ONSEM_SEMANTIC_SOURCEENUM_TABLE
};
#undef ADD_ONSEM_SEMANTIC_SOURCEENUM
#undef ONSEM_SEMANTIC_SOURCEENUM_TABLE



static inline char semanticSourceEnum_toChar(SemanticSourceEnum pSourceType)
{
  return static_cast<char>(pSourceType);
}

static inline SemanticSourceEnum semanticSourceEnum_fromChar(unsigned char pSourceType)
{
  return static_cast<SemanticSourceEnum>(pSourceType);
}

static inline std::string semanticSourceEnum_toStr(SemanticSourceEnum pSourceType)
{
  return _semanticSourceEnum_toStr[semanticSourceEnum_toChar(pSourceType)];
}

static inline SemanticSourceEnum semanticSourceEnum_fromStr
(const std::string& pSourceTypeStr)
{
  auto it = _semanticSourceEnum_fromStr.find(pSourceTypeStr);
  if (it != _semanticSourceEnum_fromStr.end())
  {
    return it->second;
  }
  return SemanticSourceEnum::UNKNOWN;
}


} // End of namespace onsem



#endif // ONSEM_COMMON_TYPES_ENUM_SEMANTICSOURCEENUM_HPP
