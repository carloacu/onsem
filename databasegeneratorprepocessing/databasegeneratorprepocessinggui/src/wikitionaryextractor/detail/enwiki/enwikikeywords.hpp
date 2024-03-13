#ifndef ENWIKIKEYWORDS_H
#define ENWIKIKEYWORDS_H

#include <map>
#include <vector>
#include "../metawiki/wikikeywords.hpp"

namespace onsem {

class EnWikiKeyWords : public WikiKeyWords {
public:
    EnWikiKeyWords();

    virtual void getGramEnum(std::set<PartOfSpeech>& pRes, const std::string& pGramStr, const std::string& pLine) const;

private:
    std::map<std::string, std::vector<PartOfSpeech> > fGramStrToGraEnum;

    static void xUpdateGramList(std::set<PartOfSpeech>& pRes, const std::vector<PartOfSpeech>& pVertGram);
};

}    // End of namespace onsem

#endif    // ENWIKIKEYWORDS_H
