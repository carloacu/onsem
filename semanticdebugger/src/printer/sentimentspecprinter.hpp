#ifndef SEMANTICANALYZER_ALSENTIMENTSPECPRINTER_H
#define SEMANTICANALYZER_ALSENTIMENTSPECPRINTER_H

#include <sstream>

namespace onsem
{
struct SentimentContext;

class ALSentimentSpecPrinter
{
public:
  static void printSentimentContext
  (std::stringstream& pRes,
   const SentimentContext& pSentimentContext,
   const std::string& pFromStr,
   const std::string& pCurrentUserId);
};



} // End of namespace onsem



#endif // SEMANTICANALYZER_ALSENTIMENTSPECPRINTER_H
