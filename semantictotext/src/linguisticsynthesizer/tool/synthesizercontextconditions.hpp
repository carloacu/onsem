#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONTEXTCONDITIONS_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONTEXTCONDITIONS_HPP

#include <string>
#include <onsem/common/utility/uppercasehandler.hpp>
#include "../synthesizertypes.hpp"

namespace onsem
{

inline bool isBeVerb(const OutSentence* pSentencePtr)
{
  return pSentencePtr != nullptr &&
      ((!pSentencePtr->verb.out.empty() &&
        pSentencePtr->verb.out.front().word.lemma == "être") ||
       (!pSentencePtr->aux.out.empty() &&
        pSentencePtr->aux.out.front().word.lemma == "être"));
}

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONTEXTCONDITIONS_HPP
