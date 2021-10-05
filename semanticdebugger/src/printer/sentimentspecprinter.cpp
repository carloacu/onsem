#include "sentimentspecprinter.hpp"
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include <onsem/texttosemantic/printer/expressionprinter.hpp>


namespace onsem
{


void ALSentimentSpecPrinter::printSentimentContext
(std::stringstream& pRes,
 const SentimentContext& pSentimentContext,
 const std::string& pFromStr,
 const std::string& pCurrentUserId)
{
  {
    std::string authorStr;
    printer::oneWordPrint(authorStr, pSentimentContext.author,
                          pCurrentUserId);
    pRes << "[" << authorStr << ", ";
  }
  pRes << pSentimentContext.sentiment << ", ";
  pRes << pSentimentContext.sentimentStrengh << ", ";
  {
    std::string receiverStr;
    printer::oneWordPrint(receiverStr, pSentimentContext.receiver,
                          pCurrentUserId);
    pRes << receiverStr << ", ";
  }
  pRes << pFromStr << "]";
}



} // End of namespace onsem
