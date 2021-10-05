#ifndef ONSEM_SEMANTICTOTEXT_SENTIMENT_SENTIMENTEXTRACTOR_HPP
#define ONSEM_SEMANTICTOTEXT_SENTIMENT_SENTIMENTEXTRACTOR_HPP

#include <list>
#include "../api.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticAgentGrounding;


class ONSEMSEMANTICTOTEXT_API SentimentAnalyzer
{
public:
  SentimentAnalyzer(const linguistics::LinguisticDatabase& pLingDb);

  void extract(std::list<std::unique_ptr<SentimentContext>>& pSentContexts,
               UniqueSemanticExpression pSemExp,
               const SemanticAgentGrounding& pAuthor);

  void inform(UniqueSemanticExpression pSemExp);

  void clear();

private:
  const SemanticMemory _semMem;
  const linguistics::LinguisticDatabase& _lingDb;
  UniqueSemanticExpression _prevSemExp;
};


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SENTIMENT_SENTIMENTEXTRACTOR_HPP
