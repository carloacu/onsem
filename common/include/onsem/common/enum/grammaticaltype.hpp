#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_GRAMMATICALTYPE_HPP
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_GRAMMATICALTYPE_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem
{

#define SEMANTIC_GRAMMATICALTYPE_TABLE                              \
  SEMANTIC_GRAMMATICALTYPE(SUBJECT, "subject")                      \
  SEMANTIC_GRAMMATICALTYPE(OBJECT, "object")                        \
  SEMANTIC_GRAMMATICALTYPE(RECEIVER, "receiver")                    \
  SEMANTIC_GRAMMATICALTYPE(LOCATION, "location")                    \
  SEMANTIC_GRAMMATICALTYPE(LANGUAGE, "language")                    \
  SEMANTIC_GRAMMATICALTYPE(CAUSE, "cause")                          \
  SEMANTIC_GRAMMATICALTYPE(MANNER, "manner")                        \
  SEMANTIC_GRAMMATICALTYPE(LENGTH, "length")                        \
  SEMANTIC_GRAMMATICALTYPE(DURATION, "duration")                    \
  SEMANTIC_GRAMMATICALTYPE(TIME, "time")                            \
  SEMANTIC_GRAMMATICALTYPE(SPECIFIER, "specifier")                  \
  SEMANTIC_GRAMMATICALTYPE(MITIGATION, "mitigation")                \
  SEMANTIC_GRAMMATICALTYPE(SIMILARITY, "similarity")                \
  SEMANTIC_GRAMMATICALTYPE(OWNER, "owner")                          \
  SEMANTIC_GRAMMATICALTYPE(PURPOSE, "purpose")                      \
  SEMANTIC_GRAMMATICALTYPE(REASONOF, "reason_of")                   \
  SEMANTIC_GRAMMATICALTYPE(REPETITION, "repetition")                \
  SEMANTIC_GRAMMATICALTYPE(INTERVAL, "interval")                    \
  SEMANTIC_GRAMMATICALTYPE(ACCORDING_TO, "according_to")            \
  SEMANTIC_GRAMMATICALTYPE(AGAINST, "against")                      \
  SEMANTIC_GRAMMATICALTYPE(BETWEEN, "between")                      \
  SEMANTIC_GRAMMATICALTYPE(STARTING_POINT, "starting_point")        \
  SEMANTIC_GRAMMATICALTYPE(IN_BACKGROUND, "in_background")          \
  SEMANTIC_GRAMMATICALTYPE(IN_CASE_OF, "in_case_of")                \
  SEMANTIC_GRAMMATICALTYPE(SUB_CONCEPT, "sub_concept")              \
  SEMANTIC_GRAMMATICALTYPE(OCCURRENCE_RANK, "occurrence_rank")      \
  SEMANTIC_GRAMMATICALTYPE(DESPITE_CONTRAINT, "despite_contraint")  \
  SEMANTIC_GRAMMATICALTYPE(OTHER_THAN, "other_than")                \
  SEMANTIC_GRAMMATICALTYPE(THANKS_TO, "thanks_to")                  \
  SEMANTIC_GRAMMATICALTYPE(TODO, "todo")                            \
  SEMANTIC_GRAMMATICALTYPE(TOPIC, "topic")                          \
  SEMANTIC_GRAMMATICALTYPE(UNITY, "unity")                          \
  SEMANTIC_GRAMMATICALTYPE(WAY, "way")                              \
  SEMANTIC_GRAMMATICALTYPE(WITH, "with")                            \
  SEMANTIC_GRAMMATICALTYPE(WITHOUT, "without")                      \
  SEMANTIC_GRAMMATICALTYPE(SUBORDINATE, "subordinate")              \
  SEMANTIC_GRAMMATICALTYPE(INTRODUCTING_WORD, "introducting_word")  \
  SEMANTIC_GRAMMATICALTYPE(NOT_UNDERSTOOD, "not_understood")        \
  SEMANTIC_GRAMMATICALTYPE(UNKNOWN, "unknown")


#define SEMANTIC_GRAMMATICALTYPE(a, b) a,
enum class GrammaticalType : char
{
  SEMANTIC_GRAMMATICALTYPE_TABLE
};
#undef SEMANTIC_GRAMMATICALTYPE


#define SEMANTIC_GRAMMATICALTYPE(a, b) b,
static const std::vector<std::string> _grammaticalType_toStr = {
  SEMANTIC_GRAMMATICALTYPE_TABLE
};
#undef SEMANTIC_GRAMMATICALTYPE

#define SEMANTIC_GRAMMATICALTYPE(a, b) {b, GrammaticalType::a},
static const std::map<std::string, GrammaticalType> _grammaticalType_fromStr = {
  SEMANTIC_GRAMMATICALTYPE_TABLE
};
#undef SEMANTIC_GRAMMATICALTYPE

#define SEMANTIC_GRAMMATICALTYPE(a, b) GrammaticalType::a,
static const std::vector<GrammaticalType> grammaticalType_allValues = {
  SEMANTIC_GRAMMATICALTYPE_TABLE
};
#undef SEMANTIC_GRAMMATICALTYPE

#define SEMANTIC_GRAMMATICALTYPE(a, b) 1 +
static const std::size_t grammaticalType_size =
  SEMANTIC_GRAMMATICALTYPE_TABLE
0;
#undef SEMANTIC_GRAMMATICALTYPE

#undef SEMANTIC_GRAMMATICALTYPE_TABLE



static inline char grammaticalType_toChar(GrammaticalType pGramType)
{
  return static_cast<char>(pGramType);
}

static inline GrammaticalType grammaticalType_fromChar(unsigned char pGramType)
{
  return static_cast<GrammaticalType>(pGramType);
}

static inline std::string grammaticalType_toStr(GrammaticalType pGramType)
{
  return _grammaticalType_toStr[grammaticalType_toChar(pGramType)];
}

static inline GrammaticalType grammaticalType_fromStr
(const std::string& pGramTypeStr)
{
  auto it = _grammaticalType_fromStr.find(pGramTypeStr);
  if (it != _grammaticalType_fromStr.end())
  {
    return it->second;
  }
  return GrammaticalType::UNKNOWN;
}


} // End of namespace onsem

#endif // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_GRAMMATICALTYPE_HPP
