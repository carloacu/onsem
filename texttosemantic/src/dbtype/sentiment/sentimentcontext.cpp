#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>

namespace onsem {

SentimentContext::SentimentContext(UniqueSemanticExpression&& pAuthor,
                                   const std::string& pSentiment,
                                   int pSentimentStrengh,
                                   UniqueSemanticExpression&& pReceiver)
    : author(std::move(pAuthor))
    , sentiment(pSentiment)
    , sentimentStrengh(pSentimentStrengh)
    , receiver(std::move(pReceiver)) {}

}    // End of namespace onsem
