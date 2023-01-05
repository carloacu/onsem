#ifndef ONSEM_COMMON_ENUM_CHUNKLINKTYPE_HPP
#define ONSEM_COMMON_ENUM_CHUNKLINKTYPE_HPP

#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/common/utility/to_underlying.hpp>

namespace onsem
{
namespace linguistics
{

#define SYNTACTIC_CHUNK_LINK_TABLE                                             \
  SYNTACTIC_CHUNK_LINK(SUBJECT, "subject", "blue")                             \
  SYNTACTIC_CHUNK_LINK(SUBJECT_OF, "subject of", "blue")                       \
  SYNTACTIC_CHUNK_LINK(AUXILIARY, "auxiliary", "blue")                         \
  SYNTACTIC_CHUNK_LINK(QUESTIONWORD, "question word", "black")                 \
  SYNTACTIC_CHUNK_LINK(REFLEXIVE, "reflexive", "green")                        \
  SYNTACTIC_CHUNK_LINK(INTERJECTION, "interjection", "black")                  \
  SYNTACTIC_CHUNK_LINK(INDIRECTOBJECT, "indirect object", "green")             \
  SYNTACTIC_CHUNK_LINK(DIRECTOBJECT, "direct object", "green")                 \
  SYNTACTIC_CHUNK_LINK(OBJECT_OF, "object of", "green")                        \
  SYNTACTIC_CHUNK_LINK(SUBORDINATE, "subordinate", "green")                    \
  SYNTACTIC_CHUNK_LINK(SUBORDINATE_CLAUSE, "subordinate clause", "green")      \
  SYNTACTIC_CHUNK_LINK(IF, "if", "green")                                      \
  SYNTACTIC_CHUNK_LINK(ELSE, "else", "green")                                  \
  SYNTACTIC_CHUNK_LINK(IN_CASE_OF, "in case of", "green")                      \
  SYNTACTIC_CHUNK_LINK(OWNER, "owner", "black")                                \
  SYNTACTIC_CHUNK_LINK(SPECIFICATION, "specification", "black")                \
  SYNTACTIC_CHUNK_LINK(COMPLEMENT, "complement", "black")                      \
  SYNTACTIC_CHUNK_LINK(TIME, "time", "green")                                  \
  SYNTACTIC_CHUNK_LINK(LENGTH, "length", "green")                              \
  SYNTACTIC_CHUNK_LINK(DURATION, "duration", "green")                          \
  SYNTACTIC_CHUNK_LINK(MANNER, "manner", "green")                              \
  SYNTACTIC_CHUNK_LINK(LOCATION, "location", "green")                          \
  SYNTACTIC_CHUNK_LINK(LANGUAGE, "language", "green")                          \
  SYNTACTIC_CHUNK_LINK(PURPOSE, "purpose", "green")                            \
  SYNTACTIC_CHUNK_LINK(PURPOSE_OF, "purpose of", "green")                      \
  SYNTACTIC_CHUNK_LINK(ACCORDINGTO, "according to", "green")                   \
  SYNTACTIC_CHUNK_LINK(AGAINST, "against", "green")                            \
  SYNTACTIC_CHUNK_LINK(BETWEEN, "between", "green")                            \
  SYNTACTIC_CHUNK_LINK(CAUSE, "cause", "green")                                \
  SYNTACTIC_CHUNK_LINK(STARTING_POINT, "starting point", "green")              \
  SYNTACTIC_CHUNK_LINK(MITIGATION, "mitigation", "green")                      \
  SYNTACTIC_CHUNK_LINK(OCCURRENCE_RANK, "occurrence rank", "green")            \
  SYNTACTIC_CHUNK_LINK(SIMILARITY, "similarity", "green")                      \
  SYNTACTIC_CHUNK_LINK(COMPARATOR_DIFFERENT, "comparator different", "green")  \
  SYNTACTIC_CHUNK_LINK(COMPARATOR_EQUAL, "comparator equal", "green")          \
  SYNTACTIC_CHUNK_LINK(COMPARATOR_LESS, "comparator less", "green")            \
  SYNTACTIC_CHUNK_LINK(COMPARATOR_MORE, "comparator more", "green")            \
  SYNTACTIC_CHUNK_LINK(RIGHTOPCOMPARISON, "right op comparison", "green")      \
  SYNTACTIC_CHUNK_LINK(REPETITION, "repetition", "green")                      \
  SYNTACTIC_CHUNK_LINK(DESPITE_CONTRAINT, "despite contraint", "green")        \
  SYNTACTIC_CHUNK_LINK(RECEIVER, "receiver", "green")                          \
  SYNTACTIC_CHUNK_LINK(REASONOF, "reason of", "green")                         \
  SYNTACTIC_CHUNK_LINK(THANKS_TO, "thanks to", "green")                        \
  SYNTACTIC_CHUNK_LINK(TODO, "todo", "green")                                  \
  SYNTACTIC_CHUNK_LINK(TOPIC, "topic", "green")                                \
  SYNTACTIC_CHUNK_LINK(UNITY, "unity", "green")                                \
  SYNTACTIC_CHUNK_LINK(WAY, "way", "green")                                    \
  SYNTACTIC_CHUNK_LINK(WITH, "with", "green")                                  \
  SYNTACTIC_CHUNK_LINK(WITHOUT, "without", "green")                            \
  SYNTACTIC_CHUNK_LINK(NOTUNDERSTOOD, "not understood", "red")                 \
  SYNTACTIC_CHUNK_LINK(IGNORE, "ignore", "red")                                \
  SYNTACTIC_CHUNK_LINK(SIMPLE, "", "black")


#define SYNTACTIC_CHUNK_LINK(a, b, c) a,
enum class ChunkLinkType
{
  SYNTACTIC_CHUNK_LINK_TABLE
};
#undef SYNTACTIC_CHUNK_LINK



#define SYNTACTIC_CHUNK_LINK(a, b, c) b,
static const std::vector<std::string> _chunkLinkType_toStr = {
  SYNTACTIC_CHUNK_LINK_TABLE
};
#undef SYNTACTIC_CHUNK_LINK

#define SYNTACTIC_CHUNK_LINK(a, b, c) c,
static const std::vector<std::string> _chunkLinkType_toColorStr = {
  SYNTACTIC_CHUNK_LINK_TABLE
};
#undef SYNTACTIC_CHUNK_LINK

#define SYNTACTIC_CHUNK_LINK(a, b, c) {b, ChunkLinkType::a},
static const std::map<std::string, ChunkLinkType> _chunkLinkType_fromStr = {
  SYNTACTIC_CHUNK_LINK_TABLE
};
#undef SYNTACTIC_CHUNK_LINK
#undef SYNTACTIC_CHUNK_LINK_TABLE


static inline std::string chunkLinkType_toStr
(ChunkLinkType pChunkLinkType)
{
  return _chunkLinkType_toStr[mystd::to_underlying(pChunkLinkType)];
}

static inline std::string chunkLinkType_toColorStr
(ChunkLinkType pChunkLinkType)
{
  return _chunkLinkType_toColorStr[mystd::to_underlying(pChunkLinkType)];
}

static inline ChunkLinkType chunkLinkType_fromStr(const std::string& pStr)
{
  return _chunkLinkType_fromStr.find(pStr)->second;
}


inline static ChunkLinkType chunkLinkType_fromCompPolarity
(ComparisonOperator pComparisonOperator)
{
  switch (pComparisonOperator)
  {
  case ComparisonOperator::DIFFERENT:
    return ChunkLinkType::COMPARATOR_DIFFERENT;
  case ComparisonOperator::EQUAL:
    return ChunkLinkType::COMPARATOR_EQUAL;
  case ComparisonOperator::MORE:
    return ChunkLinkType::COMPARATOR_MORE;
  case ComparisonOperator::LESS:
    return ChunkLinkType::COMPARATOR_LESS;
  }
  assert(false);
  return ChunkLinkType::SIMPLE;
}

inline static ComparisonOperator chunkLinkType_toCompPolarity
(ChunkLinkType pChunkLinkType)
{
  if (pChunkLinkType == ChunkLinkType::COMPARATOR_DIFFERENT)
    return ComparisonOperator::DIFFERENT;
  if (pChunkLinkType == ChunkLinkType::COMPARATOR_EQUAL)
    return ComparisonOperator::EQUAL;
  if (pChunkLinkType == ChunkLinkType::COMPARATOR_MORE)
    return ComparisonOperator::MORE;
  if (pChunkLinkType == ChunkLinkType::COMPARATOR_LESS)
    return ComparisonOperator::LESS;
  assert(false);
  return ComparisonOperator::DIFFERENT;
}

inline static bool chunkLinkType_isAComparator
(ChunkLinkType pChunkLinkType)
{
  return pChunkLinkType == ChunkLinkType::COMPARATOR_DIFFERENT ||
      pChunkLinkType == ChunkLinkType::COMPARATOR_EQUAL ||
      pChunkLinkType == ChunkLinkType::COMPARATOR_MORE ||
      pChunkLinkType == ChunkLinkType::COMPARATOR_LESS;
}


} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_COMMON_ENUM_CHUNKLINKTYPE_HPP
