#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SENTIMENT_SENTIMENTCONTEXT_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SENTIMENT_SENTIMENTCONTEXT_HPP

#include "../../api.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API SentimentContext {
    SentimentContext(UniqueSemanticExpression&& pAuthor,
                     const std::string& pSentiment,
                     int pSentimentStrengh,
                     UniqueSemanticExpression&& pReceiver);

    SentimentContext(const SentimentContext&) = delete;
    SentimentContext& operator=(const SentimentContext&) = delete;

    UniqueSemanticExpression author;
    std::string sentiment;
    int sentimentStrengh;
    UniqueSemanticExpression receiver;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SENTIMENT_SENTIMENTCONTEXT_HPP
