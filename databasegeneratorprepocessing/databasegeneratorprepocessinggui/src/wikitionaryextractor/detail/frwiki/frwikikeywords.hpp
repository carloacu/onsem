#ifndef FRWIKIKEYWORDS_H
#define FRWIKIKEYWORDS_H

#include <map>
#include <vector>
#include "../metawiki/wikikeywords.hpp"

namespace onsem
{

class FRWikiKeyWords : public WikiKeyWords
{
public:
  FRWikiKeyWords();

  virtual void getGramEnum
  (std::set<PartOfSpeech>& pRes,
   const std::string& pGramStr,
   const std::string& pLine) const;

private:
  std::map<std::string, std::vector<PartOfSpeech> > fGramStrToGraEnum;
};



} // End of namespace onsem



#endif // FRWIKIKEYWORDS_H
