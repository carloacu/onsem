#ifndef SEMANTICDEBUGGER_SRC_PRINTER_SENTIMENTSPECPRINTER_HPP
#define SEMANTICDEBUGGER_SRC_PRINTER_SENTIMENTSPECPRINTER_HPP

#include <sstream>

namespace onsem
{
struct SentimentContext;

class SentimentSpecPrinter
{
public:
  static void printSentimentContext
  (std::stringstream& pRes,
   const SentimentContext& pSentimentContext,
   const std::string& pFromStr,
   const std::string& pCurrentUserId);
};



} // End of namespace onsem



#endif // SEMANTICDEBUGGER_SRC_PRINTER_SENTIMENTSPECPRINTER_HPP
