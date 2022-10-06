#ifndef  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREQUESTTYPE_H
#define  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREQUESTTYPE_H

#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <onsem/common/enum/grammaticaltype.hpp>
#include "../api.hpp"

namespace onsem
{

// 1) Please keep the action as the first element!
// It's because any sentence have an action (verb) so when we
// we are in getResultFromMemory() in SentenceToCondition state
// we don't have to filter the elt that don't have an action
// 2) Please keep the time as the last element (except the nothing element)
// It's because, for eg, we may want to get only the most recent answer,
// so we need to have the final list of sentences when we filter with the time child.
// As we iterate on semExp children in getResultFromMemory() according to this list order
// if we want to consider the time child after the others we need to put it in the end of the enum.


#define SEMANTIC_REQUEST_TABLE                              \
  ADD_SEMANTIC_REQUEST_TYPE(YESORNO, "yes or no",           \
                            GrammaticalType::UNKNOWN)       \
  ADD_SEMANTIC_REQUEST_TYPE(ACTION, "action",               \
                            GrammaticalType::UNKNOWN)       \
  ADD_SEMANTIC_REQUEST_TYPE(LOCATION, "location",           \
                            GrammaticalType::LOCATION)      \
  ADD_SEMANTIC_REQUEST_TYPE(OBJECT, "object",               \
                            GrammaticalType::OBJECT)        \
  ADD_SEMANTIC_REQUEST_TYPE(VERB, "verb",                   \
                            GrammaticalType::UNKNOWN)       \
  ADD_SEMANTIC_REQUEST_TYPE(TIMES, "times",                 \
                            GrammaticalType::UNKNOWN)       \
  ADD_SEMANTIC_REQUEST_TYPE(PURPOSE, "purpose",             \
                            GrammaticalType::PURPOSE)       \
  ADD_SEMANTIC_REQUEST_TYPE(CAUSE, "cause",                 \
                            GrammaticalType::CAUSE)         \
  ADD_SEMANTIC_REQUEST_TYPE(CHOICE, "choice",               \
                            GrammaticalType::OBJECT)        \
  ADD_SEMANTIC_REQUEST_TYPE(MANNER, "manner",               \
                            GrammaticalType::MANNER)        \
  ADD_SEMANTIC_REQUEST_TYPE(QUANTITY, "quantity",           \
                            GrammaticalType::OBJECT)        \
  ADD_SEMANTIC_REQUEST_TYPE(SUBJECT, "subject",             \
                            GrammaticalType::SUBJECT)       \
  ADD_SEMANTIC_REQUEST_TYPE(TIME, "time",                   \
                            GrammaticalType::TIME)          \
  ADD_SEMANTIC_REQUEST_TYPE(DISTANCE, "distance",           \
                            GrammaticalType::DISTANCE)      \
  ADD_SEMANTIC_REQUEST_TYPE(DURATION, "duration",           \
                            GrammaticalType::DURATION)      \
  ADD_SEMANTIC_REQUEST_TYPE(TOPIC, "topic",                 \
                            GrammaticalType::TOPIC)         \
  ADD_SEMANTIC_REQUEST_TYPE(WAY, "way",                     \
                            GrammaticalType::WAY)           \
  ADD_SEMANTIC_REQUEST_TYPE(NOTHING, "nothing",             \
                            GrammaticalType::UNKNOWN)


#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) a,
enum class SemanticRequestType : char
{
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE



#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) b,
static const std::vector<std::string> _semanticRequestType_toStr = {
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE

#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) c,
static const std::vector<GrammaticalType> _semanticRequestType_toSemGram = {
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE

#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) {c, SemanticRequestType::a},
static const std::map<GrammaticalType, SemanticRequestType> _semanticRequestType_fromSemGram = {
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE

#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) {b, SemanticRequestType::a},
static const std::map<std::string, SemanticRequestType> _semanticRequestType_fromStr = {
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE

#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) SemanticRequestType::a,
static const std::vector<SemanticRequestType> semanticRequestType_allValues = {
  SEMANTIC_REQUEST_TABLE
};
#undef ADD_SEMANTIC_REQUEST_TYPE

#define ADD_SEMANTIC_REQUEST_TYPE(a, b, c) 1 +
static const std::size_t semanticRequestType_size =
  SEMANTIC_REQUEST_TABLE
0;
#undef ADD_SEMANTIC_REQUEST_TYPE

#undef SEMANTIC_REQUEST_TABLE




static inline unsigned char semanticRequestType_toChar(SemanticRequestType pRequestType)
{
  return static_cast<unsigned char>(pRequestType);
}

static inline SemanticRequestType semanticRequestType_fromChar(unsigned char pRequestType)
{
  return static_cast<SemanticRequestType>(pRequestType);
}

static inline std::string semanticRequestType_toStr
(SemanticRequestType pRequestType)
{
  return _semanticRequestType_toStr[semanticRequestType_toChar(pRequestType)];
}

static inline GrammaticalType semanticRequestType_toSemGram
(SemanticRequestType pRequestType)
{
  return _semanticRequestType_toSemGram[semanticRequestType_toChar(pRequestType)];
}

static inline SemanticRequestType semanticRequestType_fromSemGram
(GrammaticalType pGrammType)
{
  return _semanticRequestType_fromSemGram.find(pGrammType)->second;
}

static inline SemanticRequestType semanticRequestType_fromStr
(const std::string& pRequestTypeStr)
{
  auto it = _semanticRequestType_fromStr.find(pRequestTypeStr);
  if (it != _semanticRequestType_fromStr.end())
  {
    return it->second;
  }
  return SemanticRequestType::NOTHING;
}

static inline void setRequest(
    std::vector<SemanticRequestType>& pRequests,
    SemanticRequestType pRequest)
{
  auto requestsSize = pRequests.size();
  if (requestsSize == 0)
  {
    pRequests.push_back(pRequest);
  }
  else
  {
    if (requestsSize > 1)
      pRequests.resize(1);
    pRequests[0] = pRequest;
  }
}


struct ONSEM_COMMON_API SemanticRequests
{
  SemanticRequests(SemanticRequestType pRequestType)
    : types(1, pRequestType)
  {
  }
  SemanticRequests()
    : types()
  {
  }
  std::vector<SemanticRequestType> types;

  bool isAQuestion() const
  {
    bool res = false;
    for (const auto& type : types)
      if (type != SemanticRequestType::ACTION)
        res = true;
    return res;
  }
  bool isAQuestionNotAskingAboutTheObject() const
  {
    bool res = false;
    for (const auto& type : types)
      if (type != SemanticRequestType::ACTION &&
          type != SemanticRequestType::OBJECT)
        res = true;
    return res;
  }
  bool has(SemanticRequestType pRequest) const
  {
    for (const auto& currRequest : types)
      if (currRequest == pRequest)
        return true;
    return false;
  }
  void addFront(SemanticRequestType pRequest)
  {
    if (!has(pRequest))
      types.insert(types.begin(), pRequest);
  }
  void add(SemanticRequestType pRequest)
  {
    if (!has(pRequest))
      types.push_back(pRequest);
  }
  void addWithoutCollisionCheck(SemanticRequestType pRequest)
  {
    types.push_back(pRequest);
  }
  void set(SemanticRequestType pRequest)
  {
    setRequest(types, pRequest);
  }
  void erase(SemanticRequestType pRequest)
  {
    for (auto it = types.begin(); it != types.end(); )
    {
      if (*it == pRequest)
        it = types.erase(it);
      else
        ++it;
    }
  }
  void swap(SemanticRequests& pOther)
  {
    types.swap(pOther.types);
  }
  void clear()
  {
    types.clear();
  }
  void removeFirst()
  {
    if (!types.empty())
      types.erase(types.begin());
  }
  SemanticRequestType firstOrNothing() const
  {
    for (const auto& currRequest : types)
      return currRequest;
    return SemanticRequestType::NOTHING;
  }
  SemanticRequestType first() const
  {
    for (const auto& currRequest : types)
      return currRequest;
    assert(false);
    return SemanticRequestType::NOTHING;
  }
  bool empty() const
  {
    return types.empty();
  }
  bool operator==(const SemanticRequests& pOther) const
  {
    return types == pOther.types;
  }
  bool operator!=(const SemanticRequests& pOther) const
  {
    return types != pOther.types;
  }
  bool isASubSetOf(const SemanticRequests& pOther) const
  {
    for (const auto& currRequest : types)
      if (!pOther.has(currRequest))
        return false;
    return true;
  }
};



} // End of namespace onsem


#endif //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREQUESTTYPE_H
