#ifndef ONSEM_COMMON_ENUM_SEMANTICLANGUAGEENUM_HPP
#define ONSEM_COMMON_ENUM_SEMANTICLANGUAGEENUM_HPP

#include <string>
#include <vector>
#include <map>
#include <onsem/common/utility/optional.hpp>


namespace onsem
{

#define MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE                                                 \
  ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(FRENCH, "french", "french", "fr_FR", "French")          \
  ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(ENGLISH, "english", "english", "en_US", "English")      \
  ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(JAPANESE, "japanese", "japanese", "ja_JP", "Japanese")  \
  ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(UNKNOWN, "unknown", "common", "unknown", "unknown")     \
  ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(OTHER, "other", "other", "ot_OT", "Other")


#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) a,
enum class SemanticLanguageEnum : char
{
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS




#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) b,
static const std::vector<std::string> _semanticLanguageEnum_toLegacyStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) c,
static const std::vector<std::string> _semanticLanguageEnum_toLanguageFilenameStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) d,
static const std::vector<std::string> _semanticLanguageEnum_toStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS



#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) {b, SemanticLanguageEnum::a},
static const std::map<std::string, SemanticLanguageEnum> _semanticLanguageEnum_fromLegacyStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) {c, SemanticLanguageEnum::a},
static const std::map<std::string, SemanticLanguageEnum> _semanticLanguageEnum_fromLanguageFilenameStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) {d, SemanticLanguageEnum::a},
static const std::map<std::string, SemanticLanguageEnum> _semanticLanguageEnum_fromStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) {e, SemanticLanguageEnum::a},
static const std::map<std::string, SemanticLanguageEnum> _semanticLanguageEnum_fromTtsStr = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS


#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) SemanticLanguageEnum::a,
static const std::vector<SemanticLanguageEnum> semanticLanguageEnum_allValues = {
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
};
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#define ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS(a, b, c, d, e) 1 +
static const std::size_t semanticLanguageEnum_size =
  MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE
0;
#undef ADD_MIND_SEMANTIC_LANGUAGEGROUNDINGS

#undef MIND_SEMANTIC_LANGUAGEGROUNDINGS_TABLE


static inline char semanticLanguageEnum_toChar(SemanticLanguageEnum pLangueType)
{
  return static_cast<char>(pLangueType);
}

static inline SemanticLanguageEnum semanticLanguageEnum_fromChar(unsigned char pLangueType)
{
  return static_cast<SemanticLanguageEnum>(pLangueType);
}

static inline std::string semanticLanguageEnum_toStr
(SemanticLanguageEnum pLangueType)
{
  return _semanticLanguageEnum_toStr[semanticLanguageEnum_toChar(pLangueType)];
}

static inline std::string semanticLanguageEnum_toLegacyStr
(SemanticLanguageEnum pLangueType)
{
  return _semanticLanguageEnum_toLegacyStr[semanticLanguageEnum_toChar(pLangueType)];
}

static inline std::string semanticLanguageEnum_toLanguageFilenameStr
(SemanticLanguageEnum pLangueType)
{
  return _semanticLanguageEnum_toLanguageFilenameStr[semanticLanguageEnum_toChar(pLangueType)];
}



static inline SemanticLanguageEnum semanticLanguageEnum_fromStr
(const std::string& pLangueStr)
{
  auto it = _semanticLanguageEnum_fromStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromStr.end())
  {
    return it->second;
  }
  return SemanticLanguageEnum::UNKNOWN;
}


static inline SemanticLanguageEnum semanticLanguageEnum_fromLanguageFilenameStr
(const std::string& pLangueStr)
{
  auto it = _semanticLanguageEnum_fromLanguageFilenameStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromLanguageFilenameStr.end())
    return it->second;
  return SemanticLanguageEnum::UNKNOWN;
}


static inline mystd::optional<SemanticLanguageEnum> semanticLanguageEnum_fromStrOpt
(const std::string& pLangueStr)
{
  mystd::optional<SemanticLanguageEnum> res;
  auto it = _semanticLanguageEnum_fromStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromStr.end())
  {
    res.emplace(it->second);
    return res;
  }
  return res;
}

static inline bool semanticLanguageEnum_fromStrIfExist
(SemanticLanguageEnum& pRes,
 const std::string& pLangueStr)
{
  auto it = _semanticLanguageEnum_fromStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromStr.end())
  {
    pRes = it->second;
    return true;
  }
  return false;
}

static inline bool semanticLanguageEnum_fromLegacyStrIfExist
(SemanticLanguageEnum& pRes,
 const std::string& pLangueStr)
{
  auto it = _semanticLanguageEnum_fromLegacyStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromLegacyStr.end())
  {
    pRes = it->second;
    return true;
  }
  return false;
}


static inline bool semanticLanguageEnum_fromTtsStrIfExist
(SemanticLanguageEnum& pRes,
 const std::string& pLangueStr)
{
  auto it = _semanticLanguageEnum_fromTtsStr.find(pLangueStr);
  if (it != _semanticLanguageEnum_fromTtsStr.end())
  {
    pRes = it->second;
    return true;
  }
  return false;
}


static inline std::string sayLanguageInLanguage
(SemanticLanguageEnum pLanguageConvertInText,
 SemanticLanguageEnum pInLanguage)
{
  switch (pInLanguage)
  {
  case SemanticLanguageEnum::ENGLISH:
  {
    switch (pLanguageConvertInText)
    {
    case SemanticLanguageEnum::ENGLISH:
    {
      return "English";
    }
    case SemanticLanguageEnum::FRENCH:
    {
      return "French";
    }
    case SemanticLanguageEnum::JAPANESE:
    {
      return "Japanese";
    }
    default:
      return "";
    }
    return "";
  }
  case SemanticLanguageEnum::FRENCH:
  {
    switch (pLanguageConvertInText)
    {
    case SemanticLanguageEnum::ENGLISH:
    {
      return "anglais";
    }
    case SemanticLanguageEnum::FRENCH:
    {
      return "français";
    }
    case SemanticLanguageEnum::JAPANESE:
    {
      return "japonais";
    }
    default:
      return "";
    }
    return "";
  }
  default:
    return "";
  }
}


static inline SemanticLanguageEnum semanticLanguageTypeGroundingEnumFromStr
(const std::string& pStr)
{
  SemanticLanguageEnum res  = SemanticLanguageEnum::UNKNOWN;
  if (semanticLanguageEnum_fromStrIfExist(res, pStr) ||
      semanticLanguageEnum_fromLegacyStrIfExist(res, pStr) ||
      semanticLanguageEnum_fromTtsStrIfExist(res, pStr))
  {
    return res;
  }

  return SemanticLanguageEnum::UNKNOWN;
}

} // End of namespace onsem


#endif // ONSEM_COMMON_ENUM_SEMANTICLANGUAGEENUM_HPP
