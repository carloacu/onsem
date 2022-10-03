#ifndef TEXTEXTRACTOR_METAEXTRACTOR_H
#define TEXTEXTRACTOR_METAEXTRACTOR_H

#include <vector>
#include <string>

namespace onsem
{

class MetaExtractor
{
protected:
  virtual ~MetaExtractor() {}

  static void spitResultLine
  (std::string& pLemma,
   std::vector<int>& pNbs,
   const std::string& pLine);
};

} // End of namespace onsem

#endif // TEXTEXTRACTOR_METAEXTRACTOR_H
