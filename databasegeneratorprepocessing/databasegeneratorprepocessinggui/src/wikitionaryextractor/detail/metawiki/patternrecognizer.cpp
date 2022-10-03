#include "patternrecognizer.hpp"


void PatternRecognizer::getStradsStr
(std::list<std::string>& pTradsStr,
 const std::string& pLine)
{
  const std::string begTradSubStr = "{{t";
  const std::string endTradSubStr = "}}";
  std::size_t beginTrad = pLine.find(begTradSubStr, 6);
  while (beginTrad != std::string::npos)
  {
    std::size_t endTrad = pLine.find(endTradSubStr, beginTrad + 1);
    if (endTrad == std::string::npos)
    {
      return;
    }
    std::size_t currDelim = pLine.find_first_of("|}", beginTrad + 1);
    if (currDelim != std::string::npos)
    {
      std::size_t begWord = pLine.find_first_of("|}", currDelim + 1);
      if (begWord != std::string::npos)
      {
        ++begWord;
        std::size_t endWord = pLine.find_first_of("|}", begWord);
        if (endWord <= endTrad)
        {
          pTradsStr.emplace_back(pLine.substr(begWord, endWord - begWord));
        }
      }
    }
    beginTrad = pLine.find(begTradSubStr, endTrad);
  }
}
