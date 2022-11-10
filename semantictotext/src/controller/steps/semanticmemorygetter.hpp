#ifndef ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYGETTER_HPP

#include <map>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>
#include "../../semanticmemory/semanticlinkstogrdexps.hpp"
#include "../../semanticmemory/sentenceslinks.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
namespace semanticMemoryLinker
{
struct RequestLinks;
}
struct MemorySentencesInResult
{
  bool empty() const
  {
    return idsToDynamicSentences.empty() &&
        idToConstDynamicSentences.empty() &&
        idToStaticSentences.empty();
  }
  std::map<intSemId, SemanticMemorySentence*> idsToDynamicSentences;
  std::map<intSemId, const SemanticMemorySentence*> idToConstDynamicSentences;
  std::map<intSemId, std::unique_ptr<AnswerElementStatic>> idToStaticSentences;

};

namespace semanticMemoryGetter
{

enum class RequestContext
{
  SENTENCE,
  SENTENCE_TO_CONDITION,
  COMMAND
};



void findSentenceThatContainANominalGroup(RelationsThatMatch<false>& pRes,
                                          const SemanticExpression& pNominalGroup,
                                          const SemanticMemoryBlock& pMemBlock,
                                          const linguistics::LinguisticDatabase& pLingDb);

void findGrdExpInNominalGroupLinks(std::set<const ExpressionWithLinks*>& pRes,
                                   const GroundedExpression& pGrdExp,
                                   const SemanticMemoryBlock& pMemBlock,
                                   const linguistics::LinguisticDatabase& pLingDb);

void findGrdExpWithCoefInNominalGroupLinks(std::map<const ExpressionWithLinks*, int>& pRes,
                                           const GroundedExpression& pGrdExp,
                                           const mystd::optional<int>& pGroundingCoef,
                                           const SemanticMemoryBlock& pMemBlock,
                                           const linguistics::LinguisticDatabase& pLingDb);

template <bool IS_MODIFIABLE>
void getResultFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRes,
                         RequestToMemoryLinks<IS_MODIFIABLE>& pReqToList,
                         const semanticMemoryLinker::RequestLinks& pReqLinks,
                         RequestContext pRequestContext,
                         MemoryBlockPrivateAccessorPtr<IS_MODIFIABLE>& pMemBlockPrivate,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pCheckTimeRequest,
                         bool pConsiderCoreferences,
                         bool pFirstIteration = true,
                         const RelationsThatMatchFilter* pFilterPtr = nullptr);


bool getResultMatchingNowTimeFromMemory(RelationsThatMatch<true>& pRelations,
                                        const SentenceLinks<true>& pAlreadyMatchedSentences,
                                        RequestToMemoryLinks<true>& pReqToGrdExps,
                                        const SemanticDuration& pNowTimeDuration,
                                        const SemanticMemoryBlockPrivate& pMemBlockPrivate,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        bool pCheckChildren);

} // End of namespace semanticMemoryGetter
} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYGETTER_HPP
