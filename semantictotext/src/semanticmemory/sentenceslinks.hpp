#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SENTENCELINKS_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SENTENCELINKS_HPP

#include <map>
#include <set>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinksid.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include "answerelement.hpp"
#include "semanticlinkstogrdexps.hpp"


namespace onsem
{
struct SemanticMemoryBlockPrivate;


enum class SemanticTypeOfLinks
{
  ANSWER,
  CONDITION_INFORMATION,
  SENT_WITH_ACTION
};


template <bool IS_MODIFIABLE>
struct SentenceLinks
{
  void clear() { dynamicLinks.clear(); staticLinks.clear(); }
  bool empty() const { return dynamicLinks.empty() && staticLinks.empty(); }

  typedef GroundedExpWithLinks memSentType;
  std::map<intSemId, memSentType*> dynamicLinks{};
  std::map<intSemId, std::unique_ptr<AnswerElementStatic>> staticLinks{};
};


template <>
struct SentenceLinks<false>
{
  void clear() { dynamicLinks.clear(); staticLinks.clear(); }
  bool empty() const { return dynamicLinks.empty() && staticLinks.empty(); }

  typedef const GroundedExpWithLinks memSentType;
  std::map<intSemId, memSentType*> dynamicLinks{};
  std::map<intSemId, std::unique_ptr<AnswerElementStatic>> staticLinks{};
};


struct RelationsThatMatchFilter
{
  virtual ~RelationsThatMatchFilter() {}
  virtual bool filterCondition(const GroundedExpWithLinks& pMemSent) const = 0;
  virtual bool filterConditionStatic(AnswerElementStatic& pAnswElt) const = 0;
};


template <bool IS_MODIFIABLE>
struct RelationsThatMatch
{
  template <typename MEMSENTENCES>
  void addMemSent(MEMSENTENCES& pMemSent)
  {
    if (filterPtr == nullptr ||
        filterPtr->filterCondition(pMemSent))
      res.dynamicLinks[pMemSent.id] = &pMemSent;
  }

  void addMemSentStatic(const unsigned char* pStaticMemorySentencePtr,
                        const linguistics::LinguisticDatabase& pLingDb);

  bool empty() const { return res.empty(); }
  void clear() { res.clear(); }
  SentenceLinks<IS_MODIFIABLE> res{};
  const RelationsThatMatchFilter* filterPtr{nullptr};
};


template <bool IS_MODIFIABLE>
struct RequestToMemoryLinks
{
  RequestToMemoryLinks(std::map<SemanticRequestType, SemanticLinksToGrdExps>& pD,
                       const unsigned char* pC)
   : d(pD),
     c(pC)
  {
  }

  std::map<SemanticRequestType, SemanticLinksToGrdExps>& d;
  const unsigned char* c;
};


template<>
struct RequestToMemoryLinks<false>
{
  RequestToMemoryLinks(const std::map<SemanticRequestType, SemanticLinksToGrdExps>& pD,
                       const unsigned char* pC)
   : d(pD),
     c(pC)
  {
  }

  const std::map<SemanticRequestType, SemanticLinksToGrdExps>& d;
  const unsigned char* c;
};


template <bool IS_MODIFIABLE>
struct MemoryLinksAccessor
{
  MemoryLinksAccessor(SemanticLinksToGrdExps* pD,
                      const unsigned char* pC)
   : d(pD),
     c(pC)
  {
  }

  bool empty() const { return d == nullptr && c == nullptr; }

  SemanticLinksToGrdExps* d;
  const unsigned char* c;
};


template<>
struct MemoryLinksAccessor<false>
{
  MemoryLinksAccessor(const SemanticLinksToGrdExps* pD,
                      const unsigned char* pC)
   : d(pD),
     c(pC)
  {
  }

  bool empty() const { return d == nullptr && c == nullptr; }

  const SemanticLinksToGrdExps* d;
  const unsigned char* c;
};




template <bool IS_MODIFIABLE>
struct MemoryBlockPrivateAccessorPtr
{
  MemoryBlockPrivateAccessorPtr(SemanticMemoryBlockPrivate* pMb)
   : mb(pMb)
  {
  }

  SemanticMemoryBlockPrivate* mb;
};


template<>
struct MemoryBlockPrivateAccessorPtr<false>
{
  MemoryBlockPrivateAccessorPtr(const SemanticMemoryBlockPrivate* pMb)
   : mb(pMb)
  {
  }

  const SemanticMemoryBlockPrivate* mb;
};


template <bool IS_MODIFIABLE>
struct MemoryBlockPrivateAccessor
{
  MemoryBlockPrivateAccessor(SemanticMemoryBlockPrivate& pMb)
   : mb(pMb)
  {
  }

  SemanticMemoryBlockPrivate& mb;
};


template<>
struct MemoryBlockPrivateAccessor<false>
{
  MemoryBlockPrivateAccessor(const SemanticMemoryBlockPrivate& pMb)
   : mb(pMb)
  {
  }

  const SemanticMemoryBlockPrivate& mb;
};




template <bool IS_MODIFIABLE>
struct IntIdToMemSentenceAccessor
{
  IntIdToMemSentenceAccessor(MemoryGrdExpLinks& pM)
   : m(pM)
  {
  }

  MemoryGrdExpLinks& m;
};


template <>
struct IntIdToMemSentenceAccessor<false>
{
  IntIdToMemSentenceAccessor(const MemoryGrdExpLinks& pM)
   : m(pM)
  {
  }

  const MemoryGrdExpLinks& m;
};



template <bool IS_MODIFIABLE>
struct EntityTypeToGrdExpAccessor
{
  EntityTypeToGrdExpAccessor(std::map<SemanticEntityType, MemoryGrdExpLinks>& pM)
   : m(pM)
  {
  }

  std::map<SemanticEntityType, MemoryGrdExpLinks>& m;
};


template <>
struct EntityTypeToGrdExpAccessor<false>
{
  EntityTypeToGrdExpAccessor(const std::map<SemanticEntityType, MemoryGrdExpLinks>& pM)
   : m(pM)
  {
  }

  const std::map<SemanticEntityType, MemoryGrdExpLinks>& m;
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SENTENCELINKS_HPP
