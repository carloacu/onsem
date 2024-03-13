#ifndef ONSEM_SEMANTICTOTEXT_SENTIMENT_SEMANTDETECTOR_HPP
#define ONSEM_SEMANTICTOTEXT_SENTIMENT_SEMANTDETECTOR_HPP

#include <list>
#include <memory>
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include "../api.hpp"

namespace onsem {
struct GroundedExpression;
struct SemanticAgentGrounding;
struct SemanticExpression;
class ConceptSet;

namespace sentimentDetector {

ONSEMSEMANTICTOTEXT_API
std::unique_ptr<SentimentContext> extractMainSentiment(const GroundedExpression& pGrdExp,
                                                       const SemanticAgentGrounding& pAuthor,
                                                       const ConceptSet& pConceptSet);

ONSEMSEMANTICTOTEXT_API
void semExpToSentimentInfos(std::list<std::unique_ptr<SentimentContext>>& pSentContexts,
                            const SemanticExpression& pSemExp,
                            const SemanticAgentGrounding& pAuthor,
                            const ConceptSet& pConceptSet);

}    // End of namespace sentimentDetector
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SENTIMENT_SEMANTDETECTOR_HPP
