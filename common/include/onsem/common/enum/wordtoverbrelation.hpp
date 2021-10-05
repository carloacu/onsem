#ifndef ONSEM_COMMON_TYPE_ENUM_WORDTOVERBRELATION_HPP
#define ONSEM_COMMON_TYPE_ENUM_WORDTOVERBRELATION_HPP

#include <string>
#include <map>
#include <assert.h>

namespace onsem
{


#define WORDTOVERBRELLATION_TABLE                                               \
  WORDTOVERBRELLATION_ENUMVAL(NO, "no")                                         \
  WORDTOVERBRELLATION_ENUMVAL(ONLY_NEXT_TO_THE_VERB, "only_next_to_the_verb")   \
  WORDTOVERBRELLATION_ENUMVAL(YES, "yes")


#define WORDTOVERBRELLATION_ENUMVAL(a, b) a,
enum class WordToVerbRelation : char
{
  WORDTOVERBRELLATION_TABLE
};
#undef WORDTOVERBRELLATION_ENUMVAL


#define WORDTOVERBRELLATION_ENUMVAL(a, b) {b, WordToVerbRelation::a},
static const std::map<std::string, WordToVerbRelation> _wordToVerbRelations_fromStr = {
  WORDTOVERBRELLATION_TABLE
};
#undef WORDTOVERBRELLATION_ENUMVAL


inline static WordToVerbRelation wordToVerbRelations_fromStr
(const std::string& pStr)
{
  auto it = _wordToVerbRelations_fromStr.find(pStr);
  if (it != _wordToVerbRelations_fromStr.end())
  {
    return it->second;
  }
  assert(false);
  return WordToVerbRelation::NO;
}


inline static char wordToVerbRelations_toChar(WordToVerbRelation pWVR)
{
  return static_cast<char>(pWVR);
}

inline static WordToVerbRelation wordToVerbRelations_fromChar(char pWVR)
{
  return static_cast<WordToVerbRelation>(pWVR);
}


} // End of namespace onsem


#endif // ONSEM_COMMON_TYPE_ENUM_WORDTOVERBRELATION_HPP
