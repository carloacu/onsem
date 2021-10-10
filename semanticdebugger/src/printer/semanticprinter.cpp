#include <onsem/semanticdebugger/printer/semanticprinter.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include "sentimentspecprinter.hpp"


namespace onsem
{

namespace SemanticPrinter
{


void printSentiments(std::string& pRes,
                     const SemanticExpression& pSemExp,
                     const std::string& pCurrentUserId,
                     const ConceptSet& pConceptSet,
                     std::unique_ptr<SemanticAgentGrounding> pAuthorPtr,
                     SemanticSourceEnum pFrom)
{
  if (pAuthorPtr)
  {
    std::list<std::unique_ptr<SentimentContext>> sentContext;
    sentimentDetector::semExpToSentimentInfos(sentContext, pSemExp,
                                              *pAuthorPtr, pConceptSet);

    if (!sentContext.empty())
    {
      std::stringstream ss;
      for (const auto& currSentContext : sentContext)
      {
        SentimentSpecPrinter::printSentimentContext
            (ss, *currSentContext,
             semanticSourceEnum_toStr(pFrom),
             pCurrentUserId);
        ss << "\n";
      }
      pRes = ss.str();
    }
  }
}



} // End of namespace SemanticPrinter



} // End of namespace onsem
