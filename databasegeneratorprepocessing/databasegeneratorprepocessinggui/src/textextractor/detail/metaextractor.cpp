#include "metaextractor.hpp"
#include <iostream>
#include <onsem/common/utility/lexical_cast.hpp>


namespace onsem
{


void MetaExtractor::spitResultLine
(std::string& pLemma,
 std::vector<int>& pNbs,
 const std::string& pLine)
{
  const std::string sepStr = "\t#";
  std::size_t idSep = pLine.find(sepStr);
  if (idSep != std::string::npos &&
      idSep > 1)
  {
    pLemma = pLine.substr(1, idSep - 1);
    std::size_t begNb = idSep + sepStr.size();
    for (std::size_t i = 0; i < pNbs.size(); ++i)
    {
      std::size_t nextIdNb = pLine.find_first_of('_', begNb);
      pNbs[i] = mystd::lexical_cast<int>(pLine.substr(begNb, nextIdNb - begNb));
      begNb = nextIdNb + 1;
    }
  }
  else
  {
    std::cerr << "Line: \"" << pLine
              << "\" was no separator" << std::endl;
  }
}


} // End of namespace onsem
