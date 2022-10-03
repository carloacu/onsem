#include "frpatternrecognizer.hpp"
#include <iostream>

FrPatternRecognizer::FrPatternRecognizer()
  : fBeginLineOutLang("* {{T|en"),
    fBeginLineOutLang_size(fBeginLineOutLang.size()),
    fBeginLineOutLang2("*{{T|en"),
    fBeginLineOutLang_size2(fBeginLineOutLang2.size())
{
}


void FrPatternRecognizer::getSperator
(std::string& pSeparatorName,
 PatternRecognizerSeparatorType& pSeparatorType,
 const std::string& pLine) const
{
  pSeparatorType = PATTERNRECO_SEP_UNKNOWN;
  const std::string sepLabel = "{{S|";

  std::size_t beginSep = pLine.find(sepLabel, 2);
  if (beginSep != std::string::npos)
  {
    beginSep += sepLabel.size();
    std::size_t endSep = pLine.find_first_of("|}", beginSep);
    if (endSep != std::string::npos &&
        endSep > beginSep)
    {
      pSeparatorName = pLine.substr(beginSep, endSep - beginSep);
      if (pSeparatorName == "synonymes")
      {
        pSeparatorType = PATTERNRECO_SEP_SYNONYMS;
      }
      return;
    }
  }

  std::cerr << "isSeparator TRUE, but getGramInLineType fails, for line: " << pLine << std::endl;
}
